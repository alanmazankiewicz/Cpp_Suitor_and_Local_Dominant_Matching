#pragma once

#include <cstdint>
#include <fstream>
#include <utility>
#include <string>
#include <vector>
#include <random>

/**
 * Loads edgeList from file.
 *
 * This edgelist is adjusted to work with the format given by the RMAT graph generator. It randomly generates the weights
 * as they are not given by RMAT. Therefore it is never used for quality comparison tests.
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


inline std::pair<EdgeList, std::size_t> readEdges(const std::string& file) {
    std::pair<EdgeList, std::size_t> edges;
    std::ifstream in(file);

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_real_distribution<double> dist(0.000000000001,100.0);

    size_t last_one = 0;
    std::size_t from, to;

    while (in >> from >> to)
    {
        if(from > to)
        {
            if(from > last_one)
                last_one = from;
        }
        else
        {
            if(to > last_one)
                last_one = to;
        }

        double length = dist(rng);
        edges.first.push_back({from, to, length});
        std::swap(from, to);
        edges.first.push_back({from, to, length});
    }

    edges.second = ++last_one;


    return edges;
}
