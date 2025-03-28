#include "Metadata.h"
#include <iostream>

Metadata::Metadata(const std::string& name, size_t size, const std::string& type)
    : file_name(name), file_size(size), file_type(type) {}

void Metadata::addVersion(size_t version_id, const std::vector<size_t>& modified_blocks) {
    VersionInfo version;
    version.version_id = version_id;
    version.timestamp = std::time(nullptr);
    version.modified_blocks = modified_blocks;
    
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

void Metadata::addBlock(size_t block_index) {
    block_list.push_back(block_index);
}

const std::vector<size_t>& Metadata::getBlocks() const {
    return block_list;
}

void Metadata::printMetadata() const {
    std::cout << "Archivo: " << file_name << "\n"
              << "Tamaño: " << file_size << " bytes\n"
              << "Tipo: " << file_type << "\n"
              << "Bloques usados: ";
    
    for (size_t block : block_list) {
        std::cout << block << " ";
    }
    std::cout << "\nHistorial de versiones:\n";
    
    for (const auto& [id, version] : version_history) {
        std::cout << "  Versión " << id << " - Timestamp: " << version.timestamp << "\n";
    }
}