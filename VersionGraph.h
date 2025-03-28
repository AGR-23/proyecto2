#ifndef VERSION_GRAPH_H
#define VERSION_GRAPH_H

#include <unordered_map>
#include <vector>
#include <cstddef>
#include <string>  // Asegurar que string está incluido

class VersionGraph {
private:
    struct VersionNode {
        size_t version_id;
        std::vector<size_t> modified_blocks;
        std::vector<size_t> parent_versions;
    };

    std::unordered_map<size_t, VersionNode> graph;
    size_t current_version;
    std::unordered_map<std::string, std::unordered_map<size_t, std::vector<char>>> versiones;  // <--- Agregar esta línea

public:
    VersionGraph();

    void addVersion(size_t version_id, const std::vector<size_t>& modified_blocks, size_t parent_version);
    const VersionNode* getVersion(size_t version_id) const;
    bool restoreVersion(const std::string& file_name, size_t version_id, std::vector<char>& restored_data);
    void rollbackToVersion(size_t version_id);
    size_t getCurrentVersion() const;
};

#endif