/*
 ============================================================================
 Name        : UMV.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "configUMV.h"
#include "memoria.h"
#include "testsUmv.h"
#include "consola.h"
#include "consolaLoca.h"
#include "sincro.h"

//GLOBALES
char log_text[2000];
char* memoria;
t_list* listaDeSegmentos;
configUmv* configuracionUmv;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wrt = PTHREAD_MUTEX_INITIALIZER;
int readcount = 0;
int banderaConsola = 0;
FILE* archivoDump;

//CABECERAS DE LAS FUNCIONES
t_KER_PRO_CPU_UMV* handler_UVM_msgs(t_KER_PRO_CPU_UMV* msg);
t_KER_PRO_CPU_UMV * copiar_mensaje(t_KER_PRO_CPU_UMV* msg);
void interactuarKernel(mensajeHandshake* handshake);
void interactuarCpu(mensajeHandshake* handshake);

int main(int argc, char **argv) {
//	CHEQUEO SI QUIERO CORRER TESTS

	if (strcmp(argv[1], "-corremeLosTests") == 0)
		correrTests();
	else {

//	COMIENZA EL CODIGO

//		char* umv = malloc(sizeof(char) * strlen("UMV - "));
//		strcpy(umv, "UMV - ");
//		string_append(&umv, temporal_get_string_time());
//		string_append(&umv, ".log");
		inicializar_log("UMV.log", "UMV");
//		free(umv);
		deshabilitar_print_console();
		deshabilitar_log_debug();
		deshabilitar_log_trace();
		habilitar_log_info();
		habilitar_log_error();
		log_info_message(
				"^^^^^^^^^^^^^^^^^^^^^^^^^^^^SE INICIA LA UMV^^^^^^^^^^^^^^^^^^^^^^^^^^^^");

//	ACA SE LEVANTA LA CONF
//	---------------------------------------------------------------------------------------------------
		log_debug_message("Chequeando el path de configuracion de la UMV...");

		if (argv[1] == NULL ) {
			log_error_message("El path de la UMV no puede ser vacio.");
			finalizar_log();
			return EXIT_FAILURE;
		}

		sprintf(log_text, "Levantando configuracion en el path: %s", argv[1]);
		log_info_message(log_text);

		configuracionUmv = levantarConfiguracion(argv[1]);

		mostrarConfiguracion();
//	----------------------------------------------------------------------------------------------

//	ACA SE CREA LA MEMORIA PRINCIPAL
		memoria = crear_memoria(configuracionUmv->tamanioMemoria);

//	ACA ARRANCA LA CONSOLA
		pthread_t hiloConsola;
		pthread_create(&hiloConsola, NULL, (void*) consolaLoca, NULL);

//	SE ABREN LAS CONEXIONES
		abrir_conexion_servidor(configuracionUmv->puerto, &handler_UVM_msgs);

	}
	free(configuracionUmv);
	free(memoria);
	return EXIT_SUCCESS;

}

t_KER_PRO_CPU_UMV* handler_UVM_msgs(t_KER_PRO_CPU_UMV* msg) {
//	ACA SE MANEJAN LOS MENSAJES QUE PUEDEN LLEGAR
	log_info_message(
			"############LLEGO UN MENSAJE AL HANDLER DE HANDSHAKES############");
	if (msg != NULL ) {
//		SI NO ES NULO
		t_GEN_MSG* mensajeGen = (t_GEN_MSG *) msg;
		if (!mensajeGen->disconnected) {
//					SI NO ESTA DESCONECTADO
			sprintf(log_text, "el MENSAJE es: (%i) (%s)", mensajeGen->id_MSJ,
					id_a_mensaje(mensajeGen->id_MSJ));
			log_info_message(log_text);
			mensajeHandshake* handshake = malloc(sizeof(mensajeHandshake));
			handshake->socket = mensajeGen->socket_descriptor_server;
			handshake->ID = mensajeGen->id_MSJ;
			sprintf(log_text,
					"Se tiene una estructura de handshake con ID: (%s) y socket numero: (%i)",
					id_a_mensaje(handshake->ID), handshake->socket);
			log_debug_message(log_text);
			switch (mensajeGen->id_MSJ) {
			case HANDSHAKE_PLP_UMV: {
				log_debug_message(
						"PRIMERA VEZ, SE CONTROLA HANDSHAKE DE KERNEL");
				sprintf(log_text,
						"El socket server del HANDSHAKE_PLP_UMV es: (%i)",
						handshake->socket);
				log_debug_message(log_text);
				t_KER_PRO_CPU_UMV* mensaje = (t_KER_PRO_CPU_UMV *) msg;
				t_KER_PRO_CPU_UMV resp_msg = obtener_nueva_shared_str();
				log_debug_message("ENVIO HANDSHAKE");
				resp_msg.OK = 1;
				resp_msg.gen_msg.socket_descriptor_server =
						mensaje->gen_msg.socket_descriptor_server;
				resp_msg.gen_msg.id_MSJ = mensaje->gen_msg.id_MSJ;
				enviar_mensaje(resp_msg.gen_msg.socket_descriptor_server,
						HANDSHAKE_PLP_UMV, &resp_msg);
				log_debug_message("HANDSHAKE ENVIADO");
				pthread_t hiloKernel;
				log_debug_message(
						"--------------------SE CREA EL HILO DE KERNEL");
				sprintf(log_text,
						"El socket server que se envia al hilo es: (%i)",
						handshake->socket);
				log_debug_message(log_text);
				delegar_conexion(handshake->socket);
				pthread_create(&hiloKernel, NULL, (void*) interactuarKernel,
						handshake);
				break;
			}
			case HANDSHAKE_CPU_UMV: {
				log_debug_message("PRIMERA VEZ, SE CONTROLA HANDSHAKE DE CPU");
				sprintf(log_text,
						"El socket server del HANDSHAKE_CPU_UMV es: (%i)",
						handshake->socket);
				log_debug_message(log_text);
				t_KER_PRO_CPU_UMV* mensaje = (t_KER_PRO_CPU_UMV *) msg;
				t_KER_PRO_CPU_UMV resp_msg = obtener_nueva_shared_str();
				log_debug_message("ENVIO HANDSHAKE");
				resp_msg.OK = 1;
				resp_msg.gen_msg.socket_descriptor_server =
						mensaje->gen_msg.socket_descriptor_server;
				resp_msg.gen_msg.id_MSJ = mensaje->gen_msg.id_MSJ;
				enviar_mensaje(resp_msg.gen_msg.socket_descriptor_server,
						HANDSHAKE_CPU_UMV, &resp_msg);
				log_debug_message("HANDSHAKE ENVIADO");
				pthread_t hiloCpu;
				sprintf(log_text,
						"--------------------SE CREA EL HILO DE CPU %i",
						mensaje->identificador_cpu);
				handshake->numero = mensaje->identificador_cpu;
				log_debug_message(log_text);
				sprintf(log_text,
						"El socket server que se envia al hilo es: (%i)",
						handshake->socket);
				log_debug_message(log_text);
				delegar_conexion(handshake->socket);
				pthread_create(&hiloCpu, NULL, (void*) interactuarCpu,
						handshake);
				break;
			}
			default: {
				log_info_message("LLEGO UN MENSAJE QUE NO TIENE ID CONOCIDA");
				break;
			}
			}
		} else {
			sprintf(log_text,
					"[DESCONEXION EN UMV] de cliente con socket server %d:%s de UMV (nunca ha hecho handshake ok)",
					mensajeGen->socket_descriptor_server,
					identidad_socket_server[mensajeGen->socket_descriptor_server]);
			log_info_message(log_text);
		}
	} else {
		log_error_message("Se ha recibido mensaje nulo en UMV");
	}
	return NULL ;
}

t_KER_PRO_CPU_UMV * copiar_mensaje(t_KER_PRO_CPU_UMV* msg) {
	t_KER_PRO_CPU_UMV * aux = malloc(sizeof(t_KER_PRO_CPU_UMV));
	aux->gen_msg.disconnected = msg->gen_msg.disconnected;
	aux->gen_msg.socket_descriptor = msg->gen_msg.socket_descriptor;
	aux->gen_msg.socket_descriptor_server =
			msg->gen_msg.socket_descriptor_server;
	aux->gen_msg.id_MSJ = msg->gen_msg.id_MSJ;
	aux->gen_msg.size_str = msg->gen_msg.size_str;
	return aux;
}

void interactuarKernel(mensajeHandshake* handshake) {
	log_info_message(
			"-----------------SE COMIENZA A INTERACTUAR CON EL KERNEL--------------");
	int socketPosta = handshake->socket;
	sprintf(log_text, "El socket que llego por parametro es: (%i)",
			socketPosta);
	log_debug_message(log_text);
	free(handshake);
	int result;
	int resultado;
	t_KER_PRO_CPU_UMV* mensaje;
	while (1) {
		sprintf(log_text, "Se espera mensaje en hilo kernel por socket : (%i)",
					socketPosta);
		log_info_message(log_text);
		void *msj = recibir_mensaje(socketPosta);
		log_info_message(
				"**************SE RECIBIO UN MENSAJE EN EL HILO DE KERNEL**************");
		if (msj != NULL ) {
			mensaje = (t_KER_PRO_CPU_UMV*) msj;
		} else {
			log_error_message(
					"Se ha recibido mensaje nulo en UMV en hilo de kernel");
			sprintf(log_text,
					"[DESCONEXION EN UMV] de cliente con socket server %d:%s en el hilo de KERNEL",
					socketPosta, identidad_socket_server[socketPosta]);
			log_info_message(log_text);
			limpiarMemoria();
			pthread_exit(NULL );
		}

		sprintf(log_text, "Se recibio de socket: (%d) el mensaje: (%s)",
				mensaje->gen_msg.socket_descriptor,
				id_a_mensaje(mensaje->gen_msg.id_MSJ));
		log_info_message(log_text);
		sleep(configuracionUmv->retardo);
		switch (mensaje->gen_msg.id_MSJ) {
		case CREAR_SEGMENTOS: {
			t_KER_PRO_CPU_UMV mensajeAEnviar = obtener_nueva_shared_str();
			pthread_mutex_lock(&wrt);
			resultado = crearSegmento(mensaje->PID, mensaje->tamanio_segmento);
			pthread_mutex_unlock(&wrt);
			if (resultado == errorMemOver) {
				mensajeAEnviar.mensaje="\n++++++++++\nMEMORY OVERLOAD: NO HAY ESPACIO SUFICIENTE PARA CREAR EL SEGMENTO\n++++++++++\n";
				log_info_message(mensajeAEnviar.mensaje);
				mensajeAEnviar.OK = 0;
				enviar_mensaje(socketPosta, CREAR_SEGMENTOS, &mensajeAEnviar);
//				destruirSegmentosDePrograma(mensaje->PID);
				break;
			}
			if (resultado == errorPid0) {
				mensajeAEnviar.mensaje="\n++++++++++\nSEGMENTATION FAULT: SE QUIERE CREAR UN SEGMENTO CON PID 0\n++++++++++\n";
				log_info_message(mensajeAEnviar.mensaje);
				mensajeAEnviar.OK = 0;
				enviar_mensaje(socketPosta, CREAR_SEGMENTOS, &mensajeAEnviar);
				break;
			}
			if (resultado == errorTam0) {
				log_info_message("Se pidio crear segmento con tamanio en 0. No se crea el segmento, pero se devuelve OK en 1 y una base de error en -5");
				mensajeAEnviar.OK = 1;
				mensajeAEnviar.base_segmento=errorTam0;
				enviar_mensaje(socketPosta, CREAR_SEGMENTOS, &mensajeAEnviar);
				break;
			}
			mensajeAEnviar.OK=1;
			mensajeAEnviar.base_segmento = resultado;
			sprintf(log_text,
								"SE HA CREADO SEGMENTO PARA PID: (%i) CON EXITO, LA BASE ES: (%i) y el OK en %i",
								mensaje->PID, resultado,mensajeAEnviar.OK);
			log_info_message(log_text);
			enviar_mensaje(socketPosta, CREAR_SEGMENTOS, &mensajeAEnviar);
//				preLectura();
//				mostrarSegmentos(listaDeSegmentos);
//				postLectura();
			break;
		}
		case DESTRUIR_SEGMENTOS: {
//			t_KER_PRO_CPU_UMV mensajeAEnviar = obtener_nueva_shared_str();
			pthread_mutex_lock(&wrt);
			destruirSegmentosDePrograma(mensaje->PID);
			pthread_mutex_unlock(&wrt);
//			mensajeAEnviar.OK = Ok;
//			enviar_mensaje(socketPosta, DESTRUIR_SEGMENTOS, &mensajeAEnviar);
			break;
		}
		case ENVIAR_BYTES: {
			t_KER_PRO_CPU_UMV mensajeAEnviar = obtener_nueva_shared_str();
			sprintf(log_text, "EN EL SEGMENTO: %i del PID: %i",
					mensaje->base_bytes, mensaje->PID);
			log_debug_message(log_text);
			sprintf(log_text, "SE QUIERE GUARDAR: %s", mensaje->buffer);
			log_debug_message(log_text);
			pthread_mutex_lock(&wrt);
			result = enviarBytesParaGuardar(mensaje->base_bytes,
					mensaje->offset_bytes, mensaje->tamanio_bytes,
					mensaje->buffer, mensaje->PID);
			pthread_mutex_unlock(&wrt);
			if (result == errorTamanioLibreMenor) {
				mensajeAEnviar.mensaje="\n++++++++++\nSEGMENTATION FAULT: EL TAMANIO DEL PEDIDO EXCEDE EL SIZE DEL SEGMENTO\n++++++++++\n";
				mensajeAEnviar.OK = 0;
				enviar_mensaje(socketPosta, ENVIAR_BYTES, &mensajeAEnviar);
				break;
			}
			if (result == errorNoExisteSegmento) {
				mensajeAEnviar.mensaje="\n++++++++++\nSEGMENTATION FAULT: SEGMENTO INVALIDO. NO SE PUEDEN GUARDAR BYTES, PORQUE NO EXISTE EL SEGMENTO\n++++++++++\n";
				log_info_message(mensajeAEnviar.mensaje);
				mensajeAEnviar.OK = 0;
				enviar_mensaje(socketPosta, ENVIAR_BYTES, &mensajeAEnviar);
				break;
			}
			sprintf(log_text, "SE ENVIA RESPUESTA DE GUARDADO OK %i", result);
			log_debug_message(log_text);
			mensajeAEnviar.OK = Ok;
//				para ver que guardo
//				char* aver=solicitoBytesDesde(mensaje->base_bytes,mensaje->offset_bytes,mensaje->tamanio_bytes,mensaje->PID);
//				printf("LO QUE GUARDE: %s\n",aver);
			enviar_mensaje(socketPosta, ENVIAR_BYTES, &mensajeAEnviar);
			break;
		}
		default:
			log_info_message(
					"%%%%%%%%%%%%%LLEGO UN MENSAJE DESCONOCIDO AL HILO DEL KERNEL%%%%%%%%%%%%%");
			break;
		}
		free(mensaje);
	}
	return;
}

void interactuarCpu(mensajeHandshake* handshake) {
	sprintf(log_text,
			"+++++++++++++++++++++SE COMIENZA A INTERACTUAR CON LA CPU %i ++++++++++++++++++++",
			handshake->numero);
	log_info_message(log_text);
	int numero = handshake->numero;
	int socketPosta = handshake->socket;
	int procesoEnEjecucion;
	char* resultado;
	int result;
	sprintf(log_text, "EL socket que llego por parametro es: (%i)",
			socketPosta);
	log_debug_message(log_text);
	free(handshake);
	t_KER_PRO_CPU_UMV* mensaje;
	while (1) {
		sprintf(log_text, "Se espera mensaje en hilo cpu numero: (%i) por socket : (%i)",
		numero,socketPosta);
		log_info_message(log_text);
		void *msj = recibir_mensaje(socketPosta);
		sprintf(log_text,
				"$$$$$$$$$$$$$$$$ SE RECIBIO UN MENSAJE EN EL HILO DE CPU %i $$$$$$$$$$$$$$$$$",
				numero);
		log_info_message(log_text);
		if (msj != NULL ) {
			mensaje = (t_KER_PRO_CPU_UMV*) msj;
		} else {
			sprintf(log_text,
					"Se ha recibido mensaje nulo en UMV en hilo de CPU numero: %i",
					numero);
			log_error_message(log_text);
			sprintf(log_text,
					"[DESCONEXION EN UMV] de cliente con socket server %d:%s en el hilo de CPU numero %i",
					socketPosta, identidad_socket_server[socketPosta], numero);
			log_info_message(log_text);
			pthread_exit(NULL );
		}
		sprintf(log_text, "Se recibio de socket: (%d) el mensaje: (%s)",
				mensaje->gen_msg.socket_descriptor,
				id_a_mensaje(mensaje->gen_msg.id_MSJ));
		log_info_message(log_text);
		sleep(configuracionUmv->retardo);
		switch (mensaje->gen_msg.id_MSJ) {
		case CAMBIO_PROCESO_ACTIVO: {
			t_KER_PRO_CPU_UMV mensajeAEnviar = obtener_nueva_shared_str();
			sprintf(log_text,
					"CAMBIO PROCESO EN EJECUCION ACTUAL: %i del cpu %i\n",
					procesoEnEjecucion, numero);
			log_debug_message(log_text);
			procesoEnEjecucion = mensaje->PID;
			mensajeAEnviar.OK = 1;
			enviar_mensaje(socketPosta, CAMBIO_PROCESO_ACTIVO, &mensajeAEnviar);
			sprintf(log_text,
					"\n++++++++++\nNUEVO PROCESO EN EJECUCION ACTUAL: %i del cpu %i\n++++++++++\n",
					procesoEnEjecucion, numero);
			log_info_message(log_text);
			break;
		}
		case SOLICITAR_BYTES: {
			t_KER_PRO_CPU_UMV mensajeAEnviar = obtener_nueva_shared_str();
			sprintf(log_text,
					"Las direcciones son: \nbase: %i\noffset: %i\ntamanio: %i",
					mensaje->base_bytes, mensaje->offset_bytes,
					mensaje->tamanio_bytes);
			log_debug_message(log_text);
			preLectura();
			resultado = solicitoBytesDesde(mensaje->base_bytes,
					mensaje->offset_bytes, mensaje->tamanio_bytes,
					procesoEnEjecucion);
			postLectura();
			if(string_equals_ignore_case(resultado,errorLeoMas)){
					mensajeAEnviar.mensaje="\n++++++++++\nSEGMENTATION FAULT: SE EXCEDE EL TAMANIO PERMITIDO POR EL SEGMENTO PARA LECTURA\n++++++++++\n";
					mensajeAEnviar.OK = 0;
					enviar_mensaje(socketPosta, SOLICITAR_BYTES, &mensajeAEnviar);
					break;
			}
			if(string_equals_ignore_case(resultado,errorSegInvalido)){
					mensajeAEnviar.mensaje="\n++++++++++\nSEGMENTATION FAULT: SEGMENTO INVALIDO. NO EXISTE EL SEGMENTO\n++++++++++\n";
					log_info_message(mensajeAEnviar.mensaje);
					mensajeAEnviar.OK = 0;
					enviar_mensaje(socketPosta, SOLICITAR_BYTES, &mensajeAEnviar);
					break;
			}
			mensajeAEnviar.OK = Ok;
			mensajeAEnviar.tamanio_bytes = mensaje->tamanio_bytes;
			mensajeAEnviar.buffer = resultado;
			enviar_mensaje(socketPosta, SOLICITAR_BYTES, &mensajeAEnviar);
			free(resultado);
			break;
		}
		case ENVIAR_BYTES: {
			t_KER_PRO_CPU_UMV mensajeAEnviar = obtener_nueva_shared_str();
			sprintf(log_text, "EN EL SEGMENTO: %i del PID: %i",
					mensaje->base_bytes, procesoEnEjecucion);
			log_debug_message(log_text);
			sprintf(log_text, "SE QUIERE GUARDAR: %s", mensaje->buffer);
			log_debug_message(log_text);
			pthread_mutex_lock(&wrt);
			result = enviarBytesParaGuardar(mensaje->base_bytes,
					mensaje->offset_bytes, mensaje->tamanio_bytes,
					mensaje->buffer, procesoEnEjecucion);
			pthread_mutex_unlock(&wrt);
			if (result == errorTamanioLibreMenor) {
				mensajeAEnviar.mensaje="\n++++++++++\nSEGMENTATION FAULT: STACK OVERFLOW. EL TAMANIO DEL PEDIDO PARA GRABAR BYTES EXCEDE EL SIZE DEL SEGMENTO\n++++++++++\n";
				mensajeAEnviar.OK = 0;
				enviar_mensaje(socketPosta, ENVIAR_BYTES, &mensajeAEnviar);
				break;
			}
				if (result == errorNoExisteSegmento) {
				mensajeAEnviar.mensaje="\n++++++++++\nSEGMENTATION FAULT: SEGMENTO INVALIDO. NO SE PUEDEN GUARDAR BYTES, PORQUE NO EXISTE EL SEGMENTO\n++++++++++\n";
				log_info_message(mensajeAEnviar.mensaje);
				mensajeAEnviar.OK = 0;
				enviar_mensaje(socketPosta, ENVIAR_BYTES, &mensajeAEnviar);
				break;
			}
			sprintf(log_text, "SE ENVIA RESPUESTA DE GUARDADO OK %i", result);
			log_debug_message(log_text);
			mensajeAEnviar.OK = Ok;
//				para ver que guardo
//				char* aver=solicitoBytesDesde(mensaje->base_bytes,mensaje->offset_bytes,mensaje->tamanio_bytes,mensaje->PID);
//				printf("LO QUE GUARDE: %s\n",aver);
			enviar_mensaje(socketPosta, ENVIAR_BYTES, &mensajeAEnviar);
			break;
		}
		case DESTRUIR_SEGMENTOS: {
			t_KER_PRO_CPU_UMV mensajeAEnviar = obtener_nueva_shared_str();
			pthread_mutex_lock(&wrt);
			destruirSegmentosDePrograma(procesoEnEjecucion);
			pthread_mutex_unlock(&wrt);
			mensajeAEnviar.OK = 1;
			enviar_mensaje(socketPosta, DESTRUIR_SEGMENTOS, &mensajeAEnviar);
			break;
		}
		default: {
			sprintf(log_text,
					"@@@@@@@@@@@@@@@@@ LLEGO UN MENSAJE DESCONOCIDO AL HILO DEL CPU %i @@@@@@@@@@@@@@@@@",
					numero);
			log_info_message(log_text);
			break;
		}
		}
		free(mensaje);
	}
	return;
}

