#include "../include/fs.h"

void init_bloques(char* path) {
    // Calcular el tamaño total del archivo
    uint32_t file_size = BLOCK_SIZE * BLOCK_COUNT;
    
    FILE* archivo_bloques = fopen(path, "rb+");
    if (archivo_bloques != NULL) {
        // El archivo ya existe
        ftruncate(fileno(archivo_bloques), file_size);
        fseek(archivo_bloques, file_size - 1, SEEK_SET);
        fputc('\0', archivo_bloques);

        fclose(archivo_bloques);
        log_info(entradasalida_logger, "El archivo bloques.dat ya existe y esta inicializado.");
        return;
    }

    // Si no existe se crea
    archivo_bloques = fopen(path, "wb+");
    if (archivo_bloques == NULL) {
        log_error(entradasalida_logger, "Error al abrir el archivo bloques.dat");
        exit(EXIT_FAILURE);
    }

    ftruncate(fileno(archivo_bloques), file_size);
    fseek(archivo_bloques, file_size - 1, SEEK_SET);
    fputc('\0', archivo_bloques);

    fclose(archivo_bloques);

    log_info(entradasalida_logger, "Archivo bloques.dat inicializado correctamente.");
}

void init_bitmap(char* path) {
    FILE* arch_bitmap = fopen(path, "rb+");
    if (arch_bitmap != NULL) {
        // Usar mmap para mapear el archivo a memoria
        void* arch_bitmap_mapped = mmap(NULL, BLOCK_COUNT / 8, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(arch_bitmap), 0);
        if (arch_bitmap_mapped == MAP_FAILED) {
            log_error(entradasalida_logger, "Error al mapear el archivo bitmap.dat.");
            fclose(arch_bitmap);
            return;
        }

        // Crear el bitarray sin reinicializar
        bitmap_blocks = bitarray_create_with_mode(arch_bitmap_mapped, BLOCK_COUNT / 8, LSB_FIRST);

        fclose(arch_bitmap);
        log_info(entradasalida_logger, "El archivo bitmap.dat ya existe y esta inicializado.");
        return;
    }

    // Si no existe se crea
    arch_bitmap = fopen(path, "wb+");
    if (arch_bitmap == NULL) {
        log_error(entradasalida_logger, "Error al abrir el archivo bitmap.dat");
        exit(EXIT_FAILURE);
    }

    // Redimensionar el archivo para mmap
    fflush(arch_bitmap);
    ftruncate(fileno(arch_bitmap), BLOCK_COUNT / 8);

    // Usar mmap para mapear el archivo a memoria
    void* arch_bitmap_mapped = mmap(NULL, BLOCK_COUNT / 8, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(arch_bitmap), 0);
    if (arch_bitmap_mapped == MAP_FAILED) {
        log_error(entradasalida_logger, "Error al mapear el archivo.");
        fclose(arch_bitmap);
        exit(EXIT_FAILURE);
    }

    memset(arch_bitmap_mapped, 0, BLOCK_COUNT / 8);
    msync(arch_bitmap_mapped, BLOCK_COUNT / 8, MS_SYNC);

    // Para marcar bloques libres u ocupados
    bitmap_blocks = bitarray_create_with_mode(arch_bitmap_mapped, BLOCK_COUNT / 8, LSB_FIRST);

    // munmap(arch_bitmap_mapped, BLOCK_COUNT / 8); // ?
    fclose(arch_bitmap);

    log_info(entradasalida_logger, "Archivo bitmap.dat inicializado correctamente.");
}

// Terminar bitmap
void terminar_bitmap() {
    if (bitmap_blocks) {
        munmap(bitmap_blocks->bitarray, BLOCK_COUNT / 8);
        bitarray_destroy(bitmap_blocks);
        log_info(entradasalida_logger, "Bitarray y mapeo de memoria liberados correctamente.");
    }
}

// Crear archivo
int primer_bloque_libre(t_bitarray* bitmap_bloques) {
    for (int i = 0; i < bitarray_get_max_bit(bitmap_bloques); i++) {
        if (!bitarray_test_bit(bitmap_bloques, i)) {
            bitarray_set_bit(bitmap_bloques, i);
            return i;
        }
    }
    return -1; // Si no se encuentra un bloque libre
}

