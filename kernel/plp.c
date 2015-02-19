#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "plp.h"
#include "print_kernel.h"

extern t_PLP* plp;

uint32_t obtener_PID_programa() {
	plp->ultimo_PID_programa++;
	return plp->ultimo_PID_programa;
}

t_Programa* remover_programa_por_socket_server(int socket_server) {
	bool mismo_socket_server(t_Programa *programa) {
		return programa->socket_server == socket_server;
	}
	t_Programa* programa;
	programa = syncqueue_remover(plp->ready->planif_queue, (void*) mismo_socket_server);
	if (programa == NULL )
		programa = synclist_remover(plp->new->planif_list,
				(void*) mismo_socket_server);

	return programa;
}


void callback_programa_finalizado()
{
	void exit_manejo_nivel_multiprogramacion()
	{
		log_info_message("[PLP]  Multiprogramacion Post por finalizacion de programa");
	}
	//subo en uno el nivel del multiprogramacion restante
	syncronize_mutex_cont_post(&plp->nivel_multiprogramacion, (void*)exit_manejo_nivel_multiprogramacion);
}
void pasar_programa_new_a_ready(void* args) {

	while(1)
	{
		void productor_consumidor_new_wait() {

			void manejo_nivel_multiprogramacion()
			{
				void productor_consumidor_ready_post() {

					//RC
					void* proximo_segun_SJN()
					{
						//hay al menos un slot disponible y se agrego un nuevo programa a new, entonces ordenamos
						bool retornar_mas_chico(t_Programa *progA, t_Programa *progB) {
							return progA->peso < progB->peso;
						}
						list_sort(plp->new->planif_list->list, (void*) retornar_mas_chico);

						//como se ordeno, el primero de la cola es el mas chico (para SJN)
						t_Programa* programa_menos_pesado = list_remove(
								plp->new->planif_list->list,0);

						programa_menos_pesado->cq = 'R';
						print_estado();

						//muevo de new a ready segun SJN
						syncqueue_push(plp->ready->planif_queue,programa_menos_pesado);

						return programa_menos_pesado;
					}

					syncronize_pthread_mutex(&plp->new->planif_list->mutex,	proximo_segun_SJN);

				}
				syncqueue_mutex_cont_post(plp->ready,
						(void*) productor_consumidor_ready_post);
			}

			//se agrego un nuevo programa y entonces baja en uno la multiprogramacion restante
			syncronize_mutex_cont_wait(&plp->nivel_multiprogramacion, (void*)manejo_nivel_multiprogramacion);
		}
		//espero hasta que se agregue un nuevo programa
		synclist_mutex_cont_wait(plp->new, (void*) productor_consumidor_new_wait);
	}
}

void sync_nuevo_programa(t_Programa* prog) {

	void productor_consumidor_new_post() {
		prog->cq = 'N';
		print_estado();

		synclist_add(plp->new->planif_list, prog);
	}
	synclist_mutex_cont_post(plp->new, (void*)productor_consumidor_new_post);

}

void imprimirMetadata(t_medatada_program* metadata) {
	printf("Cantidad de etiquetas %i\n", metadata->cantidad_de_etiquetas);
	printf("Cantidad de funciones %i\n", metadata->cantidad_de_funciones);
	printf("Tamanioo del mapa serializado de etiquetas %i\n",
			metadata->etiquetas_size);
	int i = 0;
	if (metadata->etiquetas_size != 0) {
		for (i = 0; i < (metadata->etiquetas_size + 1); i++) {
			printf("%c", metadata->etiquetas[i]);
		}
	}
	printf("\n");
	printf("Tamanio del mapa serializado de instrucciones %i\n",
			metadata->instrucciones_size);
	printf("El numero de la primera instruccion es %i\n",
			metadata->instruccion_inicio);
	printf("Serializado de instrucciones, puntero a instr inicio %i\n",
			metadata->instrucciones_serializado->start);
	printf("Serializado de instrucciones, puntero a instr offset %i\n",
			metadata->instrucciones_serializado->offset);
}

