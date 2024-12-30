#include "../include/configs.h"

void init_memoria_config(void) {
    memoria_config = config_create("memoria.config");
    if (memoria_config == NULL)
    {
        perror("Error al intertar cargar el config.");
        exit(EXIT_FAILURE);
    }

    PUERTO_ESCUCHA = config_get_string_value(memoria_config, "PUERTO_ESCUCHA");
    TAM_MEMORIA = config_get_int_value(memoria_config, "TAM_MEMORIA");
    TAM_PAGINA = config_get_int_value(memoria_config, "TAM_PAGINA");
    PATH_INSTRUCCIONES = config_get_string_value(memoria_config, "PATH_INSTRUCCIONES");
    RETARDO_RESPUESTA = config_get_int_value(memoria_config, "RETARDO_RESPUESTA");
}