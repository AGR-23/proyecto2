#include "VersionGraph.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <algorithm>

namespace fs = std::filesystem;

VersionGraph::VersionGraph(BlockManager &bm) : block_manager(bm)
{
}

void VersionGraph::addVersion(const std::string &file_name, size_t version_id,
                              const std::vector<size_t> &block_list,
                              const std::vector<size_t> &modified_blocks,
                              size_t parent_version)
{

    // Si el archivo no existe en el sistema, crear sus metadatos
    if (files_metadata.find(file_name) == files_metadata.end())
    {
        files_metadata[file_name] = Metadata(file_name, 0, "");
    }

    // Añadir la versión a los metadatos del archivo
    files_metadata[file_name].addVersion(version_id, block_list, modified_blocks, parent_version);

    // Actualizar la versión actual del archivo
    current_versions[file_name] = version_id;
}

const VersionInfo *VersionGraph::getVersion(const std::string &file_name, size_t version_id) const
{
    auto it = files_metadata.find(file_name);
    if (it == files_metadata.end())
    {
        std::cerr << "Error: El archivo no existe en el sistema\n";
        return nullptr;
    }

    return it->second.getVersion(version_id);
}

bool VersionGraph::restoreVersion(const std::string &file_name, size_t version_id, std::vector<char> &restored_data)
{
    auto it = files_metadata.find(file_name);
    if (it == files_metadata.end())
    {
        std::cerr << "Error: El archivo no existe en el sistema\n";
        return false;
    }

    const VersionInfo *version_info = it->second.getVersion(version_id);
    if (!version_info)
    {
        std::cerr << "Error: La versión solicitada no existe\n";
        return false;
    }

    // Reconstruir el archivo a partir de los bloques de esta versión
    restored_data.clear();

    // Calcular tamaño REAL basado en bloques
    size_t total_size = version_info->block_list.size() * BLOCK_SIZE;
    restored_data.reserve(total_size);

    // Leer todos los bloques completos
    for (size_t block_index : version_info->block_list)
    {
        std::vector<char> block_data(BLOCK_SIZE);
        block_manager.readBlock(block_index, block_data.data(), BLOCK_SIZE);
        restored_data.insert(restored_data.end(), block_data.begin(), block_data.end());
    }

    // Ajustar al tamaño real del contenido (hasta el último byte no nulo)
    size_t actual_size = restored_data.size();
    while (actual_size > 0 && restored_data[actual_size - 1] == '\0')
    {
        actual_size--;
    }
    restored_data.resize(actual_size);

    // Actualizar la versión actual del archivo
    current_versions[file_name] = version_id;

    std::cout << "Archivo restaurado a la versión " << version_id << std::endl;
    return true;
}

void VersionGraph::collectGarbage()
{
    std::unordered_set<size_t> used_blocks;

    // Paso 1: Bloques USADOS (versiones accesibles)
    for (const auto &[file_name, metadata] : files_metadata)
    {
        size_t current_ver = current_versions[file_name];
        const VersionInfo *version = metadata.getVersion(current_ver);

        // Recorrer desde la versión actual hasta la raíz (solo historial accesible)
        while (version != nullptr)
        {
            for (size_t block : version->block_list)
            {
                used_blocks.insert(block);
            }
            version = metadata.getVersion(version->parent_version);
        }
    }

    // Paso 2: Liberar bloques huérfanos + bloques de archivos eliminados
    size_t freed_blocks = 0;
    for (size_t block = 0; block < block_manager.getTotalBlocks(); ++block)
    {
        if (block_manager.isBlockUsed(block) && used_blocks.find(block) == used_blocks.end())
        {
            block_manager.freeBlock(block);
            freed_blocks++;
        }
    }

    std::cout << "GC liberó " << freed_blocks << " bloques (huérfanos o de archivos eliminados)\n";
}

size_t VersionGraph::getCurrentVersion(const std::string &file_name) const
{
    auto it = current_versions.find(file_name);
    return (it != current_versions.end()) ? it->second : 0;
}

