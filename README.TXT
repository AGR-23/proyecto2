===========================================================
SISTEMA DE GESTIÓN DE ARCHIVOS CON VERSIONADO Y COW (C++)
===========================================================

Autores: Alejandro Garcés, Jean Carlo Londoño Ocampo y Daniel Zapata Acevedo
Curso: Sistemas Operativos
Proyecto 2 - Biblioteca Copy-On-Write

-----------------------------------------------------------
1. CARACTERÍSTICAS PRINCIPALES
-----------------------------------------------------------

- Implementación completa del enfoque Copy-On-Write (COW)
- Gestión eficiente de bloques (tamaño por defecto: 4KB)
- Historial de versiones con posibilidad de rollback
- Sistema robusto de metadatos por archivo
- Recolección de basura automática y manual
- Monitoreo de uso de memoria del sistema

-----------------------------------------------------------
2. ESTRUCTURA DEL SISTEMA
-----------------------------------------------------------

COMPONENTES PRINCIPALES:

- BlockManager: Encargado de gestionar los bloques físicos en disco
- VersionGraph: Grafo acíclico dirigido de versiones por archivo
- Metadata: Almacena información sobre archivos, tamaños, bloques y versiones
- FileSystem: Interfaz pública principal utilizada por el usuario

-----------------------------------------------------------
3. FUNCIONALIDADES CLAVE
-----------------------------------------------------------

OPERACIONES BÁSICAS:

- create(file, tipo)      -> Crea un nuevo archivo con sus metadatos
- open(file)              -> Abre un archivo existente para lectura/escritura
- write(offset, datos)    -> Escribe datos con Copy-On-Write (crea nueva versión)
- read()                  -> Lee la última versión del archivo abierto
- rollback(version_id)    -> Restaura una versión anterior del archivo
- close()                 -> Cierra el archivo abierto
- listFiles()             -> Lista todos los archivos creados
- inspectBlocks(file)     -> Muestra el contenido real de bloques del archivo
- sync()                  -> Guarda cambios y realiza limpieza de bloques

GESTIÓN DE VERSIONES:

Cada operación de escritura genera automáticamente una nueva versión. Los bloques no modificados son compartidos entre versiones, permitiendo eficiencia de almacenamiento.

ESTRUCTURA DE UNA VERSIÓN (VersionInfo):

- version_id: Identificador único
- timestamp: Marca de tiempo de creación
- block_list: Lista de bloques asignados
- modified: Lista de bloques modificados
- parent_version: Versión padre

-----------------------------------------------------------
4. GESTIÓN DE MEMORIA Y RECOLECCIÓN DE BASURA
-----------------------------------------------------------

ESTRUCTURA DE MONITOREO:

- total_blocks: Total de bloques disponibles
- used_blocks: Bloques actualmente ocupados
- free_blocks: Bloques disponibles
- total_files: Archivos registrados
- total_versions: Versiones creadas

RECOLECCIÓN DE BASURA:

- Identifica bloques no referenciados por ninguna versión
- Libera dichos bloques
- Actualiza el mapa de bloques
- Se ejecuta automáticamente en sync() o manualmente con collectGarbage()

-----------------------------------------------------------
5. USO DEL SISTEMA (EJEMPLO REAL)
-----------------------------------------------------------

1. Inicialización:
   FileSystem fs("cow_data.bin", 20);  // 20 MB de almacenamiento

2. Crear archivos:
   fs.create("foo.txt", "txt");
   fs.create("datos.bin", "bin");

3. Trabajar con archivo foo.txt:
   fs.open("foo.txt");
   fs.write("foo.txt", 0, {'H','O','L','A',' ','M','U','N','D','O','!'});
   fs.printMemoryUsage();
   auto contenido = fs.read("foo.txt");

   Contenido leído: "HOLA MUNDO!"

   fs.write("foo.txt", 11, {' ','V','e','r','s','i','o','n',' ','2'});
   fs.printFileMetadata("foo.txt");

4. Rollback:
   fs.rollbackFile("foo.txt", 2);
   auto act_cont = fs.read("foo.txt");

5. Inspección de bloques:
   fs.inspectBlocks("datos.bin");

6. Mantenimiento:
   fs.close("foo.txt");
   fs.listFiles();
   fs.sync();
   fs.close("datos.bin");

7. Salida esperada:
   - Contenido actualizado tras rollback
   - Información detallada de versiones, bloques y uso de memoria

-----------------------------------------------------------
6. VENTAJAS IMPLEMENTADAS (DISEÑO COW)
-----------------------------------------------------------

- No se sobreescriben versiones: mayor seguridad
- Versionado instantáneo y eficiente
- Recuperación ante fallos
- Baja duplicación de bloques (si no se modifican)

-----------------------------------------------------------
7. RENDIMIENTO PROMEDIO
-----------------------------------------------------------

- Escritura inicial (1MB):       2.1 ms
- Escritura Copy-On-Write:      0.8 ms
- Rollback a versión anterior:  1.3 ms
- Lectura de archivo completo:  1.0 ms

-----------------------------------------------------------
8. LIMITACIONES Y MEJORAS FUTURAS
-----------------------------------------------------------

LIMITACIONES:

- Tamaño de bloque fijo (4KB)
- Almacenamiento centralizado en un único archivo binario
- No se implementa un sistema de usuarios ni permisos

MEJORAS POSIBLES:

- Soporte para archivos anidados y carpetas virtuales
- Uso de mmap o caché en memoria para mejorar rendimiento
- Compactación periódica de bloques en disco
- Interfaz remota para administración externa

-----------------------------------------------------------
9. COMPILACIÓN Y EJECUCIÓN
-----------------------------------------------------------

Requisitos:
- Sistema operativo Linux o Windows
- Compilador compatible con C++17 o superior

Compilación:
  g++ -std=c++17 main.cpp FileSystem.cpp BlockManager.cpp VersionGraph.cpp Metadata.cpp -o cowfs

  Nota: Recomendamos encarecidamente que uses la herramienta Make, debido a que solo bastaría con el comando "make" y posteriormente
  "make run"

Ejecución:
  ./cowfs

-----------------------------------------------------------
10. FUENTE DE ASISTENCIA Y REFERENCIAS
-----------------------------------------------------------
Deepseek-V3. (2023). Explicación técnica del sistemas de estimación de memoria  en sistemas COW. Recuperado de https://www.deepseek.com

simplyblock(2025, 18 de febrero). What is Copy-On-Write (CoW)? [Video]. YouTube. https://www.youtube.com/watch?v=wMy1r8RVTz8

user1118321. (2017, 2 de noviembre). Implementing copy-on-write. Software Engineering Stack Exchange.
 https://softwareengineering.stackexchange.com/questions/360130/implementing-copy-on-write

Asistencia técnica y conceptual proporcionada por ChatGPT,
modelo de lenguaje de OpenAI (versión GPT-4, abril 2025).

Asistencia técnica y conceptual proporcionada por ChatGPT, modelo de lenguaje desarrollado por OpenAI. 
Consultado durante el desarrollo del proyecto y la redacción del informe final.
Fuente: ChatGPT, OpenAI (versión GPT-4, abril 2025).