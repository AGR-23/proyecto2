#include "FileSystem.h"
#include <iostream>
#include <fstream>
#include <cstring>

FileSystem::FileSystem(const std::string& storage_path, size_t block_size)
    : block_size(block_size), block_manager(storage_path.c_str(), block_size) // 🔹 Convertir a const char*
{
    storage.open(storage_path, std::ios::in | std::ios::out | std::ios::binary);
    if (!storage) {
        storage.open(storage_path, std::ios::out | std::ios::binary);
        storage.close();
        storage.open(storage_path, std::ios::in | std::ios::out | std::ios::binary);
    }
}

FileSystem::~FileSystem() {
    if (storage.is_open()) {
        storage.close();
    }
}

bool FileSystem::createFile(const std::string& file_name, const std::string& file_type) {
    if (files.find(file_name) != files.end()) {
        std::cerr << "Error: El archivo ya existe\n";
        return false;
    }

    files[file_name] = Metadata(file_name, 0, file_type);
    return true;
}

bool FileSystem::writeFile(const std::string& file_name, size_t offset, const std::vector<char>& data) {
    // Verificar si el archivo existe
    auto it = files.find(file_name);
    if (it == files.end()) {
        std::cerr << "Error: El archivo '" << file_name << "' no existe.\n";
        return false;
    }

    Metadata& metadata = it->second; // Obtener metadatos
    std::vector<char> current_data = readFile(file_name); // Leer contenido actual

    // Verificar si el offset es válido
    if (offset > current_data.size()) {
        std::cerr << "Error: El offset supera el tamaño del archivo.\n";
        return false;
    }

    // Ajustar el tamaño del buffer si es necesario
    if (offset + data.size() > current_data.size()) {
        current_data.resize(offset + data.size(), '\0');
    }

    // Escribir los nuevos datos en la posición especificada
    std::copy(data.begin(), data.end(), current_data.begin() + offset);

    // Calcular bloques modificados
    std::vector<size_t> modified_blocks = calculateModifiedBlocks(readFile(file_name), current_data, block_size);

    // Escribir en almacenamiento
    storage.seekp(0, std::ios::beg); // Ir al inicio para sobrescribir correctamente
    storage.write(current_data.data(), current_data.size());
    storage.flush();

    // Actualizar tamaño en metadatos
    metadata.updateFileSize(current_data.size());

    // Obtener la última versión y registrar una nueva
    size_t last_version = 0;
    for (const auto& entry : metadata.version_history) 
    {
        if (entry.first > last_version) 
        {
        last_version = entry.first;
    }
    }
    size_t new_version = last_version + 1;

    metadata.addVersion(new_version, modified_blocks);

    std::cout << "Datos escritos en '" << file_name << "' correctamente.\n";
    return true;
} 

std::vector<char> FileSystem::readFile(const std::string& file_name) {
    if (files.find(file_name) == files.end()) {
        std::cerr << "Error: Archivo no encontrado\n";
        return {};
    }

    if (!storage.is_open()) {
        std::cerr << "Error: El archivo de almacenamiento se cerró inesperadamente\n";
        return {};
    }

    storage.seekg(0, std::ios::end);
    size_t file_size = storage.tellg();
    storage.seekg(0, std::ios::beg);

    std::vector<char> buffer(file_size);
    storage.read(buffer.data(), file_size);

    if (!storage) {
        std::cerr << "Error: No se pudo leer el archivo\n";
        return {};
    }

    return buffer;
}




void FileSystem::rollbackFile(const std::string& file_name, size_t version_id) {
    std::vector<char> restored_data;
    if (!version_graph.restoreVersion(file_name, version_id, restored_data)) {
        std::cout << "Error: No se pudo restaurar la versión " << version_id << " del archivo " << file_name << std::endl;
        return;
    }

    // Sobreescribir archivo con la versión restaurada
    std::ofstream file(file_name, std::ios::binary | std::ios::trunc);
    if (!file) {
        std::cout << "Error: No se pudo abrir el archivo para rollback.\n";
        return;
    }

    file.write(restored_data.data(), restored_data.size());
    file.close();
    
    std::cout << "Archivo " << file_name << " restaurado a la versión " << version_id << std::endl;
}


void FileSystem::printFileMetadata(const std::string& file_name) {
    if (files.find(file_name) == files.end()) {
        std::cerr << "Error: Archivo no encontrado\n";
        return;
    }

    files[file_name].printMetadata();
}

std::vector<size_t> FileSystem::calculateModifiedBlocks(const std::vector<char>& oldData, const std::vector<char>& newData, size_t block_size) {
    std::vector<size_t> modified_blocks;
    size_t num_blocks = (oldData.size() > newData.size() ? oldData.size() : newData.size()) / block_size;

    for (size_t i = 0; i <= num_blocks; ++i) {
        size_t start = i * block_size;
        size_t end = std::min(start + block_size, std::max(oldData.size(), newData.size()));

        std::vector<char> oldBlock(oldData.begin() + start, oldData.begin() + std::min(end, oldData.size()));
        std::vector<char> newBlock(newData.begin() + start, newData.begin() + std::min(end, newData.size()));

        if (oldBlock != newBlock) {
            modified_blocks.push_back(i);
        }
    }
    return modified_blocks;
}