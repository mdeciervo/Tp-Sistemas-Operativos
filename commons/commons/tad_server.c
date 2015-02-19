#include <stdlib.h>
#include <string.h>
#include "tad_server.h"
#include "log_app.h"

fd_set master_glb; // conjunto maestro de descriptores de fichero
fd_set read_fds_glb; // conjunto temporal de descriptores de fichero para select()

int crear_socket_servidor(int puerto) {
	//begin log
	char log_text[500];
	sprintf(log_text, "Abriendo conexion servidor en puerto %d", puerto);
	log_debug_message(log_text);
	//end log

	int listener = 0;
	struct sockaddr_in myaddr; // direccion del servidor
	int yes = 1; // para setsockopt() SO_REUSEADDR, mas abajo

	// obtener socket a la escucha
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		//begin log
		sprintf(log_text, "Error al crear socket servidor en puerto %d",
				puerto);
		log_error_message(log_text);
		//end log
		exit(1);
	}

	// obviar el mensaje "address already in use" (la direccion ya se esta usando)
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
			== -1) {
		log_error_message("setsockopt");
		exit(1); // ERROR se cierra
	}

	// enlazar
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(puerto);
	memset(&(myaddr.sin_zero), '\0', 8);

	if (bind(listener, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
		//begin log
		sprintf(log_text,
				"Error al enlanzar-bind socket servidor con puerto %d", puerto);
		log_error_message(log_text);
		//end log

		exit(1);
	}

	// escuchar
	if (listen(listener, 10) == -1) {
		//begin log
		sprintf(log_text,
				"Error al escuchar para socket servidor con puerto %d", puerto);
		log_error_message(log_text);
		//end log

		exit(1);
	}

	//begin log
	sprintf(log_text, "Socket enlazado y escuchando en puerto %d", puerto);
	log_trace_message(log_text);
	//end log

	return listener;

}
void eliminar_una_conexion(fd_set *master, int fd) {
	FD_CLR(fd, master); // eliminar del conjunto maestro
	char log_text[500];
	//begin log
	sprintf(log_text,
			"conexion de socket descriptor server %d:%s removida de conjunto de descriptor",
			fd,identidad_socket_server[fd]);
	log_debug_message(log_text);
	//end log

}

void delegar_conexion(int descriptor) {
	eliminar_una_conexion(&master_glb, descriptor);
}

int aceptar_una_nueva_conexion(fd_set *master, int current_fdmax, int newfd) {
	char log_text[500];
	//begin log
	sprintf(log_text,
			"conexion de socket descriptor server %d agregada a conjunto de descriptor",
			newfd);
	log_debug_message(log_text);
	//end log

	FD_SET(newfd, master);
	if (newfd > current_fdmax) {
		current_fdmax = newfd;
	}
	return current_fdmax;
}

