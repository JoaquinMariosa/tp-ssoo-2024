#include "../include/main.h"

t_list* listaInterfaces;
void* espacio_usuario;
t_bitarray* marcos_ocupados;
int cant_paginas_marcos; // (?) No se si hace falta que sea global
t_list* tabla_paginas_por_proceso;

int main(int argc, char* argv[]) {

    // Inicializamos logger y logger debug
    init_memoria_logs();

    // Inicializamos config
    init_memoria_config();

    // Inicializamos estructuras de memoria y demas
    // init_memoria(); --> Falta hacer --> deberia tener todo lo q esta abajo hasta el bitarray
    
    // Nombres de IOs
    listaInterfaces = list_create();

    // Inicializamos estructuras de memoria
    // Espacio contiguo de memoria
    espacio_usuario = malloc(TAM_MEMORIA); // Espacio usuario

    // Indica marcos libres y ocupados
    cant_paginas_marcos = TAM_MEMORIA / TAM_PAGINA; // Cantidad de paginas --> Siempre es multiplo del tam de memoria
    void* bitmap_espacio_usuario = malloc(cant_paginas_marcos / 8);
    memset(bitmap_espacio_usuario, 0, cant_paginas_marcos / 8);  // Asegurarse de que esté inicializado a 0
    marcos_ocupados = bitarray_create_with_mode(bitmap_espacio_usuario, cant_paginas_marcos / 8, LSB_FIRST); // Marcos disponibles
    tabla_paginas_por_proceso = list_create();

    // Conexiones
    // Iniciar servidor
    fd_memoria = iniciar_servidor(memoria_logger, PUERTO_ESCUCHA, "Memoria");

    // Conexiones
    // Espera conexion de CPU
    log_info(memoria_logger, "Esperando al modulo CPU");
    fd_cpu = esperar_cliente(memoria_logger, fd_memoria, "CPU");
    if (recv_handshake(fd_cpu, HANDSHAKE_MEMORIA)) {
        log_info(memoria_logger, "Handshake OK de %s\n", "CPU/Memoria");
        // Envia el TAM_PAGINA para que la CPU lo use despues
        send_tam_pagina(fd_cpu, TAM_PAGINA);
    } else {
        log_error(memoria_logger, "Handshake ERROR de %s\n", "CPU/Memoria");
    }

    // Hilos
    pthread_t memoria_cpu;
    pthread_create(&memoria_cpu, NULL, (void *)conexion_memoria_cpu, NULL);
    pthread_detach(memoria_cpu);

    // Espera conexion de Kernel
    log_info(memoria_logger, "Esperando al modulo Kernel");
    fd_kernel = esperar_cliente(memoria_logger, fd_memoria, "Kernel");
    if (recv_handshake(fd_kernel, HANDSHAKE_MEMORIA)) {
        log_info(memoria_logger, "Handshake OK de %s\n", "Kernel/Memoria");
    } else {
        log_error(memoria_logger, "Handshake ERROR de %s\n", "Kernel/Memoria");
    }

    // Hilos
    pthread_t memoria_kernel;
    pthread_create(&memoria_kernel, NULL, (void *)conexion_memoria_kernel, NULL);
    pthread_detach(memoria_kernel);

    // Multiples interfaces I/O
    while (1) {
        // ESPERAR CONEXION I/O
        log_info(memoria_logger, "Esperando al modulo Entrada/Salida");
        fd_entradasalida = esperar_cliente(memoria_logger, fd_memoria, "I/O");
        if (recv_handshake(fd_entradasalida, HANDSHAKE_MEMORIA)) {
            log_info(memoria_logger, "Handshake OK de %s\n", "IO/Memoria");
        } else {
            log_error(memoria_logger, "Handshake ERROR de %s\n", "IO/Memoria");
        }

        // Hilos
        pthread_t memoria_entradasalida;
        pthread_create(&memoria_entradasalida, NULL, (void *)conexion_memoria_entradasalida, NULL);
        pthread_detach(memoria_entradasalida);
    }

    // Terminar programa
    terminar_memoria();

    return 0;
}