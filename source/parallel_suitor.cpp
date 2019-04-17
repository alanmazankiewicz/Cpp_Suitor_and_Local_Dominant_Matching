#include "implementation/parallel_suitor.hpp"

ParallelSuitor::ParallelSuitor(Input edgeLst) : G(edgeLst.second, edgeLst.first),
                                                ws(std::make_unique<std::atomic<double>[]>(G.numNodes())),
                                                ws2(std::make_unique<std::atomic<double>[]>(G.numNodes())),
                                                ws3(std::make_unique<std::atomic<double>[]>(G.numNodes())),
                                                suitor(std::make_unique<std::atomic<NodeHandle>[]>(G.numNodes())),
                                                suitor2(std::make_unique<std::atomic<NodeHandle>[]>(G.numNodes())),
                                                suitor3(std::make_unique<std::atomic<NodeHandle>[]>(G.numNodes())),
                                                mtx(std::make_unique<mutex[]>(G.numNodes())),
                                                proceesed(std::make_unique<std::atomic<bool>[]>(G.numNodes())),
                                                unionFind(G.numNodes(), *this),
                                                suitor4(G.numNodes(), G.null_node()),
                                                ws4(G.numNodes(), 0)
{
    for (auto iter = suitor.get(); iter != suitor.get() + G.numNodes(); ++iter)
    {
        iter->store(G.null_node());
    }

    for (auto iter = suitor2.get(); iter != suitor2.get() + G.numNodes(); ++iter)
    {
        iter->store(G.null_node());
    }

    for (auto iter = suitor3.get(); iter != suitor3.get() + G.numNodes(); ++iter)
    {
        iter->store(G.null_node());
    }
}

inline void ParallelSuitor::simple_procedure(size_t num_threads)
{
    procedure(num_threads, &ParallelSuitor::process, G.begin(), G.end(), G.numNodes());
}

void ParallelSuitor::process(NodeHandle start, NodeHandle finish)
{
    for (NodeHandle u = start; u != finish; ++u)
    {
        NodeHandle current = u;
        bool done = false;

        while (!done)
        {
            NodeHandle partner = G.null_node();
            double heaviest = 0;

            for (EdgeIterator v = G.beginEdges(current); v != G.endEdges(current); ++v)
            {
                double weight_v = G.edgeWeight(v);
                if (weight_v > heaviest && weight_v > ws[*v])
                {
                    partner = G.node(*v);
                    heaviest = weight_v;
                }
            }
            done = true;
            if (heaviest > 0)
            {
                std::unique_lock<std::mutex> lock(mtx[G.nodeId(partner)]);

                if (ws[G.nodeId(partner)] < heaviest)
                {
                    NodeHandle y = suitor[G.nodeId(partner)];
                    suitor[G.nodeId(partner)] = current;
                    ws[G.nodeId(partner)] = heaviest;
                    lock.unlock();

                    if (y != G.null_node())
                    {
                        current = y;
                        done = false;
                    }
                } else
                    done = false;
            }
        }
    }
}

void ParallelSuitor::procedure(size_t num_threads, void (ParallelSuitor::*process)(NodeHandle, NodeHandle),
                               vector<size_t>::const_iterator beginNode, vector<size_t>::const_iterator endNode,
                               size_t numNodes)
{
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    size_t nodes_per_thread = numNodes / num_threads;

    if (!nodes_per_thread)
    {
        for (size_t thread = 0; thread < numNodes; ++thread)
        {
            threads.emplace_back(std::thread(process, this, beginNode + thread, beginNode + thread + 1));
        }

    } else
    {
        for (size_t thread = 0; thread < num_threads; ++thread)
        {
            if (thread != num_threads - 1)
            {
                threads.emplace_back(std::thread(process, this, beginNode + thread * nodes_per_thread,
                                                 beginNode + (thread + 1) * nodes_per_thread));
            } else
            {
                threads.emplace_back(std::thread(process, this, beginNode + thread * nodes_per_thread, endNode));
            }
        }
    }

    for (auto iter = threads.begin(); iter != threads.end(); ++iter)
    {
        iter->join();
    }
}

