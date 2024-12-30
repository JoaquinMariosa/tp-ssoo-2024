#ifndef TERMINAR_IO_H_
#define TERMINAR_IO_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "configs.h"
#include "logs.h"

#include "../../utils/include/file-descriptors.h"
#include "../../utils/include/sockets.h"

#include <commons/config.h>
#include <commons/log.h>
#include <commons/bitarray.h>

extern t_bitarray* bitmap_blocks;
extern char* path_bloques;
extern char* path_bitmap;

void terminar_bitmap();
void terminar_entradasalida();

#endif