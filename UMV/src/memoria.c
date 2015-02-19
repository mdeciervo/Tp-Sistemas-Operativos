/*
 * memoria.c
 *
 *  Created on: May 5, 2013
 *      Author: ferantivero
 *      Co-Author: cristiann91
 */

#include "memoria.h"

//--------------------------------------------------------------------------------------

char* crear_memoria(int tamanio) {
	char* pmem = malloc(tamanio);
	if (pmem == NULL ) {
		log_error_message("Hubo un error al allocar la memoria.");
		return NULL ;
	} else {
		listaDeSegmentos = list_create();
		t_segmento* segmentoInicial = malloc(sizeof(t_segmento));

		segmentoInicial->pid = 0;
		segmentoInicial->size = tamanio;
		segmentoInicial->ocupado = false;
		segmentoInicial->base = 0;
		segmentoInicial->offset = 0;
		segmentoInicial->numeroSegmento = 0;

		list_add(listaDeSegmentos, segmentoInicial);

		log_info_message("Memoria alocada satisfactoriamente");
		return pmem;
	}
}

//--------------------------------------------------------------------------------------

int numeroRandom(int min, int max) {
	sprintf(log_text,
			"Se PROCEDE A BUSCAR UN NUMERO RANDOM ENTRE: (%i) y: (%i)", min,
			max);
	log_debug_message(log_text);
	if (min > max) {
		log_debug_message("No se puede buscar un numero entre esos valores");
		return -1;
	}
	if (min == max) {
		return min;
	}
	int a = max - min;
	srand(time(NULL ));
	int b = rand() % a;
	int c = b + min;
	return c;

}

//--------------------------------------------------------------------------------------

void mostrarSegmentos(t_list* lista) {
	int i = 0;
	int fin = list_size(lista);
	while (i < fin) {
		log_info_message(
				"--------------------------------------------------------------");
		t_segmento* aux = list_get(lista, i);
		sprintf(log_text, "OFFSET %i", aux->offset);
		log_info_message(log_text);
		sprintf(log_text, "SIZE %i", aux->size);
		log_info_message(log_text);
		sprintf(log_text, "BASE %i", aux->base);
		log_info_message(log_text);
		sprintf(log_text, "OCUPADO %i", aux->ocupado);
		log_info_message(log_text);
		sprintf(log_text, "PID %i", aux->pid);
		log_info_message(log_text);
		sprintf(log_text, "NUMERO DE SEGMENTO %i", aux->numeroSegmento);
		log_info_message(log_text);
		log_info_message(
				"--------------------------------------------------------------");
		i++;
	}
	return;
}

void loguearSegmentos(t_list* lista) {
	int i = 0;
	int fin = list_size(lista);
	while (i < fin) {
		sprintf(log_text,
				"--------------------------------------------------------------\n");
		fputs(log_text, archivoDump);
		t_segmento* aux = list_get(lista, i);
		sprintf(log_text, "OFFSET %i\n", aux->offset);
		fputs(log_text, archivoDump);
		sprintf(log_text, "SIZE %i\n", aux->size);
		fputs(log_text, archivoDump);
		sprintf(log_text, "BASE %i\n", aux->base);
		fputs(log_text, archivoDump);
		sprintf(log_text, "OCUPADO %i\n", aux->ocupado);
		fputs(log_text, archivoDump);
		sprintf(log_text, "PID %i\n", aux->pid);
		fputs(log_text, archivoDump);
		sprintf(log_text, "NUMERO DE SEGMENTO %i\n", aux->numeroSegmento);
		fputs(log_text, archivoDump);
		sprintf(log_text,
				"--------------------------------------------------------------\n");
		fputs(log_text, archivoDump);
		i++;
	}
	return;
}

//------------------------------------------------------------------------------------

