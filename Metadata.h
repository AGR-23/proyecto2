#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <memory>

// Estructura para almacenar información de cada versión
struct VersionInfo {
    size_t version_id;
    time_t timestamp;
    std::vector<size_t> block_list;  // Lista de bloques que conforman esta versión completa
    std::vector<size_t> modified_blocks;  // Bloques modificados en esta versión respecto a la anterior
    size_t parent_version;  // Versión desde la que derivó
};

class Metadata {
public:
    Metadata(const std::string& name = "", size_t size = 0, const std::string& type = "");
    
    // Añadir una nueva versión con su lista de bloques y bloques modificados
    void addVersion(size_t version_id, const std::vector<size_t>& block_list, 
                    const std::vector<size_t>& modified_blocks, size_t parent_version = 0);
    
    // Obtener información de una versión específica
    const VersionInfo* getVersion(size_t version_id) const;
    
    // Actualizar el tamaño del archivo
    void updateFileSize(size_t new_size);
    
    // Imprimir todos los metadatos
    void printMetadata() const;
    
    // Obtener versión más reciente
    size_t getLatestVersion() const;
    
    // Serializar metadatos para guardarlos en disco
    std::vector<char> serialize() const;
    
    // Deserializar metadatos desde datos leídos del disco
    static Metadata deserialize(const std::vector<char>& data);
    
    // Getters
    const std::string& getFileName() const { return file_name; }
    size_t getFileSize() const { return file_size; }
    const std::string& getFileType() const { return file_type; }
    const std::unordered_map<size_t, VersionInfo>& getVersionHistory() const { return version_history; }
    
private:
    std::string file_name;
    size_t file_size;
    std::string file_type;
    std::unordered_map<size_t, VersionInfo> version_history;
};