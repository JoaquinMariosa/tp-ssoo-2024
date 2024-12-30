#include "../include/manejo-memoria.h"

bool existe_tabla_paginas(uint8_t pid) {
    bool existe = false;
    for (int i = 0; i < list_size(tabla_paginas_por_proceso); i++) {
        t_list* tabla_paginas = list_get(tabla_paginas_por_proceso, i);
        // Verifica si la lista de tabla_paginas no es NULL y tiene el mismo PID
        if (tabla_paginas != NULL && ((uint8_t) i) == pid) {
            existe = true;
            break;
        }
    }
    return existe;
}

int obtener_pagina_asignada(uint8_t pid, int marco, t_list* tabla_paginas) {
    // Buscar el numero de pagina correspondiente al marco
    int pagina_asignada = -1;
    for (int i = 0; i < list_size(tabla_paginas); i++) {
        int* marco_asignado = list_get(tabla_paginas, i);
        if (marco_asignado != NULL && *marco_asignado == marco) {
            pagina_asignada = i;
            break;
        }
    }

    // Verificar si se encontro la pagina asignada al marco
    if (pagina_asignada == -1) {
        return -1;
    }

    // Devolver el numero de pagina asignado
    return pagina_asignada;
}

uint32_t obtener_marco_asignado(uint8_t pid, int pagina, t_list *tabla_paginas) {
    if (pagina >= list_size(tabla_paginas)) {
        log_error(memoria_logger, "PID: %d - Pagina: %d fuera del rango de la tabla de paginas", pid, pagina);
        return -1;
    }

    uint32_t *marco_ptr = list_get(tabla_paginas, pagina);
    if (marco_ptr == NULL) {
        log_error(memoria_logger, "PID: %d - Pagina: %d no tiene un marco asignado", pid, pagina);
        return -1;
    }

    log_info(memoria_logger, "Acceso a Tabla de Paginas: PID: %d - Pagina: %d - Marco: %d", pid, pagina, *marco_ptr);
    return *marco_ptr;
}

uint32_t obtener_marco_libre(t_bitarray* marcos_ocupados) {
    for (size_t i = 0; i < bitarray_get_max_bit(marcos_ocupados); i++) {
        if (!bitarray_test_bit(marcos_ocupados, i)) {
            bitarray_set_bit(marcos_ocupados, i);
            return i;
        }
    }
    return -1; // Si no se encuentra un marco libre
}

void liberar_marco(t_bitarray* marcos_ocupados, int marco) {
    bitarray_clean_bit(marcos_ocupados, marco);
}

// Para ver si es Out of memory
int contar_marcos_libres(t_bitarray* marcos_ocupados) {
    int count = 0;
    for (size_t i = 0; i < bitarray_get_max_bit(marcos_ocupados); i++) {
        if (!bitarray_test_bit(marcos_ocupados, i)) {
            count++;
        }
    }
    return count;
}

bool escribir_memoria(void* espacio_usuario, uint32_t numero_marco, uint32_t desplazamiento, uint32_t tamanio, void* datos) {
    uint32_t direccion_fisica = numero_marco * TAM_PAGINA + desplazamiento;
    if (direccion_fisica + tamanio > TAM_MEMORIA) {
        return false; // Error
    }
    // Escribir
    memcpy(espacio_usuario + direccion_fisica, datos, tamanio);
    return true;
}

bool leer_memoria(void* espacio_usuario, uint32_t numero_marco, uint32_t desplazamiento, uint32_t tamanio, void* datos) {
    uint32_t direccion_fisica = numero_marco * TAM_PAGINA + desplazamiento;
    if (direccion_fisica + tamanio > TAM_MEMORIA) {
        return false; // Error
    }
    // Leer
    memcpy(datos, espacio_usuario + direccion_fisica, tamanio);
    return true;
}

// Funcion para encontrar el minimo
int min(int a, int b) {
    return (a < b) ? a : b;
}