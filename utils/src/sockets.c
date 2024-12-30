#include "../include/sockets.h"

// Estas son solo las funciones entre cliente y servidor

int crear_conexion(t_log* logger, char *ip, char* puerto, char* modulo) {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    // Control de socket
    if(socket_cliente == -1) {
        log_error(logger, "Hubo un error en la creacion del socket %s:%s en el modulo %s", ip, puerto, modulo);
    }

    // Control de conexion
    if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1) {
        log_error(logger, "Hubo un error en la conexion");
    } else {
        log_info(logger, "Cliente conectado en %s:%s del modulo %s", ip, puerto, modulo);
    }

	freeaddrinfo(server_info);

	return socket_cliente;
}

int iniciar_servidor(t_log* logger, char* puerto, char* modulo) {
    int socket_servidor;
    struct addrinfo hints, *server_info;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, puerto, &hints, &server_info);

    socket_servidor = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if(setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR)failed");
    }
    bind(socket_servidor, server_info->ai_addr, server_info->ai_addrlen);
    
    listen(socket_servidor, SOMAXCONN); 

    // Aviso al logger que el servidor se inicio
    log_info(logger, "Servidor iniciado en el puerto %s del modulo %s\n", puerto, modulo);

    freeaddrinfo(server_info);

    return socket_servidor;
}

int esperar_cliente(t_log* logger, int socket_servidor, char* modulo) {
    int socket_cliente = accept(socket_servidor, NULL, NULL);
    log_info(logger, "Se conecto el cliente del modulo %s", modulo);

    return socket_cliente;
}

int recibir_operacion(int socket_cliente) {
    int cod_op;
    if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0) {
        return cod_op;
    } else {
        close(socket_cliente);
        return -1;
    }
}

void liberar_conexion(int socket_cliente) {
    close(socket_cliente);
}