#include "FileSystem.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <iomanip> // Para std::setw, std::setfill, etc.

namespace fs = std::filesystem;

FileSystem::FileSystem(const std::string &path, size_t storage_size_mb)
    : storage_path(path),
      metadata_dir(path + "_metadata"),
      block_size(BLOCK_SIZE),
      block_manager(path.c_str(), storage_size_mb * 1024 * 1024),
      version_graph(block_manager)
{
    // Crear directorio de metadatos si no existe
    if (!fs::exists(metadata_dir))
    {
        fs::create_directories(metadata_dir);
    }

    // Cargar metadatos existentes
    version_graph.loadMetadata(metadata_dir);
}

FileSystem::~FileSystem()
{
    // Asegurar que todos los cambios se guarden
    sync();
}

// Función para estadisticas de memoria 
FileSystem::GlobalMemoryUsage FileSystem::getMemoryUsage() const {
    GlobalMemoryUsage usage;
    usage.blocks = block_manager.getMemoryUsage();
    usage.versions = version_graph.getVersionMemoryUsage();
    return usage;
}

void FileSystem::printMemoryUsage() const {
    auto usage = getMemoryUsage();
    
    std::cout << "\n=== Uso de Memoria del Sistema ===\n";
    std::cout << "Bloques físicos:\n"
              << "  Usados: " << usage.blocks.used_blocks << "/" << usage.blocks.total_blocks 
              << " bloques (" << (usage.blocks.used_bytes / 1024) << " KB)\n"
              << "  Libres: " << usage.blocks.free_blocks << " bloques\n";
    
    std::cout << "\nVersiones lógicas:\n"
              << "  Archivos: " << usage.versions.total_files << "\n"
              << "  Versiones totales: " << usage.versions.total_versions << "\n"
              << "  Metadatos aprox.: " << (usage.versions.metadata_size_approx / 1024) << " KB\n";
    
    std::cout << "\nTotal estimado: " << (usage.total_memory_approx() / 1024) << " KB\n";
}

bool FileSystem::create(const std::string &file_name, const std::string &file_type)
{
    if (version_graph.fileExists(file_name))
    {
        std::cerr << "Error: El archivo '" << file_name << "' ya existe.\n";
        return false;
    }

    // Crear primera versión (vacía)
    std::vector<size_t> empty_blocks;   // Sin bloques
    std::vector<size_t> empty_modified; // Sin bloques modificados

    version_graph.addVersion(file_name, 1, empty_blocks, empty_modified, 0);

    // Actualizar metadatos
    const Metadata *metadata = version_graph.getFileMetadata(file_name);
    if (metadata)
    {
        // Los metadatos ya se habrán creado en addVersion
        std::cout << "Archivo '" << file_name << "' creado correctamente.\n";
        return true;
    }

    std::cerr << "Error al crear el archivo '" << file_name << "'.\n";
    return false;
}

bool FileSystem::open(const std::string &filename)
{
    // 1. Validar que existe y no está ya abierto
    if (!version_graph.fileExists(filename) || isOpen(filename))
    {
        return false;
    }

    // 2. Obtener metadata y marcarlo como abierto
    const Metadata *meta = version_graph.getFileMetadata(filename);
    if (!meta)
        return false;

    open_files[filename] = {*meta, true};
    return true;
}

bool FileSystem::close(const std::string &filename)
{
    // 1. Validar que está abierto
    if (!isOpen(filename))
        return false;

    // 2. Sincronizar cambios (opcional)
    sync();

    // 3. Eliminar de archivos abiertos
    open_files.erase(filename);
    return true;
}

bool FileSystem::isOpen(const std::string &filename) const
{
    return open_files.find(filename) != open_files.end();
}

