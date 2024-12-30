#ifndef MANEJO_MEMORIA_H_
#define MANEJO_MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include "logs.h"
#include "configs.h"

extern t_list* tabla_paginas_por_proceso;

bool existe_tabla_paginas(uint8_t pid);
int obtener_pagina_asignada(uint8_t pid, int marco, t_list* tabla_paginas_por_proceso);
uint32_t obtener_marco_asignado(uint8_t pid, int pagina, t_list *tabla_paginas);
uint32_t obtener_marco_libre(t_bitarray* marcos_ocupados);
void liberar_marco(t_bitarray* marcos_ocupados, int marco);
int contar_marcos_libres(t_bitarray* marcos_ocupados);
bool escribir_memoria(void* espacio_usuario, uint32_t numero_marco, uint32_t desplazamiento, uint32_t tamanio, void* datos);
bool leer_memoria(void* espacio_usuario, uint32_t numero_marco, uint32_t desplazamiento, uint32_t tamanio, void* datos);

int min(int a, int b);

#endif