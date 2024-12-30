#include <../include/logs.h>

void init_kernel_logs(void) {
    kernel_logger = log_create("kernel.log", "KERNEL LOGGER", 1, LOG_LEVEL_INFO);
    if(kernel_logger == NULL){
        perror("Algo raro paso con el log. No se pudo crear o encontrar el archivo.");
        exit(EXIT_FAILURE);
    }

    kernel_debug = log_create("kernel-debug.log", "KERNEL DEBUG LOGGER", 1, LOG_LEVEL_TRACE);
    if(kernel_debug == NULL){
        perror("Algo raro paso con el log. No se pudo crear o encontrar el archivo.");
        exit(EXIT_FAILURE);
    }
}