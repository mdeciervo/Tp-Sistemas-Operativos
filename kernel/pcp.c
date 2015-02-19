#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <commons/tad_server.h>
#include <commons/shared_strs.h>
#include <commons/log_app.h>
#include <commons/string.h>
#include <commons/sync.h>
#include "pcp.h"
#include "print_kernel.h"

extern t_PCP* pcp;

uint32_t obtener_PID_cpu() {
	pcp->ultimo_PID_cpu++;
	return pcp->ultimo_PID_cpu;
}

void encolar_cpu(t_CPU* cpu) {

	void productor_consumidor_cpu_post() {
		syncqueue_push(pcp->CPUs->planif_queue, cpu);
		con_vals.cpus_tot =
				pcp->CPUs->planif_queue->queue->elements->elements_count;
		print_estado();
	}
	syncqueue_mutex_cont_post(pcp->CPUs, (void*) productor_consumidor_cpu_post);
}

void encolar_req_io(t_REQ_IO* req_io, char* id_disp) {
	char log_text[500];

	t_sync_queue_mutex_cont* reqs;
	if (dictionary_has_key(pcp->reqs_ios, id_disp))
		reqs = dictionary_get(pcp->reqs_ios, id_disp);
	else {
		//begin log
		sprintf(log_text,

		"[IO] No se encontro el dispositivo con id %s", id_disp);
		log_error_message(log_text);
		//end log

		exit(EXIT_FAILURE);
	}

	void productor_consumidor_req_io_post() {
		syncqueue_push(reqs->planif_queue, req_io);
	}
	syncqueue_mutex_cont_post(reqs, (void*) productor_consumidor_req_io_post);
}

t_REQ_IO* obtener_req_io(t_sync_queue_mutex_cont* reqs) {

	void* productor_consumidor_req_io_wait() {
		return syncqueue_pop(reqs->planif_queue);
	}
	return syncqueue_mutex_cont_wait(reqs,
			(void*) productor_consumidor_req_io_wait);
}

t_CPU* obtener_cpu() {

	void* productor_consumidor_cpu_wait() {
		t_CPU* cpu = syncqueue_pop(pcp->CPUs->planif_queue);

		return cpu;
	}
	t_CPU* cpu_disponible = syncqueue_mutex_cont_wait(pcp->CPUs,
			(void*) productor_consumidor_cpu_wait);

	synclist_add(pcp->CPUs_en_uso, cpu_disponible);

	return cpu_disponible;
}

void eliminar_cpu_en_uso(uint32_t identificador_cpu) {

	bool mismo_pid(t_CPU *cpu) {
		return cpu->id == identificador_cpu;
	}
	t_CPU* cpu = synclist_remover(pcp->CPUs_en_uso, (void*) mismo_pid);

	if (cpu != NULL )
		free(cpu);

}

void eliminar_cpu(uint32_t identificador_cpu) {
	void* productor_consumidor_cpu_wait() {
		bool mismo_pid(t_CPU *cpu) {
			return cpu->id == identificador_cpu;
		}
		return syncqueue_remover(pcp->CPUs->planif_queue, (void*) mismo_pid);
	}
log_info_message("[PCP] Se hace wait de prod cons de CPU porque se elimina");
	t_CPU* cpu = syncqueue_mutex_cont_wait(pcp->CPUs,
			(void*) productor_consumidor_cpu_wait);
	con_vals.cpus_tot =
			pcp->CPUs->planif_queue->queue->elements->elements_count;
log_info_message("[PCP] Se hizo wait de prod cons de CPU porque se elimina");
	print_estado();
	if (cpu != NULL )
		free(cpu);

}

void liberar_cpu_en_uso(uint32_t identificador_unico) {

	void productor_consumidor_cpu_post() {
		bool mismo_pid(t_CPU *cpu) {
			if (cpu->id == identificador_unico) {
				cpu->programa_en_ejecucion = NULL;
				return true;
			}
			return false;
		}
		synclist_mover_item_de_lista_a_cola(pcp->CPUs_en_uso,
				pcp->CPUs->planif_queue, (void*) mismo_pid);
	}

	syncqueue_mutex_cont_post(pcp->CPUs, (void*) productor_consumidor_cpu_post);

}