bool FileSystem::write(const std::string &file_name, size_t offset, const std::vector<char> &data)
{
    if (!version_graph.fileExists(file_name))
    {
        std::cerr << "Error: El archivo '" << file_name << "' no existe.\n";
        return false;
    }
    if (!isOpen(file_name))
    {
        std::cerr << "Error: El archivo '" << file_name << "' no está abierto.\n";
        return false;
    }

    // Leer contenido actual
    std::vector<char> current_data = read(file_name);

    // Crear copia del contenido actual para modificar
    std::vector<char> new_data;

    if (offset <= current_data.size())
    {
        // Caso normal: offset dentro del tamaño actual
        new_data.assign(current_data.begin(), current_data.begin() + offset);
        new_data.insert(new_data.end(), data.begin(), data.end());
        if (offset + data.size() < current_data.size())
        {
            new_data.insert(new_data.end(),
                            current_data.begin() + offset + data.size(),
                            current_data.end());
        }
    }
    else
    {
        // Caso especial: offset más allá del tamaño actual
        new_data = current_data;
        // Rellenar con espacios (podría ser '\0' u otro carácter)
        new_data.resize(offset, ' '); // Cambiado de '\0' a ' ' para mejor visualización
        new_data.insert(new_data.end(), data.begin(), data.end());
    }
    // implementacion de copy on write

    // 1. Calcular qué bloques se modificaron
    std::vector<size_t> modified_blocks = calculateModifiedBlocks(current_data, new_data);

    // 2. Dividir datos nuevos en bloques
    std::vector<std::pair<size_t, std::vector<char>>> new_blocks = splitIntoBlocks(new_data);

    // 3. Obtener bloques de la versión actual
    size_t current_version = version_graph.getCurrentVersion(file_name);
    const VersionInfo *current_version_info = version_graph.getVersion(file_name, current_version);

    if (!current_version_info)
    {
        std::cerr << "Error: No se pudo obtener información de la versión actual.\n";
        return false;
    }

    // 4. Crear lista de bloques para la nueva versión
    std::vector<size_t> new_version_blocks;

    // Para cada bloque lógico de datos
    for (size_t i = 0; i < new_blocks.size(); i++)
    {
        // Verificar si este bloque lógico se modificó
        if (std::find(modified_blocks.begin(), modified_blocks.end(), i) != modified_blocks.end())
        {
            // Bloque modificado: asignar un nuevo bloque físico
            size_t new_block_index = block_manager.allocateBlock();
            if (new_block_index == static_cast<size_t>(-1))
            {
                std::cerr << "Error: No hay espacio disponible para asignar nuevos bloques.\n";
                return false;
            }

            // Escribir datos en el nuevo bloque
            block_manager.writeBlock(new_block_index, new_blocks[i].second.data(), new_blocks[i].second.size());
            new_version_blocks.push_back(new_block_index);
        }
        else
        {
            // Bloque no modificado: reutilizar el bloque de la versión anterior
            if (i < current_version_info->block_list.size())
            {
                new_version_blocks.push_back(current_version_info->block_list[i]);
            }
            else
            {
                // Este caso no debería ocurrir, pero lo manejamos por si acaso
                std::cerr << "Advertencia: Inconsistencia en bloques no modificados.\n";
                size_t new_block_index = block_manager.allocateBlock();
                block_manager.writeBlock(new_block_index, new_blocks[i].second.data(), new_blocks[i].second.size());
                new_version_blocks.push_back(new_block_index);
            }
        }
    }

    // 5. Crear nueva versión
    size_t new_version = current_version + 1;
    version_graph.addVersion(file_name, new_version, new_version_blocks, modified_blocks, current_version);

    // Actualizar metadatos
    // Ya tenemos la referencia a los metadatos, no necesitamos obtenerlos de nuevo
    std::cout << "Archivo '" << file_name << "' modificado correctamente (versión " << new_version << ").\n";
    return true;
}

