/*
 ============================================================================
 Name        : Programa.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/tad_cliente.h>
#include <commons/log_app.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/shared_strs.h>

char* prepend(char * str, char c);
char* append(const char *s, char* c);

int main(int argc, char **argv) {
	char log_text[2000];
	char* nombre = string_substring_from(argv[1], 2);
	char* nombre_log = string_duplicate(nombre);
	char* log = ".log";
	string_append(&nombre_log, log);
	inicializar_log(nombre_log, "Programa");
	habilitar_log_info();
	sprintf(log_text, "Proceso Programa %s Iniciado", nombre);
	log_info_message(log_text);
	log_info_message("Chequeando el path del script...");
	if (argv[1] == NULL ) {
		log_error_message("El path del script no puede ser vacio.");
		finalizar_log();
		return EXIT_FAILURE;
	}

//	CONEXION CON KERNEL
	t_KER_PRO_CPU_UMV mensajeAEnviar = obtener_nueva_shared_str();
	t_KER_PRO_CPU_UMV* mensajeARecibir;
	log_debug_message("Se inicia la conexion con el KERNEL");
	int socket;
//	ACA VA LO DE LA VARIABLE DE ENTORNO
	char* rutaConfiguracion = getenv("ANSISOP_CONFIG");
	t_config *config = config_create(rutaConfiguracion);
	int puerto = config_get_int_value(config, "Puerto");
	char* ip = config_get_string_value(config, "IP");
	sprintf(log_text, "LA IP ES: %s\n", ip);
	log_info_message(log_text);
	sprintf(log_text, "EL PUERTO ES: %i\n", puerto);
	log_info_message(log_text);
	socket = conectar_a_servidor(ip, puerto);
	if (socket == -1) {
		return EXIT_FAILURE;
	}
	log_debug_message("Conectado\n");

//	SE LEVANTA EL SCRIPT
	sprintf(log_text, "Se levantan datos del script en el path:%s", argv[1]);
	log_info_message(log_text);

	int num = 0;
	log_debug_message("se abre el archivo");
	FILE* script;
	script = fopen(argv[1], "r");
	log_debug_message("se abrio el archivo");
	int tamanio;
	fseek(script, 0, SEEK_END);
	tamanio = ftell(script) + 1;
	sprintf(log_text, "El tamanio del script es: (%d)", tamanio);
	log_info_message(log_text);
	char codigo[tamanio];
	rewind(script);
	char aux;
	while (feof(script) == 0) {
		aux = fgetc(script);
		codigo[num] = aux;
		num++;
	}
	log_debug_message("se termino de leer");

//	printf("num vale: (%d)\n", num);
	num = num - 1;
//	printf("ahora num vale: (%d)\n", num);

	sprintf(log_text, "LEO: ('nueva linea')y pongo en posicion (%d)", num);
	log_debug_message(log_text);
	codigo[num] = '\n';
	num++;
	sprintf(log_text, "LEO: ('barra 0')y pongo en posicion (%d)", num);
	log_debug_message(log_text);
	codigo[num] = '\0';

	sprintf(log_text, "El script es: \n%s", codigo);
	log_debug_message(log_text);

	sprintf(log_text, "La longitud del codigo es: %i\n", strlen(codigo));
	log_info_message(log_text);

	fclose(script);

	mensajeAEnviar.codigo = codigo;
	mensajeAEnviar.mensaje = nombre;
	log_debug_message("se copia el codigo");

	sprintf(log_text, "El codigo copiado en el mensaje es: (%s)",
			mensajeAEnviar.codigo);
	log_debug_message(log_text);

//	SE ENVIA EL HANDSHAKE

	log_info_message(
			"Se envia HANDSHAKE al PLP con el Codigo y el Nombre del programa\n");
	enviar_mensaje_a_servidor(socket, HANDSHAKE_PROGRAMA_PLP, &mensajeAEnviar);
	void *msj = recibir_mensaje_de_servidor(socket);
	if (msj != NULL ) {
		mensajeARecibir = (t_KER_PRO_CPU_UMV*) msj;
	} else {
		log_debug_message("ERROR AL RECIBIR MENSAJE NULO");
		return -1;
	}
	if (mensajeARecibir->gen_msg.id_MSJ != HANDSHAKE_PROGRAMA_PLP
			|| mensajeARecibir->OK != 1) {
		sprintf(log_text, "%s\n", mensajeARecibir->mensaje);
		log_info_message(log_text);
		return -1;
	} else {
		log_info_message("Respuesta Handshake entre PROGRAMA-KERNEL recibida");
	}

	while (1) {
		log_info_message(
				"\nEL PROGRAMA SE QUEDA ESPERANDO MENSAJES DE IMPRIMIR/ IMPRIMIR TEXTO O FINALIZAR EJECUCION\n");
		msj = recibir_mensaje_de_servidor(socket);
		if (msj != NULL ) {
			mensajeARecibir = (t_KER_PRO_CPU_UMV*) msj;
		} else {
			log_debug_message("ERROR AL RECIBIR MENSAJE NULO");
			return -1;
		}

		switch (mensajeARecibir->gen_msg.id_MSJ) {
		case IMPRIMIR: {
			log_info_message("LLEGO MENSAJE CON ID IMPRIMIR");
			sprintf(log_text, "SE IMPRIME EL VALOR DE LA VARIABLE: %i\n",
					mensajeARecibir->valor);
			log_info_message(log_text);
			break;
		}
		case IMPRIMIR_TEXTO: {
//			SE HACE LO DE IMPRIMIR TEXTO
			log_info_message("LLEGO MENSAJE CON ID IMPRIMIR TEXTO");
			sprintf(log_text, "SE IMPRIME EL TEXTO:\n\n %s\n",
					mensajeARecibir->texto);
			log_info_message(log_text);
			break;
		}
		case FINALIZACION_EJECUCION_PROGRAMA: {
			log_info_message(
					"\n---------------LLEGO UN MENSAJE DE FINALIZACION----------------\n");
			log_info_message("\nLA EJECUCION DE ESTE PROGRAMA HA CONCLUIDO\n");
			return EXIT_SUCCESS;
			break;
		}
		default: {
			log_info_message(
					"\n---------------LLEGO UN MENSAJE DESCONOCIDO A PROGRAMA----------------\n");
		}
		}
	}
	return EXIT_SUCCESS;
}

char *prepend(char * str, char c) {
	char aux[1]; // add 2 to make room for the character we will prepend and the null termination character at the end
	aux[0] = c;
	aux[1] = '\0';
	char* ol = append(str, aux);
	return ol;
}

char *append(const char *s, char* c) {
	int len = strlen(s);
	char buf[len + 2];
	strcpy(buf, s);
	strcat(buf, c);
	return strdup(buf);
}
