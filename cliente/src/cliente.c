/*
 ============================================================================
 Name        : cliente.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/tad_cliente.h>
#include <commons/shared_strs.h>
#include <commons/log_app.h>

int main(void) {

	inicializar_log("cliente.log", "cliente");
	t_KER_PRO_CPU_UMV str = obtener_nueva_shared_str();
	int s = conectar_a_servidor("127.0.0.1",7000);
	enviar_mensaje_a_servidor(s,1,&str);
	t_KER_PRO_CPU_UMV* str_rec = (t_KER_PRO_CPU_UMV*) malloc(sizeof(t_KER_PRO_CPU_UMV));
	str_rec = recibir_mensaje_de_servidor(s);
	puts(str_rec->buffer);
	str_rec = recibir_mensaje_de_servidor(s);
	puts(str_rec->buffer);
	return EXIT_SUCCESS;
}
