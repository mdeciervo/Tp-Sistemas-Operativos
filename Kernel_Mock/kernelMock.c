/*
 * kernelMock.c
 *
 *  Created on: 21/04/2014
 *      Author: utnso
 */

#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <commons/tad_server.h>
#include <commons/tad_cliente.h>
#include <commons/log_app.h>
#include <commons/shared_strs.h>
#include <parser/metadata_program.h>

#define PUERTO 5001
#define BUFFER_TECLADO 250

t_KER_PRO_CPU_UMV* manejo_conexiones(t_KER_PRO_CPU_UMV* msg);
int conectarAUMV(void);
void enviarBytes(t_PCB* pcb,char* codigo,t_medatada_program* metadata);
void imprimirMetadata(t_medatada_program* metadata);
t_PCB* crear_segmentos(t_medatada_program* metaData,int lenCod);
void imprimirStream(void* stream, int tamanio);
int socket_cliente;

int varGlobal;

int main(void) {
	puts("----------- INICIO DE NUEVA EJECUCION ---------------------");

	puts("Inicio mock de Kernel");
	printf("Se abre conexion en puerto %d \n", PUERTO);
	inicializar_log("Kernel_Mock.log", "kernelM");
	habilitar_log_debug();
	printf("Creo log \n");
	varGlobal = 5; // Asigno un valor cualquiera a la variable global

//	SE ABREN LAS CONEXIONES
	abrir_conexion_servidor(PUERTO, &manejo_conexiones);
//	ACA ARRANCA LA CONSOLA
	return EXIT_SUCCESS;
}

