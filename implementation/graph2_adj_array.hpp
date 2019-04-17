#pragma once

#include <cstddef>
#include <vector>
#include <queue>
#include <algorithm>

#include "edge_list2.hpp" // #include external/edge_list.hpp in case a .graph file should be given not a .raw_graph

/**
 * Underlying graph data structure. This graph contains additionally an heads array that is nessesary for par_free_suitor.
 * For all others graph_adj_array is used.
 *
 * Ties are broken as long as we not parallelize the search over adjacent edges for a node, which we never do as the edges
 * are sorted accroding to their id
 *
 * It is important that num_nodes is exactly one greater then the highest vertex id
 */

using namespace std;


class AdjacencyArray
{
public:
    using NodeHandle = vector<size_t>::const_iterator;
    using EdgeIterator = vector<pair<size_t, double>>::const_iterator;

    AdjacencyArray() = default;

    AdjacencyArray(size_t num_nodes, const EdgeList &edges): num_nodes_(num_nodes), num_edges_(edges.size())
    {
        edges_ = vector<std::pair<size_t, double>>(edges.size());
        heads_ = vector<size_t>(edges.size());
        index_ = vector<size_t>(num_nodes);
        null_val_vec.emplace_back(num_nodes);
        null_val_edge.emplace_back(std::make_pair(num_nodes, 0.0));

        EdgeList sorted_edges = edges;
        sort(sorted_edges.begin(), sorted_edges.end());

        size_t current = 0;
        index_[0] = 0;

        for(auto iter = sorted_edges.begin(); iter != sorted_edges.end(); ++iter)
        {
            if(iter->from != current)
            {
                for(size_t i = current +1; i <= iter->from; ++i)
                {
                    index_[i] = iter - sorted_edges.begin();
                }
                current = iter->from;
                index_[current] = iter - sorted_edges.begin();
            }
            edges_[iter - sorted_edges.begin()] = std::make_pair(iter->to, iter->length);
            heads_[iter - sorted_edges.begin()] = iter->from;
        }
    }

    inline const size_t &numNodes() const
    {
        return num_nodes_;
    }

    inline const size_t &numEdges() const
    {
        return num_edges_;
    }

    inline const NodeHandle node(size_t n) const
    {
        if(n == num_nodes_)
            return null_val_vec.begin();
        else
            return index_.begin() + n;
    }

    inline const size_t nodeId(NodeHandle n) const
    {
        return n - index_.begin();
    }

    // Do not call with nodeHandles that dont have edges
    inline const EdgeIterator beginEdges(NodeHandle n) const
    {
        return edges_.begin() + *n;
    }

    // Do not call with nodeHandles that dont have edges
    inline const EdgeIterator endEdges(NodeHandle n) const
    {
        if((n+1) == index_.end())
            return edges_.end();
        else
            return beginEdges(n+1);
    }

    inline const NodeHandle null_node() const
    {
        return null_val_vec.begin();
    }

    inline const EdgeIterator null_edge() const
    {
        return null_val_edge.begin();
    }

    inline const NodeHandle begin() const
    {
        return index_.begin();
    }

    inline const NodeHandle end() const
    {
        return index_.end();
    }

    inline const NodeHandle edgeHead(EdgeIterator e) const
    {
        if(e == null_val_edge.begin())
            return null_val_vec.begin();

        return node(heads_[e - edges_.begin()]);
    }

private:
    size_t num_nodes_;
    vector<std::pair<size_t, double>> edges_;
    vector<size_t> heads_;
    vector<size_t> index_;
    vector<size_t> null_val_vec;
    vector<pair<size_t, double>> null_val_edge;
    size_t num_edges_;
};
