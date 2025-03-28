#include "BlockManager.h"
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <limits>

#ifdef _WIN32
#include <io.h>
#define fsync _commit
#endif

BlockManager::BlockManager(const char *file_path, size_t total_size)
{
    total_blocks = total_size / BLOCK_SIZE;
    file_descriptor = open(file_path, O_RDWR | O_CREAT, 0644);
    metadata_file = std::string(file_path) + ".meta"; // Archivo de metadatos

    if (file_descriptor < 0)
    {
        perror("Error abriendo archivo de bloques");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(file_descriptor, total_size) == -1)
    {
        perror("Error ajustando el tamaño del archivo");
        close(file_descriptor);
        exit(EXIT_FAILURE);
    }

    loadBlockMap();
}

BlockManager::~BlockManager()
{
    saveBlockMap();
    close(file_descriptor);
}

void BlockManager::loadBlockMap()
{
    std::ifstream meta(metadata_file, std::ios::binary);
    if (meta.is_open())
    {
        std::string bitset_str;
        meta >> bitset_str;
        block_map = std::bitset<65536>(bitset_str);
        meta.close();
    }
    else
    {
        block_map.reset(); // Inicializar todos los bloques como libres
    }
}

void BlockManager::saveBlockMap()
{
    std::ofstream meta(metadata_file, std::ios::binary);
    if (meta.is_open())
    {
        std::string bitset_str = block_map.to_string();
        meta.write(bitset_str.c_str(), bitset_str.size());
        meta.close();
    }
}

size_t BlockManager::allocateBlock()
{
    for (size_t i = 0; i < total_blocks; i++)
    {
        if (!block_map[i])
        {
            block_map[i] = true;
            saveBlockMap();
            return i;
        }
    }
    throw std::runtime_error("No hay bloques disponibles");
}

void BlockManager::freeBlock(size_t block_index)
{
    if (block_index < total_blocks && block_map[block_index])
    {
        block_map[block_index] = false;
        saveBlockMap();
    }
}

void BlockManager::writeBlock(size_t block_index, const void *data, size_t size)
{
    if (block_index >= total_blocks || !block_map[block_index])
    {
        throw std::runtime_error("Índice de bloque inválido o no asignado");
    }

    if (lseek(file_descriptor, block_index * BLOCK_SIZE, SEEK_SET) == -1)
    {
        throw std::runtime_error("Error al posicionar el puntero en el archivo");
    }

    ssize_t bytes_written = write(file_descriptor, data, std::min(size, BLOCK_SIZE));
    if (bytes_written == -1)
    {
        throw std::runtime_error("Error al escribir en el archivo");
    }
}

void BlockManager::readBlock(size_t block_index, void *buffer, size_t size)
{
    if (block_index >= total_blocks || !block_map[block_index])
    {
        throw std::runtime_error("Índice de bloque inválido o no asignado");
    }

    if (lseek(file_descriptor, block_index * BLOCK_SIZE, SEEK_SET) == -1)
    {
        throw std::runtime_error("Error al posicionar el puntero en el archivo");
    }

    ssize_t bytes_read = read(file_descriptor, buffer, std::min(size, BLOCK_SIZE));
    if (bytes_read == -1)
    {
        throw std::runtime_error("Error al leer el archivo");
    }
}

void BlockManager::sync()
{
    if (fsync(file_descriptor) != 0)
    {
        throw std::runtime_error("Error al sincronizar los bloques en disco");
    }
}