void crear_archivo(char* nombre, t_bitarray* bitmap_bloques) {
    // Concatenar nombre con PATH_BASE_DIALFS
    uint32_t MAX_LENGTH = 256;
    char* path_archivo_a_crear = malloc(MAX_LENGTH);
    strcpy(path_archivo_a_crear, PATH_BASE_DIALFS);
    strcat(path_archivo_a_crear, "/");
    strcat(path_archivo_a_crear, nombre);

    int primer_bloque = primer_bloque_libre(bitmap_bloques);

    if (primer_bloque == -1) {
        log_error(entradasalida_logger, "Error: No se encontraron bloques libres.");
        return;
    }

    // Crear archivo de metadata
    FILE *metadata = fopen(path_archivo_a_crear, "w");
    if (!metadata) {
        log_error(entradasalida_logger, "Error: No se pudo crear el archivo de metadata.");
        return;
    }

    // Escribir bloque inicial y tamaño en el archivo
    fprintf(metadata, "BLOQUE_INICIAL=%d\n", primer_bloque);
    fprintf(metadata, "TAMANIO_ARCHIVO=0");
    
    fclose(metadata);

    log_info(entradasalida_logger, "Archivo '%s' creado con bloque inicial %d y tamaño 0.", nombre, primer_bloque);

    free(path_archivo_a_crear);
}

// Borrar archivo
void liberar_bloque(t_bitarray* bitmap_bloques, int bloque) {
    bitarray_clean_bit(bitmap_bloques, bloque);
}

void borrar_archivo(char* nombre, t_bitarray* bitmap_bloques) {
    // Concatenar nombre con PATH_BASE_DIALFS
    uint32_t MAX_LENGTH = 256;
    char* path_archivo_a_borrar = malloc(MAX_LENGTH);
    strcpy(path_archivo_a_borrar, PATH_BASE_DIALFS);
    strcat(path_archivo_a_borrar, "/");
    strcat(path_archivo_a_borrar, nombre);

    t_config* metadata = config_create(path_archivo_a_borrar);

    // Leer los valores como si fuera config
    int bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
    int tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");

    // Cuantos bloques tiene el archivo
    int bloques_ocupados = ceil(tamanio_archivo / (float)BLOCK_SIZE);

    // Solamente tiene el bloque inicial pero su tamaño es 0
    if (bloques_ocupados == 0) {
        liberar_bloque(bitmap_bloques, bloque_inicial);
    }

    // Liberar los bloques en el bitmap
    for (int i = 0; i < bloques_ocupados; i++) {
        liberar_bloque(bitmap_bloques, bloque_inicial + i);
    }

    // Eliminar el archivo del FS
    if (remove(path_archivo_a_borrar) != 0) {
        log_error(entradasalida_logger, "No se pudo eliminar el archivo de metadata.");
    } else {
        log_info(entradasalida_logger, "Archivo '%s' borrado y bloques liberados.", nombre);
    }

    config_destroy(metadata);
    free(path_archivo_a_borrar);
}

// Para ver si hay suficientes bloques libres
int contar_bloques_libres(t_bitarray* bitmap_bloques) {
    int count = 0;
    for (int i = 0; i < bitarray_get_max_bit(bitmap_bloques); i++) {
        if (!bitarray_test_bit(bitmap_bloques, i)) {
            count++;
        }
    }
    return count;
}

// Ver si hay que compactar
int contar_bloques_libres_continuos(t_bitarray* bitmap_bloques, int bloque_inicial, int bloques_necesarios) {
    int bloques_libres = 0;
    int index = bloque_inicial + 1;
    
    while (index < bitarray_get_max_bit(bitmap_bloques)) {
        if (!bitarray_test_bit(bitmap_bloques, index)) {
            bloques_libres++;
            if (bloques_libres >= bloques_necesarios) {
                break; // Ya encontramos suficientes bloques libres
            }
        } else {
            break; // Encontramos un bloque ocupado
        }
        index++;
    }

    return bloques_libres;
}