int crearSegmento(int pid, int tamanio) {
	log_debug_message("Se procede a crear un segmento");
	int baseCreada = -1;
	log_debug_message("Empieza la busqueda del segmento libre");
	sprintf(log_text,
			"El tamanio necesitado para crear el segmento es: %i del pid %i",
			tamanio, pid);
	log_info_message(log_text);
	if (tamanio == 0) {
		return errorTam0;
	}
//  PREGUNTO SI EL ALGORITMO ES FIRST-FIT
	if (string_equals_ignore_case(configuracionUmv->algoritmo, "First-Fit")) {
		baseCreada = firstfit(pid, tamanio);
	} else {
		baseCreada = worstfit(pid, tamanio);
	}
	if (baseCreada == -1) {
		log_info_message("\n\n&&&&&&& NO HAY ESPACIO, SE PROCEDE A COMPACTAR &&&&&&&\n\n");
		mostrarSegmentos(listaDeSegmentos);
		compactacion();
		mostrarSegmentos(listaDeSegmentos);
//  PREGUNTO SI EL ALGORITMO ES FIRST-FIT
		if (string_equals_ignore_case(configuracionUmv->algoritmo,
				"First-Fit")) {
			baseCreada = firstfit(pid, tamanio);
		} else {
			baseCreada = worstfit(pid, tamanio);
		}
	}
	return baseCreada;
}
//---------------------------------------------------------------------------------------------

int wostFitFiltroYOrden(int tamanio) {
	t_list* listaDeSegmentosLibres = list_create();
	t_segmento* segmentoAux;
	bool condicionOcupado(t_segmento* seg) {
		return !seg->ocupado;
	}
	listaDeSegmentosLibres = list_filter(listaDeSegmentos,
			(void*) condicionOcupado);
	bool ordenPorTamanio(t_segmento* seg, t_segmento* seg2) {
		return seg->size > seg2->size;
	}
	list_sort(listaDeSegmentosLibres, (void*) ordenPorTamanio);
	segmentoAux = list_get(listaDeSegmentosLibres, 0);
	return buscarIndexSegmento(segmentoAux->offset, 0);
}

//---------------------------------------------------------------------------------------------

int worstfit(int pid, int tamanio) {
	int base;
	int index;
	int numero;
	int cantidad = list_size(listaDeSegmentos);
	log_debug_message("Se utiliza el algoritmo Worst-Fit");
	sprintf(log_text,"la lista tiene %i elementos\n", cantidad);
	log_debug_message(log_text);
	index = wostFitFiltroYOrden(tamanio);
	t_segmento* segmentoAux = list_get(listaDeSegmentos, index);
	if (index != -1 && segmentoAux->size >= tamanio) {
		sprintf(log_text,
				"Se encontro un segmento libre con espacio suficiente, offset: (%i), size: (%i)",
				segmentoAux->offset, segmentoAux->size);
		log_debug_message(log_text);
		base = buscarBaseValida(pid, tamanio);
		sprintf(log_text, "Nueva base valida: (%i)", base);
		log_debug_message(log_text);
		if (base == errorPid0) {
			return errorPid0;
		}
		numero = buscarNumeroSegmento(pid);
		sprintf(log_text, "El nuevo segmento va a ser el numero: (%i)", numero);
		log_debug_message(log_text);
		if (segmentoAux->size > tamanio) {
			t_segmento* segmentoAux2 = dameNuevoSegmento(pid,
					segmentoAux->offset, base, true, tamanio, numero);
			list_add_in_index(listaDeSegmentos, index, segmentoAux2);
			segmentoAux->offset = segmentoAux->offset + tamanio;
			segmentoAux->size = segmentoAux->size - tamanio;
		} else {
			segmentoAux->base = base;
			segmentoAux->ocupado = true;
			segmentoAux->pid = pid;
			segmentoAux->numeroSegmento = numero;
		}
		return base;
	} else {
		log_error_message("NO HAY SEGMENTOS DISPONIBLES CON ESE TAMANIO");
		return errorMemOver;
	}
}

//---------------------------------------------------------------------------------------------

