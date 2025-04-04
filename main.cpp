#include "FileSystem.h"
#include <iostream>
#include <string>
#include <limits>

void mostrarMenu()
{

    std::cout << "\n===== SISTEMA DE ARCHIVOS COW ====\n"
              << "1. Crear archivo\n"
              << "2. Abrir archivo\n"
              << "3. Cerrar archivo\n"
              << "4. Escribir en archivo\n"
              << "5. Leer archivo\n"
              << "6. Mostrar metadatos\n"
              << "7. Listar archivos\n"
              << "8. Rollback a versión anterior\n"
              << "9. Sincronizar cambios\n"
              << "10. Inspeccionar bloques (debug)\n"
              << "11. Ver estadisticas de Memoria\n"
              << "12. Salir\n"
              << "Selecciona una opción: ";
}

std::string leerLinea()
{
    std::string linea;
    std::getline(std::cin, linea);
    return linea;
}

int leerNumero()
{
    int numero;
    std::cin >> numero;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return numero;
}

int main()
{
    /*/
    std::cout << "Inicializando sistema de archivos...\n";
    FileSystem fs("storage.bin", 10); // 10 MB de almacenamiento

    int opcion;
    std::string nombre_archivo_abierto; // Track del archivo abierto

    
    do
    {
        mostrarMenu();
        opcion = leerNumero();

        switch (opcion)
        {
        case 1:
        { // Crear archivo
            std::cout << "Nombre del archivo: ";
            std::string nombre = leerLinea();
            std::cout << "Extensión (sin punto): ";
            std::string extension = leerLinea();

            if (fs.create(nombre, extension))
            {
                std::cout << "Archivo creado.\n";
            }
            else
            {
                std::cout << "Error: Archivo ya existe.\n";
            }
            break;
        }

        case 2:
        { // Abrir archivo
            if (!nombre_archivo_abierto.empty())
            {
                std::cout << "Cierra el archivo actual primero.\n";
                break;
            }
            std::cout << "Nombre del archivo: ";
            nombre_archivo_abierto = leerLinea();

            if (fs.open(nombre_archivo_abierto))
            {
                std::cout << "Archivo abierto.\n";
            }
            else
            {
                std::cout << "Error: No existe o ya está abierto.\n";
                nombre_archivo_abierto.clear();
            }
            break;
        }

        case 3:
        { // Cerrar archivo
            if (nombre_archivo_abierto.empty())
            {
                std::cout << "No hay archivo abierto.\n";
                break;
            }
            if (fs.close(nombre_archivo_abierto))
            {
                std::cout << "Archivo cerrado.\n";
                nombre_archivo_abierto.clear();
            }
            else
            {
                std::cout << "Error al cerrar.\n";
            }
            break;
        }

        case 4:
        { // Escribir (requiere archivo abierto)
            if (nombre_archivo_abierto.empty())
            {
                std::cout << "Abre un archivo primero.\n";
                break;
            }

            auto contenido = fs.read(nombre_archivo_abierto);
            std::cout << "Contenido actual (" << contenido.size() << " bytes):\n";
            std::cout << std::string(contenido.begin(), contenido.end()) << "\n";

            std::cout << "Offset [0-" << contenido.size() << "]: ";
            size_t offset = static_cast<size_t>(leerNumero());

            std::cout << "Texto a escribir: ";
            std::string texto = leerLinea();
            std::vector<char> datos(texto.begin(), texto.end());

            if (fs.write(nombre_archivo_abierto, offset, datos))
            {
                std::cout << "Escrito. Nueva versión: "
                          << fs.getCurrentVersion(nombre_archivo_abierto) << "\n";
            }
            else
            {
                std::cout << "Error al escribir.\n";
            }
            break;
        }

        case 5:
        { // Leer (requiere archivo abierto)
            if (nombre_archivo_abierto.empty())
            {
                std::cout << "Abre un archivo primero.\n";
                break;
            }
            auto contenido = fs.read(nombre_archivo_abierto);
            std::cout << "Contenido (v" << fs.getCurrentVersion(nombre_archivo_abierto) << "):\n";
            std::cout << std::string(contenido.begin(), contenido.end()) << "\n";
            break;
        }

        case 6:
        { // Mostrar metadatos
            std::cout << "Nombre del archivo: ";
            std::string nombre = leerLinea();
            fs.printFileMetadata(nombre);
            break;
        }

        case 7:
        { // Listar archivos
            fs.listFiles();
            break;
        }

        case 8:
        { // Rollback
            if (nombre_archivo_abierto.empty())
            {
                std::cout << "Abre un archivo primero.\n";
                break;
            }

            // Mostrar versiones disponibles
            fs.printFileMetadata(nombre_archivo_abierto);

            std::cout << "Número de versión a restaurar: ";
            size_t version = static_cast<size_t>(leerNumero());

            if (fs.rollbackFile(nombre_archivo_abierto, version))
            {
                std::cout << "Rollback completado. Mostrando contenido restaurado:\n";
                auto contenido = fs.read(nombre_archivo_abierto);
                std::cout << std::string(contenido.begin(), contenido.end()) << "\n";
            }
            else
            {
                std::cout << "Error en rollback.\n";
            }
            break;
        }

        case 9:
        { // Sincronizar cambios
            fs.sync();
            std::cout << "Cambios sincronizados con disco.\n";
            break;
        }

        case 10:
        { // Inspeccionar bloques
            std::cout << "Nombre del archivo: ";
            std::string nombre = leerLinea();
            fs.inspectBlocks(nombre);
            break;
        }

        case 11:
            fs.printMemoryUsage();
            break;

        default:
            std::cout << "Opción no válida.\n";

        case 12:
        { // Salir
            if (!nombre_archivo_abierto.empty())
            {
                fs.close(nombre_archivo_abierto);
            }
            std::cout << "Cerrando sistema...\n";
            break;
        }
        }
    } while (opcion != 12);
/**/

    // 1. Inicialización del sistema de archivos
    std::cout << "=== DEMOSTRACIÓN BIBLIOTECA COW - <):>D ===\n";
    FileSystem fs("cow_data.bin", 20); // 20 MB de almacenamiento

    // 2. Creación de archivos
    std::cout << "\n--- Creando archivos ---\n";
    fs.create("foo.txt", "txt");
    fs.create("datos.bin", "bin");

    // 3. Trabajando con un archivo de texto
    std::cout << "\n--- Ejemplo foo.txt ---\n";
    fs.open("foo.txt");

    // Escritura inicial
    fs.write("foo.txt", 0, {'H', 'O', 'L', 'A', ' ', 'M', 'U', 'N', 'D', 'O', '!'});
    std::cout << "Escritura inicial completada.\n";
    fs.printMemoryUsage(); // mostrar uso de memoria

    // Mostrar contenido y metadatos
    auto contenido = fs.read("foo.txt");
    std::cout << "Contenido actual: " << std::string(contenido.begin(), contenido.end()) << "\n";
    fs.printFileMetadata("foo.txt");

    // 4. Modificación del archivo (COW en acción)
    std::cout << "\n--- Modificando archivo (versión 2) ---\n";
    fs.write("foo.txt", 11, {' ', 'V', 'e', 'r', 's', 'i', 'o', 'n', ' ', '2'});

    // Mostrar cambios
    contenido = fs.read("foo.txt");
    std::cout << "Nueva versión: " << fs.getCurrentVersion("foo.txt") << "\n";
    std::cout << "Contenido actual: " << std::string(contenido.begin(), contenido.end()) << "\n";
    fs.printFileMetadata("foo.txt");
    fs.printMemoryUsage(); // mostrar uso de memoria

    // 5. Rollback a versión anterior
    std::cout << "\n--- Rollback a versión 2 ---\n";
    fs.rollbackFile("foo.txt", 2);
    auto act_cont = fs.read("foo.txt");
    std::cout << "Contenido Actual: " << std::string(act_cont.begin(), act_cont.end()) << "\n";

    // 6. Trabajando con archivo binario

    // Inspección de bloques
    std::cout << "Inspeccionando bloques:\n";
    fs.inspectBlocks("datos.bin");

    // 7. Operaciones de mantenimiento
    fs.close("foo.txt");
    std::cout << "\n--- Mantenimiento del sistema ---\n";
    fs.listFiles();
    fs.sync();

    // 8. Limpieza

    fs.close("datos.bin");

    std::cout << "\n=== Demostración completada ===\n";

    return 0;
}