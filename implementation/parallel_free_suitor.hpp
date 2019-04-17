#pragma once

#include "implementation/graph2_adj_array.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <cstddef>
#include <memory>
#include <thread>
#include <atomic>
#include <fstream>
#include <sstream>
#include <assert.h>

using Input = std::pair<EdgeList, size_t>;

class ParallelLocklessSuitor
{
public:
    using EdgeIterator = AdjacencyArray::EdgeIterator;
    using NodeHandle = AdjacencyArray::NodeHandle;
    using Mate = std::unique_ptr<std::atomic<EdgeIterator>[]>;
    using Weights = std::unique_ptr<std::atomic_size_t[]>;


    ParallelLocklessSuitor(Input edgeLst);

    // Run algorithm without heavy matching, calls parallely process() and passes subsample of nodes to be processed
    void simple_procedure(size_t num_threads);

    void clear_mate();

    void print_matching() const;

    const double matching_quality() const;

    void test_matching();

private:
    AdjacencyArray G;

    // Here the suitor contains the EdgeIterators not the NodeHandles. Therefore we are using the graph2_adj_array
    // which allows constant time lockup of the head too
    Mate suitor;

    // the parallel lockfree suitor algorithm
    void process(NodeHandle start, NodeHandle finish);
};