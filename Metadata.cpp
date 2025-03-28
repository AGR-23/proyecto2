#include "Metadata.h"
#include <iostream>
#include <algorithm>
#include <cstring>

Metadata::Metadata(const std::string& name, size_t size, const std::string& type)
    : file_name(name), file_size(size), file_type(type) {}

void Metadata::addVersion(size_t version_id, const std::vector<size_t>& block_list, 
                          const std::vector<size_t>& modified_blocks, size_t parent_version) {
    VersionInfo version;
    version.version_id = version_id;
    version.timestamp = std::time(nullptr);
    version.block_list = block_list;
    version.modified_blocks = modified_blocks;
    version.parent_version = parent_version;
    
    version_history[version_id] = version;
}

const VersionInfo* Metadata::getVersion(size_t version_id) const {
    auto it = version_history.find(version_id);
    if (it != version_history.end()) {
        return &it->second;
    }
    return nullptr;
}

void Metadata::updateFileSize(size_t new_size) {
    file_size = new_size;
}

void Metadata::printMetadata() const {
    std::cout << "Archivo: " << file_name << "\n"
              << "Tamaño: " << file_size << " bytes\n"
              << "Tipo: " << file_type << "\n";
    
    std::cout << "\nHistorial de versiones:\n";
    
    for (const auto& [id, version] : version_history) {
        char time_str[100];
        struct tm *timeinfo = localtime(&version.timestamp);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
        
        std::cout << "  Versión " << id << " - Fecha: " << time_str << "\n";
        std::cout << "    Bloques totales: " << version.block_list.size() << "\n";
        
        std::cout << "    Bloques modificados: ";
        for (size_t block : version.modified_blocks) {
            std::cout << block << " ";
        }
        std::cout << "\n";
        
        if (version.parent_version > 0) {
            std::cout << "    Deriva de versión: " << version.parent_version << "\n";
        }
        std::cout << "\n";
    }
}

size_t Metadata::getLatestVersion() const {
    size_t latest_version = 0;
    for (const auto& [id, _] : version_history) {
        if (id > latest_version) {
            latest_version = id;
        }
    }
    return latest_version;
}

// Serializar metadatos para guardarlos en disco
std::vector<char> Metadata::serialize() const {
    std::vector<char> result;
    
    // Tamaño del nombre del archivo
    size_t name_len = file_name.size();
    result.insert(result.end(), reinterpret_cast<char*>(&name_len), reinterpret_cast<char*>(&name_len) + sizeof(size_t));
    
    // Nombre del archivo
    result.insert(result.end(), file_name.begin(), file_name.end());
    
    // Tamaño del archivo
    result.insert(result.end(), reinterpret_cast<const char*>(&file_size), reinterpret_cast<const char*>(&file_size) + sizeof(size_t));
    
    // Tamaño del tipo de archivo
    size_t type_len = file_type.size();
    result.insert(result.end(), reinterpret_cast<char*>(&type_len), reinterpret_cast<char*>(&type_len) + sizeof(size_t));
    
    // Tipo de archivo
    result.insert(result.end(), file_type.begin(), file_type.end());
    
    // Número de versiones
    size_t version_count = version_history.size();
    result.insert(result.end(), reinterpret_cast<char*>(&version_count), reinterpret_cast<char*>(&version_count) + sizeof(size_t));
    
    // Datos de cada versión
    for (const auto& [id, version] : version_history) {
        // ID de versión
        result.insert(result.end(), reinterpret_cast<const char*>(&id), reinterpret_cast<const char*>(&id) + sizeof(size_t));
        
        // Timestamp
        result.insert(result.end(), reinterpret_cast<const char*>(&version.timestamp), 
                    reinterpret_cast<const char*>(&version.timestamp) + sizeof(time_t));
        
        // Versión padre
        result.insert(result.end(), reinterpret_cast<const char*>(&version.parent_version), 
                    reinterpret_cast<const char*>(&version.parent_version) + sizeof(size_t));
        
        // Número de bloques totales
        size_t block_count = version.block_list.size();
        result.insert(result.end(), reinterpret_cast<char*>(&block_count), 
                    reinterpret_cast<char*>(&block_count) + sizeof(size_t));
        
        // Lista de bloques
        for (size_t block : version.block_list) {
            result.insert(result.end(), reinterpret_cast<const char*>(&block), 
                        reinterpret_cast<const char*>(&block) + sizeof(size_t));
        }
        
        // Número de bloques modificados
        size_t mod_block_count = version.modified_blocks.size();
        result.insert(result.end(), reinterpret_cast<char*>(&mod_block_count), 
                    reinterpret_cast<char*>(&mod_block_count) + sizeof(size_t));
        
        // Lista de bloques modificados
        for (size_t block : version.modified_blocks) {
            result.insert(result.end(), reinterpret_cast<const char*>(&block), 
                        reinterpret_cast<const char*>(&block) + sizeof(size_t));
        }
    }
    
    return result;
}

