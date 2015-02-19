/*
 * ansisop_prim.h
 *
 *  Created on: 22/04/2014
 *      Author: utnso
 */

#ifndef ANSISOP_PRIM_H_
#define ANSISOP_PRIM_H_

#include <stdio.h>
#include <parser/parser.h>


t_puntero definirVariable(t_nombre_variable identificador_variable);

t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable);

t_valor_variable dereferenciar(t_puntero direccion_variable);

void asignar(t_puntero direccion_variable, t_valor_variable valor);

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);

void irAlLabel(t_nombre_etiqueta t_nombre_etiqueta);

void llamarSinRetorno(t_nombre_etiqueta etiqueta);

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);

void finalizar(void);

void retornar(t_valor_variable retorno);

void imprimir(t_valor_variable valor_mostrar);

void imprimirTexto(char* texto);

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);

void semaforo_wait(t_nombre_semaforo identificador_semaforo);

void semaforo_signal(t_nombre_semaforo identificador_semaforo);



#endif /* ANSISOP_PRIM_H_ */
