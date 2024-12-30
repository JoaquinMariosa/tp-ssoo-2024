#include "../include/terminar-kernel.h"

void terminar_kernel() {
    log_destroy(kernel_logger);
    config_destroy(kernel_config);
    liberar_conexion(fd_kernel);
    liberar_conexion(fd_cpu_dispatch);
    liberar_conexion(fd_cpu_interrupt);
    liberar_conexion(fd_memoria);
    liberar_conexion(fd_entradasalida);
}