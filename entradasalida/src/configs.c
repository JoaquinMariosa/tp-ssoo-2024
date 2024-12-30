#include <../include/configs.h>

void init_entradasalida_config(char* config) {
    entradasalida_config = config_create(config);
    if (entradasalida_config == NULL) {
        perror("Error al intertar cargar el config.");
        exit(EXIT_FAILURE);
    }

    TIPO_INTERFAZ = config_get_string_value(entradasalida_config, "TIPO_INTERFAZ");
    // Ver tipo de interfaz
    if(strcmp(TIPO_INTERFAZ, "GENERICA") == 0) {
        TIEMPO_UNIDAD_TRABAJO = config_get_int_value(entradasalida_config, "TIEMPO_UNIDAD_TRABAJO");
        IP_KERNEL = config_get_string_value(entradasalida_config, "IP_KERNEL");
        PUERTO_KERNEL = config_get_string_value(entradasalida_config, "PUERTO_KERNEL");
    } else if(strcmp(TIPO_INTERFAZ, "STDIN") == 0) {
        IP_KERNEL = config_get_string_value(entradasalida_config, "IP_KERNEL");
        PUERTO_KERNEL = config_get_string_value(entradasalida_config, "PUERTO_KERNEL");
        IP_MEMORIA = config_get_string_value(entradasalida_config, "IP_MEMORIA");
        PUERTO_MEMORIA = config_get_string_value(entradasalida_config, "PUERTO_MEMORIA");
    } else if(strcmp(TIPO_INTERFAZ, "STDOUT") == 0) {
        IP_KERNEL = config_get_string_value(entradasalida_config, "IP_KERNEL");
        PUERTO_KERNEL = config_get_string_value(entradasalida_config, "PUERTO_KERNEL");
        IP_MEMORIA = config_get_string_value(entradasalida_config, "IP_MEMORIA");
        PUERTO_MEMORIA = config_get_string_value(entradasalida_config, "PUERTO_MEMORIA");
    } else if(strcmp(TIPO_INTERFAZ, "DIALFS") == 0) {
        TIEMPO_UNIDAD_TRABAJO = config_get_int_value(entradasalida_config, "TIEMPO_UNIDAD_TRABAJO");
        IP_KERNEL = config_get_string_value(entradasalida_config, "IP_KERNEL");
        PUERTO_KERNEL = config_get_string_value(entradasalida_config, "PUERTO_KERNEL");
        IP_MEMORIA = config_get_string_value(entradasalida_config, "IP_MEMORIA");
        PUERTO_MEMORIA = config_get_string_value(entradasalida_config, "PUERTO_MEMORIA");
        PATH_BASE_DIALFS = config_get_string_value(entradasalida_config, "PATH_BASE_DIALFS");
        BLOCK_SIZE = config_get_int_value(entradasalida_config, "BLOCK_SIZE");
        BLOCK_COUNT = config_get_int_value(entradasalida_config, "BLOCK_COUNT");
        RETRASO_COMPACTACION = config_get_int_value(entradasalida_config, "RETRASO_COMPACTACION");
    }
}