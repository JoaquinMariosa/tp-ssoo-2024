#ifndef CONFIGS_H_
#define CONFIGS_H_

#include <stdio.h>
#include <stdlib.h>

#include <commons/config.h>

extern t_config* cpu_config;

// Declaracion archivos de configuracion
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA; //char para func
extern char* PUERTO_ESCUCHA_DISPATCH;//char para func
extern char* PUERTO_ESCUCHA_INTERRUPT;//char para func
extern int CANTIDAD_ENTRADAS_TLB;
extern char* ALGORITMO_TLB;

void init_cpu_config();

#endif