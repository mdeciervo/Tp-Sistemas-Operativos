/*
 * consola.c
 *
 *  Created on: 28/04/2014
 *      Author: utnso
 */
#include "consola.h"

void estaSeriaLaConsola() {
	puts("SE INICIA LA CONSOLA");
	int opcion;
	while (1) {
		__fpurge(stdin);
		puts(
				"ELIJA LA OPCION DESEADA INGRESANDO EL NUMERO:\n1 - OPERACION\n2 - RETARDO\n3 - ALGORITMO\n4 - COMPACTACION\n5 - DUMP");
		scanf("%i", &opcion);
		//	fgets(opcion,sizeof(opcion),stdin);
		switch (opcion) {
		case OPERACION:
			puts("SE ELIGIO LA OPCION OPERACION");
			operacion();
			puts("TERMINO LA OPERACION\n");
			break;
		case RETARDO:
			puts("SE ELIGIO LA OPCION PARA CAMBIAR EL RETARDO");
			retardo();
			break;
		case ALGORITMO:
			puts("SE ELIGIO LA OPCION PARA CAMBIAR EL ALGORITMO");
			algoritmo();
			break;
		case COMPACTACION:
			puts("SE ELIGIO LA OPCION DE COMPACTACION");
			pthread_mutex_lock(&wrt);
			compactacion();
			pthread_mutex_unlock(&wrt);
			break;
		case DUMP:
			puts("SE ELIGIO LA OPCION DUMP");
			dump();
			break;
		default:
			puts("OPCION NO VALIDA");
			break;
		}
	}
	return;
}

void operacion() {
	bool sigo = true;
	int opc;
	while (sigo) {
		__fpurge(stdin);
		puts(
				"INDIQUE QUE DESEA HACER:\n1-SOLICITAR BYTES\n2-ESCRIBIR BYTES\n3-CREAR SEGMENTO\n4-DESTRUIR SEGMENTOS\n5-VOLVER AL MENU ANTERIOR");
		scanf("%i", &opc);
		switch (opc) {
		case 1: {
			mostrarPosicionDeMemoria();
			sigo = false;
			break;
		}
		case 2: {
			escribirEnBuffer();
			sigo = false;
			break;
		}
		case 3: {
			opcionCrearSegmento();
			sigo = false;
			break;
		}
		case 4: {
			opcionDestruirSegmento();
			sigo = false;
			break;
		}
		case 5: {
			sigo = false;
			break;
		}
		default:
			puts("OPCION ERRONEA");
			break;
		}
	}
	return;
}

void mostrarPosicionDeMemoria() {
	puts("SE ELIGIO LA OPCION DE SOLICITAR BYTES");
	int numPro = verificarPID();
	int base = verificarBase();
	int offset = verificarOffset();
	int tam = verificarTamanio();
	puts("SE TRAE LA POSICION:");
	char* buffer;
	preLectura();
	buffer = solicitoBytesDesde(base, offset, tam, numPro);
	postLectura();
	if (string_equals_ignore_case(buffer, errorLeoMas)) {
		printf(
				"\nSEGMENTATION FAULT: SE EXCEDE EL TAMANIO PERMITIDO POR EL SEGMENTO PARA LECTURA\n");
		return;
	}
	if (string_equals_ignore_case(buffer, errorSegInvalido)) {
		printf(
				"\nSEGMENTATION FAULT: SEGMENTO INVALIDO. NO EXISTE EL SEGMENTO\n");
		return;
	}
	mostrarString(buffer, tam);
	escribirEnDisco(buffer, tam);
	return;
}

void escribirEnBuffer() {
	int resultado;
	puts("SE ELIGIO LA OPCION DE ENVIAR BYTES");
	int numPro = verificarPID();
	int base = verificarBase();
	int offset = verificarOffset();
	int tam = verificarTamanio();
	char* buffer = verificarBuffer(tam);
	mostrarString(buffer, tam);
	pthread_mutex_lock(&wrt);
	resultado = enviarBytesParaGuardar(base, offset, tam, buffer, numPro);
	pthread_mutex_unlock(&wrt);
	switch (resultado) {
	case errorTamanioLibreMenor: {
		printf(
				"\n++++++++++\nSEGMENTATION FAULT: EL TAMANIO DEL PEDIDO EXCEDE EL SIZE DEL SEGMENTO\n++++++++++\n");
		break;
	}
	case errorNoExisteSegmento: {
		printf(
				"\n++++++++++\nSEGMENTATION FAULT: SEGMENTO INVALIDO. NO SE PUEDEN GUARDAR BYTES, PORQUE NO EXISTE EL SEGMENTO\n++++++++++\n");
		break;
	}
	default: {
		printf("SE ESCRIBIO EN EL BUFFER:\n%s\n", buffer);
		escribirEnDisco(buffer, tam);
		break;
	}
	}
	free(buffer);
	return;
}

