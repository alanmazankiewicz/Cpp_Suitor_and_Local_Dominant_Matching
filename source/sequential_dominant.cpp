#include "implementation/sequential_dominant.hpp"

SequentialDominant::SequentialDominant(Input edgeLst) : G(edgeLst.second, edgeLst.first),
                                                        mate(G.numNodes(), G.null_node()),
                                                        mate2(G.numNodes(), G.null_node()), // Bonus
                                                        mate3(G.numNodes(), G.null_node()) // Bonus
{}

void SequentialDominant::processVertex(NodeHandle u, LocalQueue<NodeHandle> &Q, Mate &candidate, Mate &mate)
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
        mate[u_id] = candidate[u_id];
        mate[G.nodeId(candidate[u_id])] = u;
        Q.push_back(u);
        Q.push_back(candidate[u_id]);
    }
}


void SequentialDominant::procedure(Mate &mate, void (SequentialDominant::*processVertex)(NodeHandle, LocalQueue<NodeHandle> &,Mate &, Mate &))
{
    Mate candidate(G.numNodes(), G.null_node());
    LocalQueue<NodeHandle> Q(G.numNodes());

    for (NodeHandle u = G.begin(); u != G.end(); ++u)
    {
        (this->*processVertex)(u, Q, candidate, mate);
    }

    while (Q.get_size())
    {
        NodeHandle u = Q.pop_front();

        for (EdgeIterator v = G.beginEdges(u); v != G.endEdges(u); ++v)
        {
            if ((G.nodeId(mate[G.nodeId(u)]) != *v) && (G.nodeId(candidate[*v]) == G.nodeId(u)))
            {
                (this->*processVertex)(G.node(*v), Q, candidate, mate);
            }
        }
    }
}

inline void SequentialDominant::simple_procedure()
{
    procedure(mate, &SequentialDominant::processVertex);
}

void SequentialDominant::clear_mate()
{
    for (auto iter = mate.begin(); iter != mate.end(); ++iter)
    {
        *iter = G.null_node();
    }

    if (optimized) // Bonus
    {
        for (auto iter = mate2.begin(); iter != mate2.end(); ++iter)
        {
            *iter = G.null_node();
        }

        for (auto iter = mate3.begin(); iter != mate3.end(); ++iter)
        {
            *iter = G.null_node();
        }
        optimized = false;
    }
}

void SequentialDominant::print_matching() const
{
    if (!optimized)
    {
        for (auto iter = mate.begin(); iter != mate.end(); ++iter)
        {
            if (*iter != G.null_node())
            {
                std::cout << iter - mate.begin() << " " << G.nodeId(*iter) << std::endl;
            }
        }
    } else // Bonus
    {
        for (auto iter = mate3.begin(); iter != mate3.end(); ++iter)
        {
            if (*iter != G.null_node())
            {
                std::cout << iter - mate3.begin() << " " << G.nodeId(*iter) << std::endl;
            }
        }
    }
}

const double SequentialDominant::matching_quality() const
{
    double total_weight = 0;
    if (!optimized)
    {
        for (auto iter = mate.begin(); iter != mate.end(); ++iter)
        {
            if (*iter != G.null_node())
            {
                EdgeIterator edgeiter = std::find(G.beginEdges(*iter), G.endEdges(*iter), iter - mate.begin());
                total_weight += G.edgeWeight(edgeiter);
            }
        }
    } else // Bonus
    {
        for (auto iter = mate3.begin(); iter != mate3.end(); ++iter)
        {
            if (*iter != G.null_node())
            {
                EdgeIterator edgeiter = std::find(G.beginEdges(*iter), G.endEdges(*iter), iter - mate3.begin());
                total_weight += G.edgeWeight(edgeiter);
            }
        }
    }

    return total_weight / 2;
}

