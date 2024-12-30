#ifndef TLB_H_
#define TLB_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <string.h>
#include "configs.h"

typedef struct {
    uint8_t pid;
    uint32_t pagina;
    uint32_t marco;
} t_tlb;

extern t_list* lista_tlb;

t_tlb* buscar_en_tlb(uint8_t pid, uint32_t pagina);
void agregar_a_tlb(uint8_t pid, uint32_t pagina, uint32_t marco);
int buscar_indice_en_lista(t_list* lista, t_tlb* entrada);
void mover_elemento_al_principio(t_list *lista, int index);

#endif