int firstfit(int pid, int tamanio) {
	int i;
	int base;
	int numero;
	int cantidad = list_size(listaDeSegmentos);
	log_info_message("Se utiliza el algoritmo First-Fit");
	for (i = 0; i < cantidad;) {
//			OBTENGO UN SEGMENTO
		t_segmento* segmentoAux = list_get(listaDeSegmentos, i);
//			PREGUNTO SI EL SEGMENTO ESTA LIBRE Y SI EL TAMANIO ES MAS SUFICIENTE
		if (!segmentoAux->ocupado && segmentoAux->size >= tamanio) {
			sprintf(log_text,
					"Se encontro un segmento libre con espacio suficiente, offset: (%i), size: (%i)",
					segmentoAux->offset, segmentoAux->size);
			log_debug_message(log_text);
			base = buscarBaseValida(pid, tamanio);
			sprintf(log_text, "Nueva base valida: (%i)", base);
			log_debug_message(log_text);
			if (base == errorPid0) {
//					Base invalida, pid=0
				return errorPid0;
			}
			numero = buscarNumeroSegmento(pid);
			sprintf(log_text, "El nuevo segmento va a ser el numero: (%i)",
					numero);
			log_debug_message(log_text);
//				EL SEGMENTO AUX ESTA LIBRE Y ES MAS GRANDE QUE EL SEGMENTO PEDIDO.
//				EL SEGMENTO SE CREA JUSTO AL PRINCIPIO.
			if (segmentoAux->size > tamanio) {
//				EL SEGMENTO ES MAS GRANDE DE LO NECESARIO, POR ENDE SOBRA Y VA AL FINAL
//				CREO EL NUEVO SEGMENTO QUE VA A IR EN EL INDEX I
				t_segmento* segmentoAux2 = dameNuevoSegmento(pid,
						segmentoAux->offset, base, true, tamanio, numero);
				list_add_in_index(listaDeSegmentos, i, segmentoAux2);
//				EL SEGMENTO LIBRE PASA AL FINAL
				segmentoAux->offset = segmentoAux->offset + tamanio;
				segmentoAux->size = segmentoAux->size - tamanio;
			} else {
//					EL SEGMENTO SE CREA AL PRINCIPIO Y TIENE TAMANIO JUSTO
				segmentoAux->base = base;
				segmentoAux->ocupado = true;
				segmentoAux->pid = pid;
				segmentoAux->numeroSegmento = numero;
			}
			return base;
		} else
			i++;
	}
//	SI SALE DEL FOR ES PORQUE NO HAY NINGUNO LIBRE
	log_error_message("NO HAY SEGMENTOS DISPONIBLES CON ESE TAMANIO");
	return errorMemOver;
}

//---------------------------------------------------------------------------------------------
int buscarBaseValida(int pid, int tamanio) {
	log_debug_message("Se procede a buscar una base valida");
	t_list* segmentos;
	segmentos = listaDeSegmentosDe(pid);
	bool ordenPorBase(t_segmento* seg, t_segmento* seg2) {
		return seg->base < seg2->base;
	}
	if (segmentos == NULL && pid == 0) {
		log_debug_message(
				"La lista esta en NULL porque se quizo buscar una base para el pid 0");
		return errorPid0;
	}
	list_sort(segmentos, (void*) ordenPorBase);
	int i = 0;
	int fin = list_size(segmentos);
	sprintf(log_text, "El tamanio de la lista para buscar la base es: %i", fin);
	log_debug_message(log_text);
	int base = -1;
	t_segmento* seg;
	if (fin > 0) {
		while (i < fin) {
//		AGARRO UN SEGMENTO DE LA LISTA
			seg = list_get(segmentos, i);
//		SI ES EL PRIMERO
			if (i == 0) {
//		SI LA BASE QUE TIENE ES MAYOR AL TAMANIO, DEVUELVO UN NUMERO ENTRE 0 Y LA BASE - EL TAMANIO
				if (seg->base > tamanio) {
					base = numeroRandom(0, seg->base - tamanio);
					return base;
				}
//		SI LA BASE ES IGUAL AL TAMANIO, Y ES EL PRIMERO, LE DOY 0
				if (seg->base == tamanio) {
					return 0;
				}
			}
//		SI NO ES EL ULTIMO
			if (i != (fin - 1)) {
				int a = i + 1;
				t_segmento* segTop = list_get(segmentos, a);
				int suma = seg->base + seg->size;
				int dif = segTop->base - suma;
//		SI LA BASE DEL PROXIMO MENOS LA SUMA ENTRE LA BASE Y EL TAMANIO DEL ACTUAL
//		ES MAYOR AL SIZE QUE QUIERO METER
				if (dif >= tamanio) {
					base = numeroRandom(suma, segTop->base - tamanio);
					return base;
				}
			} else {
//		SI ES EL ULTIMO
				if ((configuracionUmv->tamanioMemoria - (seg->base + seg->size))
						>= tamanio) {
					base = numeroRandom(seg->base + seg->size,
							configuracionUmv->tamanioMemoria - tamanio);
					return base;
				}
			}
			i++;
		}
//		SALE DEL WHILE
//	SI ESTO SE EJECUTA ES PORQUE NO ENCONTRO BASE VALIDA, POR ENDE BUSCA UNA BASE
//	PASANDOSE DEL TAMANIO DE LA MEMORIA
		log_info_message(
				"NO SE ENCONTRO BASE VALIDA,SE AMPLIA EL INTERVALO DE MEMORIA");
		i--;
		seg = list_get(segmentos, i);
		base = numeroRandom(seg->base + seg->size,
				configuracionUmv->tamanioMemoria + (tamanio * 2));
		return base;
	} else {
		log_debug_message(
				"Es el primer segmento, se busca una base random entre 0 y memoria-tamanio");
		return (numeroRandom(0, configuracionUmv->tamanioMemoria - tamanio));
	}
}

