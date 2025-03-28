# Sistema de Gestión de Archivos con Versionado  

PARA VISUALIZAR ESTO MEJOR DESDE VS CODE DAR LA COMBINACIÓN DE TECLAS ctrl + shift + v (EN LINUX ME FUNCIONA)

# IMPORTANTE:
El código tiene la estructura y fundamentos para implementar COW (identificación de bloques modificados, seguimiento de versiones, historial de cambios), pero no implementa completamente el mecanismo de COW. La implementación actual rastrea qué bloques se modifican, pero parece seguir copiando todo el archivo en lugar de solo los bloques modificados, porque no estaba agregando el archivo que maneja los bloques en compilación y no me entere si funcionaba o no, por eso no hay evidencia clara de que los bloques no modificados se compartan entre versiones.

## Características principales  

- **Gestión de archivos**: Crear, escribir, leer y listar archivos almacenados.
- **Almacenamiento basado en bloques**: La información se almacena en un archivo binario `.bin`, donde los bloques de datos se asignan dinámicamente a los archivos.
- **Historial de versiones**: Se mantiene un registro de los cambios en cada archivo, permitiendo restaurar versiones anteriores.
- **No implementa Copy-On-Write (COW)**: Aunque el sistema gestiona versiones, no usa COW para minimizar la duplicación de datos. Cada versión guarda una copia de los bloques modificados sin reutilizar referencias a datos inmutables.
- **Metadatos detallados**: Cada archivo tiene un conjunto de metadatos que incluyen su tamaño, bloques asignados y versión histórica.
  
---

## 1. Explicación de las funciones principales  

### Crear archivo (`createFile`)  

- Recibe como parámetros el nombre y la extensión del archivo.
- Crea una nueva entrada en el sistema de archivos, inicializando su estructura de metadatos.
- No asigna bloques de memoria de inmediato, sino que los asignará cuando se escriba información en el archivo.
- Se almacena la información en el archivo binario `.bin` como referencia dentro del sistema.  

### Escribir en archivo (`writeFile`)  

- Permite escribir datos en una posición específica del archivo.
- Si la posición ya contiene datos, el usuario puede elegir:  
  - **Sobrescribir (S)**: Reemplaza los datos existentes en esa posición.  
  - **Insertar (I)**: Desplaza los datos existentes hacia la derecha e inserta la nueva información.  
- Se crea una nueva versión del archivo con un identificador único, pero no reutiliza bloques previos como lo haría un sistema con Copy-On-Write.  
- **No es un sistema COW**, ya que cada versión almacena una copia de los datos modificados sin compartir bloques inmutables con versiones anteriores.  

### Leer archivo (`readFile`)  

- Recupera los datos almacenados en los bloques asignados al archivo.
- Si el archivo no existe o está vacío, muestra un mensaje de error.
- Los datos se extraen desde el archivo binario `.bin`, donde se han almacenado secuencialmente.

### Mostrar metadatos (`showMetadata`)  

- Muestra información estructurada del archivo, incluyendo:  
  - Nombre del archivo  
  - Tamaño en bytes  
  - Tipo de archivo  
  - Lista de bloques usados  
  - Historial de versiones  

### Restaurar versión (`rollbackToVersion`) - **No funciona correctamente**  

- **Objetivo:** Devolver el archivo a una versión anterior registrada en el historial.  
- **Problema actual:**  
  - Aunque el historial de versiones está registrado, los bloques originales no se reasignan correctamente.  
  - Se pierde la relación entre el estado actual y la versión que se intenta restaurar.  
  - Posible solución: Implementar un mecanismo de recuperación de bloques que reemplace los actuales con los de la versión deseada.  
- **No usa COW**, lo que significa que cada versión almacena datos de manera independiente, generando duplicación innecesaria.  

---

## 2. Uso del archivo binario `.bin`  

El sistema usa un archivo binario (`filesystem.bin`) en lugar de un almacenamiento en texto plano por varias razones:  

1. **Eficiencia de almacenamiento**:  
   - Almacenar datos en binario reduce el tamaño del archivo comparado con formatos de texto.  
   - Permite una mejor gestión de bloques sin necesidad de conversiones innecesarias.  

2. **Acceso estructurado**:  
   - Cada archivo se representa con metadatos y bloques asignados dentro del `.bin`.  
   - Esto permite recuperar rápidamente archivos sin necesidad de recorrer toda la estructura como en texto plano.  

3. **Manejo de versiones y bloques**:  
   - La organización en binario permite registrar de manera eficiente el historial de versiones.  
   - Facilita la reasignación de bloques cuando se realizan modificaciones en los archivos.  

---

## 3. Estructura del código  

### **`Metadata.cpp`**  
- Gestiona la información de cada archivo.  
- Mantiene el historial de versiones y la asignación de bloques.  

### **`BlockManager.cpp`**  
- Administra los bloques de datos dentro del archivo binario.  
- Asigna y libera bloques cuando se escriben o eliminan datos.  

### **`FileSystem.cpp`**  
- Implementa las funciones principales de gestión de archivos.  
- Conecta la lectura/escritura con el sistema de almacenamiento.  

### **`VersionGraph.cpp`**  
- Se encarga de manejar el historial de versiones.  
- **Problema:** No está gestionando correctamente la restauración de versiones previas.  

---

## 4. Compilación y ejecución  

Para compilar el proyecto, usar `make` y luego `make run`

Another approach is known as copy-on-write (yes, COW), and is used
in a number of popular file systems, including Sun’s ZFS [B07]. This technique never overwrites files or directories in place; rather, it places new
updates to previously unused locations on disk. After a number of updates are completed, COW file systems flip the root structure of the file
system to include pointers to the newly updated structures. Doing so
makes keeping the file system consistent straightforward. We’ll be learning more about this technique when we discuss the log-structured file
system (LFS) in a future chapter; LFS is an early example of a COW