void mostrarString(char* cadena, int longitud) {
	int o = 0;
	while (o < longitud) {
		printf("%c", cadena[o]);
		o++;
	}
	puts("");
	return;
}

void escribirString(char* cadena, int longitud, FILE* archivo) {
	int o = 0;
	while (o < longitud) {
		if (cadena[o] != '\0') {
			fputc(cadena[o], archivo);
		}
		o++;
	}
	return;
}

void escribirEnDisco(char* bufferAux, int tamanio) {
	int opcD;
	__fpurge(stdin);
	puts("¿DESEA ESCRIBIRLO EN DISCO?\n1-SI\n2-NO");
	scanf("%i", &opcD);
	while (opcD != 1 && opcD != 2) {
		__fpurge(stdin);
		puts("OPCION INVALIDA!");
		puts("¿DESEA ESCRIBIRLO EN DISCO?\n1-SI\n2-NO");
		scanf("%i", &opcD);
	}
	if (opcD == 1) {
		__fpurge(stdin);
		banderaConsola++;
		char* nombreBuffer = malloc(13);
		sprintf(nombreBuffer, "buffer%i.txt", banderaConsola);
		printf("%s\n", nombreBuffer);
		FILE* archivoBuffer;
		archivoBuffer = fopen(nombreBuffer, "a");
		puts("SE QUIERE ESCRIBIR EN DISCO:");
		mostrarString(bufferAux, tamanio);
		escribirString(bufferAux, tamanio, archivoBuffer);
		fclose(archivoBuffer);
		puts("SE ESCRIBIO EN DISCO:");
		mostrarString(bufferAux, tamanio);
		free(nombreBuffer);
	}
	return;
}

void opcionCrearSegmento() {
	puts("SE ELIGIO LA OPCION DE CREAR SEGMENTO");
	int numPro = verificarPID();
	int tam = verificarTamanio();
	pthread_mutex_lock(&wrt);
	int resultado = crearSegmento(numPro, tam);
	pthread_mutex_unlock(&wrt);
	switch (resultado) {
	case errorMemOver: {
		printf(
				"\n++++++++++\nMEMORY OVERLOAD: NO HAY ESPACIO SUFICIENTE PARA CREAR EL SEGMENTO\n++++++++++\n");
		break;
	}
	case errorPid0: {
		printf(
				"\n++++++++++\nSEGMENTATION FAULT: SE QUIERE CREAR UN SEGMENTO CON PID 0\n++++++++++\n");
		break;
	}
	case errorTam0: {
		printf(
				"Se pidio crear segmento con tamanio en 0. No se crea el segmento, pero se devuelve OK en 1 y una base de error en -5\n");
		break;
	}
	default: {
		printf("SE CREO EL SEGMENTO CON LA BASE %i\n", resultado);
	}
	}
	return;
}

void opcionDestruirSegmento() {
	puts("SE ELIGIO LA OPCION DE DESTRUIR SEGMENTO");
	int numPro = verificarPID();
	pthread_mutex_lock(&wrt);
	destruirSegmentosDePrograma(numPro);
	pthread_mutex_unlock(&wrt);
	puts("SE DESTRUYERON LOS SEGMENTOS CORRECTAMENTE");
	return;
//	TODO
}

void retardo() {
	int retardo = 0;
	printf("EL RETARDO ACTUAL ES: %i\n", configuracionUmv->retardo);
	while (1) {
		__fpurge(stdin);
		printf(
				"INGRESE EL NUEVO NUMERO DE RETARDO ('-1' PARA VOLVER AL MENU ANTERIOR): ");
		scanf("%i", &retardo);
		if (retardo >= 0) {
			configuracionUmv->retardo = retardo;
			printf("RETARDO ACTUALIZADO SATISFACTORIAMENTE: %i\n",
					configuracionUmv->retardo);
			return;
		}
		if (retardo != -1) {
			puts("NUMERO DE RETARDO INVALIDO");
		}
		if (retardo == -1) {
			return;
		}
	}
	__fpurge(stdin);
}

void algoritmo() {
	int a;
	printf("EL ALGORITMO ACTUAL ES: %s\n", configuracionUmv->algoritmo);
	while (1) {
		__fpurge(stdin);
		puts(
				"ELIJA EL NUEVO ALGORITMO:\n1-FIRST-FIT\n2-WORST-FIT\n3-VOLVER AL MENU ANTERIOR");
		scanf("%i", &a);
		switch (a) {
		case 1: {
			configuracionUmv->algoritmo = "First-Fit";
			printf("EL ALGORITMO ACTUAL ES: %s\n", configuracionUmv->algoritmo);
			return;
		}
		case 2: {
			configuracionUmv->algoritmo = "Worst-Fit";
			printf("EL ALGORITMO ACTUAL ES: %s\n", configuracionUmv->algoritmo);
			return;
		}
		case 3: {
			return;
		}
		default: {
			puts("OPCION INVALIDA");
		}
		}
	}
}

