/*
 * cpu.c
 *
 *  Created on: 17/04/2014
 *      Author: utnso
 */


#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <commons/temporal.h>
#include <commons/log_app.h>
#include <commons/string.h>
#include <commons/tad_cliente.h>
#include <commons/shared_strs.h>
#include <commons/collections/dictionary.h>
#include "cpu.h"
#include "ansisop_prim.h"
#include "config_cpu.h"
#include <parser/metadata_program.h>

#define PATH_CONFIG "config_cpu.cfg"

// --------------------- FUNCIONES DEL PARSER ANSISOP -------------------

AnSISOP_funciones functions = {
		.AnSISOP_definirVariable		= definirVariable,
		.AnSISOP_imprimirTexto			= imprimirTexto,
		.AnSISOP_obtenerPosicionVariable= obtenerPosicionVariable,
		.AnSISOP_dereferenciar			= dereferenciar,
		.AnSISOP_asignar				= asignar,
		.AnSISOP_imprimir				= imprimir,
		.AnSISOP_irAlLabel				= irAlLabel,
		.AnSISOP_asignarValorCompartida = asignarValorCompartida,
		.AnSISOP_entradaSalida 			= entradaSalida,
		.AnSISOP_obtenerValorCompartida	= obtenerValorCompartida,
		.AnSISOP_llamarSinRetorno		= llamarSinRetorno,
		.AnSISOP_llamarConRetorno		= llamarConRetorno,
		.AnSISOP_finalizar				= finalizar,
		.AnSISOP_retornar				= retornar,
};
AnSISOP_kernel kernel_functions = {
		.AnSISOP_signal 				= semaforo_signal,
		.AnSISOP_wait					= semaforo_wait,
};

// ------------------------- FIN PARSER ANSISOP -------------------------


// ------------------------ VARIABLES GLOBALES ---------------------------------

// Variables para la conexion
int port_kernel;
int port_umv;
char* ip_kernel;
char* ip_umv;
int socketKernel;
int socketUMV;

// Variables para la ejecucion

char* stackChar; // Literal de recepcion de stack segment
char* sentenciaEjec; // Puntero a la sentencia a ejecutar
int finProg; // Controla la finalizacion del cliente
int finalizado; // Se cambia a 1 si el programa finalizo, solo desde las primitivas
int senialRec; // Es 1 si recibi señal, 0 si no.
int enviadoIO; //1 si se envio el proceso a I/O
uint32_t idCpu; // Identificador de CPU. 50 es sin identificar.
uint32_t tengoProg; // Chequea con 1 que tengo programa asignado. 0 es que no
int fallaEtiquetas;
char* mjeError;


// -----------------------------  FIN V.G. -----------------------------------------------------



// ---------------------- FUNCIONES DE MANEJO DEL DICCIONARIO DEL STACK ---------------------------

/*
 * Funcion que pone en 0 el diccionario
 * de datos que luego contendra variables
 * y su offset en el stack
 */
void inicializarDiccionario(t_dicVar* diccionario){
	int i=0;
	for (i=0;i<MAXVARIABLES;i++){
		diccionario->variable[i] = '-';
		diccionario->offset[i] = 0;
	}
	diccionario->ultimo = 0;
	stack->cursor = pcb.cursorStack;
	deshabilitar_print_console();
	log_debug_message("Inicializado el diccionario de variables");
	habilitar_print_console();
}

/*
 * Agrega una variable con su ubicacion dentro del stack
 * en el diccionario, y aumenta el valor del ultimo lugar
 * disponible para almacenar
 */
void indexarEnDiccionario(t_dicVar* diccionario, char id, int offset){
	int i = diccionario->ultimo;
	diccionario->offset[i] = offset;
	diccionario->variable[i] = id;
	diccionario->ultimo++;

	sprintf(log_cpu,"Se indexa la variable %c ubicada en %i del stack, en posicion %i del diccionario",id,offset,i);
	deshabilitar_print_console();
	log_debug_message(log_cpu);
	habilitar_print_console();
}

/*
 * Funcion que retorna la posicion de una variable dentro del stack.
 * Si no existe retorna -1.
 */
int obtenerPosicionDic(t_dicVar* diccionario, char id){
	int max = diccionario->ultimo;
	int i=0;
	int posicion = -1;
	for (i=0;i<max;i++){
		if (diccionario->variable[i] == id){
			posicion = diccionario->offset[i];
		}
	}

	sprintf(log_cpu,"La posicion de la variable %c en el stack indexada en el diccionario es %i",id,posicion);
	deshabilitar_print_console();
	log_debug_message(log_cpu);
	habilitar_print_console();

	return posicion;
}

/*
 * Funcion que crea el diccionario de datos
 * a partir de la recepcion del stack por parte
 * del StackSegment procedente del UMV
 */
void cargarDiccionario(t_dicVar* diccionario, t_stack_c* stack, int tamContext){
	int i=0;
	char id = '-';
	int offset = stack->cursor;
	for (i=0;i<tamContext;i++){
		memcpy(&id,stack->segmento+offset,1);
		indexarEnDiccionario(diccionario,id,offset+1);
		offset = offset + sizeof(t_variable);
	}
	deshabilitar_print_console();
	log_debug_message("Se carga el diccionario de variables en base al stack");
	habilitar_print_console();
}

/*
 * Funcion para visualizacion que imprime el indexado del diccionario
 * con las variables indexadas.
 */
void imprimirDiccionario(t_dicVar* diccionario){
	deshabilitar_print_console();
	log_debug_message("\n");
	log_debug_message("---------------------- DICCIONARIO ACTUAL ----------------\n");
	if (diccionario->ultimo == 0){
		sprintf(log_cpu,"El diccionario se encuentra sin variables indexadas");
		log_debug_message(log_cpu);
		sprintf(log_cpu,"Ultimo es: %i\n",diccionario->ultimo);
		log_debug_message(log_cpu);
		sprintf(log_cpu,"El cursor apunta a %i\n",stack->cursor);
		log_debug_message(log_cpu);
	} else {
		int i=0;
		for (i=0;i<diccionario->ultimo;i++){
			sprintf(log_cpu,"Variable %c   ||    Posicion stack %i\n",diccionario->variable[i],diccionario->offset[i]);
			log_debug_message(log_cpu);
		}
	}
	log_debug_message("-----------------------------------------------------------\n");
	log_debug_message("\n");
	habilitar_print_console();
}

// ---------------------- FIN FUNCIONES DICCIONARIO ---------------------------



// ---------------- MJES SIN TRACE ---------------------------------------------------


int enviarMjeSinConsola(int socket_descriptor, int id_MSG, t_KER_PRO_CPU_UMV *message){
	deshabilitar_print_console ();
	int mje = enviar_mensaje_a_servidor(socket_descriptor,id_MSG,message);
	habilitar_print_console ();
	return mje;
}

t_KER_PRO_CPU_UMV* recibirMjeSinConsola(int sock_descriptor){
	deshabilitar_print_console ();
	t_KER_PRO_CPU_UMV* mensaje = recibir_mensaje_de_servidor(sock_descriptor);
	habilitar_print_console ();
	return mensaje;
}

// ---------------- FIN MJES SIN TRACE ---------------------------------------------------



// --------------------------- ENVIOS AL KERNEL ---------------------


/*
 * Funcion que envia al kernel un aviso de finalizacion
 * de programa con el mensaje de error del motivo.
 * Cargado en el campo texto del mje de envio.
 */
void informoErrorYCierre(){
	// Aviso a kernel que finalizo
	t_KER_PRO_CPU_UMV mensaje = obtener_nueva_shared_str();
	mensaje.gen_msg.socket_descriptor = socketKernel;
	mensaje.gen_msg.socket_descriptor_server = socketKernel;
	mensaje.gen_msg.id_MSJ = FINALIZACION_EJECUCION_PROGRAMA;
	mensaje.identificador_cpu = idCpu;
	mensaje.PID = pcb.id;
	mensaje.PCB = pcb;
	if (senialRec == 1){
		mensaje.OK = 10;
	} else {
		mensaje.OK = 1;
	}
	mensaje.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);

	mensaje.texto = string_new();
	string_append(&mensaje.texto,mjeError);

	deshabilitar_print_console();
	sprintf(log_cpu,"Informo Falla y envio mje de error al kernel: %s",mensaje.texto);
	log_error_message(log_cpu);
	habilitar_print_console();

	enviarMjeSinConsola(mensaje.gen_msg.socket_descriptor_server,mensaje.gen_msg.id_MSJ,&mensaje);

	solicitarDestruirSegmentos();

	free(mensaje.texto);
}

/*
 * Funcion que ejecuta el signal del semaforo
 * en cuestion.
 */
void hacerSignalSemaforo(t_nombre_semaforo identificador_semaforo){
	t_KER_PRO_CPU_UMV mensajeEnvio = obtener_nueva_shared_str();
	mensajeEnvio.gen_msg.socket_descriptor = socketKernel;
	mensajeEnvio.gen_msg.socket_descriptor_server = socketKernel;
	mensajeEnvio.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	mensajeEnvio.gen_msg.id_MSJ = SIGNAL;
	mensajeEnvio.PID = pcb.id;
	mensajeEnvio.identificador_cpu = idCpu;
	if (senialRec == 1){
		mensajeEnvio.OK = 10;
	} else {
		mensajeEnvio.OK = 1;
	}

	mensajeEnvio.id_sem = string_new();
	string_append(&mensajeEnvio.id_sem,identificador_semaforo);

	sprintf(log_cpu,"Realizo Signal en semaforo: %s",mensajeEnvio.id_sem);
	log_info_message(log_cpu);
	enviarMjeSinConsola(mensajeEnvio.gen_msg.socket_descriptor_server,mensajeEnvio.gen_msg.id_MSJ,&mensajeEnvio);
	// Espero recepcion de confirmacion de parte del kernel
//	t_KER_PRO_CPU_UMV *pMensaje = malloc(sizeof(t_KER_PRO_CPU_UMV*));
//	pMensaje = recibirMjeSinConsola(socketKernel);
	t_KER_PRO_CPU_UMV* pMensaje = recibirMjeSinConsola(socketKernel);

	// Chequeo recepcion del valor asignado
	if ((pMensaje->gen_msg.id_MSJ == SIGNAL)&&(pMensaje->OK == 1)) {
		deshabilitar_print_console();
		sprintf(log_cpu,"Se realizo correctamente el signal");
		log_debug_message(log_cpu);
		habilitar_print_console();
	} else {
		log_error_message("No se realizo el signal del semaforo");
		sprintf(log_cpu,"Fallo: %s",pMensaje->mensaje);
		log_error_message(log_cpu);
		// DESALOJO EL PROGRAMA
		desalojar();
		corteProg = 1;
	}
	free(mensajeEnvio.id_sem);
	free(pMensaje);
}


