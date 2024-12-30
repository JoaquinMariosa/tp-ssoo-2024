#ifndef FS_H_
#define FS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <dirent.h>
#include <stdint.h>

#include "configs.h"
#include "logs.h"

#include "../../utils/include/sockets.h"
#include "../../utils/include/protocolo.h"
#include "../../utils/include/file-descriptors.h"

extern t_bitarray* bitmap_blocks;
extern char* path_bloques;

void init_bloques(char* path);
void init_bitmap(char* path);
void terminar_bitmap();
int primer_bloque_libre(t_bitarray* bitmap_bloques);
void crear_archivo(char* nombre, t_bitarray* bitmap_bloques);
void liberar_bloque(t_bitarray* bitmap_bloques, int bloque);
void borrar_archivo(char* nombre, t_bitarray* bitmap_bloques);
int contar_bloques_libres(t_bitarray* bitmap_bloques);
int contar_bloques_libres_continuos(t_bitarray* bitmap_bloques, int bloque_inicial, int bloques_necesarios);
void ocupar_bloque(t_bitarray* bitmap_bloques, int bloque);
void truncate_archivo(char* nombre, int tamanio_nuevo, t_bitarray* bitmap_bloques, t_pcb* pcb);
void write_archivo(char* nombre, void* datos, int tamanio_write, int ptr_archivo_write, t_bitarray* bitmap_bloques);
void read_archivo(char* nombre, int tamanio_read, int dir_fisica_read, int ptr_archivo_read, t_pcb* pcb_fs_read, t_bitarray* bitmap_bloques);
void limpiar_bitmap(t_bitarray* bitmap_bloques);
void iniciar_compactacion(t_bitarray* bitmap_bloques, char* nombre_truncate, int tamanio_nuevo);

#endif