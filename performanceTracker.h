#ifndef PERFORMANCETRACKER_H
#define PERFORMANCETRACKER_H

#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <sstream>

// PerformanceTracker class to measure execution time and memory usage
// https://labex.io/es/tutorials/cpp-how-to-track-runtime-memory-usage-419977
class PerformanceTracker
{
private:
    std::chrono::steady_clock::time_point startTime;
    size_t initialMemory;
    size_t peakMemory = 0;

public:
    void start()
    {
        startTime = std::chrono::steady_clock::now();
        initialMemory = getCurrentMemoryUsage();
    }

    void report(const std::string &operation)
    {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime);

        size_t currentMemory = getCurrentMemoryUsage();
        size_t memoryUsed = (currentMemory > initialMemory) ? (currentMemory - initialMemory) : 0;
        peakMemory = std::max(peakMemory, currentMemory);

        std::cout << "Execution time " << operation << " - " << duration.count() << "ms"
                  << " | Memory used: " << memoryUsed << " bytes" << std::endl;
    }

    void summary()
    {
        std::cout << "\n[Performance Summary]\n";
        std::cout << "Maximum memory peak: " << peakMemory << " bytes\n";
    }

private:
    size_t getCurrentMemoryUsage() // get memory usage in bytes from /proc/self/status in Linux
    {
        std::ifstream statusFile("/proc/self/status");
        std::string line;
        while (std::getline(statusFile, line))
        {
            if (line.find("VmRSS:") == 0)
            {
                std::istringstream iss(line);
                std::string label;
                size_t memoryKb;
                iss >> label >> memoryKb;
                return memoryKb * 1024;
            }
        }
        return 0;
    }
};

#endif // PERFORMANCETRACKER_H