void SequentialDominant::test_matching()
{
    if (!optimized)
    {
        for (NodeHandle u = G.begin(); u != G.end(); ++u)
        {
            if (mate[G.nodeId(u)] != G.null_node())
                assert(mate[G.nodeId(mate[G.nodeId(u)])] == u);
            else
                assert(mate[G.nodeId(u)] == G.null_node());
        }
    } else // Bonus
    {
        for (NodeHandle u = G.begin(); u != G.end(); ++u)
        {
            if (mate[G.nodeId(u)] != G.null_node())
                assert(mate[G.nodeId(mate[G.nodeId(u)])] == u);
            else
                assert(mate[G.nodeId(u)] == G.null_node());
        }

        for (NodeHandle u = G.begin(); u != G.end(); ++u)
        {
            if (mate2[G.nodeId(u)] != G.null_node())
                assert(mate2[G.nodeId(mate2[G.nodeId(u)])] == u);
            else
                assert(mate2[G.nodeId(u)] == G.null_node());
        }

        for (NodeHandle u = G.begin(); u != G.end(); ++u)
        {
            if (mate3[G.nodeId(u)] != G.null_node())
                assert(mate3[G.nodeId(mate3[G.nodeId(u)])] == u);
            else
                assert(mate3[G.nodeId(u)] == G.null_node());
        }
    }
}

// ########## Bonus begin ################

void SequentialDominant::processVertex2(NodeHandle u, LocalQueue<NodeHandle> &Q, Mate &candidate, Mate &mate2)
{
    double max_wt = 0;
    NodeHandle max_wt_id = G.null_node();
    size_t u_id = G.nodeId(u);

    for (EdgeIterator t = G.beginEdges(u); t != G.endEdges(u); ++t)
    {
        if (mate2[*t] == G.null_node() && max_wt < G.edgeWeight(t) && mate[u_id] != G.node(*t))
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
        mate2[u_id] = candidate[u_id];
        mate2[G.nodeId(candidate[u_id])] = u;
        Q.push_back(u);
        Q.push_back(candidate[u_id]);
    }
}

