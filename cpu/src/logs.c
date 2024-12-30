#include "../include/logs.h"

void init_cpu_logs(void) {
    cpu_logger = log_create("cpu.log", "CPU LOGGER", 1, LOG_LEVEL_INFO);
    if(cpu_logger == NULL){
        perror("Algo raro paso con el log. No se pudo crear o encontrar el archivo.");
        exit(EXIT_FAILURE);
    }

    cpu_debug = log_create("cpu-debug.log", "CPU DEBUG LOGGER", 1, LOG_LEVEL_TRACE);
    if(cpu_debug == NULL){
        perror("Algo raro paso con el log. No se pudo crear o encontrar el archivo.");
        exit(EXIT_FAILURE);
    }
}