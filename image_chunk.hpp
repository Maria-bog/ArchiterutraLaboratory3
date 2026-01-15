#pragma once
#include <vector>
#include <memory>
#include <cstddef>

struct ImageChunk {
    int id;
    int width;
    int height;
    int channels;
    std::vector<unsigned char> data;
    size_t offset_x;
    size_t offset_y;
    
    ImageChunk(int id, int w, int h, int ch, size_t ox, size_t oy)
        : id(id), width(w), height(h), channels(ch), 
          offset_x(ox), offset_y(oy), data(w * h * ch) {}
    
    void invert_colors() {
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = 255 - data[i];
        }
    }
    
    size_t size() const { return data.size(); }
};