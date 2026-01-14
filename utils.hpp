#pragma once
#include "image_chunk.hpp"
#include <vector>
#include <memory>
#include <fstream>
#include <iostream>


#pragma pack(push, 1)
struct BMPHeader {
    uint16_t signature;
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
    
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t important_colors;
};
#pragma pack(pop)

std::vector<std::unique_ptr<ImageChunk>> load_and_split_image(
    const std::string& filename, 
    int chunk_width, 
    int chunk_height) {
    
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    BMPHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(BMPHeader));
    
    if (header.signature != 0x4D42) { // 'BM'
        throw std::runtime_error("Not a BMP file");
    }
    
    if (header.bits_per_pixel != 24) {
        throw std::runtime_error("Only 24-bit BMP supported");
    }
    
    file.seekg(header.data_offset, std::ios::beg);
    
    int width = header.width;
    int height = header.height;
    int channels = 3; // RGB
    
    int row_padding = (4 - (width * channels) % 4) % 4;
    
   
    std::vector<unsigned char> image_data(width * height * channels);
    
    for (int y = height - 1; y >= 0; --y) { 
        file.read(reinterpret_cast<char*>(&image_data[y * width * channels]), 
                  width * channels);
        file.seekg(row_padding, std::ios::cur);
    }
    
    
    std::vector<std::unique_ptr<ImageChunk>> chunks;
    int chunk_id = 0;
    
    for (int y = 0; y < height; y += chunk_height) {
        for (int x = 0; x < width; x += chunk_width) {
            int actual_chunk_width = std::min(chunk_width, width - x);
            int actual_chunk_height = std::min(chunk_height, height - y);
            
            auto chunk = std::make_unique<ImageChunk>(
                chunk_id++, actual_chunk_width, actual_chunk_height,
                channels, x, y
            );
            
            
            for (int cy = 0; cy < actual_chunk_height; ++cy) {
                for (int cx = 0; cx < actual_chunk_width; ++cx) {
                    for (int c = 0; c < channels; ++c) {
                        size_t src_idx = ((y + cy) * width + (x + cx)) * channels + c;
                        size_t dst_idx = (cy * actual_chunk_width + cx) * channels + c;
                        chunk->data[dst_idx] = image_data[src_idx];
                    }
                }
            }
            
            chunks.push_back(std::move(chunk));
        }
    }
    
    return chunks;
}

void save_image_from_chunks(
    const std::string& filename,
    const std::vector<std::unique_ptr<ImageChunk>>& chunks,
    int total_width,
    int total_height) {
    
    int channels = 3;
    std::vector<unsigned char> image_data(total_width * total_height * channels, 0);
    
   
    for (const auto& chunk : chunks) {
        for (int y = 0; y < chunk->height; ++y) {
            for (int x = 0; x < chunk->width; ++x) {
                for (int c = 0; c < channels; ++c) {
                    size_t src_idx = (y * chunk->width + x) * channels + c;
                    size_t dst_idx = ((chunk->offset_y + y) * total_width + 
                                     (chunk->offset_x + x)) * channels + c;
                    image_data[dst_idx] = chunk->data[src_idx];
                }
            }
        }
    }
    
    
    BMPHeader header = {};
    header.signature = 0x4D42;
    header.file_size = sizeof(BMPHeader) + total_width * total_height * channels + 
                       total_height * ((4 - (total_width * channels) % 4) % 4);
    header.data_offset = sizeof(BMPHeader);
    header.header_size = 40;
    header.width = total_width;
    header.height = total_height;
    header.planes = 1;
    header.bits_per_pixel = 24;
    header.compression = 0;
    header.image_size = total_width * total_height * channels;
    header.x_pixels_per_meter = 2835;
    header.y_pixels_per_meter = 2835;
    header.colors_used = 0;
    header.important_colors = 0;
    
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot create file: " + filename);
    }
    
    file.write(reinterpret_cast<char*>(&header), sizeof(BMPHeader));
    
    int row_padding = (4 - (total_width * channels) % 4) % 4;
    char padding[3] = {0, 0, 0};
    
    for (int y = total_height - 1; y >= 0; --y) {
        file.write(reinterpret_cast<char*>(&image_data[y * total_width * channels]), 
                   total_width * channels);
        if (row_padding > 0) {
            file.write(padding, row_padding);
        }
    }
}