/*
 * stack.c
 *
 *  Created on: 19/04/2014
 *      Author: utnso
 */

/*
 * TAD de stack para apilar
 * y desapilar elementos
 */

#include "list.h"
#include "stack.h"
#include <stdlib.h>

/*
 * Crea una estructura de stack y retorna el puntero
 * con la lista de elementos en 0 y el tope en NULL.
 */
t_stack* crearStack(){
	t_stack* stack = malloc(sizeof(t_stack));
	stack->listaElementos = list_create();
	stack->elements_count = 0;
	stack->tamanio = 0;
	return stack;
}

/*
 * Elimina la pila y todos los elementos que
 * posee en su interior.
 */
void destruirStack(t_stack* pila){
	pila->elements_count = 0;
	list_destroy(pila->listaElementos);
}

/*
 * Coloco el elemento al final de la pila.
 * Aumenta la cantidad de elementos.
 */
int apilar(t_stack * pila, void *element){
	pila->elements_count++;
	int resul = list_add(pila->listaElementos,element);
	pila->tamanio = pila->tamanio + sizeof(element);
	return resul;
}

/*
 * Quito el elemento superior de la pila.
 * Reduce la cantidad de elementos disponibles.
 */
void* desapilar(t_stack* pila){
	pila->elements_count = pila->elements_count - 1;
	return list_remove(pila->listaElementos,pila->elements_count);
}

//-------------- NUEVO TIPO PILA ---------------------

// Devuelve un puntero a una estructura de stack creada vacia
t_stack_c* crearStackC(int tamanio){
	t_stack_c* pila= malloc(sizeof(t_stack_c));
	pila->segmento = malloc(tamanio);
	pila->punteroUlt = 0;
	pila->cantElem = 0;
	pila->tamanio = tamanio;
	return pila;
}

// Elimina la estructura de stack existente
void destruirStackC(t_stack_c* pila){
	free(pila->segmento);
	free(pila);
}

// Apila variables al tope del stack un elemento. Retorna la posicion del valor apilado
int apilarC(t_stack_c * pila, void* element){
	int posicion = 0;
	if ((pila->punteroUlt + 5) <= pila->tamanio){
		memcpy(pila->segmento+pila->punteroUlt,element,5);
		posicion = pila->punteroUlt + 1;
		pila->punteroUlt = pila->punteroUlt + 5;
		pila->cantElem++;
	} else {
		posicion = -1;
	}
	return posicion;
}

// Apila referencias al tope del stack un elemento. Retorna la posicion del valor apilado
int apilarCRef(t_stack_c * pila, void *element){
	int posicion = 0;
	if ((pila->punteroUlt + 4) <= pila->tamanio){
		memcpy(pila->segmento+pila->punteroUlt,element,4);
		posicion = pila->punteroUlt;
		pila->punteroUlt = pila->punteroUlt + 4;
		pila->cantElem++;
	} else {
		posicion = -1;
	}
	return posicion;
}

// Desapila la variable superior del stack
char* desapilarC(t_stack_c* pila){
	char* resul = malloc(sizeof(char)*5);
	if ((pila->punteroUlt - 5) > -1 ){
		memcpy(resul,pila->segmento+pila->punteroUlt-5,5);
		pila->punteroUlt = pila->punteroUlt-5;
		pila->cantElem--;
	} else {
		char idFalla = '#';
		memcpy(resul,&idFalla,1);
	}
	return resul;
}

// Desapilo la referencia superior del stack
char* desapilarCRef(t_stack_c* pila){
	char* resul = malloc(sizeof(char)*4);
	if ((pila->punteroUlt - 4) > -1 ){
		memcpy(resul,pila->segmento+pila->punteroUlt-4,4);
		pila->punteroUlt = pila->punteroUlt-4;
		pila->cantElem--;
	} else {
		int retorno = -1;
		memcpy(resul,&retorno,4);
	}
	return resul;
}
