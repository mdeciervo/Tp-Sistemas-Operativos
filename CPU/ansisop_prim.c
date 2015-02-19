/*
 * ansisop_prim.c
 *
 *  Created on: 22/04/2014
 *      Author: utnso
 */
#include <stdio.h>
#include "ansisop_prim.h"
#include "cpu.h"
#include <commons/log_app.h>


/*
 * IMPRIMIR TEXTO
 *
 * Envía mensaje al Kernel, para que termine siendo mostrado en la consola del Programa en ejecución. mensaje no posee parámetros, secuencias de escape, variables ni nada.
 *
 * @sintax TEXT_PRINT_TEXT (textPrint )
 * @param	texto	Texto a imprimir
 * @return void
 */
void imprimirTexto(char* texto){
	deshabilitar_print_console();
	if (corteProg != 1){
		sprintf(log_cpu,"Entro en primitiva imprimir texto");
		log_debug_message(log_cpu);
		enviarMensajeAKernel(texto,0,IMPRIMIR_TEXTO);
	} else {
		sprintf(log_cpu,"La primitiva imprimirTexto no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
	}
	habilitar_print_console();
}


/*
 * DEFINIR VARIABLE
 *
 * Reserva en el Contexto de Ejecución Actual el espacio necesario para una variable llamada identificador_variable y
 * la registra tanto en el Stack como en el Diccionario de Variables. Retornando la posición del valor de esta nueva variable del stack
 * El valor de la variable queda indefinido: no deberá inicializarlo con ningún valor default.
 * Esta función se invoca una vez por variable, a pesar que este varias veces en una línea.
 * Ej: Evaluar "variables a, b, c" llamará tres veces a esta función con los parámetros "a", "b" y "c"
 *
 * @sintax	TEXT_VARIABLE (variables identificador[,identificador]*)
 * @param	identificador_variable	Nombre de variable a definir
 * @return	Puntero a la variable recien asignada
 */
t_puntero definirVariable(t_nombre_variable identificador_variable){
	int posicion = -1;
	if (corteProg != 1){
		deshabilitar_print_console();
		sprintf(log_cpu,"Entro en primitiva definir variable");
		log_debug_message(log_cpu);
		habilitar_print_console();
		t_variable* variable = malloc(sizeof(t_variable));
		variable->id = identificador_variable;
		posicion = apilarC(stack,variable);
		if (posicion != -1){
			indexarEnDiccionario(diccionario,identificador_variable,posicion);
			pcb.tamContext++; // Aumento el tamaño del contexto actual
			sprintf(log_cpu,"Se ejecuto definir variable. ID:%c - Posicion:%i",identificador_variable,posicion);
			log_info_message(log_cpu);
		} else {
			controloPosStackUMV(pcb.stackSeg,stack->punteroUlt,5,2);
		}
		free(variable);
		return posicion;
	} else {
		deshabilitar_print_console();
		sprintf(log_cpu,"La primitiva definirVariable no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
		habilitar_print_console();
	}
	return posicion;
}


/*
 * OBTENER POSICION de una VARIABLE
 *
 * Devuelve el desplazamiento respecto al inicio del segmento Stack en que se encuentra el valor de la
 * variable identificador_variable del contexto actual.
 * En caso de error, retorna -1.
 *
 * @sintax	TEXT_REFERENCE_OP (&)
 * @param	identificador_variable 		Nombre de la variable a buscar (De ser un parametro, se invocara sin el '$')
 * @return	Donde se encuentre la variable buscada
 */
t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable){
	int posicion = -1;
	if (corteProg != 1){
		deshabilitar_print_console();
		sprintf(log_cpu,"Entro en obtener posicion variable");
		log_debug_message(log_cpu);
		habilitar_print_console();
		int posicion = obtenerPosicionDic(diccionario,identificador_variable);
		if (posicion == -1){
			corteProg = 1;
			sprintf(log_cpu,"No se obtiene posicion de la variable %c ",identificador_variable);
			log_error_message(log_cpu);
		} else {
			sprintf(log_cpu,"Obtengo posicion de la variable %c, que es %i ",identificador_variable,posicion);
			log_info_message(log_cpu);
			return posicion;
		}
	} else {
		deshabilitar_print_console();
		sprintf(log_cpu,"La primitiva obtenerPosicionVariable no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
		habilitar_print_console();
	}
	return posicion;
}


/*
 * DEREFERENCIAR
 *
 * Obtiene el valor resultante de leer a partir de direccion_variable, sin importar cual fuera el contexto actual
 *
 * @sintax	TEXT_DEREFERENCE_OP (*)
 * @param	direccion_variable	Lugar donde buscar
 * @return	Valor que se encuentra en esa posicion
 */
t_valor_variable dereferenciar(t_puntero direccion_variable){
	t_valor_variable valor;
	if (corteProg != 1){
		deshabilitar_print_console();
		log_debug_message("Entro en primitiva dereferenciar");
		habilitar_print_console();
		if (direccion_variable > stack->tamanio){
			controloPosStackUMV(pcb.stackSeg,direccion_variable,5,1);
			valor = -1;
		} else {
			memcpy(&valor,stack->segmento+direccion_variable,sizeof(t_valor_variable));
			sprintf(log_cpu,"Dereferenciado el valor de la variable de la posicion %i es %i",direccion_variable,valor);
			log_info_message(log_cpu);
		}
	} else {
		deshabilitar_print_console();
		sprintf(log_cpu,"La primitiva dereferenciar no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
		habilitar_print_console();
	}
	return valor;
}


/*
 * ASIGNAR
 *
 * Inserta una copia del valor en la variable ubicada en direccion_variable.
 *
 * @sintax	TEXT_ASSIGNATION (=)
 * @param	direccion_variable	lugar donde insertar el valor
 * @param	valor	Valor a insertar
 * @return	void
 */
void asignar(t_puntero direccion_variable, t_valor_variable valor ){
	if (corteProg != 1){
		deshabilitar_print_console();
		log_debug_message("Entro en primitiva asignar");
		habilitar_print_console();
		if (direccion_variable > stack->tamanio){
			controloPosStackUMV(pcb.stackSeg,direccion_variable,5,2);
		} else {
			memcpy(stack->segmento+direccion_variable,&valor,sizeof(t_valor_variable));
			sprintf(log_cpu,"Asigno el valor %i a la variable de la posicion %i",valor,direccion_variable);
			log_info_message(log_cpu);
		}
	} else {
		deshabilitar_print_console();
		sprintf(log_cpu,"La primitiva asignar no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
		habilitar_print_console();
	}
}


/*
 * IMPRIMIR
 *
 * Envía valor_mostrar al Kernel, para que termine siendo mostrado en la consola del Programa en ejecución.
 *
 * @sintax	TEXT_PRINT (print )
 * @param	valor_mostrar	Dato que se quiere imprimir
 * @return	void
 */
void imprimir(t_valor_variable valor_mostrar){
	if (corteProg != 1){
		deshabilitar_print_console();
		log_debug_message("Entro en primitiva imprimir");
		habilitar_print_console();
		enviarMensajeAKernel(" ",valor_mostrar,IMPRIMIR);
		sprintf(log_cpu,"Envio valor a imprimir al Kernel: %i", valor_mostrar);
		log_info_message(log_cpu);
	} else {
		deshabilitar_print_console();
		sprintf(log_cpu,"La primitiva imprimir no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
		habilitar_print_console();
	}
}


/*
 * IR a la ETIQUETA
 *
 * Cambia la linea de ejecucion a la correspondiente de la etiqueta buscada.
 *
 * @sintax	TEXT_GOTO (goto )
 * @param	t_nombre_etiqueta	Nombre de la etiqueta
 * @return	void
 */
void irAlLabel(t_nombre_etiqueta etiqueta){
	if (corteProg != 1){
		deshabilitar_print_console();
		sprintf(log_cpu,"Ingreso a primitiva irAlLabel, busco etiqueta %s",etiqueta);
		log_debug_message(log_cpu);
		int nroInstr = -5;
		nroInstr = buscarEtiqueta(etiqueta,serEtiquetas,pcb.tamEtiq);
		sprintf(log_cpu,"Resultado de irAlLabel da %i\n",nroInstr);
		log_debug_message(log_cpu);
		habilitar_print_console();
		if (nroInstr >-1){
			saltoLinea = 1;
			pcb.PC = nroInstr;
			sprintf(log_cpu,"El label %s salta a la instruccion %i",etiqueta,nroInstr);
			log_info_message(log_cpu);
		} else {
			log_error_message("ERROR: no se obtiene la proxima instruccion a ejecutar");
			// DESALOJO EL PROGRAMA
			desalojar();
			corteProg = 1;
		}
	} else {
		deshabilitar_print_console();
		sprintf(log_cpu,"La primitiva irAlLabel no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
		habilitar_print_console();
	}
}


/*
 * LLAMAR SIN RETORNO
 *
 * Preserva el contexto de ejecución actual para poder retornar luego al mismo.
 * Modifica las estructuras correspondientes para mostrar un nuevo contexto vacío.
 *
 * Los parámetros serán definidos luego de esta instrucción de la misma manera que una variable local, con identificadores
 * numéricos empezando por el 0.
 *
 * @sintax	Sin sintaxis particular, se invoca cuando no coresponde a ninguna de las otras reglas sintacticas
 * @param	etiqueta	Nombre de la funcion
 * @return	void
 */
void llamarSinRetorno(t_nombre_etiqueta etiqueta){
	if (corteProg != 1){
		deshabilitar_print_console();
		log_debug_message("Ingreso a primitiva llamar sin retorno");
		habilitar_print_console();
		uint32_t cursor = stack->cursor;
		int control = apilarCRef(stack,&cursor); // Apilo el cursor del contexto de ejecucion
		if (control != -1){
			uint32_t proxPC = pcb.PC + 1;
			control = apilarCRef(stack,&proxPC); // Apilo el PC para la proxima instruccion
			if (control != -1){
				stack->cursor = stack->punteroUlt; // Coloco el cursor al final del apilado
				deshabilitar_print_console();
				sprintf(log_cpu,"Cursor Guardado: %i - PC Guardado: %i - Cursor Nuevo: %i",cursor,proxPC,stack->cursor);
				log_debug_message(log_cpu);
				habilitar_print_console();
				pcb.cursorStack = stack->cursor;
				pcb.tamContext = 0;
				inicializarDiccionario(diccionario); //Blanqueo el diccionario de variables
				irAlLabel(etiqueta);
				log_info_message("Llamo a funcion sin retorno");
			} else {
				controloPosStackUMV(pcb.stackSeg,stack->punteroUlt,5,2);
			}
		} else {
			controloPosStackUMV(pcb.stackSeg,stack->punteroUlt,5,2);
		}
	} else {
		deshabilitar_print_console();
		sprintf(log_cpu,"La primitiva llamarSinRetorno no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
		habilitar_print_console();
	}
}



/*
 * LLAMAR CON RETORNO
 *
 * Preserva el contexto de ejecución actual para poder retornar luego al mismo, junto con la posicion de la variable entregada por donde_retornar.
 * Modifica las estructuras correspondientes para mostrar un nuevo contexto vacío.
 *
 * Los parámetros serán definidos luego de esta instrucción de la misma manera que una variable local, con identificadores
 * numéricos empezando por el 0.
 *
 * @sintax	TEXT_CALL (<-)
 * @param	etiqueta	Nombre de la funcion
 * @param	donde_retornar	Posicion donde insertar el valor de retorno
 * @return	void
 */
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	if (corteProg != 1){
		deshabilitar_print_console();
		log_debug_message("Ingreso a primitiva llamar con retorno");
		habilitar_print_console();
		uint32_t cursor = stack->cursor;
		int control = apilarCRef(stack,&cursor); // Apilo el cursor del contexto de ejecucion
		if (control != -1){
			uint32_t proxPC = pcb.PC + 1;
			control = apilarCRef(stack,&proxPC); // Apilo el PC para la proxima instruccion
			if (control != -1){
				control = apilarCRef(stack,&donde_retornar); // Apilo la direccion de retorno de la variable
				if (control != -1){
						stack->cursor = stack->punteroUlt; // Coloco el cursor al final del apilado
						deshabilitar_print_console();
						sprintf(log_cpu,"Cursor Guardado: %i - PC Guardado: %i - Cursor Nuevo: %i",cursor,proxPC,stack->cursor);
						log_debug_message(log_cpu);
						habilitar_print_console();
						pcb.cursorStack = stack->cursor;
						pcb.tamContext = 0;
						inicializarDiccionario(diccionario); //Blanqueo el diccionario de variables
						irAlLabel(etiqueta);
						log_info_message("Llamo a funcion con retorno");
				} else {
					controloPosStackUMV(pcb.stackSeg,stack->punteroUlt,5,2);
				}
			} else {
				controloPosStackUMV(pcb.stackSeg,stack->punteroUlt,5,2);
			}
		} else {
			controloPosStackUMV(pcb.stackSeg,stack->punteroUlt,5,2);
		}
	} else {
		deshabilitar_print_console();
		sprintf(log_cpu,"La primitiva llamarConRetorno no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
		habilitar_print_console();
	}
}


/*
 * FINALIZAR
 *
 * Cambia el Contexto de Ejecución Actual para volver al Contexto anterior al que se está ejecutando, recuperando el Cursor de Contexto Actual
 * y el Program Counter previamente apilados en el Stack.
 * En caso de estar finalizando el Contexto principal (el ubicado al inicio del Stack), deberá finalizar la ejecución del programa devolviendo.
 *
 * @sintax	TEXT_END (end )
 * @param	void
 * @return	void
 */
void finalizar(void){
	if (corteProg != 1){
		deshabilitar_print_console();
		log_debug_message("Ingreso a primitiva finalizar");
		habilitar_print_console();
		int control = 1;
		if (stack->cursor > 0){
			deshabilitar_print_console();
			sprintf(log_cpu,"El cursor esta en %i y el ultimo del stack es %i\n",stack->cursor,stack->punteroUlt);
			log_debug_message(log_cpu);
			habilitar_print_console();
			if ( stack->punteroUlt > stack->cursor ){
				int j=0;
				for (j=0;j<pcb.tamContext;j++){
					t_variable* variableRet =  desapilarC(stack);
					// Saco las variables que esten por sobre el cursor para luego desapilar
				}
			}
			int* pcAct = (int*) desapilarCRef(stack); // Desapilo el PC de la proxima instruccion a ejecutar
			memcpy(&control,pcAct,4);
			if (control != -1){
				int* cursorAnt = (int*) desapilarCRef(stack); // Desapilo el contexto de ejecucion anterior
				memcpy(&control,cursorAnt,4);
				if (control != -1){
					int cantAlmacenado = stack->cursor - *cursorAnt - 2 * sizeof(uint32_t);
					int contextoAnt = cantAlmacenado/5;
					deshabilitar_print_console();
					sprintf(log_cpu,"pc anterior: %i - cursor anterior: %i - cant almacenado: %i - contexto Anterior: %i",*pcAct,*cursorAnt,cantAlmacenado,contextoAnt);
					log_debug_message(log_cpu);
					habilitar_print_console();
					pcb.cursorStack = *cursorAnt; // Asigno el cursor del contexto al anterior
					pcb.PC = *pcAct; // Asigno el PC con la instruccion apilada anteriormente
					pcb.tamContext = contextoAnt; // Reestablezco el tamaño del contexto anterior
					inicializarDiccionario(diccionario); //Blanqueo el diccionario de variables
					cargarDiccionario(diccionario,stack,contextoAnt); // Vuelvo a obtener los indices del diccionario
					sprintf(log_cpu,"Orden de Finalizar. El cursor vuelve a la posicion %i",stack->cursor);
					log_info_message(log_cpu);
					saltoLinea = 1;
				} else {
					controloPosStackUMV(pcb.stackSeg,stack->punteroUlt-4,4,1);
				}
			} else {
				controloPosStackUMV(pcb.stackSeg,stack->punteroUlt-4,4,1);
			}
		} else {
			deshabilitar_print_console();
			sprintf(log_cpu,"Se invoca la finalizacion desde el contexto principal. Se finaliza ejecucion de programa");
			log_debug_message(log_cpu);
			habilitar_print_console();
			programaFinalizado(); // Se invoca la finalizacion del programa si quiere finalizar el contexto principal
		}
	} else {
		deshabilitar_print_console();
		sprintf(log_cpu,"La primitiva finalizar no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
		habilitar_print_console();
	}
}


/*
 * RETORNAR
 *
 * Cambia el Contexto de Ejecución Actual para volver al Contexto anterior al que se está ejecutando, recuperando el Cursor de Contexto Actual,
 * el Program Counter y la direccion donde retornar, asignando el valor de retorno en esta, previamente apilados en el Stack.
 *
 * @sintax	TEXT_RETURN (return )
 * @param	retorno	Valor a ingresar en la posicion corespondiente
 * @return	void
 */
void retornar(t_valor_variable retorno){
	if (corteProg != 1){
		deshabilitar_print_console();
		log_debug_message("Ingreso a primitiva retornar");
		habilitar_print_console();
		int control = 1;
		if (stack->cursor > 0){
			deshabilitar_print_console();
			sprintf(log_cpu,"El cursor esta en %i y el ultimo del stack es %i\n",stack->cursor,stack->punteroUlt);
			log_debug_message(log_cpu);
			habilitar_print_console();
			if ( stack->punteroUlt > stack->cursor ){
				int j=0;
				for (j=0;j<pcb.tamContext;j++){
					t_variable* variableRet =  desapilarC(stack);
					// Saco las variables que esten por sobre el cursor para luego desapilar
					log_debug_message("Desapile una variable"); // DESPUES BORRAR!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				}
			}
			int* dirRet = (int*) desapilarCRef(stack); // Desapilo la direccion de retorno
			memcpy(&control,dirRet,4);
			if (control != -1){
				int* pcAct = (int*) desapilarCRef(stack); // Desapilo el PC de la proxima instruccion a ejecutar
				memcpy(&control,pcAct,4);
				if (control != -1){
					int* cursorAnt = (int*) desapilarCRef(stack); // Desapilo el contexto de ejecucion anterior
					memcpy(&control,cursorAnt,4);
					if (control != -1){
						int cantAlmacenado = stack->cursor - *cursorAnt - 3 * sizeof(uint32_t);
						int contextoAnt = cantAlmacenado/5;
						deshabilitar_print_console();
						sprintf(log_cpu,"pc anterior: %i - cursor anterior: %i - cant almacenado: %i - contexto Anterior: %i",*pcAct,*cursorAnt,cantAlmacenado,contextoAnt);
						log_debug_message(log_cpu);
						habilitar_print_console();
						pcb.cursorStack = *cursorAnt; // Asigno el cursor del contexto al anterior
						pcb.PC = *pcAct; // Asigno el PC con la instruccion apilada anteriormente
						pcb.tamContext = contextoAnt; // Reestablezco el tamaño del contexto anterior
						asignar(*dirRet,retorno); // Usando la primitiva asigno el valor de retorno
						inicializarDiccionario(diccionario); //Blanqueo el diccionario de variables
						cargarDiccionario(diccionario,stack,contextoAnt); // Vuelvo a obtener los indices del diccionario
						sprintf(log_cpu,"Orden de retornar. El cursor vuelve a la posicion %i",stack->cursor);
						log_info_message(log_cpu);
						saltoLinea = 1;
					} else {
						controloPosStackUMV(pcb.stackSeg,stack->punteroUlt-4,4,1);
					}
				} else {
					controloPosStackUMV(pcb.stackSeg,stack->punteroUlt-4,4,1);
				}
			} else {
				controloPosStackUMV(pcb.stackSeg,stack->punteroUlt-4,4,1);
			}
		} else {
			deshabilitar_print_console();
			sprintf(log_cpu,"Se invoca el retorno desde el contexto principal. Se finaliza ejecucion \n");
			log_debug_message(log_cpu);
			habilitar_print_console();
			programaFinalizado(); // Se invoca la finalizacion del programa si quiere finalizar el contexto principal
		}
	} else {
		deshabilitar_print_console();
		sprintf(log_cpu,"La primitiva retornar no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
		deshabilitar_print_console();
	}
}



/*
 * OBTENER VALOR de una variable COMPARTIDA
 *
 * Pide al kernel el valor (copia, no puntero) de la variable compartida por parametro.
 *
 * @sintax	TEXT_VAR_START_GLOBAL (!)
 * @param	variable	Nombre de la variable compartida a buscar
 * @return	El valor de la variable compartida
 */
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){
	t_valor_variable valor;
	if (corteProg != 1){
		deshabilitar_print_console();
		sprintf(log_cpu,"Pido el valor de la variable compartida %s",variable);
		log_debug_message(log_cpu);
		habilitar_print_console();
		valor = pedirValorGlobal(variable);
		return valor;
	} else {
		deshabilitar_print_console();
		sprintf(log_cpu,"La primitiva obtenerValorCompartida no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
		habilitar_print_console();
		valor = 0;
	}
	return valor;
}


/*
 * ASIGNAR VALOR a variable COMPARTIDA
 *
 * Pide al kernel asignar el valor a la variable compartida.
 * Devuelve el valor asignado.
 *
 * @sintax	TEXT_VAR_START_GLOBAL (!) IDENTIFICADOR TEXT_ASSIGNATION (=) EXPRESION
 * @param	variable	Nombre (sin el '!') de la variable a pedir
 * @param	valor	Valor que se le quire asignar
 * @return	Valor que se asigno
 */
t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){
	t_valor_variable resul;
	if (corteProg != 1){
		deshabilitar_print_console();
		sprintf(log_cpu,"Asigno valor de la variable compartida %s",variable);
		log_debug_message(log_cpu);
		habilitar_print_console();
		resul = asignarValorGlobal(variable,valor);
		return resul;
	} else {
		deshabilitar_print_console();
		sprintf(log_cpu,"La primitiva asignarValorCompartida no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
		habilitar_print_console();
		resul = 0;
	}
	return resul;
}


/*
 *	ENTRADA y SALIDA
 *
 *
 *	@sintax TEXT_IO (io )
 *	@param	dispositivo	Nombre del dispositivo a pedir
 *	@param	tiempo	Tiempo que se necesitara el dispositivo
 *	@return	void
 */
void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){
	if (corteProg != 1){
		deshabilitar_print_console();
		sprintf(log_cpu,"Envio solicitud de I/O al Kernel");
		log_debug_message(log_cpu);
		habilitar_print_console();
		int resul = enviarIOalKernel(dispositivo,tiempo);
		if (resul == 1){
			deshabilitar_print_console();
			log_debug_message("I/O realizada correctamente");
			habilitar_print_console();
		} else {
			log_error_message("Fallo el envio de I/O al Kernel");
		}
		// El programa se desaloja al enviarse a entrada-salida
	} else {
		deshabilitar_print_console();
		sprintf(log_cpu,"La primitiva entradaSalida no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
		habilitar_print_console();
	}
}


/*
 * WAIT
 *
 * Informa al kernel que ejecute la función wait para el semáforo con el nombre identificador_semaforo.
 * El kernel deberá decidir si bloquearlo o no.
 *
 * @sintax	TEXT_WAIT (wait )
 * @param	identificador_semaforo	Semaforo a aplicar WAIT
 * @return	void
 */
void semaforo_wait(t_nombre_semaforo identificador_semaforo){
	if (corteProg != 1){
		deshabilitar_print_console();
		log_debug_message("Hago wait");
		habilitar_print_console();
		hacerWaitSemaforo(identificador_semaforo);
	} else {
		deshabilitar_print_console();
		sprintf(log_cpu,"La primitiva semaforo_wait no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
		habilitar_print_console();
	}
}


/*
 * SIGNAL
 *
 * Informa al kernel que ejecute la función signal para el semáforo con el nombre identificador_semaforo.
 * El kernel deberá decidir si desbloquear otros procesos o no.
 *
 * @sintax	TEXT_SIGNAL (signal )
 * @param	identificador_semaforo	Semaforo a aplicar SIGNAL
 * @return	void
 */
void semaforo_signal(t_nombre_semaforo identificador_semaforo){
	if (corteProg != 1){
		deshabilitar_print_console();
		log_debug_message("Hago signal");
		habilitar_print_console();
		hacerSignalSemaforo(identificador_semaforo);
	} else {
		deshabilitar_print_console();
		sprintf(log_cpu,"La primitiva semaforo_signal no se debe ejecutar, corte de programa");
		log_debug_message(log_cpu);
		habilitar_print_console();
	}
}