/*
 * Funcion que ejecuta el wait del semaforo
 * en cuestion.
 */
void hacerWaitSemaforo(t_nombre_semaforo identificador_semaforo){
	t_KER_PRO_CPU_UMV mensajeEnvio = obtener_nueva_shared_str();
	pcb.PC++;
	mensajeEnvio.gen_msg.socket_descriptor = socketKernel;
	mensajeEnvio.gen_msg.socket_descriptor_server = socketKernel;
	mensajeEnvio.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	mensajeEnvio.gen_msg.id_MSJ = WAIT;
	mensajeEnvio.identificador_cpu = idCpu;
	mensajeEnvio.PID = pcb.id;
	mensajeEnvio.PCB = pcb;

	mensajeEnvio.id_sem = string_new();
	string_append(&mensajeEnvio.id_sem,identificador_semaforo);

	sprintf(log_cpu,"Realizo Wait en semaforo: %s",mensajeEnvio.id_sem);
	log_info_message(log_cpu);
	enviarMjeSinConsola(mensajeEnvio.gen_msg.socket_descriptor_server,mensajeEnvio.gen_msg.id_MSJ,&mensajeEnvio);
	free(mensajeEnvio.id_sem);

	grabarEnStackUMV(socketUMV,pcb); //AGREGAGO AHORA VER SI ESTA BIENNN #########################

	// Espero recepcion de confirmacion de parte del kernel
//	t_KER_PRO_CPU_UMV *pMensaje = malloc(sizeof(t_KER_PRO_CPU_UMV*));
//	pMensaje = recibirMjeSinConsola(socketKernel);
	t_KER_PRO_CPU_UMV* pMensaje = recibirMjeSinConsola(socketKernel);

	// CON OK=1 DESALOJARME
	// CON OK=0 NO HACER NADA. SIGUE LA EJECUCION
	deshabilitar_print_console();
	if ((pMensaje->gen_msg.id_MSJ == WAIT)&&(pMensaje->OK == 0)) {
		sprintf(log_cpu,"Se realizo correctamente el wait");
		log_debug_message(log_cpu);
		pcb.PC--;
	} else if ((pMensaje->gen_msg.id_MSJ == WAIT)&&(pMensaje->OK == 1)) {
		sprintf(log_cpu,"El wait indica que debo desalojarme");
		log_debug_message(log_cpu);
		// DESALOJO EL PROGRAMA
		desalojar();
		corteProg = 1; // Para desalojar el programa y no volver a ejecutarlo
		finalizado = 1;
	} else { // EL ok por ser error es 15, por ahora no debo tratarlo
		log_error_message("No se realizo el wait del semaforo");
		sprintf(log_cpu,"Fallo: %s",pMensaje->mensaje);
		log_error_message(log_cpu);
		desalojar();
	}
	habilitar_print_console();
	free(pMensaje);
}

/*
 * Funcion que asigna el valor de una variable global
 * en el kernel
 */
t_valor_variable asignarValorGlobal(t_nombre_compartida variable, t_valor_variable valor){
	t_valor_variable resul;
	t_KER_PRO_CPU_UMV mensajeEnvio = obtener_nueva_shared_str();
	mensajeEnvio.gen_msg.socket_descriptor = socketKernel;
	mensajeEnvio.gen_msg.socket_descriptor_server = socketKernel;
	mensajeEnvio.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	mensajeEnvio.gen_msg.id_MSJ = GRABAR_VALOR;
	mensajeEnvio.valor = valor;
	mensajeEnvio.identificador_cpu = idCpu;
	mensajeEnvio.PID = pcb.id;

	mensajeEnvio.id_var_comp = string_new();
	string_append(&mensajeEnvio.id_var_comp,variable);

	enviarMjeSinConsola(mensajeEnvio.gen_msg.socket_descriptor_server,mensajeEnvio.gen_msg.id_MSJ,&mensajeEnvio);
	// Espero recepcion de confirmacion de parte del kernel
//	t_KER_PRO_CPU_UMV *pMensaje = malloc(sizeof(t_KER_PRO_CPU_UMV));
//	pMensaje = recibirMjeSinConsola(socketKernel);
	t_KER_PRO_CPU_UMV* pMensaje = recibirMjeSinConsola(socketKernel);

	// Chequeo recepcion del valor asignado
	if ((pMensaje->gen_msg.id_MSJ == GRABAR_VALOR)&&(pMensaje->OK == 1)) {
		sprintf(log_cpu,"Asigno el valor %i de la variable compartida %s",pMensaje->valor,mensajeEnvio.id_var_comp);
		log_info_message(log_cpu);
		deshabilitar_print_console();
		sprintf(log_cpu,"Recibo el valor de confirmacion %i de la variable compartida",pMensaje->valor);
		log_debug_message(log_cpu);
		habilitar_print_console();
		resul = pMensaje->valor;
	} else {
		log_error_message("No se recibio el valor de la variable compartida");
		sprintf(log_cpu,"Fallo: %s",pMensaje->mensaje);
		log_error_message(log_cpu);
		resul = -1;
		// DESALOJO EL PROGRAMA
		desalojar();
		corteProg = 1;
	}
	free(mensajeEnvio.id_var_comp);
	free(pMensaje);
	return resul;
}

/*
 * Funcion que solicita al kernel el valor
 * de una variable global. Retorna el valor
 * de la variable o -1 si da error
 */
t_valor_variable pedirValorGlobal(t_nombre_compartida variable){
	t_valor_variable valor;
	t_KER_PRO_CPU_UMV mensajeEnvio = obtener_nueva_shared_str();
	mensajeEnvio.gen_msg.socket_descriptor = socketKernel;
	mensajeEnvio.gen_msg.socket_descriptor_server = socketKernel;
	mensajeEnvio.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	mensajeEnvio.gen_msg.id_MSJ = OBTENER_VALOR;
	mensajeEnvio.identificador_cpu = idCpu;
	mensajeEnvio.PID = pcb.id;
	mensajeEnvio.id_var_comp = string_new();
	string_append(&mensajeEnvio.id_var_comp,variable);
	enviarMjeSinConsola(mensajeEnvio.gen_msg.socket_descriptor_server,mensajeEnvio.gen_msg.id_MSJ,&mensajeEnvio);
	sprintf(log_cpu,"Solicito el valor de la variable compartida %s",mensajeEnvio.id_var_comp);
	log_info_message(log_cpu);
	// Espero recepcion de confirmacion de parte del kernel
//	t_KER_PRO_CPU_UMV *pMensaje = malloc(sizeof(t_KER_PRO_CPU_UMV*));
//	pMensaje = recibirMjeSinConsola(socketKernel);
	t_KER_PRO_CPU_UMV* pMensaje = recibirMjeSinConsola(socketKernel);

	// Chequeo recepcion valor de la variable global
	if ((pMensaje->gen_msg.id_MSJ == OBTENER_VALOR)&&(pMensaje->OK == 1)) {
		sprintf(log_cpu,"Recibo el valor: %i",pMensaje->valor);
		log_info_message(log_cpu);
		valor = pMensaje->valor;
	} else {
		log_error_message("No se recibio el valor de la variable compartida");
		sprintf(log_cpu,"Fallo: %s",pMensaje->mensaje);
		log_error_message(log_cpu);
		valor = -1;
		// DESALOJO EL PROGRAMA
		desalojar();
		corteProg = 1;
	}
	free(mensajeEnvio.id_var_comp);
	free(pMensaje);
	return valor;
}


/*
 * Funcion que avisa al Kernel que el programa ha finalizado
 * su ejecucion, llegando al final y correctamente. ENvia el estado
 * final de las variables tambien
 */
void avisoFinalizacionPrograma(){

	t_KER_PRO_CPU_UMV mensaje = obtener_nueva_shared_str();

	mensaje.gen_msg.socket_descriptor = socketKernel;
	mensaje.gen_msg.socket_descriptor_server = socketKernel;
	mensaje.gen_msg.id_MSJ = FINALIZACION_EJECUCION_PROGRAMA;
	mensaje.identificador_cpu = idCpu;
	mensaje.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	mensaje.PID = pcb.id;
	mensaje.PCB = pcb;
	if (senialRec == 1){
		mensaje.OK = 10;
	} else {
		mensaje.OK = 1;
	}

	char* cadena = string_new();
	string_append(&cadena,"------------ESTADO FINAL DE VARIABLES-----------\n");

	int inicio = stack->cursor;
	int i=0;
	int offset = 0;
	for(i=0;i<pcb.tamContext;i++){
		int valor;
		char id;
		char* mjeVariables = string_new();
		string_append(&mjeVariables,"..... Variable id: %c ----- Variable valor: %i ------\n");
		memcpy(&id,stack->segmento+inicio+offset,sizeof(char));
		memcpy(&valor,stack->segmento+inicio+offset+1,sizeof(uint32_t));
		sprintf(mjeVariables,"--- VAR ID: %c  = %i ---\n",id,valor);
		offset = offset + sizeof(t_variable);
		string_append(&cadena,mjeVariables);
		free(mjeVariables);
	}
	sprintf(log_cpu,"Largo de la cadena quedo %i \n",strlen(cadena));
	log_debug_message(log_cpu);
	mensaje.texto = string_new();
	string_append(&mensaje.texto,cadena);
	// Generado el mensaje con el estado final de las variables

	enviarMjeSinConsola(mensaje.gen_msg.socket_descriptor_server,mensaje.gen_msg.id_MSJ,&mensaje);
	free(cadena);
	free(mensaje.texto);
}