//---------------------------------------------------------------------------------------------

int buscarNumeroSegmento(int pid) {
	t_list* lista;
	lista = listaDeSegmentosDe(pid);
	bool ordenPorNumero(t_segmento* seg, t_segmento* seg2) {
		return seg->numeroSegmento > seg2->numeroSegmento;
	}
	list_sort(lista, (void*) ordenPorNumero);
	if (list_size(lista) > 0) {
		t_segmento* segmento = list_get(lista, 0);
		return segmento->numeroSegmento + 1;
	}
	return 1;
}

//-----------------------------------------------------------------------------------

t_list* listaDeSegmentosDe(int pid) {
	//	FILTER POR UN PID
	if (pid == 0) {
		log_debug_message("LLEGO UN PID EN 0");
		return NULL ;
	}

	t_list* listaAuxiliarDeSegmentos;

	bool sosElMismo(t_segmento* seg) {
		return seg->pid == pid;
	}

	sprintf(log_text,
			"Se filtra a ver si hay algun segmento anterior del pid: %i", pid);
	log_debug_message(log_text);

	listaAuxiliarDeSegmentos = list_filter(listaDeSegmentos,
			(void*) sosElMismo);

	sprintf(log_text, "Se retorna la lista de segmentos del pid: %i", pid);
	log_debug_message(log_text);

	return listaAuxiliarDeSegmentos;

}

//------------------------------------------------------------------------------------

t_segmento* dameNuevoSegmento(int pid, int offset, int base, bool ocupado,
		int size, int numero) {
	t_segmento* segmento = malloc(sizeof(t_segmento));
	segmento->base = base;
	segmento->pid = pid;
	segmento->size = size;
	segmento->offset = offset;
	segmento->ocupado = ocupado;
	segmento->numeroSegmento = numero;
	return segmento;
}

//---------------------------------------------------------------------------------------------

void destruirSegmentosDePrograma(int pid) {
	sprintf(log_text, "SE PROCEDE A BORRAR LOS SEGMENTOS DEL PID %i", pid);
	log_info_message(log_text);
	int fin = list_size(listaDeSegmentos);
	int i = 0;
	int existe = 0;
	while (i < fin) {
		t_segmento* segAux = list_get(listaDeSegmentos, i);
		sprintf(log_text, "EL segmento obtenido es el %i", i);
		log_debug_message(log_text);
		if (segAux->pid == pid) {
			sprintf(log_text,
					"EL segmento obtenido es del pid %i. Se procede a liberarlo",
					pid);
			log_debug_message(log_text);
			segAux->base = 0;
			segAux->numeroSegmento = 0;
			segAux->pid = 0;
			segAux->ocupado = false;
			existe = 1;
		}
		i++;
	}
	if (existe == 0) {
		sprintf(log_text, "NO HAY SEGMENTOS DEL PROGRAMA: (%i) EN MEMORIA",
				pid);
		log_info_message(log_text);
		return;
	}
	mostrarSegmentos(listaDeSegmentos);
	return;
}

