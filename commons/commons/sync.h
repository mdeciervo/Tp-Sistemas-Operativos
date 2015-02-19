#ifndef _SYNFH_
#define _SYNFH_

#include <semaphore.h>
#include <pthread.h>
#include "collections/list.h"
#include "collections/queue.h"

typedef struct strSyncQueue {
	pthread_mutex_t mutex;
	t_queue * queue;
} t_sync_queue;

typedef struct strSyncList {
	pthread_mutex_t mutex;
	t_list * list;
} t_sync_list;

typedef struct strSyncQueueMutexCont {
	sem_t sem_cont;
	t_sync_queue* planif_queue;
} t_sync_queue_mutex_cont;

typedef struct strSyncListMutextCont {
	sem_t sem_cont;
	t_sync_list* planif_list;
} t_sync_list_mutex_cont;


void* syncronize_pthread_mutex(pthread_mutex_t*, void* (*func)());
void* syncronize_mutex_cont_wait(sem_t*, void* (*func)());
void syncronize_mutex_cont_post(sem_t*, void (*func)());


//Sincronizacion de listas y colas

void sync_queue_push(t_queue* queue, pthread_mutex_t* m, void* item);
bool sync_queue_is_empty(t_queue* queue, pthread_mutex_t* m);
void* sync_queue_pop(t_queue* queue, pthread_mutex_t* m);
void* sync_queue_peek(t_queue* queue, pthread_mutex_t* m);
void* sync_queue_remover(t_queue* queue, pthread_mutex_t* m, bool(*condition)(void*));

void* sync_list_remover(t_list* list, pthread_mutex_t* m, bool(*condition)(void*));
void* sync_list_find(t_list* lista, pthread_mutex_t* m, bool(*condition)(void*));
void sync_list_add(t_list* lista, pthread_mutex_t* m, void* item);
void* sync_list_get(t_list* lista, pthread_mutex_t* m,int index);
void sync_list_sort(t_list *lista, pthread_mutex_t* m, bool (*comparator)(void *, void *));

void sync_mover_item_entre_listas(t_list* lista_origen, pthread_mutex_t mu_o, t_list* lista_destino, pthread_mutex_t mu_d, bool(*condition)(void*));
bool sync_mover_item_entre_colas(t_queue* cola_origen, pthread_mutex_t mu_o,t_queue* cola_destino, pthread_mutex_t mu_d, bool (*condition)(void*));
void sync_mover_item_de_lista_a_cola(t_list* lista_origen, pthread_mutex_t mu_o, t_queue* cola_destino, pthread_mutex_t mu_d, bool(*condition)(void*));
void sync_mover_item_de_cola_a_lista(t_queue* cola_origen, pthread_mutex_t mu_o, t_list* lista_destino, pthread_mutex_t mu_d, bool(*condition)(void*));
#endif