void dump() {
	archivoDump = fopen("Dump.txt", "a");
	while (1) {
		__fpurge(stdin);
		puts(
				"ELIJA EL REPORTE DESEADO:\n1-ESTRUCTURAS DE MEMORIA\n2-MEMORIA PRINCIPAL\n3-CONTENIDO DE LA MEMORIA PRINCIPAL\n4-VOLVER AL MENU ANTERIOR");
		int a;
		scanf("%i", &a);
		switch (a) {
		case 1: {
			fputs(temporal_get_string_time(), archivoDump);
			fputs("\nESTRUCTURAS DE MEMORIA:\n", archivoDump);
			puts("SE ELIGIO LA OPCION DE ESTRUCTURAS DE MEMORIA");
			puts(
					"INGRESE NUMERO DE PID: \nSI DESEA LAS TABLAS DE TODOS LOS PROCESOS INGRESE CERO");
			int b;
			__fpurge(stdin);
			scanf("%i", &b);
			preLectura();
			estructurasDeMemoria(b);
			postLectura();
			fputs(
					"\n|||||||||||||||||||||||||FIN DE REPORTE||||||||||||||||||||||||||\n\n\n\n",
					archivoDump);
			fclose(archivoDump);
			return;
		}
		case 2: {
			fputs(temporal_get_string_time(), archivoDump);
			fputs("\nSEGMENTOS DE LA MEMORIA:\n", archivoDump);
			puts("SE ELIGIO LA OPCION DE MEMORIA PRINCIPAL");
			preLectura();
			loguearSegmentos(listaDeSegmentos);
			postLectura();
			fputs(
					"\n|||||||||||||||||||||||||FIN DE REPORTE||||||||||||||||||||||||||\n\n\n\n",
					archivoDump);
			fclose(archivoDump);
			return;
		}
		case 3: {
			fputs(temporal_get_string_time(), archivoDump);
			fputs("%s\nSE MUESTRA EL CONTENIDO DE LA MEMORIA PRINCIPAL:\n",
					archivoDump);
			puts("SE ELIGIO LA OPCION DE CONTENIDO EN LA MEMORIA PRINCIPAL");
			int offset = verificarOffset();
			int bytes = verificarMayorACero("INGRESE LA CANTIDAD DE BYTES: ");
			preLectura();
			contenidoDeLaMp(offset, bytes);
			postLectura();
			fputs(
					"\n|||||||||||||||||||||||||FIN DE REPORTE||||||||||||||||||||||||||\n\n\n\n",
					archivoDump);
			fclose(archivoDump);
			return;
		}
		case 4: {
			fclose(archivoDump);
			return;
		}
		default:
			puts("SE ELIGIO UNA OPCION NO VALIDA");
			break;
		}
	}
}

int verificarMayorACero(char* cadena) {
	int numero = 0;
	while (numero <= 0) {
		__fpurge(stdin);
		printf("%s", cadena);
		scanf("%i", &numero);
	}
	return numero;
}

int verificarMayorOIgualACero(char* cadena) {
	int numero = -1;
	while (numero < 0) {
		__fpurge(stdin);
		printf("%s", cadena);
		scanf("%i", &numero);
	}
	return numero;
}

int verificarTamanio() {
	int tamanio = 0;
	while (tamanio <= 0 || tamanio > configuracionUmv->tamanioMemoria) {
		__fpurge(stdin);
		printf("INGRESE EL TAMANIO: ");
		scanf("%i", &tamanio);
	}
	return tamanio;
}

char* verificarBuffer(int tamanio) {
	char* buffer = calloc(tamanio, sizeof(char));
	__fpurge(stdin);
	printf("LA CADENA TIENE QUE TENER COMO MAXIMO %i CARACTERES\n",
			tamanio - 1);
	puts("ESCRIBA EN EL BUFFER (TENGA EN CUENTA EL CARACTER NULO '\\0'):");
	fgets(buffer, tamanio, stdin);
	mostrarString(buffer, tamanio);
	return buffer;
}

int verificarPID() {
	return verificarMayorACero("INGRESE EL NUMERO DE PROCESO: ");
}

int verificarBase() {
	return verificarMayorACero("INGRESE LA BASE: ");
}

int verificarOffset() {
	return verificarMayorOIgualACero("INGRESE EL OFFSET: ");
}
