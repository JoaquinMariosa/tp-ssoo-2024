#include "../include/main.h"

t_queue* colaNew;
t_queue* colaReady;
t_queue* colaBlocked;
t_queue* colaExec;
t_queue* colaAux;

t_list* listaInterfaces;

sem_t semaforoPlanificacion;
sem_t semaforoPlanificacion2;
bool control_primera_vez;

int main(int argc, char* argv[]) {
    // Inicializamos logger y logger debug
    init_kernel_logs();
    // Inicializamos config
    init_kernel_config();
    // Inicializamos recursos
    iniciar_recursos();

    // Inicializamos colas para planificacion
    colaNew = queue_create();
    colaReady = queue_create();
    colaBlocked = queue_create();
    colaExec = queue_create();
    colaAux = queue_create();

    //Inicializamos semaforo
    control_primera_vez = true;
    sem_init(&semaforoPlanificacion, 1, 0);
    sem_init(&semaforoPlanificacion2, 1, 0);

    // Creamos lista de interfaces
    listaInterfaces = list_create();

    // Iniciar servidor
    fd_kernel = iniciar_servidor(kernel_logger, PUERTO_ESCUCHA, "Kernel");

    // Conectamos como cliente con el modulo MEMORIA
    fd_memoria = crear_conexion(kernel_logger, IP_MEMORIA, PUERTO_MEMORIA, "Memoria");
    send_handshake(kernel_logger, fd_memoria, HANDSHAKE_MEMORIA, "Kernel/Memoria");

    // Hilos
    pthread_t kernel_memoria;
    pthread_create(&kernel_memoria, NULL, (void *)conexion_kernel_memoria, NULL);
    pthread_detach(kernel_memoria);

    //  Conectamos como cliente con el modulo CPU - Dispatch
    fd_cpu_dispatch = crear_conexion(kernel_logger, IP_CPU, PUERTO_CPU_DISPATCH, "Cpu - Dispatch");
    send_handshake(kernel_logger, fd_cpu_dispatch, HANDSHAKE_CPU_DISPATCH, "Kernel/CPU (Dispatch)");

    // Hilos
    pthread_t kernel_cpu_dispatch;
    pthread_create(&kernel_cpu_dispatch, NULL, (void *)conexion_kernel_cpu_dispatch, NULL);
    pthread_detach(kernel_cpu_dispatch);

    // Conectamos como cliente con el modulo CPU - Interrupt
    fd_cpu_interrupt = crear_conexion(kernel_logger, IP_CPU, PUERTO_CPU_INTERRUPT,"Cpu - Interrupt");
    send_handshake(kernel_logger, fd_cpu_interrupt, HANDSHAKE_CPU_INTERRUPT, "Kernel/CPU (Interrupt)");

    // Hilos
    pthread_t kernel_cpu_interrupt;
    pthread_create(&kernel_cpu_interrupt, NULL, (void *)conexion_kernel_cpu_interrupt, NULL);
    pthread_detach(kernel_cpu_interrupt);

    // Consola interactiva
    pthread_t consola_interactiva;
    pthread_create(&consola_interactiva, NULL, (void *)iniciar_consola, NULL);
    pthread_detach(consola_interactiva);

    // Multiples interfaces I/O
    while (1) {
        // Conectamos como servidor con el modulo I/O
        log_info(kernel_logger, "Esperando al modulo Entrada Salida");
        fd_entradasalida = esperar_cliente(kernel_logger, fd_kernel, "Entrada Salida");

        if (recv_handshake(fd_entradasalida, HANDSHAKE_KERNEL)) {
            log_info(kernel_logger, "Handshake OK de %s\n", "IO/Kernel");
        } else {
            log_error(kernel_logger, "Handshake ERROR de %s\n", "IO/Kernel");
        }

        // Hilos
        pthread_t kernel_entradasalida;
        pthread_create(&kernel_entradasalida, NULL, (void *)conexion_kernel_entradasalida, NULL);
        pthread_detach(kernel_entradasalida);
    }
    
    // Terminar programa
    terminar_kernel();

    return 0;
}