void ParallelSuitor::print_matching() const
{
    if (!optimized)
    {
        for (auto iter = suitor.get(); iter != suitor.get() + G.numNodes(); ++iter)
        {
            if (iter->load() != G.null_node())
            {
                std::cout << iter - suitor.get() << " " << G.nodeId(iter->load()) << std::endl;
            }
        }
    } else
    {
        for (auto iter = suitor3.get(); iter != suitor3.get() + G.numNodes(); ++iter)
        {
            if (iter->load() != G.null_node())
            {
                std::cout << iter - suitor3.get() << " " << G.nodeId(iter->load()) << std::endl;
            }
        }
    }
}

const double ParallelSuitor::matching_quality() const
{
    if(!optimized)
        return std::accumulate(ws.get(), ws.get() + G.numNodes(), (double) 0) / 2;
    else
        return std::accumulate(ws3.get(), ws3.get() + G.numNodes(), (double) 0) / 2;
}

void ParallelSuitor::clear_mate()
{
    for (auto iter = suitor.get(); iter != suitor.get() + G.numNodes(); ++iter)
    {
        iter->store(G.null_node());
    }

    for (auto iter = ws.get(); iter != ws.get() + G.numNodes(); ++iter)
    {
        iter->store(0);
    }

    if (optimized)
    {
        for (auto iter = suitor2.get(); iter != suitor2.get() + G.numNodes(); ++iter)
        {
            iter->store(G.null_node());
        }

        for (auto iter = ws2.get(); iter != ws2.get() + G.numNodes(); ++iter)
        {
            iter->store(0);
        }

        for (auto iter = suitor3.get(); iter != suitor3.get() + G.numNodes(); ++iter)
        {
            iter->store(G.null_node());
        }

        for (auto iter = ws3.get(); iter != ws3.get() + G.numNodes(); ++iter)
        {
            iter->store(0);
        }

        for (auto iter = proceesed.get(); iter != proceesed.get() + G.numNodes(); ++iter)
        {
            *iter = false;
        }
        optimized = false;
    }
}

void ParallelSuitor::test_matching()
{
    if (!optimized)
    {
        for (NodeHandle u = G.begin(); u != G.end(); ++u)
        {
            if (suitor[G.nodeId(u)].load() != G.null_node())
                assert(suitor[G.nodeId(suitor[G.nodeId(u)])].load() == u);
            else
                assert(suitor[G.nodeId(u)].load() == G.null_node());
        }
    } else
    {
        for (NodeHandle u = G.begin(); u != G.end(); ++u)
        {
            if (suitor[G.nodeId(u)].load() != G.null_node())
                assert(suitor[G.nodeId(suitor[G.nodeId(u)])].load() == u);
            else
                assert(suitor[G.nodeId(u)].load() == G.null_node());
        }

        for (NodeHandle u = G.begin(); u != G.end(); ++u)
        {
            if (suitor2[G.nodeId(u)].load() != G.null_node())
                assert(suitor2[G.nodeId(suitor2[G.nodeId(u)])].load() == u);
            else
                assert(suitor2[G.nodeId(u)].load() == G.null_node());
        }

        for (NodeHandle u = G.begin(); u != G.end(); ++u)
        {
            if (suitor3[G.nodeId(u)].load() != G.null_node())
                assert(suitor3[G.nodeId(suitor3[G.nodeId(u)])].load() == u);
            else
                assert(suitor3[G.nodeId(u)].load() == G.null_node());
        }
    }
}


// ###### Bonus #########
void ParallelSuitor::process2(NodeHandle start, NodeHandle finish)
{
    for (NodeHandle u = start; u != finish; ++u)
    {
        NodeHandle current = u;
        bool done = false;

        while (!done)
        {
            NodeHandle partner = G.null_node();
            double heaviest = 0;

            for (EdgeIterator v = G.beginEdges(current); v != G.endEdges(current); ++v)
            {
                double weight_v = G.edgeWeight(v);
                if (weight_v > ws2[*v] && weight_v > heaviest && suitor[*v].load() != current)
                {
                    partner = G.node(*v);
                    heaviest = weight_v;
                }
            }
            done = true;
            if (heaviest > 0)
            {
                std::lock_guard<std::mutex> lock(mtx[G.nodeId(partner)]);

                if (ws2[G.nodeId(partner)] < heaviest)
                {
                    NodeHandle y = suitor2[G.nodeId(partner)]; // TODO mit swap auf current
                    suitor2[G.nodeId(partner)] = current;
                    ws2[G.nodeId(partner)] = heaviest;

                    if (y != G.null_node())
                    {
                        current = y;
                        done = false;
                    }
                } else
                    done = false;
            }
        }
    }
}