void ocupar_bloque(t_bitarray* bitmap_bloques, int bloque) {
    bitarray_set_bit(bitmap_bloques, bloque);
}

// Truncate
void truncate_archivo(char* nombre, int tamanio_nuevo, t_bitarray* bitmap_bloques, t_pcb* pcb) {
    // Concatenar nombre con PATH_BASE_DIALFS
    uint32_t MAX_LENGTH = 256;
    char* path_archivo_a_truncar = malloc(MAX_LENGTH);
    strcpy(path_archivo_a_truncar, PATH_BASE_DIALFS);
    strcat(path_archivo_a_truncar, "/");
    strcat(path_archivo_a_truncar, nombre);

    t_config* metadata = config_create(path_archivo_a_truncar);

    // Leer los valores como si fuera config
    int bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
    int tamanio_actual = config_get_int_value(metadata, "TAMANIO_ARCHIVO");

    config_destroy(metadata);

    int bloques_actuales = ceil(tamanio_actual / (float)BLOCK_SIZE);
    if (bloques_actuales == 0) {
        bloques_actuales++; // Tiene tamaño 0 pero tiene un bloque inicial
    }
    // Cuanto bloques necesito
    int bloques_necesarios = ceil(tamanio_nuevo / (float)BLOCK_SIZE) - bloques_actuales; // Restamos 1 porque ya tiene un bloque inicial

    bool compactacion_realizada = false;
    if(tamanio_actual > tamanio_nuevo) {
        // Disminuir tamaño
        int bloques_a_eliminar = abs(bloques_necesarios);
        log_info(entradasalida_logger, "Bloques a eliminar %d", bloques_a_eliminar);
        int bloque_final = bloque_inicial + bloques_actuales - 1;
        for (int i = 0; i < bloques_a_eliminar; i++) {
            liberar_bloque(bitmap_blocks, bloque_final);
            log_info(entradasalida_logger, "Bloque %d liberado", bloque_final);
            bloque_final--;
        }
    } else if(tamanio_actual < tamanio_nuevo) {
        // Logica de cambiar tamaño
        int bloques_libres = contar_bloques_libres(bitmap_blocks);
        if (bloques_libres < bloques_necesarios) {
            log_error(entradasalida_logger, "No hay suficiente espacio para el archivo");
            return;
        }

        // Bloques libres continuos
        int bloques_libres_continuos = contar_bloques_libres_continuos(bitmap_blocks, bloque_inicial, bloques_necesarios);

        log_info(entradasalida_logger, "Bloques necesarios: %d", bloques_necesarios);
        log_info(entradasalida_logger, "Bloques libres continuos: %d", bloques_libres_continuos);
        if(bloques_libres_continuos < bloques_necesarios) {
            log_info(entradasalida_logger, "PID: %d - Inicio Compactacion.", pcb->pid);
            // Compactar
            iniciar_compactacion(bitmap_blocks, nombre, tamanio_nuevo);
            log_info(entradasalida_logger, "PID: %d - Fin Compactacion.", pcb->pid);
            compactacion_realizada = true;
            usleep(RETRASO_COMPACTACION * 1000);
        } else {
            // Aumentar tamaño
            int bloque_final = bloque_inicial + bloques_actuales - 1;
            for (int i = 0; i < bloques_necesarios; i++) {
                bloque_final++;
                ocupar_bloque(bitmap_blocks, bloque_final);
                log_info(entradasalida_logger, "Bloque %d ocupado", bloque_final);
            }
        }
    }

    // Escribir nuevo tamaño en el archivo si no se tuvo que hacer compactacion
    if (!compactacion_realizada) {
        FILE* archivo_a_truncar = fopen(path_archivo_a_truncar, "w");
        if (!archivo_a_truncar) {
            log_error(entradasalida_logger, "Error: No se pudo crear el archivo a truncar.");
            return;
        }

        // Escribir nuevo tamaño en el archivo
        fprintf(archivo_a_truncar, "BLOQUE_INICIAL=%d\n", bloque_inicial);
        fprintf(archivo_a_truncar, "TAMANIO_ARCHIVO=%d", tamanio_nuevo);

        fclose(archivo_a_truncar);
    }

    free(path_archivo_a_truncar);
}

