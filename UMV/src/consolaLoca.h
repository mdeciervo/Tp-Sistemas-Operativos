/*
 * consolaLoca.h
 *
 *  Created on: 03/07/2014
 *      Author: utnso
 */

#ifndef CONSOLALOCA_H_
#define CONSOLALOCA_H_

#include "configUMV.h"
#include "memoria.h"
#include "sincro.h"

#define OPERACION 1
#define RETARDO 2
#define ALGORITMO 3
#define COMPACTACION 4
#define DUMP 5

//MENU PRINCIPAL DE LA CONSOLA
void consolaLoca(void);
//NUMERO HASTA UN \n
int detectarLong(char* string);
//NUMERO HASTA UN \0
int detectarEspacio(char* string);
//DETECTA COMANDOS
void manejarOperacion(char* operacion,char* restoComando);
//GRABAR BYTES DESDE CONSOLA
void grabarBytesConsola(char* restoComando);
//SOLICITAR BYTES DESDE CONSOLA
void solicitarBytesConsola(char* restoComando);
//CREAR SEGMENTOS DESDE CONSOLA
void crearSegmentoConsola(char* restoComando);
//DESTRUIR SEGMENTOS DESDE CONSOLA
void destruirSegmentosConsola(char* restoComando);
//MOSTRAR STRING SIN \0
void mostrarStringSinNulo(char* cadena, int longitud);
//GRABAR EN ARCHIVO STRING SIN \0
void escribirStringSinNulo(char* cadena, int longitud, FILE* archivo);

void escribirStringLoco(char* cadena, int longitud, FILE* archivo);

void mostrarStringLoco(char* cadena, int longitud);

void escribirEnDiscoLoco(char* bufferAux, int tamanio,int tipo);

void dumpLoco(int opcion, int pid,int offset,int cantidad);

void contenidoMP(char* restoComando);

#endif /* CONSOLALOCA_H_ */
