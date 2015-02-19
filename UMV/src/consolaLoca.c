/*
 * consolaLoca.c
 *
 *  Created on: 03/07/2014
 *      Author: utnso
 */

#include "consolaLoca.h"

void consolaLoca(void) {
	puts(
			"\nSE INICIO LA CONSOLA. ESCRIBA ALGUN COMANDO, USANDO ESPACIO PARA CADA PARAMETRO.\n");
	while (1) {
		puts(
				"\nOPERACIONES VALIDAS:\n**********   grabar PID BASE INICIO TEMPLATE BUFFER\n**********   solicitar PID BASE INICIO TAMANIO TEMPLATE\n**********   crear PID TAMANIO\n**********   destruir PID ---- 0 LIBERA MEMORIA\n**********   algoritmo ALGORITMO---- FIRST-FIT / WORST FIT\n**********   retardo RETARDO");
		puts(
				"**********   compactacion\n**********   dumpMP\n**********   dumpEstructuras PID------0 PARA TODAS LAS TABLAS\n**********   dumpContenidoMP OFFSET CANTIDAD\n");
		char* comando = malloc(100);
		__fpurge(stdin);
		fgets(comando, 100, stdin);
		int operacion = detectarEspacio(comando);
		if (string_equals_ignore_case(comando, "compactacion\n")) {
//			porque es el unico comando que va solo, sino es error
			operacion = 12;
		}
		if (string_equals_ignore_case(comando, "dumpMP\n")) {
//			porque es el unico comando que va solo, sino es error
			operacion = 6;
		}
		if (operacion == -1) {
			printf("\nOPERACION INCORRECTA.\n\n");
		} else {
			char* op = string_substring_until(comando, operacion);
			char* restoComando = string_substring_from(comando, operacion + 1);
			manejarOperacion(op, restoComando);
		}
		free(comando);
	}
	return;
}

int detectarLong(char* string) {
	int fin = strlen(string);
//	printf("la long es %i\n", fin);
	int i = 0;
	while (i < fin) {
//		printf("el caracter leido es : (%c)\n",string[i]);
		if (string[i] == '\n') {
//			printf("SE RETORNA %i\n",i);
			return i;
		}
		i++;
	}
//	SI LLEGO ACA ES PORQUE NO HABIA ESPACIO
	return -1;
}

int detectarEspacio(char* string) {
	int fin = strlen(string);
//	printf("la long es %i\n",fin);
	int i = 0;
	while (i < fin) {
//		printf("el caracter leido es : (%c)\n",string[i]);
		if (string[i] == ' ') {
//			printf("SE RETORNA %i\n",i);
			return i;
		}
		i++;
	}
//	SI LLEGO ACA ES PORQUE NO HABIA ESPACIO
	return -1;
}

void manejarOperacion(char* operacion, char* restoComando) {
	printf("LA OPERACION ES : (%s)\n", operacion);
//	printf("RESTO ES : (%s)\n", restoComando);
	if (string_equals_ignore_case(operacion, "grabar")) {
		grabarBytesConsola(restoComando);
		return;
	}
	if (string_equals_ignore_case(operacion, "solicitar")) {
		solicitarBytesConsola(restoComando);
		return;
	}
	if (string_equals_ignore_case(operacion, "crear")) {
		crearSegmentoConsola(restoComando);
		return;
	}
	if (string_equals_ignore_case(operacion, "destruir")) {
		destruirSegmentosConsola(restoComando);
		return;
	}
	if (string_equals_ignore_case(operacion, "algoritmo")) {
		char* strAlgo = string_substring(restoComando, 0,
				strlen(restoComando) - 1);
		printf("EL ALGORITMO ACTUAL ES (%s)\n", configuracionUmv->algoritmo);
		printf("SE QUIERE CAMBIAR A (%s)\n", strAlgo);
		if (string_equals_ignore_case(strAlgo, "First-fit")
				|| string_equals_ignore_case(strAlgo, "Worst-fit")) {
			configuracionUmv->algoritmo = strAlgo;
			printf("\nSE CAMBIO A (%s)\n\n", strAlgo);
		} else {
			printf("\nSE INGRESO UN ALGORITMO NO VALIDO\n\n");
		}
		return;
	}
	if (string_equals_ignore_case(operacion, "retardo")) {
		char* strAlgo = string_substring(restoComando, 0,
				strlen(restoComando) - 1);
		printf("EL RETARDO ACTUAL ES (%i)\n", configuracionUmv->retardo);
		int ret = atoi(strAlgo);
		if (ret >= 0) {
			configuracionUmv->retardo = ret;
			printf("\nSE CAMBIO A (%i)\n\n", ret);
		} else {
			printf("\nSE INGRESO UN RETORNO NO VALIDO\n\n");
		}
		return;
	}
	if (string_equals_ignore_case(operacion, "compactacion")) {
		printf("SE EJECUTA LA COMPACTACION\n");
		pthread_mutex_lock(&wrt);
		compactacion();
		pthread_mutex_unlock(&wrt);
		return;
	}
	if (string_equals_ignore_case(operacion, "dumpMp")) {
		dumpLoco(2, 0, 0, 0);
		return;
	}
	if (string_equals_ignore_case(operacion, "dumpEstructuras")) {
		char* strPid = string_substring(restoComando, 0,
		strlen(restoComando) - 1);
		int pid = atoi(strPid);
		if (pid < 0) {
			puts(
					"\n++++++++++\nSEGMENTATION FAULT: SE QUIERE GENERAR UN DUMP CON PID < 0\n++++++++++\n");
			return;
		}
		dumpLoco(1, pid, 0, 0);
		return;
	}
	if (string_equals_ignore_case(operacion, "dumpContenidoMp")) {
		contenidoMP(restoComando);
		return;
	} else {
		printf("\nOPERACION NO VALIDA!!!! \n\n");
	}

}