void ParallelSuitor::process3()
{
    for (NodeHandle u = G.begin(); u != G.end(); ++u)
    {
        NodeHandle current = u;
        bool done = false;

        while (!done)
        {
            NodeHandle partner = G.null_node();
            double heaviest = 0;

            for (EdgeIterator v = G.beginEdges(current); v != G.endEdges(current); ++v)
            {
                double weight_v = G.edgeWeight(v);
                if (weight_v > heaviest && weight_v > ws4[*v] && suitor3[*v].load() == G.null_node() && suitor3[G.nodeId(current)].load() == G.null_node())
                {
                    partner = G.node(*v);
                    heaviest = weight_v;
                }
            }
            done = true;
            if (heaviest > 0)
            {
                std::swap(suitor4[G.nodeId(partner)], current);
                ws4[G.nodeId(partner)] = heaviest;

                if (current != G.null_node())
                {
                    done = false;
                }
            }
        }
    }
}

void ParallelSuitor::path_discovery(NodeHandle start, NodeHandle finish)
{
    for (NodeHandle u = start; u != finish; ++u)
    {
        size_t nodeId = G.nodeId(u);

        // noops
        if (suitor[nodeId].load() == G.null_node() && suitor2[nodeId].load() == G.null_node())
            proceesed[nodeId] = true;

        // paths
        else if ((suitor[nodeId].load() == G.null_node() || suitor2[nodeId].load() == G.null_node()) &&
                 !proceesed[nodeId])
        {
            // erstmal den path lang laufen bis zum ende um zu prüfen ob die aktuelle nodeID die höhere ist
            NodeHandle current_node = u;
            size_t current_nodeId = nodeId;
            size_t mate_runner;
            if (suitor[current_nodeId].load() == G.null_node())
            {
                mate_runner = 0;
                current_node = suitor2[current_nodeId];
                current_nodeId = G.nodeId(current_node);
            } else
            {
                mate_runner = 1;
                current_node = suitor[current_nodeId];
                current_nodeId = G.nodeId(current_node);
            }

            while (suitor[current_nodeId].load() != G.null_node() && suitor2[current_nodeId].load() != G.null_node())
            {
                if (mate_runner == 1)
                {
                    current_node = suitor2[current_nodeId];
                    --mate_runner;
                } else
                {
                    current_node = suitor[current_nodeId];
                    ++mate_runner;
                }
                current_nodeId = G.nodeId(current_node);
            }


            if (nodeId < current_nodeId)
                continue;

            // um sich gleich bei der dynmiaschen programmierung das kopieren der vektoren zu sparen
            std::vector<vector<std::tuple<NodeHandle, NodeHandle, double>> *> M(G.numEdges());
            std::vector<std::tuple<NodeHandle, NodeHandle, double>> zero_container;
            std::vector<std::tuple<NodeHandle, NodeHandle, double>> one_container;
            M[0] = &zero_container;
            M[1] = &one_container;

            std::vector<double> W(G.numEdges());

            current_node = u;
            current_nodeId = nodeId;
            proceesed[current_nodeId] = true;


            if (suitor[current_nodeId].load() == G.null_node())
            {
                mate_runner = 0;
                M[1]->emplace_back(std::make_tuple(current_node, suitor2[current_nodeId].load(), ws2[current_nodeId].load()));
                W[1] = ws2[current_nodeId];
                current_node = suitor2[current_nodeId];
                current_nodeId = G.nodeId(current_node);
                proceesed[current_nodeId] = true;
            } else
            {
                mate_runner = 1;
                M[1]->emplace_back(std::make_tuple(current_node, suitor[current_nodeId].load(), ws[current_nodeId].load()));
                W[1] = ws[current_nodeId];
                current_node = suitor[current_nodeId];
                current_nodeId = G.nodeId(current_node);
                proceesed[current_nodeId] = true;
            }

            size_t counter = 2;

            while (suitor[current_nodeId].load() != G.null_node() && suitor2[current_nodeId].load() != G.null_node())
            {
                NodeHandle myMate;
                double edgeWeight;
                if (mate_runner == 1)
                {
                    myMate = suitor2[current_nodeId];
                    edgeWeight = ws2[current_nodeId];
                    --mate_runner;
                } else
                {
                    myMate = suitor[current_nodeId];
                    edgeWeight = ws[current_nodeId];
                    ++mate_runner;
                }

                W[counter] = W[counter - 2] + edgeWeight;
                M[counter] = M[counter - 2];
                M[counter]->emplace_back(std::make_tuple(current_node, myMate, edgeWeight));

                current_node = myMate;
                current_nodeId = G.nodeId(current_node);
                proceesed[current_nodeId] = true;
                ++counter;
            }

            std::vector<std::tuple<NodeHandle, NodeHandle, double>>* valid_path;

            if(W[counter -1] > W[counter -2])
                valid_path = M[counter - 1];
            else
                valid_path = M[counter -2];

            for (auto iter = valid_path->begin(); iter != valid_path->end(); ++iter)
            {
                NodeHandle a, b;
                double weight;
                std::tie(a, b, weight) = *iter;
                suitor3[G.nodeId(a)] = b;
                ws3[G.nodeId(a)] = weight;
            }
        }
    }
}

