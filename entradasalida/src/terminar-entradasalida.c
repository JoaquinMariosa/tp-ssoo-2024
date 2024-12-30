#include "../include/terminar-entradasalida.h"

void terminar_entradasalida() {
    if(strcmp(TIPO_INTERFAZ, "DIALFS") == 0) {
        terminar_bitmap();
        free(path_bloques);
        free(path_bitmap);
    }
    log_destroy(entradasalida_logger);
    config_destroy(entradasalida_config);
    liberar_conexion(fd_kernel);
    liberar_conexion(fd_memoria);
}