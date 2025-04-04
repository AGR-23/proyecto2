#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include "BlockManager.h"
#include "VersionGraph.h"

class FileSystem
{
public:
    // Constructor con ruta de almacenamiento y tamaño (en MB)
    FileSystem(const std::string &storage_path, size_t storage_size_mb = 100);
    ~FileSystem();

    // Crear un nuevo archivo
    bool create(const std::string &file_name, const std::string &file_type);

    // Escribir datos en un archivo (implementando COW)
    bool write(const std::string &file_name, size_t offset, const std::vector<char> &data);

    // Leer el contenido completo de un archivo
    std::vector<char> read(const std::string &file_name);

    // Abrir un archivo
    bool open(const std::string &filename);

    // Cerrar un archivo
    bool close(const std::string &filename);

    // Restaurar un archivo a    una versión anterior
    bool rollbackFile(const std::string &file_name, size_t version_id);

    // Mostrar metadatos de un archivo
    void printFileMetadata(const std::string &file_name);

    // Listar todos los archivos en el sistema
    void listFiles();

    // Obtener el número de la versión actual de un archivo
    size_t getCurrentVersion(const std::string &file_name);

    // Sincronizar todos los cambios a disco
    void sync();

    // Para depuración: inspeccionar contenido real de bloques
    void inspectBlocks(const std::string &file_name);

    // Función para mostrar estadísticas de memoria
    struct GlobalMemoryUsage
    {
        BlockManager::MemoryUsage blocks;
        VersionGraph::VersionMemoryUsage versions;
        size_t total_memory_approx() const
        {
            return blocks.used_bytes + versions.metadata_size_approx;
        }
    };

    GlobalMemoryUsage getMemoryUsage() const;
    void printMemoryUsage() const; // Versión amigable para consola

private:
    // Mapa de archivos abiertos: {nombre_archivo -> metadata + estado}
    std::unordered_map<std::string, std::pair<Metadata, bool>> open_files;

    // Helpers (privados)
    bool isOpen(const std::string &filename) const;

    std::string storage_path;   // Ruta del archivo de almacenamiento
    std::string metadata_dir;   // Directorio para metadatos
    size_t block_size;          // Tamaño de bloque (constante)
    BlockManager block_manager; // Gestor de bloques
    VersionGraph version_graph; // Grafo de versiones

    // Método auxiliar para dividir datos en bloques
    std::vector<std::pair<size_t, std::vector<char>>> splitIntoBlocks(const std::vector<char> &data);

    // Método para calcular qué bloques se modificaron entre dos versiones
    std::vector<size_t> calculateModifiedBlocks(const std::vector<char> &oldData,
                                                const std::vector<char> &newData);
};