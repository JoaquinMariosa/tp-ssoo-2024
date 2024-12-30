#include "../include/funciones-serializacion.h"

t_paquete* crear_paquete(op_code cod_op, t_buffer* buffer) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = cod_op;
    paquete->buffer = buffer;
	return paquete;
}

void* serializar_paquete(t_paquete* paquete, int bytes) {
    void * magic = malloc(bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento+= sizeof(int);
    memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    desplazamiento+= paquete->buffer->size;

    return magic;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void cargar_string_al_buffer(t_buffer* buffer, char* string) {
    // Agregar la longitud del string'
    uint32_t length = strlen(string) + 1;
    memcpy(buffer->stream + buffer->offset, &length, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
    // Agregar el string
    memcpy(buffer->stream + buffer->offset, string, length);
    buffer->offset += length;
}

char* extraer_string_del_buffer(t_buffer* buffer) {
    // Leer el largo del string
    uint32_t length;
    memcpy(&length, buffer->stream + buffer->offset, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);

    // Leer el string
    char* string = malloc(length);
    memcpy(string, buffer->stream + buffer->offset, length);
    buffer->offset += length;

    return string;
}

void cargar_char_al_buffer(t_buffer* buffer, char value) {
    memcpy(buffer->stream + buffer->offset, &value, sizeof(char));
    buffer->offset += sizeof(char);
}

char extraer_char_del_buffer(t_buffer* buffer) {
    char value;
    memcpy(&value, buffer->stream + buffer->offset, sizeof(char));
    buffer->offset += sizeof(char);
    return value;
}

void cargar_uint8_al_buffer(t_buffer* buffer, uint8_t value) {
    memcpy(buffer->stream + buffer->offset, &value, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);
}

uint8_t extraer_uint8_del_buffer(t_buffer* buffer) {
    uint8_t value;
    memcpy(&value, buffer->stream + buffer->offset, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);
    return value;
}

void cargar_uint32_al_buffer(t_buffer* buffer, uint32_t value) {
    memcpy(buffer->stream + buffer->offset, &value, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
}

uint32_t extraer_uint32_del_buffer(t_buffer* buffer) {
    uint32_t value;
    memcpy(&value, buffer->stream + buffer->offset, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
    return value;
}