#ifndef MEMORIA_KERNEL_H_
#define MEMORIA_KERNEL_H_

#include <stdlib.h>
#include <stdio.h>

#include "configs.h"
#include "logs.h"
#include "archivos.h"
#include "manejo-memoria.h"
#include <commons/bitarray.h>

#include "../include/file-descriptors.h"

#include "../../utils/include/sockets.h"
#include "../../utils/include/protocolo.h"

extern t_list* instrucciones;
extern t_list* instrucciones_por_proceso;

extern t_bitarray* marcos_ocupados;
extern t_list* tabla_paginas_por_proceso;

void conexion_memoria_kernel();

#endif