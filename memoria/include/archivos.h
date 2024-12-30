#ifndef ARCHIVOS_H_
#define ARCHIVOS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logs.h"

#include <commons/collections/list.h>

extern t_list* instrucciones;

t_list* leer_archivo(char* path);

#endif