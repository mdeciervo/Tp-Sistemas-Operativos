// structs propietarios del PLP/PCP (ej: PCB no es una estructura propietaria ya que es compartida con CPU)

#ifndef _PLANIFH_
#define _PLANIFH_
#include <stdint.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/shared_strs.h>
#include <semaphore.h>
#include <pthread.h>
#include "commons/sync.h"

typedef struct strSemPCP {
	pthread_mutex_t mutex; //para el valor
	int valor;
	t_sync_queue* bloqueados;
	char* id_sem;
} t_sem_pcp;

typedef struct strReqSemPCP {
	t_sem_pcp* sem;
	uint32_t PID;
	uint32_t identificador_cpu;
} t_req_sem_pcp;

typedef struct strSemPCPProcBlock
{
	uint32_t pid;
	void (*wakeup_pcp)(t_req_sem_pcp*);
	t_req_sem_pcp* req_sem;
} t_sem_pcp_proc_block;


typedef struct strPrograma {
	uint32_t PID;
	char* nombre;
	int socket_server;
	t_PCB PCB;
	int peso;

	//luego se busca por pid y se actualiza la siguiente informacion
	int cpu; // cuando cambia de CPU
	char cq; //queue actual - cuando cambia de estado en CP
	unsigned long long int q; //quatum actual c- cuando concluye un quantum
	char io; //cuando se va a IO
	int ios;

	// para uso interno
	int r; //row
	bool delegado; //campo que indica si el programa se ha llegado a enviar al CPU y este no se desconecto en el intento!

} t_Programa;


typedef struct strCPU {
	uint32_t id;
	int socket_server;
	t_Programa *programa_en_ejecucion;
} t_CPU;


typedef struct strPLP {
	int puerto;
	int multiprogramacion;
	int puerto_umv;
	char* ip_umv;
	int socket_umv;
	int stack_size;
	t_sync_list_mutex_cont* new; // es cont tambien porque tiene que manejar el nivel de multiprogramacion
	t_sync_queue_mutex_cont* ready; //compartida con PCP
	uint32_t ultimo_PID_programa;
	sem_t nivel_multiprogramacion;
} t_PLP;

typedef struct strPCP {
	int puerto;
	int quantum;
	int retardo;
	int socket_descriptor_server;

	t_sync_queue_mutex_cont* CPUs;
	t_sync_list* CPUs_en_uso;

	t_sync_queue_mutex_cont* ready; //compartida con PLP
	t_sync_queue* block;
	t_sync_queue* exec;
	t_sync_queue* exit;

	t_dictionary* semaforos;
	t_dictionary* ios;
	t_dictionary* reqs_ios;
	t_dictionary* variables_compartidas;

	uint32_t ultimo_PID_cpu;
	pthread_mutex_t mutex_ios;

	void (*callback_programa_finalizado)();
} t_PCP;

typedef struct strHIO {
	int ms;
	char* id;
} t_HIO;

typedef struct strREQIO {
	int utilizacion;
	void (*io_finally)(void*);
	int PID;
	int identificador_cpu;
	char* id;
} t_REQ_IO;

typedef struct strVC {
	pthread_mutex_t mutex;
	int valor;
} t_VC;


t_sync_queue_mutex_cont* syncqueue_mutex_cont_create(int init_val);
t_sync_list_mutex_cont* synclist_mutex_cont_create(int init_val);
t_sync_queue* syncqueue_create();
t_sync_list* synclist_create();
void* synclist_find(t_sync_list* lista, bool (*condition)(void*));
void synclist_add(t_sync_list* lista, void* item);
void* syncqueue_pop(t_sync_queue* queue);
void syncqueue_push(t_sync_queue* queue, void* item);
void syncqueue_sort(t_sync_queue* queue, bool (*comparator)(void *, void *));
void* syncqueue_peek(t_sync_queue* queue);
bool syncqueue_is_empty(t_sync_queue* queue);
void* syncqueue_find(t_sync_queue* queue, bool (*condition)(void*));
void* syncqueue_remover(t_sync_queue* queue, bool(*condition)(void*));
void* synclist_remover(t_sync_list* list, bool(*condition)(void*));
void synclist_sort(t_sync_list* list, bool (*comparator)(void *, void *));
void synclist_mover_item_entre_listas(t_sync_list* lista_origen,
		t_sync_list* lista_destino, bool (*condition)(void*));
bool syncqueue_mover_item_entre_colas(t_sync_queue* cola_origen,
		t_sync_queue* cola_destino, bool (*condition)(void*));
void synclist_mover_item_de_lista_a_cola(t_sync_list* lista_origen,
		t_sync_queue* cola_destino, bool (*condition)(void*));
void syncqueue_mover_item_de_cola_a_lista(t_sync_queue* cola_origen,
		t_sync_list* lista_destino, bool (*condition)(void*));
void* syncqueue_mutex_cont_wait(t_sync_queue_mutex_cont* queue, void* (*func)());
void syncqueue_mutex_cont_post(t_sync_queue_mutex_cont* queue, void (*func)());
void* synclist_mutex_cont_wait(t_sync_list_mutex_cont* list, void* (*func)());
void synclist_mutex_cont_post(t_sync_list_mutex_cont* list, void (*func)());
void signal_pcp(t_sem_pcp* sem);
void wait_pcp(t_req_sem_pcp* sem, uint32_t pid, void (*sleep_pcp)(), void (*wakeup_pcp)(t_req_sem_pcp*),void (*continue_pcp)());
#endif