/*
 * Funcion que comunica al kernel que se ha desconectado
 * (Se va a cerrar el proceso)
 */
void meDesconectoKernel(){
	t_KER_PRO_CPU_UMV mensajeEnvio = obtener_nueva_shared_str();
	mensajeEnvio.gen_msg.socket_descriptor = socketKernel;
	mensajeEnvio.gen_msg.socket_descriptor_server = socketKernel;
	mensajeEnvio.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	mensajeEnvio.PID = pcb.id;
	mensajeEnvio.OK = 1;
	mensajeEnvio.identificador_cpu = idCpu;
	mensajeEnvio.gen_msg.id_MSJ = DESCONEXION_CPU;
	deshabilitar_print_console();
	sprintf(log_cpu,"Envio a desconectar PID: %i",mensajeEnvio.PID);
	log_debug_message(log_cpu);
	habilitar_print_console();
	enviarMjeSinConsola(mensajeEnvio.gen_msg.socket_descriptor_server,mensajeEnvio.gen_msg.id_MSJ,&mensajeEnvio);

}


/*
 * Funcion para solicitar una entrada/salida al kernel,
 * a traves del envio de un mensaje.
 */
int enviarIOalKernel(t_nombre_dispositivo dispositivo, int tiempo){
	t_KER_PRO_CPU_UMV mensajeEnvio = obtener_nueva_shared_str();
	mensajeEnvio.gen_msg.socket_descriptor = socketKernel;
	mensajeEnvio.gen_msg.socket_descriptor_server = socketKernel;
	mensajeEnvio.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	mensajeEnvio.gen_msg.id_MSJ = ENTRADA_SALIDA;

	mensajeEnvio.id_dispositivo = string_new();
	string_append(&mensajeEnvio.id_dispositivo,dispositivo);

	mensajeEnvio.utilizacion = tiempo;
	mensajeEnvio.PID = pcb.id;
	mensajeEnvio.identificador_cpu = idCpu;
	pcb.PC++; //Aumento el PC previamente a enviar el pcb al Kernel
	mensajeEnvio.PCB = pcb; // Mando el pcb con el estado final
	if (senialRec == 1){
		mensajeEnvio.OK = 10;
	} else {
		mensajeEnvio.OK = 1;
	}
	log_debug_message(log_cpu);
	enviarMjeSinConsola(mensajeEnvio.gen_msg.socket_descriptor_server,mensajeEnvio.gen_msg.id_MSJ,&mensajeEnvio);
	// Espero recepcion de confirmacion de parte del kernel

		sprintf(log_cpu,"Solicito I/O: Dispositivo: %s - Tiempo: %i",mensajeEnvio.id_dispositivo,mensajeEnvio.utilizacion);
		log_info_message(log_cpu);
		// Desalojo el programa luego del envio a I/O
		grabarEnStackUMV(socketUMV,pcb); //AGREGAGO AHORA VER SI ESTA BIENNN #########################
		desalojar();
		corteProg = 1;
		enviadoIO = 1;
		return 1;

	free(mensajeEnvio.id_dispositivo);

}

/*
 * Funcion que envia al Kernel el pcb en el estado actual
 * (luego de ejecutar sentencias).
 */
void enviarPCBalKernel(){
	t_KER_PRO_CPU_UMV mensajeEnvio = obtener_nueva_shared_str();
	mensajeEnvio.gen_msg.socket_descriptor = socketKernel;
	mensajeEnvio.gen_msg.socket_descriptor_server = socketKernel;
	char* mensaje = "El programa se desaloja por contener sentencias erroneas";

	mensajeEnvio.texto = string_new();
	string_append(&mensajeEnvio.texto,mensaje);

	if (corteProg == 1){
		mensajeEnvio.gen_msg.id_MSJ = FINALIZACION_EJECUCION_PROGRAMA;

	} else {
		mensajeEnvio.gen_msg.id_MSJ = FINALIZAR_EJECUCION;
	}
	mensajeEnvio.identificador_cpu = idCpu;
	mensajeEnvio.PID = pcb.id;

	mensajeEnvio.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	// Copio los valores del pcb
	memcpy(&mensajeEnvio.PCB,&pcb,sizeof(t_PCB));
	if (senialRec == 1){
		mensajeEnvio.OK = 10;
	} else {
		mensajeEnvio.OK = 1;
	}
	inicializarDiccionario(diccionario);
	enviarMjeSinConsola(mensajeEnvio.gen_msg.socket_descriptor_server,mensajeEnvio.gen_msg.id_MSJ,&mensajeEnvio);
	free(mensajeEnvio.texto);
}


/*
 * Funcion que envia aviso de finalizacion de un
 * quantum unitario.
 */
void enviarFinQuantumUnit(){
	t_KER_PRO_CPU_UMV mensajeEnvio = obtener_nueva_shared_str();
	mensajeEnvio.gen_msg.socket_descriptor = socketKernel;
	mensajeEnvio.gen_msg.socket_descriptor_server = socketKernel;
	mensajeEnvio.identificador_cpu = idCpu;
	mensajeEnvio.PID = pcb.id;
	mensajeEnvio.OK = pcb.PC;
	mensajeEnvio.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	mensajeEnvio.gen_msg.id_MSJ = NOTIFICACION_QUANTUM;

	enviarMjeSinConsola(mensajeEnvio.gen_msg.socket_descriptor_server,mensajeEnvio.gen_msg.id_MSJ,&mensajeEnvio);
	deshabilitar_print_console();
	sprintf(log_cpu,"Envio finalizacion unitaria de quantum");
	log_debug_message(log_cpu);
	habilitar_print_console();
}


/*
 * Funcion para enviar un mensaje al Kernel con un literal
 * REVISAR LUEGO
 */
//void enviarMensajeAKernel(char* mensaje, t_valor_variable valor, int idMje, uint32_t largo){
void enviarMensajeAKernel(char* mensaje, t_valor_variable valor, int idMje){
	t_KER_PRO_CPU_UMV mensajeEnvio = obtener_nueva_shared_str();
	mensajeEnvio.gen_msg.socket_descriptor = socketKernel;
	mensajeEnvio.gen_msg.socket_descriptor_server = socketKernel;
	mensajeEnvio.identificador_cpu = idCpu;
	mensajeEnvio.PID = pcb.id;
	mensajeEnvio.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	if (idMje == IMPRIMIR_TEXTO){
		mensajeEnvio.gen_msg.id_MSJ = IMPRIMIR_TEXTO;
		mensajeEnvio.texto = string_new();
		string_append(&mensajeEnvio.texto,mensaje);
	} else if (idMje == IMPRIMIR){
		mensajeEnvio.gen_msg.id_MSJ = IMPRIMIR;
		mensajeEnvio.valor = valor;
	}
	enviarMjeSinConsola(mensajeEnvio.gen_msg.socket_descriptor_server,mensajeEnvio.gen_msg.id_MSJ,&mensajeEnvio);
	if (idMje == IMPRIMIR_TEXTO){
		sprintf(log_cpu,"Enviar al Kernel el texto: %s ",mensajeEnvio.texto);
		log_info_message(log_cpu);
		free(mensajeEnvio.texto);
	} else if (idMje == IMPRIMIR){
		sprintf(log_cpu,"Envio a imprimir al Kernel el valor: %i",mensajeEnvio.valor);
		log_info_message(log_cpu);
	}
}


// --------------------------- FIN ENVIO KERNEL ----------------------



// ------------------------- FUNCIONES AUXILIARES ---------------------

/*
 * Funcion que tiene las funcionalidades
 * de desalojar un programa.
 */
void desalojar(){
	desalojado = 1;
	controlQuantum = quantum;
}


/*
 * Funcion que busca la instruccion de una etiqueta solicitada
 * por las primitivas de AnSISOP
 */
t_puntero_instruccion buscarEtiqueta(t_nombre_etiqueta etiq, char* indEtiq,t_size tamEtiq){
	// BUSCAR POR QUE DEVUELVE -1 AL BUSCAR LA ETIQUETA SIGUIENTE
	t_puntero_instruccion puntero;
	puntero = metadata_buscar_etiqueta(etiq,serEtiquetas,pcb.tamEtiq);
	return puntero;
}


/*
 * Funcion auxiliar. Es solo para probar lo que lee
 * el pcb.
 * DESPUES BORRARLA
 */
void imprimirPCB(t_PCB* pcb){
	deshabilitar_print_console();
	sprintf(log_cpu,"\n");
	log_debug_message(log_cpu);
	sprintf(log_cpu,"----------- ESTADO DE PCB --------------- \n");
	log_debug_message(log_cpu);
	sprintf(log_cpu,"Id: %i      -------------    PC: %i\n",pcb->id,pcb->PC);
	log_debug_message(log_cpu);
	sprintf(log_cpu,"codSeg: %i    ----------------   Cursor Stack:%i\n",pcb->codSeg,pcb->cursorStack);
	log_debug_message(log_cpu);
	sprintf(log_cpu,"IndCod: %i     ----------------    IndEtiq: %i - Tamanio: %i\n",pcb->indCod,pcb->indEtiq,pcb->tamEtiq);
	log_debug_message(log_cpu);
	sprintf(log_cpu,"StackSeg: %i   ------------   TamContext: %i \n",pcb->stackSeg,pcb->tamContext);
	log_debug_message(log_cpu);
	sprintf(log_cpu,"_____________________________________________\n");
	log_debug_message(log_cpu);
	sprintf(log_cpu,"\n");
	log_debug_message(log_cpu);
	habilitar_print_console();
}


/*
 * Funcion auxiliar. Es solo para probar lo que lee
 * el analizador de metadata.
 * DESPUES BORRARLA
 */
