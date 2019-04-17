#pragma once

#include "implementation/graph_adj_array.hpp"
#include "external/concurrentqueue-master/concurrentqueue.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <fstream>
#include <sstream>
#include <assert.h>


using Input = std::pair<EdgeList, size_t>;

class ParallelDominant
{
public:
    using EdgeIterator = AdjacencyArray::EdgeIterator;
    using NodeHandle = AdjacencyArray::NodeHandle;
    using Mate = std::vector<NodeHandle>;

    ParallelDominant(Input edgeLst);

    // run without heavy matching, Algo3 from paper (phase 1 and 2), the private functions are called in this function to handle
    // specific sections of the algorithm
    void simple_procedure(size_t num_threads);

    // reset instance (graph stays as it is)
    void clear_mate();

    void print_matching() const;

    const double matching_quality() const;

    void test_matching();


private:
    // the graph
    AdjacencyArray G;

    // array containg resulting matching, NodeHandles, index: Tail element: Head
    // Contains G.null_nodes() which means vertex is not part of matched edges
    Mate mate;

    // Algorithm 2. This one gets called in phase2() and is run in parallel for diffrent nodes.
    void processVertex(NodeHandle u, moodycamel::ConcurrentQueue<NodeHandle>& Q, Mate& candidate);

    // Parallel search for candiate mates in Phase1, 1st Section
    void parallelProcessVertexSec1(NodeHandle start, NodeHandle finish, Mate& candidate);

    // Parallel matching of found mates in Phase2, 2nd Section
    void parallelProcessVertexSec2(NodeHandle start, NodeHandle finish, Mate& candidate, moodycamel::ConcurrentQueue<NodeHandle>& QC);

    // Handles the dequing of nodes and passing them to processVertex. provessVertex runs in parallel for diffrent nodes
    void phase2(Mate& candidate, moodycamel::ConcurrentQueue<NodeHandle>& QC, moodycamel::ConcurrentQueue<NodeHandle>& QN);

};



