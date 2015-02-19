#ifndef LIBMEMORIA_H_
#define LIBMEMORIA_H_

#include "configUMV.h"
#include "consola.h"
#include "consolaLoca.h"
#include "sincro.h"

//	Funcion que crea la memoria. Devuelve un puntero a la memoria creada
char* crear_memoria(int tamanio);

//  Crea un segmento, recibiendo un pid y un tamanio. Devuelve -1 si no hay espacio, o -2 si el pid es 0
int crearSegmento(int pid, int tamanio);

//	Funcion auxiliar al algoritmo worst-fit
int wostFitFiltroYOrden(int tamanio);

//	Algoritmo worst-fit
int worstfit(int pid, int tamanio);

//  Algoritmo first-fit
int firstfit(int pid, int tamanio);

//  FILTRA POR PID, Y DEVUELVE EL MAYOR NUMERO DE SEGMENTO +1
int buscarNumeroSegmento(int pid);

//  BUSCA UNA BASE VALIDA PARA CREAR EL SEGMENTO. SI EL PID ES 0 RETORNA -2
int buscarBaseValida(int pid, int tamanio);

//  Devuelve un numero random
int nuneroRandom(int min, int max);

//  Muestra un listado de la memoria, con los segmentos libres y ocupados
void mostrarSegmentos(t_list* lista);

//  Loguea un listado de la memoria, con los segmentos libres y ocupados
void loguearSegmentos(t_list* lista);

//  Dado un segmento i+1 y un segmento i, agranda el segmento i+1 hasta que ocupe
//  el tamanio de los 2, y borra el segmento i
void mergeTop(t_segmento* segAuxTop, t_segmento* segAux, int i);

//  Dado un segmento i-1 y un segmento i, agranda el segmento i-1 hasta que ocupe
//  el tamanio de los 2, y borra el segmento i
void mergeBot(t_segmento* segAuxBop, t_segmento* segAux, int i);

//  Dados los segmentos i-1,i+1 e i, Agranda el i-1 para que ocupe el tamanio de los 3,
//  y borra los 2 restantes
void dobleMerge(t_segmento* segAuxBot, t_segmento* segAuxTop,
		t_segmento* segAux, int i);

//  Borra todos los segmentos de un programa en particular.
void destruirSegmentosDePrograma(int pid);

//  Retorna un puntero a la direccion de los bytes pedidos.
char* solicitoBytesDesde(int base, int offset, int tamanio, int pid);

//  Dado una base y un numero de pid, busca si existe el segmento en la memoria
t_segmento* buscarSegmento(int base, int pid);

//	Dada una base y un pid, devuelve la posicion del segmento
int buscarIndexSegmento(int base, int pid);

//  guarda bytes en memoria. Retorna si se pudo hacer o no
int enviarBytesParaGuardar(int base, int offset, int tamanio, void* buffer,
		int pid);
//	solicitar bytes desde consola
void* solicitoMemoria(int pid,int base, int inicio, int tamanio, char* template);
//	grabar bytes desde consola
int grabarElBuffer(int pid,int base,int inicio, char* template, char* buffer);

//	Borra todos los segmentos
void limpiarMemoria(void);

//  Dado un segmento, retorna un puntero a otro segmento con los mismos campos
t_segmento* copiarSegmento(t_segmento* segmentoACopiar);

//  Ejecuta la compactacion
int compactacion(void);

//  Muestra tablas de segmentos de un proceso en particular
void mostrarTablaDe(int pid);

// 	Muesta las tablas de segmentos de un proceso en particular, pero si es 0 muestra de todos.
void estructurasDeMemoria(int pid);

//	DEVUELVE UN NUEVO SEGMENTO CON LOS CAMPOS ENVIADOS
t_segmento* dameNuevoSegmento(int pid, int offset, int base, bool ocupado,
		int size, int numero);

//	FILTER POR PID
t_list* listaDeSegmentosDe(int pid);

//	DEVUELVE EL CONTENIDO DE LA MP DESDE UN OFFSET DADO, Y LA CANTIDAD DE BYTES
void contenidoDeLaMp(int offset, int bytes);

#endif /* LIBMEMORIA_H_ */