void grabarBytesConsola(char* restoComando) {
	int aux;
//	PID
	aux = detectarEspacio(restoComando);
	if (aux == -1) {
		printf(
				"Error falta un caracter de espacio en el comando para detectar el PID\n");
		return;
	}
	char* strPid = string_substring_until(restoComando, aux);
	int pid = atoi(strPid);
	printf("EL PID ES (%i)\n", pid);
	if(pid<=0){
			printf(
					"\n******SEGMENTATION FAULT: PID <= 0 *********\n\n");
			return;
		}
	restoComando = string_substring_from(restoComando, aux + 1);
//	printf("EL STRING QUE QUEDA ES (%s)\n", restoComando);
//	BASE
	aux = detectarEspacio(restoComando);
	if (aux == -1) {
		printf(
				"Error falta un caracter de espacio en el comando para detectar la BASE\n");
		return;
	}
	char* strBase = string_substring_until(restoComando, aux);
	int base = atoi(strBase);
	printf("LA BASE ES (%i)\n", base);
	restoComando = string_substring_from(restoComando, aux + 1);
//	printf("EL STRING QUE QUEDA ES (%s)\n", restoComando);
//	INICIO
	aux = detectarEspacio(restoComando);
	if (aux == -1) {
		printf(
				"Error falta un caracter de espacio en el comando para detectar el INICIO\n");
		return;
	}
	char* strIni = string_substring_until(restoComando, aux);
	int inicio = atoi(strIni);
	printf("EL INICIO ES (%i)\n", inicio);
	if(inicio<0){
		printf(
				"\n******SEGMENTATION FAULT: OFFSET < 0 *********\n\n");
		return;
	}
	restoComando = string_substring_from(restoComando, aux + 1);
//	printf("EL STRING QUE QUEDA ES (%s)\n", restoComando);
//	TEMPLATE
	aux = detectarEspacio(restoComando);
	if (aux == -1) {
		printf(
				"Error falta un caracter de espacio en el comando para detectar el TEMPLATE\n");
		return;
	}
	char* strTemplate = string_substring_until(restoComando, aux);
	printf("EL TEMPLATE ES (%s)\n", strTemplate);
	restoComando = string_substring(restoComando, aux + 1,
			strlen(restoComando) - 4);
//	printf("EL STRING QUE QUEDA ES (%s) con tamanio (%i)\n", restoComando,
//			strlen(restoComando));
	aux = grabarElBuffer(pid, base, inicio, strTemplate, restoComando);
	if (aux == errorTamanioLibreMenor) {
		puts(
				"\n++++++++++\nSEGMENTATION FAULT: EL TAMANIO DEL PEDIDO EXCEDE EL SIZE DEL SEGMENTO\n++++++++++\n");
		return;
	}
	if (aux == errorNoExisteSegmento) {
		puts(
				"\n++++++++++\nSEGMENTATION FAULT: SEGMENTO INVALIDO. NO SE PUEDEN GUARDAR BYTES, PORQUE NO EXISTE EL SEGMENTO\n++++++++++\n");
		return;
	}
	if (aux == -1) {
		return;
	}
	printf("SE GRABO CORRECTAMENTE\n");
	escribirEnDiscoLoco(restoComando, strlen(restoComando) + 1, aux);
	return;
}

