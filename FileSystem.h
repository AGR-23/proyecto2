#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "Metadata.h"
#include "VersionGraph.h"
#include "BlockManager.h" // 🔹 Incluir el gestor de bloques
#include <unordered_map>
#include <fstream>

class FileSystem {
private:
    std::fstream storage;
    size_t block_size;
    std::unordered_map<std::string, Metadata> files;
    VersionGraph version_graph;
    BlockManager block_manager; // 🔹 Agregado para gestionar los bloques

public:
    FileSystem(const std::string& storage_path, size_t block_size = 4096);
    ~FileSystem();

    bool createFile(const std::string& file_name, const std::string& file_type);
    bool writeFile(const std::string& file_name, size_t offset, const std::vector<char>& data);
    std::vector<char> readFile(const std::string& file_name);
    void rollbackFile(const std::string& file_name, size_t version_id);
    void printFileMetadata(const std::string& file_name);
    std::vector<size_t> calculateModifiedBlocks(const std::vector<char>& oldData, const std::vector<char>& newData, size_t block_size);
    size_t getBlockSize() const { return block_size; }
};

#endif // FILE_SYSTEM_H