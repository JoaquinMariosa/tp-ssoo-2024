#include <../include/logs.h>

void init_entradasalida_logs(void) {
    entradasalida_logger = log_create("entradasalida.log", "ENTRADA SALIDA LOGGER", 1, LOG_LEVEL_INFO);
    if(entradasalida_logger == NULL){
        perror("Algo raro paso con el log. No se pudo crear o encontrar el archivo.");
        exit(EXIT_FAILURE);
    }

    entradasalida_debug = log_create("entradasalida-debug.log", "ENTRADA SALIDA DEBUG LOGGER", 1, LOG_LEVEL_TRACE);
    if(entradasalida_debug == NULL){
        perror("Algo raro paso con el log. No se pudo crear o encontrar el archivo.");
        exit(EXIT_FAILURE);
    }
}