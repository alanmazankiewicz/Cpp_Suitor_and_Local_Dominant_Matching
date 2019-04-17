#include "implementation/parallel_dominant.hpp"


void ParallelDominant::processVertex(NodeHandle u, moodycamel::ConcurrentQueue<NodeHandle> &Q, Mate &candidate)
{
    double max_wt = 0;
    NodeHandle max_wt_id = G.null_node();
    size_t u_id = G.nodeId(u);

    for (EdgeIterator t = G.beginEdges(u); t != G.endEdges(u); ++t)
    {
        if ((mate[*t] == G.null_node()) && (max_wt < G.edgeWeight(t)))
        {
            max_wt = G.edgeWeight(t);
            max_wt_id = G.node(*t);
        }
    }
    candidate[u_id] = max_wt_id;

    if (candidate[u_id] == G.null_node())
        return;

    if (G.nodeId(candidate[G.nodeId(candidate[u_id])]) == u_id)
    {
        /*
         * Actually this is a data race but in this case it is ok because if two thread are going to write simultaneously
         * they are going to write the exact same value.
         *
         * https://stackoverflow.com/questions/8315931/does-writing-the-same-value-to-the-same-memory-location-cause-a-data-race
         * https://stackoverflow.com/questions/22959519/two-more-more-threads-writing-the-same-value-to-the-same-memory-location
         */

        mate[u_id] = candidate[u_id];
        mate[G.nodeId(candidate[u_id])] = u;
        Q.enqueue(u);
        Q.enqueue(candidate[u_id]);
    }
}

void ParallelDominant::parallelProcessVertexSec1(NodeHandle start, NodeHandle finish, Mate &candidate)
{
    for (EdgeIterator u = start; u != finish; ++u)
    {
        double max_wt = 0;
        NodeHandle max_wt_id = G.null_node();

        for (EdgeIterator t = G.beginEdges(u); t != G.endEdges(u); ++t)
        {
            if ((mate[*t] == G.null_node()) && (max_wt < G.edgeWeight(t)))
            {
                max_wt = G.edgeWeight(t);
                max_wt_id = G.node(*t);
            }
        }
        candidate[G.nodeId(u)] = max_wt_id;
    }
}

void ParallelDominant::parallelProcessVertexSec2(NodeHandle start, NodeHandle finish, Mate &candidate,
                                                 moodycamel::ConcurrentQueue<NodeHandle> &QC)
{
    for (EdgeIterator u = start; u != finish; ++u)
    {
        size_t u_id = G.nodeId(u);

        if (candidate[u_id] == G.null_node())
            continue;

        if (G.nodeId(candidate[G.nodeId(candidate[u_id])]) == u_id)
        {
            mate[u_id] = candidate[u_id];
            QC.enqueue(u);
        }
    }
}

void ParallelDominant::phase2(Mate &candidate, moodycamel::ConcurrentQueue<NodeHandle> &QC,
                              moodycamel::ConcurrentQueue<NodeHandle> &QN)
{
    NodeHandle u;
    while (QC.try_dequeue(u))
    {
        for (EdgeIterator v = G.beginEdges(u); v != G.endEdges(u); ++v)
        {
            if ((G.nodeId(mate[G.nodeId(u)]) != *v) && (G.nodeId(candidate[*v]) == G.nodeId(u)))
            {
                processVertex(G.node(*v), QN, candidate);
            }
        }
    }
}