t_KER_PRO_CPU_UMV* manejo_conexiones(t_KER_PRO_CPU_UMV* msg){
	//	ACA SE MANEJAN LOS MENSAJES QUE PUEDEN LLEGAR
	if (msg != NULL) {
		//	SI NO ES NULO
		t_KER_PRO_CPU_UMV mensaje = obtener_nueva_shared_str();
		memcpy(&mensaje,msg,sizeof(t_KER_PRO_CPU_UMV));
//		t_KER_PRO_CPU_UMV* mensaje = (t_KER_PRO_CPU_UMV *) msg;
		printf("el MENSAJE es de tipo: (%d) \n", mensaje.gen_msg.id_MSJ);
		t_KER_PRO_CPU_UMV resp_msg = obtener_nueva_shared_str();

		switch(mensaje.gen_msg.id_MSJ){

		case (HANDSHAKE_PROGRAMA_PLP):
			printf("LLEGO UN HANDSHAKE PROGRAMA\n");
			char* ip="127.0.0.1";
			printf("la ip es: %s\n",ip);
			int puerto=5002;
			socket_cliente = conectar_a_servidor(ip, puerto);
			if (socket_cliente == -1) {
				return NULL;
			}
			conectarAUMV();

			t_KER_PRO_CPU_UMV* mensajeARecibir=msg;
			char* codigo = mensajeARecibir->codigo;
			printf("El codigo es: %s\n",codigo);
			t_medatada_program* metaData = metadatada_desde_literal(codigo);
			imprimirMetadata(metaData);
			t_PCB *PCB = crear_segmentos(metaData,strlen(codigo));
			enviarBytes(PCB,codigo,metaData);
			printf("SE TERMINO DE CREAR LOS SEGMENTOS Y SE TIENE EL PCB\n");
			t_KER_PRO_CPU_UMV mensajeAEnviar=obtener_nueva_shared_str();
			printf("SE ENVIA RESPUESTA HANDSHAKE A PROGRAMA\n");
			mensajeAEnviar.OK=1;
			enviar_mensaje(mensajeARecibir->gen_msg.socket_descriptor_server,HANDSHAKE_PROGRAMA_PLP,&mensajeAEnviar);
			break;

		case (HANDSHAKE_CPU_PCP):
			// SI ES UN HANDSHAKE
			puts("PRIMER MENSAJE RECIBIDO SE RESPONDE CON HANDSHAKE\n");
//			resp_msg->gen_msg.disconnected = 0;
			printf("gen_msg descriptor recibido es %i y descriptor server es %i\n",mensaje.gen_msg.socket_descriptor,mensaje.gen_msg.socket_descriptor_server);
			resp_msg.gen_msg.socket_descriptor = mensaje.gen_msg.socket_descriptor;
			resp_msg.gen_msg.socket_descriptor_server = mensaje.gen_msg.socket_descriptor_server;
			resp_msg.gen_msg.id_MSJ = HANDSHAKE_CPU_PCP;
			resp_msg.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
			resp_msg.OK = 1;
			puts("ENVIO HANDSHAKE\n");
			enviar_mensaje(resp_msg.gen_msg.socket_descriptor_server,resp_msg.gen_msg.id_MSJ, &resp_msg);
			printf("mensaje enviado respuesta handshake al socket %i\n",resp_msg.gen_msg.socket_descriptor_server);

			sleep(3);

			// SI EL CPU ESTA LISTO LE ENVIO PCB
			puts("ENVIO PCB PARA TRABAJAR\n");
//			resp_msg->gen_msg.disconnected = 0;
			resp_msg.gen_msg.socket_descriptor = mensaje.gen_msg.socket_descriptor;
			resp_msg.gen_msg.socket_descriptor_server = mensaje.gen_msg.socket_descriptor_server;
			resp_msg.gen_msg.id_MSJ = EJECUTAR_PROGRAMA;
			resp_msg.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
			resp_msg.OK = 1;
			resp_msg.quantum = 200;
			resp_msg.retardo = 1;
			//---------PRUEBO CREAR UN PCB
			t_PCB pcb;
			pcb.id = 2;
			pcb.codSeg = 0;
			pcb.cursorStack = 0;
			pcb.PC = 4; // para completo.ansisop
//			pcb.PC = 11; // para prueboTodo.ansisop
//			pcb.PC = 0; // para facil.ansisop y consumidor y for y prueba de saltos y completo simplificado
//			pcb.PC = 7; // para prueba de imprimir variables
			pcb.indCod = 1536;
			pcb.indEtiq = 1024;
			pcb.stackSeg = 2048;
			pcb.tamContext = 2;
//			pcb.tamContext = 0; // Pruebo arrancando los programas desde 0
//			pcb.tamEtiq = 15; // para ejemplo de saltos en guia
//			pcb.tamEtiq = 73; // para completo.ansisop
			pcb.tamEtiq = 61; // para completo.ansisop sin etiqueta proximo
//			pcb.tamEtiq = 53; // para prueboTodo.ansisop
//			pcb.tamEtiq = 0; // para facil.ansisop y dereferenciar
//			pcb.tamEtiq = 1; // para consumidor.ansisop
//			pcb.tamEtiq = 13; // para productor.ansisop y para probar imprimir variables
// 			pcb.tamEtiq = 11; // para for.ansisop
//			pcb.tamEtiq = 10; // completo simplificado
			resp_msg.PCB = pcb;
			//--- FIN DE PRUEBA DE CREAR PCB
			// Se responde con el mismo handshake del tipo
			enviar_mensaje(resp_msg.gen_msg.socket_descriptor_server,resp_msg.gen_msg.id_MSJ, &resp_msg);
			puts("Envio un PCB a ser ejecutado por el CPU\n");
			break;

		case (IMPRIMIR_TEXTO):
			printf("Se recibe texto a imprimir en programa desde la primitiva\n");
			printf("Texto: %s\n",mensaje.texto);
			break;

		case (IMPRIMIR):
			printf("Se recibe valor a imprimir en programa desde la primitiva\n");
			printf("Valor: %i\n",mensaje.valor);
			break;

		case (FINALIZACION_EJECUCION_PROGRAMA):
			printf("Recibi finalizacion de programa\n");
			printf("ESTADO FINAL DE LAS VARIABLES\n");
			printf("%s\n",mensaje.texto);
			break;

		case (DESCONEXION_CPU):
			printf("Recibi desconexion de cpu\n");
			break;

		case (EJECUTAR_PROGRAMA):
			printf("Recibi aviso de ejecutar programa con pcb dentro\n");
			break;

		case (ENTRADA_SALIDA):
			printf("Recibo pedido de entrada salida\n");
			resp_msg.gen_msg.socket_descriptor = mensaje.gen_msg.socket_descriptor;
			resp_msg.gen_msg.socket_descriptor_server = mensaje.gen_msg.socket_descriptor_server;
			resp_msg.gen_msg.id_MSJ = ENTRADA_SALIDA;
			resp_msg.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
			resp_msg.OK = 1;
			printf("Recibo IO de dispositivo %s y tiempo %i \n",mensaje.id_dispositivo,mensaje.utilizacion);
			enviar_mensaje(resp_msg.gen_msg.socket_descriptor_server,resp_msg.gen_msg.id_MSJ, &resp_msg);
			break;

		case (OBTENER_VALOR):
			printf("Recibo pedido de enviar el valor de la variable %s es %i\n",mensaje.id_var_comp,mensaje.valor);
			resp_msg.gen_msg.socket_descriptor = mensaje.gen_msg.socket_descriptor;
			resp_msg.gen_msg.socket_descriptor_server = mensaje.gen_msg.socket_descriptor_server;
			resp_msg.gen_msg.id_MSJ = OBTENER_VALOR;
			resp_msg.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
			resp_msg.OK = 1;
			resp_msg.valor = varGlobal;
			printf("Envio el valor de la varibale global %i\n",resp_msg.valor);
			enviar_mensaje(resp_msg.gen_msg.socket_descriptor_server,resp_msg.gen_msg.id_MSJ, &resp_msg);
			break;

		case (GRABAR_VALOR):
			printf("Recibo orden de grabar valor en variable compartida\n");
			printf("El valor de la variable %s es %i\n",mensaje.id_var_comp,mensaje.valor);
			resp_msg.gen_msg.socket_descriptor = mensaje.gen_msg.socket_descriptor;
			resp_msg.gen_msg.socket_descriptor_server = mensaje.gen_msg.socket_descriptor_server;
			resp_msg.gen_msg.id_MSJ = GRABAR_VALOR;
			resp_msg.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
			resp_msg.OK = 1;
			varGlobal = resp_msg.valor;
			enviar_mensaje(resp_msg.gen_msg.socket_descriptor_server,resp_msg.gen_msg.id_MSJ, &resp_msg);
			break;

		case (SIGNAL):
			printf("Recibo orden de hacer signal de un semaforo\n");
			printf("El semaforo es %s\n",mensaje.id_sem);
			resp_msg.gen_msg.socket_descriptor = mensaje.gen_msg.socket_descriptor;
			resp_msg.gen_msg.socket_descriptor_server = mensaje.gen_msg.socket_descriptor_server;
			resp_msg.gen_msg.id_MSJ = SIGNAL;
			resp_msg.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
			resp_msg.OK = 1;
			enviar_mensaje(resp_msg.gen_msg.socket_descriptor_server,resp_msg.gen_msg.id_MSJ, &resp_msg);
			break;

		case (WAIT):
			printf("Recibo orden de hacer wait de un semaforo\n");
			printf("El semaforo es %s\n",mensaje.id_sem);
			resp_msg.gen_msg.socket_descriptor = mensaje.gen_msg.socket_descriptor;
			resp_msg.gen_msg.socket_descriptor_server = mensaje.gen_msg.socket_descriptor_server;
			resp_msg.gen_msg.id_MSJ = WAIT;
			resp_msg.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
			resp_msg.OK = 0; // Para que siga ejecutando
//			resp_msg.OK = 1; // Para que desaloje
			enviar_mensaje(resp_msg.gen_msg.socket_descriptor_server,resp_msg.gen_msg.id_MSJ, &resp_msg);
			break;

		case (NOTIFICACION_QUANTUM):
			puts("Se consumio un quantum");
			break;



		default:
			printf("Recibi un mje default del tipo %i\n",mensaje.gen_msg.id_MSJ);
			printf("gen_msg descriptor recibido es %i y descriptor server es %i\n",mensaje.gen_msg.socket_descriptor,mensaje.gen_msg.socket_descriptor_server);
			puts("NO SE QUE HACER CON ESTE MENSAJE\n");
			break;

		}
//		free(resp_msg);
	}
	puts("Fin primera recepcion");
	return NULL;
}

