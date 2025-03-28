#include "VersionGraph.h"
#include <iostream>

VersionGraph::VersionGraph() {
    current_version = 0;  // Primera versión
    graph[current_version] = {0, {}, {}};  // Versión inicial sin bloques modificados
}

void VersionGraph::addVersion(size_t version_id, const std::vector<size_t>& modified_blocks, size_t parent_version) {
    if (graph.find(parent_version) == graph.end()) {
        std::cerr << "Error: La versión padre no existe\n";
        return;
    }

    graph[version_id] = {version_id, modified_blocks, {parent_version}};
    current_version = version_id;
}

const VersionGraph::VersionNode* VersionGraph::getVersion(size_t version_id) const {
    auto it = graph.find(version_id);
    return (it != graph.end()) ? &it->second : nullptr;
}

void VersionGraph::rollbackToVersion(size_t version_id) {
    if (graph.find(version_id) != graph.end()) {
        current_version = version_id;
    } else {
        std::cerr << "Error: La versión solicitada no existe\n";
    }
}

bool VersionGraph::restoreVersion(const std::string& file_name, size_t version_id, std::vector<char>& restored_data) {
    if (versiones.find(file_name) == versiones.end()) {
        std::cerr << "Error: El archivo no tiene versiones almacenadas\n";
        return false;
    }
    
    auto& versionesArchivo = versiones[file_name];
    if (versionesArchivo.find(version_id) == versionesArchivo.end()) {
        std::cerr << "Error: La versión solicitada no existe\n";
        return false;
    }
    
    restored_data = versionesArchivo[version_id]; // Recuperar datos de la versión
    return true;
}


size_t VersionGraph::getCurrentVersion() const {
    return current_version;
} 