void mergeTop(t_segmento* segAuxTop, t_segmento* segAux, int i) {
	log_debug_message("Se procede a mergear con top");
	segAuxTop->offset = segAux->offset;
	segAuxTop->size = segAuxTop->size + segAux->size;
	free(segAux);
	list_remove(listaDeSegmentos, i);
}
void mergeBot(t_segmento* segAuxBot, t_segmento* segAux, int i) {
	log_debug_message("Se procede a mergear con bot");
	segAuxBot->size = segAuxBot->size + segAux->size;
	free(segAux);
	list_remove(listaDeSegmentos, i);
}
void dobleMerge(t_segmento* segAuxBot, t_segmento* segAuxTop,
		t_segmento* segAux, int i) {
	log_debug_message("Se procede a mergear Doble");
	segAuxBot->size = segAuxBot->size + segAux->size + segAuxTop->size;
	free(segAux);
	free(segAuxTop);
	list_remove(listaDeSegmentos, i);
	list_remove(listaDeSegmentos, i);

}

//----------------------------------------------------------------------------------------

int compactacion(void) {
	log_info_message(
			"\n******************\n~~~~~~~~~~~~\n\nEMPEZO LA COMPACTACION\n\n******************\n~~~~~~~~~~~~\n");
	int i = 0;
	int fin = list_size(listaDeSegmentos);
	while (i < fin) {
		sprintf(log_text, "SE LEE EL SEGMENTO: (%i)", i);
		log_debug_message(log_text);
		t_segmento* segmento = list_get(listaDeSegmentos, i);
//		SI EL SEGMENTO ESTA LIBRE Y NO ES EL ULTIMO
		if (!segmento->ocupado && ((i + 1) < fin)) {
			log_debug_message("ESTA LIBRE");
			int next = i + 1;
			sprintf(log_text, "SE LEE EL SEGMENTO SIGUIENTE: (%i)", next);
			log_debug_message(log_text);
			t_segmento* segTop = list_get(listaDeSegmentos, next);
			if (!segTop->ocupado) {
//				aca va cuando se juntan libres
				log_debug_message("SON 2 LIBRES, MERGEO TOP");
				mergeTop(segTop, segmento, i);
				log_debug_message("SE MERGEO CORRECTAMENTE");
				fin = list_size(listaDeSegmentos);
				i--;
			} else {
//				es uno libre y arriba uno con datos
//				creo un segmento auxiliar con el top
				log_debug_message("ESTA OCUPADO");
				t_segmento* segAux = copiarSegmento(segTop);
//				al top lo hago que sea el segmento libre
				segTop->offset = segmento->offset;
//				al segmento libre, paso el que estaba arriba y copio los datos
				segmento->offset = segTop->offset + segTop->size;
				log_debug_message("SE COPIA LA MEMORIA MEMORIA");
				memcpy((memoria + segTop->offset), (memoria + segAux->offset),
						segAux->size);
				list_replace(listaDeSegmentos, i, segTop);
				list_replace(listaDeSegmentos, next, segmento);
				log_debug_message("SE COPIO CORRECTAMENTE");
				free(segAux);
			}
		}
		i++;
	}
	log_info_message(
			"\n******************\n~~~~~~~~~~~~\n\nSE COMPACTO CON EXITO\n\n******************\n~~~~~~~~~~~~\n");
	return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------------------


void limpiarMemoria(void) {
	log_info_message("SE PROCEDE A LIMPIAR MEMORIA");
	int fin = list_size(listaDeSegmentos);
	int i = 0;
	while (i < fin) {
		t_segmento* segAux = list_get(listaDeSegmentos, i);
		segAux->base = 0;
		segAux->numeroSegmento = 0;
		segAux->pid = 0;
		segAux->ocupado = false;
		i++;
	}
	mostrarSegmentos(listaDeSegmentos);
	return;
}

//---------------------------------------------------------------------------------------

t_segmento* copiarSegmento(t_segmento* segmentoACopiar) {
	t_segmento* segmentoAux = malloc(sizeof(t_segmento));
	segmentoAux->offset = segmentoACopiar->offset;
	segmentoAux->size = segmentoACopiar->size;
	segmentoAux->pid = segmentoACopiar->pid;
	segmentoAux->ocupado = segmentoACopiar->ocupado;
	segmentoAux->base = segmentoACopiar->base;
	segmentoAux->numeroSegmento = segmentoACopiar->numeroSegmento;
	return segmentoAux;
}

//---------------------------------------------------------------------------------------

int enviarBytesParaGuardar(int base, int offset, int tamanio, void* buffer,
		int pid) {
	t_segmento* segEncontrado = buscarSegmento(base, pid);
	if (segEncontrado) {
//		Encontro el segmento
		int paraEscribir = segEncontrado->size - offset;
		if (paraEscribir >= tamanio) {
//			escribo
			memcpy((memoria + segEncontrado->offset + offset), buffer, tamanio);
			return Ok;
		} else {
			sprintf(log_text,
					"\n++++++++++\nSEGMENTATION FAULT: EL TAMANIO DEL PEDIDO %i EXCEDE AL SEGMENTO %i MENOS OFFSET %i\n++++++++++\n",
					tamanio, segEncontrado->size, offset);
			log_info_message(log_text);
			return errorTamanioLibreMenor;
		}
	}
	return errorNoExisteSegmento;
}


//------------------------------------------------------------------------------------

t_segmento* buscarSegmento(int base, int pid) {
	bool _search_by_id(t_segmento* segmento) {
		return (segmento->base == base) && (segmento->pid == pid);
	}

	return list_find(listaDeSegmentos, (void*) _search_by_id);

}

//---------------------------------------------------------------------------------------------

int buscarIndexSegmento(int offset, int pid) {
	int i;
	int cantidad = list_size(listaDeSegmentos);
	t_segmento* segmentoAux2;
	for (i = 0; i < cantidad;) {
		segmentoAux2 = list_get(listaDeSegmentos, i);
		if ((segmentoAux2->offset == offset) && (segmentoAux2->pid == pid)) {
			return i;
		} else {
			i++;
		}
	}
	return -1;
}

//---------------------------------------------------------------------------------------

char* solicitoBytesDesde(int base, int offset, int tamanio, int pid) {
	t_segmento* segEncontrado = buscarSegmento(base, pid);
	if (segEncontrado) {
//		Encontro el segmento
//		printf("numero de segmento: %i\n", segEncontrado->numeroSegmento);
		char* bytes = malloc(tamanio);
		int leoHasta = segEncontrado->offset + offset + tamanio;
		int puedoLeer = segEncontrado->offset + segEncontrado->size;
		if (puedoLeer >= leoHasta) {
			log_debug_message("Se pueden solicitar bytes correctamente");
			memcpy(bytes, memoria + segEncontrado->offset + offset, tamanio);
			return bytes;
		} else {
			sprintf(log_text,
					"\n++++++++++\nSEGMENTATION FAULT: SE PUEDE LEER HASTA el byte (%i), Y LA SOLICITUD ES DE (%i)\n++++++++++\n",
					puedoLeer, leoHasta);
			log_info_message(log_text);
			return errorLeoMas;
		}
	}
	return errorSegInvalido;
}

//--------------OPERACIONES DEL DUMP-------------------------------------------------------------------------

void estructurasDeMemoria(int pid) {
	int i = 0;
	int j;
	int encontrado;
	int fin = list_size(listaDeSegmentos);
	int pidsMostrados[fin];
	int contador = 0;
	if (pid == 0) {
		t_segmento* segmento;
		while (i < fin) {
			segmento = list_get(listaDeSegmentos, i);
			encontrado = 0;
			for (j = 0; j < contador && encontrado == 0; j++) {
				if (pidsMostrados[j] == segmento->pid) {
					encontrado = 1;
					break;
				}
			}
			if (encontrado == 0 && segmento->pid != 0) {
				pidsMostrados[contador] = segmento->pid;
				mostrarTablaDe(segmento->pid);
				contador++;
			}
			i++;
		}
	} else {
		mostrarTablaDe(pid);
	}
	return;
}

//--------------------------------------------------------------------------------------------

void mostrarTablaDe(int pid) {
	t_list* tablaDelSegmento = listaDeSegmentosDe(pid);
	bool ordenPorNumero(t_segmento* seg, t_segmento* seg2) {
		return seg->numeroSegmento < seg2->numeroSegmento;
	}
	list_sort(tablaDelSegmento, (void*) ordenPorNumero);
	int fin = list_size(tablaDelSegmento);
	int i = 0;
	sprintf(log_text, "TABLA DE SEGMENTOS DEL PROCESO %i\n", pid);
	fputs(log_text, archivoDump);
	if (fin == 0) {
		sprintf(log_text, "NO HAY SEGMENTOS EN MEMORIA DE ESE PROCESO\n");
		fputs(log_text, archivoDump);
		return;
	}
	while (i < fin) {
		t_segmento* segmento = list_get(tablaDelSegmento, i);
		sprintf(log_text, "-------------------------------------------\n");
		fputs(log_text, archivoDump);
		sprintf(log_text, "Identificador: Segmento %i\n",
				segmento->numeroSegmento);
		fputs(log_text, archivoDump);
		sprintf(log_text, "Inicio: %i\n", segmento->base);
		fputs(log_text, archivoDump);
		sprintf(log_text, "Tamanio:  %i\n", segmento->size);
		fputs(log_text, archivoDump);
		sprintf(log_text, "Ubicacion en MP: %i\n", segmento->offset);
		fputs(log_text, archivoDump);
		sprintf(log_text, "-------------------------------------------\n");
		fputs(log_text, archivoDump);
		i++;
	}
	return;
}

//----------------------------------------------------------------------------------------------

void contenidoDeLaMp(int offset, int bytes) {
	char* buffer=malloc(bytes);
	char* direccion = memoria + offset;
	memcpy(buffer, direccion, bytes);
	sprintf(log_text, "El contenido de la MP es:\n");
	fputs(log_text, archivoDump);
	printf("SE ESCRIBE EN DISCO:\n");
	mostrarStringLoco(buffer, bytes);
	escribirStringLoco(buffer, bytes, archivoDump);
	free(buffer);
	return;
}

//----------------------------------------------------------------------------------------------

// DE CONSOLA

//----------------------------------------------------------------------------------------------

int grabarElBuffer(int pid,int base,int inicio, char* template, char* buffer){
	int tam;
	int result;
	if(string_equals_ignore_case(template,"%s")){
		tam = strlen(buffer)+1;
		printf("se guarda el string:\npid: %i\nbase: %i\ninicio: %i\ntamanio: %i\nbuffer: (%s)\n",pid,base,inicio,tam,buffer);
		pthread_mutex_lock(&wrt);
		result=enviarBytesParaGuardar(base,inicio,tam,buffer,pid);
		pthread_mutex_unlock(&wrt);
		if(result==Ok){
			result = STRING;
		}
		return result;
	}
	if(string_equals_ignore_case(template,"%i")){
		int *i=malloc(sizeof(int));
		*i=	atoi(buffer);
		tam=sizeof(int);
		printf("se guarda el int:\npid: %i\nbase: %i\ninicio: %i\ntamanio: %i\nbuffer: (%i)\n",pid,base,inicio,tam,*i);
		pthread_mutex_lock(&wrt);
		result=enviarBytesParaGuardar(base,inicio,tam,i,pid);
		pthread_mutex_unlock(&wrt);
		free(i);
		if (result == Ok) {
			result = INT;
		}
		return result;
	}
	if(string_equals_ignore_case(template,"%c")){
		tam = sizeof(char);
		pthread_mutex_lock(&wrt);
		result=enviarBytesParaGuardar(base,inicio,tam,buffer,pid);
		pthread_mutex_unlock(&wrt);
		char a = buffer[0];
		printf("se guarda el char:\npid: %i\nbase: %i\ninicio: %i\ntamanio: %i\nbuffer: (%c)\n",pid,base,inicio,tam,a);
		if(result==Ok){
			result = CHAR;
		}
		return result;
	}else{
		printf("SE INGRESO UN TEMPLATE INCORRECTO. SOLO SE ADMINTEN %%s - %%i - %%c\n");
		return -1;
	}
}

//----------------------------------------------------------------------------------------------

void* solicitoMemoria(int pid,int base, int inicio, int tamanio, char* template){
	void* result=NULL;
	if(string_equals_ignore_case(template,"%s")){
		preLectura();
		result = solicitoBytesDesde(base, inicio, tamanio, pid);
		postLectura();
		if (string_equals_ignore_case(result, errorLeoMas)) {
			puts(
					"\nSEGMENTATION FAULT: SE EXCEDE EL TAMANIO PERMITIDO POR EL SEGMENTO PARA LECTURA\n");
			return result;
		}
		if (string_equals_ignore_case(result, errorSegInvalido)) {
			puts(
					"\nSEGMENTATION FAULT: SEGMENTO INVALIDO. NO EXISTE EL SEGMENTO\n");
			return result;
		}
		printf(
				"se pide el string:\npid: %i\nbase: %i\ninicio: %i\ntamanio: %i\nstring:",
				pid, base, inicio, tamanio);
		mostrarStringLoco(result,tamanio);
		int a = STRING;
		escribirEnDiscoLoco(result,tamanio,a);
		return result;
	}
	if (string_equals_ignore_case(template, "%i")) {
		if (tamanio >4) {
			puts(
					"\nSEGMENTATION FAULT: SE PIDIO UN INT Y EL TAMANIO ES > 4\n");
			return result ;
		}
		preLectura();
		result = solicitoBytesDesde(base, inicio, tamanio, pid);
		postLectura();
		if (string_equals_ignore_case(result, errorLeoMas)) {
			puts(
					"\nSEGMENTATION FAULT: SE EXCEDE EL TAMANIO PERMITIDO POR EL SEGMENTO PARA LECTURA\n");
			return result ;
		}
		if (string_equals_ignore_case(result, errorSegInvalido)) {
			puts(
					"\nSEGMENTATION FAULT: SEGMENTO INVALIDO. NO EXISTE EL SEGMENTO\n");
			return result ;
		}
		int *i = result;
		printf(
				"se pide el int:\npid: %i\nbase: %i\ninicio: %i\ntamanio: %i\nint: (%i)\n",
				pid, base, inicio, tamanio, *i);

		int b = 12;
		escribirEnDiscoLoco(result,tamanio,b);
		return result;
	}
	if (string_equals_ignore_case(template, "%c")) {
		if(tamanio>1){
			puts(
			"\nSEGMENTATION FAULT: SE PIDIO UN CHAR Y EL TAMANIO ES > 1\n");
			return result ;
		}
		preLectura();
		result = solicitoBytesDesde(base, inicio, tamanio, pid);
		postLectura();
		if (string_equals_ignore_case(result, errorLeoMas)) {
			puts(
					"\nSEGMENTATION FAULT: SE EXCEDE EL TAMANIO PERMITIDO POR EL SEGMENTO PARA LECTURA\n");
			return result ;
		}
		if (string_equals_ignore_case(result, errorSegInvalido)) {
			puts(
					"\nSEGMENTATION FAULT: SEGMENTO INVALIDO. NO EXISTE EL SEGMENTO\n");
			return result ;
		}
		char a;
		char* str=(char*) result;
		a = str[0];
		printf(
				"se pide el char:\npid: %i\nbase: %i\ninicio: %i\ntamanio: %i\nchar: (%c)\n",
				pid, base, inicio, tamanio, a);
		mostrarStringSinNulo(result,tamanio);
		int c = CHAR;
		escribirEnDiscoLoco(result,tamanio,c);
		return result;
	} else {
		printf(
				"SE INGRESO UN TEMPLATE INCORRECTO. SOLO SE ADMINTEN %%s - %%i - %%c\n");
		return result;
	}
}