void imprimirStream(void* stream, int tamanio) {
	int i = 0;
	char* cadena = (char*) stream;
	for (i = 0; i < tamanio; i++) {
		printf("%c", cadena[i]);
	}
	printf("\n");
}

void enviarBytes(t_PCB pcb, char* codigo, t_medatada_program* metadata) {

	t_KER_PRO_CPU_UMV* mensajeARecibir;
//	CODIGO
	t_KER_PRO_CPU_UMV mensajeAEnviar = obtener_nueva_shared_str();
	mensajeAEnviar.PID = pcb.id;
	mensajeAEnviar.base_bytes = pcb.codSeg;
	mensajeAEnviar.tamanio_bytes = strlen(codigo);
	mensajeAEnviar.buffer = codigo;
	mensajeAEnviar.offset_bytes = 0;
//	printf("EL BUFFER es %s\n", mensajeAEnviar.buffer);
//	printf("SE ENVIA MENSAJE %i POR SOCKET %i\n", mensajeAEnviar.gen_msg.id_MSJ,plp->socket_umv);
	enviar_mensaje_a_servidor(plp->socket_umv, ENVIAR_BYTES, &mensajeAEnviar);
//	printf("SE ESPERA RESPUESTA (SE LE ENVIO EL CODIGO)\n");
	mensajeARecibir = recibir_mensaje_de_servidor(plp->socket_umv);
	if (mensajeARecibir->OK == 0) {
		log_error_message("[PLP] error al recibir respuesta de ENVIAR_BYTES codigo \n");
		return;
	}
	if (mensajeARecibir->gen_msg.id_MSJ == ENVIAR_BYTES) {
		log_info_message("[PLP] ENVIAR_BYTES codigo OK\n");
	}
//	INSTRUCCIONES
	mensajeAEnviar = obtener_nueva_shared_str();
	mensajeAEnviar.PID = pcb.id;
	mensajeAEnviar.base_bytes = pcb.indCod;
	mensajeAEnviar.tamanio_bytes = metadata->instrucciones_size * 8;
//	printf("EL TAMANIO DE LAS INTRUCCIONES X 8 es %i\n",mensajeAEnviar.tamanio_bytes);
	mensajeAEnviar.buffer = malloc(metadata->instrucciones_size * 8);
	memcpy(mensajeAEnviar.buffer, metadata->instrucciones_serializado,
			metadata->instrucciones_size * 8);
//	printf("SE COPIARON LAS INTRUCCIONES AL BUFFER\n");
	mensajeAEnviar.offset_bytes = 0;
//	printf("EL BUFFER es %s\n", mensajeAEnviar.buffer);
//	imprimirStream(mensajeAEnviar.buffer, mensajeAEnviar.tamanio_bytes);
//	printf("SE ENVIA MENSAJE %i POR SOCKET %i\n", mensajeAEnviar.gen_msg.id_MSJ,plp->socket_umv);
	enviar_mensaje_a_servidor(plp->socket_umv, ENVIAR_BYTES, &mensajeAEnviar);
	free(mensajeAEnviar.buffer);
//	printf("SE ESPERA RESPUESTA (SE LE ENVIARON INSTRUCCIONES)\n");
	mensajeARecibir = recibir_mensaje_de_servidor(plp->socket_umv);
	if (mensajeARecibir->OK == 0) {
		log_error_message("[PLP] error al recibir respuesta de ENVIAR_BYTES instrucciones \n");
		return;
	}
	if (mensajeARecibir->gen_msg.id_MSJ == ENVIAR_BYTES) {
		log_info_message("[PLP] ENVIAR_BYTES instrucciones OK\n");
	}

//	ETIQUETAS
	if(pcb.tamEtiq!=0){
		mensajeAEnviar = obtener_nueva_shared_str();
		mensajeAEnviar.PID = pcb.id;
		mensajeAEnviar.base_bytes = pcb.indEtiq;
		mensajeAEnviar.tamanio_bytes = pcb.tamEtiq;
		mensajeAEnviar.buffer = metadata->etiquetas;
		mensajeAEnviar.offset_bytes = 0;
//		printf("EL BUFFER es %s\n", mensajeAEnviar.buffer);
//		imprimirStream(mensajeAEnviar.buffer, mensajeAEnviar.tamanio_bytes);
		if (mensajeAEnviar.buffer == NULL ) {
			mensajeAEnviar.buffer = "\0";
		}
//		printf("SE ENVIA MENSAJE %i POR SOCKET %i\n",
//				mensajeAEnviar.gen_msg.id_MSJ, plp->socket_umv);
		enviar_mensaje_a_servidor(plp->socket_umv, ENVIAR_BYTES,
				&mensajeAEnviar);
//		printf("SE ESPERA RESPUESTA(SE LE ENVIARON ETIQUETAS)\n");
		mensajeARecibir = recibir_mensaje_de_servidor(plp->socket_umv);
		if (mensajeARecibir->OK == 0) {
			log_error_message("[PLP] error al recibir respuesta de ENVIAR_BYTES etiquetas \n");
			return;
		}
		if (mensajeARecibir->gen_msg.id_MSJ == ENVIAR_BYTES) {
			log_info_message("[PLP] ENVIAR_BYTES etiquetas OK\n");
		}
	}
	return;
}

