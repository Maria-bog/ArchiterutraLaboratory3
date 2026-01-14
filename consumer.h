#ifndef CONSUMER_H
#define CONSUMER_H

#include "blocking_queue.h"
#include "image_task.h"
#include "task.h"  
#include <atomic>
#include <iostream>
#include <vector>
#include <memory>

class Consumer{
    private:
        BlockingQueue<TaskPtr>& task_queue;
        std::vector<std::unique_ptr<ImageChunk>>& results_collection;
        std::mutex& m;
        int consumer_id;
        std::atomic<int>& processed_counter;

        void process_task(std::shared_ptr<ImageTask> task){
            task->process();
            save_result(task);
        }
        
        void save_result(std::shared_ptr<ImageTask> task){
            auto chunk = task->get_result(); 
            if (!chunk){
                std::cout << "результата нет" << std::endl;
                return;
            }
            std::lock_guard<std::mutex> lock(m);
            results_collection.push_back(std::move(chunk));
            processed_counter++;
        }

    public:
        Consumer(BlockingQueue<TaskPtr>& queue,
                  std::vector<std::unique_ptr<ImageChunk>>& results,
                  std::mutex& mutex,
                  int id,
                  std::atomic<int>& counter)
        : task_queue(queue), 
          results_collection(results), 
          m(mutex), 
          consumer_id(id),
          processed_counter(counter) {}
          
        void run() {
            while (true)
            {
                auto task_opt = task_queue.pop();
                if(!task_opt.has_value()){
                    break;
                }
                TaskPtr task_ptr = task_opt.value();
                if(!task_ptr){
                    continue;
                }
                auto chunk_task = std::dynamic_pointer_cast<ImageTask>(task_ptr);
                if(!chunk_task){
                    continue;
                }
                process_task(chunk_task);
            }
            std::cout << "Consumer " << consumer_id << " завершил работу" << std::endl;
        }
};

#endif