// Deserializar metadatos desde datos leídos del disco
Metadata Metadata::deserialize(const std::vector<char>& data) {
    if (data.empty()) {
        return Metadata();
    }
    
    Metadata metadata;
    size_t pos = 0;
    
    // Leer tamaño del nombre del archivo
    size_t name_len;
    std::memcpy(&name_len, &data[pos], sizeof(size_t));
    pos += sizeof(size_t);
    
    // Leer nombre del archivo
    metadata.file_name.assign(data.begin() + pos, data.begin() + pos + name_len);
    pos += name_len;
    
    // Leer tamaño del archivo
    std::memcpy(&metadata.file_size, &data[pos], sizeof(size_t));
    pos += sizeof(size_t);
    
    // Leer tamaño del tipo de archivo
    size_t type_len;
    std::memcpy(&type_len, &data[pos], sizeof(size_t));
    pos += sizeof(size_t);
    
    // Leer tipo de archivo
    metadata.file_type.assign(data.begin() + pos, data.begin() + pos + type_len);
    pos += type_len;
    
    // Leer número de versiones
    size_t version_count;
    std::memcpy(&version_count, &data[pos], sizeof(size_t));
    pos += sizeof(size_t);
    
    // Leer cada versión
    for (size_t i = 0; i < version_count; i++) {
        VersionInfo version;
        
        // Leer ID de versión
        size_t version_id;
        std::memcpy(&version_id, &data[pos], sizeof(size_t));
        pos += sizeof(size_t);
        version.version_id = version_id;
        
        // Leer timestamp
        std::memcpy(&version.timestamp, &data[pos], sizeof(time_t));
        pos += sizeof(time_t);
        
        // Leer versión padre
        std::memcpy(&version.parent_version, &data[pos], sizeof(size_t));
        pos += sizeof(size_t);
        
        // Leer número de bloques totales
        size_t block_count;
        std::memcpy(&block_count, &data[pos], sizeof(size_t));
        pos += sizeof(size_t);
        
        // Leer lista de bloques
        version.block_list.resize(block_count);
        for (size_t j = 0; j < block_count; j++) {
            size_t block;
            std::memcpy(&block, &data[pos], sizeof(size_t));
            pos += sizeof(size_t);
            version.block_list[j] = block;
        }
        
        // Leer número de bloques modificados
        size_t mod_block_count;
        std::memcpy(&mod_block_count, &data[pos], sizeof(size_t));
        pos += sizeof(size_t);
        
        // Leer lista de bloques modificados
        version.modified_blocks.resize(mod_block_count);
        for (size_t j = 0; j < mod_block_count; j++) {
            size_t block;
            std::memcpy(&block, &data[pos], sizeof(size_t));
            pos += sizeof(size_t);
            version.modified_blocks[j] = block;
        }
        
        metadata.version_history[version_id] = version;
    }
    
    return metadata;
}