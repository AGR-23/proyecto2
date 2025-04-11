#include "FileSystem.h"
#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>

extern "C" {

static FileSystem* fs = nullptr;
static std::string archivo_abierto;

void fs_init(const char* path, size_t size_mb) {
    if (fs) delete fs;
    fs = new FileSystem(path, size_mb);
}

bool create(const char* file, const char* tipo) {
    return fs->create(file, tipo);
}

bool open(const char* file) {
    if (!archivo_abierto.empty()) return false;
    if (fs->open(file)) {
        archivo_abierto = file;
        return true;
    }
    return false;
}

bool write(size_t offset, const char* datos, size_t len) {
    if (archivo_abierto.empty()) return false;
    std::vector<char> buffer(datos, datos + len);
    return fs->write(archivo_abierto, offset, buffer);
}

char* read(size_t* out_len) {
    if (archivo_abierto.empty()) {
        *out_len = 0;
        return nullptr;
    }
    std::vector<char> content = fs->read(archivo_abierto);
    *out_len = content.size();
    char* result = new char[content.size()];
    std::memcpy(result, content.data(), content.size());
    return result;
}

bool rollback(size_t version_id) {
    if (archivo_abierto.empty()) return false;
    return fs->rollbackFile(archivo_abierto, version_id);
}

bool close() {
    if (archivo_abierto.empty()) return false;
    if (fs->close(archivo_abierto)) {
        archivo_abierto.clear();
        return true;
    }
    return false;
}

void listFiles() {
    fs->listFiles();
}

void inspectBlocks(const char* file) {
    fs->inspectBlocks(file);
}

void sync() {
    fs->sync();
}

void printMemoryUsage() {
    fs->printMemoryUsage();
}

void fs_destroy() {
    if (fs) {
        delete fs;
        fs = nullptr;
        archivo_abierto.clear();
    }
}

}
