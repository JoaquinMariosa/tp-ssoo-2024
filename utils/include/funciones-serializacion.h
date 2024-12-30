#ifndef FUNCIONES_SERIALIZACION_H_
#define FUNCIONES_SERIALIZACION_H_

#include <inttypes.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <commons/log.h>
#include <sys/socket.h>

#include "pcb.h"

typedef enum {
	MENSAJE,
	PAQUETE,

    // Se van a crear mas operaciones segun el enunciado
    // Para organizar mejor vamos a definirlos asi: Nombre_ModuloQueEnvia_ModuloQueRecibe,
    // La abreviatura de los modulos son:
    // - Kernel = K
    // - CPU = CPU
    // - Memoria = M
    // - Entrada/Salida = IO
    // Ejemplo: INICIAR_PROCESO_KM
    
    // ESTAS SON LAS OPERACIONES QUE PENSAMOS QUE VAN A HABER EN NUESTRO TP, PERO PUEDEN IR CAMBIANDO A LO LARGO DEL MISMO

    // Kernel -> Consola interactiva
    // EJECUTAR_SCRIPT 
    INICIAR_PROCESO, // Envia el path y el pid del proceso a Memoria
    // FINALIZAR_PROCESO
    // DETENER_PLANIFICACION
    // INICIAR_PLANIFICACION
    // MULTIPROGRAMACION
    // PROCESO_ESTADO
    RECIBIR_CONTEXTO_DE_EJEC, // Recibir registros y PC del CPU
    RECIBIR_SIZE_INSTRUCCIONES, // Para pedir la cantidad necesaria de instrucciones
    WAIT, // Resta 1 al recurso
    SIGNAL, // Suma 1 al recurso

    // CPU (no hace falta ponerlas aca despues las sacamos)
    RECIBIR_PCB, // Recibir PCB de Kernel
    RECIBIR_PID, // Recibe el pid del proceso que se tiene que interrumpir
    RECIBIR_PID_A_BORRAR, // Recibe el pid del proceso a borrar
    RECIBIR_INSTRUCCION, // Recibe la instruccion a ejecutar
    RECIBIR_TAM_PAGINA, // Recibe el tamaño de pagina para usar en la MMU
    RECIBIR_PID_A_INTERRUMPIR, // Recibe el pid a interrumpir
    RECIBIR_MARCO, // En caso de que sea TLB MISS
    RECIBIR_VALOR_MEMORIA, // Para MOV_IN
    ESCRITURA_OK, // Para MOV_OUT
    RECURSOS_OK, // Para ver si sigue ejecutando el proceso o si esta bloqueado por falta de recursos
    OUT_OF_MEMORY, // Para manejar errores de Out of Memory

    // Memoria
    RECIBIR_PC_PID, // Recibe la instruccion a enviar de un proceso para CPU
    RECIBIR_TAMANIO, // Funcion RESIZE
    ESCRIBIR_MEMORIA, // Funciones MOV
    LEER_MEMORIA, // Funciones MOV
    NUMERO_PAGINA, // Para devolver el marco asociado a esa pagina
    FIN_PROCESO, // Se recibe de Kernel y se tienen que marcar los marcos como libres de ese proceso
    // COPY_STRING,
    // WAIT,
    // SIGNAL,

    // EntradaSalida (IO)
    RECIBIR_NOMBRE_IO,
    FIN_IO,
    IO_GEN_SLEEP,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_TRUNCATE,
    IO_FS_WRITE,
    IO_FS_READ,

    // Consola
    RECIBIR_PCB_FP // Recibir PCB si esta en Exec al hacer FINALIZAR_PROCESO
} op_code;

typedef enum {
    HANDSHAKE_KERNEL,
    HANDSHAKE_CPU_DISPATCH,
    HANDSHAKE_CPU_INTERRUPT,
    HANDSHAKE_MEMORIA,
} op_handshake;

// Estrucuta del buffer
typedef struct {
    uint32_t size;   // Tamaño del payload
    uint32_t offset; // Desplazamiento dentro del payload
    void* stream;    // Payload
} t_buffer;

// Estructura del paquete
typedef struct {
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

t_paquete* crear_paquete(op_code co_op, t_buffer* buffer);
void* serializar_paquete(t_paquete* paquete, int bytes);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);

// Tipos de datos
void cargar_string_al_buffer(t_buffer* buffer, char* string);
char* extraer_string_del_buffer(t_buffer* buffer);

void cargar_char_al_buffer(t_buffer* buffer, char caracter);
char extraer_char_del_buffer(t_buffer* buffer);

void cargar_uint8_al_buffer(t_buffer* buffer, uint8_t int8);
uint8_t extraer_uint8_del_buffer(t_buffer* buffer);

void cargar_uint32_al_buffer(t_buffer* buffer, uint32_t int32);
uint32_t extraer_uint32_del_buffer(t_buffer* buffer);

#endif