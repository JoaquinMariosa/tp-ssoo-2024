#ifndef LOGS_H_
#define LOGS_H_

#include <stdio.h>
#include <stdlib.h>

#include <commons/log.h>

extern t_log* kernel_logger;
extern t_log* kernel_debug;

void init_kernel_logs();

#endif