void administrador_de_conexiones(int listener, t_KER_PRO_CPU_UMV* (*handler_msgs_ptr)(t_KER_PRO_CPU_UMV*)) {
	espera_activa = 1;
	char log_text[500];

	t_KER_PRO_CPU_UMV* mensaje_respuesta;

	socklen_t addrlen;
	INTERNET_SOCKET remoteaddr; // direccin del cliente
	int fdmax; // nmero mximo de descriptores de fichero
	int newfd; // descriptor de socket de nueva conexin aceptada
	int nbytes;
	int i;

	FD_ZERO(&master_glb);
	FD_ZERO(&read_fds_glb);

	FD_SET(listener, &master_glb);

	fdmax = listener;

	while (espera_activa) {
		read_fds_glb = master_glb;
		//begin log
		sprintf(log_text,
				"Esperando nueva conexion y/o mensaje");
		log_debug_message(log_text);
		//end log

		if (select(fdmax + 1, &read_fds_glb, NULL, NULL, NULL ) == -1) {
			perror("select");
			exit(1);
		} // ERROR se cierra

		// explorar conexiones existentes en busca de datos que leer
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds_glb)) {
				if (i == listener) {
					// gestionar nuevas conexiones
					addrlen = sizeof(remoteaddr);
					if ((newfd = accept(listener,
							(struct sockaddr *) &remoteaddr, &addrlen)) != -1) { //BLOQUEANTE: CUANDO SALE DIO ERROR O CONEXION NUEVA
						//begin log
						sprintf(log_text,
								"Nueva conexion aceptada de socket descriptor server %d con IP %s",
								newfd, inet_ntoa(remoteaddr.sin_addr));
						log_trace_message(log_text);
						//end log
						fdmax = aceptar_una_nueva_conexion(&master_glb, fdmax,
								newfd);

					} else {
						//begin log
						sprintf(log_text,
								"Error al aceptar nueva conexion entrante");
						log_error_message(log_text);
						//end log

					}
				} else {
					t_KER_PRO_CPU_UMV package;
					nbytes = deserializar_buffer_a_shared_str(&package, i);

					if (nbytes > 0) {
						package.gen_msg.disconnected = 0;
						package.gen_msg.socket_descriptor_server = i;
						socklen_t addrlen_aux;
						INTERNET_SOCKET remoteaddr_aux; // direccin del cliente
						addrlen_aux = sizeof(remoteaddr_aux);
						getsockname(i, (INTERNET_SOCKET *) &remoteaddr_aux,
								&addrlen_aux);

						//begin log
						sprintf(log_text,
								"Mensaje recibido con socket descriptor server %d desde IP %s, tamanio buffer %d bytes y id %d:%s",
								package.gen_msg.socket_descriptor_server,
								inet_ntoa(remoteaddr_aux.sin_addr),
								nbytes, package.gen_msg.id_MSJ, id_a_mensaje(package.gen_msg.id_MSJ));
						log_trace_message(log_text);
						//end log


						mensaje_respuesta = (*handler_msgs_ptr)(
								&package);

						if (mensaje_respuesta != NULL ) {
							t_GEN_MSG gen_msg_rpt =
									mensaje_respuesta->gen_msg;
							gen_msg_rpt.disconnected = 0;
							//begin log
							sprintf(log_text,
									"Servidor intenta enviar mensaje para socket descriptor server %d:%s con id %d:%s",
									gen_msg_rpt.socket_descriptor_server,
									identidad_socket_server[gen_msg_rpt.socket_descriptor_server],
									gen_msg_rpt.id_MSJ, id_a_mensaje(gen_msg_rpt.id_MSJ));
							log_debug_message(log_text);
							//end log
							int nbytes_send = 0;

							calcular_tamanio_shared_str(mensaje_respuesta);
							char* buffer = serializar_shared_str(mensaje_respuesta);
							nbytes_send = send(i, buffer, mensaje_respuesta->gen_msg.size_str, 0);

							if (nbytes_send > 0) {
								//begin log
								sprintf(log_text,
										"Servidor ha enviado mensaje con exito para socket descriptor server %d:%s con id %d:%s",
										gen_msg_rpt.socket_descriptor_server,
										identidad_socket_server[gen_msg_rpt.socket_descriptor_server],
										gen_msg_rpt.id_MSJ, id_a_mensaje(gen_msg_rpt.id_MSJ));
								log_trace_message(log_text);
								//end log
							} else {
								//begin log
								sprintf(log_text,
										"Ha ocurrido un error al enviar datos para socket descriptor server %d:%s con id %d:%s",
										gen_msg_rpt.socket_descriptor_server,
										identidad_socket_server[gen_msg_rpt.socket_descriptor_server],
										gen_msg_rpt.id_MSJ, id_a_mensaje(gen_msg_rpt.id_MSJ));
								log_error_message(log_text);
								//end log
							}

							free(mensaje_respuesta);
						}

					} else {
						if (nbytes == 0) {
							//begin log
							sprintf(log_text,
									"Cliente con socket descriptor server %d se ha desconectado",
									i);
							log_trace_message(log_text);
							//end log
							t_KER_PRO_CPU_UMV peek;
							peek.gen_msg.disconnected = 1;
							peek.gen_msg.socket_descriptor_server = i;
							(*handler_msgs_ptr)(&peek);
						} else {
							//begin log
							sprintf(log_text,
									"Ha ocurrido un error al recibir datos para socket descriptor server %d",
									i);
							log_error_message(log_text);
							//end log

						}

						desconectar(i);
						eliminar_una_conexion(&master_glb, i); // eliminar del conjunto maestro
					}
				}
			}
		}
	//no fue procesado por este
	}
}

