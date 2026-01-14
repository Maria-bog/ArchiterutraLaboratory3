#ifndef PRODUCER_H
#define PRODUCER_H

#include "blocking_queue.h"
#include "image_task.h"
#include "utils.hpp" 
#include <iostream>
#include <memory>

class Producer{
    private:
        BlockingQueue<TaskPtr>& task_queue;
        int num_consumers;
    public:
        Producer(BlockingQueue<TaskPtr>& t, int c) : task_queue(t), num_consumers(c){}
        void produceFile(const std::string& filename, int width, int height){
            try{
                auto chunks = load_and_split_image(filename, width, height);
                for (auto& chunk : chunks) {
                    auto task = std::make_shared<ImageTask>(std::move(chunk));
                    task_queue.push(task);
                }

            } catch(const std::exception& e){
                std::cerr << "Ошибка: " << e.what() << std::endl;
            }

        }
        void stop_queue() {
            task_queue.stop();
        }


};

#endif