t_CPU* obtener_cpu_por_socket_server(int socket_server) {

	bool mismo_socket_server(t_CPU *cpu) {
		return cpu->socket_server == socket_server;
	}
	t_CPU* cpu;
	log_info_message("[PCP] Busco CPU en uso por socket");
	cpu = synclist_find(pcp->CPUs_en_uso, (void*) mismo_socket_server);

	log_info_message("[PCP] Busque CPU en uso por socket");
	if (cpu == NULL ){

	log_info_message("[PCP] No se encontro CPU en uso por socket. Busco iddle");
		cpu = syncqueue_find(pcp->CPUs->planif_queue,
				(void*) mismo_socket_server);

	log_info_message("[PCP] Busque CPU iddle por socket");
	}
	if(cpu==NULL)
		log_info_message("[PCP] CPU por socket no se encontro");
	return cpu;
}

t_Programa* obtener_proc_de_exit(uint32_t pid) {
	bool mismo_pid(t_Programa *programa) {
		return programa->PID == pid;
	}
	return syncqueue_find(pcp->exit, (void*) mismo_pid);

}
t_Programa* obtener_proc_de_block(uint32_t pid) {
	bool mismo_pid(t_Programa *programa) {
		return programa->PID == pid;
	}
	return syncqueue_find(pcp->block, (void*) mismo_pid);

}
t_Programa* obtener_proc_de_exec(uint32_t pid) {
	char log_text[500];

	bool mismo_pid(t_Programa *programa) {
		return programa->PID == pid;
	}
	t_Programa* programa = syncqueue_find(pcp->exec, (void*) mismo_pid);

	if (programa == NULL ) {
		sprintf(log_text,
				"[PCP] Se quizo obtener Programa %d de exec pero no se encontro. Seguramente esta bloqueado/finalizado y CPU mando mensaje de todas formas",
				pid);
		log_error_message(log_text);

		exit(0);
	}

	return programa;
}
t_Programa* obtener_proc_de_ready() {

	void* productor_consumidor_ready_wait() {
		return syncqueue_pop(pcp->ready->planif_queue);
	}
	t_Programa* programa = syncqueue_mutex_cont_wait(pcp->ready,
			(void*) productor_consumidor_ready_wait);

	return programa;
}
t_Programa* estado_cp_proc_ready_a_exec(t_Programa* programa) {

	programa->cq = 'X';
	print_estado();

	syncqueue_push(pcp->exec, programa);

	return programa;
}
void estado_cp_proc_exec_a_ready(uint32_t pid, t_PCB pcb) {
	char log_text[500];
	void productor_consumidor_ready_post() {
		bool mismo_pid(t_Programa *programa) {
			if (programa->PID == pid) {
				programa->PCB = pcb;
				programa->cq = 'R';
				programa->cpu = 0;
				print_estado();
				return true;
			}
			return false;
		}
		bool pudo_mover = syncqueue_mover_item_entre_colas(pcp->exec,
				pcp->ready->planif_queue, (void*) mismo_pid);
		if (!pudo_mover) {
			sprintf(log_text,
					"[PCP] Se quizo mover Programa %d de exec a ready pero no se encontro. Seguramente esta bloqueado y CPU mando finalizar ejecucion",
					pid);
			log_error_message(log_text);

			exit(0);
		}
	}
	syncqueue_mutex_cont_post(pcp->ready,
			(void*) productor_consumidor_ready_post);
}
void estado_cp_proc_block_a_ready(uint32_t pid) {
	void productor_consumidor_ready_post() {
		bool mismo_pid(t_Programa *programa) {

			if (programa->PID == pid) {
				programa->cq = 'R';
				programa->cpu = 0;
				print_estado();
				return true;
			}
			return false;

		}
		syncqueue_mover_item_entre_colas(pcp->block, pcp->ready->planif_queue,
				(void*) mismo_pid);
	}
	syncqueue_mutex_cont_post(pcp->ready,
			(void*) productor_consumidor_ready_post);
}
void estado_cp_proc_exec_a_exit(uint32_t pid, t_PCB pcb) {

	bool mismo_pid(t_Programa *programa) {
		if (programa->PID == pid) {
			programa->PCB = pcb;
			programa->cq = 'E';
			programa->cpu = 0;
			print_estado();
			return true;
		}
		return false;
	}
	syncqueue_mover_item_entre_colas(pcp->exec, pcp->exit, (void*) mismo_pid);

	pcp->callback_programa_finalizado();

}
void estado_cp_proc_exec_a_block(uint32_t pid, t_PCB pcb) {
	bool mismo_pid(t_Programa *programa) {
		if (programa->PID == pid) {
			programa->PCB = pcb;
			programa->cq = 'B';
			programa->cpu = 0;
			print_estado();
			return true;
		}
		return false;
	}
	syncqueue_mover_item_entre_colas(pcp->exec, pcp->block, (void*) mismo_pid);
}

