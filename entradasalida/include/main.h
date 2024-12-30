#ifndef ENTRADASALIDA_MAIN_H_
#define ENTRADASALIDA_MAIN_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>

#include "configs.h"
#include "logs.h"
#include "terminar-entradasalida.h"
#include "entradasalida-kernel.h"
#include "entradasalida-memoria.h"
#include "fs.h"
#include <readline/readline.h>

#include "../../utils/include/sockets.h"
#include "../../utils/include/protocolo.h"
#include "../../utils/include/file-descriptors.h"

extern t_bitarray* bitmap_blocks;
extern char* path_bloques;
extern char* path_bitmap;

int fd_kernel;
int fd_memoria;

t_log* entradasalida_logger;
t_log* entradasalida_debug;

t_config* entradasalida_config;

// Declaracion archivos de configuracion
char* TIPO_INTERFAZ;
int TIEMPO_UNIDAD_TRABAJO;
char* IP_KERNEL;
char* PUERTO_KERNEL; //pdf dice int?
char* IP_MEMORIA;
char* PUERTO_MEMORIA;//pdf dice int?
char* PATH_BASE_DIALFS;
int BLOCK_SIZE;
int BLOCK_COUNT;
int RETRASO_COMPACTACION;

#endif