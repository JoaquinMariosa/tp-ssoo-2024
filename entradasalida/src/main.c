#include "../include/main.h"

t_bitarray* bitmap_blocks;
char* path_bloques;
char* path_bitmap;

int main(int argc, char* argv[]) {

    // Inicializamos logger y logger debug
    init_entradasalida_logs();

    // Pedir nombre y archivo config
    log_info(entradasalida_logger, "Ingrese el nombre de la I/O:"); // EJemplo --> TECLADO
	char* nombre = readline("> ");
    log_info(entradasalida_logger, "Nombre de la I/O leido: %s\n", nombre);
	
    log_info(entradasalida_logger, "Ingrese el archivo de configuracion:"); // Ejemplo --> TECLADO.config
    char* config = readline("> ");
    log_info(entradasalida_logger, "Archivo de config leido: %s\n", config);

    // Inicializamos config
    init_entradasalida_config(config);

    // Inicializamos bloques y bitmap
    if (strcmp(TIPO_INTERFAZ, "DIALFS") == 0) {
        // Crear carpeta dialfs si no existe
        struct stat st = {0};
        if (stat(PATH_BASE_DIALFS, &st) == -1) {
            mkdir(PATH_BASE_DIALFS, 0777);
        }

        // Concatenar con path de PATH_BASE_DIALFS
        uint32_t MAX_LENGTH = 256;
        path_bloques = malloc(MAX_LENGTH);
        strcpy(path_bloques, PATH_BASE_DIALFS);
        strcat(path_bloques, "/bloques.dat");

        init_bloques(path_bloques); // bloques.dat

        // Concatenar con path de PATH_BASE_DIALFS
        path_bitmap = malloc(MAX_LENGTH);
        strcpy(path_bitmap, PATH_BASE_DIALFS);
        strcat(path_bitmap, "/bitmap.dat");

        init_bitmap(path_bitmap); // bitmap.dat
    }

    // Conexiones
    // Conexión con Memoria
    if (strcmp(TIPO_INTERFAZ, "GENERICA") != 0) {
        fd_memoria = crear_conexion(entradasalida_logger, IP_MEMORIA, PUERTO_MEMORIA, "Memoria");
        send_handshake(entradasalida_logger, fd_memoria, HANDSHAKE_MEMORIA, "IO/Memoria");

        // Enviar a Memoria el nombre
        send_interfaz(fd_memoria, nombre, strlen(nombre) + 1, TIPO_INTERFAZ, strlen(TIPO_INTERFAZ) + 1);

        // Hilos para Memoria
        pthread_t entradasalida_memoria;
        pthread_create(&entradasalida_memoria, NULL, (void *)conexion_entradasalida_memoria, NULL);
        pthread_detach(entradasalida_memoria);
    }

    // Conexion con Kernel
    fd_kernel = crear_conexion(entradasalida_logger, IP_KERNEL, PUERTO_KERNEL, "Kernel");
    send_handshake(entradasalida_logger, fd_kernel, HANDSHAKE_KERNEL, "IO/Kernel");

    // Envia a Kernel el nombre
    send_interfaz(fd_kernel, nombre, strlen(nombre) + 1, TIPO_INTERFAZ, strlen(TIPO_INTERFAZ) + 1);

    // Hilos
    pthread_t entradasalida_kernel;
    pthread_create(&entradasalida_kernel, NULL, (void *)conexion_entradasalida_kernel, NULL);
    pthread_join(entradasalida_kernel, NULL);

    // Terminar programa
    free(nombre);
    free(config);
    terminar_entradasalida();

    return 0;
}