//Callbacks comienzo
void callback_io(t_REQ_IO* req) {
	char log_text[500];
	sprintf(log_text,
			"[IO] finaliza ejecucion %s con utilizacion %d solcitado por CPU %d para programa %d",
			req->id, req->utilizacion, req->identificador_cpu, req->PID);
	log_info_message(log_text);
	estado_cp_proc_block_a_ready(req->PID);
}

void callback_wakeup_process(t_req_sem_pcp* req_sem) {
	char log_text[500];
	sprintf(log_text,
			"[PCP] WAIT-WAKEUP de semaforo %s solcitado por CPU %d para programa %d",
			req_sem->sem->id_sem, req_sem->identificador_cpu, req_sem->PID);
	log_info_message(log_text);

	estado_cp_proc_block_a_ready(req_sem->PID);
}

t_KER_PRO_CPU_UMV* handler_pcp_msgs(t_KER_PRO_CPU_UMV* msg) {
	char log_text[500];
	if (msg != NULL ) {
		t_GEN_MSG gen_msg = msg->gen_msg;
		if (!gen_msg.disconnected) {
			switch (gen_msg.id_MSJ) {
			case HANDSHAKE_CPU_PCP: {

				log_info_message("[CPU] HANDSHAKE CON PCP");

				t_CPU* cpu = malloc(sizeof(t_CPU));

				cpu->id = obtener_PID_cpu();
				cpu->socket_server = msg->gen_msg.socket_descriptor_server;
				cpu->programa_en_ejecucion = NULL;

				sprintf(
						identidad_socket_server[msg->gen_msg.socket_descriptor_server],
						"CPU %d", cpu->id);
//				puts(identidad_socket_server[msg->gen_msg.socket_descriptor_server]);

				t_KER_PRO_CPU_UMV msg_rpta = obtener_nueva_shared_str();
				msg_rpta.OK = 1;
				msg_rpta.identificador_cpu = cpu->id;
				enviar_mensaje(msg->gen_msg.socket_descriptor_server,
						HANDSHAKE_CPU_PCP, &msg_rpta);
				encolar_cpu(cpu);

				sprintf(log_text, "[CPU %d] HANDSHAKE CON PCP ok? %d", cpu->id,
						1);
				log_info_message(log_text);

				break;
			}
			case NOTIFICACION_QUANTUM: {

				sprintf(log_text,
						"[CPU %d] Concluyo un quatum para el programa %d",
						msg->identificador_cpu, msg->PID);
				log_info_message(log_text);

				t_Programa* prog_x = obtener_proc_de_exec(msg->PID);
				prog_x->q = prog_x->q + 1;
				print_estado();

				sprintf(log_text,
						"[CPU %d] Concluyo un quatum #%llu para el programa %d",
						msg->identificador_cpu, prog_x->q, msg->PID);
				log_info_message(log_text);

				break;
			}
			case ENTRADA_SALIDA: {

				sprintf(log_text,
						"[CPU %d] IO %s con una utilizacion de %d para programa %d",
						msg->identificador_cpu, msg->id_dispositivo,
						msg->utilizacion, msg->PID);

				log_info_message(log_text);

				if (msg->OK == 10) //esto significa que luego de entrada salida el CPU va a proceder a cerrarse ya sea por senial o error
					eliminar_cpu_en_uso(msg->identificador_cpu);
				else
					liberar_cpu_en_uso(msg->identificador_cpu);

				estado_cp_proc_exec_a_block(msg->PID, msg->PCB);

				t_REQ_IO* req_io = (t_REQ_IO*) malloc(sizeof(t_REQ_IO));
				req_io->utilizacion = msg->utilizacion;
				req_io->io_finally = callback_io;
				req_io->PID = msg->PID;
				req_io->identificador_cpu = msg->identificador_cpu;
				req_io->id = (char*) malloc(strlen(msg->id_dispositivo));
				strcpy(req_io->id, msg->id_dispositivo);
				encolar_req_io(req_io, msg->id_dispositivo);

				break;
			}
			case WAIT: {
				sprintf(log_text,
						"[CPU %d] WAIT Semaforo %s con para programa %d",
						msg->identificador_cpu, msg->id_sem, msg->PID);
				log_info_message(log_text);

				t_KER_PRO_CPU_UMV msg_rpta = obtener_nueva_shared_str();

				if (dictionary_has_key(pcp->semaforos, msg->id_sem)) {

					sprintf(log_text,
							"[PCP] WAIT de semaforo %s solcitado por CPU %d para programa %d OK (existe)",
							msg->id_sem, msg->identificador_cpu, msg->PID);
					log_info_message(log_text);

					void continue_pcp() { //no requirio desalojo alguno

						sprintf(log_text,
								"[PCP] WAIT-CONTINUA sin desalojo ante semaforo %s solcitado por CPU %d para programa %d",
								msg->id_sem, msg->identificador_cpu, msg->PID);

						print_estado();

						msg_rpta.OK = 0; //indica que no hubo desalojo
						enviar_mensaje(msg->gen_msg.socket_descriptor_server,
								WAIT, &msg_rpta);
					}

					void sleep_process() { //requiere desalojo y luego llamara wake_up
						sprintf(log_text,
								"[PCP] WAIT-SLEEP de semaforo %s solcitado por CPU %d para programa %d",
								msg->id_sem, msg->identificador_cpu, msg->PID);
						log_info_message(log_text);

						msg_rpta.OK = 1; //requiere desalojo se indica con ok 1
						enviar_mensaje(msg->gen_msg.socket_descriptor_server,
								WAIT, &msg_rpta);

						if (msg->OK == 10) //esto significa que luego de wait el CPU va a proceder a cerrarse ya sea por senial o error (esto es porque aca se desaloja el CPU y no se quiere volver a poner disponible)
							eliminar_cpu_en_uso(msg->identificador_cpu);
						else
							liberar_cpu_en_uso(msg->identificador_cpu);

						estado_cp_proc_exec_a_block(msg->PID, msg->PCB);
					}

					t_sem_pcp* sem_pcp = (t_sem_pcp*) dictionary_get(
							pcp->semaforos, msg->id_sem);

					t_req_sem_pcp* req_sem = (t_req_sem_pcp*) malloc(
							sizeof(t_req_sem_pcp));
					req_sem->PID = msg->PID;
					req_sem->identificador_cpu = msg->identificador_cpu;
					req_sem->sem = sem_pcp;

					sprintf(log_text,
							"[PCP] WAIT de semaforo %s con valor actual %d desde CPU %d para programa %d",
							req_sem->sem->id_sem, req_sem->sem->valor,
							req_sem->identificador_cpu, req_sem->PID);
					log_debug_message(log_text);

					wait_pcp(req_sem, msg->PID, sleep_process,
							callback_wakeup_process, continue_pcp);

				} else {
					sprintf(log_text,
							"[PCP] WAIT de semaforo %s solcitado por CPU %d FALLO (no existe)",
							msg->id_sem, msg->identificador_cpu);
					log_error_message(log_text);
					exit(EXIT_FAILURE);
				}

				break;
			}
			case SIGNAL: {
				sprintf(log_text,
						"[CPU %d] SIGNAL Semaforo %s para Programa %d",
						msg->identificador_cpu, msg->id_sem, msg->PID);
				log_info_message(log_text);

				t_KER_PRO_CPU_UMV msg_rpta = obtener_nueva_shared_str();
				if (dictionary_has_key(pcp->semaforos, msg->id_sem)) {

					sprintf(log_text,
							"[PCP] SIGNAL de semaforo %s solcitado por CPU %d para programa %d OK (existe)",
							msg->id_sem, msg->identificador_cpu, msg->PID);
					log_info_message(log_text);

					t_sem_pcp* sem_pcp = dictionary_get(pcp->semaforos,
							msg->id_sem);

					sprintf(log_text,
							"[PCP] SIGNAL de semaforo %s con valor actual %d desde CPU %d para programa %d",
							sem_pcp->id_sem, sem_pcp->valor,
							msg->identificador_cpu, msg->PID);
					log_debug_message(log_text);

					signal_pcp(sem_pcp);
					print_estado();
					msg_rpta.OK = 1;
				} else {
					sprintf(log_text,
							"[PCP] SIGNAL de semaforo %s solcitado por CPU %d para programa %d FALLO (no existe)",
							msg->id_sem, msg->identificador_cpu, msg->PID);
					log_error_message(log_text);

					exit(EXIT_FAILURE);
				}

				enviar_mensaje(msg->gen_msg.socket_descriptor_server, SIGNAL,
						&msg_rpta);

				break;
			}
			case OBTENER_VALOR: {
				sprintf(log_text,
						"[CPU %d] Solicita Obtener Valor Variable Compartida %s para programa %d",
						msg->identificador_cpu, msg->id_var_comp, msg->PID);
				log_info_message(log_text);

				t_KER_PRO_CPU_UMV msg_rpta = obtener_nueva_shared_str();
				msg_rpta.id_var_comp = (char*) malloc(strlen(msg->id_var_comp));
				strcpy(msg_rpta.id_var_comp, msg->id_var_comp);
				if (dictionary_has_key(pcp->variables_compartidas,
						msg->id_var_comp)) {

					sprintf(log_text,
							"[CPU %d] Solicita Obtener Valor Variable Compartida %s para programa %d OK (existe)",
							msg->identificador_cpu, msg->id_var_comp, msg->PID);
					log_debug_message(log_text);

					msg_rpta.OK = 1;

					t_VC* vc = ((t_VC*) dictionary_get(
							pcp->variables_compartidas, msg->id_var_comp));
					//WAIT!!!!!!!!!!
					pthread_mutex_lock(&vc->mutex);
					msg_rpta.valor = vc->valor;
					pthread_mutex_unlock(&vc->mutex);
					//SIGNAL!!!!!!!!!!
				} else {
					sprintf(log_text,
							"[CPU %d] Solicita Obtener Valor Variable Compartida %s para programa %d FALLO (no existe)",
							msg->identificador_cpu, msg->id_var_comp, msg->PID);
					log_error_message(log_text);

					//es ERROR ver de cerrar el kernel (script chotado)
					exit(EXIT_FAILURE);
				}

				enviar_mensaje(msg->gen_msg.socket_descriptor_server,
						OBTENER_VALOR, &msg_rpta);

				sprintf(log_text,
						"[PCP] Variable compartida %s con valor %d retornado a CPU %d para programa %d",
						msg_rpta.id_var_comp, msg_rpta.valor,
						msg->identificador_cpu, msg->PID);
				log_info_message(log_text);

				break;
			}
			case GRABAR_VALOR: {
				sprintf(log_text,
						"[CPU %d] Solicita Grabar Valor Variable Compartida %s con valor %d para programa %d",
						msg->identificador_cpu, msg->id_var_comp, msg->valor,
						msg->PID);
				log_info_message(log_text);
				t_KER_PRO_CPU_UMV msg_rpta = obtener_nueva_shared_str();

				if (dictionary_has_key(pcp->variables_compartidas,
						msg->id_var_comp)) {

					sprintf(log_text,
							"[CPU %d] Solicita Grabar Valor Variable Compartida %s para programa %d OK (existe)",
							msg->identificador_cpu, msg->id_var_comp, msg->PID);
					log_debug_message(log_text);
					t_VC* vc = (t_VC*) dictionary_get(
							pcp->variables_compartidas, msg->id_var_comp);

					sprintf(log_text,
							"[CPU %d] Grabar Valor Variable Compartida %s tiene valor %d y grabara %d para programa %d OK (existe)",
							msg->identificador_cpu, msg->id_var_comp, vc->valor,
							msg->valor, msg->PID);
					log_debug_message(log_text);
					//WAIT!!!!!!!!!!
					pthread_mutex_lock(&vc->mutex);
					vc->valor = msg->valor;
					msg_rpta.valor = vc->valor;
					print_estado();
					//SIGNAL!!!!!!!!!!
					pthread_mutex_unlock(&vc->mutex);
					msg_rpta.OK = 1;

				} else {
					sprintf(log_text,
							"[CPU %d] Solicita Grabar Valor Variable Compartida %s para programa %d FALLO (no existe)",
							msg->identificador_cpu, msg->id_var_comp, msg->PID);
					log_error_message(log_text);

					//es ERROR ver de cerrar el kernel (script chotado)
					exit(EXIT_FAILURE);
				}

				enviar_mensaje(msg->gen_msg.socket_descriptor_server,
						GRABAR_VALOR, &msg_rpta);

				sprintf(log_text,
						"[PCP] Variable compartida %s salvada con valor %d solcitado por CPU %d para programa %d",
						msg->id_var_comp, msg->valor, msg->identificador_cpu,
						msg->PID);
				log_info_message(log_text);

				break;
			}
			case FINALIZAR_EJECUCION: {
				sprintf(log_text,
						"[CPU %d] Finalizo rafaga ejecucion para Programa %d",
						msg->identificador_cpu, msg->PID);
				log_info_message(log_text);

				if (msg->OK == 10) //esto significa que luego de finalizar la ejecucion el CPU va a proceder a cerrarse ya sea por senial o error (esto es porque aca se desaloja el CPU y no se quiere volver a poner disponible)
					eliminar_cpu_en_uso(msg->identificador_cpu);
				else
					liberar_cpu_en_uso(msg->identificador_cpu);

				estado_cp_proc_exec_a_ready(msg->PID, msg->PCB);

				break;
			}
			case DESCONEXION_CPU: { //cuando finaliza el CPU y finaliza el programa por quatum/IO/sem

				//si CPU me manda PID > 0 estaba ejecutando entonce me manda PCB y re planifico a ready el programa

	sprintf(log_text,
						"[CPU %d] SE RECIBIO ALTA DESONEXION ",
						msg->identificador_cpu);
				log_info_message(log_text);



				if (msg->PID > 0) //el CPU que se desconecto tenia una proceso en ejecucion
						{
					eliminar_cpu_en_uso(msg->identificador_cpu);
					estado_cp_proc_exec_a_exit(msg->PID, msg->PCB);

					t_Programa *programa = obtener_proc_de_exit(msg->PID);

					t_KER_PRO_CPU_UMV msg_rpta = obtener_nueva_shared_str();
					msg_rpta.texto =
							"[PCP] DESCONEXION DE CPU: PROGRAMA FINALIZA POR EXCEPCION!!!";
					msg_rpta.PID = msg->PID;
					msg_rpta.OK = 1;

					enviar_mensaje(programa->socket_server, IMPRIMIR_TEXTO,
							&msg_rpta);
					enviar_mensaje(programa->socket_server,
							FINALIZACION_EJECUCION_PROGRAMA, &msg_rpta);

				} 
				else
				{
	sprintf(log_text,
						"[CPU %d] SE RECIBIO DESONEXION Y NO TENIA PROGRAMA",
						msg->identificador_cpu);
				log_info_message(log_text);


if(msg->OK == 1)
{
	sprintf(log_text,
						"[CPU %d] SE RECIBIO DESONEXION Y NO TENIA PROGRAMA, ENTONCES SE ENVIA MENSAJE DE CHECKSUM a socket %d",


						msg->identificador_cpu,
						msg->gen_msg.socket_descriptor_server);
				log_info_message(log_text);
	t_CPU *cpu = obtener_cpu_por_socket_server(
				msg->gen_msg.socket_descriptor_server);

if(cpu!=NULL)
{
sprintf(log_text, "[PCP] obtuve CPU %d para chequear si tiene programa en ejecucion",cpu->id);
log_info_message(log_text);
	if(cpu->programa_en_ejecucion != NULL){
sprintf(log_text, "[PCP] CPU %d tiene programa %d en ejecucion",cpu->id, cpu->programa_en_ejecucion->PID);
log_info_message(log_text);

		estado_cp_proc_exec_a_ready(cpu->programa_en_ejecucion->PID, cpu->programa_en_ejecucion->PCB);

sprintf(log_text, "[PCP] Ante desconexion CPU se vuelve a ready programa en ejecucion %d", cpu->programa_en_ejecucion->PID);
log_info_message(log_text);
eliminar_cpu_en_uso(cpu->id);

sprintf(log_text, "[PCP] CPU %d eliminado",cpu->id);
log_info_message(log_text);
}
else
					//simplemente volar el CPU (estaba en desuso)
					eliminar_cpu(cpu->id);


}






					t_KER_PRO_CPU_UMV msg_rpta = obtener_nueva_shared_str();
					msg_rpta.PID = 0;
					msg_rpta.OK = 1;
sprintf(log_text, "[PCP] ENVIO DESCONEXION A CPU %d ",msg->identificador_cpu);
log_info_message(log_text);


					enviar_mensaje(msg->gen_msg.socket_descriptor_server, DESCONEXION_CPU,&msg_rpta);







}
else
{
//se desconecto con ctrl-c y no tenia programa en ejecucion
eliminar_cpu(msg->identificador_cpu);

}
				}

				break;
			}
			case FINALIZACION_EJECUCION_PROGRAMA: //cuando termina el programa normalmente
			{
				sprintf(log_text,
						"[CPU %d] Finalizo ejecucion para Programa %d",
						msg->identificador_cpu, msg->PID);
				log_info_message(log_text);
				if (msg->OK == 10) //esto significa que luego de Finalizacion Ejecucion Programa el CPU va a proceder a cerrarse ya sea por senial o error (esto es porque aca se desaloja el CPU y no se quiere volver a poner disponible)
					eliminar_cpu_en_uso(msg->identificador_cpu);
				else
					liberar_cpu_en_uso(msg->identificador_cpu);

				estado_cp_proc_exec_a_exit(msg->PID, msg->PCB);

				//tomo de exit el programa
				t_Programa *programa = obtener_proc_de_exit(msg->PID);
				//imprimo texto en el programa con las variables
				t_KER_PRO_CPU_UMV msg_rpta = obtener_nueva_shared_str();
				msg_rpta.texto = (char*) malloc(strlen(msg->texto));
				strcpy(msg_rpta.texto, msg->texto);
				msg_rpta.PID = msg->PID;
				msg_rpta.OK = 1;

				enviar_mensaje(programa->socket_server, IMPRIMIR_TEXTO,
						&msg_rpta);
				enviar_mensaje(programa->socket_server,
						FINALIZACION_EJECUCION_PROGRAMA, &msg_rpta);

				break;
			}
			case IMPRIMIR_TEXTO: {
				sprintf(log_text,
						"[CPU %d] Programa %d Envio a Imprimir Texto %s",
						msg->identificador_cpu, msg->PID, msg->texto);
				log_info_message(log_text);
				t_KER_PRO_CPU_UMV msg_rpta = obtener_nueva_shared_str();
				msg_rpta.texto = (char*) malloc(strlen(msg->texto));
				strcpy(msg_rpta.texto, msg->texto);
				msg_rpta.PID = msg->PID;
				msg_rpta.OK = 1;
				t_Programa *programa = obtener_proc_de_exec(msg->PID);
				enviar_mensaje(programa->socket_server, IMPRIMIR_TEXTO,
						&msg_rpta);
				break;

			}
			case IMPRIMIR: {
				sprintf(log_text,
						"[CPU %d] Programa %d Envio a Imprimir el valor %d",
						msg->identificador_cpu, msg->PID, msg->valor);
				log_info_message(log_text);
				t_KER_PRO_CPU_UMV msg_rpta = obtener_nueva_shared_str();
				msg_rpta.gen_msg = gen_msg;
				msg_rpta.valor = msg->valor;
				msg_rpta.texto = (char*) malloc(strlen(msg->texto));
				strcpy(msg_rpta.texto, msg->texto);
				msg_rpta.PID = msg->PID;
				msg_rpta.OK = 1;
				t_Programa *programa = obtener_proc_de_exec(msg->PID);
				enviar_mensaje(programa->socket_server, IMPRIMIR, &msg_rpta);
				break;
			}

			default:
				//Mensaje no valido
			{
				sprintf(log_text,
						"[MENSAJE INVALIDO] PLANIFICADOR CORTO PLAZO no tiene handler para id mensaje %d",
						gen_msg.id_MSJ);

				log_info_message(log_text);
				break;
			}
			}
		} else {
			sprintf(log_text,
					"[DESCONEXION EN PCP] de CPU con socket server %d (%s) del PCP",
					gen_msg.socket_descriptor_server,
					identidad_socket_server[gen_msg.socket_descriptor_server]);
			log_info_message(log_text);

			//obtener PID por socket
			t_CPU *cpu = obtener_cpu_por_socket_server(
					gen_msg.socket_descriptor_server);
			if (cpu != NULL ) {
				//llamanda recursiva para finalizacion desconexion de CPU
				t_KER_PRO_CPU_UMV msg_rpta = obtener_nueva_shared_str();
				msg_rpta.gen_msg = gen_msg;
				msg_rpta.gen_msg.id_MSJ = DESCONEXION_CPU;
				msg_rpta.gen_msg.disconnected = 0;
				msg_rpta.OK = 0;

				if (cpu->programa_en_ejecucion != NULL ) {
					if (cpu->programa_en_ejecucion->delegado) {
						msg_rpta.PCB = cpu->programa_en_ejecucion->PCB;
						msg_rpta.PID = cpu->programa_en_ejecucion->PID;
					}
				}
				msg_rpta.identificador_cpu = cpu->id;
				msg_rpta.texto = "FALLO POR DESCONEXION DE CPU";
				handler_pcp_msgs_ptr(&msg_rpta);
			}
		}

	} else {
		log_error_message("Se ha recibido mensaje nulo en planificador");
	}

	return NULL ;
}