void administrador_de_conexiones_locales(int listener, t_KER_PRO_CPU_UMV* (*handler_msgs_ptr)(t_KER_PRO_CPU_UMV*)) {
	espera_activa = 1;
	char log_text[500];

	t_KER_PRO_CPU_UMV* mensaje_respuesta;

	socklen_t addrlen;
	INTERNET_SOCKET remoteaddr; // direccin del cliente
	int fdmax; // nmero mximo de descriptores de fichero
	int newfd; // descriptor de socket de nueva conexin aceptada
	int nbytes;
	int i;

	fd_set master; // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	FD_SET(listener, &master);

	fdmax = listener;

	while (espera_activa) {
		read_fds = master;
		//begin log
		sprintf(log_text,
				"Esperando nueva conexion y/o mensaje");
		log_debug_message(log_text);
		//end log

		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL ) == -1) {
			perror("select");
			exit(1);
		} // ERROR se cierra

		// explorar conexiones existentes en busca de datos que leer
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == listener) {
					// gestionar nuevas conexiones
					addrlen = sizeof(remoteaddr);
					if ((newfd = accept(listener,
							(struct sockaddr *) &remoteaddr, &addrlen)) != -1) { //BLOQUEANTE: CUANDO SALE DIO ERROR O CONEXION NUEVA
						//begin log
						sprintf(log_text,
								"Nueva conexion aceptada de socket descriptor server %d con IP %s",
								newfd, inet_ntoa(remoteaddr.sin_addr));
						log_trace_message(log_text);
						//end log
						fdmax = aceptar_una_nueva_conexion(&master, fdmax,
								newfd);

					} else {
						//begin log
						sprintf(log_text,
								"Error al aceptar nueva conexion entrante");
						log_error_message(log_text);
						//end log

					}
				} else {
					t_KER_PRO_CPU_UMV package;
					nbytes = deserializar_buffer_a_shared_str(&package, i);

					if (nbytes > 0) {
						package.gen_msg.disconnected = 0;
						package.gen_msg.socket_descriptor_server = i;
						socklen_t addrlen_aux;
						INTERNET_SOCKET remoteaddr_aux; // direccin del cliente
						addrlen_aux = sizeof(remoteaddr_aux);
						getsockname(i, (INTERNET_SOCKET *) &remoteaddr_aux,
								&addrlen_aux);

						//begin log
						sprintf(log_text,
								"Mensaje recibido con socket descriptor server %d desde IP %s, tamanio buffer %d bytes y id %d:%s",
								package.gen_msg.socket_descriptor_server,
								inet_ntoa(remoteaddr_aux.sin_addr),
								nbytes, package.gen_msg.id_MSJ, id_a_mensaje(package.gen_msg.id_MSJ));
						log_trace_message(log_text);
						//end log


						mensaje_respuesta = (*handler_msgs_ptr)(
								&package);

						if (mensaje_respuesta != NULL ) {
							t_GEN_MSG gen_msg_rpt =
									mensaje_respuesta->gen_msg;
							gen_msg_rpt.disconnected = 0;
							//begin log
							sprintf(log_text,
									"Servidor intenta enviar mensaje para socket descriptor server %d:%s con id %d:%s",
									gen_msg_rpt.socket_descriptor_server,
									identidad_socket_server[gen_msg_rpt.socket_descriptor_server],
									gen_msg_rpt.id_MSJ, id_a_mensaje(gen_msg_rpt.id_MSJ));
							log_debug_message(log_text);
							//end log
							int nbytes_send = 0;

							calcular_tamanio_shared_str(mensaje_respuesta);
							char* buffer = serializar_shared_str(mensaje_respuesta);
							nbytes_send = send(i, buffer, mensaje_respuesta->gen_msg.size_str, 0);

							if (nbytes_send > 0) {
								//begin log
								sprintf(log_text,
										"Servidor ha enviado mensaje con exito para socket descriptor server %d:%s con id %d:%s",
										gen_msg_rpt.socket_descriptor_server,
										identidad_socket_server[gen_msg_rpt.socket_descriptor_server],
										gen_msg_rpt.id_MSJ, id_a_mensaje(gen_msg_rpt.id_MSJ));
								log_trace_message(log_text);
								//end log
							} else {
								//begin log
								sprintf(log_text,
										"Ha ocurrido un error al enviar datos para socket descriptor server %d:%s con id %d:%s",
										gen_msg_rpt.socket_descriptor_server,
										identidad_socket_server[gen_msg_rpt.socket_descriptor_server],
										gen_msg_rpt.id_MSJ, id_a_mensaje(gen_msg_rpt.id_MSJ));
								log_error_message(log_text);
								//end log
							}

							free(mensaje_respuesta);
						}

					} else {
						if (nbytes == 0) {
							//begin log
							sprintf(log_text,
									"Cliente con socket descriptor server %d se ha desconectado",
									i);
							log_trace_message(log_text);
							//end log
							t_KER_PRO_CPU_UMV peek;
							peek.gen_msg.disconnected = 1;
							peek.gen_msg.socket_descriptor_server = i;
							(*handler_msgs_ptr)(&peek);
						} else {
							//begin log
							sprintf(log_text,
									"Ha ocurrido un error al recibir datos para socket descriptor server %d",
									i);
							log_error_message(log_text);
							//end log

						}

						desconectar(i);
						eliminar_una_conexion(&master, i); // eliminar del conjunto maestro
					}
				}
			}
		}
	//no fue procesado por este
	}
}


