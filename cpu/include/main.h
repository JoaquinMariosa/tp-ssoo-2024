#ifndef CPU_MAIN_H_
#define CPU_MAIN_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <commons/string.h>

#include "configs.h"
#include "logs.h"
#include "cpu-memoria.h"
#include "cpu-kernel-dispatch.h"
#include "cpu-kernel-interrupt.h"
#include "terminar-cpu.h"
#include "instrucciones.h"

#include "../../utils/include/sockets.h"
#include "../../utils/include/protocolo.h"
#include "../../utils/include/file-descriptors.h"
#include "../../entradasalida/include/configs.h"

int fd_kernel_dispatch;
int fd_kernel_interrupt;
int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_memoria;

t_log* cpu_logger;
t_log* cpu_debug;

t_config* cpu_config;

t_registros miCPU;

// Declaracion archivos de configuracion
char* IP_MEMORIA;
char* PUERTO_MEMORIA; //char para func
char* PUERTO_ESCUCHA_DISPATCH;//char para func
char* PUERTO_ESCUCHA_INTERRUPT;//char para func
int CANTIDAD_ENTRADAS_TLB;
char* ALGORITMO_TLB;

#endif