void* asignar_cpu(t_PCP *pcp) {
	char log_text[500];
	int quantum = pcp->quantum;
	int retardo = pcp->retardo;

	while (1) {
		//begin log
		log_debug_message(

		"[PCP] Esperando Programa en Cola de Ready");
		//end log

		//Tomar Proximo Programa (FIFO)
		t_Programa* programa = obtener_proc_de_ready();
		programa->delegado = false;
		log_debug_message("[PCP] Se obtuvo programa");

		//begin log
		sprintf(log_text,

		"[PCP] Obteniendo CPU para programa %d - %s", programa->PID,
				programa->nombre);

		log_trace_message(log_text);
		//end log

		//Tomar CPU libre
		t_CPU* cpu = obtener_cpu();
		programa->cpu = cpu->id;
		estado_cp_proc_ready_a_exec(programa);


		int cpu_id = cpu->id;
		t_KER_PRO_CPU_UMV shared_str = obtener_nueva_shared_str();

		shared_str.quantum = quantum;
		shared_str.retardo = retardo;
		shared_str.PID = programa->PID;
		shared_str.PCB = programa->PCB;

		//enviamos a ejecutar al CPU el programa
		int nbytes_env = enviar_mensaje(cpu->socket_server, EJECUTAR_PROGRAMA,
				&shared_str);
		if (nbytes_env > 0) {
			cpu->programa_en_ejecucion = programa;
			programa->delegado = true;
			sprintf(log_text,

			"[PCP] Se envio a ejecutar programa %d - %s en CPU %d con exito",
					programa->PID, programa->nombre, cpu->id);

			log_info_message(log_text);

		} else {
			sprintf(log_text,

			"[PCP] ERROR al enviar a ejecutar programa %d - %s en CPU %d",
					programa->PID, programa->nombre, cpu_id);

			log_error_message(log_text);
			estado_cp_proc_exec_a_ready(programa->PID, programa->PCB);
		}
	}
	return EXIT_SUCCESS;
}

