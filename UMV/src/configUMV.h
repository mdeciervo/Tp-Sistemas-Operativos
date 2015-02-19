/*
 * configUMV.h
 *
 *  Created on: 22/04/2014
 *      Author: utnso
 */

#ifndef CONFIGUMV_H_
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/tad_server.h>
#include <string.h>
#include <unistd.h>
#include <commons/log_app.h>
#include <commons/shared_strs.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <commons/temporal.h>
#include <commons/string.h>
#include <time.h>
#include <pthread.h>

#define errorPid0 -2
#define errorMemOver -1
#define errorNoExisteSegmento -3
#define Ok 1
#define errorTamanioLibreMenor -4
#define errorTam0 -5
#define errorLeoMas "errorLeoMas"
#define errorSegInvalido "errorSegInvalido"
#define errorComando 8;
#define STRING 9;
#define INT 10;
#define CHAR 11;


#define CONFIGUMV_H_
typedef struct configUmv {
	int tamanioMemoria;
	int retardo;
	char* algoritmo;
	char* ip;
	int puerto;
}__attribute__((__packed__)) configUmv;

typedef struct mensajeHandshake {
	int ID;
	int socket;
	int numero;
}__attribute__((__packed__)) mensajeHandshake;

typedef struct {
	int pid;
	int base;
	int size;
	int offset;
	bool ocupado;
	int numeroSegmento;
} t_segmento;

extern configUmv* configuracionUmv;
extern char log_text[2000];
extern char* memoria;
extern t_list* listaDeSegmentos;
extern configUmv* configuracionUmv;
extern pthread_mutex_t mutex;
extern pthread_mutex_t wrt;
extern int readcount;
extern FILE* archivoDump;
extern int banderaConsola;

configUmv* levantarConfiguracion(char *rutaArchivo);
void mostrarConfiguracion(void);


#endif /* CONFIGUMV_H_ */
