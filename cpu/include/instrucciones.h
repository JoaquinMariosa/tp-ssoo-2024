#ifndef INSTRUCCIONES_H_
#define INSTRUCCIONES_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <commons/collections/dictionary.h>

#include "../include/tlb.h"
#include "../../utils/include/protocolo.h"

// Cuando tengo que esperar a recibir el marco u otro dato
typedef struct {
    char* instruccion;
    char* registro_datos;
    char* registro_direccion;
    uint32_t direccion_logica;
    uint32_t tamanio;
    void* datos;
    char* nombre_interfaz;
    uint32_t puntero_archivo;
    char* nombre_archivo;
} t_instruccion_pendiente;

extern uint32_t tam_pagina;
extern bool esperando_datos;

// MMU
uint32_t mmu(uint32_t dir_logica);

// Instrucciones
void funcion_set(t_dictionary* dictionary_registros, char* registro, int valor);
void funcion_sum(t_dictionary* dictionary_registros, char* registro_destino, char* registro_origen);
void funcion_sub(t_dictionary* dictionary_registros, char* registro_destino, char* registro_origen);
void funcion_mov_in(t_dictionary* dictionary_registros, char* registro_datos, char* registro_direccion);
void funcion_mov_out(t_dictionary* dictionary_registros, char* registro_direccion, char* registro_datos);
void funcion_jnz(t_dictionary* dictionary_registros, char* registro, uint32_t valor_pc);
void funcion_resize(uint32_t tamanio);
void funcion_copy_string(t_dictionary* dictionary_registros, uint32_t tamanio);
void funcion_wait(char* recurso);
void funcion_signal(char* recurso);
void funcion_io_gen_sleep(char* interfaz, uint32_t unidades_trabajo);
void funcion_io_stdin_read(t_dictionary* dictionary_registros, char* interfaz, char* registro_direccion, char* registro_tamanio);
void funcion_io_stdout_write(t_dictionary* dictionary_registros, char* interfaz, char* registro_direccion, char* registro_tamanio);
void funcion_io_fs_create(char* interfaz, char* nombre_archivo);
void funcion_io_fs_delete(char* interfaz, char* nombre_archivo);
void funcion_io_fs_truncate(t_dictionary* dictionary_registros, char* interfaz, char* nombre_archivo, char* registro_tamanio);
void funcion_io_fs_write(t_dictionary* dictionary_registros, char* interfaz, char* nombre_archivo, char* reg_direccion, char* reg_tamanio, char* reg_puntero_archivo);
void funcion_io_fs_read(t_dictionary* dictionary_registros, char* interfaz, char* nombre_archivo, char* reg_direccion, char* reg_tamanio, char* reg_puntero_archivo);
void funcion_exit();

extern t_instruccion_pendiente* instruccion_pendiente;
extern pthread_mutex_t instruccion_pendiente_mutex;
extern t_pcb* pcb_a_ejecutar;
extern t_log* cpu_logger;

#endif