void ParallelSuitor::find_cycle(NodeHandle start, NodeHandle finish)
{
    for (auto iter = G.begin(); iter != G.end(); ++iter)
    {
        if (!proceesed[G.nodeId(iter)])
        {
            // unify mit m1 partner
            while (true)
            {
                NodeHandle rep_iter = unionFind.find_representative(iter);
                NodeHandle rep_suitor = unionFind.find_representative(suitor[G.nodeId(iter)]);

                if (rep_iter == rep_suitor) break;

                lock(unionFind.rec_mtx[G.nodeId(rep_iter)], unionFind.rec_mtx[G.nodeId(rep_suitor)]);
                lock_guard<recursive_mutex> fst_lock(unionFind.rec_mtx[G.nodeId(rep_iter)], adopt_lock);
                lock_guard<recursive_mutex> snd_lock(unionFind.rec_mtx[G.nodeId(rep_suitor)], adopt_lock);

                NodeHandle rep_iter_true = unionFind.find_representative(iter);
                NodeHandle rep_suitor_true = unionFind.find_representative(suitor[G.nodeId(iter)]);

                if (rep_iter_true == rep_suitor_true) break;

                if ((rep_iter != rep_iter_true) || (rep_suitor != rep_suitor_true)) continue;

                unionFind.unify(rep_iter, rep_suitor);
                break;
            }

            // unify mit m2 partner
            while (true)
            {
                size_t nodeId = G.nodeId(iter);

                NodeHandle rep_iter = unionFind.find_representative(iter);
                NodeHandle rep_suitor = unionFind.find_representative(suitor2[G.nodeId(iter)]);

                if (rep_iter == rep_suitor) break;

                lock(unionFind.rec_mtx[G.nodeId(rep_iter)], unionFind.rec_mtx[G.nodeId(rep_suitor)]);
                lock_guard<recursive_mutex> fst_lock(unionFind.rec_mtx[G.nodeId(rep_iter)], adopt_lock);
                lock_guard<recursive_mutex> snd_lock(unionFind.rec_mtx[G.nodeId(rep_suitor)], adopt_lock);

                NodeHandle rep_iter_true = unionFind.find_representative(iter);
                NodeHandle rep_suitor_true = unionFind.find_representative(suitor2[G.nodeId(iter)]);

                if (rep_iter_true == rep_suitor_true) break;

                if ((rep_iter != rep_iter_true) || (rep_suitor != rep_suitor_true))
                    continue;

                unionFind.unify(rep_iter, rep_suitor);
                break;
            }
        }
    }
}