t_PCB crear_segmentos(t_medatada_program* metaData, int lenCod, t_Programa* programa) {
	char log_text[500];
	t_PCB pcb; // = malloc(sizeof(t_PCB));
	t_KER_PRO_CPU_UMV mensajeAEnviar;
	t_KER_PRO_CPU_UMV* mensaje;
	int pid = programa->PID;
	pcb.id = pid;
	pcb.PC = metaData->instruccion_inicio;
	pcb.cursorStack = 0;
	pcb.tamContext = 0;
	pcb.tamEtiq = metaData->etiquetas_size;
	pcb.stack_size = plp->stack_size;

//	SEGMENTO 1: ETIQUETAS
	mensajeAEnviar = obtener_nueva_shared_str();
	mensajeAEnviar.tamanio_segmento = metaData->etiquetas_size;
	mensajeAEnviar.PID = pid;
	enviar_mensaje_a_servidor(plp->socket_umv, CREAR_SEGMENTOS,
			&mensajeAEnviar);

	mensaje = recibir_mensaje_de_servidor(plp->socket_umv);

	sprintf(log_text, "[PLP] UMV Etiquetas %s OK? %d",
			id_a_mensaje(mensaje->gen_msg.id_MSJ), mensaje->OK);
	log_info_message(log_text);


	if (mensaje->gen_msg.id_MSJ == CREAR_SEGMENTOS && mensaje->OK == 1) {
		pcb.indEtiq = mensaje->base_segmento;
	} else {

		sprintf(log_text, "[PLP] PROGRAMA %d - %s HANDSHAKE CON PLP ok? %d",
				programa->PID,programa->nombre, 0);
		log_info_message(log_text);


		mensajeAEnviar = obtener_nueva_shared_str();
		mensajeAEnviar.OK = 0;
		mensajeAEnviar.mensaje=mensaje->mensaje;
		enviar_mensaje(programa->socket_server, HANDSHAKE_PROGRAMA_PLP,
				&mensajeAEnviar);
		mensajeAEnviar = obtener_nueva_shared_str();
		mensajeAEnviar.PID = programa->PID;
		enviar_mensaje_a_servidor(plp->socket_umv, DESTRUIR_SEGMENTOS,
				&mensajeAEnviar);
		pcb.id = -1;
		return pcb;
	}
//	SEGMENTO 2: INSTRUCCIONES
	mensajeAEnviar = obtener_nueva_shared_str();
	mensajeAEnviar.tamanio_segmento = metaData->instrucciones_size * 8;
	mensajeAEnviar.PID = pid;
	enviar_mensaje_a_servidor(plp->socket_umv, CREAR_SEGMENTOS,
			&mensajeAEnviar);
	mensaje = recibir_mensaje_de_servidor(plp->socket_umv);

	sprintf(log_text, "[PLP] UMV Instrucciones %s OK? %d",
			id_a_mensaje(mensaje->gen_msg.id_MSJ), mensaje->OK);
	log_info_message(log_text);

	if (mensaje->gen_msg.id_MSJ == CREAR_SEGMENTOS && mensaje->OK == 1) {
		pcb.indCod = mensaje->base_segmento;
	} else {
		sprintf(log_text, "[PLP] PROGRAMA %d - %s HANDSHAKE CON PLP ok? %d",
				programa->PID,programa->nombre, 0);
		log_info_message(log_text);

		mensajeAEnviar = obtener_nueva_shared_str();
		mensajeAEnviar.OK = 0;
		mensajeAEnviar.mensaje =
				mensaje->mensaje;
		enviar_mensaje(programa->socket_server, HANDSHAKE_PROGRAMA_PLP,
				&mensajeAEnviar);
		mensajeAEnviar = obtener_nueva_shared_str();
		mensajeAEnviar.PID = programa->PID;
		enviar_mensaje_a_servidor(plp->socket_umv, DESTRUIR_SEGMENTOS,
				&mensajeAEnviar);
		pcb.id = -1;
		return pcb;
	}
//	SEGMENTO 3: STACK
	mensajeAEnviar = obtener_nueva_shared_str();
	mensajeAEnviar.tamanio_segmento = plp->stack_size;
	mensajeAEnviar.PID = pid;
	enviar_mensaje_a_servidor(plp->socket_umv, CREAR_SEGMENTOS,
			&mensajeAEnviar);
	mensaje  = recibir_mensaje_de_servidor(plp->socket_umv);
	sprintf(log_text, "[PLP] UMV Stack %s OK? %d",
			id_a_mensaje(mensaje->gen_msg.id_MSJ), mensaje->OK);
	log_info_message(log_text);

	if (mensaje->gen_msg.id_MSJ == CREAR_SEGMENTOS && mensaje->OK == 1) {
		pcb.stackSeg = mensaje->base_segmento;
	} else {
		sprintf(log_text, "[PLP] PROGRAMA %d - %s HANDSHAKE CON PLP ok? %d",
				programa->PID,programa->nombre, 0);
		log_info_message(log_text);

		mensajeAEnviar = obtener_nueva_shared_str();
		mensajeAEnviar.OK = 0;
		mensajeAEnviar.mensaje =
				mensaje->mensaje;
		enviar_mensaje(programa->socket_server, HANDSHAKE_PROGRAMA_PLP,
				&mensajeAEnviar);
		mensajeAEnviar = obtener_nueva_shared_str();
		mensajeAEnviar.PID = programa->PID;
		enviar_mensaje_a_servidor(plp->socket_umv, DESTRUIR_SEGMENTOS,
				&mensajeAEnviar);
		pcb.id = -1;
		return pcb;
	}
//	SEGMENTO 4: CODIGO
	mensajeAEnviar = obtener_nueva_shared_str();
	mensajeAEnviar.tamanio_segmento = lenCod;
	mensajeAEnviar.PID = pid;
	enviar_mensaje_a_servidor(plp->socket_umv, CREAR_SEGMENTOS,
			&mensajeAEnviar);
	mensaje  = recibir_mensaje_de_servidor(plp->socket_umv);
	sprintf(log_text, "[UMV] %s OK? %d",
			id_a_mensaje(mensaje->gen_msg.id_MSJ), mensaje->OK);
	log_info_message(log_text);

	if (mensaje->gen_msg.id_MSJ == CREAR_SEGMENTOS && mensaje->OK == 1) {
		pcb.codSeg = mensaje->base_segmento;
	} else {

		sprintf(log_text, "[PLP] PROGRAMA %d - %s HANDSHAKE CON PLP ok? %d",
				programa->PID,programa->nombre, 0);
		log_info_message(log_text);

		mensajeAEnviar = obtener_nueva_shared_str();
		mensajeAEnviar.OK = 0;
		mensajeAEnviar.mensaje =
				mensaje->mensaje;
		enviar_mensaje(programa->socket_server, HANDSHAKE_PROGRAMA_PLP,
				&mensajeAEnviar);
		mensajeAEnviar = obtener_nueva_shared_str();
		mensajeAEnviar.PID = programa->PID;
		enviar_mensaje_a_servidor(plp->socket_umv, DESTRUIR_SEGMENTOS,
				&mensajeAEnviar);
		pcb.id = -1;
		return pcb;
	}
	return pcb;
}

