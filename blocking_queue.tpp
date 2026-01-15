#ifndef BLOCKING_QUEUE_TPP
#define BLOCKING_QUEUE_TPP

#include <iostream>
#include <chrono>

template<typename T>
void BlockingQueue<T>::push(T item){
    {
        std::lock_guard<std::mutex> lock(m);
        if(stopped) return;
        q.push(std::move(item));
    }
    cv.notify_one();
}

template<typename T>
void BlockingQueue<T>::stop(){
    {
        std::lock_guard<std::mutex> lock(m);
        stopped = true;
    }
    cv.notify_all();
}

template<typename T>
std::optional<T> BlockingQueue<T>::pop(){
    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [this](){
        return !q.empty() || stopped;
    });

    if (stopped && q.empty()) {
        return std::nullopt;
    }
    T item = std::move(q.front());
    q.pop();
    return item;
}

template<typename T>
bool BlockingQueue<T>::empty() const {
    std::lock_guard<std::mutex> lock(m);
    return q.empty();
}

template<typename T>
size_t BlockingQueue<T>::size() const {
    std::lock_guard<std::mutex> lock(m);
    return q.size();
}

#endif