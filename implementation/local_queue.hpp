#pragma once

#include <cstddef>
#include <memory>
#include <algorithm>

/**
 * Non-concurrent queue, user must not exeed size through push and pop.
 *
 * This queue is used for the sequential version of the dominant algo.
 * The parallel verison uses a external queue found in external/
 */


template <class T>
class LocalQueue
{
public:
    LocalQueue(std::size_t capacity_): head(0), tail(0), capacity(capacity_), size(0)
    {
        array = std::vector<T>(capacity_);
    }

    // fully concurrent
    void push_back (const T& e){
        array[head] = e;
        head  = (head + 1) % capacity;
        ++size;
    }


    // fully concurrent
    T pop_front (){
        T& output = array[tail];
        tail = (tail + 1) % capacity;
        --size;

        return output;
    }

    // Whatever other functions you want

    inline std::size_t get_size() const {
        return size;
    }

private:
    std::vector<T> array;
    std::size_t tail;
    std::size_t head;
    std::size_t size;
    const std::size_t capacity;
};
