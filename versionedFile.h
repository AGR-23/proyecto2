#ifndef VERSIONEDFILE_H
#define VERSIONEDFILE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <fstream>
#include "performanceTracker.h"
#include <sstream>


struct FileVersion
{
    std::string filename;
    size_t size;
};

struct Delta
{
    size_t position;
    std::string NewData;
};

class versionedFile
{
private:
    std::string filename;
    bool isOpen;
    std::string selectedFile;
    std::vector<FileVersion> versions;
    PerformanceTracker performanceTracker;

public:
    versionedFile();
    bool create();
    bool open();
    std::string read();
    void write(const std::string &data);
    void close();
    void showPerformanceSummary()
    {
        performanceTracker.summary();
    }
    void garbageCollector();
};

#endif