int conectarAUMV(void){

		printf("SE CONECTA POR SOCKET %i\n",socket_cliente);
		t_KER_PRO_CPU_UMV* mensajeARecibir;
		t_KER_PRO_CPU_UMV mensaje=obtener_nueva_shared_str();
		mensaje.gen_msg.id_MSJ = HANDSHAKE_PLP_UMV;
		printf("SE ENVIA MENSAJE %i POR SOCKET %i\n",mensaje.gen_msg.id_MSJ,socket_cliente);
		enviar_mensaje_a_servidor(socket_cliente,HANDSHAKE_PLP_UMV,&mensaje);
		printf("SE ESPERA RESPUESTA\n");
		mensajeARecibir = recibir_mensaje_de_servidor(socket_cliente);
		if(mensajeARecibir->gen_msg.id_MSJ==HANDSHAKE_PLP_UMV){
			printf("HANDHSAKE OK\n");
			return EXIT_SUCCESS;
		}
		return -1;
}

t_PCB* crear_segmentos(t_medatada_program* metaData,int lenCod){
	t_PCB *pcb = malloc(sizeof(t_PCB));
	t_KER_PRO_CPU_UMV mensajeAEnviar=obtener_nueva_shared_str();
	t_KER_PRO_CPU_UMV* mensaje;
	int pid=1;
	pcb->id=pid;
	pcb->PC=metaData->instruccion_inicio;
	pcb->cursorStack=0;
	pcb->tamContext=0;
	pcb->tamEtiq=metaData->etiquetas_size;

	mensajeAEnviar.tamanio_segmento = metaData->etiquetas_size;
	mensajeAEnviar.PID=pid;
	enviar_mensaje_a_servidor(socket_cliente,CREAR_SEGMENTOS,&mensajeAEnviar);
	void *msj = recibir_mensaje_de_servidor(socket_cliente);
	if (msj != NULL) {
	mensaje = (t_KER_PRO_CPU_UMV*) msj;
	}
	if(mensaje->gen_msg.id_MSJ == CREAR_SEGMENTOS && mensaje->OK == 1){
		pcb->indEtiq = mensaje->base_segmento;
	}else{
		abort();
	}
	mensajeAEnviar=obtener_nueva_shared_str();
	mensajeAEnviar.tamanio_segmento = metaData->instrucciones_size * 8;
	mensajeAEnviar.PID=pid;
	enviar_mensaje_a_servidor(socket_cliente,CREAR_SEGMENTOS,&mensajeAEnviar);
	msj = recibir_mensaje_de_servidor(socket_cliente);
	if (msj != NULL) {
		mensaje = (t_KER_PRO_CPU_UMV*) msj;
		}
	if(mensaje->gen_msg.id_MSJ == CREAR_SEGMENTOS && mensaje->OK == 1){
		pcb->indCod = mensaje->base_segmento;
		}else{
			abort();
		}
	mensajeAEnviar=obtener_nueva_shared_str();
	mensajeAEnviar.tamanio_segmento = 100;
	mensajeAEnviar.PID=pid;
	enviar_mensaje_a_servidor(socket_cliente,CREAR_SEGMENTOS,&mensajeAEnviar);
	msj = recibir_mensaje_de_servidor(socket_cliente);
	if (msj != NULL) {
			mensaje = (t_KER_PRO_CPU_UMV*) msj;
		}

	if(mensaje->gen_msg.id_MSJ == CREAR_SEGMENTOS && mensaje->OK == 1){
			pcb->stackSeg = mensaje->base_segmento;
					}else{
						abort();
					}


	mensajeAEnviar=obtener_nueva_shared_str();
	mensajeAEnviar.tamanio_segmento = lenCod;
	mensajeAEnviar.PID=pid;
	enviar_mensaje_a_servidor(socket_cliente,CREAR_SEGMENTOS,&mensajeAEnviar);
	msj = recibir_mensaje_de_servidor(socket_cliente);
	if (msj != NULL) {
	mensaje = (t_KER_PRO_CPU_UMV*) msj;
	}
	if(mensaje->gen_msg.id_MSJ == CREAR_SEGMENTOS && mensaje->OK == 1){
				pcb->codSeg = mensaje->base_segmento;
		}else{
			abort();
		}
	return pcb;
}

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

