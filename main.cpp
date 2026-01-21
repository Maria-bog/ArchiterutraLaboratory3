#include "blocking_queue.h"
#include "task.h"
#include "image_task.h"
#include "producer.h"
#include "consumer.h"
#include "utils.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <algorithm>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0]
                  << " <input_image.bmp> <output_image.bmp> <num_consumers> [chunk_width] [chunk_height]"
                  << std::endl;
        return 1;
    }
    
    std::string input_file = argv[1];
    std::string output_file = argv[2];
    int num_consumers = std::stoi(argv[3]);
    int chunk_width = (argc > 4) ? std::stoi(argv[4]) : 128;
    int chunk_height = (argc > 5) ? std::stoi(argv[5]) : 128;
    
    try {
        
        BMPHeader header = get_bmp_header(input_file);
        
        
        
        std::cout << "Исходное изображение: " << input_file << std::endl;
        std::cout << "Итоговое изображение: " << output_file << std::endl;
        std::cout << "Разрешение изображения: " << header.width << "x" << header.height << std::endl;
        std::cout << "Размер сегмента:: " << chunk_width << "x" << chunk_height << std::endl;
        std::cout << "Число рабочих потоков: " << num_consumers << std::endl;
        
        
        BlockingQueue<TaskPtr> task_queue;
        
        std::vector<std::unique_ptr<ImageChunk>> results_storage;
        std::mutex results_mutex;
        std::atomic<int> processed_counter{0};
        
        Producer producer(task_queue, num_consumers);
        
        std::vector<std::thread> consumer_threads;
        
        for (int i = 0; i < num_consumers; ++i) {
            consumer_threads.emplace_back([&task_queue, &results_storage, &results_mutex, i, &processed_counter]() {
                
                Consumer consumer(task_queue, results_storage, results_mutex, i, processed_counter);
                consumer.run();
            });
        }
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        producer.produceFile(input_file, chunk_width, chunk_height);
        
        producer.stop_queue();
        
        for (auto& thread : consumer_threads) {
            thread.join();
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time);
        
        std::cout << "\nВсе задачи обработаны" << std::endl;
        std::cout << "Обработано блоков: " << processed_counter << std::endl;
        std::cout << "Собираем результаты" << std::endl;
        
        if (results_storage.empty()) {
            std::cerr << "Error: No results collected!" << std::endl;
            return 1;
        }
        
        std::sort(results_storage.begin(), results_storage.end(),
            [](const std::unique_ptr<ImageChunk>& a, const std::unique_ptr<ImageChunk>& b) {
                return a->id < b->id;
            });
        
        std::cout << "Saving processed image: " << output_file << std::endl;
        save_image_from_chunks(output_file, results_storage, header.width, header.height);
        
        std::cout << "\n" << std::endl;
        std::cout << "Обработка завершена" << std::endl;
        std::cout << "Итоговое время: " << duration.count() << " ms" << std::endl;
        std::cout << "Всего обработано блоков: " << processed_counter.load() << std::endl;
        std::cout << "Среднее время на блок: "
                  << (processed_counter.load() > 0 ? duration.count() / processed_counter.load() : 0)
                  << " ms" << std::endl;
        
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}