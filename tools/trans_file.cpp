#include <fstream>
#include <sstream>
#include <utility>
#include <string>
#include <cstdint>
#include <random>
#include <algorithm>

/**
 * Transforms format of created PaRMAT Graph and adds random weights. It is slow for large graphs.
 */

int main(int argc, char *argv[])
{
    std::ofstream myfile;
    myfile.open(argv[2]);

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_real_distribution<double> dist(0.0000000001,100);

    {
        std::ifstream in(argv[1]);
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
        }
        myfile << ++last_one << std::endl;
    }

    {
        std::ifstream in(argv[1]);
        std::size_t from, to;
        double length;
        while (in >> from >> to)
        {
            myfile << from << " " << to << " " << dist(rng) << std::endl;
        }
    }

    myfile.close();
}