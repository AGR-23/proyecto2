#ifndef VERSIONEDFILE_H
#define VERSIONEDFILE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>

struct FileVersion 
{
    std::string filename;
    size_t size;
};

class versionedFile 
{
    private:

    std::string filename;
    bool isOpen;
    std::string selectedFile;
    std::vector<FileVersion> versions;

    public:

    versionedFile();
    bool create();
    bool open();
    std::string read();
    void write(const std::string& data);
    void close();
};

#endif