void ParallelSuitor::work_cycle(std::vector<size_t>::const_iterator start, std::vector<size_t>::const_iterator finish)
{
    for (auto iter = start; iter != finish; ++iter)
    {
        NodeHandle u = G.node(*iter);

        std::vector<vector<std::tuple<NodeHandle, NodeHandle, double>> *> M(G.numEdges());
        std::vector<std::tuple<NodeHandle, NodeHandle, double>> zero_container;
        std::vector<std::tuple<NodeHandle, NodeHandle, double>> one_container;
        M[0] = &zero_container;
        M[1] = &one_container;

        std::vector<double> W(G.numEdges());

        NodeHandle current_node = u;
        size_t current_nodeId = G.nodeId(u);
        proceesed[current_nodeId] = true;

        size_t mate_runner = 0;

        M[1]->emplace_back(std::make_tuple(current_node, suitor2[current_nodeId].load(), ws2[current_nodeId].load()));
        W[1] = ws2[current_nodeId];
        current_node = suitor2[current_nodeId];
        current_nodeId = G.nodeId(current_node);
        proceesed[current_nodeId] = true;

        size_t counter = 2;
        while (current_node != u)
        {
            NodeHandle myMate;
            double edgeWeight;
            if (mate_runner == 1)
            {
                myMate = suitor2[current_nodeId];
                edgeWeight = ws2[current_nodeId];
                --mate_runner;
            } else
            {
                myMate = suitor[current_nodeId];
                edgeWeight = ws[current_nodeId];
                ++mate_runner;
            }

            W[counter] = W[counter - 2] + edgeWeight;
            M[counter] = M[counter - 2];
            M[counter]->emplace_back(std::make_tuple(current_node, myMate, edgeWeight));

            current_node = myMate;
            current_nodeId = G.nodeId(current_node);
            proceesed[current_nodeId] = true;
            ++counter;
        }

        std::vector<std::tuple<NodeHandle, NodeHandle, double>>* valid_path;

        if(W[counter -1] > W[counter -2])
            valid_path = M[counter - 1];
        else
            valid_path = M[counter -2];

        for (auto iter = valid_path->begin(); iter != valid_path->end(); ++iter)
        {
            NodeHandle a, b;
            double weight;
            std::tie(a, b, weight) = *iter;
            suitor3[G.nodeId(a)] = b;
            ws3[G.nodeId(a)] = weight;
        }
    }
}


void ParallelSuitor::improved_procedure(size_t num_threads)
{
    optimized = true;
    procedure(num_threads, &ParallelSuitor::process, G.begin(), G.end(), G.numNodes());
    procedure(num_threads, &ParallelSuitor::process2, G.begin(), G.end(), G.numNodes());

    procedure(num_threads, &ParallelSuitor::path_discovery, G.begin(), G.end(), G.numNodes());
    procedure(num_threads, &ParallelSuitor::find_cycle, G.begin(), G.end(), G.numNodes());

    std::vector<size_t> unique_representatives;
    unique_representatives.reserve(G.numNodes());

    for (long i = 0; i < G.numNodes(); ++i)
    {
        NodeHandle rep = unionFind.union_find[i].load();
        if (rep == G.null_node() && !proceesed[i])
            unique_representatives.emplace_back(i);
    }

    procedure(num_threads, &ParallelSuitor::work_cycle, unique_representatives.begin(), unique_representatives.end(),
    unique_representatives.size());

    // gegenkanten einfügen
    for (auto iter = suitor3.get(); iter != suitor3.get() + G.numNodes(); ++iter)
    {
        if (iter->load() != G.null_node())
        {
            suitor3[G.nodeId(*iter)] = G.node(iter - suitor3.get());
            ws3[G.nodeId(*iter)] = ws3[iter - suitor3.get()].load();
        }
    }

    process3();

    std::vector<bool> processed(G.numNodes());
    for(auto iter = suitor4.begin(); iter != suitor4.end(); ++iter)
    {
        if(*iter != G.null_node() && !processed[iter - suitor4.begin()])
        {
            suitor3[iter - suitor4.begin()] = *iter;
            processed[iter - suitor4.begin()] = true;
        }
    }
}


