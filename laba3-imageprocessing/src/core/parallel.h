#ifndef PARALLEL_H
#define PARALLEL_H

#include <atomic>
#include <functional>

struct Progress {
    Progress();
    void reset(int total_rows);
    std::atomic<int> value;
    std::atomic<int> total;
};

int max_threads();
void parallel_rows(int rows, int thread_count, const std::function<void(int, int)>& range_fn);

#endif
