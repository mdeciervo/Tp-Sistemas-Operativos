/*
 * umvMock.c
 *
 *  Created on: 21/04/2014
 *      Author: utnso
 */

#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <commons/tad_server.h>
#include <commons/log_app.h>
#include <commons/shared_strs.h>
#include <parser/metadata_program.h>
#include <parser/parser.h>


#define PUERTO 5002

char* memoria;
//char* codigo = "function restoGlobal \n variables R \n R = $0 - !Global \n return R \n end \n \n function sumar2 \n *$0 = 2 + *$0 \n signal mutexB \n variables A \n A = 15 \n print A \n textPrint A dentro de funcion \n end \n begin \n variables A,j \n textPrint Inicio Programa \n A= 10 \n j = 3 \n A <- restoGlobal A \n print A \n jnz j Etiqueta \n :Terminar \n textPrint Fin Programa! \n end \n :Etiqueta \n j = j -1 \n \n \n wait mutexA \n jnz j Etiqueta \n sumar2 &A \n variables L \n L = $A + j \n print L \n jz j Terminar \n end \n "; //pruebaTodo.ansisop creado por mi
//char* codigo = "#!/usr/bin/ansisop \n function imprimir \n wait mutexA \n print $0+1 \n signal mutexB \n end \n begin \n variables f,  A,  g \n   A = 	1 \n     !Global = A-1 \n print !Global \n jnz !Global Siguiente \n f = 8 \n g <- doble !Global \n io impresora 20 \n :Siguiente \n imprimir A \n textPrint    Hola Mundo! \n sumar1 &g \n print 		g \n  sinParam \n end \n function sinParam \n textPrint Bye \n end \n  \n #Devolver el doble del \n #primer parametro \n function doble \n variables f \n f = $0 + $0 \n return f \n end \n function sumar1 \n *$0 = 1 + *$0 \n end \n "; // Completo.ansisop modificado para que no salte
//char* codigo = "#!/usr/bin/ansisop \n function imprimir \n wait mutexA \n print $0+1 \n signal mutexB \n end \n begin \n variables f,  A,  g \n   A = 	0 \n     !Global = 1+A \n print !Global \n jnz !Global Siguiente \n f = 8 \n g <- doble !Global \n io impresora 20 \n :Siguiente \n imprimir A \n textPrint    Hola Mundo! \n sumar1 &g \n print 		g \n  sinParam \n end \n function sinParam \n textPrint Bye \n end \n  \n #Devolver el doble del \n #primer parametro \n function doble \n variables f \n f = $0 + $0 \n return f \n end \n function sumar1 \n *$0 = 1 + *$0 \n end \n "; // Completo.ansisop
char* codigo = "#!/usr/bin/ansisop \n function imprimir \n wait mutexA \n print $0+1 \n signal mutexB \n end \n begin \n variables f,  A,  g \n   A = 	0 \n     !Global = 1+A \n print !Global \n jnz !Global Siguiente \n f = 8 \n g <- doble !Global \n :Siguiente \n imprimir A \n textPrint    Hola Mundo! \n sumar1 &g \n print 		g \n  sinParam \n end \n function sinParam \n textPrint Bye \n end \n  \n #Devolver el doble del \n #primer parametro \n function doble \n variables f \n f = $0 + $0 \n return f \n end \n function sumar1 \n *$0 = 1 + *$0 \n end \n "; // Completo.ansisop sin io
//char* codigo = "#!/usr/bin/ansisop \n begin \n variables f,  A,  g \n A = 	0 \n f = 8 \n g <- doble !Global \n io impresora 20 \n textPrint    Hola Mundo! \n end \n function doble \n variables f \n f = $0 + $0 \n return f \n end \n "; //Completo modificado
//char* codigo = "#!/usr/bin/ansisop \n begin \n	variables a, b \n	a = 3 \n	b = 5 \n	a = b + 12 \n end"; // facil de ansisop FUNCIONA!!
//char* codigo = "#!/usr/bin/ansisop \n begin \n	:etiqueta \n wait c \n		print !colas \n signal b \n	#Ciclar indefinidamente \n	goto etiqueta \n end \n"; // productor
//char* codigo = "#!/usr/bin/ansisop \n begin \n variables f, i, t \n #`f`: Hasta donde contar \n f=20 \n i=0 \n :inicio \n #`i`: Iterador \n i=i+1 \n #Imprimir el contador \n print i \n #`t`: Comparador entre `i` y `f` \n t=f-i \n #De no ser iguales, salta a inicio \n jnz t inicio \n end"; // For ansisop FUNCIONA!!
//char* codigo = "#!/usr/bin/ansisop \n begin \n #un vector de 5 posiciones \n variables a, p \n #Como AnSISOP no soporta parentesis, el operador * tiene prioridad maxima \n #La variable p apunta a la variable a \n	p = &a \n #Asigno a la variable a \n a = 19 \n #imprimir *p es identico a imprimir a (sin el *) \n print *p \n	#Si quiero imprimir lo que referencie p, y sumarle 30 \n	print 30+*p \n	#Esto seria como hacer: 30 + *(p) \n #Que no es lo mismo que: \n print *p+30 \n #Esto ultimo imprime 30 posiciones desde lo que referencia p \n	#Seria hacer: *(p+30) \n	#Y podria dar fallo de segmento \n #Etas reglas de prioridad solo se aplican con el operador * \n #No con el & \n p = &a+30 \n	#Esto asigna a p el valor de la posicion de a, corrido 30 lugares \n #osea: &(a) + 30 \n print p \n #Y es identico a hacer \n p = 30+&a \n #Que seria como hacer: 30 + &(a) \n print p \n #No hay forma de hacer algo como: &(a+30) \n #Pero... para que querriamos hacer eso? \n end \n"; // para dereferenciar.ansisop
//char* codigo = "#!/usr/bin/ansisop \n begin \n variables i,b \n i = 1 \n :inicio_for \n i = i + 1 \n print i \n b = i - 10 \n jnz b inicio_for \n #fuera del for \n end \n"; // Prueba de saltos en guia // FUNCIONA!!!
//char* codigo = "#!/usr/bin/ansisop \n function prueba \n variables a,b \n a = 2 \n b = 16 \n print b \n print a \n a = a + b \n end \n begin \n variables a, b \n a = 20 \n print a \n call prueba \n print a \n end \n "; // Ejemplo imprime variables de guia
char* stackSeg;

