#ifndef MEMORIA_INTERFACES_H_
#define MEMORIA_INTERFACES_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>

#include "configs.h"
#include "logs.h"
#include "archivos.h"

#include <commons/collections/list.h>

#include "../include/file-descriptors.h"
#include "../include/memoria-interfaces.h"

#include "manejo-memoria.h"
#include "../../utils/include/file-descriptors.h"
#include "../../utils/include/sockets.h"
#include "../../utils/include/protocolo.h"
#include "../../utils/include/io.h"

extern t_list* listaInterfaces;
extern t_list* instrucciones_por_proceso;
extern void* espacio_usuario;
extern t_bitarray* marcos_ocupados;
extern t_list* tabla_paginas_por_proceso;

void conexion_memoria_interfaces(void* arg);

#endif