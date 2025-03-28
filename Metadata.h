#ifndef METADATA_H
#define METADATA_H

#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>

struct VersionInfo {
    size_t version_id;
    std::time_t timestamp;
    std::vector<size_t> modified_blocks; // Bloques modificados en esta versión
};

class Metadata {
    public:
        std::string file_name;
        size_t file_size;
        std::string file_type;
        std::vector<size_t> block_list; // Bloques actuales del archivo
        std::unordered_map<size_t, VersionInfo> version_history; // Historial de versiones
        Metadata() : file_name(""), file_size(0), file_type("") {}  // Constructor por defecto
        Metadata(const std::string& name, size_t size, const std::string& type);
        size_t getCurrentVersion() const;
        void addVersion(size_t version_id, const std::vector<size_t>& modified_blocks);
        const VersionInfo* getVersion(size_t version_id) const;
    
        void updateFileSize(size_t new_size);
        void addBlock(size_t block_index);
        const std::vector<size_t>& getBlocks() const;
    
        void printMetadata() const;
    };    

#endif