void solicitarBytesConsola(char* restoComando) {
	int aux;
	//	PID
	aux = detectarEspacio(restoComando);
	if (aux == -1) {
		printf(
				"Error falta un caracter de espacio en el comando para detectar el PID\n");
		return;
	}
	char* strPid = string_substring_until(restoComando, aux);
	int pid = atoi(strPid);
	printf("EL PID ES (%i)\n", pid);
	if(pid<=0){
				printf(
						"\n******SEGMENTATION FAULT: PID <= 0 *********\n\n");
				return;
			}
	restoComando = string_substring_from(restoComando, aux + 1);
//	printf("EL STRING QUE QUEDA ES (%s)\n", restoComando);
	//	BASE
	aux = detectarEspacio(restoComando);
	if (aux == -1) {
		printf(
				"Error falta un caracter de espacio en el comando para detectar la BASE\n");
		return;
	}
	char* strBase = string_substring_until(restoComando, aux);
	int base = atoi(strBase);
	printf("LA BASE ES (%i)\n", base);
	restoComando = string_substring_from(restoComando, aux + 1);
//	printf("EL STRING QUE QUEDA ES (%s)\n", restoComando);
	//	INICIO
	aux = detectarEspacio(restoComando);
	if (aux == -1) {
		printf(
				"Error falta un caracter de espacio en el comando para detectar el INICIO\n");
		return;
	}
	char* strIni = string_substring_until(restoComando, aux);
	int inicio = atoi(strIni);
	printf("EL INICIO ES (%i)\n", inicio);
	if (inicio < 0) {
		printf("\n******SEGMENTATION FAULT: OFFSET < 0 *********\n\n");
		return;
	}
	restoComando = string_substring_from(restoComando, aux + 1);
//	printf("EL STRING QUE QUEDA ES (%s)\n", restoComando);
	//	TAMANIO
	aux = detectarEspacio(restoComando);
	if (aux == -1) {
		printf(
				"Error falta un caracter de espacio en el comando para detectar el TAMANIO\n");
		return;
	}
	char* strTam = string_substring_until(restoComando, aux);
	int tamanio = atoi(strTam);
	printf("EL TAMANIO ES (%i)\n", tamanio);
	if (tamanio <= 0) {
			printf("\nSEGMENTATION FAULT: TAMANIO <= 0\n\n");
			return;
		}
	restoComando = string_substring_from(restoComando, aux + 1);
//	printf("EL STRING QUE QUEDA ES (%s)\n", restoComando);

	aux = detectarLong(restoComando);
	if (aux == -1) {
		printf(
				"Error falta un caracter de espacio en el comando para detectar la TEMPLATE\n");
		return;
	}
	char* strTem = string_substring_until(restoComando, aux);
	printf("EL TEMPLATE ES (%s)\n", strTem);
	char* resu = solicitoMemoria(pid, base, inicio, tamanio, strTem);
	free(resu);
	return;
}

