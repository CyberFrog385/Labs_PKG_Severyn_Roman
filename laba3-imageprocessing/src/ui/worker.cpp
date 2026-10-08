#include "ui/worker.h"

Worker::Worker()
    : running(false), joined(true) {
}

Worker::~Worker() {
    if (thread.joinable()) {
        thread.join();
    }
}

bool Worker::busy() const {
    return running.load();
}

void Worker::start(const Job& new_job, const DoneCallback& new_done) {
    if (running.load()) return;
    if (thread.joinable()) {
        thread.join();
    }
    job = new_job;
    done = new_done;
    joined = false;
    running.store(true);
    thread = std::thread(&Worker::run, this);
}

void Worker::run() {
    result = job(&progress);
    running.store(false);
}

void Worker::poll() {
    if (running.load()) return;
    if (!joined && thread.joinable()) {
        thread.join();
        joined = true;
        if (done) {
            done(result);
        }
    }
}
