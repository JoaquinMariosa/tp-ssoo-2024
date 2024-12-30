#ifndef IO_H_
#define IO_H_

#include <inttypes.h>
#include <commons/collections/list.h>
#include <string.h>

typedef struct {
    int socket;
    char* nombre;
    char* tipo;
} t_interfaz;

bool coincide_nombre(t_interfaz* interfaz, char* nombre_interfaz);
int buscar_socket_interfaz(t_list* interfaces, char* nombre_interfaz);

#endif