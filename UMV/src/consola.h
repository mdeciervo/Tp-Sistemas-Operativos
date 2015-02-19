/*
 * consola.h
 *
 *  Created on: 28/04/2014
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_
#include "configUMV.h"
#include "memoria.h"
#include "sincro.h"

#define OPERACION 1
#define RETARDO 2
#define ALGORITMO 3
#define COMPACTACION 4
#define DUMP 5

//MENU PRINCIPAL DE LA CONSOLA
void estaSeriaLaConsola(void);
//MENU DE OPERACIONES
void operacion(void);
//DADO UN BUFFER Y SU LONGITUD, LO ESCRIBE EN DISCO
void escribirEnDisco(char* buffer, int longitud);
//SE PIDE PID, BASE, OFFSET, TAMANIO Y MUESTRA EL CONTENIDO EN MEMORIA
void mostrarPosicionDeMemoria(void);
//SE PIDE PID, BASE, OFFSET, TAMANIO, BUFFER Y SE ESCRIBE EN MEMORIA
void escribirEnBuffer(void);
//SE PIDE EL PID, TAMANIO Y SE CREA SEGMENTO
void opcionCrearSegmento(void);
//SE PIDE PID Y SE DESTRUYEN SUS SEGMENTOS
void opcionDestruirSegmento(void);
//ESCRIBE EN UN ARCHIVO UNA CADENA DESDE SU INICIO HASTA LA LONGITUD, IGNORANDO '\0'
void escribirString(char* cadena, int longitud, FILE* archivo);
//MUESTRA UNA CADENA DESDE SU INICIO HASTA LA LONGITUD, IGNORANDO '\0'
void mostrarString(char* cadena, int longitud);
//SE ACTUALIZA EL VALOR DE RETARDO POR EL INGRESADO
void retardo(void);
//SE ACTUALIZA EL ALGORITMO POR EL PEDIDO
void algoritmo(void);
//MENU DE DUMP
void dump(void);
//MOSTRAR MENSAJE CADENA Y DEVOLVER UN ENTERO MAYOR A CERO
int verificarMayorACero(char* cadena);
//MOSTRAR MENSAJE CADENA Y DEVOLVER UN ENTERO MAYOR O IGUAL A CERO
int verificarMayorOIgualACero(char* cadena);
//PIDE UN TAMANIO, VERIFICA QUE SEA !=0 Y MENOR AL TAMANIO DE MEMORIA, Y LO DEVUELVE
int verificarTamanio();
//VERIFICA QUE EL BUFFER INGRESADO SEA MAYOR AL TAMANIO
char* verificarBuffer(int tamanio);
//DEVUELVE UN PID VERIFICADO
int verificarPID();
//DEVUELVE UNA BASE VERIFICADA
int verificarBase();
//DEVUELVE UN OFFSET VERIFICADO
int verificarOffset();

#endif /* CONSOLA_H_ */