void SequentialDominant::improved_procedure()
{
    optimized = true;
    procedure(mate, &SequentialDominant::processVertex);
    procedure(mate2, &SequentialDominant::processVertex2);

    std::vector<bool> proceesed(G.numNodes(), false);

    for (NodeHandle u = G.begin(); u != G.end(); ++u)
    {
        size_t nodeId = G.nodeId(u);

        // noops
        if (mate[nodeId] == G.null_node() && mate2[nodeId] == G.null_node())
            proceesed[nodeId] = true;

        // paths
        else if ((mate[nodeId] == G.null_node() || mate2[nodeId] == G.null_node()) && !proceesed[G.nodeId(u)])
        {
            std::vector<vector<std::pair<NodeHandle, NodeHandle>> *> M(G.numEdges());
            std::vector<std::pair<NodeHandle, NodeHandle>> zero_container;
            std::vector<std::pair<NodeHandle, NodeHandle>> one_container;
            M[0] = &zero_container;
            M[1] = &one_container;

            std::vector<double> W(G.numEdges());

            NodeHandle current_node = u;
            size_t current_nodeId = nodeId;
            proceesed[current_nodeId] = true;

            size_t mate_runner;
            if (mate[current_nodeId] == G.null_node())
            {
                mate_runner = 0;
                M[1]->emplace_back(std::make_pair(current_node, mate2[current_nodeId]));
                W[1] = G.edgeWeight(G.get_edge(current_node, mate2[current_nodeId]));
                current_node = mate2[current_nodeId];
                current_nodeId = G.nodeId(current_node);
                proceesed[current_nodeId] = true;
            } else
            {
                mate_runner = 1;
                M[1]->emplace_back(std::make_pair(current_node, mate[current_nodeId]));
                W[1] = G.edgeWeight(G.get_edge(current_node, mate[current_nodeId]));
                current_node = mate[current_nodeId];
                current_nodeId = G.nodeId(current_node);
                proceesed[current_nodeId] = true;
            }

            size_t counter = 2;

            while (mate[current_nodeId] != G.null_node() && mate2[current_nodeId] != G.null_node())
            {
                NodeHandle myMate;
                if (mate_runner == 1)
                {
                    myMate = mate2[current_nodeId];
                    --mate_runner;
                }
                else{
                    myMate = mate[current_nodeId];
                    ++mate_runner;
                }


                double edgeWeight = G.edgeWeight(G.get_edge(current_node, myMate));
                W[counter] = W[counter - 2] + edgeWeight;
                M[counter] = M[counter - 2];
                M[counter]->emplace_back(std::make_pair(current_node, myMate));

                current_node = myMate;
                current_nodeId = G.nodeId(current_node);
                proceesed[current_nodeId] = true;
                ++counter;
            }

            std::vector<std::pair<NodeHandle, NodeHandle>>* valid_path;

            if(W[counter -1] > W[counter -2])
                valid_path = M[counter - 1];
            else
                valid_path = M[counter -2];

            for (auto iter = valid_path->begin(); iter != valid_path->end(); ++iter)
            {
                NodeHandle a, b;
                std::tie(a, b) = *iter;
                mate3[G.nodeId(a)] = b;
            }
        }
    }
    // cycles
    for (NodeHandle u = G.begin(); u != G.end(); ++u)
    {
        if (!proceesed[G.nodeId(u)])
        {
            std::vector<vector<std::pair<NodeHandle, NodeHandle>> *> M(G.numEdges());
            std::vector<std::pair<NodeHandle, NodeHandle>> zero_container;
            std::vector<std::pair<NodeHandle, NodeHandle>> one_container;
            M[0] = &zero_container;
            M[1] = &one_container;

            std::vector<double> W(G.numEdges());

            NodeHandle current_node = u;
            size_t current_nodeId = G.nodeId(u);
            proceesed[current_nodeId] = true;

            size_t mate_runner = 0;

            M[1]->emplace_back(std::make_pair(current_node, mate2[current_nodeId]));
            W[1] = G.edgeWeight(G.get_edge(current_node, mate2[current_nodeId]));
            current_node = mate2[current_nodeId];
            current_nodeId = G.nodeId(current_node);
            proceesed[current_nodeId] = true;

            size_t counter = 2;
            while (current_node != u)
            {
                NodeHandle myMate;
                if (mate_runner == 1)
                {
                    myMate = mate2[current_nodeId];
                    --mate_runner;
                }

                else{
                    myMate = mate[current_nodeId];
                    ++mate_runner;
                }

                double edgeWeight = G.edgeWeight(G.get_edge(current_node, myMate));
                W[counter] = W[counter - 2] + edgeWeight;
                M[counter] = M[counter - 2];
                M[counter]->emplace_back(std::make_pair(current_node, myMate));

                current_node = myMate;
                current_nodeId = G.nodeId(current_node);
                proceesed[current_nodeId] = true;
                ++counter;
            }

            std::vector<std::pair<NodeHandle, NodeHandle>>* valid_path;

            if(W[counter -1] > W[counter -2])
                valid_path = M[counter - 1];
            else
                valid_path = M[counter -2];

            for (auto iter = valid_path->begin(); iter != valid_path->end(); ++iter)
            {
                NodeHandle a, b;
                std::tie(a, b) = *iter;
                mate3[G.nodeId(a)] = b;
            }
        }
    }
    // gegenkanten einfügen
    for (auto iter = mate3.begin(); iter != mate3.end(); ++iter)
    {
        if (*iter != G.null_node())
            mate3[G.nodeId(*iter)] = G.node(iter - mate3.begin());
    }

    procedure(mate3, &SequentialDominant::processVertex);
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
        SequentialDominant test_1(edgeLst);
        test_1.simple_procedure();
        test_1.test_matching();

        if(test_time == -1)
        {
            test_1.clear_mate();
            test_1.improved_procedure();
            test_1.test_matching();
        }

        std::cout << "matching successfull" << std::endl;

    } else
    {
        std::string str = argv[2];
        Input edgeLst = readEdges(str);
        SequentialDominant test_1(edgeLst);

        ofstream myfile;
        std::size_t found = str.find_last_of("/\\");
        std::string out = "../evaluation/SeqDomi_" + str.substr(found+1) + ".txt";
        myfile.open(out);

        size_t rounds = 5;

        std::stringstream header;
        header << "Algorithm Runtime Quality" << std::endl;
        myfile << header.str();
        cout << header.str();

        for (size_t i = 0; i < rounds; ++i)
        {
            auto start = chrono::high_resolution_clock::now();
            test_1.simple_procedure();
            auto finish = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsed = finish - start;

            std::stringstream line;
            line << "SeqDomiSimp " << elapsed.count() << " " << test_1.matching_quality() << std::endl;
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
                test_1.improved_procedure();
                auto finish = chrono::high_resolution_clock::now();
                chrono::duration<double> elapsed = finish - start;

                std::stringstream line;
                line << "SeqDomiImp " << elapsed.count() << " " << test_1.matching_quality() << std::endl;
                myfile << line.str();
                cout << line.str();
                test_1.clear_mate();
            }
        }
        myfile.close();
    }
    return 0;
}

