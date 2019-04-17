#include "implementation/sequential_suitor.hpp"

SequentialSuitor::SequentialSuitor(Input edgeLst) : G(edgeLst.second, edgeLst.first),
                                                    suitor(G.numNodes(), G.null_node()),
                                                    ws(G.numNodes(), 0), suitor2(G.numNodes(), G.null_node()),
                                                    suitor3(G.numNodes(), G.null_node()), ws2(G.numNodes(), 0),
                                                    ws3(G.numNodes(), 0), suitor4(G.numNodes(), G.null_node()),
                                                    ws4(G.numNodes(), 0)
{}

inline void SequentialSuitor::simple_procedure()
{
    procedure();
}

void SequentialSuitor::procedure()
{
    for (NodeHandle u = G.begin(); u != G.end(); ++u)
    {
        NodeHandle current = u;
        bool done = false;

        while (!done)
        {
            NodeHandle partner = suitor[G.nodeId(current)];
            double heaviest = ws[G.nodeId(current)];

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
                std::swap(suitor[G.nodeId(partner)], current);
                ws[G.nodeId(partner)] = heaviest;

                if (current != G.null_node())
                {
                    done = false;
                }
            }
        }
    }
}

// ### Utils Stuff #####

void SequentialSuitor::print_matching() const
{
    if (!optimized)
    {
        for (auto iter = suitor.begin(); iter != suitor.end(); ++iter)
        {
            if (*iter != G.null_node())
            {
                std::cout << iter - suitor.begin() << " " << G.nodeId(*iter) << std::endl;
            }
        }
    } else
    {
        for (auto iter = suitor3.begin(); iter != suitor3.end(); ++iter)
        {
            if (*iter != G.null_node())
            {
                std::cout << iter - suitor3.begin() << " " << G.nodeId(*iter) << std::endl;
            }
        }
    }

}

const double SequentialSuitor::matching_quality() const
{
    double total_weight;
    if (!optimized)
        total_weight = std::accumulate(ws.begin(), ws.end(), (double) 0);
    else
    {
        total_weight = std::accumulate(ws3.begin(), ws3.end(), (double) 0);
    }
    return total_weight / 2;
}

void SequentialSuitor::clear_mate()
{
    for (auto iter = suitor.begin(); iter != suitor.end(); ++iter)
    {
        *iter = G.null_node();
    }

    for (auto iter = ws.begin(); iter != ws.end(); ++iter)
    {
        *iter = 0;
    }

    if (optimized)
    {
        for (auto iter = suitor2.begin(); iter != suitor2.end(); ++iter)
        {
            *iter = G.null_node();
        }

        for (auto iter = ws2.begin(); iter != ws2.end(); ++iter)
        {
            *iter = 0;
        }

        for (auto iter = suitor3.begin(); iter != suitor3.end(); ++iter)
        {
            *iter = G.null_node();
        }

        for (auto iter = ws3.begin(); iter != ws3.end(); ++iter)
        {
            *iter = 0;
        }
        optimized = false;
    }
}

void SequentialSuitor::test_matching()
{
    if (!optimized)
    {
        for (NodeHandle u = G.begin(); u != G.end(); ++u)
        {
            if (suitor[G.nodeId(u)] != G.null_node())
                assert(suitor[G.nodeId(suitor[G.nodeId(u)])] == u);
            else
                assert(suitor[G.nodeId(u)] == G.null_node());
        }
    } else
    {
        for (NodeHandle u = G.begin(); u != G.end(); ++u)
        {
            if (suitor[G.nodeId(u)] != G.null_node())
                assert(suitor[G.nodeId(suitor[G.nodeId(u)])] == u);
            else
                assert(suitor[G.nodeId(u)] == G.null_node());
        }

        for (NodeHandle u = G.begin(); u != G.end(); ++u)
        {
            if (suitor2[G.nodeId(u)] != G.null_node())
                assert(suitor2[G.nodeId(suitor2[G.nodeId(u)])] == u);
            else
                assert(suitor2[G.nodeId(u)] == G.null_node());
        }

        for (NodeHandle u = G.begin(); u != G.end(); ++u)
        {
            if (suitor3[G.nodeId(u)] != G.null_node())
                assert(suitor3[G.nodeId(suitor3[G.nodeId(u)])] == u);
            else
                assert(suitor3[G.nodeId(u)] == G.null_node());
        }

        for (NodeHandle u = G.begin(); u != G.end(); ++u)
        {
            if (suitor4[G.nodeId(u)] != G.null_node())
                assert(suitor4[G.nodeId(suitor4[G.nodeId(u)])] == u);
            else
                assert(suitor4[G.nodeId(u)] == G.null_node());
        }
    }
}

// ######## Heavy Matching stuff ############