// Write
void write_archivo(char* nombre, void* datos, int tamanio_write, int ptr_archivo_write, t_bitarray* bitmap_bloques) {
    // Concatenar nombre con PATH_BASE_DIALFS
    uint32_t MAX_LENGTH = 256;
    char* path_archivo_write = malloc(MAX_LENGTH);
    strcpy(path_archivo_write, PATH_BASE_DIALFS);
    strcat(path_archivo_write, "/");
    strcat(path_archivo_write, nombre);

    t_config* metadata = config_create(path_archivo_write);

    // Leer los valores como si fuera config
    int bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");

    config_destroy(metadata);

    int posicion_archivo_write = bloque_inicial * BLOCK_SIZE + ptr_archivo_write;

    // Leo de memoria, espero recibir los datos y lo escribo en el archivo
    // Abrir el archivo de bloques
    FILE* archivo_bloques = fopen(path_bloques, "rb+");
    if (archivo_bloques == NULL) {
        log_error(entradasalida_logger, "Error al abrir bloques.dat para write");
        return;
    }

    // Moverse al puntero archivo
    if (fseek(archivo_bloques, posicion_archivo_write, SEEK_SET) != 0) {
        log_error(entradasalida_logger, "Error al moverse al puntero del archivo");
        fclose(archivo_bloques);
        return;
    }

    // Escribir los datos
    fwrite(datos, 1, tamanio_write, archivo_bloques);

    fclose(archivo_bloques);
    free(path_archivo_write);
}

// Read
void read_archivo(char* nombre, int tamanio_read, int dir_fisica_read, int ptr_archivo_read, t_pcb* pcb_fs_read, t_bitarray* bitmap_bloques) {
    // Concatenar nombre con PATH_BASE_DIALFS
    uint32_t MAX_LENGTH = 256;
    char* path_archivo_read = malloc(MAX_LENGTH);
    strcpy(path_archivo_read, PATH_BASE_DIALFS);
    strcat(path_archivo_read, "/");
    strcat(path_archivo_read, nombre);

    t_config* metadata = config_create(path_archivo_read);

    // Leer los valores como si fuera config
    int bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");

    config_destroy(metadata);

    int posicion_archivo_read = bloque_inicial * BLOCK_SIZE + ptr_archivo_read;

    // Leo de archivo, envio los datos a escribir en memoria y recibo si la escritura fue ejecutada
    // Abrir el archivo de bloques
    FILE* archivo_bloques = fopen(path_bloques, "rb+");
    if (archivo_bloques == NULL) {
        log_error(entradasalida_logger, "Error al abrir bloques.dat para read");
        return;
    }

    // Moverse a la posicion deseada
    if (fseek(archivo_bloques, posicion_archivo_read, SEEK_SET) != 0) {
        log_error(entradasalida_logger, "Error al moverse al puntero del archivo");
        fclose(archivo_bloques);
        return;
    }

    // Escribir los datos
    void* datos_leidos = malloc(tamanio_read);
    fread(datos_leidos, 1, tamanio_read, archivo_bloques);

    send_escribir_memoria(fd_memoria, pcb_fs_read->pid, dir_fisica_read, datos_leidos, tamanio_read);

    free(datos_leidos);
    fclose(archivo_bloques);
    free(path_archivo_read);
}

// Compactacion
void limpiar_bitmap(t_bitarray* bitmap_bloques) {
    int index = 0;
    while (index < bitarray_get_max_bit(bitmap_bloques)) {
        if (bitarray_test_bit(bitmap_bloques, index)) {
            liberar_bloque(bitmap_bloques, index);
        }
        index++;
    }
}

