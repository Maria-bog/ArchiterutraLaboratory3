#ifndef IMAGE_TASK_H
#define IMAGE_TASK_H

#include "task.h"
#include "image_chunk.hpp"
#include <iostream>
#include <memory>

class ImageTask : public ITask {
    private:
        std::unique_ptr<ImageChunk> chunk;
    public:
        ImageTask(std::unique_ptr<ImageChunk> ch) : chunk(std::move(ch)){}
        int getId() const override { 
            return chunk ? chunk->id : -1; 
        }
        void process() override{
            if (!chunk) return;
            chunk->invert_colors();
        }
        std::unique_ptr<ImageChunk> get_result() override {
            return std::move(chunk);
        }
        int get_width() const { return chunk ? chunk->width : 0; }
        int get_height() const { return chunk ? chunk->height : 0; }
        size_t get_offset_x() const { return chunk ? chunk->offset_x : 0; }
        size_t get_offset_y() const { return chunk ? chunk->offset_y : 0; }
};

#endif