void enviarBytes(t_PCB* pcb,char* codigo,t_medatada_program* metadata){

	t_KER_PRO_CPU_UMV* mensajeARecibir;
	t_KER_PRO_CPU_UMV mensajeAEnviar=obtener_nueva_shared_str();
	mensajeAEnviar.PID=pcb->id;
	mensajeAEnviar.base_bytes=pcb->codSeg;
	mensajeAEnviar.tamanio_bytes=strlen(codigo);
	mensajeAEnviar.buffer=codigo;
	mensajeAEnviar.offset_bytes=0;
	printf("EL BUFFER es %s\n",mensajeAEnviar.buffer);
	printf("SE ENVIA MENSAJE %i POR SOCKET %i\n",mensajeAEnviar.gen_msg.id_MSJ,socket_cliente);
	enviar_mensaje_a_servidor(socket_cliente,ENVIAR_BYTES,&mensajeAEnviar);
	printf("SE ESPERA RESPUESTA (SE LE ENVIO EL CODIGO)\n");
	mensajeARecibir = recibir_mensaje_de_servidor(socket_cliente);
	if(mensajeARecibir->OK==0){
		printf("error al recibir respuesta\n");
		return;
		}
	if(mensajeARecibir->gen_msg.id_MSJ==ENVIAR_BYTES){
				printf("--------------------TODO OK\n");
		}

	mensajeAEnviar=obtener_nueva_shared_str();
		mensajeAEnviar.PID=pcb->id;
		mensajeAEnviar.base_bytes=pcb->indCod;
		mensajeAEnviar.tamanio_bytes=metadata->instrucciones_size*8;
		printf("EL TAMANIO DE LAS INTRUCCIONES X 8 es %i\n",mensajeAEnviar.tamanio_bytes);
		mensajeAEnviar.buffer=malloc(metadata->instrucciones_size*8);
		memcpy(mensajeAEnviar.buffer,metadata->instrucciones_serializado,metadata->instrucciones_size*8);
		printf("SE COPIARON LAS INTRUCCIONES AL BUFFER\n");
		mensajeAEnviar.offset_bytes=0;
		printf("EL BUFFER es %s\n",mensajeAEnviar.buffer);
		imprimirStream(mensajeAEnviar.buffer,mensajeAEnviar.tamanio_bytes);
		printf("SE ENVIA MENSAJE %i POR SOCKET %i\n",mensajeAEnviar.gen_msg.id_MSJ,socket_cliente);
		enviar_mensaje_a_servidor(socket_cliente,ENVIAR_BYTES,&mensajeAEnviar);
		free(mensajeAEnviar.buffer);
		printf("SE ESPERA RESPUESTA (SE LE ENVIARON INSTRUCCIONES)\n");
		mensajeARecibir = recibir_mensaje_de_servidor(socket_cliente);
		if(mensajeARecibir->OK==0){
			printf("error al recibir respuesta\n");
			return;
			}
		if(mensajeARecibir->gen_msg.id_MSJ==ENVIAR_BYTES){
					printf("--------------------TODO OK\n");
	}
	mensajeAEnviar=obtener_nueva_shared_str();
			mensajeAEnviar.PID=pcb->id;
			mensajeAEnviar.base_bytes=pcb->indEtiq;
			mensajeAEnviar.tamanio_bytes=pcb->tamEtiq;
			mensajeAEnviar.buffer=metadata->etiquetas;
			mensajeAEnviar.offset_bytes=0;
			printf("EL BUFFER es %s\n",mensajeAEnviar.buffer);
			imprimirStream(mensajeAEnviar.buffer,mensajeAEnviar.tamanio_bytes);
			if(mensajeAEnviar.buffer==NULL){
				mensajeAEnviar.buffer="\0";
			}
			printf("SE ENVIA MENSAJE %i POR SOCKET %i\n",mensajeAEnviar.gen_msg.id_MSJ,socket_cliente);
			enviar_mensaje_a_servidor(socket_cliente,ENVIAR_BYTES,&mensajeAEnviar);
			printf("SE ESPERA RESPUESTA(SE LE ENVIARON ETIQUETAS)\n");
			mensajeARecibir = recibir_mensaje_de_servidor(socket_cliente);
			if(mensajeARecibir->OK==0){
				printf("error al recibir respuesta\n");
				return;
				}
			if(mensajeARecibir->gen_msg.id_MSJ==ENVIAR_BYTES){
						printf("--------------------TODO OK\n");
		}

	return;
}

void imprimirStream(void* stream, int tamanio){
 int i=0;
 char* cadena = (char*) stream;
 for (i=0;i<tamanio;i++){
  printf("%c",cadena[i]);
 }
 printf("\n");
}
