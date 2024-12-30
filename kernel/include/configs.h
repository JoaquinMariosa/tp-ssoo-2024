#ifndef CONFIGS_H_
#define CONFIGS_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <commons/config.h>

extern t_config* kernel_config;

// Declaracion archivos de configuracion
extern char* PUERTO_ESCUCHA; //pdf dice int
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;//pdf dice int
extern char* IP_CPU;
extern char* PUERTO_CPU_DISPATCH;//pdf dice int
extern char* PUERTO_CPU_INTERRUPT;//pdf dice int
extern char* ALGORITMO_PLANIFICACION;
extern uint32_t QUANTUM;
extern char** RECURSOS;//lista, luego usar common de string
extern char** INSTANCIAS_RECURSOS;//lista, luego usar common de string
extern int GRADO_MULTIPROGRAMACION;

void init_kernel_config();

#endif