void imprimirMetadata(t_medatada_program* metadata){
	deshabilitar_print_console();
	sprintf(log_cpu,"Cantidad de etiquetas %i\n",metadata->cantidad_de_etiquetas);
	log_debug_message(log_cpu);
	sprintf(log_cpu,"Cantidad de funciones %i\n",metadata->cantidad_de_funciones);
	log_debug_message(log_cpu);
	sprintf(log_cpu,"Tamaño del mapa serializado de etiquetas %i\n",metadata->etiquetas_size);
	log_debug_message(log_cpu);
	int i=0;
	for (i=0;i<(metadata->etiquetas_size+1);i++){
		sprintf(log_cpu,"%c",metadata->etiquetas[i]);
		log_debug_message(log_cpu);
	}
	sprintf(log_cpu,"\n");
	log_debug_message(log_cpu);
	sprintf(log_cpu,"Tamaño del mapa serializado de instrucciones %i\n",metadata->instrucciones_size);
	log_debug_message(log_cpu);
	sprintf(log_cpu,"El numero de la primera instruccion es %i\n",metadata->instruccion_inicio);
	log_debug_message(log_cpu);
	sprintf(log_cpu,"Serializado de instrucciones, puntero a instr inicio %i\n",metadata->instrucciones_serializado->start);
	log_debug_message(log_cpu);
	sprintf(log_cpu,"Serializado de instrucciones, puntero a instr offset %i\n",metadata->instrucciones_serializado->offset);
	log_debug_message(log_cpu);
	habilitar_print_console();
}

void imprimirStream(void* stream, int tamanio){
	int i=0;
	char* cadena = (char*) stream;
	for (i=0;i<tamanio;i++){
		printf("%c",cadena[i]);
	}
	printf("\n");
}

/*
 * Funcion auxiliar. Imprime el estado del stack.
 */
void imprimirStack(t_stack_c* stack){
	deshabilitar_print_console();
	int i=0;
	int max = stack->cantElem;
	for (i=0;i<max;i++){
		t_variable* variableAux = (t_variable*) desapilarC(stack);
		sprintf(log_cpu,"Desapilo la variable %c, con un valor de %i\n",variableAux->id,variableAux->valor);
		log_debug_message(log_cpu);
		sprintf(log_cpu,"La pila tiene %i elementos y el puntero final es %i\n",stack->cantElem,stack->punteroUlt);
		log_debug_message(log_cpu);
	}
	habilitar_print_console();
}


/*
 * Funcion que imprime el stack del contexto actual
 */
void imprimirStackActivo(t_stack_c* stack){
	deshabilitar_print_console();
	sprintf(log_cpu,"\n");
	log_debug_message(log_cpu);
	sprintf(log_cpu,"---------------- STACK CONTEXTO ACTUAL ------------------\n");
	log_debug_message(log_cpu);
	int inicio = stack->cursor;
	int i=0;
	int offset = 0;
	for(i=0;i<pcb.tamContext;i++){
		int valor;
		char id;
		memcpy(&id,stack->segmento+inicio+offset,sizeof(char));
		memcpy(&valor,stack->segmento+inicio+offset+1,sizeof(uint32_t));
		sprintf(log_cpu,"..... Variable id: %c ----- Variable valor: %i ------ Posicion: %i ......\n",id,valor,inicio+offset+1);
		log_debug_message(log_cpu);
		offset = offset + sizeof(t_variable);
	}
	sprintf(log_cpu,"Cursor: %i         -        Ultimo: %i \n",stack->cursor,stack->punteroUlt);
	log_debug_message(log_cpu);
	sprintf(log_cpu,"------------------------------------------------------\n");
	log_debug_message(log_cpu);
	sprintf(log_cpu,"\n");
	log_debug_message(log_cpu);
	habilitar_print_console();
}

void imprimirEstado(){
	imprimirStackActivo(stack);
	imprimirPCB(&pcb);
	imprimirDiccionario(diccionario);
}

// -------------------------------- FIN FUNC AUXILIARES -------------------------------



// --------------------------- FUNCIONES GRABAR EN UMV ---------------------------------

/*
 * Funcion que envia los datos del buffer
 * a grabarse en el UMV
 */
void grabarEnStackUMV(int socketUMV,t_PCB pcb){
	t_KER_PRO_CPU_UMV mensajeLocal = obtener_nueva_shared_str();

	mensajeLocal.gen_msg.socket_descriptor = socketUMV;
	mensajeLocal.gen_msg.socket_descriptor_server = socketUMV;
	mensajeLocal.gen_msg.id_MSJ = ENVIAR_BYTES;
	mensajeLocal.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	mensajeLocal.base_bytes = pcb.stackSeg;
	mensajeLocal.offset_bytes = 0;
	mensajeLocal.tamanio_bytes = stack->tamanio;
	mensajeLocal.buffer = (char*) malloc(mensajeLocal.tamanio_bytes);
	memcpy(mensajeLocal.buffer,stack->segmento,mensajeLocal.tamanio_bytes);
	enviarMjeSinConsola(mensajeLocal.gen_msg.socket_descriptor_server,mensajeLocal.gen_msg.id_MSJ, &mensajeLocal);
	free(mensajeLocal.buffer);

	deshabilitar_print_console();
	sprintf(log_cpu,"Envio datos a grabar al UMV. [Base,Offset,Tamanio] : %i,%i,%i",mensajeLocal.base_bytes,mensajeLocal.offset_bytes,mensajeLocal.tamanio_bytes);
	log_debug_message(log_cpu);
	habilitar_print_console();

	// Espero recepcion de datos de los indices del UMV
//	t_KER_PRO_CPU_UMV *mensaje_rec = malloc(sizeof(t_KER_PRO_CPU_UMV*));
//	mensaje_rec = recibirMjeSinConsola(socketUMV);
	t_KER_PRO_CPU_UMV *mensaje_rec = recibirMjeSinConsola(socketUMV);

	if ((mensaje_rec->gen_msg.id_MSJ == ENVIAR_BYTES)&&(mensaje_rec->OK == 1)) {
		log_info_message("Datos de stack grabados correctamente en UMV");
	} else {
		sprintf(log_cpu,"Fallo grabacion de stack final en UMV: %s",mensaje_rec->mensaje);
		log_error_message(log_cpu);
		// DESALOJO EL PROGRAMA
		desalojar();
	}
	free(mensaje_rec);
}


/*
 * Informo id de programa activo
 */
int informoProgActivo(){
	int resul = 0;
	t_KER_PRO_CPU_UMV mensajeLocal = obtener_nueva_shared_str();
	mensajeLocal.gen_msg.socket_descriptor = socketUMV;
	mensajeLocal.gen_msg.socket_descriptor_server = socketUMV;
	mensajeLocal.gen_msg.id_MSJ = CAMBIO_PROCESO_ACTIVO;
	mensajeLocal.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	mensajeLocal.PID = pcb.id;
	enviarMjeSinConsola(mensajeLocal.gen_msg.socket_descriptor_server,mensajeLocal.gen_msg.id_MSJ,&mensajeLocal);

	deshabilitar_print_console();
	sprintf(log_cpu,"Envie informacion de programa activo %i con Mje id %i",mensajeLocal.PCB.id,mensajeLocal.gen_msg.id_MSJ);
	log_debug_message(log_cpu);
	habilitar_print_console();

	// Espero confirmacion de programa activo
//	t_KER_PRO_CPU_UMV *mensaje_rec = malloc(sizeof(t_KER_PRO_CPU_UMV*));
//	mensaje_rec = recibirMjeSinConsola(socketUMV);
	t_KER_PRO_CPU_UMV *mensaje_rec = recibirMjeSinConsola(socketUMV);

	if (mensaje_rec != NULL ) {

		if ((mensaje_rec->gen_msg.id_MSJ == CAMBIO_PROCESO_ACTIVO) && (mensaje_rec->OK == 1)) {
			log_info_message("Cambio de proceso exitoso en UMV");
			resul = 1;
		} else {
			sprintf(log_cpu,"Cambio de Proc.Act fallo: %s",mensaje_rec->mensaje);
			log_error_message(log_cpu);
			string_append(&mjeError,mensaje_rec->mensaje);
			// DESALOJO EL PROGRAMA
			desalojar();
		}
	} else {
		resul = -1;
	}
	free(mensaje_rec);
	return resul;
}

// --------------------------------- FIN GRABAR EN UMV ----------------------------------



// ---------------------------- FUNCIONES DE PEDIDOS AL UMV -----------------------------

/*
 * Funcion que chequea que las posiciones a utilizar del UMV
 * correspondientes al stack segment no esten fuera de los limites.
 * Con tipo 1, solicitar bytes.
 * Con tipo 2, grabar bytes.
 */
int validarPosUMV(uint32_t base, uint32_t offset, uint32_t tamanio, uint32_t tipo){
	int resul = 0;

	t_KER_PRO_CPU_UMV mensaje = obtener_nueva_shared_str();

	mensaje.gen_msg.socket_descriptor = socketUMV;
	mensaje.gen_msg.socket_descriptor_server = socketUMV;
	mensaje.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	if (tipo == 1){
		mensaje.gen_msg.id_MSJ = SOLICITAR_BYTES;
	} else {
		mensaje.gen_msg.id_MSJ = ENVIAR_BYTES;
	}
	mensaje.PID = pcb.id;
	mensaje.identificador_cpu = idCpu;
	mensaje.base_bytes = base;
	mensaje.offset_bytes = offset;
	mensaje.tamanio_bytes = tamanio;

	deshabilitar_print_console();
	sprintf(log_cpu,"Envio solicitud validacion de datos al UMV. [Base,Offset,Tamanio] : %i,%i,%i",mensaje.base_bytes,mensaje.offset_bytes,mensaje.tamanio_bytes);
	log_debug_message(log_cpu);
	habilitar_print_console();

	enviarMjeSinConsola(mensaje.gen_msg.socket_descriptor_server,mensaje.gen_msg.id_MSJ,&mensaje);
	// Espero recepcion de confirmacion de parte del kernel
//	t_KER_PRO_CPU_UMV *pMensaje = malloc(sizeof(t_KER_PRO_CPU_UMV*));
//	pMensaje = recibirMjeSinConsola(socketUMV);
	t_KER_PRO_CPU_UMV *pMensaje = recibirMjeSinConsola(socketUMV);

	// Chequeo recepcion del valor asignado
	if ((pMensaje->gen_msg.id_MSJ == SOLICITAR_BYTES)&&(pMensaje->OK == 1)) {
		sprintf(log_cpu,"Solicitud a UMV Validada");
		log_debug_message(log_cpu);
		resul = 1;
	} else if ((pMensaje->gen_msg.id_MSJ == ENVIAR_BYTES)&&(pMensaje->OK == 1)){
		sprintf(log_cpu,"Escritura en UMV Validada");
		log_debug_message(log_cpu);
		resul = 1;
	} else {
		log_error_message("Validacion de UMV Rechazada");
		sprintf(log_cpu,"Fallo: %s",pMensaje->mensaje);
		log_error_message(log_cpu);
		// DESALOJO EL PROGRAMA
		desalojar();
		corteProg = 1; // Para desalojar el programa y no volver a ejecutarlo
		resul = -1;
	}
	free(pMensaje);
	return resul;

}


