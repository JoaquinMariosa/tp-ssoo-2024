#include "../include/terminar-cpu.h"

void terminar_cpu() {
    log_destroy(cpu_logger);
    config_destroy(cpu_config);
    liberar_conexion(fd_cpu_dispatch);
    liberar_conexion(fd_cpu_interrupt);
    liberar_conexion(fd_kernel_dispatch);
    liberar_conexion(fd_kernel_interrupt);
    liberar_conexion(fd_memoria);
}