void crearSegmentoConsola(char* restoComando) {
	int aux;
	aux = detectarEspacio(restoComando);
	if (aux == -1) {
		printf(
				"Error falta un caracter de espacio en el comando para detectar el PID\n");
		return;
	}
	char* strPid = string_substring_until(restoComando, aux);
	int pid = atoi(strPid);
	printf("EL PID ES (%i)\n", pid);
	if(pid<=0){
			printf(
					"\n******SEGMENTATION FAULT: PID <= 0 *********\n\n");
			return;
	}
	restoComando = string_substring(restoComando, aux + 1,
			strlen(restoComando) - 1);
//	printf("EL STRING QUE QUEDA ES (%s) de tamanio (%i)\n", restoComando,
//			strlen(restoComando));
	int tam = atoi(restoComando);
	printf("EL TAMANIO ES (%i)\n", tam);
	if (tam < 0) {
		printf(
				"\n****** SEGMENTATION FAULT: NO SE PUEDE CREAR UN SEGMENTO CON TAMANIO NEGATIVO *******\n\n");
		return;
	}
	pthread_mutex_lock(&wrt);
	int base = crearSegmento(pid, tam);
	pthread_mutex_unlock(&wrt);
	if (base == errorMemOver) {
		puts(
				"\n++++++++++\nMEMORY OVERLOAD: NO HAY ESPACIO SUFICIENTE PARA CREAR EL SEGMENTO\n++++++++++\n");
		return;
	}
	if (base == errorPid0) {
		puts(
				"\n++++++++++\nSEGMENTATION FAULT: SE QUIERE CREAR UN SEGMENTO CON PID 0\n++++++++++\n");
		return;
	}
	if (base == errorTam0) {
		puts(
				"\n++++++++++\nSEGMENTATION FAULT: SE QUIERE CREAR UN SEGMENTO CON TAMANIO 0\n++++++++++\n");
		return;
	}
	printf("SE CREO CON EXITO. LA BASE ES (%i)\n", base);
	return;
}
void destruirSegmentosConsola(char* restoComando) {
	char* strAlgo = string_substring(restoComando, 0, strlen(restoComando) - 1);
	int pid = atoi(strAlgo);
	if (pid > 0) {
		printf("\nSE DESTRUYEN LOS SEGMENTOS DEL PID (%i)\n\n", pid);
		pthread_mutex_lock(&wrt);
		destruirSegmentosDePrograma(pid);
		pthread_mutex_unlock(&wrt);
		printf("SE EJECUTO LA DESTRUCCION CORRECTAMENTE\n");
		return;
	}
	if(pid==0){
		printf("\nSE DESTRUYEN TODOS LOS SEGMENTOS\n\n");
		pthread_mutex_lock(&wrt);
		limpiarMemoria();
		pthread_mutex_unlock(&wrt);
	}else{
		puts(
			"\n++++++++++\nSEGMENTATION FAULT: EL PID PARA DESTRUIR SEGMENTOS DEBE SER > 0\n++++++++++\n");
		return;
	}
	return;
}

void escribirEnDiscoLoco(char* bufferAux, int tamanio, int tipo) {
	puts("\n¿DESEA ESCRIBIRLO EN DISCO?\n1-SI\n2-NO");
	int opcD;
	__fpurge(stdin);
	scanf("%i", &opcD);
	if (opcD == 1) {
		banderaConsola++;
		char* nombreBuffer = malloc(14);
		sprintf(nombreBuffer, "buffer%i.txt", banderaConsola);
		printf("El nombre del archivo buffer es (%s)\n", nombreBuffer);
		FILE* archivoBuffer;
		archivoBuffer = fopen(nombreBuffer, "w");
//		printf(
//				"SE QUIERE ESCRIBIR EN DISCO &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");
		switch (tipo) {
		case 9: { //string
//			printf("string\n");
			mostrarStringLoco(bufferAux, tamanio);
			escribirStringLoco(bufferAux, tamanio, archivoBuffer);
			break;
		}
		case 10: { //int str
//			printf("int\n");
			mostrarStringSinNulo(bufferAux, tamanio);
			escribirStringSinNulo(bufferAux, tamanio, archivoBuffer);
			break;
		}
		case 11: { //char
//			printf("char\n");
			mostrarStringSinNulo(bufferAux, tamanio);
			escribirStringSinNulo(bufferAux, tamanio, archivoBuffer);
			break;
		}
		case 12: { //int solo
//			printf("int solo\n");
			int *i = (int *) bufferAux;
//			printf("\n%i\n", *i);
			fprintf(archivoBuffer, "%i", *i);
			break;
		}
		}
//		fwrite(bufferAux,tamanio,1,archivoBuffer);
		fclose(archivoBuffer);
		printf("SE ESCRIBIO EN DISCO CORRECTAMENTE\n");
		free(nombreBuffer);
	}
//	free(bufferAux);
	return;
}

void escribirStringLoco(char* cadena, int longitud, FILE* archivo) {
	int o = 0;
	while (o < longitud) {
		if (cadena[o] == '\0') {
			fprintf(archivo, "\\0");
			o++;
		}
		fputc(cadena[o], archivo);
		o++;
	}
	return;
}

