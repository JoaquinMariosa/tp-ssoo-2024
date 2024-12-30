#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include <commons/string.h>
#include <commons/collections/queue.h>

#include "logs.h"
#include "funciones-pcb.h"
#include "planificador.h"
#include "recursos.h"
#include <readline/readline.h>

#include <../../utils/include/file-descriptors.h>
#include <../../utils/include/protocolo.h>

extern t_queue* colaNew;
extern t_queue* colaReady;
extern t_queue* colaBlocked;
extern t_queue* colaExec;

extern t_list* recursos_de_procesos;
extern t_list* recursos;

extern bool control_primera_vez;
extern sem_t semaforoPlanificacion;
extern sem_t semaforoPlanificacion2;

extern pthread_mutex_t colaNewMutex;

t_pcb* buscar_pcb(t_queue* cola, uint8_t pid_a_borrar);
void finalizar_proceso(uint8_t pid_a_borrar);
void buscar_en_queues_y_finalizar(t_pcb* pcb_borrar, uint8_t pid_a_borrar);
void listar_procesos(t_queue* cola, pthread_mutex_t mutex, char* estado);
void listar_procesos_blocked();
void iniciar_planificacion();
void atender_instruccion (char* leido);
void iniciar_consola(t_log* logger);
int size_all_queues();

#endif