std::vector<char> FileSystem::read(const std::string &file_name)
{
    if (!isOpen(file_name))
    {
        std::cerr << "Error: El archivo '" << file_name << "' no está abierto.\n";
        return {};
    }

    if (!version_graph.fileExists(file_name))
    {
        std::cerr << "Error: El archivo '" << file_name << "' no existe.\n";
        return {};
    }

    // Obtener versión actual
    size_t current_version = version_graph.getCurrentVersion(file_name);

    // Restaurar versión actual
    std::vector<char> file_data;
    if (!version_graph.restoreVersion(file_name, current_version, file_data))
    {
        std::cerr << "Error: No se pudo leer el archivo '" << file_name << "'.\n";
        return {};
    }

    // Eliminar nulls al final para visualización
    while (!file_data.empty() && file_data.back() == '\0')
    {
        file_data.pop_back();
    }

    return file_data;
}

bool FileSystem::rollbackFile(const std::string &file_name, size_t version_id)
{
    if (!version_graph.fileExists(file_name))
    {
        std::cerr << "Error: El archivo '" << file_name << "' no existe.\n";
        return false;
    }

    // Verificar si la versión existe
    const VersionInfo *version_info = version_graph.getVersion(file_name, version_id);
    if (!version_info)
    {
        std::cerr << "Error: La versión " << version_id << " no existe para el archivo '" << file_name << "'.\n";
        return false;
    }

    // Restaurar la versión
    std::vector<char> restored_data;
    if (!version_graph.restoreVersion(file_name, version_id, restored_data))
    {
        std::cerr << "Error durante el rollback.\n";
        return false;
    }
    // No es necesario restaurar los datos, solo actualizar la versión actual
    std::vector<char> dummy;
    if (!version_graph.restoreVersion(file_name, version_id, dummy))
    {
        std::cerr << "Error durante el rollback.\n";
        return false;
    }

    std::cout << "Archivo '" << file_name << "' restaurado a la versión " << version_id << ".\n";
    return true;
}

void FileSystem::printFileMetadata(const std::string &file_name)
{
    const Metadata *metadata = version_graph.getFileMetadata(file_name);
    if (!metadata)
    {
        std::cerr << "Error: El archivo '" << file_name << "' no existe.\n";
        return;
    }

    metadata->printMetadata();
}

void FileSystem::listFiles()
{
    std::cout << "Archivos en el sistema:\n";

    bool no_files = true;
    for (const auto &entry : fs::directory_iterator(metadata_dir))
    {
        std::string path = entry.path().string();
        // Reemplazar ends_with con una función compatible con C++17
        if (path.size() >= 5 && path.substr(path.size() - 5) == ".meta" &&
            (path.size() < 21 || path.substr(path.size() - 21) != "current_versions.meta"))
        {
            std::string file_name = entry.path().filename().string();
            file_name = file_name.substr(0, file_name.size() - 5); // Quitar extensión .meta

            const Metadata *metadata = version_graph.getFileMetadata(file_name);
            if (metadata)
            {
                std::cout << "- " << file_name << " (Tipo: " << metadata->getFileType()
                          << ", Tamaño: " << metadata->getFileSize() << " bytes, "
                          << "Versión actual: " << version_graph.getCurrentVersion(file_name) << ")\n";
                no_files = false;
            }
        }
    }

    if (no_files)
    {
        std::cout << "No hay archivos en el sistema.\n";
    }
}

size_t FileSystem::getCurrentVersion(const std::string &file_name)
{
    return version_graph.getCurrentVersion(file_name);
}

void FileSystem::sync()
{
    // Sincronizar bloques
    block_manager.sync();

    // Guardar metadatos
    version_graph.saveMetadata(metadata_dir);

    std::cout << "Datos sincronizados a disco.\n";
}