/*
 * Funcion que se verifica mediante la posicion del stack presumida
 * como segmentation fault, si es valida la posicion.
 * Con tipo 1, solicitar bytes.
 * Con tipo 2, grabar bytes.
 */
void controloPosStackUMV(uint32_t base, uint32_t offset, uint32_t tamanio, uint32_t tipo){

	t_KER_PRO_CPU_UMV mensaje = obtener_nueva_shared_str();
	mensaje.gen_msg.socket_descriptor = socketUMV;
	mensaje.gen_msg.socket_descriptor_server = socketUMV;
	mensaje.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	if (tipo == 1){
		mensaje.gen_msg.id_MSJ = SOLICITAR_BYTES;
	} else {
		mensaje.gen_msg.id_MSJ = ENVIAR_BYTES;
	}
	mensaje.PID = pcb.id;
	mensaje.identificador_cpu = idCpu;
	mensaje.base_bytes = base;
	mensaje.offset_bytes = offset;
	mensaje.tamanio_bytes = tamanio;

	deshabilitar_print_console();
	sprintf(log_cpu,"Envio solicitud validacion de datos al UMV. [Base,Offset,Tamanio] : %i,%i,%i",mensaje.base_bytes,mensaje.offset_bytes,mensaje.tamanio_bytes);
	log_debug_message(log_cpu);
	habilitar_print_console();

	enviarMjeSinConsola(mensaje.gen_msg.socket_descriptor_server,mensaje.gen_msg.id_MSJ,&mensaje);
	// Espero recepcion de confirmacion de parte del kernel
//	t_KER_PRO_CPU_UMV *pMensaje = malloc(sizeof(t_KER_PRO_CPU_UMV*));
//	pMensaje = recibirMjeSinConsola(socketUMV);
	t_KER_PRO_CPU_UMV *pMensaje = recibirMjeSinConsola(socketUMV);

	// Chequeo recepcion del valor asignado
	if ((pMensaje->gen_msg.id_MSJ == SOLICITAR_BYTES)&&(pMensaje->OK == 1)) {
		sprintf(log_cpu,"Solicitud a UMV Validada");
		log_debug_message(log_cpu);
	} else if ((pMensaje->gen_msg.id_MSJ == ENVIAR_BYTES)&&(pMensaje->OK == 1)){
		sprintf(log_cpu,"Escritura en UMV Validada");
		log_debug_message(log_cpu);
	} else {
		log_error_message("Validacion de UMV Rechazada");
		sprintf(log_cpu,"Fallo: %s",pMensaje->mensaje);
		log_error_message(log_cpu);

		// Aviso a kernel que finalizo
		mensaje.gen_msg.socket_descriptor = socketKernel;
		mensaje.gen_msg.socket_descriptor_server = socketKernel;
		mensaje.gen_msg.id_MSJ = FINALIZACION_EJECUCION_PROGRAMA;
		mensaje.identificador_cpu = idCpu;
		mensaje.PID = pcb.id;
		mensaje.PCB = pcb;
		if (senialRec == 1){
			mensaje.OK = 10;
		} else {
			mensaje.OK = 1;
				}
		mensaje.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);

		mensaje.texto = string_new();
		string_append(&mensaje.texto,pMensaje->mensaje);

		enviarMjeSinConsola(mensaje.gen_msg.socket_descriptor_server,mensaje.gen_msg.id_MSJ,&mensaje);

		solicitarDestruirSegmentos();

		// DESALOJO EL PROGRAMA
		corteProg = 1; // Para desalojar el programa y no volver a ejecutarlo
		finalizado =1;
		desalojar();

		free(mensaje.texto);
		free(pMensaje);
	}

}


int solicitarDestruirSegmentos(){
	t_KER_PRO_CPU_UMV mensaje = obtener_nueva_shared_str();

	mensaje.gen_msg.socket_descriptor = socketUMV;
	mensaje.gen_msg.socket_descriptor_server = socketUMV;
	mensaje.gen_msg.id_MSJ = DESTRUIR_SEGMENTOS;
	mensaje.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	enviarMjeSinConsola(mensaje.gen_msg.socket_descriptor_server,mensaje.gen_msg.id_MSJ, &mensaje);

	deshabilitar_print_console();
	sprintf(log_cpu,"Envio solicitud de destruccion de segmentos del Programa %i",pcb.id);
	log_debug_message(log_cpu);
	habilitar_print_console();

	// Espero recepcion de datos de los indices del UMV
//	t_KER_PRO_CPU_UMV *mensajeRec = malloc(sizeof(t_KER_PRO_CPU_UMV*));
//	mensajeRec = recibirMjeSinConsola(socketUMV);
	t_KER_PRO_CPU_UMV *mensajeRec = recibirMjeSinConsola(socketUMV);

	if ((mensajeRec->gen_msg.id_MSJ == DESTRUIR_SEGMENTOS) && (mensajeRec->OK == 1)) {
		log_info_message("Se destruyen correctamente los segmentos en el UMV");
	} else {
		sprintf(log_cpu,"Fallo solicitud de destruccion de segmentos: %s",mensajeRec->mensaje);
		log_error_message(log_cpu);
		// DESALOJO EL PROGRAMA
		desalojar();
		return -1;
	}
	free(mensajeRec);
	return 1;
}


/*
 * Funcion que pide al UMV el serializado de etiquetas
 */
char* obtenerSerializadoEtiquetas(int socketUMV,t_PCB pcb){
	t_KER_PRO_CPU_UMV mensaje = obtener_nueva_shared_str();
	// Envio de solicitud de datos al UMV
	mensaje.gen_msg.socket_descriptor = socketUMV;
	mensaje.gen_msg.socket_descriptor_server = socketUMV;
	mensaje.gen_msg.id_MSJ = SOLICITAR_BYTES;
	mensaje.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	mensaje.base_bytes = pcb.indEtiq;
	mensaje.offset_bytes = 0;
	mensaje.tamanio_bytes = pcb.tamEtiq;
	enviarMjeSinConsola(mensaje.gen_msg.socket_descriptor_server,mensaje.gen_msg.id_MSJ, &mensaje);

	deshabilitar_print_console();
	sprintf(log_cpu,"Envio solicitud de datos al UMV. [Base,Offset,Tamanio] : %i,%i,%i",mensaje.base_bytes,mensaje.offset_bytes,mensaje.tamanio_bytes);
	log_debug_message(log_cpu);
	habilitar_print_console();

	// Espero recepcion de datos de los indices del UMV
	serEtiquetas = malloc(mensaje.tamanio_bytes);

//	t_KER_PRO_CPU_UMV *mensajeRec = malloc(sizeof(t_KER_PRO_CPU_UMV*));
//	mensajeRec = recibirMjeSinConsola(socketUMV);
	t_KER_PRO_CPU_UMV *mensajeRec = recibirMjeSinConsola(socketUMV);

	if ((mensajeRec->gen_msg.id_MSJ == SOLICITAR_BYTES) && (mensajeRec->OK == 1)) {
		memcpy(serEtiquetas,mensajeRec->buffer,mensajeRec->tamanio_bytes);

		deshabilitar_print_console();
		sprintf(log_cpu,"Se recibe el serializado de etiquetas del UMV: %s",serEtiquetas);
		log_debug_message(log_cpu);
		habilitar_print_console();

	} else {
		log_error_message("No recibo los datos pedidos del UMV");
		fallaEtiquetas = 1;
		string_append(&mjeError,mensajeRec->mensaje);
	}
	free(mensajeRec);
	return serEtiquetas;
}


/*
 * Funcion mediante la cual se pide el segmento de stack,
 * para copiarlo y trabajar la estructura localmente.
 */
int obtenerStack(int socketUMV,int stackSeg, char* stackChar){
	t_KER_PRO_CPU_UMV mensaje = obtener_nueva_shared_str();

	int resul = 0;
	mensaje.gen_msg.socket_descriptor = socketUMV;
	mensaje.gen_msg.socket_descriptor_server = socketUMV;
	mensaje.gen_msg.id_MSJ = SOLICITAR_BYTES;
	mensaje.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	mensaje.base_bytes = stackSeg;
	mensaje.offset_bytes = 0;
	mensaje.tamanio_bytes = stack->tamanio;
	mensaje.buffer = (char*) malloc(mensaje.tamanio_bytes);
	enviarMjeSinConsola(mensaje.gen_msg.socket_descriptor_server,mensaje.gen_msg.id_MSJ, &mensaje);

	deshabilitar_print_console();
	sprintf(log_cpu,"Envio solicitud de datos al UMV. [Base,Offset,Tamanio] : %i,%i,%i",mensaje.base_bytes,mensaje.offset_bytes,mensaje.tamanio_bytes);
	log_debug_message(log_cpu);
	habilitar_print_console();

	// Espero recepcion de datos de los indices del UMV
//	t_KER_PRO_CPU_UMV *mensajeRec = malloc(sizeof(t_KER_PRO_CPU_UMV*));
//	mensajeRec = recibirMjeSinConsola(socketUMV);
	t_KER_PRO_CPU_UMV *mensajeRec = recibirMjeSinConsola(socketUMV);

	if (mensajeRec != NULL ) {

		if ((mensajeRec->gen_msg.id_MSJ == SOLICITAR_BYTES) && (mensajeRec->OK == 1)) {
			memcpy(stackChar,mensajeRec->buffer,mensajeRec->tamanio_bytes);
			log_info_message("Se recibe el stack desde el UMV");
			resul = 1;
		} else {
			log_error_message("No recibo los datos pedidos del stack del UMV");
			string_append(&mjeError,mensajeRec->mensaje);
			resul = -1;
		}
	} else {
		resul = -1;
	}
	free(mensaje.buffer);
	free(mensajeRec);
	return resul;
}

