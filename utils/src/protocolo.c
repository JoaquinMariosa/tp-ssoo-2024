#include "../include/protocolo.h"

// Funciones de send y recv del protocolo
// Handshake
void send_handshake(t_log* logger, int fd, int32_t handshake, const char *conexion) {
    size_t bytes;
    int32_t result;

    bytes = send(fd, &handshake, sizeof(int32_t), 0);
    bytes = recv(fd, &result, sizeof(int32_t), MSG_WAITALL);

    if (result == 0 && bytes != 0) {
        log_info(logger, "Handshake OK de %s\n", conexion);
    }
    else {
        log_error(logger, "Handshake ERROR de %s\n", conexion);
    }
}

bool recv_handshake(int fd, int32_t handshakeModulo) {
    size_t bytes;
    int32_t handshake;
    int32_t resultOk = 0;
    int32_t resultError = -1;

    bytes = recv(fd, &handshake, sizeof(int32_t), MSG_WAITALL);
    if (handshake == handshakeModulo && bytes != 0) {
        bytes = send(fd, &resultOk, sizeof(int32_t), 0);
        return true;
    }
    else {
        bytes = send(fd, &resultError, sizeof(int32_t), 0);
        return false;
    }
}

// uint32_t
t_buffer* serializar_uint32(uint32_t entero) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size = sizeof(uint32_t);

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint32_al_buffer(buffer, entero);

    return buffer;
}

t_buffer* serializar_uint8(uint8_t entero) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size = sizeof(uint8_t);

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint8_al_buffer(buffer, entero);

    return buffer;
}

