#ifndef CONFIGS_H_
#define CONFIGS_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <commons/config.h>

extern t_config* memoria_config;

// Declaracion archivos de configuracion
extern char* PUERTO_ESCUCHA; //pdf dice int
extern int TAM_MEMORIA;
extern uint32_t TAM_PAGINA;
extern char* PATH_INSTRUCCIONES;
extern int RETARDO_RESPUESTA;

void init_memoria_config();

#endif