#ifndef WORKER_H
#define WORKER_H

#include <atomic>
#include <functional>
#include <thread>

#include "core/image_buffer.h"
#include "core/parallel.h"

class Worker {
public:
    typedef std::function<ImageBuffer(Progress*)> Job;
    typedef std::function<void(const ImageBuffer&)> DoneCallback;

    Worker();
    ~Worker();

    bool busy() const;
    void start(const Job& job, const DoneCallback& done);
    void poll();

    Progress progress;

private:
    Worker(const Worker&);
    Worker& operator=(const Worker&);

    void run();

    Job job;
    DoneCallback done;
    ImageBuffer result;
    std::thread thread;
    std::atomic<bool> running;
    bool joined;
};

#endif
