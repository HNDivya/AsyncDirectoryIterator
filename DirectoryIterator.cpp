#include "DirectoryIterator.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

DirectoryIterator::DirectoryIterator(const std::string& path)
    : path_(path) {}

void DirectoryIterator::iterate(std::deque<std::string>& sharedQueue, std::mutex& mtx, std::condition_variable& cv) {
    auto start = std::chrono::high_resolution_clock::now();
    size_t directoryCount = 0;
    std::vector<std::string> batch;
    const size_t batchSize = 10; // Adjust the batch size as needed

    try {
        for (const auto& entry : fs::recursive_directory_iterator(path_, fs::directory_options::skip_permission_denied)) {
            try {
                if (entry.is_directory()) {
                    batch.push_back(entry.path().string());
                    ++directoryCount;
                }

                if (batch.size() >= batchSize) {
                    {
                        std::scoped_lock lock(mtx);
                        sharedQueue.insert(sharedQueue.end(), batch.begin(), batch.end());
                    }
                    cv.notify_all();
                    batch.clear();
                }
            }
            catch (const fs::filesystem_error& e) {
                std::cerr << "Error accessing path: " << entry.path() << " - " << e.what() << std::endl;
            }
        }

        // Process any remaining items in the batch
        if (!batch.empty()) {
            {
                std::scoped_lock lock(mtx);
                sharedQueue.insert(sharedQueue.end(), batch.begin(), batch.end());
            }
            cv.notify_all();
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Error iterating directory " << path_ << ": " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;

    // Log to file
    std::ofstream log_file("log.txt", std::ios_base::app);
    if (log_file.is_open()) {
        log_file << "Thread ID: " << std::this_thread::get_id()
            << " processed directory: " << path_
            << " Directory Count: " << directoryCount
            << " in " << duration.count() << " ms." << std::endl;
    }
    else {
        std::cerr << "Failed to open log file." << std::endl;
    }
}
