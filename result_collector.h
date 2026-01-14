#ifndef RESULT_COLLECTOR_H
#define RESULT_COLLECTOR_H

#include "image_chunk.hpp"
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>

class ResultCollector {
private:
    std::vector<std::unique_ptr<ImageChunk>> results;
    mutable std::mutex mutex;
    std::condition_variable cv_all_done;
    std::atomic<int> total_tasks{0};
    std::atomic<int> completed_tasks{0};
    
public:
    ResultCollector(int expected_tasks) : total_tasks(expected_tasks) {
        results.reserve(expected_tasks);
    }
    void add_result(std::unique_ptr<ImageChunk> chunk) {
        std::lock_guard<std::mutex> lock(mutex);
        
        if (chunk) {
            results.push_back(std::move(chunk));
            completed_tasks++;

            if (completed_tasks >= total_tasks) {
                cv_all_done.notify_all();
            }
        }
    }
    
    std::vector<std::unique_ptr<ImageChunk>> wait_for_all() {
        std::unique_lock<std::mutex> lock(mutex);
        
        cv_all_done.wait(lock, [this]() {
            return completed_tasks >= total_tasks;
        });
        
        
        return std::move(results);  
    }

    float get_progress() const {
        if (total_tasks == 0) return 0.0f;
        return static_cast<float>(completed_tasks) / total_tasks;
    }
    
    int get_completed_count() const { return completed_tasks; }
    int get_total_count() const { return total_tasks; }
};

#endif