#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H

#include <queue>      
#include <mutex>      
#include <condition_variable>  
#include <optional> 
#include <atomic>   

template <typename T>
class BlockingQueue{
    private:
        std::queue<T> q;
        std::mutex m;
        std::condition_variable cv;
        std::atomic<bool> stopped{false};
    public:
        BlockingQueue() = default;
        ~BlockingQueue() = default;
        void push(T&& item);
        void stop();
        std::optional<T> pop();
        bool empty() const;
        size_t size() const;

};

#include "blocking_queue.tpp" 
#endif