void SequentialSuitor::procedure2()
{
    {
        for (NodeHandle u = G.begin(); u != G.end(); ++u)
        {
            NodeHandle current = u;
            bool done = false;

            while (!done)
            {
                NodeHandle partner = suitor2[G.nodeId(current)];
                double heaviest = ws2[G.nodeId(current)];

                for (EdgeIterator v = G.beginEdges(current); v != G.endEdges(current); ++v)
                {
                    double weight_v = G.edgeWeight(v);
                    if (weight_v > heaviest && weight_v > ws2[*v] && suitor[*v] != current)
                    {
                        partner = G.node(*v);
                        heaviest = weight_v;
                    }
                }
                done = true;
                if (heaviest > 0)
                {
                    std::swap(suitor2[G.nodeId(partner)], current);
                    ws2[G.nodeId(partner)] = heaviest;

                    if (current != G.null_node())
                    {
                        done = false;
                    }
                }
            }
        }
    }
}

void SequentialSuitor::procedure3()
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
                if (weight_v > heaviest && weight_v > ws4[*v] && suitor3[*v] == G.null_node() && suitor3[G.nodeId(current)] == G.null_node()) 
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

void SequentialSuitor::improved_procedure()
{
    optimized = true;
    procedure();
    procedure2();

    std::vector<bool> proceesed(G.numNodes(), false);

    for (NodeHandle u = G.begin(); u != G.end(); ++u)
    {
        size_t nodeId = G.nodeId(u);

        // noops
        if (suitor[nodeId] == G.null_node() && suitor2[nodeId] == G.null_node())
            proceesed[nodeId] = true;

            // paths
        else if ((suitor[nodeId] == G.null_node() || suitor2[nodeId] == G.null_node()) && !proceesed[G.nodeId(u)])
        {
            // um sich gleich bei der dynmiaschen programmierung das kopieren der vektoren zu sparen
            std::vector<vector<std::tuple<NodeHandle, NodeHandle, double>> *> M(G.numEdges());
            std::vector<std::tuple<NodeHandle, NodeHandle, double>> zero_container; // TODO ggf ist das ein highlight mit den pointern -> iwie billig
            std::vector<std::tuple<NodeHandle, NodeHandle, double>> one_container;
            M[0] = &zero_container;
            M[1] = &one_container;

            std::vector<double> W(G.numEdges());

            NodeHandle current_node = u;
            size_t current_nodeId = nodeId;
            proceesed[current_nodeId] = true;

            size_t mate_runner;
            if (suitor[current_nodeId] == G.null_node())
            {
                mate_runner = 0;
                M[1]->emplace_back(std::make_tuple(current_node, suitor2[current_nodeId], ws2[current_nodeId]));
                W[1] = ws2[current_nodeId];
                current_node = suitor2[current_nodeId];
                current_nodeId = G.nodeId(current_node);
                proceesed[current_nodeId] = true;
            } else
            {
                mate_runner = 1;
                M[1]->emplace_back(std::make_tuple(current_node, suitor[current_nodeId], ws[current_nodeId]));
                W[1] = ws[current_nodeId];
                current_node = suitor[current_nodeId];
                current_nodeId = G.nodeId(current_node);
                proceesed[current_nodeId] = true;
            }

            size_t counter = 2;

            while (suitor[current_nodeId] != G.null_node() && suitor2[current_nodeId] != G.null_node())
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

            std::vector<std::tuple<NodeHandle, NodeHandle, double>> *valid_path;

            if (W[counter - 1] > W[counter - 2])
                valid_path = M[counter - 1];
            else
                valid_path = M[counter - 2];

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

    // cycles
    for (NodeHandle u = G.begin(); u != G.end(); ++u)
    {
        if (!proceesed[G.nodeId(u)])
        {
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

            M[1]->emplace_back(std::make_tuple(current_node, suitor2[current_nodeId], ws2[current_nodeId]));
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

            std::vector<std::tuple<NodeHandle, NodeHandle, double>> *valid_path;

            if (W[counter - 1] > W[counter - 2])
                valid_path = M[counter - 1];
            else
                valid_path = M[counter - 2];

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
    // gegenkanten einfügen
    for (auto iter = suitor3.begin(); iter != suitor3.end(); ++iter)
    {
        if (*iter != G.null_node())
        {
            suitor3[G.nodeId(*iter)] = G.node(iter - suitor3.begin());
            ws3[G.nodeId(*iter)] = ws3[iter - suitor3.begin()];
        }
    }
    procedure3();


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
        SequentialSuitor test_1(edgeLst);

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
        SequentialSuitor test_1(edgeLst);

        ofstream myfile;
        std::size_t found = str.find_last_of("/\\");
        std::string out = "../evaluation/SeqSuitor " + str.substr(found + 1) + ".txt";
        myfile.open(out);

        std::stringstream header;
        header << "Algorithm Runtime Quality" << std::endl;
        myfile << header.str();
        cout << header.str();

        size_t rounds = 5;

        for (size_t i = 0; i < rounds; ++i)
        {
            auto start = chrono::high_resolution_clock::now();
            test_1.simple_procedure();
            auto finish = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsed = finish - start;

            std::stringstream line;
            line << "SeqSuitorSimp " << elapsed.count() << " " << test_1.matching_quality() << std::endl;
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
                line << "SeqSuitorImp " << elapsed.count() << " " << test_1.matching_quality() << std::endl;
                myfile << line.str();
                cout << line.str();
                test_1.clear_mate();
            }
        }
        myfile.close();
    }

    return 0;
}