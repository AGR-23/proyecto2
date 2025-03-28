#pragma once
#include <unordered_map>
#include <string>
#include <cstdlib>

// Tamaño fijo de los bloques (4 KB por defecto)
const size_t BLOCK_SIZE = 4096;

class BlockManager {
public:
    // Constructor que recibe ruta del archivo y tamaño total
    BlockManager(const char* file_path, size_t total_size);
    ~BlockManager();
    
    // Reservar un bloque libre y marcarlo como usado
    size_t allocateBlock();
    
    // Liberar un bloque previamente asignado
    void freeBlock(size_t block_index);
    
    // Escribir datos en un bloque específico
    void writeBlock(size_t block_index, const void* data, size_t size);
    
    // Leer datos desde un bloque específico
    void readBlock(size_t block_index, void* buffer, size_t size);
    
    // Sincronizar cambios a disco
    void sync();
    
    // Obtener número total de bloques
    size_t getTotalBlocks() const;
    
    // Verificar si un bloque está en uso
    bool isBlockUsed(size_t block_index) const;
    
private:
    std::string data_file_path;      // Ruta del archivo de datos
    std::string metadata_file_path;  // Ruta del archivo de metadatos
    int file_descriptor;             // Descriptor del archivo
    size_t total_blocks;             // Número total de bloques
    std::unordered_map<size_t, bool> block_map;  // Mapa de bloques (índice, usado)
    
    // Cargar mapa de bloques desde archivo de metadatos
    void loadBlockMap();
    
    // Guardar mapa de bloques en archivo de metadatos
    void saveBlockMap();
};