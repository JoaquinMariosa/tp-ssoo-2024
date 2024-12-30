#ifndef KERNEL_MEMORIA_H_
#define KERNEL_MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>

#include "configs.h"
#include "logs.h"
#include "recursos.h"
#include <pthread.h>
#include <semaphore.h>

#include "../../utils/include/file-descriptors.h"
#include "../../utils/include/sockets.h"
#include "../../utils/include/protocolo.h"

extern t_queue* colaNew;
extern t_queue* colaExec;
extern t_queue* colaReady;
extern t_queue* colaBlocked;
extern t_queue* colaAux;

extern int control_planificacion;
extern int contador_procesos;

extern pthread_mutex_t colaExecMutex;
extern sem_t semaforoPlanificacion;
extern sem_t semaforoPlanificacion2;

void conexion_kernel_memoria();
int size_all_queues();

#endif