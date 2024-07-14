#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <future>
#include <deque>
#include <mutex>
#include <condition_variable>
#include "DirectoryIterator.h"

// Shared data structure
std::deque<std::string> sharedQueue;
std::mutex mtx;
std::condition_variable cv;
bool done = false;

// Function to print the contents of the shared queue
void print_contents() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return !sharedQueue.empty() || done; });

        while (!sharedQueue.empty()) {
            std::string path = sharedQueue.front();
            sharedQueue.pop_front();
            std::cout << path << std::endl;
        }

        if (done && sharedQueue.empty()) {
            break;
        }
    }
}

int main(int argc, char** argv) 
{    
    std::vector<std::string> directories = { "C:\\Users\\navee\\source\\repos\\", "C:\\" };

    // Create futures for asynchronous directory iteration
    std::vector<std::future<void>> futures;
    for (const auto& dir : directories) {
        auto iterator = std::make_shared<DirectoryIterator>(dir);
        futures.push_back(std::async(std::launch::async, &DirectoryIterator::iterate, iterator, std::ref(sharedQueue), std::ref(mtx), std::ref(cv)));
    }

    // Create and start the consumer thread
    std::thread consumer_thread(print_contents);

    // Wait for all directory iteration tasks to complete
    for (auto& future : futures) {
        future.get();
    }

    // Notify the consumer thread to finish
    {
        std::unique_lock<std::mutex> lock(mtx);
        done = true;
        cv.notify_one();
    }

    // Join the consumer thread
    consumer_thread.join();

    return 0;
}