void FileSystem::inspectBlocks(const std::string &file_name)
{
    if (!version_graph.fileExists(file_name))
    {
        std::cout << "El archivo no existe.\n";
        return;
    }

    size_t current_version = version_graph.getCurrentVersion(file_name);
    const VersionInfo *version_info = version_graph.getVersion(file_name, current_version);

    if (!version_info)
    {
        std::cout << "No se pudo obtener información de la versión.\n";
        return;
    }

    std::cout << "Inspeccionando bloques del archivo '" << file_name << "' (versión " << current_version << "):\n";
    std::cout << "Tamaño según metadatos: " << version_graph.getFileMetadata(file_name)->getFileSize() << " bytes\n";
    std::cout << "Número de bloques: " << version_info->block_list.size() << "\n\n";

    for (size_t i = 0; i < version_info->block_list.size(); i++)
    {
        size_t block_index = version_info->block_list[i];

        // Leer bloque
        char buffer[BLOCK_SIZE];
        block_manager.readBlock(block_index, buffer, BLOCK_SIZE);

        // Mostrar información del bloque
        std::cout << "Bloque " << i << " (índice físico " << block_index << "):\n";

        // Mostrar primeros 64 bytes como texto (o menos si contiene nulos)
        std::cout << "  Contenido (texto): '";
        for (size_t j = 0; j < 64 && j < BLOCK_SIZE; j++)
        {
            if (buffer[j] == '\0')
            {
                std::cout << "\\0";
                break;
            }
            else if (buffer[j] >= 32 && buffer[j] <= 126)
            {
                std::cout << buffer[j];
            }
            else
            {
                std::cout << "\\x" << std::hex << static_cast<int>(buffer[j]) << std::dec;
            }
        }
        std::cout << "'\n";

        // Mostrar primeros 16 bytes como hexadecimal
        std::cout << "  Primeros bytes (hex): ";
        for (size_t j = 0; j < 16 && j < BLOCK_SIZE; j++)
        {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(static_cast<unsigned char>(buffer[j])) << " ";
        }
        std::cout << std::dec << "\n\n";
    }
}

// Métodos auxiliares

std::vector<std::pair<size_t, std::vector<char>>> FileSystem::splitIntoBlocks(const std::vector<char> &data)
{
    std::vector<std::pair<size_t, std::vector<char>>> blocks;

    size_t num_blocks = (data.size() + block_size - 1) / block_size; // Redondear hacia arriba

    for (size_t i = 0; i < num_blocks; i++)
    {
        size_t start = i * block_size;
        size_t end = std::min(start + block_size, data.size());

        std::vector<char> block_data(data.begin() + start, data.begin() + end);
        // Si el último bloque no está completo, rellenar con ceros
        if (block_data.size() < block_size)
        {
            block_data.resize(block_size, '\0');
        }

        blocks.push_back({i, block_data});
    }

    return blocks;
}

std::vector<size_t> FileSystem::calculateModifiedBlocks(const std::vector<char> &oldData, const std::vector<char> &newData)
{
    std::vector<size_t> modified_blocks;

    // Calcular cuántos bloques lógicos hay en cada versión
    size_t old_num_blocks = (oldData.size() + block_size - 1) / block_size;
    size_t new_num_blocks = (newData.size() + block_size - 1) / block_size;

    // Máximo número de bloques entre ambas versiones
    size_t max_num_blocks = std::max(old_num_blocks, new_num_blocks);

    for (size_t i = 0; i < max_num_blocks; i++)
    {
        size_t old_start = i * block_size;
        size_t new_start = i * block_size;

        // Verificar si el bloque existe en ambas versiones
        bool block_in_old = old_start < oldData.size();
        bool block_in_new = new_start < newData.size();

        // Si el bloque solo existe en una versión, ha sido modificado
        if (block_in_old != block_in_new)
        {
            modified_blocks.push_back(i);
            continue;
        }

        // Si el bloque existe en ambas versiones, comparar contenido
        if (block_in_old && block_in_new)
        {
            size_t old_end = std::min(old_start + block_size, oldData.size());
            size_t new_end = std::min(new_start + block_size, newData.size());

            // Si los tamaños son diferentes, el bloque ha sido modificado
            if (old_end - old_start != new_end - new_start)
            {
                modified_blocks.push_back(i);
                continue;
            }

            // Comparar contenido byte a byte
            bool different = false;
            for (size_t j = 0; j < old_end - old_start; j++)
            {
                if (oldData[old_start + j] != newData[new_start + j])
                {
                    different = true;
                    break;
                }
            }

            if (different)
            {
                modified_blocks.push_back(i);
            }
        }
    }

    return modified_blocks;

}