void io(t_HIO* hio) {
	char log_text[500];

	t_sync_queue_mutex_cont* requests;
	if (dictionary_has_key(pcp->reqs_ios, hio->id))
		requests = dictionary_get(pcp->reqs_ios, hio->id);
	else {
		//begin log
		sprintf(log_text,

		"[IO] No se encontro el dispositivo con id %s", hio->id);
		log_error_message(log_text);
		//end log

		exit(EXIT_FAILURE);
	}

	t_REQ_IO* req_io;
	while (1) {

		req_io = obtener_req_io(requests);
		pthread_mutex_lock(&pcp->mutex_ios);
		t_Programa* prog_b = obtener_proc_de_block(req_io->PID);
		prog_b->ios = prog_b->ios + 1;
		prog_b->io = req_io->id[0];
		print_estado();
		usleep(hio->ms * 1000 * req_io->utilizacion); //convertimos a microsecs y luego multiplicamos por la utilizacion
		pthread_mutex_unlock(&pcp->mutex_ios);
		req_io->io_finally(req_io);
		free(req_io);
	}
}

void lanzar_hios(t_dictionary* hios) {
	char log_text[500];
	int result;
	void each(char* key, t_HIO* hio) {
		pthread_t thread_hio;
		result = pthread_create(&thread_hio, NULL, (void*) io, (void *) hio); //Hilo Planificacion Largo Plazo
		if (result) {
			//begin log
			sprintf(log_text,

			"[KERNEL] Hilo %s con %d de retardo", hio->id, hio->ms);
			log_error_message(log_text);
			//end log

		} else {
			//begin log
			sprintf(log_text,

			"[KERNEL] Hilo %s con %d de retardo lanzada EXITOSAMENTE", hio->id,
					hio->ms);

			log_info_message(log_text);
			//end log
		}
	}

	dictionary_iterator(hios, (void*) each);
}

void* lanzar_pcp(t_PCP *pcp) {
	char log_text[500];

	lanzar_hios(pcp->ios);

	pthread_t thread_planificacion_corto_plazo;

	int result;
	result = pthread_create(&thread_planificacion_corto_plazo, NULL,
			(void *) asignar_cpu, (void *) pcp); //Hilo Planificacion Corto Plazo
	if (result) {
		//begin log
		log_error_message(

		"Planificacion PCP %d");
		//end log
		exit(EXIT_FAILURE);

	} else {
		//begin log
		log_info_message(

		"Planificacion PCP Iniciada");
		//end log
	}

	sprintf(log_text,

	"PCP Iniciado para puerto %d", pcp->puerto);

	log_info_message(log_text);

	handler_pcp_msgs_ptr = &handler_pcp_msgs;

	abrir_conexion_servidor_local(pcp->puerto, handler_pcp_msgs_ptr);

	return EXIT_SUCCESS;
}
