#ifndef LOGS_H_
#define LOGS_H_

#include <stdio.h>
#include <stdlib.h>

#include <commons/log.h>

extern t_log* memoria_logger;
extern t_log* memoria_debug;

void init_memoria_logs();

#endif