#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "log_app.h"
#include "tad_cliente.h"
#include "shared_strs.h"

t_KER_PRO_CPU_UMV* recibir_mensaje_de_servidor(int sock_descriptor) {
	//begin log
	char log_text[500];
	sprintf(log_text, "Esperando mensaje del servidor por socket %d",sock_descriptor);
	log_debug_message(log_text);
	//end log
	t_KER_PRO_CPU_UMV *response_message = (t_KER_PRO_CPU_UMV *)malloc(sizeof(t_KER_PRO_CPU_UMV));
	int nbytes_rec = deserializar_buffer_a_shared_str(response_message, sock_descriptor);

	if (nbytes_rec < 0) {
		//beging log
		sprintf(log_text, "Error al recibir mensaje del servidor por socket %d",
				sock_descriptor);
		log_error_message(log_text);
		//end log
		return NULL ;
	} else if (nbytes_rec == 0) {
		//beging log
		sprintf(log_text,
				"Error al recibir mensaje del servidor por socket %d llego con 0 bytes",
				sock_descriptor);
		log_error_message(log_text);
		return NULL ;
		//end log
	}

	t_GEN_MSG temp = response_message->gen_msg;

	//beging log
	sprintf(log_text,
			"Mensaje recibido del servidor por socket %d con id %d:%s y tamanio buffer %d bytes",
			sock_descriptor, temp.id_MSJ, id_a_mensaje(temp.id_MSJ),temp.size_str);
	log_trace_message(log_text);
	//end log

	return response_message;
}

int enviar_mensaje_a_servidor(int socket_descriptor, int id_MSG, t_KER_PRO_CPU_UMV *message) {

	message->gen_msg.disconnected = 0;
	message->gen_msg.id_MSJ = id_MSG;
	message->gen_msg.socket_descriptor = socket_descriptor;

	//begin log
	char log_text[500];
	sprintf(log_text, "Enviando mensaje a servidor por socket %d con id %d:%s",
			socket_descriptor, id_MSG,id_a_mensaje(id_MSG));
	log_debug_message(log_text);
	//end log
	calcular_tamanio_shared_str(message);
	char* buffer = serializar_shared_str(message);
	int i = send(socket_descriptor, buffer, message->gen_msg.size_str, 0);
	free(buffer);
	if (i >= 0) {
		//begin log
		sprintf(log_text, "Mensaje enviado a servidor por socket %d con id %d:%s",
				socket_descriptor, id_MSG,id_a_mensaje(id_MSG));
		log_trace_message(log_text);
		//end log
	} else {
		//begin log
		sprintf(log_text,
				"Error al enviar datos a servidor por socket %d con id %d:%s",
				socket_descriptor, id_MSG,id_a_mensaje(id_MSG));
		log_error_message(log_text);
		//end log
	}

	return i;
}
int conectar_a_servidor(char* unaIP, int unPuerto) {
	//begin log
	char log_text[500];
	sprintf(log_text, "Conectando con ip %s puerto %d", unaIP, unPuerto);
	log_debug_message(log_text);
	//end log

	int unSocket;

	INTERNET_SOCKET socketInfo;

	if ((unSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		//begin log
		sprintf(log_text, "Error al crear socket con ip %s puerto %d", unaIP,
				unPuerto);
		log_error_message(log_text);
		//end log
		return unSocket;
	}

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = inet_addr(unaIP);
	socketInfo.sin_port = htons(unPuerto);

	// Conectar el socket con la direccion 'socketInfo'.
	if (connect(unSocket, (struct sockaddr*) &socketInfo, sizeof(socketInfo))
			!= 0) {
		//begin log
		sprintf(log_text, "Error al conectar socket con ip %s puerto %d", unaIP,
				unPuerto);
		log_error_message(log_text);
		//end log
		return -1;
	}

	//begin log
	sprintf(log_text, "Conectado con ip %s puerto %d y socket descriptor %d",
			unaIP, unPuerto, unSocket);
	log_trace_message(log_text);
	//end log

	return unSocket;
}
void desconectar_de_servidor(int sock_descriptor) {
	//begin log
	char log_text[500];
	sprintf(log_text, "Cerrando socket %d", sock_descriptor);
	log_debug_message(log_text);
	//end log

	close(sock_descriptor);

	//beging log
	sprintf(log_text, "Socket cerrado: %d", sock_descriptor);
	log_trace_message(log_text);
	//end log

}
