#pragma once

#include "implementation/graph_adj_array.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <mutex>
#include <cstddef>
#include <memory>
#include <thread>
#include <atomic>
#include <fstream>
#include <sstream>
#include <assert.h>

using Input = std::pair<EdgeList, size_t>;

class ParallelSuitor
{
public:
    using EdgeIterator = AdjacencyArray::EdgeIterator;
    using NodeHandle = AdjacencyArray::NodeHandle;
    using Mate = std::unique_ptr<std::atomic<NodeHandle>[]>;
    using Weights = std::unique_ptr<std::atomic<double>[]>;

    // Bonus: UnionFind to detect which node is part of whoch cycle in heavy matching
    class UnionFind {
    public:

        friend ParallelSuitor;

        UnionFind(size_t _num_nodes, ParallelSuitor& owner_);

        NodeHandle parentOf(NodeHandle n) const;

        NodeHandle find_representative(NodeHandle a);

        void unify(NodeHandle root1, NodeHandle root2); // only under locks!

    private:
        long num_nodes;
        ParallelSuitor* const owner;

        Mate union_find;
        std::vector<long> ranks;

        std::unique_ptr<std::recursive_mutex []> rec_mtx;
    };


    ParallelSuitor(Input edgeLst);

    // run without heavy matching, wrapper function that calls procedure to invoke simple par_suitor run (process())
    inline void simple_procedure(size_t num_threads);

    // Runs Heavy Matching.
    void improved_procedure(size_t num_threads);

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
    Mate suitor;

    // Bonus: Like suitor but assumes that edges matched in mate are not present in Graph
    Mate suitor2;

    // Bonus: Result of heavy matching optimization (after path growing suitor and suitor)
    Mate suitor3; // TODO: does probably not have to be atomic

    // Bonus: Nessesary for last matching (after path growing), the resulting matching gets transfered into suito3
    std::vector<NodeHandle> suitor4;

    // Array containing weights of heaviest partner (the partner that suitor points to)
    Weights ws;

    // Bonus: like ws for suitor2
    Weights ws2;

    // Bonus: like ws for suitor3
    Weights ws3;

    // Bonus: like ws for suitor3
    std::vector<double> ws4;

    // Mutexes for each node
    std::unique_ptr<mutex[]> mtx;

    // Indicates if Heavy Matching has been run, improved procedure changes to true, clear
    // mate changes to false.
    bool optimized = false;

    // Bonus: Nessesary for heavy matching, seq version contains this inside the function
    std::unique_ptr<std::atomic<bool>[]> proceesed; // TODO: does probably not have to be atomic

    // Bonus: UnionFind to detect which node is part of whoch cycle in heavy matching
    UnionFind unionFind;


    // Parallized Algoritm II from paper, the actuall suitor algorithm, processes nodes in the range start finish
    // in contrast to sequential version that by itself runs through all nodes
    void process(NodeHandle start, NodeHandle finish);

    // Bonus: Like process but for heavy matching. Adjusted for matching only edges in suitor2 that are not part of suitor
    void process2(NodeHandle start, NodeHandle finish);

    // Bonus: Like process but for heavy matching. Adjusted for matching only edges in suitor4 that are not part of suitor3
    // This one is sequential.
    void process3();

    // Function to invoke other functions (such as process()) in parallel. Contains this whole call to std::thread and the join()
    void procedure(size_t num_threads, void (ParallelSuitor::*process) (NodeHandle, NodeHandle),
            vector<size_t>::const_iterator beginNode, vector<size_t>::const_iterator endNode, size_t numNodes);

    // Bonus: the logic to discover and process paths in the heavy matching
    void path_discovery(NodeHandle start, NodeHandle finish);

    // Bonus: the logic to discover cycles in the heavy matching (which node belongs to a cycle and if so to which cycle)
    void find_cycle(NodeHandle start, NodeHandle finish);

    // Bonus: Process the found cycles by find_cycle()
    void work_cycle(std::vector<size_t>::const_iterator start, std::vector<size_t>::const_iterator finish);

};