#ifndef MEMORIA_MAIN_H_
#define MEMORIA_MAIN_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <commons/bitarray.h>

#include "configs.h"
#include "logs.h"
#include "memoria-kernel.h"
#include "memoria-cpu.h"
#include "memoria-entradasalida.h"
#include "terminar-memoria.h"

#include "../../utils/include/sockets.h"
#include "../../utils/include/protocolo.h"
#include "../../utils/include/file-descriptors.h"
#include "../../utils/include/io.h"

//variables para conexiones
int fd_kernel;
int fd_memoria;
int fd_entradasalida;
int fd_cpu;

t_config* memoria_config;

// Declaracion archivos de configuracion
char* PUERTO_ESCUCHA; //pdf dice int
int   TAM_MEMORIA;
uint32_t   TAM_PAGINA;
char* PATH_INSTRUCCIONES;
int   RETARDO_RESPUESTA;

t_log* memoria_logger;
t_log* memoria_debug;

extern t_list* listaInterfaces;
extern void* espacio_usuario;
extern t_bitarray* marcos_ocupados;
extern int cant_paginas_marcos;
extern t_list* tabla_paginas_por_proceso;

#endif