#include "../include/tlb.h"

pthread_mutex_t tlb_mutex = PTHREAD_MUTEX_INITIALIZER;

t_tlb* buscar_en_tlb(uint8_t pid, uint32_t pagina) {
    pthread_mutex_lock(&tlb_mutex);
    t_tlb* tlb_entry_encontrada = NULL;
    for (int i = 0; i < list_size(lista_tlb); i++) {
        t_tlb* entrada = list_get(lista_tlb, i);
        if (entrada->pid == pid && entrada->pagina == pagina) {
            tlb_entry_encontrada = entrada;
            break;
        }
    }
    pthread_mutex_unlock(&tlb_mutex);
    return tlb_entry_encontrada;
}

void agregar_a_tlb(uint8_t pid, uint32_t pagina, uint32_t marco) {
    pthread_mutex_lock(&tlb_mutex);
    t_tlb* nueva_entrada = malloc(sizeof(t_tlb));
    nueva_entrada->pid = pid;
    nueva_entrada->pagina = pagina;
    nueva_entrada->marco = marco;

    if (list_size(lista_tlb) >= CANTIDAD_ENTRADAS_TLB) {
        // Si la TLB está llena, eliminar la entrada
        t_tlb* entrada_a_eliminar = list_remove(lista_tlb, 0);
        free(entrada_a_eliminar);
    }

    list_add(lista_tlb, nueva_entrada);
    pthread_mutex_unlock(&tlb_mutex);
}

// Para LRU
int buscar_indice_en_lista(t_list* lista, t_tlb* entrada) {
    for (int i = 0; i < list_size(lista); i++) {
        t_tlb* elemento = list_get(lista, i);
        if (elemento == entrada) {
            return i; // Devuelve el indice
        }
    }
    return -1; // Devuelve -1 si hay un error
}

// Para LRU
void mover_elemento_al_principio(t_list *lista, int index) {
    void *elemento = list_remove(lista, index);
    list_add(lista, elemento);
}