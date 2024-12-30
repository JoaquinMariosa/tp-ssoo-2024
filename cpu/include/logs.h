#ifndef LOGS_H
#define LOGS_H

#include <stdio.h>
#include <stdlib.h>

#include <commons/log.h>

extern t_log* cpu_logger;
extern t_log* cpu_debug;

void init_cpu_logs();

#endif