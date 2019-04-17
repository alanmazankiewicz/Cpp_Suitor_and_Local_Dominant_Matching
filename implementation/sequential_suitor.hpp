#pragma once

#include "implementation/graph_adj_array.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <tuple>
#include <fstream>
#include <sstream>
#include <assert.h>


using Input = std::pair<EdgeList, size_t>;

class SequentialSuitor
{
public:
    using EdgeIterator = AdjacencyArray::EdgeIterator;
    using NodeHandle = AdjacencyArray::NodeHandle;
    using Mate = std::vector<NodeHandle>;
    using Weights = std::vector<double>;


    SequentialSuitor(Input edgeLst);

    // run without heavy matching, wrapper function that calls procedure
    inline void simple_procedure();

    // reset instance (graph stays as it is)
    void clear_mate();

    void print_matching() const;

    const double matching_quality() const;

    // Runs Heavy Matching.
    void improved_procedure();

    void test_matching();

private:
    // the graph
    AdjacencyArray G;

    // array containg resulting matching, NodeHandles, index: Tail element: Head
    // Contains G.null_nodes() which means vertex is not part of matched edges
    Mate suitor;

    // Like suitor but assumes that edges matched in mate are not present in Graph
    Mate suitor2;

    // Result of heavy matching optimization (after path growing suitor and suitor)
    Mate suitor3;

    // Nessesary for last matching (after path growing), the resulting matching gets transfered into suito3
    Mate suitor4;

    // Array containing weights of heaviest partner (the partner that suitor points to)
    Weights ws;

    // like ws for suitor2
    Weights ws2;

    // like ws for suitor3
    Weights ws3;

    // like ws for suitor3
    Weights ws4;

    // Indicates if Heavy Matching has been run, improved procedure changes to true, clear
    // mate changes to false.
    bool optimized = false;

    // Algoritm II from paper, the actuall suitor algorithm
    void procedure();

    // Algorithm II adjusted for matching only edges in suitor2 that are not part of suitor
    void procedure2();

    // Algorithm II adjusted for matching only edges in suitor4 that are not part of suitor3
    void procedure3();
};