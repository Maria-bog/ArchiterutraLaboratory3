#ifndef TASK_H
#define TASK_H

#include <memory>
#include "image_chunk.hpp" 

class ITask{
    public:
        virtual ~ITask() = default;
        virtual int getId() const = 0;
        virtual void process() = 0;
        virtual std::unique_ptr<ImageChunk> get_result() = 0;  
};
using TaskPtr = std::shared_ptr<ITask>;

#endif