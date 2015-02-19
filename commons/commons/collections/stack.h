/*
 * stack.h
 *
 *  Created on: 20/04/2014
 *      Author: utnso
 */

#ifndef STACK_H_
#define STACK_H_

#include "list.h"

typedef struct PILA {
		t_list* listaElementos;
		int elements_count;
		int tamanio;
		int puntero;
	} t_stack;


/*
 * Estructura de pila que no maneja estructuras,
 * sino que apila y desapila copiando datos en un
 * array de char.
 */
typedef struct PILACHAR {
	int cantElem;
	char* segmento;
	int punteroUlt;
	int cursor;
	int tamanio;
} t_stack_c;


// Devuelve un puntero a una estructura de stack creada vacia
t_stack* crearStack();

// Elimina la estructura de stack existente
void destruirStack(t_stack* pila);

// Apila al tope del stack un elemento. Retorna 1 si es correcto
int apilar(t_stack * pila, void *element);

// Desapila el elemento superior del stack
void* desapilar(t_stack* pila);

//-------------- NUEVO TIPO PILA ---------------------

// Devuelve un puntero a una estructura de stack creada vacia
t_stack_c* crearStackC(int tamanio);

// Elimina la estructura de stack existente
void destruirStackC(t_stack_c* pila);

// Apila variables al tope del stack un elemento. Retorna la posicion del valor apilado. -1 es Out of Bounds
int apilarC(t_stack_c * pila, void *element);

// Apila referencias al tope del stack un elemento. Retorna la posicion del valor apilado. -1 es Out of Bounds
int apilarCRef(t_stack_c * pila, void *element);

// Desapila el elemento superior del stack. '#' en id es que pide por fuera, Out of Bounds
char* desapilarC(t_stack_c* pila);

// Desapilo una referencia del stack. '#' en id es que pide por fuera, Out of Bounds
char* desapilarCRef(t_stack_c* pila);

#endif /* STACK_H_ */
