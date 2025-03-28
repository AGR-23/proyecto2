#include "BlockManager.h"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <algorithm>

BlockManager::BlockManager(const char* file_path, size_t total_size) 
    : data_file_path(file_path), metadata_file_path(std::string(file_path) + ".meta") {
    
    total_blocks = total_size / BLOCK_SIZE;
    
    // Abrir archivo de datos
    file_descriptor = open(file_path, O_RDWR | O_CREAT, 0644);
    if (file_descriptor < 0) {
        perror("Error opening block file");
        exit(EXIT_FAILURE);
    }
    
    // Verificar tamaño del archivo y expandirlo si es necesario
    off_t current_size = lseek(file_descriptor, 0, SEEK_END);
    if (current_size < static_cast<off_t>(total_size)) {
        // Crear archivo del tamaño adecuado
        lseek(file_descriptor, total_size - 1, SEEK_SET);
        char null_byte = 0;
        write(file_descriptor, &null_byte, 1);
    }
    
    // Inicializar mapa de bloques (cargar desde metadata si existe)
    loadBlockMap();
}

BlockManager::~BlockManager() {
    // Guardar mapa de bloques antes de cerrar
    saveBlockMap();
    close(file_descriptor);
}

void BlockManager::loadBlockMap() {
    block_map.clear();
    // Inicializar todos los bloques como libres
    for (size_t i = 0; i < total_blocks; i++) {
        block_map[i] = false;
    }
    
    // Intentar cargar mapa de bloques desde archivo de metadatos
    std::ifstream meta_file(metadata_file_path, std::ios::binary);
    if (meta_file) {
        // Formato: número de bloque (size_t) + estado (bool)
        size_t block_number;
        bool is_used;
        
        while (meta_file.read(reinterpret_cast<char*>(&block_number), sizeof(size_t)) &&
               meta_file.read(reinterpret_cast<char*>(&is_used), sizeof(bool))) {
            if (block_number < total_blocks) {
                block_map[block_number] = is_used;
            }
        }
        meta_file.close();
    }
}

void BlockManager::saveBlockMap() {
    std::ofstream meta_file(metadata_file_path, std::ios::binary | std::ios::trunc);
    if (!meta_file) {
        std::cerr << "Error: No se pudo guardar el mapa de bloques\n";
        return;
    }
    
    // Guardar todos los bloques y su estado
    for (const auto& [block_number, is_used] : block_map) {
        meta_file.write(reinterpret_cast<const char*>(&block_number), sizeof(size_t));
        meta_file.write(reinterpret_cast<const char*>(&is_used), sizeof(bool));
    }
    
    meta_file.close();
}

size_t BlockManager::allocateBlock() {
    for (auto& pair : block_map) {
        if (!pair.second) {
            pair.second = true;
            return pair.first;
        }
    }
    std::cerr << "No hay bloques disponibles\n";
    return static_cast<size_t>(-1);
}

void BlockManager::freeBlock(size_t block_index) {
    if (block_index < total_blocks) {
        block_map[block_index] = false;
    }
}

void BlockManager::writeBlock(size_t block_index, const void* data, size_t size) {
    if (block_index >= total_blocks) {
        std::cerr << "Índice de bloque fuera de rango\n";
        return;
    }
    
    size_t write_size = std::min(size, BLOCK_SIZE);
    off_t offset = block_index * BLOCK_SIZE;
    
    lseek(file_descriptor, offset, SEEK_SET);
    write(file_descriptor, data, write_size);
    
    // Marcar bloque como utilizado
    block_map[block_index] = true;
}

void BlockManager::readBlock(size_t block_index, void* buffer, size_t size) {
    if (block_index >= total_blocks) {
        std::cerr << "Índice de bloque fuera de rango\n";
        return;
    }
    
    size_t read_size = std::min(size, BLOCK_SIZE);
    off_t offset = block_index * BLOCK_SIZE;
    
    lseek(file_descriptor, offset, SEEK_SET);
    read(file_descriptor, buffer, read_size);
}

size_t BlockManager::getTotalBlocks() const {
    return total_blocks;
}

bool BlockManager::isBlockUsed(size_t block_index) const {
    auto it = block_map.find(block_index);
    return (it != block_map.end()) && it->second;
}

void BlockManager::sync() {
    fsync(file_descriptor);
    // También guardar mapa de bloques
    saveBlockMap();
}