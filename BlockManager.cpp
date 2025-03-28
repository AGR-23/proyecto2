#include "BlockManager.h"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

BlockManager::BlockManager(const char* file_path, size_t total_size) {
    total_blocks = total_size / BLOCK_SIZE;
    file_descriptor = open(file_path, O_RDWR | O_CREAT, 0644);

    if (file_descriptor < 0) {
        perror("Error abriendo archivo de bloques");
        exit(EXIT_FAILURE);
    }

    // Inicializar mapa de bloques
    for (size_t i = 0; i < total_blocks; i++) {
        block_map[i] = false;
    }
}

BlockManager::~BlockManager() {
    close(file_descriptor);
}

size_t BlockManager::allocateBlock() {
    for (auto& pair : block_map) {
        if (!pair.second) {
            pair.second = true;
            return pair.first;
        }
    }
    std::cerr << "No hay bloques disponibles\n";
    return -1;
}

void BlockManager::writeBlock(size_t block_index, const void* data, size_t size) {
    if (block_index >= total_blocks) {
        std::cerr << "Índice de bloque fuera de rango\n";
        return;
    }

    lseek(file_descriptor, block_index * BLOCK_SIZE, SEEK_SET);
    write(file_descriptor, data, size);
}

void BlockManager::readBlock(size_t block_index, void* buffer, size_t size) {
    if (block_index >= total_blocks) {
        std::cerr << "Índice de bloque fuera de rango\n";
        return;
    }

    lseek(file_descriptor, block_index * BLOCK_SIZE, SEEK_SET);
    read(file_descriptor, buffer, size);
}

void BlockManager::sync() {
    fsync(file_descriptor);
} 