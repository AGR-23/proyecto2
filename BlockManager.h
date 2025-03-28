#ifndef BLOCK_MANAGER_H
#define BLOCK_MANAGER_H

#include <vector>
#include <bitset>  // Usemos bitset en lugar de unordered_map (más eficiente)
#include <cstdint>
#include <fstream>

class BlockManager {
private:
    static constexpr size_t BLOCK_SIZE = 4096;
    size_t total_blocks;
    int file_descriptor;  // Descriptor del archivo de bloques
    std::bitset<65536> block_map;  // Usamos bitset para eficiencia (ajustar tamaño si es necesario)
    std::string metadata_file;  // Archivo para almacenar el estado de los bloques

    void saveBlockMap();  // Guarda el estado de los bloques en disco
    void loadBlockMap();  // Carga el estado de los bloques desde disco

public:
    BlockManager(const char* file_path, size_t total_size);
    ~BlockManager();

    size_t allocateBlock();  // Asigna un nuevo bloque
    void freeBlock(size_t block_index);  // Libera un bloque para reutilización
    void writeBlock(size_t block_index, const void* data, size_t size);
    void readBlock(size_t block_index, void* buffer, size_t size);
    void sync();  // Guarda cambios en disco
};

#endif
