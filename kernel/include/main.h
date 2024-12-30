#ifndef KERNEL_MAIN_H_
#define KERNEL_MAIN_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#include "configs.h"
#include "logs.h"
#include "consola.h"
#include "recursos.h"
#include "kernel-memoria.h"
#include "kernel-cpu-dispatch.h"
#include "kernel-cpu-interrupt.h"
#include "kernel-entradasalida.h"
#include "terminar-kernel.h"
#include "funciones-pcb.h"

#include <commons/collections/queue.h>

#include "../../utils/include/sockets.h"
#include "../../utils/include/protocolo.h"
#include "../../utils/include/funciones-serializacion.h"
#include "../../utils/include/file-descriptors.h"
#include "../../utils/include/io.h"

int fd_kernel;
int fd_memoria;
int fd_entradasalida;
int fd_cpu_dispatch;
int fd_cpu_interrupt;

t_config* kernel_config;

t_log* kernel_logger;
t_log* kernel_debug;

// Declaracion archivos de configuracion
char* PUERTO_ESCUCHA; //pdf dice int
char* IP_MEMORIA;
char* PUERTO_MEMORIA;//pdf dice int
char* IP_CPU;
char* PUERTO_CPU_DISPATCH;//pdf dice int
char* PUERTO_CPU_INTERRUPT;//pdf dice int
char* ALGORITMO_PLANIFICACION;
uint32_t QUANTUM;
char** RECURSOS;//lista, luego usar common de string
char** INSTANCIAS_RECURSOS;//lista, luego usar common de string
int GRADO_MULTIPROGRAMACION;

extern t_queue* colaNew;
extern t_queue* colaReady;
extern t_queue* colaBlocked;
extern t_queue* colaExec;
extern t_queue* colaAux;

extern t_list* listaInterfaces;

extern sem_t semaforoPlanificacion;
extern sem_t semaforoPlanificacion2;
extern bool control_primera_vez;

#endif