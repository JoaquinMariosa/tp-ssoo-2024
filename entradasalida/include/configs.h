#ifndef CONFIGS_H_
#define CONFIGS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/config.h>

extern t_config* entradasalida_config;

// Declaracion archivos de configuracion
extern char* TIPO_INTERFAZ;
extern int TIEMPO_UNIDAD_TRABAJO;
extern char* IP_KERNEL;
extern char* PUERTO_KERNEL; //pdf dice int?
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;//pdf dice int?
extern char* PATH_BASE_DIALFS;
extern int BLOCK_SIZE;
extern int BLOCK_COUNT;
extern int RETRASO_COMPACTACION;

void init_entradasalida_config(char* config);

#endif