int abrir_conexion_servidor(int puerto, t_KER_PRO_CPU_UMV* (*handler_msgs_ptr)(t_KER_PRO_CPU_UMV*)) {
	char log_text[500];
	//begin log
	sprintf(log_text,
			"Empezando a administrar conexiones servidor en puerto %d", puerto);
	log_debug_message(log_text);
	//end log

	int listener = crear_socket_servidor(puerto);
	administrador_de_conexiones(listener, handler_msgs_ptr);
	return 0;
}

int abrir_conexion_servidor_local(int puerto, t_KER_PRO_CPU_UMV* (*handler_msgs_ptr)(t_KER_PRO_CPU_UMV*)) {
	char log_text[500];
	//begin log
	sprintf(log_text,
			"Empezando a administrar conexiones servidor en puerto %d", puerto);
	log_debug_message(log_text);
	//end log

	int listener = crear_socket_servidor(puerto);
	administrador_de_conexiones_locales(listener, handler_msgs_ptr);
	return 0;
}

void desconectar(int sock_or_port) {
	//begin log
	char log_text[500];
	sprintf(log_text, "Cerrando Socket o Puerto %d", sock_or_port);
	log_debug_message(log_text);
	//end log

	close(sock_or_port);

	//beging log
	sprintf(log_text, "Socket o Puerto cerrado : %d", sock_or_port);
	log_trace_message(log_text);
	//end log

}

void deshabilitar_espera_activa() {
	espera_activa = 0;
}

int enviar_mensaje(int socket_descriptor_server, int id_MSG, t_KER_PRO_CPU_UMV *message) {

	message->gen_msg.disconnected = 0;
	message->gen_msg.id_MSJ = id_MSG;
	message->gen_msg.socket_descriptor = socket_descriptor_server;
	//begin log
	char log_text[500];
	sprintf(log_text, "Enviando mensaje a cliente por socket descriptor server %d:%s con id %d:%s",
			socket_descriptor_server, identidad_socket_server[socket_descriptor_server],id_MSG, id_a_mensaje(id_MSG));

	log_debug_message(log_text);
	//end log

	calcular_tamanio_shared_str(message);
	char* buffer = serializar_shared_str(message);

	int i = send(socket_descriptor_server, buffer , message->gen_msg.size_str, 0);
	free(buffer);
	if (i >= 0) {
		//begin log
		sprintf(log_text, "Mensaje enviado a cliente por socket descriptor server %d:%s con id %d:%s",
				socket_descriptor_server, identidad_socket_server[socket_descriptor_server], id_MSG, id_a_mensaje(id_MSG));
		log_trace_message(log_text);
		//end log
	} else {
		//begin log
		sprintf(log_text,
				"Error al enviar datos a servidor por socket descriptor server %d:%s con id %d:%s",
				socket_descriptor_server, identidad_socket_server[socket_descriptor_server], id_MSG,id_a_mensaje(id_MSG));
		log_error_message(log_text);
		//end log
	}

	return i;
}

t_KER_PRO_CPU_UMV* recibir_mensaje(int sock_descriptor_server) {
	//begin log
	char log_text[500];
	sprintf(log_text, "Esperando mensaje del cliente por socket descriptor server %d:%s",
			sock_descriptor_server, identidad_socket_server[sock_descriptor_server]);
	log_debug_message(log_text);
	//end log

	t_KER_PRO_CPU_UMV *response_message =(t_KER_PRO_CPU_UMV *)malloc(sizeof(t_KER_PRO_CPU_UMV));
	int nbytes_rec = deserializar_buffer_a_shared_str(response_message, sock_descriptor_server);

	if (nbytes_rec < 0) {
		//beging log
		sprintf(log_text, "Error al recibir mensaje del cliente por socket descriptor server %d:%s",
				sock_descriptor_server, identidad_socket_server[sock_descriptor_server]);
		log_error_message(log_text);
		//end log
		return NULL ;
	} else if (nbytes_rec == 0) {
		//beging log
		sprintf(log_text,
				"Error al recibir mensaje del cliente por socket descriptor server %d:%s llego con 0 bytes",
				sock_descriptor_server, identidad_socket_server[sock_descriptor_server]);
		log_error_message(log_text);
		return NULL ;
		//end log
	}

	t_GEN_MSG temp = response_message->gen_msg;

	//beging log
	sprintf(log_text, "Mensaje recibido del cliente por socket descriptor server %d:%s con id %d:%s y tamanio buffer %d bytes",
			sock_descriptor_server, identidad_socket_server[sock_descriptor_server], temp.id_MSJ,id_a_mensaje(temp.id_MSJ),temp.size_str);
	log_trace_message(log_text);
	//end log

	return response_message;
}

