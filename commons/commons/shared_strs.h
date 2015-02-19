
#ifndef _SHAREDSTRSH_
#define _SHAREDSTRSH_
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>


#define BUFFER_SIZE 512

// USAR ESTOS MENSAJES
#define HANDSHAKE_PROGRAMA_PLP 1
#define HANDSHAKE_CPU_PCP 2
#define HANDSHAKE_CPU_UMV 3
#define HANDSHAKE_PLP_UMV 4
#define IMPRIMIR_TEXTO 5
#define IMPRIMIR 6
#define CREAR_SEGMENTOS 7
#define EJECUTAR_PROGRAMA 8
#define OBTENER_VALOR 9
#define GRABAR_VALOR 10
#define WAIT 11
#define SIGNAL 12
#define ENTRADA_SALIDA 13
#define SOLICITAR_BYTES 14
#define ENVIAR_BYTES 15
#define SOLICTUD_INSTRUCCION 16
#define CAMBIO_PROCESO_ACTIVO 17
#define FINALIZAR_EJECUCION 18
#define DESTRUIR_SEGMENTOS 19
#define NOTIFICACION_QUANTUM 20
#define ENTRADA_INDICE 21
#define FINALIZACION_EJECUCION_PROGRAMA 22
#define DESCONEXION_CPU 23

/*
 * Estructuras utilizadas para el manejo y apilado
 * del stack.
 */
typedef struct VARIABLE{
	char id;
	uint32_t valor;
} __attribute__ ((__packed__)) t_variable;

typedef struct strGenMsg { // estructura generica, debe estar en todas las estructuras
	uint32_t id_MSJ;
	uint32_t disconnected;
	uint32_t socket_descriptor;
	uint32_t socket_descriptor_server;
	uint32_t size_str;
} __attribute__ ((__packed__)) t_GEN_MSG;

typedef struct strPCB { // Estructura que representa el PCB
	uint32_t id; //Nro identificador del proceso
	uint32_t codSeg; //Referencia al code segment
	uint32_t stackSeg; //Referencia al stack segment
	uint32_t cursorStack; //Referencia al cursor del stack
	uint32_t indCod; //Referencia al indice de codigo
	uint32_t indEtiq; //Referencia al indice de etiquetas
	uint32_t tamEtiq; //Tamanio del serializado de etiquetas
	uint32_t PC; //Program Counter
	uint32_t tamContext; //Tamanio del contexto actual
	uint32_t stack_size; //Tamanio del stack
} __attribute__ ((__packed__)) t_PCB;

typedef struct strPedidoMem { // Estructura que representa la solicitud de memoria al UMV
	int base; // Base de inicio de la memoria
	int offset; // Desplazamiento desde la base
	int tamanio; // tamanio de datos a solicitar
	char buffer[BUFFER_SIZE]; //Se utiliza para colocar los datos de respuesta cuando contesta la UMV

} __attribute__ ((__packed__)) t_solicitud;

typedef struct mjePedidoDatos{ //Estructura que contiene un PCB internamente a ser enviado desde el kernel
	t_GEN_MSG gen_msg;
	t_solicitud solicitud;
}__attribute__ ((__packed__)) t_UMV_pedido;

typedef struct strKernelProgramaCPUUMV {
	t_GEN_MSG gen_msg;

	//HANDSHAKE_PROGRAMA_PLP y HANDSHAKE_CPU_PCP
	uint32_t identificador_cpu; //(identificador propio de CPU)
	//HANDSHAKE_PROGRAMA_PLP y ENVIO_COD_INS_ETI
	uint32_t codigo_long;
	char* codigo; //Ansisop
	//IMPRIMIR - IMPRIMIR_TEXTO
	uint32_t texto_long;
	char* texto;
	//CREAR_SEGMENTOS
	uint32_t tamanio_segmento;
	uint32_t base_segmento;

	//EJECUTAR_PROGRAMA
	uint32_t quantum;
	uint32_t retardo;
	//OBTENER_VALOR y GRABAR_VALOR
	uint32_t id_var_comp_long;
	char* id_var_comp;
	int valor;
	//WAIT y SIGNAL
	uint32_t id_sem_long;
	char* id_sem;
	//ENTRADA_SALIDA
	uint32_t id_dispositivo_long;
	char* id_dispositivo;
	uint32_t utilizacion;
	//SOLICITAR_BYTES y ENVIAR_BYTES
	uint32_t  base_bytes;
	uint32_t  offset_bytes;
	uint32_t  tamanio_bytes;
	//SOLICITAR_BYTES, ENVIAR_BYTES y SOLICTUD_INSTRUCCION
	uint32_t buffer_long;
	char* buffer;
	//EJECUTAR_PROGRAMA
	t_PCB PCB;
	//HANDSHAKE_PROGRAMA_PLP, HANDSHAKE_CPU_PCP, HANDSHAKE_CPU_UMV, CAMBIO_PROCESO_ACTIVO, FINALIZAR_EJECUCION, DESTRUIR_SEGMENTOS, NOTIFICACION_QUANTUM
	uint32_t PID;
	//ENTRADA_INDICE
	uint32_t  indice;
	// Proposito Gral
	uint32_t OK;
	uint32_t mensaje_long;
	char* mensaje;

}__attribute__ ((__packed__)) t_KER_PRO_CPU_UMV;

char* id_a_mensaje(int id);
void calcular_tamanio_shared_str(t_KER_PRO_CPU_UMV *shared_str);
char* serializar_shared_str(t_KER_PRO_CPU_UMV *shared_str);
int deserializar_buffer_a_shared_str(t_KER_PRO_CPU_UMV* shared_str, int socket_cliente);
t_KER_PRO_CPU_UMV obtener_nueva_shared_str();
#endif
