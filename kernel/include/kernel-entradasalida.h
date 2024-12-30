#ifndef KERNEL_IO_H_
#define KERNEL_IO_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "configs.h"
#include "logs.h"

#include <commons/collections/queue.h>

#include "../include/kernel-interfaces.h"

#include "../../utils/include/file-descriptors.h"
#include "../../utils/include/sockets.h"
#include "../../utils/include/protocolo.h"
#include "../../utils/include/io.h"

void conexion_kernel_entradasalida();

#endif