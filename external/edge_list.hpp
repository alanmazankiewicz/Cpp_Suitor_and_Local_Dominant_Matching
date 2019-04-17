#pragma once

#include <cstdint>
#include <fstream>
#include <utility>
#include <string>
#include <vector>

/**
 * Loads edgeList from file.
 */

struct Edge {
    std::size_t from;
    std::size_t to;
    double length;

    bool operator < (const Edge& that) const {
        if (from < that.from) return true;
        else if (from > that.from) return false;
        else return to < that.to;
    }
};

using EdgeList = std::vector<Edge>;

/**
 * Returns the list of edges and the number of nodes. Takes directed graph and makes it undirected
 */

inline std::pair<EdgeList, std::size_t> readEdges(const std::string& file) {
    std::pair<EdgeList, std::size_t> edges;
    std::ifstream in(file);
    in >> edges.second;
    std::size_t from, to;
    double length;
    while (in >> from >> to >> length)
    {
        edges.first.push_back({from, to, length});
        std::swap(from, to);
        edges.first.push_back({from, to, length});
    }


    return edges;
}