/*
 * Pido datos al UMV al pedir dereferenciar
 */
int solicitarDatosUMV(t_puntero direccion_variable){
	int resul = 1;
	t_KER_PRO_CPU_UMV mensaje = obtener_nueva_shared_str();
	mensaje.gen_msg.socket_descriptor = socketUMV;
	mensaje.gen_msg.socket_descriptor_server = socketUMV;
	mensaje.gen_msg.id_MSJ = SOLICITAR_BYTES;
	mensaje.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	mensaje.base_bytes = pcb.stackSeg;
	mensaje.offset_bytes = 0;
	mensaje.tamanio_bytes = direccion_variable;

	deshabilitar_print_console();
	sprintf(log_cpu,"Envio solicitud de datos al UMV. [Base,Offset,Tamanio] : %i,%i,%i",mensaje.base_bytes,mensaje.offset_bytes,mensaje.tamanio_bytes);
	log_debug_message(log_cpu);
	habilitar_print_console();

	enviarMjeSinConsola(mensaje.gen_msg.socket_descriptor_server,mensaje.gen_msg.id_MSJ, &mensaje);

	// Espero recepcion de datos de los indices del UMV
//	t_KER_PRO_CPU_UMV *pMensaje = malloc(sizeof(t_KER_PRO_CPU_UMV*));
//	pMensaje = recibirMjeSinConsola(socketUMV);
	t_KER_PRO_CPU_UMV *pMensaje = recibirMjeSinConsola(socketUMV);

	if ((pMensaje->gen_msg.id_MSJ == SOLICITAR_BYTES) && (pMensaje->OK == 1)) {
		sprintf(log_cpu,"Se recibe la sentencia: %s",sentenciaEjec);
		log_info_message(log_cpu);
		} else {
			log_error_message("----- Falla solicitud de datos al UMV-----");
			resul = 0;
			return resul;
		}
	free(pMensaje);
	return resul;
}

/*
 * Funcion mediante la cual con los indices de instruccion
 * obtenidos, se pide en el UMV la sentencia correspondiente
 * en el codigo.
 */
char* pedirSentencia(t_intructions indice){
	t_KER_PRO_CPU_UMV mensaje = obtener_nueva_shared_str();

	// Envio de solicitud de datos al UMV
	mensaje.gen_msg.socket_descriptor = socketUMV;
	mensaje.gen_msg.socket_descriptor_server = socketUMV;
	mensaje.gen_msg.id_MSJ = SOLICITAR_BYTES;
	mensaje.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	mensaje.base_bytes = pcb.codSeg;
	mensaje.offset_bytes = indice.start;
	mensaje.tamanio_bytes = indice.offset;
	enviarMjeSinConsola(mensaje.gen_msg.socket_descriptor_server,mensaje.gen_msg.id_MSJ, &mensaje);

	deshabilitar_print_console();
	sprintf(log_cpu,"Envio solicitud de datos al UMV. [Base,Offset,Tamanio] : %i,%i,%i",mensaje.base_bytes,mensaje.offset_bytes,mensaje.tamanio_bytes);
	log_debug_message(log_cpu);
	habilitar_print_console();

	// Espero recepcion de datos de los indices del UMV
	sentenciaEjec = malloc(sizeof(char)*(mensaje.tamanio_bytes));
//	t_KER_PRO_CPU_UMV *mensajeRec = malloc(sizeof(t_KER_PRO_CPU_UMV*)); // AGREGO PARA PROBAR LO DEL MEMORY CORRUPTION
//	mensajeRec = recibirMjeSinConsola(socketUMV);
	t_KER_PRO_CPU_UMV *mensajeRec = recibirMjeSinConsola(socketUMV);

	if ((mensajeRec->gen_msg.id_MSJ == SOLICITAR_BYTES) && (mensajeRec->OK == 1)) {
		memcpy(sentenciaEjec,mensajeRec->buffer,indice.offset-1);
		sentenciaEjec[indice.offset-1] = '\0';
		sprintf(log_cpu,"Se recibe la sentencia: %s",sentenciaEjec);
		log_info_message(log_cpu);

	} else {
		log_error_message("No recibo los datos pedidos del UMV");
		// DESALOJO EL PROGRAMA
		desalojar();
	}
	free(mensajeRec);
	return sentenciaEjec;
}

/*
 * Funcion mediante la cual con el valor del PC
 * se solicita el inicio y tamaño de la
 * sentencia a obtener del codigo
 */
t_intructions pedirIndInstruccion(){
	t_KER_PRO_CPU_UMV mensaje = obtener_nueva_shared_str();
	// Envio de solicitud de datos al UMV
	mensaje.gen_msg.socket_descriptor = socketUMV;
	mensaje.gen_msg.socket_descriptor_server = socketUMV;
	mensaje.gen_msg.id_MSJ = SOLICITAR_BYTES;
	mensaje.gen_msg.size_str = sizeof(t_KER_PRO_CPU_UMV);
	mensaje.base_bytes = pcb.indCod;
	mensaje.offset_bytes = (pcb.PC)*8;
	mensaje.tamanio_bytes = 8;
	enviarMjeSinConsola(mensaje.gen_msg.socket_descriptor_server,mensaje.gen_msg.id_MSJ, &mensaje);

	deshabilitar_print_console();
	sprintf(log_cpu,"Envio solicitud de datos al UMV. [Base,Offset,Tamanio] : %i,%i,%i",mensaje.base_bytes,mensaje.offset_bytes,mensaje.tamanio_bytes);
	log_debug_message(log_cpu);
	habilitar_print_console();

	// Espero recepcion de datos de los indices del UMV
	t_intructions* indice = malloc(sizeof(t_intructions));
	t_intructions resul;
//	t_KER_PRO_CPU_UMV *mensajeRec = malloc(sizeof(t_KER_PRO_CPU_UMV*));
//	mensajeRec = recibirMjeSinConsola(socketUMV);
	t_KER_PRO_CPU_UMV *mensajeRec = recibirMjeSinConsola(socketUMV);

	if ((mensajeRec->gen_msg.id_MSJ == SOLICITAR_BYTES) && (mensajeRec->OK == 1)) {
		memcpy(indice,mensajeRec->buffer,sizeof(t_intructions));
		deshabilitar_print_console();
		sprintf(log_cpu,"La instruccion tiene inicio: %i y Offset: %i",indice->start,indice->offset);
		log_debug_message(log_cpu);
		habilitar_print_console();
	} else {
		log_error_message("No recibo los datos pedidos del UMV");
		// DESALOJO EL PROGRAMA
		desalojar();
	}
	resul.start = indice->start;
	resul.offset = indice->offset;
	free(indice);
	free(mensajeRec);
	return resul;
}

// ---------------------------- FIN DE PEDIDOS AL UMV -----------------------------




//--------------------------------FUNCIONES DE EJECUCION------------------------------


/*
 * Funcion que finaliza la ejecucion del programa
 * y libera los recursos utilizados
 */
void finalizarEjecucionProceso(int socketKernel, int socketUMV){
	desconectar_de_servidor(socketKernel);
	desconectar_de_servidor(socketUMV);
	free(sentenciaEjec);
	free(serEtiquetas);
	free(diccionario);
	log_info_message("Se desconecta de servidores");
	log_info_message("-----------------------FIN DE EJECUCION---------------------------");
	finalizar_log();
	free(log_cpu);
	close_current_config();
	exit(0);
}


/*
 * Funcion que se invoca al detectar que el programa ha finalizado su ejecucion.
 * Se solicita la eliminacion de los segmentos en el UMV y se comunica al Kernel
 * que el programa ha sido finalizado
 */
void programaFinalizado(){
	sprintf(log_cpu,"Se detecto la finalizacion del programa en ejecucion");
	log_info_message(log_cpu);
	t_KER_PRO_CPU_UMV mensaje = obtener_nueva_shared_str();
	int resul = solicitarDestruirSegmentos(&mensaje);
	if (resul == 1){
		// Si destrui correctamente los segmentos aviso al Kernel
		avisoFinalizacionPrograma();
		sprintf(log_cpu,"&&&&&&&&&&&       EL PROGRAMA %i FINALIZO CORRECTAMENTE      &&&&&&&&&&&&",pcb.id);
		log_info_message(log_cpu);
	} else {
		sprintf(log_cpu,"Fallo la finalizacion del programa %i",pcb.id);
		log_error_message(log_cpu);
	}
	controlQuantum = quantum; // Establezo el quantum que se itera al total asi sale del for
	finalizado = 1;
}


/*
 * Funcion que realiza lo necesario para dar por finalizada la ejecucion de un programa
 * No por finalizar el total del programa, sino por cumplir sus quantum asignados.
 */
void quantumsFinalizados(){
	sprintf(log_cpu,"#####     Tareas de finalizacion de los quantums asignados     #####");
	log_info_message(log_cpu);
	// Mando a grabar el segmento de stack del umv con el estado resultante
	if (corteProg != 1){
		grabarEnStackUMV(socketUMV,pcb);
		// Envio al Kernel el PCB en el estado resultante
	}

	if (enviadoIO != 1){
		enviarPCBalKernel();
	}

	// FIN DE QUANTUMS o PROGRAMA DESALOJADO
	if (enviadoIO == 1){
		sprintf(log_cpu,"############## DESALOJO POR I/O ###############");
		log_error_message(log_cpu);
	} else if (desalojado == 1){
		sprintf(log_cpu,"############## PROGRAMA DESALOJADO ###############");
		log_error_message(log_cpu);
	} else if (senialRec == 1) {
		sprintf(log_cpu,"############## PROGRAMA DESALOJADO POR SEÑAL ###############");
		log_info_message(log_cpu);
	} else {
		sprintf(log_cpu,"############## FIN DE QUANTUMS ###############");
		log_info_message(log_cpu);
	}

	// Elimino la estructura de stack usada
	destruirStackC(stack);

}


