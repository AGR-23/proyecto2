#include "FileSystem.h"
#include <iostream>
#include <fstream>
#include <cstring>

FileSystem::FileSystem(const std::string &storage_path, size_t block_size)
    : block_size(block_size), block_manager(storage_path.c_str(), block_size)
{
    storage.open(storage_path, std::ios::in | std::ios::out | std::ios::binary);
    if (!storage)
    {
        storage.open(storage_path, std::ios::out | std::ios::binary);
        storage.close();
        storage.open(storage_path, std::ios::trunc | std::ios::in | std::ios::out | std::ios::binary);
    }
}

FileSystem::~FileSystem()
{
    if (storage.is_open())
    {
        storage.close();
    }
}

bool FileSystem::createFile(const std::string &file_name, const std::string &file_type)
{
    if (files.find(file_name) != files.end())
    {
        std::cerr << "Error: El archivo ya existe\n";
        return false;
    }

    files[file_name] = Metadata(file_name, 0, file_type);
    return true;
}

bool FileSystem::writeFile(const std::string &file_name, size_t offset, const std::vector<char> &data)
{
    auto it = files.find(file_name);
    if (it == files.end())
    {
        std::cerr << "Error: El archivo '" << file_name << "' no existe.\n";
        return false;
    }

    Metadata &metadata = it->second;
    std::vector<char> current_data = readFile(file_name);

    // Validar offset
    if (offset > current_data.size())
    {
        std::cerr << "Error: Offset fuera de los límites del archivo.\n";
        return false;
    }

    // Asegurar tamaño suficiente
    if (offset + data.size() > current_data.size())
    {
        current_data.resize(offset + data.size(), '\0');
    }

    // Copiar los datos en la posición indicada
    std::copy(data.begin(), data.end(), current_data.begin() + offset);

    // Obtener bloques modificados
    std::vector<size_t> modified_blocks = calculateModifiedBlocks(readFile(file_name), current_data, block_size);

    // Guardar bloques en el almacenamiento
    for (size_t block : modified_blocks)
    {
        size_t block_offset = block * block_size;
        size_t write_size = std::min(block_size, current_data.size() - block_offset);

        storage.seekp(block_offset, std::ios::beg);
        if (storage.fail())
        {
            std::cerr << "Error: No se pudo posicionar en el archivo para escritura.\n";
            return false;
        }
        storage.write(current_data.data() + block_offset, write_size);
    }
    storage.flush();

    // Actualizar metadatos
    metadata.updateFileSize(current_data.size());
    metadata.addVersion(metadata.version_history.size() + 1, modified_blocks);

    std::cout << "Datos escritos en '" << file_name << "' correctamente.\n";
    return true;
}

std::vector<char> FileSystem::readFile(const std::string &file_name)
{
    auto it = files.find(file_name);
    if (it == files.end())
    {
        std::cerr << "Error: Archivo no encontrado\n";
        return {};
    }

    Metadata &metadata = it->second;
    std::vector<char> buffer(metadata.file_size); // Reservar espacio justo

    storage.seekg(0, std::ios::beg); // Adjusted to start reading from the beginning of the file
    storage.read(buffer.data(), metadata.file_size);

    if (!storage)
    {
        std::cerr << "Error: No se pudo leer el archivo\n";
        return {};
    }

    return buffer;
}

void FileSystem::rollbackFile(const std::string &file_name, size_t version_id)
{
    auto it = files.find(file_name);
    if (it == files.end())
    {
        std::cerr << "Error: Archivo no encontrado\n";
        return;
    }

    Metadata &metadata = it->second;
    std::vector<char> restored_data;

    if (!version_graph.restoreVersion(file_name, version_id, restored_data))
    {
        std::cerr << "Error: No se pudo restaurar la versión " << version_id << " del archivo.\n";
        return;
    }

    // Restaurar solo los bloques afectados
    std::vector<size_t> modified_blocks = calculateModifiedBlocks(readFile(file_name), restored_data, block_size);

    for (size_t block : modified_blocks)
    {
        size_t block_offset = block * block_size;
        size_t write_size = std::min(block_size, restored_data.size() - block_offset);

        storage.seekp(block_offset, std::ios::beg);

        storage.write(restored_data.data() + block_offset, write_size);
    }
    storage.flush();

    std::cout << "Archivo '" << file_name << "' restaurado a la versión " << version_id << ".\n";
}

void FileSystem::printFileMetadata(const std::string &file_name)
{
    if (files.find(file_name) == files.end())
    {
        std::cerr << "Error: Archivo no encontrado\n";
        return;
    }

    files[file_name].printMetadata();
}

std::vector<size_t> FileSystem::calculateModifiedBlocks(
    const std::vector<char> &oldData, const std::vector<char> &newData, size_t block_size)
{
    std::vector<size_t> modified_blocks;
    size_t num_blocks = (std::max(oldData.size(), newData.size()) + block_size - 1) / block_size;

    for (size_t i = 0; i < num_blocks; ++i)
    {
        size_t start = i * block_size;
        size_t end_old = std::min(start + block_size, oldData.size());
        size_t end_new = std::min(start + block_size, newData.size());

        // Si los datos viejos o nuevos son más cortos, rellenar con ceros
        std::vector<char> oldBlock(end_old > start ? oldData.begin() + start : oldData.end(), oldData.begin() + end_old);
        std::vector<char> newBlock(end_new > start ? newData.begin() + start : newData.end(), newData.begin() + end_new);

        if (oldBlock != newBlock)
        {
            modified_blocks.push_back(i);
        }
    }
    return modified_blocks;
}