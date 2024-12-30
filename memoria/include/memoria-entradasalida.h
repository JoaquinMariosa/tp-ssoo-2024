#ifndef MEMORIA_IO_H_
#define MEMORIA_IO_H_

#include <stdlib.h>
#include <stdio.h>

#include "configs.h"
#include "logs.h"

#include "../include/file-descriptors.h"
#include "../include/memoria-interfaces.h"

#include "../../utils/include/sockets.h"
#include "../../utils/include/protocolo.h"
#include "../../utils/include/io.h"

extern t_list* listaInterfaces;

void conexion_memoria_entradasalida();

#endif