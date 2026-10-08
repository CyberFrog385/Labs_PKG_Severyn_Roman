#include "core/parallel.h"

#include <cstddef>
#include <thread>
#include <vector>

Progress::Progress()
    : value(0), total(0) {
}

void Progress::reset(int total_rows) {
    value.store(0);
    total.store(total_rows);
}

int max_threads() {
    unsigned int count = std::thread::hardware_concurrency();
    if (count < 1) return 1;
    if (count > 4) return 4;
    return static_cast<int>(count);
}

void parallel_rows(int rows, int thread_count, const std::function<void(int, int)>& range_fn) {
    if (rows <= 0) return;
    if (thread_count <= 1 || rows < 64) {
        range_fn(0, rows);
        return;
    }
    int chunk = (rows + thread_count - 1) / thread_count;
    std::vector<std::thread> threads;
    for (int t = 0; t < thread_count; ++t) {
        int y0 = t * chunk;
        int y1 = y0 + chunk;
        if (y1 > rows) y1 = rows;
        if (y0 >= y1) break;
        threads.push_back(std::thread(range_fn, y0, y1));
    }
    for (size_t i = 0; i < threads.size(); ++i) {
        threads[i].join();
    }
}
