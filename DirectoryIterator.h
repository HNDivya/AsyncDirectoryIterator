#ifndef DIRECTORYITERATOR_H
#define DIRECTORYITERATOR_H

#include <string>
#include <filesystem>
#include <future>
#include <deque>
#include <mutex>
#include <condition_variable>

namespace fs = std::filesystem;

class DirectoryIterator {
public:
    DirectoryIterator(const std::string& path);
    void iterate(std::deque<std::string>& sharedQueue, std::mutex& mtx, std::condition_variable& cv);

private:
    std::string path_;
};

#endif // DIRECTORYITERATOR_H
