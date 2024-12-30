#ifndef RECURSOS_H_
#define RECURSOS_H_

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <pthread.h>

#include "configs.h"
#include "logs.h"

#include <commons/collections/queue.h>
#include <commons/collections/list.h>

#include "../../utils/include/file-descriptors.h"
#include "../../utils/include/sockets.h"
#include "../../utils/include/protocolo.h"
#include "../../utils/include/pcb.h"

typedef struct {
    char* nombre;
    int instancias;
    t_queue* blocked;
} t_recurso;

extern t_list* recursos_de_procesos;
extern t_list* recursos;

extern t_queue* colaReady;
extern pthread_mutex_t colaReadyMutex;
extern t_queue* colaAux;
extern pthread_mutex_t colaAuxMutex;

void ingreso_ready_aux(t_queue* cola, pthread_mutex_t mutex, char* estado);
void iniciar_recursos();
// Finaliza un proceso (de consola o cuando termina proceso normalmente)
void liberar_recursos(uint8_t pid);

#endif