int calcularLineas(char* codigo) {
	char** lineas = string_split(codigo, "\n");
	int cant = 0;
	void cantidadLineas(char *linea) {
		cant++;
	}
	string_iterate_lines(lineas, cantidadLineas);

	return cant;
}

int calcularJob(t_medatada_program* metaData, char* codigo) {
	int cantidadLineasDeCodigo = calcularLineas(codigo);
	int peso = (5 * metaData->cantidad_de_etiquetas)
			+ (3 * metaData->cantidad_de_funciones) + cantidadLineasDeCodigo;
	return peso;
}

t_KER_PRO_CPU_UMV* handler_plp_msgs(t_KER_PRO_CPU_UMV* msg) {
	char log_text[500];

	if (msg != NULL ) {

		t_GEN_MSG gen_msg = msg->gen_msg;
		if (!gen_msg.disconnected) {
			switch (gen_msg.id_MSJ) {
			case HANDSHAKE_PROGRAMA_PLP: {

				t_Programa* programa = malloc(sizeof(t_Programa));
				programa->PID = obtener_PID_programa();

				char** nombre_con_barra = string_split(msg->mensaje,"/");
				char** nombre_con_punto;
				char* nombre;
				while (*nombre_con_barra != NULL)
				{
					nombre = *nombre_con_barra++;
					if(string_ends_with(nombre,".ansisop"))
					{
						nombre_con_punto = string_split(nombre,".");
						programa->nombre = (char*)malloc(strlen(nombre_con_punto[0]) + 3);
						strcpy(programa->nombre, nombre_con_punto[0]);
					}

				}
				free(nombre_con_punto);
				free(nombre);



				programa->cpu = 0;
				programa->r = -1;
				programa->q = 0;
				programa->ios = 0;
				programa->io = ' ';

				// Manejo de crear segmento.
//				puts(programa->nombre);
				t_KER_PRO_CPU_UMV* mensajeARecibir = msg;
				char* codigo = mensajeARecibir->codigo;
				programa->socket_server =
						mensajeARecibir->gen_msg.socket_descriptor_server;
//				printf("El codigo es: %s\n", codigo);
				t_medatada_program* metaData = metadata_desde_literal(codigo);
//				imprimirMetadata(metaData);
				t_PCB PCB = crear_segmentos(metaData, strlen(codigo), programa);
				if (PCB.id == -1) {
					break;
				}
				programa->PCB = PCB;
				enviarBytes(PCB, codigo, metaData);

				log_debug_message("[PLP] SE TERMINO DE CREAR LOS SEGMENTOS Y SE TIENE EL PCB");

				t_KER_PRO_CPU_UMV mensajeAEnviar = obtener_nueva_shared_str();
				log_debug_message("[PLP] SE ENVIA RESPUESTA HANDSHAKE A PROGRAMA");
				mensajeAEnviar.OK = 1;
				enviar_mensaje(
						mensajeARecibir->gen_msg.socket_descriptor_server,
						HANDSHAKE_PROGRAMA_PLP, &mensajeAEnviar);

				int peso = calcularJob(metaData, codigo);
				programa->peso = peso;

				sprintf(log_text, "[PROGRAMA %d - %s] HANDSHAKE CON PLP ok? %d",
						programa->PID,programa->nombre, 1);
				log_info_message(log_text);

				sprintf(
						identidad_socket_server[msg->gen_msg.socket_descriptor_server],
						"PROGRAMA %d - %s", programa->PID,programa->nombre);
//				puts(identidad_socket_server[msg->gen_msg.socket_descriptor_server]);
				list_add(con_vals.procs,programa);
				sync_nuevo_programa(programa);

				break;
			}
			default: //Mensaje no valido
			{
				sprintf(log_text,
						"[MENSAJE INVALIDO] PLP no tiene handler para id mensaje %d",
						gen_msg.id_MSJ);

				log_info_message(log_text);

				break;
			}
			}
		} else {
			sprintf(log_text,
					"[DESCONEXION EN PLP] de PROGRAMA con socket server %d (%s) del PLP",
					gen_msg.socket_descriptor_server,
					identidad_socket_server[gen_msg.socket_descriptor_server]);

			log_info_message(log_text);

			t_Programa* programa = remover_programa_por_socket_server(gen_msg.socket_descriptor_server);

			if(programa!=NULL)
			{
				sprintf(log_text,
						"[PLP] se removio PROGRAMA %d - %s",
						programa->PID, programa->nombre);

				log_info_message(log_text);

			}

		}

	} else {
		log_error_message("[PLP] Se ha recibido mensaje nulo en PLP");
	}

	return NULL ;
}