void iniciar_compactacion(t_bitarray* bitmap_bloques, char* nombre_truncate, int tamanio_nuevo) {
    uint32_t MAX_LENGTH = 256;

    // Abrir el directorio que contiene los metadata
    // Para leer direcciones
    struct dirent* entry;
    DIR* dir = opendir(PATH_BASE_DIALFS);
    if (dir == NULL) {
        log_error(entradasalida_logger, "Error al abrir el directorio de metadatas para compactacion");
        return;
    }

    // Abrir el archivo de bloques
    FILE* archivo_bloques = fopen(path_bloques, "rb+");
    if (archivo_bloques == NULL) {
        log_error(entradasalida_logger, "Error al abrir bloques.dat para compactacion");
        return;
    }

    uint32_t file_size = BLOCK_SIZE * BLOCK_COUNT;
    void* buffer_viejo = malloc(file_size); // Tiene todo el archivo anterior a la compactacion y lo va utilizando para no pisar datos

    // Leer el archivo completo en el buffer viejo
    fread(buffer_viejo, 1, file_size, archivo_bloques);
    // Poner todo el bitmap en 0
    limpiar_bitmap(bitmap_blocks);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strcmp(entry->d_name, "bloques.dat") != 0 && strcmp(entry->d_name, "bitmap.dat") != 0) {
            char* path_archivo = malloc(MAX_LENGTH);
            snprintf(path_archivo, MAX_LENGTH, "%s/%s", PATH_BASE_DIALFS, entry->d_name);

            t_config* metadata = config_create(path_archivo);

            int bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
            int tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");

            config_destroy(metadata);

            int bloques_actuales;
            if (strcmp(entry->d_name, nombre_truncate) == 0) {
                bloques_actuales = ceil(tamanio_nuevo / (float)BLOCK_SIZE);
                if (bloques_actuales == 0) {
                    bloques_actuales++; // Solo tiene el bloque inicial pero su tamaño es 0
                }
            } else {
                bloques_actuales = ceil(tamanio_archivo / (float)BLOCK_SIZE);
                if (bloques_actuales == 0) {
                    bloques_actuales++; // Solo tiene el bloque inicial pero su tamaño es 0
                }
            }

            int bloque_nuevo = primer_bloque_libre(bitmap_bloques);
            if (bloque_nuevo == -1) {
                log_error(entradasalida_logger, "No hay suficientes bloques libres para la compactacion");
                fclose(archivo_bloques);
                closedir(dir);
                return;
            }

            for (int j = 0; j < bloques_actuales; j++) {
                int bloque_origen = bloque_inicial + j;
                int bloque_destino = bloque_nuevo + j;

                // Leer datos del bloque origen desde el buffer de los datos del archivo anterior a la compactacion
                void* datos_bloque = (char*)buffer_viejo + bloque_origen * BLOCK_SIZE;

                // Escribir datos en el bloque destino en el archivo de bloques (bloques.dat)
                fseek(archivo_bloques, bloque_destino * BLOCK_SIZE, SEEK_SET);
                fwrite(datos_bloque, 1, BLOCK_SIZE, archivo_bloques);

                // Marcar el bloque destino como ocupado
                ocupar_bloque(bitmap_bloques, bloque_destino);
            }

            // Actualizar el metadata
            FILE* archivo_metadata = fopen(path_archivo, "w");
            if (!archivo_metadata) {
                log_error(entradasalida_logger, "Error al abrir el archivo de metadatos para escritura %s", entry->d_name);
                fclose(archivo_bloques);
                closedir(dir);
                return;
            }

            if (strcmp(entry->d_name, nombre_truncate) == 0) {
                fprintf(archivo_metadata, "BLOQUE_INICIAL=%d\n", bloque_nuevo);
                fprintf(archivo_metadata, "TAMANIO_ARCHIVO=%d", tamanio_nuevo);
            } else {
                fprintf(archivo_metadata, "BLOQUE_INICIAL=%d\n", bloque_nuevo);
                fprintf(archivo_metadata, "TAMANIO_ARCHIVO=%d", tamanio_archivo);
            }

            fclose(archivo_metadata);
            free(path_archivo);
        }
    }

    fclose(archivo_bloques);
    closedir(dir);
    log_info(entradasalida_logger, "Compactacion completada con exito.");
    free(buffer_viejo);
}