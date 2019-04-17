#include "implementation/parallel_free_suitor.hpp"

ParallelLocklessSuitor::ParallelLocklessSuitor(Input edgeLst) : G(edgeLst.second, edgeLst.first),
                                                                suitor(std::make_unique<std::atomic<EdgeIterator>[]>(G.numNodes()))
{
    for (auto iter = suitor.get(); iter != suitor.get() + G.numNodes(); ++iter)
    {
        iter->store(G.null_edge());
    }
}

void ParallelLocklessSuitor::process(NodeHandle start, NodeHandle finish)
{
    for (NodeHandle u = start; u != finish; ++u)
    {
        NodeHandle current = u;
        bool done = false;

        while (!done)
        {
            EdgeIterator partner = G.null_edge();
            double heaviest = 0;

            for (EdgeIterator v = G.beginEdges(current); v != G.endEdges(current); ++v)
            {
                double weight_v = v->second; // to only load the array once
                if (weight_v > heaviest && weight_v > suitor[v->first].load()->second)
                {
                    partner = v;
                    heaviest = weight_v;
                }
            }
            done = true;
            if (heaviest > 0)
            {
                size_t partnerId = partner->first; // to only load the array once
                EdgeIterator current_suitor = suitor[partnerId];
                if (current_suitor->second < heaviest)
                {
                    if (suitor[partnerId].compare_exchange_strong(current_suitor, partner)) // here they swap (partner instead of counteredge to partner)
                    {
                        NodeHandle y = G.edgeHead(current_suitor); // therefore here 0 instead of 1 (edgehead -> instead of *current_suitor) -> only relevant at this point here
                        if (y != G.null_node())
                        {
                            current = y;
                            done = false;
                        }
                    } else
                        done = false;
                } else
                    done = false;
            }
        }
    }
}

void ParallelLocklessSuitor::print_matching() const
{
    for (auto iter = suitor.get(); iter != suitor.get() + G.numNodes(); ++iter)
    {
        EdgeIterator output = *iter;
        if (output != G.null_edge())
        {
            std::cout << iter - suitor.get() << " " << G.nodeId(G.edgeHead(output)) << std::endl;
        }
    }
}

const double ParallelLocklessSuitor::matching_quality() const
{
    double counter = 0;
    for (auto iter = suitor.get(); iter != suitor.get() + G.numNodes(); ++iter)
    {
        counter += iter->load()->second;
    }

    return counter / 2;
}

void ParallelLocklessSuitor::simple_procedure(size_t num_threads)
{
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    size_t nodes_per_thread = G.numNodes() / num_threads;

    if(!nodes_per_thread)
    {
        for (size_t thread = 0; thread < G.numNodes(); ++thread)
        {
            threads.emplace_back(
                    std::thread(&ParallelLocklessSuitor::process, this, G.begin() + thread, G.begin() + (thread + 1)));
        }
    } else
    {
        for (size_t thread = 0; thread < num_threads; ++thread)
        {
            if (thread != num_threads - 1)
            {
                threads.emplace_back(
                        std::thread(&ParallelLocklessSuitor::process, this, G.begin() + thread * nodes_per_thread,
                                    G.begin() + (thread + 1) * nodes_per_thread));
            } else
            {
                threads.emplace_back(
                        std::thread(&ParallelLocklessSuitor::process, this, G.begin() + thread * nodes_per_thread,
                                    G.end()));
            }
        }
    }

    for (auto iter = threads.begin(); iter != threads.end(); ++iter)
    {
        iter->join();
    }
}

void ParallelLocklessSuitor::clear_mate()
{
    for (auto iter = suitor.get(); iter != suitor.get() + G.numNodes(); ++iter)
    {
        iter->store(G.null_edge());
    }
}

void ParallelLocklessSuitor::test_matching()
{
    for (NodeHandle u = G.begin(); u != G.end(); ++u)
    {
        if (suitor[G.nodeId(u)].load() != G.null_edge())
        {
            EdgeIterator suit1 = suitor[G.nodeId(u)];
            EdgeIterator suit2 = suitor[suit1->first];
            assert(G.node(suit2->first) == u);
        }
        else
            assert(suitor[G.nodeId(u)].load() == G.null_edge());
    }
}

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
        ParallelLocklessSuitor test_1(edgeLst);
        for (size_t i = 0; i < 20; ++i)
        {
            test_1.simple_procedure(20);
            test_1.test_matching();
            test_1.clear_mate();
        }
        std::cout << "matching successfull" << std::endl;

    } else
    {
        std::string str = argv[2];
        Input edgeLst = readEdges(str);
        ParallelLocklessSuitor test_1(edgeLst);

        ofstream myfile;
        std::size_t found = str.find_last_of("/\\");
        std::string out = "../evaluation/ParFreeSuitor" + str.substr(found + 1) + ".txt";
        myfile.open(out);

        std::stringstream header;
        header << "Algorithm Runtime Quality Threads" << std::endl;
        myfile << header.str();
        cout << header.str();

        size_t max_threads = std::thread::hardware_concurrency() > 32 ? 32 : std::thread::hardware_concurrency();
        size_t rounds = 5;

        for (size_t threads = 2; threads <= max_threads; threads += 2)
        {
            for (size_t i = 0; i < rounds; ++i)
            {
                auto start = chrono::high_resolution_clock::now();
                test_1.simple_procedure(threads);
                auto finish = chrono::high_resolution_clock::now();
                chrono::duration<double> elapsed = finish - start;

                std::stringstream line;
                line << "ParFreeSuitorSimp " << elapsed.count() << " " << test_1.matching_quality() << " " << threads
                     << std::endl;
                myfile << line.str();
                cout << line.str();
                test_1.clear_mate();
            }
        }
    }
    return 0;
}