// PID
void send_pid(int fd, uint8_t pid) {
    t_buffer *buffer = serializar_uint8(pid);
    t_paquete *a_enviar = crear_paquete(RECIBIR_PID, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_pid(int fd, uint8_t* pid) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *pid = extraer_uint8_del_buffer(paquete->buffer);

    eliminar_paquete(paquete);

    return true;
}

// PID a borrar (EXIT)
void send_pid_a_borrar(int fd, uint8_t pid_a_borrar) {
    t_buffer *buffer = serializar_uint8(pid_a_borrar);
    t_paquete *a_enviar = crear_paquete(RECIBIR_PID_A_BORRAR, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_pid_a_borrar(int fd, uint8_t* pid_a_borrar) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *pid_a_borrar = extraer_uint8_del_buffer(paquete->buffer);

    eliminar_paquete(paquete);

    return true;
}

// Interrupcion (por conexion interrupt) --> a CPU
void send_interrupcion(int fd, uint8_t pid_a_interrumpir) {
    t_buffer *buffer = serializar_uint8(pid_a_interrumpir);
    t_paquete *a_enviar = crear_paquete(RECIBIR_PID_A_INTERRUMPIR, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_interrupcion(int fd, uint8_t* pid_a_interrumpir) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *pid_a_interrumpir = extraer_uint8_del_buffer(paquete->buffer);

    eliminar_paquete(paquete);

    return true;
}

// send y recv pc y pid (juntos) --> para fetch
t_buffer* serializar_pc_pid(uint32_t pc, uint8_t pid) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size = 
        sizeof(uint32_t) +  // PC
        sizeof(uint8_t);    // PID

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint32_al_buffer(buffer, pc);
    cargar_uint8_al_buffer(buffer, pid);

    return buffer;
}

void send_pc_pid(int fd, uint32_t pc, uint8_t pid) {
    t_buffer *buffer = serializar_pc_pid(pc, pid);
    t_paquete *a_enviar = crear_paquete(RECIBIR_PC_PID, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_pc_pid(int fd, uint32_t* pc, uint8_t* pid) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *pc = extraer_uint32_del_buffer(paquete->buffer);
    *pid = extraer_uint8_del_buffer(paquete->buffer);

    eliminar_paquete(paquete);

    return true;
}

// Size instrucciones
void send_size_instrucciones(int fd, uint8_t size) {
    t_buffer *buffer = serializar_uint8(size);
    t_paquete *a_enviar = crear_paquete(RECIBIR_SIZE_INSTRUCCIONES, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_size_instrucciones(int fd, uint8_t* size) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *size = extraer_uint8_del_buffer(paquete->buffer);

    eliminar_paquete(paquete);

    return true;
}

// PCB --> tambien utilizado como contexto de ejecucion
// Serializar PCB
t_buffer* serializar_pcb(t_pcb* pcb) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size =
        sizeof(uint8_t) +    // pid
        sizeof(uint32_t) +   // pc
        sizeof(char) +       // estado
        sizeof(uint32_t) +   // quantum
        sizeof(uint8_t) +    // flag_int
        sizeof(uint8_t) * 4 + sizeof(uint32_t) * 6; // tamaño de registros

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint8_al_buffer(buffer, pcb->pid);
    cargar_uint32_al_buffer(buffer, pcb->pc);
    cargar_char_al_buffer(buffer, pcb->estado);
    cargar_uint32_al_buffer(buffer, pcb->quantum);
    cargar_uint8_al_buffer(buffer, pcb->flag_int);

    // Registros
    cargar_uint8_al_buffer(buffer, pcb->registros->AX);
    cargar_uint8_al_buffer(buffer, pcb->registros->BX);
    cargar_uint8_al_buffer(buffer, pcb->registros->CX);
    cargar_uint8_al_buffer(buffer, pcb->registros->DX);
    cargar_uint32_al_buffer(buffer, pcb->registros->EAX);
    cargar_uint32_al_buffer(buffer, pcb->registros->EBX);
    cargar_uint32_al_buffer(buffer, pcb->registros->ECX);
    cargar_uint32_al_buffer(buffer, pcb->registros->EDX);
    cargar_uint32_al_buffer(buffer, pcb->registros->SI);
    cargar_uint32_al_buffer(buffer, pcb->registros->DI);

    return buffer;
}

// Deserializar PCB
t_pcb* deserializar_pcb(t_buffer *buffer, t_pcb *pcb) {
    // Datos del pcb
    pcb->pid = extraer_uint8_del_buffer(buffer);
    pcb->pc = extraer_uint32_del_buffer(buffer);
    pcb->estado = extraer_char_del_buffer(buffer);
    pcb->quantum = extraer_uint32_del_buffer(buffer);
    pcb->flag_int = extraer_uint8_del_buffer(buffer);

    // Registros
    pcb->registros->AX = extraer_uint8_del_buffer(buffer);
    pcb->registros->BX = extraer_uint8_del_buffer(buffer);
    pcb->registros->CX = extraer_uint8_del_buffer(buffer);
    pcb->registros->DX = extraer_uint8_del_buffer(buffer);
    pcb->registros->EAX = extraer_uint32_del_buffer(buffer);
    pcb->registros->EBX = extraer_uint32_del_buffer(buffer);
    pcb->registros->ECX = extraer_uint32_del_buffer(buffer);
    pcb->registros->EDX = extraer_uint32_del_buffer(buffer);
    pcb->registros->SI = extraer_uint32_del_buffer(buffer);
    pcb->registros->DI = extraer_uint32_del_buffer(buffer);

    return pcb;
}

void send_pcb(int fd, t_pcb *pcb) {
    t_buffer *buffer = serializar_pcb(pcb);
    t_paquete *a_enviar = crear_paquete(RECIBIR_PCB, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_pcb(int fd, t_pcb* pcb) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    deserializar_pcb(paquete->buffer, pcb);

    eliminar_paquete(paquete);

    return true;
}

// Char* (String)
t_buffer* serializar_string(char* string, uint32_t length) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size =
        sizeof(uint32_t) + // longitud del string
        length;            // string

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_string_al_buffer(buffer, string);

    return buffer;
}

char* deserializar_string(t_buffer* buffer) {
    char* string = extraer_string_del_buffer(buffer);
    return string;
}

// INICIAR_PROCESO
// Serializacion path y pid
t_buffer* serializar_iniciar_proceso(uint8_t pid, char* path, uint32_t length) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size =
        sizeof(uint8_t) +  // pid
        sizeof(uint32_t) + // longitud del path
        length;            // path

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint8_al_buffer(buffer, pid);
    cargar_string_al_buffer(buffer, path);

    return buffer;
}

void send_iniciar_proceso(int fd, uint8_t pid, char* path, uint32_t length) {
    t_buffer *buffer = serializar_iniciar_proceso(pid, path, length);
    t_paquete *a_enviar = crear_paquete(INICIAR_PROCESO, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_iniciar_proceso(int fd, uint8_t* pid, char* path) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);
    if (bytes_recibidos != paquete->buffer->size) {
        eliminar_paquete(paquete);
        return false;
    }

    *pid = extraer_uint8_del_buffer(paquete->buffer);

    char* received_path = deserializar_string(paquete->buffer);
    strcpy(path, received_path);
    free(received_path);

    eliminar_paquete(paquete);
    return true;
}

// Instruccion
void send_instruccion(int fd, char* instruccion, uint32_t length) {
    t_buffer *buffer = serializar_string(instruccion, length);
    t_paquete *a_enviar = crear_paquete(RECIBIR_INSTRUCCION, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_instruccion(int fd, char* instruccion) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    char* received_instruccion = deserializar_string(paquete->buffer);
    strcpy(instruccion, received_instruccion);
    free(received_instruccion);

    eliminar_paquete(paquete);

    return true;
}

// WAIT y SIGNAL
t_buffer* serializar_wait_signal(t_pcb* pcb_wait_signal, char* recurso, uint32_t length) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size =
        sizeof(uint8_t) +     // pid
        sizeof(uint32_t) +    // pc
        sizeof(char) +        // estado
        sizeof(uint32_t) +    // quantum
        sizeof(uint8_t) +     // flag_int
        sizeof(uint8_t) * 4 + sizeof(uint32_t) * 6 + // tamaño de registros
        sizeof(uint32_t) +    // longitud del recurso
        length;               // recurso

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint8_al_buffer(buffer, pcb_wait_signal->pid);
    cargar_uint32_al_buffer(buffer, pcb_wait_signal->pc);
    cargar_char_al_buffer(buffer, pcb_wait_signal->estado);
    cargar_uint32_al_buffer(buffer, pcb_wait_signal->quantum);
    cargar_uint8_al_buffer(buffer, pcb_wait_signal->flag_int);

    // Registros
    cargar_uint8_al_buffer(buffer, pcb_wait_signal->registros->AX);
    cargar_uint8_al_buffer(buffer, pcb_wait_signal->registros->BX);
    cargar_uint8_al_buffer(buffer, pcb_wait_signal->registros->CX);
    cargar_uint8_al_buffer(buffer, pcb_wait_signal->registros->DX);
    cargar_uint32_al_buffer(buffer, pcb_wait_signal->registros->EAX);
    cargar_uint32_al_buffer(buffer, pcb_wait_signal->registros->EBX);
    cargar_uint32_al_buffer(buffer, pcb_wait_signal->registros->ECX);
    cargar_uint32_al_buffer(buffer, pcb_wait_signal->registros->EDX);
    cargar_uint32_al_buffer(buffer, pcb_wait_signal->registros->SI);
    cargar_uint32_al_buffer(buffer, pcb_wait_signal->registros->DI);

    cargar_string_al_buffer(buffer, recurso);

    return buffer;
}

void send_wait(int fd, t_pcb* pcb_wait, char* recurso, uint32_t length) {
    t_buffer *buffer = serializar_wait_signal(pcb_wait, recurso, length);
    t_paquete *a_enviar = crear_paquete(WAIT, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

void send_signal(int fd, t_pcb* pcb_signal, char* recurso, uint32_t length) {
    t_buffer *buffer = serializar_wait_signal(pcb_signal, recurso, length);
    t_paquete *a_enviar = crear_paquete(SIGNAL, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_wait_signal(int fd, t_pcb* pcb_wait_signal, char* recurso) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    // Datos del pcb
    pcb_wait_signal->pid = extraer_uint8_del_buffer(paquete->buffer);
    pcb_wait_signal->pc = extraer_uint32_del_buffer(paquete->buffer);
    pcb_wait_signal->estado = extraer_char_del_buffer(paquete->buffer);
    pcb_wait_signal->quantum = extraer_uint32_del_buffer(paquete->buffer);
    pcb_wait_signal->flag_int = extraer_uint8_del_buffer(paquete->buffer);

    // Registros
    pcb_wait_signal->registros->AX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_wait_signal->registros->BX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_wait_signal->registros->CX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_wait_signal->registros->DX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_wait_signal->registros->EAX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_wait_signal->registros->EBX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_wait_signal->registros->ECX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_wait_signal->registros->EDX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_wait_signal->registros->SI = extraer_uint32_del_buffer(paquete->buffer);
    pcb_wait_signal->registros->DI = extraer_uint32_del_buffer(paquete->buffer);

    char* received_recurso = deserializar_string(paquete->buffer);
    strcpy(recurso, received_recurso);
    free(received_recurso);

    eliminar_paquete(paquete);

    return true;
}

// RECURSOS_OK --> Notifica si hay recursos disponibles para que siga ejecutando
void send_recursos_ok(int fd, uint8_t recursos_ok) {
    t_buffer *buffer = serializar_uint8(recursos_ok);
    t_paquete *a_enviar = crear_paquete(RECURSOS_OK, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_recursos_ok(int fd, uint8_t* recursos_ok) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *recursos_ok = extraer_uint8_del_buffer(paquete->buffer);

    eliminar_paquete(paquete);

    return true;
}

// TAM_PAGINA
void send_tam_pagina(int fd, uint32_t tam_pagina) {
    t_buffer *buffer = serializar_uint32(tam_pagina);
    t_paquete *a_enviar = crear_paquete(RECIBIR_TAM_PAGINA, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_tam_pagina(int fd, uint32_t* tam_pagina) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *tam_pagina = extraer_uint32_del_buffer(paquete->buffer);

    eliminar_paquete(paquete);

    return true;
}

// Resize
void send_tamanio(int fd, uint32_t tamanio, uint8_t pid_resize) {
    t_buffer *buffer = serializar_pc_pid(tamanio, pid_resize); // Dice pc y pid pero sirve para esta serializacion
    t_paquete *a_enviar = crear_paquete(RECIBIR_TAMANIO, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_tamanio(int fd, uint32_t* tamanio, uint8_t* pid_resize) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *tamanio = extraer_uint32_del_buffer(paquete->buffer);
    *pid_resize = extraer_uint8_del_buffer(paquete->buffer);

    eliminar_paquete(paquete);

    return true;
}

// Out of Memory
void send_out_of_memory(int fd, uint8_t pid_oom) {
    t_buffer *buffer = serializar_uint8(pid_oom);
    t_paquete *a_enviar = crear_paquete(OUT_OF_MEMORY, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_out_of_memory(int fd, uint8_t* pid_oom) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *pid_oom = extraer_uint8_del_buffer(paquete->buffer);

    eliminar_paquete(paquete);

    return true;
}

// Escribir memoria
t_buffer* serializar_escribir_memoria(uint8_t pid_a_escribir, uint32_t direccion_fisica, void* datos, uint32_t tamanio_a_escribir) {
    t_buffer* buffer = malloc(sizeof(t_buffer));

    buffer->size = 
        sizeof(uint8_t) +      // pid_a_escribir
        sizeof(uint32_t) +     // direccion_fisica
        sizeof(uint32_t) +     // tamanio_a_escribir
        tamanio_a_escribir;    // datos

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint8_al_buffer(buffer, pid_a_escribir);
    cargar_uint32_al_buffer(buffer, direccion_fisica);
    cargar_uint32_al_buffer(buffer, tamanio_a_escribir);

    // Cargo datos (void*)
    memcpy(buffer->stream + buffer->offset, datos, tamanio_a_escribir);
    buffer->offset += tamanio_a_escribir;

    return buffer;
}

void send_escribir_memoria(int fd, uint8_t pid_a_escribir, uint32_t direccion_fisica, void* datos, uint32_t tamanio_a_escribir) {
    t_buffer* buffer = serializar_escribir_memoria(pid_a_escribir, direccion_fisica, datos, tamanio_a_escribir);
    t_paquete* paquete = crear_paquete(ESCRIBIR_MEMORIA, buffer);
    enviar_paquete(paquete, fd);
    eliminar_paquete(paquete);
}

bool recv_escribir_memoria(int fd, uint8_t* pid, uint32_t* direccion_fisica, void** datos, uint32_t* tamanio_a_escribir) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error al recibir tamaño del buffer: %d bytes", bytes_recibidos);
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);
    if (bytes_recibidos != paquete->buffer->size) {
        eliminar_paquete(paquete);
        return false;
    }

    *pid = extraer_uint8_del_buffer(paquete->buffer);
    *direccion_fisica = extraer_uint32_del_buffer(paquete->buffer);
    *tamanio_a_escribir = extraer_uint32_del_buffer(paquete->buffer);
    
    // Extraer datos (void*)
    *datos = malloc(*tamanio_a_escribir);
    memcpy(*datos, paquete->buffer->stream + paquete->buffer->offset, *tamanio_a_escribir);

    eliminar_paquete(paquete);
    return true;
}

// Leer memoria
t_buffer* serializar_leer_memoria(uint8_t pid_a_leer, uint32_t direccion_fisica, uint32_t tamanio_a_leer) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size =
        sizeof(uint8_t) +    // pid_a_leer
        sizeof(uint32_t) +   // direccion_fisica
        sizeof(uint32_t);    // tamanio_a_leer

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint8_al_buffer(buffer, pid_a_leer);
    cargar_uint32_al_buffer(buffer, direccion_fisica);
    cargar_uint32_al_buffer(buffer, tamanio_a_leer);

    return buffer;
}

void send_leer_memoria(int fd, uint8_t pid_a_leer, uint32_t direccion_fisica, uint32_t tamanio_a_leer) {
    t_buffer *buffer = serializar_leer_memoria(pid_a_leer, direccion_fisica, tamanio_a_leer);
    t_paquete *a_enviar = crear_paquete(LEER_MEMORIA, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_leer_memoria(int fd, uint8_t* pid_a_leer, uint32_t* direccion_fisica, uint32_t* tamanio_a_leer) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *pid_a_leer = extraer_uint8_del_buffer(paquete->buffer);
    *direccion_fisica = extraer_uint32_del_buffer(paquete->buffer);
    *tamanio_a_leer = extraer_uint32_del_buffer(paquete->buffer);

    eliminar_paquete(paquete);

    return true;
}

// Numero de pagina o marco --> desplazamiento seria num de marco en RECIBIR_MARCO --> pero se puede reutilizar
t_buffer* serializar_pagina_marco(uint8_t pid, uint32_t num_pagina, uint32_t desplazamiento) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size =
        sizeof(uint8_t) +     // pid
        sizeof(uint32_t) +    // num_pagina
        sizeof(uint32_t);     // desplazamiento

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint8_al_buffer(buffer, pid);
    cargar_uint32_al_buffer(buffer, num_pagina);
    cargar_uint32_al_buffer(buffer, desplazamiento);

    return buffer;
}

void send_num_pagina(int fd, uint8_t pid, uint32_t num_pagina, uint32_t desplazamiento) {
    t_buffer *buffer = serializar_pagina_marco(pid, num_pagina, desplazamiento);
    t_paquete *a_enviar = crear_paquete(NUMERO_PAGINA, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_num_pagina(int fd, uint8_t* pid, uint32_t* num_pagina, uint32_t* desplazamiento) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *pid = extraer_uint8_del_buffer(paquete->buffer);
    *num_pagina = extraer_uint32_del_buffer(paquete->buffer);
    *desplazamiento = extraer_uint32_del_buffer(paquete->buffer);

    eliminar_paquete(paquete);

    return true;
}

// RECIBIR_MARCO
void send_num_marco(int fd, uint8_t pid_marco, uint32_t numero_pagina, uint32_t marco) {
    t_buffer *buffer = serializar_pagina_marco(pid_marco, numero_pagina, marco);
    t_paquete *a_enviar = crear_paquete(RECIBIR_MARCO, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_num_marco(int fd, uint8_t* pid_marco, uint32_t* numero_pagina, uint32_t* marco) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *pid_marco = extraer_uint8_del_buffer(paquete->buffer);
    *numero_pagina = extraer_uint32_del_buffer(paquete->buffer);
    *marco = extraer_uint32_del_buffer(paquete->buffer);

    eliminar_paquete(paquete);

    return true;
}


// RECIBIR_VALOR_MEMORIA
t_buffer* serializar_valor_memoria(uint32_t direccion_fisica, void* valor, uint32_t tam_dato) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size =
        sizeof(uint32_t) +     // direccion_fisica
        sizeof(uint32_t) +     // tam_dato
        tam_dato;              // valor

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint32_al_buffer(buffer, direccion_fisica);
    cargar_uint32_al_buffer(buffer, tam_dato);

    // Cargo datos (void*)
    memcpy(buffer->stream + buffer->offset, valor, tam_dato);
    buffer->offset += tam_dato;

    return buffer;
}

void send_valor_memoria(int fd, uint32_t direccion_fisica, void* valor, uint32_t tam_dato) {
    t_buffer *buffer = serializar_valor_memoria(direccion_fisica, valor, tam_dato);
    t_paquete *a_enviar = crear_paquete(RECIBIR_VALOR_MEMORIA, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_valor_memoria(int fd, uint32_t* direccion_fisica, void** valor, uint32_t* tam_dato) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *direccion_fisica = extraer_uint32_del_buffer(paquete->buffer);
    *tam_dato = extraer_uint32_del_buffer(paquete->buffer);

    // Extraer datos (void*)
    *valor = malloc(*tam_dato);
    memcpy(*valor, paquete->buffer->stream + paquete->buffer->offset, *tam_dato);

    eliminar_paquete(paquete);

    return true;
}

// ESCRITURA_OK --> para MOV_OUT
void send_escritura_ok(int fd, uint8_t escritura_ok) {
    t_buffer *buffer = serializar_uint8(escritura_ok);
    t_paquete *a_enviar = crear_paquete(ESCRITURA_OK, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_escritura_ok(int fd, uint8_t* escritura_ok) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *escritura_ok = extraer_uint8_del_buffer(paquete->buffer);

    eliminar_paquete(paquete);

    return true;
}

// FIN_PROCESO
void send_fin_proceso(int fd, uint8_t pid_fin) {
    t_buffer *buffer = serializar_uint8(pid_fin);
    t_paquete *a_enviar = crear_paquete(FIN_PROCESO, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_fin_proceso(int fd, uint8_t* pid_fin) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *pid_fin = extraer_uint8_del_buffer(paquete->buffer);

    eliminar_paquete(paquete);

    return true;
}

// Interfaces IO
// Envio de nombre de 
t_buffer* serializar_dos_string(char* string1, uint32_t length1, char* string2, uint32_t length2) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size =
        sizeof(uint32_t) + // longitud del string1
        sizeof(uint32_t) + // longitud del string2
        length1 +          // string1
        length2;           // string2

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_string_al_buffer(buffer, string1);
    cargar_string_al_buffer(buffer, string2);

    return buffer;
}

void send_interfaz(int fd, char* nombre_interfaz, uint32_t length, char* tipo_interfaz, uint32_t length_tipo) {
    t_buffer *buffer = serializar_dos_string(nombre_interfaz, length, tipo_interfaz, length_tipo);
    t_paquete *a_enviar = crear_paquete(RECIBIR_NOMBRE_IO, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_interfaz(int fd, char* nombre_interfaz, char* tipo_interfaz) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    char* nombre_interfaz_recibido = deserializar_string(paquete->buffer);
    strcpy(nombre_interfaz, nombre_interfaz_recibido);
    free(nombre_interfaz_recibido);

    char* tipo_interfaz_recibido = deserializar_string(paquete->buffer);
    strcpy(tipo_interfaz, tipo_interfaz_recibido);
    free(tipo_interfaz_recibido);

    eliminar_paquete(paquete);

    return true;
}

// Fin de IO
t_buffer* serializar_fin_io(uint8_t pid_fin_io, char* nombre, uint32_t length) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size =
        sizeof(uint8_t) +     // pid_fin_io
        sizeof(uint32_t) +    // longitud del nombre
        length;               // nombre


    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint8_al_buffer(buffer, pid_fin_io);
    cargar_string_al_buffer(buffer, nombre);

    return buffer;
}

void send_fin_io(int fd, uint8_t pid_fin_io, char* nombre, uint32_t length) {
    t_buffer *buffer = serializar_fin_io(pid_fin_io, nombre, length);
    t_paquete *a_enviar = crear_paquete(FIN_IO, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_fin_io(int fd, uint8_t* pid_fin_io, char* nombre) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    *pid_fin_io = extraer_uint8_del_buffer(paquete->buffer);

    char* received_nombre = deserializar_string(paquete->buffer);
    strcpy(nombre, received_nombre);
    free(received_nombre);

    eliminar_paquete(paquete);

    return true;
}

// IO_GEN_SLEEP
t_buffer* serializar_io_gen_sleep(t_pcb* pcb_io, uint32_t unidades_trabajo, char* nombre, uint32_t length) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size =
        sizeof(uint8_t) +     // pid
        sizeof(uint32_t) +    // pc
        sizeof(char) +        // estado
        sizeof(uint32_t) +    // quantum
        sizeof(uint8_t) +     // flag_int
        sizeof(uint8_t) * 4 + sizeof(uint32_t) * 6 + // tamaño de registros
        sizeof(uint32_t) +    // unidades_trabajo
        sizeof(uint32_t) +    // longitud del nombre
        length;               // nombre

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint8_al_buffer(buffer, pcb_io->pid);
    cargar_uint32_al_buffer(buffer, pcb_io->pc);
    cargar_char_al_buffer(buffer, pcb_io->estado);
    cargar_uint32_al_buffer(buffer, pcb_io->quantum);
    cargar_uint8_al_buffer(buffer, pcb_io->flag_int);

    // Registros
    cargar_uint8_al_buffer(buffer, pcb_io->registros->AX);
    cargar_uint8_al_buffer(buffer, pcb_io->registros->BX);
    cargar_uint8_al_buffer(buffer, pcb_io->registros->CX);
    cargar_uint8_al_buffer(buffer, pcb_io->registros->DX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->EAX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->EBX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->ECX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->EDX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->SI);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->DI);

    cargar_uint32_al_buffer(buffer, unidades_trabajo);
    cargar_string_al_buffer(buffer, nombre);

    return buffer;
}

void send_io_gen_sleep(int fd, t_pcb* pcb_io, uint32_t unidades_trabajo, char* nombre, uint32_t length) {
    t_buffer *buffer = serializar_io_gen_sleep(pcb_io, unidades_trabajo, nombre, length);
    t_paquete *a_enviar = crear_paquete(IO_GEN_SLEEP, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_io_gen_sleep(int fd, t_pcb* pcb_io, uint32_t* unidades_trabajo, char* nombre) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    // Datos del pcb
    pcb_io->pid = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->pc = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->estado = extraer_char_del_buffer(paquete->buffer);
    pcb_io->quantum = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->flag_int = extraer_uint8_del_buffer(paquete->buffer);

    // Registros
    pcb_io->registros->AX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->BX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->CX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->DX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->EAX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->EBX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->ECX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->EDX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->SI = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->DI = extraer_uint32_del_buffer(paquete->buffer);

    *unidades_trabajo = extraer_uint32_del_buffer(paquete->buffer);

    char* received_nombre = deserializar_string(paquete->buffer);
    strcpy(nombre, received_nombre);
    free(received_nombre);

    eliminar_paquete(paquete);

    return true;
}

// IO_STDIN_READ y IO_STDOUT_WRITE
// Serializacion sirve para las dos
// recv tambien
t_buffer* serializar_io_stdin_stdout(t_pcb* pcb_io, uint32_t direccion_fisica, uint32_t tamanio_maximo, char* nombre, uint32_t length) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size =
        sizeof(uint8_t) +     // pid
        sizeof(uint32_t) +    // pc
        sizeof(char) +        // estado
        sizeof(uint32_t) +    // quantum
        sizeof(uint8_t) +     // flag_int
        sizeof(uint8_t) * 4 + sizeof(uint32_t) * 6 + // tamaño de registros
        sizeof(uint32_t) +    // direccion_fisica
        sizeof(uint32_t) +    // tamanio_maximo
        sizeof(uint32_t) +    // longitud del nombre
        length;               // nombre

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint8_al_buffer(buffer, pcb_io->pid);
    cargar_uint32_al_buffer(buffer, pcb_io->pc);
    cargar_char_al_buffer(buffer, pcb_io->estado);
    cargar_uint32_al_buffer(buffer, pcb_io->quantum);
    cargar_uint8_al_buffer(buffer, pcb_io->flag_int);

    // Registros
    cargar_uint8_al_buffer(buffer, pcb_io->registros->AX);
    cargar_uint8_al_buffer(buffer, pcb_io->registros->BX);
    cargar_uint8_al_buffer(buffer, pcb_io->registros->CX);
    cargar_uint8_al_buffer(buffer, pcb_io->registros->DX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->EAX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->EBX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->ECX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->EDX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->SI);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->DI);

    cargar_uint32_al_buffer(buffer, direccion_fisica);
    cargar_uint32_al_buffer(buffer, tamanio_maximo);
    cargar_string_al_buffer(buffer, nombre);

    return buffer;
}

void send_io_stdin_read(int fd, t_pcb* pcb_io, uint32_t direccion_fisica, uint32_t tamanio_maximo, char* nombre, uint32_t length) {
    t_buffer *buffer = serializar_io_stdin_stdout(pcb_io, direccion_fisica, tamanio_maximo, nombre, length);
    t_paquete *a_enviar = crear_paquete(IO_STDIN_READ, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

void send_io_stdout_write(int fd, t_pcb* pcb_io, uint32_t direccion_fisica, uint32_t tamanio_maximo, char* nombre, uint32_t length) {
    t_buffer *buffer = serializar_io_stdin_stdout(pcb_io, direccion_fisica, tamanio_maximo, nombre, length);
    t_paquete *a_enviar = crear_paquete(IO_STDOUT_WRITE, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_io_stdin_stdout(int fd, t_pcb* pcb_io, uint32_t* direccion_fisica, uint32_t* tamanio_maximo, char* nombre) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    // Datos del pcb
    pcb_io->pid = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->pc = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->estado = extraer_char_del_buffer(paquete->buffer);
    pcb_io->quantum = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->flag_int = extraer_uint8_del_buffer(paquete->buffer);

    // Registros
    pcb_io->registros->AX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->BX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->CX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->DX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->EAX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->EBX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->ECX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->EDX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->SI = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->DI = extraer_uint32_del_buffer(paquete->buffer);

    *direccion_fisica = extraer_uint32_del_buffer(paquete->buffer);
    *tamanio_maximo = extraer_uint32_del_buffer(paquete->buffer);

    char* received_nombre = deserializar_string(paquete->buffer);
    strcpy(nombre, received_nombre);
    free(received_nombre);

    eliminar_paquete(paquete);

    return true;
}

// FS
// IO_FS_CREATE y IO_FS_DELETE
t_buffer* serializar_io_fs_create_delete(t_pcb* pcb_io, char* nombre_archivo, uint32_t length_nombre_archivo, char* interfaz, uint32_t length_interfaz) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size =
        sizeof(uint8_t) +     // pid
        sizeof(uint32_t) +    // pc
        sizeof(char) +        // estado
        sizeof(uint32_t) +    // quantum
        sizeof(uint8_t) +     // flag_int
        sizeof(uint8_t) * 4 + sizeof(uint32_t) * 6 + // tamaño de registros
        sizeof(uint32_t) +    // longitud del nombre_archivo
        length_nombre_archivo + // nombre_archivo
        sizeof(uint32_t) +    // longitud del interfaz
        length_interfaz;      // interfaz

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint8_al_buffer(buffer, pcb_io->pid);
    cargar_uint32_al_buffer(buffer, pcb_io->pc);
    cargar_char_al_buffer(buffer, pcb_io->estado);
    cargar_uint32_al_buffer(buffer, pcb_io->quantum);
    cargar_uint8_al_buffer(buffer, pcb_io->flag_int);

    // Registros
    cargar_uint8_al_buffer(buffer, pcb_io->registros->AX);
    cargar_uint8_al_buffer(buffer, pcb_io->registros->BX);
    cargar_uint8_al_buffer(buffer, pcb_io->registros->CX);
    cargar_uint8_al_buffer(buffer, pcb_io->registros->DX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->EAX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->EBX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->ECX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->EDX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->SI);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->DI);

    cargar_string_al_buffer(buffer, nombre_archivo);
    cargar_string_al_buffer(buffer, interfaz);

    return buffer;
}

void send_io_fs_create(int fd, t_pcb* pcb_io, char* nombre_archivo, uint32_t length_nombre_archivo, char* interfaz, uint32_t length_interfaz) {
    t_buffer *buffer = serializar_io_fs_create_delete(pcb_io, nombre_archivo, length_nombre_archivo, interfaz, length_interfaz);
    t_paquete *a_enviar = crear_paquete(IO_FS_CREATE, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

void send_io_fs_delete(int fd, t_pcb* pcb_io, char* nombre_archivo, uint32_t length_nombre_archivo, char* interfaz, uint32_t length_interfaz) {
    t_buffer *buffer = serializar_io_fs_create_delete(pcb_io, nombre_archivo, length_nombre_archivo, interfaz, length_interfaz);
    t_paquete *a_enviar = crear_paquete(IO_FS_DELETE, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_io_fs_create_delete(int fd, t_pcb* pcb_io, char* nombre_archivo, char* interfaz) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    // Datos del pcb
    pcb_io->pid = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->pc = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->estado = extraer_char_del_buffer(paquete->buffer);
    pcb_io->quantum = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->flag_int = extraer_uint8_del_buffer(paquete->buffer);

    // Registros
    pcb_io->registros->AX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->BX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->CX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->DX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->EAX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->EBX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->ECX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->EDX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->SI = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->DI = extraer_uint32_del_buffer(paquete->buffer);

    char* received_nombre_arch = deserializar_string(paquete->buffer);
    strcpy(nombre_archivo, received_nombre_arch);
    free(received_nombre_arch);

    char* received_intefaz = deserializar_string(paquete->buffer);
    strcpy(interfaz, received_intefaz);
    free(received_intefaz);

    eliminar_paquete(paquete);

    return true;
}

// IO_FS_TRUNCATE
t_buffer* serializar_io_fs_truncate(t_pcb* pcb_io, uint32_t tamanio, char* nombre_archivo, uint32_t length_nombre_archivo, char* interfaz, uint32_t length_interfaz) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size =
        sizeof(uint8_t) +     // pid
        sizeof(uint32_t) +    // pc
        sizeof(char) +        // estado
        sizeof(uint32_t) +    // quantum
        sizeof(uint8_t) +     // flag_int
        sizeof(uint8_t) * 4 + sizeof(uint32_t) * 6 + // tamaño de registros
        sizeof(uint32_t) +    // tamanio
        sizeof(uint32_t) +    // longitud del nombre_archivo
        length_nombre_archivo + // nombre_archivo
        sizeof(uint32_t) +    // longitud del interfaz
        length_interfaz;      // interfaz

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint8_al_buffer(buffer, pcb_io->pid);
    cargar_uint32_al_buffer(buffer, pcb_io->pc);
    cargar_char_al_buffer(buffer, pcb_io->estado);
    cargar_uint32_al_buffer(buffer, pcb_io->quantum);
    cargar_uint8_al_buffer(buffer, pcb_io->flag_int);

    // Registros
    cargar_uint8_al_buffer(buffer, pcb_io->registros->AX);
    cargar_uint8_al_buffer(buffer, pcb_io->registros->BX);
    cargar_uint8_al_buffer(buffer, pcb_io->registros->CX);
    cargar_uint8_al_buffer(buffer, pcb_io->registros->DX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->EAX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->EBX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->ECX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->EDX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->SI);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->DI);

    cargar_uint32_al_buffer(buffer, tamanio);
    cargar_string_al_buffer(buffer, nombre_archivo);
    cargar_string_al_buffer(buffer, interfaz);

    return buffer;
}

void send_io_fs_truncate(int fd, t_pcb* pcb_io, uint32_t tamanio, char* nombre_archivo, uint32_t length_nombre_archivo, char* interfaz, uint32_t length_interfaz) {
    t_buffer *buffer = serializar_io_fs_truncate(pcb_io, tamanio, nombre_archivo, length_nombre_archivo, interfaz, length_interfaz);
    t_paquete *a_enviar = crear_paquete(IO_FS_TRUNCATE, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_io_fs_truncate(int fd, t_pcb* pcb_io, uint32_t* tamanio, char* nombre_archivo, char* interfaz) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    // Datos del pcb
    pcb_io->pid = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->pc = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->estado = extraer_char_del_buffer(paquete->buffer);
    pcb_io->quantum = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->flag_int = extraer_uint8_del_buffer(paquete->buffer);

    // Registros
    pcb_io->registros->AX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->BX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->CX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->DX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->EAX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->EBX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->ECX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->EDX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->SI = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->DI = extraer_uint32_del_buffer(paquete->buffer);

    *tamanio = extraer_uint32_del_buffer(paquete->buffer);

    char* received_nombre_arch = deserializar_string(paquete->buffer);
    strcpy(nombre_archivo, received_nombre_arch);
    free(received_nombre_arch);

    char* received_intefaz = deserializar_string(paquete->buffer);
    strcpy(interfaz, received_intefaz);
    free(received_intefaz);

    eliminar_paquete(paquete);

    return true;
}

// IO_FS_WRITE y IO_FS_READ
t_buffer* serializar_io_fs_write_read(t_pcb* pcb_io, uint32_t tamanio, uint32_t direccion_fisica, uint32_t puntero_archivo, char* nombre_archivo, uint32_t length_nombre_archivo, char* interfaz, uint32_t length_interfaz) {
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size =
        sizeof(uint8_t) +     // pid
        sizeof(uint32_t) +    // pc
        sizeof(char) +        // estado
        sizeof(uint32_t) +    // quantum
        sizeof(uint8_t) +     // flag_int
        sizeof(uint8_t) * 4 + sizeof(uint32_t) * 6 + // tamaño de registros
        sizeof(uint32_t) +    // tamanio
        sizeof(uint32_t) +    // direccion_fisica
        sizeof(uint32_t) +    // puntero_archivo
        sizeof(uint32_t) +    // longitud del nombre_archivo
        length_nombre_archivo + // nombre_archivo
        sizeof(uint32_t) +    // longitud del interfaz
        length_interfaz;      // interfaz

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    cargar_uint8_al_buffer(buffer, pcb_io->pid);
    cargar_uint32_al_buffer(buffer, pcb_io->pc);
    cargar_char_al_buffer(buffer, pcb_io->estado);
    cargar_uint32_al_buffer(buffer, pcb_io->quantum);
    cargar_uint8_al_buffer(buffer, pcb_io->flag_int);

    // Registros
    cargar_uint8_al_buffer(buffer, pcb_io->registros->AX);
    cargar_uint8_al_buffer(buffer, pcb_io->registros->BX);
    cargar_uint8_al_buffer(buffer, pcb_io->registros->CX);
    cargar_uint8_al_buffer(buffer, pcb_io->registros->DX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->EAX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->EBX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->ECX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->EDX);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->SI);
    cargar_uint32_al_buffer(buffer, pcb_io->registros->DI);

    cargar_uint32_al_buffer(buffer, tamanio);
    cargar_uint32_al_buffer(buffer, direccion_fisica);
    cargar_uint32_al_buffer(buffer, puntero_archivo);
    cargar_string_al_buffer(buffer, nombre_archivo);
    cargar_string_al_buffer(buffer, interfaz);

    return buffer;
}

void send_io_fs_write(int fd, t_pcb* pcb_io, uint32_t tamanio, uint32_t direccion_fisica, uint32_t puntero_archivo, char* nombre_archivo, uint32_t length_nombre_archivo, char* interfaz, uint32_t length_interfaz) {
    t_buffer *buffer = serializar_io_fs_write_read(pcb_io, tamanio, direccion_fisica, puntero_archivo, nombre_archivo, length_nombre_archivo, interfaz, length_interfaz);
    t_paquete *a_enviar = crear_paquete(IO_FS_WRITE, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

void send_io_fs_read(int fd, t_pcb* pcb_io, uint32_t tamanio, uint32_t direccion_fisica, uint32_t puntero_archivo, char* nombre_archivo, uint32_t length_nombre_archivo, char* interfaz, uint32_t length_interfaz) {
    t_buffer *buffer = serializar_io_fs_write_read(pcb_io, tamanio, direccion_fisica, puntero_archivo, nombre_archivo, length_nombre_archivo, interfaz, length_interfaz);
    t_paquete *a_enviar = crear_paquete(IO_FS_READ, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_io_fs_write_read(int fd, t_pcb* pcb_io, uint32_t* tamanio, uint32_t* direccion_fisica, uint32_t* puntero_archivo, char* nombre_archivo, char* interfaz) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    // Datos del pcb
    pcb_io->pid = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->pc = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->estado = extraer_char_del_buffer(paquete->buffer);
    pcb_io->quantum = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->flag_int = extraer_uint8_del_buffer(paquete->buffer);

    // Registros
    pcb_io->registros->AX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->BX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->CX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->DX = extraer_uint8_del_buffer(paquete->buffer);
    pcb_io->registros->EAX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->EBX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->ECX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->EDX = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->SI = extraer_uint32_del_buffer(paquete->buffer);
    pcb_io->registros->DI = extraer_uint32_del_buffer(paquete->buffer);

    *tamanio = extraer_uint32_del_buffer(paquete->buffer);
    *direccion_fisica = extraer_uint32_del_buffer(paquete->buffer);
    *puntero_archivo = extraer_uint32_del_buffer(paquete->buffer);

    char* received_nombre_arch = deserializar_string(paquete->buffer);
    strcpy(nombre_archivo, received_nombre_arch);
    free(received_nombre_arch);

    char* received_intefaz = deserializar_string(paquete->buffer);
    strcpy(interfaz, received_intefaz);
    free(received_intefaz);

    eliminar_paquete(paquete);

    return true;
}

// PCB al FINALIZAR_PROCESO
void send_pcb_fp(int fd, t_pcb* pcb_fp) {
    t_buffer *buffer = serializar_pcb(pcb_fp);
    t_paquete *a_enviar = crear_paquete(RECIBIR_PCB_FP, buffer);
    enviar_paquete(a_enviar, fd);
    eliminar_paquete(a_enviar);
}

bool recv_pcb_fp(int fd, t_pcb* pcb_fp) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->offset = 0;

    // Control para recibir el buffer
    int bytes_recibidos = recv(fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if (bytes_recibidos != sizeof(uint32_t)) {
        printf("Error: No se recibió el tamaño del buffer completo\n");
        free(paquete->buffer);
        free(paquete);
        return false;
    }

    paquete->buffer->stream = malloc(paquete->buffer->size);
    bytes_recibidos = recv(fd, paquete->buffer->stream, paquete->buffer->size, 0);

    deserializar_pcb(paquete->buffer, pcb_fp);

    eliminar_paquete(paquete);

    return true;
}