#include "FileSystem.h"
#include <iostream>

void menu() {
    std::cout << "\nOpciones:\n"
              << "1. Crear archivo\n"
              << "2. Escribir en archivo\n"
              << "3. Leer archivo\n"
              << "4. Mostrar metadatos\n"
              << "5. Rollback a versión anterior\n"
              << "6. Salir\n"
              << "Selecciona una opción: ";
}

int main() {
    FileSystem fs("storage.bin");
    std::string file_name, extension;
    int opcion;

    do {
        menu();
        std::cin >> opcion;
        std::cin.ignore();

        switch (opcion) {
            case 1: { // Crear archivo
                std::cout << "Nombre del archivo: ";
                std::getline(std::cin, file_name);
                std::cout << "Extensión del archivo: ";
                std::getline(std::cin, extension);
                fs.createFile(file_name, extension);
                std::cout << "Archivo creado.\n";
                break;
            }
            case 2: { // Escribir en archivo
                std::cout << "Nombre del archivo: ";
                std::getline(std::cin, file_name);
            
                // Leer contenido actual del archivo
                std::vector<char> current_data = fs.readFile(file_name);
                if (current_data.empty()) {
                    std::cout << "El archivo está vacío o no existe.\n";
                } else {
                    std::cout << "Contenido actual del archivo:\n";
                    std::cout << std::string(current_data.begin(), current_data.end()) << "\n";
                }
            
                // Pedir datos a escribir
                std::string data;
                std::cout << "Datos a escribir: ";
                std::getline(std::cin, data);
                std::vector<char> buffer(data.begin(), data.end());
            
                // Pedir offset
                std::string offset_str;
                std::cout << "Ingrese la posición (offset) donde escribir: ";
                std::getline(std::cin, offset_str);
                size_t offset = std::stoul(offset_str);
            
                // Si el offset es mayor al tamaño, simplemente se escribe al final
                if (offset >= current_data.size()) {
                    offset = current_data.size();
                }
            
                // Preguntar al usuario si quiere sobrescribir o insertar
                char option = ' ';
                if (offset < current_data.size()) { // Solo preguntar si hay datos en esa posición
                    std::cout << "Ya existen datos en esa posición. ¿Desea sobrescribir (S) o insertar (I) sin sobrescribir? ";
                    std::cin >> option;
                    std::cin.ignore(); // Limpiar el buffer de entrada
                }
            
                if (option == 'S' || option == 's') {
                    // Sobrescribir: Reemplaza los caracteres en la posición dada
                    for (size_t i = 0; i < buffer.size() && (offset + i) < current_data.size(); ++i) {
                        current_data[offset + i] = buffer[i];
                    }
            
                    // Si los nuevos datos son más largos, agregar al final
                    if (offset + buffer.size() > current_data.size()) {
                        current_data.insert(current_data.end(), buffer.begin() + (current_data.size() - offset), buffer.end());
                    }
            
                } else { 
                    // Insertar sin sobrescribir: Desplaza el contenido existente
                    current_data.insert(current_data.begin() + offset, buffer.begin(), buffer.end());
                }
            
                // Escribir el nuevo contenido en el archivo
                fs.writeFile(file_name, 0, current_data);
            
                std::cout << "Datos escritos correctamente.\n";
                break;
            }            
            
            case 3: { // Leer archivo
                std::cout << "Nombre del archivo: ";
                std::getline(std::cin, file_name);
                std::vector<char> content = fs.readFile(file_name);
                std::cout << "Contenido leído: " << std::string(content.begin(), content.end()) << "\n";
                break;
            }
            case 4: { // Mostrar metadatos
                std::cout << "Nombre del archivo: ";
                std::getline(std::cin, file_name);
                fs.printFileMetadata(file_name);
                break;
            }
            case 5: { // Rollback a versión anterior
                std::cout << "Nombre del archivo: ";
                std::getline(std::cin, file_name);
                int version;
                std::cout << "Número de versión a restaurar: ";
                std::cin >> version;
                std::cin.ignore();
                fs.rollbackFile(file_name, version);
                std::cout << "Rollback realizado.\n";
                break;
            }
            case 6:
                break;
            default:
                std::cout << "Opción no válida.\n";
        }
    } while (opcion != 6);

    return 0;
}