ParallelSuitor::UnionFind::UnionFind(size_t _num_nodes, ParallelSuitor &owner_) : owner(&owner_), num_nodes(_num_nodes),
union_find(std::make_unique<std::atomic<NodeHandle>[]>(num_nodes)), ranks(_num_nodes, 1),
rec_mtx(std::make_unique<std::recursive_mutex[]>(owner->G.numNodes()))
{
    for (auto iter = union_find.get(); iter != union_find.get() + _num_nodes; ++iter)
    {
        iter->store(owner->G.null_node());
    }
}


ParallelSuitor::NodeHandle ParallelSuitor::UnionFind::parentOf(NodeHandle n) const
{
    return union_find[owner->G.nodeId(n)];
}

void ParallelSuitor::UnionFind::unify(NodeHandle root1, NodeHandle root2)
{

    if (root1 == root2)
    {
        return;
    }

    else
    {
        if (ranks[owner->G.nodeId(root1)] < ranks[owner->G.nodeId(root2)])
        {
            union_find[owner->G.nodeId(root1)] = root2;
            if (ranks[owner->G.nodeId(root1)] == ranks[owner->G.nodeId(root2)])
                ++ranks[owner->G.nodeId(root2)];

        } else
        {
            union_find[owner->G.nodeId(root2)] = root1;
            if (ranks[owner->G.nodeId(root1)] == ranks[owner->G.nodeId(root2)])
                ++ranks[owner->G.nodeId(root1)];
        }
        --num_nodes;
    }
}

ParallelSuitor::NodeHandle ParallelSuitor::UnionFind::find_representative(NodeHandle a)
{
    // find
    NodeHandle root = a;


    while (union_find[owner->G.nodeId(root)].load() != owner->G.null_node())
    {
        root = union_find[owner->G.nodeId(root)];
    }

    while (a != root)
    {
        NodeHandle n_1 = union_find[owner->G.nodeId(a)];
        union_find[owner->G.nodeId(a)] = root;
        a = n_1;
    }
    return root;
}


int main(int argc, char *argv[])
{
    if(argc < 3 || argc > 3)
    {
        std::cout << "Invalid number of arguments" << std::endl;
        return 1;
    }


    int test_time = std::atoi(argv[1]);

    if(test_time < -1 || test_time > 2)
    {
        std::cout << "Invalid first argument" << std::endl;
        return 2;
    }

    if (test_time < 1)
    {
        std::string str = argv[2];
        Input edgeLst = readEdges(str);
        ParallelSuitor test_1(edgeLst);

        for (size_t i = 0; i < 100; ++i)
        {
            test_1.simple_procedure(20);
            test_1.test_matching();
            test_1.clear_mate();
        }

        if(test_time == -1)
        {
            test_1.clear_mate();
            test_1.improved_procedure(20);
            test_1.test_matching();
        }

        std::cout << "matching successfull" << std::endl;

    } else
    {
        std::string str = argv[2];
        Input edgeLst = readEdges(str);
        ParallelSuitor test_1(edgeLst);

        ofstream myfile;
        std::size_t found = str.find_last_of("/\\");
        std::string out = "../evaluation/ParSuitor" + str.substr(found+1) + ".txt";
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
                line << "ParSuitorSimp " << elapsed.count() << " " << test_1.matching_quality() << " " << threads << std::endl;
                myfile << line.str();
                cout << line.str();
                test_1.clear_mate();
            }


            if (test_time == 2)
            {
                size_t rounds = 5;
                for (size_t i = 0; i < rounds; ++i)
                {
                    auto start = chrono::high_resolution_clock::now();
                    test_1.improved_procedure(threads);
                    auto finish = chrono::high_resolution_clock::now();
                    chrono::duration<double> elapsed = finish - start;

                    std::stringstream line;
                    line << "ParSuitorImp " << elapsed.count() << " " << test_1.matching_quality() << " " << threads << std::endl;
                    myfile << line.str();
                    cout << line.str();
                    test_1.clear_mate();
                }
            }
        }
        myfile.close();
    }
    return 0;
}