void imprimirStream(void* stream, int tamanio){
	int i=0;
	char* cadena = (char*) stream;
	for (i=0;i<tamanio;i++){
		printf("%c",cadena[i]);
	}
	printf("\n");
}

/*
 * Funcion auxiliar. Es solo para probar lo que lee
 * el analizador de metadata.
 * DESPUES BORRARLA
 */
void imprimirMetadata(t_medatada_program* metadata){
	printf("Cantidad de etiquetas %i\n",metadata->cantidad_de_etiquetas);
	printf("Cantidad de funciones %i\n",metadata->cantidad_de_funciones);
	printf("Tamaño del mapa serializado de etiquetas %i\n",metadata->etiquetas_size);
	int i=0;
	if(metadata->etiquetas_size!=0){
	for (i=0;i<(metadata->etiquetas_size+1);i++){
		printf("%c",metadata->etiquetas[i]);
	}
	}
	printf("\n");
	printf("Tamaño del mapa serializado de instrucciones %i\n",metadata->instrucciones_size);
	printf("El numero de la primera instruccion es %i\n",metadata->instruccion_inicio);
	printf("Serializado de instrucciones, puntero a instr inicio %i\n",metadata->instrucciones_serializado->start);
	printf("Serializado de instrucciones, puntero a instr offset %i\n",metadata->instrucciones_serializado->offset);
}

t_KER_PRO_CPU_UMV* manejo_conexiones(t_KER_PRO_CPU_UMV* msg);