void mostrarStringLoco(char* cadena, int longitud) {
	int o = 0;
	printf("\n");
	while (o < longitud) {

		if (cadena[o] == '\0') {
			printf("\\0");
		}
		printf("%c", cadena[o]);
		o++;
	}
	printf("\n\n");
}

void contenidoMP(char* restoComando) {
	int aux;
	//	OFFSET
	aux = detectarEspacio(restoComando);
	if (aux == -1) {
		printf(
				"Error falta un caracter de espacio en el comando para detectar el Offset\n");
		return;
	}
	char* strOffset = string_substring_until(restoComando, aux);
	int offset = atoi(strOffset);
	printf("EL offset ES (%i)\n", offset);
	if (offset < 0) {
		puts(
				"\n++++++++++\nSEGMENTATION FAULT: SE QUIERE PEDIR EL CONTENIDO DE MEMORIA DESDE UN OFFSET NEGATIVO\n++++++++++\n");
		return;
	}
	restoComando = string_substring_from(restoComando, aux + 1);
//	printf("EL STRING QUE QUEDA ES (%s)\n", restoComando);
	//	CANTIDAD
	aux = detectarLong(restoComando);
	if (aux == -1) {
		printf(
				"Error falta un caracter de espacio en el comando para detectar la Cantidad\n");
		return;
	}
	char* strCant = string_substring_until(restoComando, aux);
	int cant = atoi(strCant);
	printf("LA CANTIDAD ES (%i)\n", cant);
	if (cant < 0) {
			puts(
					"\n++++++++++\nSEGMENTATION FAULT: SE QUIERE PEDIR EL CONTENIDO DE MEMORIA CON UN TAMANIO NEGATIVO\n++++++++++\n");
			return;
	}

	dumpLoco(3, 0, offset, cant);
	return;
}

void dumpLoco(int opcion, int pid, int offset, int cantidad) {

	archivoDump = fopen("Dump.txt", "a");
	switch (opcion) {
	case 1: {
		fputs(temporal_get_string_time(), archivoDump);
		fputs("\nESTRUCTURAS DE MEMORIA:\n", archivoDump);
		puts("SE EJECUTA EL COMANDO DE ESTRUCTURAS DE MEMORIA");
		preLectura();
		estructurasDeMemoria(pid);
		postLectura();
		fputs(
				"\n|||||||||||||||||||||||||FIN DE REPORTE||||||||||||||||||||||||||\n\n\n\n",
				archivoDump);
		puts("REPORTE GENERADO CORRECTAMENTE\n");
		break;
	}
	case 2: {
		fputs(temporal_get_string_time(), archivoDump);
		fputs("\nSEGMENTOS DE LA MEMORIA:\n", archivoDump);
		puts("SE EJECUTA EL COMANDO DE REPORTE MEMORIA PRINCIPAL");
		preLectura();
		loguearSegmentos(listaDeSegmentos);
		postLectura();
		fputs(
				"\n|||||||||||||||||||||||||FIN DE REPORTE||||||||||||||||||||||||||\n\n\n\n",
				archivoDump);
		puts("REPORTE GENERADO CORRECTAMENTE\n");
		break;
	}
	case 3: {
		fputs(temporal_get_string_time(), archivoDump);
		fputs("\nSE MUESTRA EL CONTENIDO DE LA MEMORIA PRINCIPAL:\n",
				archivoDump);
		puts("SE EJECUTA EL COMANDO DE CONTENIDO EN LA MEMORIA PRINCIPAL");
		preLectura();
		contenidoDeLaMp(offset, cantidad);
		postLectura();
		fputs(
				"\n|||||||||||||||||||||||||FIN DE REPORTE||||||||||||||||||||||||||\n\n\n\n",
				archivoDump);
		puts("REPORTE GENERADO CORRECTAMENTE\n");
		break;
	}
	default:
		puts("SE ELIGIO UNA OPCION NO VALIDA");
		break;
	}
	fclose(archivoDump);
	return;
}

void mostrarStringSinNulo(char* cadena, int longitud) {
	int o = 0;
	printf("\n");
	while (o < longitud) {

		if(cadena[o]!='\0'){
		printf("%c", cadena[o]);
		}
		o++;
	}
	printf("\n\n");
}

void escribirStringSinNulo(char* cadena, int longitud, FILE* archivo) {
	int o = 0;
	while (o < longitud) {
		if (cadena[o] != '\0') {
			fputc(cadena[o], archivo);
		}
		o++;
	}
	return;
}