/*
 * Funcion que desserializa los datos obtenidos como stack
 * desde el UMV en forma de char* y carga la pila global.
 */
void stringAStack(char* stackChar,t_stack_c* stack,int tamContex){
	int i=0;
	int size= sizeof(t_variable);
	stack->cursor = pcb.cursorStack;

	if (pcb.cursorStack != 0){
		memcpy(stack->segmento,stackChar,stack->tamanio); //Copio el stack entero
		stack->punteroUlt = stack->cursor + 5 * pcb.tamContext; // Seteo el ultimo del stack por la cantidad de variables
	} else {
		int offset=0;
		for (i=0;i<tamContex;i++){
			t_variable recVar;
			memcpy(&recVar,stackChar+offset,size);

			deshabilitar_print_console();
			sprintf(log_cpu,"Desserializada variable %i: Id %c ----- Valor %i con offset %i",i,recVar.id,recVar.valor,offset);
			log_debug_message(log_cpu);

			offset = offset+size;
			apilarC(stack,&recVar);

			sprintf(log_cpu,"Agregada la variable %i al stack",i);
			log_debug_message(log_cpu);
			habilitar_print_console();
			}
	}
	cargarDiccionario(diccionario,stack,tamContex);
}


/*
 * Funcion principal de la CPU.
 * Contiene la logica y la secuencia fundamental de ejecucion
 * de sentencias.
 */
void ejecutar(){
	t_intructions indice = pedirIndInstruccion();
	if (desalojado != 1){
		deshabilitar_print_console();
		log_debug_message("Obtenidos los punteros a la sentencia a ejecutar");
		habilitar_print_console();
		char* sentencia = pedirSentencia(indice);
		if (desalojado != 1){
			deshabilitar_print_console();
			log_debug_message("Obtenida la sentencia a ejecutar");
			habilitar_print_console();
			analizadorLinea(strdup(sentencia), &functions, &kernel_functions); //FUNCION QUE EJECUTA LAS PRIMITIVAS
		}
	}
}

//--------------------------------FIN FUNC DE EJECUCION------------------------------



// --------------- MANEJADOR DE SEÑALES -----------------------
/*
 * Manejador de señales.
 * Debe cerrar la CPU luego de finalizar la linea
 * en ejecucion
 */
void manejarSenial(int sig, siginfo_t *siginfo, void *context) {
	switch (sig) {

			case SIGINT:
				log_info_message("*** El usuario envia señalizacion de finalizacion de proceso ***");
				corteProg = 1;
				if (tengoProg == 1){
					solicitarDestruirSegmentos();
				}
				finalizarEjecucionProceso(socketKernel,socketUMV);
				exit(0);
				break;

			case SIGUSR1:

				puts("*** Se recibe señal SIGUSR1, se cierra la CPU ***");
				

printf("LLEGO SIGUSR1 y tenia programa si/no? %d se envia a socket %d\n", tengoProg,socketKernel);
				if (tengoProg == 0){
			



		t_KER_PRO_CPU_UMV msj_checksum = obtener_nueva_shared_str();
	msj_checksum.identificador_cpu = idCpu;
	msj_checksum.OK = 1;
	
	msj_checksum.PID = 0;
	enviarMjeSinConsola(socketKernel,DESCONEXION_CPU,&msj_checksum);
puts("ENVIE DESCINEXION_CPU con OK 1");
	t_KER_PRO_CPU_UMV *msj_desconectar = recibir_mensaje_de_servidor(socketKernel);
	
	if(msj_desconectar != NULL && msj_desconectar->gen_msg.id_MSJ == DESCONEXION_CPU)
	{
		puts("[CPU] SE DESCONECTO CORRECTAMENTE");

		
	}else{
		while(msj_desconectar->gen_msg.id_MSJ!=DESCONEXION_CPU){
		msj_desconectar=recibir_mensaje_de_servidor(socketKernel);
		}
		puts("CPU se desconecto Correctamente");
	}




		puts("se procede a desconectar el socket kernel ");

					desconectar_de_servidor(socketKernel);
					exit(0);
					log_info_message("Cierro proceso desde la señal por no tener programa asignado");
//					meDesconectoKernel();
					log_info_message("\n");
					log_info_message("#######################  Finalizo la ejecucion del proceso CPU por SEÑAL ######################");
					finalizarEjecucionProceso(socketKernel,socketUMV);
				}
log_info_message("*** Se recibe señal SIGUSR1, se cierra la CPU ***");
				senialRec = 1; // recibi una señal
				finProg = 0; // Finaliza la escucha de mensajes del kernel y cierra el proceso
				puts("*** PROCESO CERRADO POR SEÑAL ***");
				break;
	}
}
// --------------- FIN MANEJADOR DE SEÑALES -----------------------



// ############################ MAIN DE LA APLICACION ###################################

/*
 * Proceso CPU.
 * Posee conexiones para recepcion y envio de datos
 * con los procesos Kernel y UMV
 */

int main(void){

	idCpu = 50; //Asigno id a la Cpu. 50 es que no fue asignado su valor
	tengoProg = 0; // No tengo programa asignado

	// Genero el nombre de archivo de log que contiene la fecha de ejecucion
	log_cpu = malloc(sizeof(char)*500);
	char *cpu = string_new();
	string_append(&cpu, "CPU - ");
	string_append(&cpu,temporal_get_string_time());
	string_append(&cpu,".log");
	inicializar_log(cpu, "cpu");
	free(cpu);
	// Log generado, un archivo por ejecucion

	habilitar_log_debug();
	habilitar_log_error();
	habilitar_log_trace();

	log_info_message("---------------------INICIO DE NUEVA EJECUCION---------------------------");

	//Leo archivo de configuracion
	open_config(PATH_CONFIG);
	port_kernel = puertoKernel();
	port_umv = puertoUMV();
	ip_kernel = ipKernel();
	ip_umv = ipUMV();

	deshabilitar_print_console();
	log_debug_message("Cargada la configuracion");
	habilitar_print_console();

	//Mutex que controlan estado de conexion con Kernel y UMV.
	pthread_mutex_t mutex_kernel = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t mutex_umv = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&mutex_kernel);
	pthread_mutex_lock(&mutex_umv);

	signal(SIGUSR1, (void*) manejarSenial);
	signal(SIGINT, (void*) manejarSenial);

	diccionario = malloc(sizeof(t_dicVar));

	//------------------------------------------------------------------------

	//Se conecta la CPU al Kernel
	socketKernel = conectar_a_servidor(ip_kernel, port_kernel);
	if (socketKernel < 0) {
		sprintf(log_cpu,"ERROR: No se puede establecer conexion con el kernel");
		log_error_message(log_cpu);
		return EXIT_FAILURE;
	} else {
		sprintf(log_cpu,"Establecida conexion con el Kernel \n Ip: %s. Puerto: %i - Socket %i",ip_kernel,port_kernel,socketKernel);
		log_info_message(log_cpu);
	}


	// Envio de handshake al Kernel
	t_KER_PRO_CPU_UMV mensajeAEnviar = obtener_nueva_shared_str();
	mensajeAEnviar.gen_msg.id_MSJ= HANDSHAKE_CPU_PCP;
	mensajeAEnviar.gen_msg.socket_descriptor = socketKernel;
	mensajeAEnviar.gen_msg.socket_descriptor_server = socketKernel;
	enviarMjeSinConsola(socketKernel,mensajeAEnviar.gen_msg.id_MSJ,&mensajeAEnviar);

	deshabilitar_print_console();
	sprintf(log_cpu,"Envio de handshake al Kernel de tipo %i por socket %i",mensajeAEnviar.gen_msg.id_MSJ,socketKernel);
	log_debug_message(log_cpu);
	habilitar_print_console();

	// Chequeo respuesta de handshake de parte del kernel y desbloqueo mutex.
//	t_KER_PRO_CPU_UMV *mensajeRec = malloc(sizeof(t_KER_PRO_CPU_UMV*));
//	mensajeRec = recibirMjeSinConsola(socketKernel);
	t_KER_PRO_CPU_UMV *mensajeRec = recibirMjeSinConsola(socketKernel);

	if (mensajeRec != NULL ) {

		if ((mensajeRec->gen_msg.id_MSJ == HANDSHAKE_CPU_PCP) && (mensajeRec->OK == 1)) {

			sprintf(log_cpu,"Recepcion de handshake de Kernel de tipo %i por socket %i",mensajeRec->gen_msg.id_MSJ,socketKernel);
			log_info_message(log_cpu);
			idCpu = mensajeRec->identificador_cpu;
			sprintf(log_cpu,"ASIGNO ID DE CPU %i",idCpu);
			log_debug_message(log_cpu);

			pthread_mutex_unlock(&mutex_kernel);
		} else {
			sprintf(log_cpu,"Error en handshake Kernel: %s",mensajeRec->mensaje);
			log_error_message(log_cpu);
		}

	} else {
		sprintf(log_cpu,"ERROR: No recibo Handshake del kernel");
		log_error_message(log_cpu);
		return EXIT_FAILURE;
	}
	free(mensajeRec);

	//------------------------------------------------------------------------

	//Se conecta la CPU al UMV
	socketUMV = conectar_a_servidor(ip_umv, port_umv);
	if (socketUMV < 0) {
		sprintf(log_cpu,"ERROR: No se puede establecer conexion con el UMV");
		log_error_message(log_cpu);
		return EXIT_FAILURE;
	} else {
		sprintf(log_cpu,"Establecida conexion con el UMV \n Ip: %s. Puerto: %i - Socket %i",ip_umv,port_umv,socketUMV);
		log_info_message(log_cpu);
	}

	// Envio de handshake al UMV
	t_KER_PRO_CPU_UMV mjeUMV = obtener_nueva_shared_str();
	mjeUMV.gen_msg.id_MSJ = HANDSHAKE_CPU_UMV;
	mjeUMV.gen_msg.socket_descriptor = socketUMV;
	mjeUMV.gen_msg.socket_descriptor_server = socketUMV;
	mjeUMV.identificador_cpu = idCpu;
	enviarMjeSinConsola(socketUMV,mjeUMV.gen_msg.id_MSJ,&mjeUMV);

	deshabilitar_print_console();
	sprintf(log_cpu,"Envio de handshake al UMV de tipo %i por socket %i",mensajeAEnviar.gen_msg.id_MSJ,socketUMV);
	log_debug_message(log_cpu);
	habilitar_print_console();

	// Chequeo respuesta de handshake de parte del umv y desbloqueo mutex.