int main(void) {
	puts("Inicio mock de UMV");
	printf("Se abre conexion en puerto %d \n", PUERTO);
	inicializar_log("UMV_Mock.log", "umvM");
	habilitar_log_debug();
	printf("Creo log \n");
	//Creo segmento de memoria ficticio
	memoria = malloc(3096);

	// Hasta aca el stack segment
	t_medatada_program* programa = metadatada_desde_literal(codigo);
	imprimirMetadata(programa);

	printf("Paso la creacion del metadata \n");

	printf("Largo del codigo %i\n",strlen(codigo));

	// Creo un stack serializado
		t_variable varA;
		varA.id = 'a';
		t_variable varB;
		varB.id = 'b';
		varB.valor = 10;
		stackSeg = malloc(STACK_SIZE);
		int size=0;
		int offset=0;
		size = sizeof(t_variable);
		memcpy(stackSeg,&varA,size);
		printf("Grabe una variable id:%c y valor:%i ocupando %i bytes\n",varA.id,varA.valor,size);
		offset = size;
		memcpy(stackSeg+offset,&varB,size);
		printf("Grabe una variable id:%c y valor:%i ocupando %i bytes\n",varB.id,varB.valor,size);


	// Agrego el codigo en memoria a partir del 0
	memcpy(memoria,codigo,strlen(codigo));
	// Agrego a memoria el serializado de etiquetas, a partir de posicion 500
	memcpy(memoria+1024,programa->etiquetas,programa->etiquetas_size);
	// Agrego a memoria el serializado de instrucciones, a partir de la posicion 800
	memcpy(memoria+1536,programa->instrucciones_serializado,programa->instrucciones_size*8);
	// Agrego a memoria el stack segment desde el 1024
	memcpy(memoria+2048,stackSeg,STACK_SIZE);

	printf("Paso la creacion de segmentos \n");

//	SE ABREN LAS CONEXIONES
	abrir_conexion_servidor(PUERTO, &manejo_conexiones);
//	ACA ARRANCA LA CONSOLA

	free(memoria);
	return EXIT_SUCCESS;
}

