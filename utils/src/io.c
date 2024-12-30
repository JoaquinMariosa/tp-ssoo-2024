#include "../include/io.h"

bool coincide_nombre(t_interfaz* interfaz, char* nombre_interfaz) {
    return strcmp(interfaz->nombre, nombre_interfaz) == 0;
}

int buscar_socket_interfaz(t_list* interfaces, char* nombre_interfaz) {
    for (int i = 0; i < list_size(interfaces); i++) {
        t_interfaz* interfaz = list_get(interfaces, i);
        if (coincide_nombre(interfaz, nombre_interfaz)) {
            return interfaz->socket;
        }
    }
    return -1; // No se encontro
}