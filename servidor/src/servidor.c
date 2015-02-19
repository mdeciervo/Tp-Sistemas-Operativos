/*
 ============================================================================
 Name        : servidor.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/tad_server.h>
#include <commons/shared_strs.h>
#include <commons/log_app.h>
int main(void) {

	inicializar_log("servidor.log", "servidor");
	t_KER_PRO_CPU_UMV* h(t_KER_PRO_CPU_UMV* m) {
		if (!m->gen_msg.disconnected) {
			printf("mensage %i\n", m->gen_msg.id_MSJ);
			m->buffer = (char*) malloc(sizeof(char) * 50);
			strcpy(m->buffer, "testenviar_mensagebuffer\0");
			enviar_mensaje(m->gen_msg.socket_descriptor_server, 2, m);
		} else {
			puts("se desconecto el clientito");
		}
		return NULL ;
	}
	abrir_conexion_servidor(7000, (void*) h);
	return EXIT_SUCCESS;
}