//	t_KER_PRO_CPU_UMV *mensajeRecUMV = malloc(sizeof(t_KER_PRO_CPU_UMV*));
//	mensajeRecUMV = recibirMjeSinConsola(socketUMV);
	t_KER_PRO_CPU_UMV *mensajeRecUMV = recibirMjeSinConsola(socketUMV);

	if (mensajeRecUMV != NULL ) {

		if ((mensajeRecUMV->gen_msg.id_MSJ == HANDSHAKE_CPU_UMV) && (mensajeRecUMV->OK == 1)){

			sprintf(log_cpu,"Recepcion de handshake de UMV de tipo %i por socket %i",mensajeRecUMV->gen_msg.id_MSJ,socketUMV);
			log_info_message(log_cpu);

			pthread_mutex_unlock(&mutex_umv);
		} else {
			sprintf(log_cpu,"Error en handshake UMV: %s",mensajeRecUMV->mensaje);
			log_error_message(log_cpu);
		}
	} else {
		sprintf(log_cpu,"ERROR: No recibo Handshake del UMV");
		log_error_message(log_cpu);
		return EXIT_FAILURE;
	}
	free(mensajeRecUMV);

	//------------------------------------------------------------------------

	finProg = 1;
	sleep(1);

	pthread_mutex_lock(&mutex_kernel);
	deshabilitar_print_console();
	log_debug_message("Se desbloqueo el semaforo del kernel");
	pthread_mutex_lock(&mutex_umv);
	log_debug_message("Se desbloqueo el semaforo del umv");
	habilitar_print_console();

	senialRec = 0;

	while(finProg){

		sprintf(log_cpu," ---- CPU %i ESPERANDO PCB PARA EJECUCION ---- ",idCpu);
		log_info_message(log_cpu);

		int control = 0;
		fallaEtiquetas = 0;
		finalizado = 0;
		desalojado = 0;

		// Espero mensaje de envio de PCB
//		t_KER_PRO_CPU_UMV *mensajeWhile = malloc(sizeof(t_KER_PRO_CPU_UMV*));
//		mensajeWhile = recibirMjeSinConsola(socketKernel);
		t_KER_PRO_CPU_UMV *mensajeWhile = recibirMjeSinConsola(socketKernel);

		if (mensajeWhile == NULL){
			sprintf(log_cpu," #### Fallo: El socket del Kernel se ha cerrado ###");
			log_error_message(log_cpu);
			finalizarEjecucionProceso(socketKernel,socketUMV);
		}

		deshabilitar_print_console();
		sprintf(log_cpu,"Se recibe mensaje de tipo %i por socket %i",mensajeWhile->gen_msg.id_MSJ,socketKernel);
		log_debug_message(log_cpu);
		habilitar_print_console();

		switch (mensajeWhile->gen_msg.id_MSJ){

		// Se recibe un PCB de parte del Kernel
		case (EJECUTAR_PROGRAMA):
						tengoProg = 1; // Tengo programa asignado
						log_info_message("-  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  -");
						sprintf(log_cpu,"Se recibe un PCB para iniciar ejecucion de Programa");
						log_info_message(log_cpu);
												
						

						pcb = mensajeWhile->PCB;
						quantum = mensajeWhile->quantum;
						stackChar = malloc(pcb.stack_size);
						mjeError = string_new();

						sprintf(log_cpu,"Ha sido asignado el programa %i a ejecutar",pcb.id);
						log_info_message(log_cpu);

						// Comunico al UMV el programa que tengo activo
						control = informoProgActivo();
						if (control == 1){
							deshabilitar_print_console();
							sprintf(log_cpu,"Informo Proceso %i activo al UMV por socket %i",pcb.id,socketUMV);
							log_debug_message(log_cpu);
							habilitar_print_console();

							log_info_message("Informado al UMV el programa actual en ejecucion");

							stack = crearStackC(pcb.stack_size);

							// Limpio el diccionario de variables
							inicializarDiccionario(diccionario);
							deshabilitar_print_console();
							log_debug_message("Inicializado el diccionario de variables");
							habilitar_print_console();

							// Solicito el STACK del UMV. Lo recibo y creo el diccionario de variables
							control = obtenerStack(socketUMV,pcb.stackSeg,stackChar);

							if (control == 1){

								stringAStack(stackChar,stack,pcb.tamContext);

								deshabilitar_print_console();
								sprintf(log_cpu,"Recibido el segmento de stack");
								log_debug_message(log_cpu);
								sprintf(log_cpu,"Convertido a tipo stack. Tamanio del contexto: %i",pcb.tamContext);
								log_debug_message(log_cpu);
								habilitar_print_console();

								log_info_message("Recibido el segmento de stack del programa");
								log_info_message("-  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  -\n\n");
								// Hasta aca la recepcion del stack segment

								// Solicito el serializado de etiquetas, si es mayor a 0
								if (pcb.tamEtiq > 0){
									serEtiquetas = obtenerSerializadoEtiquetas(socketUMV,pcb);
									deshabilitar_print_console();
									sprintf(log_cpu,"Obtenido el serializado de etiquetas, de largo %i",pcb.tamEtiq);
									log_debug_message(log_cpu);
									habilitar_print_console();
								} else {
									deshabilitar_print_console();
									sprintf(log_cpu,"El serializado de etiquetas tiene tamanio 0");
									log_debug_message(log_cpu);
									habilitar_print_console();
								}

								if (fallaEtiquetas == 0){

									enviadoIO=0;
									controlQuantum=0;
									corteProg=0;

									int qTotal = mensajeWhile->quantum;

									for (controlQuantum=0;controlQuantum<qTotal;controlQuantum++){

										desalojado = 0;
										saltoLinea = 0;

										imprimirEstado(); // SOLO PARA VISUALIZAR EN LOG

										sprintf(log_cpu,"------- Ejecucion de Quantum nro %i ---------",controlQuantum+1);
										log_info_message(log_cpu);
										sprintf(log_cpu,"  PC: %i       -     Contexto: %i  ",pcb.PC,pcb.tamContext);
										log_info_message(log_cpu);

										deshabilitar_print_console();
										sprintf(log_cpu,"Iniciado ciclo de ejecucion de programa %i. Comunicacion con UMV por socket %i",pcb.id,socketUMV);
										log_debug_message(log_cpu);
										habilitar_print_console();

										ejecutar();

										if (finalizado == 0){

											deshabilitar_print_console();
											// Comunico al Kernel que finalice la ejecucion de un quantum
											log_debug_message("Fin de quantum, lo comunico al kernel");
											habilitar_print_console();

											if (saltoLinea != 1){
												pcb.PC++; // Incremento el PC
											}

											if (desalojado != 1){
												enviarFinQuantumUnit(); //Aviso finalizacion unitaria de quantum
											}

										}

										usleep(mensajeWhile->retardo);

									}

									// REALIZO TAREAS NECESARIAS POR LA FINALIZACION CORRECTA DE LOS QUANTUMS ASIGNADOS
									// Esta funcion comunica al Kernel la finalizacion de ejecucion y su nueva disponibilidad
									if (finalizado == 0){
										quantumsFinalizados();
									}

								} else {
									// FALLA AL OBTENER ETIQUETAS
									informoErrorYCierre();
								}

							} else {
								// FALLA OBTENER STACK
								informoErrorYCierre();
							}
						} else {
							//FALLA INFORME DE PROGRAMA ACTIVO
							informoErrorYCierre();
						}

						free(mjeError);
						free(stackChar);
						tengoProg = 0;
//						puts("################### ESTOY DESALOJADOOOOOOOOOOOOOOO #######################");
						pcb.id = 0; //Cuando dejo de tener un programa, el id de programa pasa a ser -1
						break;

		default:
			sprintf(log_cpu,"Mensaje desconocido: Tipo %i de socket %i",mensajeWhile->gen_msg.id_MSJ,mensajeWhile->gen_msg.socket_descriptor);
			log_error_message(log_cpu);
			break;

		}
		free(mensajeWhile);
	}

	//------------------------------------------------------------------------

//	meDesconectoKernel();
	log_info_message("\n");
	if (senialRec == 1){
		log_info_message("#######################  Finalizo la ejecucion del proceso CPU por SEÑAL ######################");


	t_KER_PRO_CPU_UMV msj_checksum = obtener_nueva_shared_str();
	msj_checksum.identificador_cpu = idCpu;
	msj_checksum.OK = 1;
	
	msj_checksum.PID = 0;
	enviarMjeSinConsola(socketKernel,DESCONEXION_CPU,&msj_checksum);
puts("ENVIE DESCINEXION_CPU con OK 1");
	t_KER_PRO_CPU_UMV *msj_desconectar = recibir_mensaje_de_servidor(socketKernel);
	
	if(msj_desconectar != NULL && msj_desconectar->gen_msg.id_MSJ == DESCONEXION_CPU)
	{
		puts("[CPU] SE DESCONECTO CORRECTAMENTE");

		
	}else
	{
		while(msj_desconectar->gen_msg.id_MSJ!=DESCONEXION_CPU){
		msj_desconectar=recibir_mensaje_de_servidor(socketKernel);
		}
		puts("CPU se desconecto Correctamente");
	}

	} else {
		log_info_message("#######################  Finalizo la ejecucion del proceso CPU ######################");
	}

	finalizarEjecucionProceso(socketKernel,socketUMV);

	return 1;
}


// ################################      F  I  N       ###################################