bool VersionGraph::saveMetadata(const std::string &metadata_dir)
{
    try
    {
        // Crear directorio de metadatos si no existe
        if (!fs::exists(metadata_dir))
        {
            fs::create_directories(metadata_dir);
        }

        // Guardar metadatos de cada archivo
        for (const auto &[file_name, metadata] : files_metadata)
        {
            std::string file_path = metadata_dir + "/" + file_name + ".meta";
            std::vector<char> serialized_data = metadata.serialize();

            std::ofstream file(file_path, std::ios::binary | std::ios::trunc);
            if (!file)
            {
                std::cerr << "Error: No se pudieron guardar los metadatos de " << file_name << std::endl;
                continue;
            }

            file.write(serialized_data.data(), serialized_data.size());
            file.close();
        }

        // Guardar versiones actuales
        std::string versions_path = metadata_dir + "/current_versions.meta";
        std::ofstream versions_file(versions_path, std::ios::binary | std::ios::trunc);

        // Formato: número de archivos + (tamaño de nombre + nombre + versión actual) para cada archivo
        size_t file_count = current_versions.size();
        versions_file.write(reinterpret_cast<char *>(&file_count), sizeof(size_t));

        for (const auto &[file_name, version] : current_versions)
        {
            size_t name_len = file_name.size();
            versions_file.write(reinterpret_cast<char *>(&name_len), sizeof(size_t));
            versions_file.write(file_name.data(), name_len);
            versions_file.write(reinterpret_cast<const char *>(&version), sizeof(size_t));
        }

        versions_file.close();
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error al guardar metadatos: " << e.what() << std::endl;
        return false;
    }
}

bool VersionGraph::loadMetadata(const std::string &metadata_dir)
{
    try
    {
        // Verificar si existe el directorio
        if (!fs::exists(metadata_dir))
        {
            std::cerr << "Error: Directorio de metadatos no encontrado\n";
            return false;
        }

        // Limpiar colecciones existentes
        files_metadata.clear();
        current_versions.clear();

        // Cargar metadatos de cada archivo
        for (const auto &entry : fs::directory_iterator(metadata_dir))
        {
            std::string path = entry.path().string();

            // Solo procesar archivos de metadatos de archivo (no el de versiones actuales)
            // Reemplazar ends_with con una función compatible con C++17
            if (path.size() >= 5 && path.substr(path.size() - 5) == ".meta" &&
                (path.size() < 21 || path.substr(path.size() - 21) != "current_versions.meta"))
            {
                std::string file_name = entry.path().filename().string();
                file_name = file_name.substr(0, file_name.size() - 5); // Quitar extensión .meta

                std::ifstream file(path, std::ios::binary);
                if (!file)
                {
                    std::cerr << "Error: No se pudieron leer los metadatos de " << file_name << std::endl;
                    continue;
                }

                // Leer todo el archivo
                std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                file.close();

                // Deserializar los metadatos
                files_metadata[file_name] = Metadata::deserialize(data);
            }
        }

        // Cargar versiones actuales
        std::string versions_path = metadata_dir + "/current_versions.meta";
        if (fs::exists(versions_path))
        {
            std::ifstream versions_file(versions_path, std::ios::binary);
            if (versions_file)
            {
                // Leer número de archivos
                size_t file_count;
                versions_file.read(reinterpret_cast<char *>(&file_count), sizeof(size_t));

                // Leer versión actual de cada archivo
                for (size_t i = 0; i < file_count; i++)
                {
                    // Leer nombre del archivo
                    size_t name_len;
                    versions_file.read(reinterpret_cast<char *>(&name_len), sizeof(size_t));

                    std::string file_name(name_len, '\0');
                    versions_file.read(&file_name[0], name_len);

                    // Leer versión actual
                    size_t version;
                    versions_file.read(reinterpret_cast<char *>(&version), sizeof(size_t));

                    current_versions[file_name] = version;
                }

                versions_file.close();
            }
        }

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error al cargar metadatos: " << e.what() << std::endl;
        return false;
    }
}

const Metadata *VersionGraph::getFileMetadata(const std::string &file_name) const
{
    auto it = files_metadata.find(file_name);
    return (it != files_metadata.end()) ? &it->second : nullptr;
}

bool VersionGraph::fileExists(const std::string &file_name) const
{
    return files_metadata.find(file_name) != files_metadata.end();
}

void VersionGraph::updateFileSize(const std::string &file_name, size_t new_size)
{
    auto it = files_metadata.find(file_name);
    if (it != files_metadata.end())
    {
        it->second.updateFileSize(new_size);
    }
}

VersionGraph::VersionMemoryUsage VersionGraph::getVersionMemoryUsage() const {
    VersionMemoryUsage usage;
    usage.total_files = files_metadata.size();
    
    size_t total_versions = 0;
    for (const auto& [_, meta] : files_metadata) {
        total_versions += meta.getVersionHistory().size();
    }
    usage.total_versions = total_versions;
    
    usage.avg_versions_per_file = usage.total_files > 0 
                                ? total_versions / usage.total_files 
                                : 0;
    
    // Estimación: 100 bytes por versión (ajustable)
    usage.metadata_size_approx = total_versions * 100; 
    
    return usage;
}