#include "../include/logs.h"

void init_memoria_logs(void) {
    memoria_logger = log_create("memoria.log", "MEMORIA LOGGER", 1, LOG_LEVEL_INFO);
    if(memoria_logger == NULL){
        perror("Algo raro paso con el log. No se pudo crear o encontrar el archivo.");
        exit(EXIT_FAILURE);
    }

    memoria_debug = log_create("memoria-debug.log", "MEMORIA DEBUG LOGGER", 1, LOG_LEVEL_TRACE);
    if(memoria_debug == NULL){
        perror("Algo raro paso con el log. No se pudo crear o encontrar el archivo.");
        exit(EXIT_FAILURE);
    }
}