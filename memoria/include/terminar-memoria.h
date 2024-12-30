#ifndef TERMINAR_MEMORIA_H_
#define TERMINAR_MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "configs.h"
#include "logs.h"

#include "../../utils/include/file-descriptors.h"
#include "../../utils/include/sockets.h"

#include <commons/config.h>
#include <commons/log.h>

extern t_list* listaInterfaces;

extern t_list* instrucciones;
extern t_list* instrucciones_por_proceso;

void terminar_memoria();

#endif