#pragma once

#include "implementation/graph_adj_array.hpp"
#include "implementation/local_queue.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <assert.h>


using Input = std::pair<EdgeList, size_t>;

class SequentialDominant
{
public:
    using EdgeIterator = AdjacencyArray::EdgeIterator;
    using NodeHandle = AdjacencyArray::NodeHandle;
    using Mate = std::vector<NodeHandle>;


    SequentialDominant(Input edgeLst);


    //run without heavy matching, wrapper function that calls procedure with mate and processVertex
    inline void simple_procedure();

    // reset instance (graph stays as it is)
    void clear_mate();

    void print_matching() const;

    const double matching_quality() const;

    // Bonus: Runs Heavy Matching.
    void improved_procedure();

    void test_matching();


private:
    // the graph
    AdjacencyArray G;

    // array containg resulting matching, NodeHandles, index: Tail element: Head
    // Contains G.null_nodes() which means vertex is not part of matched edges
    Mate mate;

    // Bonus: Like mate but assumes that edges matched in mate are not present in Graph
    Mate mate2;

    // Bonus: Result of heavy matching optimization (after path growing mate and mate2)
    Mate mate3;

    // Indicates if Heavy Matching has been run, improved procedure changes to true, clear
    // mate changes to false.
    bool optimized = false;

    // Algorithm I from paper. To be DRY regarding the heavy matching the mate as well which
    // processVertex variant has to be run has to be specified. See simple_procedure.
    void procedure(Mate& mate, void(SequentialDominant::*processVertex) (NodeHandle, LocalQueue<NodeHandle>&, Mate&, Mate&));

    // Algorithm II from paper. Finds candidates and matches if local dominant is found.
    void processVertex(NodeHandle u, LocalQueue<NodeHandle>& Q, Mate& candidate, Mate& mate);

    // Algorithm II from paper. Finds candidates and matches if local dominant is found.
    // Adjusted for heavy matching. Disregards edges that are matched in mate
    void processVertex2(NodeHandle u, LocalQueue<NodeHandle>& Q, Mate& candidate, Mate& mate);
};