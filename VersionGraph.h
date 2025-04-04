#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "BlockManager.h"
#include "Metadata.h"

class VersionGraph {
public:
    VersionGraph(BlockManager& block_manager);
    
    // Añadir una nueva versión de un archivo
    void addVersion(const std::string& file_name, size_t version_id, 
                    const std::vector<size_t>& block_list,
                    const std::vector<size_t>& modified_blocks,
                    size_t parent_version = 0);
    
    // Obtener información de una versión
    const VersionInfo* getVersion(const std::string& file_name, size_t version_id) const;
    
    // Restaurar una versión específica de un archivo
    bool restoreVersion(const std::string& file_name, size_t version_id, std::vector<char>& restored_data);
    
    // Obtener versión actual de un archivo
    size_t getCurrentVersion(const std::string& file_name) const;
    
    // Guardar todos los metadatos de versiones en disco
    bool saveMetadata(const std::string& metadata_dir);
    
    // Cargar todos los metadatos de versiones desde disco
    bool loadMetadata(const std::string& metadata_dir);
    
    // Obtener metadatos de un archivo
    const Metadata* getFileMetadata(const std::string& file_name) const;
    
    // Verificar si un archivo existe en el sistema
    bool fileExists(const std::string& file_name) const;
    
    // Actualizar el tamaño de un archivo en sus metadatos
    void updateFileSize(const std::string& file_name, size_t new_size);

    // Eliminar bloques no referenciados por ninguna versión
    void collectGarbage();

    // Función para mostrar estadísticas de memoria
    struct VersionMemoryUsage {
        size_t total_files;
        size_t total_versions;
        size_t avg_versions_per_file;
        size_t metadata_size_approx; // Tamaño estimado de metadatos
    };
    
    VersionMemoryUsage getVersionMemoryUsage() const;
    
private:
    BlockManager& block_manager;
    std::unordered_map<std::string, Metadata> files_metadata;
    std::unordered_map<std::string, size_t> current_versions;
};