void ParallelDominant::simple_procedure(size_t num_threads)
{
    Mate candidate(G.numNodes(), G.null_node());
    moodycamel::ConcurrentQueue<NodeHandle> QC(G.numNodes());
    moodycamel::ConcurrentQueue<NodeHandle> QN(G.numNodes());
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    size_t nodes_per_thread = G.numNodes() / num_threads;

    // Phase 1
    // Sec1
    if(!nodes_per_thread)
    {
        for (size_t thread = 0; thread < G.numNodes(); ++thread)
        {
            threads.emplace_back(std::thread(&ParallelDominant::parallelProcessVertexSec1, this,
                                             G.begin() + thread, G.begin() + (thread + 1), std::ref(candidate)));
        }
    } else
    {
        for (size_t thread = 0; thread < num_threads; ++thread)
        {
            if (thread != num_threads - 1)
            {
                threads.emplace_back(std::thread(&ParallelDominant::parallelProcessVertexSec1, this,
                                                 G.begin() + thread * nodes_per_thread,
                                                 G.begin() + (thread + 1) * nodes_per_thread, std::ref(candidate)));
            } else
            {
                threads.emplace_back(std::thread(&ParallelDominant::parallelProcessVertexSec1, this,
                                                 G.begin() + thread * nodes_per_thread, G.end(),
                                                 std::ref(candidate)));
            }
        }
    }

    for (auto iter = threads.begin(); iter != threads.end(); ++iter)
    {
        iter->join();
    }
    threads.clear();



    // Sec2
    if(!nodes_per_thread)
    {
        for (size_t thread = 0; thread < G.numNodes(); ++thread)
        {
            threads.emplace_back(std::thread(&ParallelDominant::parallelProcessVertexSec2, this,
                                             G.begin() + thread , G.begin() + (thread + 1), std::ref(candidate),
                                             std::ref(QC)));
        }
    } else
    {
        for (size_t thread = 0; thread < num_threads; ++thread)
        {
            if (thread != num_threads - 1)
            {
                threads.emplace_back(std::thread(&ParallelDominant::parallelProcessVertexSec2, this,
                                                 G.begin() + thread * nodes_per_thread,
                                                 G.begin() + (thread + 1) * nodes_per_thread, std::ref(candidate),
                                                 std::ref(QC)));
            } else
            {
                threads.emplace_back(std::thread(&ParallelDominant::parallelProcessVertexSec2, this,
                                                 G.begin() + thread * nodes_per_thread, G.end(),
                                                 std::ref(candidate), std::ref(QC)));
            }
        }
    }

    for (auto iter = threads.begin(); iter != threads.end(); ++iter)
    {
        iter->join();
    }
    threads.clear();


    // phase2
    while (QC.size_approx())
    {
        for (size_t thread = 0; thread < num_threads; ++thread)
        {
            threads.emplace_back(
                    std::thread(&ParallelDominant::phase2, this, std::ref(candidate), std::ref(QC), std::ref(QN)));
        }

        for (auto iter = threads.begin(); iter != threads.end(); ++iter)
        {
            iter->join();
        }
        threads.clear();

        while (QN.size_approx())
        {
            NodeHandle u;
            QN.try_dequeue(u);
            QC.enqueue(u);
        }
    }
}


void ParallelDominant::print_matching() const
{
    for (auto iter = mate.begin(); iter != mate.end(); ++iter)
    {
        if (*iter != G.null_node())
        {
            std::cout << iter - mate.begin() << " " << G.nodeId(*iter) << std::endl;
        }
    }
}

const double ParallelDominant::matching_quality() const
{
    double total_weight = 0;
    for (auto iter = mate.begin(); iter != mate.end(); ++iter)
    {
        if (*iter != G.null_node())
        {
            EdgeIterator edgeiter = std::find(G.beginEdges(*iter), G.endEdges(*iter), iter - mate.begin());
            total_weight += G.edgeWeight(edgeiter);
        }
    }
    return total_weight / 2;
}

void ParallelDominant::clear_mate()
{
    for (auto iter = mate.begin(); iter != mate.end(); ++iter)
    {
        *iter = G.null_node();
    }
}

void ParallelDominant::test_matching()
{
    for (NodeHandle u = G.begin(); u != G.end(); ++u)
    {
        if (mate[G.nodeId(u)] != G.null_node())
            assert(mate[G.nodeId(mate[G.nodeId(u)])] == u);
        else
            assert(mate[G.nodeId(u)] == G.null_node());
    }
}

ParallelDominant::ParallelDominant(Input edgeLst) : G(edgeLst.second, edgeLst.first), mate(G.numNodes(), G.null_node())
{}

int main(int argc, char *argv[])
{

    if(argc < 3 || argc > 3)
    {
        std::cout << "Invalid number of arguments" << std::endl;
        return 1;
    }


    int test_time = std::atoi(argv[1]);

    if(test_time < 0 || test_time > 1)
    {
        std::cout << "Invalid first argument" << std::endl;
        return 2;
    }

    if (test_time < 1)
    {
        std::string str = argv[2];
        Input edgeLst = readEdges(str);
        ParallelDominant test_1(edgeLst);

        for(size_t i = 0; i < 20; ++i)
        {
            test_1.simple_procedure(20);
            test_1.test_matching();
            test_1.clear_mate();
        }
        std::cout << "matching successfull" << std::endl;
    }

    else
    {
        std::string str = argv[2];
        Input edgeLst = readEdges(str);
        ParallelDominant test_1(edgeLst);

        ofstream myfile;
        std::size_t found = str.find_last_of("/\\");
        std::string out = "../evaluation/ParDomi" + str.substr(found+1) + ".txt";
        myfile.open(out);

        std::stringstream header;
        header << "Algorithm Runtime Quality Threads" << std::endl;
        myfile << header.str();
        cout << header.str();

        size_t max_threads = std::thread::hardware_concurrency() > 32 ? 32 : std::thread::hardware_concurrency();
        size_t rounds = 5;

        for(size_t threads = 2; threads <= max_threads; threads += 2)
        {
            for (size_t i = 0; i < rounds; ++i)
            {
                auto start = chrono::high_resolution_clock::now();
                test_1.simple_procedure(threads);
                auto finish = chrono::high_resolution_clock::now();
                chrono::duration<double> elapsed = finish - start;

                std::stringstream line;
                line << "ParDomi " << elapsed.count() << " " << test_1.matching_quality() << " " << threads << std::endl;
                myfile << line.str();
                cout << line.str();
                test_1.clear_mate();
            }
        }
    }
    return 0;
}

