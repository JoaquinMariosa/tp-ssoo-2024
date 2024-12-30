#include "../include/archivos.h"

t_list* leer_archivo(char* path) {
    FILE* archivo;
    archivo = fopen(path, "rt");

    if (archivo == NULL) {
        perror("Error abriendo archivo");
        exit(EXIT_FAILURE);
    }

    u_int32_t MAX_LENGTH = 128;
    t_list* instrucciones = list_create(); // Crear la lista para guardar las instrucciones
    char* linea = malloc(MAX_LENGTH);
    int contador = 0;

	log_info(memoria_logger, "Leyendo el archivo");
    while (fgets(linea, MAX_LENGTH, archivo) != NULL) { // Leer línea por línea
        // Crea una nueva cadena para cada instrucción y la copia desde `linea`
        char* instruccion = malloc(strlen(linea) + 1); // +1 para el terminador nulo
        strcpy(instruccion, linea); // Copiar la línea leída a la nueva memoria
	    log_info(memoria_logger, "Instruccion: %s", instruccion);
        list_add(instrucciones, instruccion); // Agregar a la lista
        contador++;
    }

    log_info(memoria_logger, "Lectura de archivo finalizada");

    fclose(archivo); // Cerrar el archivo
    free(linea); // Libero memoria
    return instrucciones; // Devolver la lista de instrucciones
}