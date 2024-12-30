#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>

int crear_conexion(t_log* logger, char *ip, char* puerto, char* modulo);
int iniciar_servidor(t_log* logger, char* puerto, char* modulo);
int esperar_cliente(t_log* logger, int socket_servidor, char* modulo);
int recibir_operacion(int socket_cliente);
void liberar_conexion(int socket_cliente);

#endif