t_KER_PRO_CPU_UMV* manejo_conexiones(t_KER_PRO_CPU_UMV* msg){
	//	ACA SE MANEJAN LOS MENSAJES QUE PUEDEN LLEGAR
	if (msg != NULL) {
		//	SI NO ES NULO
		t_KER_PRO_CPU_UMV gen_msg = obtener_nueva_shared_str();
		memcpy(&gen_msg,msg,sizeof(t_KER_PRO_CPU_UMV));
		printf("el MENSAJE es de tipo: (%d) \n", gen_msg.gen_msg.id_MSJ);
		t_KER_PRO_CPU_UMV resp_msg = obtener_nueva_shared_str();

		switch(gen_msg.gen_msg.id_MSJ){

		case (HANDSHAKE_CPU_UMV):
			// SI ES UN HANDSHAKE
			puts("PRIMER MENSAJE RECIBIDO SE RESPONDE CON HANDSHAKE\n");
//			resp_msg->gen_msg.disconnected = 0;
			resp_msg.gen_msg.socket_descriptor = gen_msg.gen_msg.socket_descriptor;
			resp_msg.gen_msg.socket_descriptor_server = gen_msg.gen_msg.socket_descriptor_server;
			resp_msg.gen_msg.id_MSJ = HANDSHAKE_CPU_UMV;
			resp_msg.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
			resp_msg.OK = 1;
			puts("ENVIO HANDSHAKE\n");
			// Se responde con el mismo handshake del tipo
			enviar_mensaje(resp_msg.gen_msg.socket_descriptor_server,resp_msg.gen_msg.id_MSJ,&resp_msg);
			puts("mensaje enviado respuesta handshake\n");
			break;

		case (SOLICITAR_BYTES):
			// SI RECIBE SOLICITUD DE DATOS
			puts("----------------------RECIBO SOLICITUD DE ENVIO DE DATOS----------------------\n");
			resp_msg.gen_msg.socket_descriptor = gen_msg.gen_msg.socket_descriptor;
			resp_msg.gen_msg.socket_descriptor_server = gen_msg.gen_msg.socket_descriptor_server;
			resp_msg.gen_msg.id_MSJ = SOLICITAR_BYTES;
			resp_msg.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
			resp_msg.base_bytes = gen_msg.base_bytes;
			resp_msg.offset_bytes = gen_msg.offset_bytes;
			resp_msg.tamanio_bytes = gen_msg.tamanio_bytes;
			resp_msg.buffer = (char*)malloc(resp_msg.tamanio_bytes);
			resp_msg.OK = 1;
			printf("CPU solicita los datos [Base,Offset,Tamanio] : %i,%i,%i\n",resp_msg.base_bytes,resp_msg.offset_bytes,resp_msg.tamanio_bytes);
			memcpy(resp_msg.buffer,memoria+resp_msg.base_bytes+resp_msg.offset_bytes,resp_msg.tamanio_bytes);
			imprimirStream(resp_msg.buffer,resp_msg.tamanio_bytes);
//			printf("El buffer del pedido solicitado es: %s\n",resp_msg.buffer);
			enviar_mensaje(resp_msg.gen_msg.socket_descriptor_server,resp_msg.gen_msg.id_MSJ, &resp_msg);
			puts("-------------------- ENVIO DATOS SOLICITADOS ---------------------------------\n");
			break;

		case (CAMBIO_PROCESO_ACTIVO):
			// Recibo indicacion de programa activo
			printf("El id del programa activo es %i\n",gen_msg.PCB.id);
			resp_msg.gen_msg.socket_descriptor = gen_msg.gen_msg.socket_descriptor;
			resp_msg.gen_msg.socket_descriptor_server = gen_msg.gen_msg.socket_descriptor_server;
			resp_msg.gen_msg.id_MSJ = CAMBIO_PROCESO_ACTIVO;
			resp_msg.OK = 1;
			enviar_mensaje(resp_msg.gen_msg.socket_descriptor_server,resp_msg.gen_msg.id_MSJ, &resp_msg);
			break;

		case (ENVIAR_BYTES):
		// SI RECIBE SOLICITUD DE DATOS
			puts("----------------------RECIBO DATOS A SER GRABADOS----------------------\n");
			resp_msg.gen_msg.socket_descriptor = gen_msg.gen_msg.socket_descriptor;
			resp_msg.gen_msg.socket_descriptor_server = gen_msg.gen_msg.socket_descriptor_server;
			resp_msg.gen_msg.id_MSJ = ENVIAR_BYTES;
			resp_msg.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
			resp_msg.base_bytes = gen_msg.base_bytes;
			resp_msg.offset_bytes = gen_msg.offset_bytes;
			resp_msg.tamanio_bytes = gen_msg.tamanio_bytes;
			resp_msg.OK = 1;
			printf("CPU graba los datos [Base,Offset,Tamanio] : %i,%i,%i\n",resp_msg.base_bytes,resp_msg.offset_bytes,resp_msg.tamanio_bytes);
			memcpy(memoria+resp_msg.base_bytes+resp_msg.offset_bytes,resp_msg.buffer,resp_msg.tamanio_bytes);
			imprimirStream(resp_msg.buffer,resp_msg.tamanio_bytes);
			enviar_mensaje(resp_msg.gen_msg.socket_descriptor_server,resp_msg.gen_msg.id_MSJ, &resp_msg);
			puts("-------------------- ENVIO CONFIRMACION DE GRABAR DATOS ---------------------------------\n");
			break;

		case (DESTRUIR_SEGMENTOS):
			puts("----------------------RECIBO AVISO DE DESTRUIR SEGMENTOS----------------------\n");
			resp_msg.gen_msg.socket_descriptor = gen_msg.gen_msg.socket_descriptor;
			resp_msg.gen_msg.socket_descriptor_server = gen_msg.gen_msg.socket_descriptor_server;
			resp_msg.gen_msg.id_MSJ = DESTRUIR_SEGMENTOS;
			resp_msg.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
			resp_msg.OK = 1;
			enviar_mensaje(resp_msg.gen_msg.socket_descriptor_server,resp_msg.gen_msg.id_MSJ, &resp_msg);
			puts("------------- Segmentos destruidos----------------\n");
			break;

		default:
			printf("Recibi un mje default del tipo %i\n",gen_msg.gen_msg.id_MSJ);
			puts("NO ENTIENDO EL MENSAJE SOLICITADO\n");
			break;

		}
//		free(resp_msg);
	}
	puts("Fin primera recepcion");
	return NULL;
}
