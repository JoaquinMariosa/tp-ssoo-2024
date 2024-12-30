#ifndef FUNCIONES_PCB_H_
#define FUNCIONES_PCB_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "../../utils/include/pcb.h"
#include "configs.h"

void asignar_pid(t_pcb* pcb);
t_pcb* crear_pcb();

#endif