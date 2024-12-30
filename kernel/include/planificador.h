#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "logs.h"
#include "configs.h"
#include "consola.h"
#include "semaphore.h"

#include <pthread.h>
#include <semaphore.h>

#include <commons/collections/queue.h>
#include <commons/temporal.h>

#include "../../utils/include/pcb.h"

extern t_queue* colaNew;
extern t_queue* colaReady;
extern t_queue* colaBlocked;
extern t_queue* colaExec;
extern t_queue* colaAux;

extern pthread_t quantum_thread;

extern t_temporal* tiempo_vrr;
extern int control_planificacion;
extern int contador_procesos;

extern sem_t semaforoPlanificacion;
extern bool control_primera_vez;

int size_all_queues();
void ingreso_ready_aux(t_queue* cola, pthread_mutex_t mutex, char* estado);

void planificacionFIFO();
void planificacionRR();
void planificacionVRR();

extern pthread_mutex_t colaNewMutex;
extern pthread_mutex_t colaReadyMutex;
extern pthread_mutex_t colaBlockedMutex;
extern pthread_mutex_t colaExecMutex;
extern pthread_mutex_t colaAuxMutex;

#endif