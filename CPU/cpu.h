/*
 * cpu.h
 *
 *  Created on: 22/04/2014
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include <parser/metadata_program.h>
#include <commons/collections/stack.h>
#include <commons/shared_strs.h>
#include <signal.h>

#define MAXVARIABLES 100

typedef struct dicVariables { // estructura generica, debe estar en todas las estructuras
	char variable[MAXVARIABLES];
	int offset[MAXVARIABLES];
	int ultimo;
} t_dicVar;

// ---------------- VARIABLES GLOBALES PARA QUE PUEDAN SER ACCEDIDAS DESDE PRIMITIVAS ---------

t_stack_c* stack; // Tipo pila para manejo del stack
t_dicVar* diccionario; // Diccionario de variables del stack
char* serEtiquetas; // Sentencia que se ejecutara
t_PCB pcb; // PCB recibido para ejecucion
char* log_cpu; // Variable para logueo de ejecucion
int saltoLinea; // Se cambia a 1 si las primitivas provocan un salto en el codigo, para no aumentar el pc
int desalojado; // Es 1 si desaloja el programa en vez de terminar quantum
int controlQuantum; // Controla el numero de quantums ejecutados
int quantum; // Es el numero de quantums a ejecutar
int corteProg; // Se cambia a 1 para determinar que se desaloja el programa y no se vuelve a ejecutar

// -----------------------------------------------------------------------------------------

void finalizarEjecucionProceso(int socketKernel, int socketUMV);

void inicializarDiccionario(t_dicVar* diccionario);

void indexarEnDiccionario(t_dicVar* diccionario, char id, int posicion);

int obtenerPosicionDic(t_dicVar* diccionario, char id);

void programaFinalizado();

void desalojar();

void cargarDiccionario(t_dicVar* diccionario, t_stack_c* stack, int tamContext);

void imprimirPCB(t_PCB* pcb);

void imprimirMetadata(t_medatada_program* metadata);

void imprimirStack(t_stack_c* stack);

void grabarEnStackUMV(int socketUMV, t_PCB pcb);

int informoProgActivo();

char* obtenerSerializadoEtiquetas(int socketUMV,t_PCB pcb);

int obtenerStack(int socketUMV,int stackSeg,char* stackChar);

char* pedirSentencia(t_intructions indice);

t_intructions pedirIndInstruccion();

void stringAStack(char* stackChar,t_stack_c* stack,int tamContex);

int enviarIOalKernel(t_nombre_dispositivo dispositivo, int tiempo);

void enviarPCBalKernel();

void hacerWaitSemaforo(t_nombre_semaforo identificador_semaforo);

void hacerSignalSemaforo(t_nombre_semaforo identificador_semaforo);

t_valor_variable pedirValorGlobal(t_nombre_compartida variable);

t_valor_variable asignarValorGlobal(t_nombre_compartida variable, t_valor_variable valor);

int solicitarDestruirSegmentos();

void avisoFinalizacionPrograma();

void ejecutar();

void manejarSenial(int sig, siginfo_t *siginfo, void *context);

//void enviarMensajeAKernel(char* mensaje, t_valor_variable valor, int idMje, uint32_t largo);
void enviarMensajeAKernel(char* mensaje, t_valor_variable valor, int idMje);

t_puntero_instruccion buscarEtiqueta(t_nombre_etiqueta etiq, char* indEtiq,t_size tamEtiq);

void controloPosStackUMV(uint32_t base, uint32_t offset, uint32_t tamanio, uint32_t tipo);


#endif /* CPU_H_ */
