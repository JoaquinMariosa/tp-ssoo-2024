#ifndef FD_H_ 
#define FD_H_

#include <inttypes.h>

// Variables globales de los FD
// Kernel
extern int fd_kernel;
// Conexion entre Kernel y CPU (Dispatch e Interrupt)
extern int fd_kernel_dispatch;
extern int fd_kernel_interrupt;
extern int fd_cpu_dispatch;
extern int fd_cpu_interrupt;
// CPU
extern int fd_cpu;
// Memoria
extern int fd_memoria;
// IO
extern int fd_entradasalida;

#endif