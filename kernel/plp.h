#include "glb_header_kernel.h"
#include <commons/tad_server.h>
#include <commons/tad_cliente.h>
#include <commons/shared_strs.h>
#include <commons/log_app.h>
#include <commons/string.h>
#include <commons/sync.h>
#include <parser/metadata_program.h>
void * lanzar_plp (t_PLP *plp);

t_KER_PRO_CPU_UMV* (*handler_plp_msgs_ptr)(t_KER_PRO_CPU_UMV*);
void callback_programa_finalizado();
void handshakeInicialUMV(int socketUMV);
void enviarBytes(t_PCB pcb,char* codigo,t_medatada_program* metadata);
void imprimirMetadata(t_medatada_program* metadata);
t_PCB crear_segmentos(t_medatada_program* metaData,int lenCod, t_Programa* programa);
void imprimirStream(void* stream, int tamanio);
int calcularJob(t_medatada_program* metaData, char* codigo);
int calcularLineas(char* codigo);

