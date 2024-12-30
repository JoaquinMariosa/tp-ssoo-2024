#ifndef CPU_KERNEL_DISPATCH_H_
#define CPU_KERNEL_DISPATCH_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/collections/list.h>

#include "configs.h"
#include "logs.h"
#include "semaphore.h"
#include "../../utils/include/pcb.h"

#include "../../utils/include/file-descriptors.h"
#include "../../utils/include/sockets.h"
#include "../../utils/include/protocolo.h"

extern uint8_t size_instrucciones;
extern t_pcb* pcb_a_ejecutar;

extern t_dictionary* dictionary_registros;
extern char* instruccion;
extern char* instruccion_recibida;
extern char** instruccion_separada;

extern bool esperando_datos;

void printear_pcb();
void conexion_cpu_memoria();
void crear_diccionario(t_dictionary* dictionary_registros);

void conexion_cpu_kernel_dispatch();

#endif