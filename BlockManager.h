#ifndef BLOCK_MANAGER_H
#define BLOCK_MANAGER_H

#include <vector>
#include <unordered_map>
#include <cstdint>

class BlockManager {
private:
    static constexpr size_t BLOCK_SIZE = 4096;
    size_t total_blocks;
    int file_descriptor;  // Descriptor del archivo que almacena los bloques
    std::unordered_map<size_t, bool> block_map;  // Mapa de bloques utilizados

public:
    BlockManager(const char* file_path, size_t total_size);
    ~BlockManager();

    size_t allocateBlock();  // Asigna un nuevo bloque
    void writeBlock(size_t block_index, const void* data, size_t size);
    void readBlock(size_t block_index, void* buffer, size_t size);

    void sync();  // Guarda cambios en disco
};

#endif 