void handshakeInicialUMV(int socketUMV) {
	t_KER_PRO_CPU_UMV mensaje = obtener_nueva_shared_str();
	enviar_mensaje_a_servidor(socketUMV, HANDSHAKE_PLP_UMV, &mensaje);
	t_KER_PRO_CPU_UMV* mensaje_rpta = recibir_mensaje_de_servidor(socketUMV);

	if (mensaje_rpta != NULL
			&& mensaje_rpta->gen_msg.id_MSJ == HANDSHAKE_PLP_UMV
			&& mensaje_rpta->OK == 1) {
		log_info_message("[PLP] Se dio correctamente el Handshake entre PLP y UMV");
	} else {
		log_error_message("[PLP] Ocurrio un error al conectar con la UMV");
	}
}


void lanzar_manejo_multiprogramacion() {
	char log_text[500];
	int result;

	pthread_t thread_multiprogramacion;
	result = pthread_create(&thread_multiprogramacion, NULL, (void*) pasar_programa_new_a_ready, NULL); //Hilo Planificacion Largo Plazo
	if (result) {
		//begin log
		log_error_message(
				"[PLP] Hilo Manejo Multiprogramacion"
				);
		//end log

	} else {
		//begin log
		sprintf(log_text,

						"[PLP] Hilo Manejo Multiprogramacion con %d lanzado EXITOSAMENTE"
						,
				plp->multiprogramacion);

		log_info_message(log_text);
		//end log
	}

}
void* lanzar_plp(t_PLP *plp) {
	char log_text[500];

	lanzar_manejo_multiprogramacion();

	sprintf(log_text,

					"PLP Iniciado para puerto %d"
					,
			plp->puerto);

	log_info_message(log_text);
	int socketUMV = conectar_a_servidor(plp->ip_umv, plp->puerto_umv);
	plp->socket_umv = socketUMV;
	handshakeInicialUMV(socketUMV);
	handler_plp_msgs_ptr = &handler_plp_msgs;
	abrir_conexion_servidor_local(plp->puerto, handler_plp_msgs_ptr);
	return EXIT_SUCCESS;
}

