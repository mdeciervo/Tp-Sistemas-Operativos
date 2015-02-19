// structs propietarios del PLP/PCP (ej: PCB no es una estructura propietaria ya que es compartida con CPU)

#include "glb_header_kernel.h"

pthread_mutex_t mutex_thread_init = PTHREAD_MUTEX_INITIALIZER;

t_sync_queue* syncqueue_create()
{
	t_sync_queue* planif_queue = malloc(sizeof(t_sync_queue));
	planif_queue->mutex = mutex_thread_init;
	planif_queue->queue = queue_create();
	return planif_queue;
}

t_sync_list* synclist_create()
{
	t_sync_list* planif_list = malloc(sizeof(t_sync_list));
	planif_list->mutex = mutex_thread_init;
	planif_list->list = list_create();
	return planif_list;
}

t_sync_queue_mutex_cont* syncqueue_mutex_cont_create(int init_val)
{
	t_sync_queue_mutex_cont* planif_queue = malloc(sizeof(t_sync_queue_mutex_cont));
	sem_init(&planif_queue->sem_cont, 0, init_val);
	planif_queue->planif_queue = syncqueue_create();
	return planif_queue;
}

t_sync_list_mutex_cont* synclist_mutex_cont_create(int init_val)
{
	t_sync_list_mutex_cont* planif_list = malloc(sizeof(t_sync_list_mutex_cont));
	sem_init(&planif_list->sem_cont, 0, init_val);
	planif_list->planif_list = synclist_create();
	return planif_list;
}

void* syncqueue_find(t_sync_queue* queue, bool(*condition)(void*))
{
	return sync_list_find(queue->queue->elements, &queue->mutex, condition);
}

void* synclist_find(t_sync_list* lista, bool(*condition)(void*))
{
	return sync_list_find(lista->list, &lista->mutex, condition);
}

void synclist_add(t_sync_list* lista, void* item)
{
	sync_list_add(lista->list, &lista->mutex, item);
}

void* syncqueue_pop(t_sync_queue* queue) {
	return sync_queue_pop(queue->queue,&queue->mutex);
}

void* syncqueue_peek(t_sync_queue* queue) {
	return sync_queue_peek(queue->queue,&queue->mutex);
}

void syncqueue_sort(t_sync_queue* queue, bool (*comparator)(void *, void *)) {
	sync_list_sort(queue->queue->elements,&queue->mutex, comparator);
}

void synclist_sort(t_sync_list* list, bool (*comparator)(void *, void *))
{
	sync_list_sort(list->list,&list->mutex, comparator);
}

void syncqueue_push(t_sync_queue* queue, void* item)
{
	sync_queue_push(queue->queue,&queue->mutex, item);
}

void* synclist_remover(t_sync_list* list, bool(*condition)(void*))
{
	return sync_list_remover(list->list,&list->mutex, condition);
}


void* syncqueue_remover(t_sync_queue* queue, bool(*condition)(void*))
{
	return sync_queue_remover(queue->queue,&queue->mutex, condition);
}

bool syncqueue_is_empty(t_sync_queue* queue)
{
	return sync_queue_is_empty(queue->queue,&queue->mutex);
}

void synclist_mover_item_entre_listas(t_sync_list* lista_origen, t_sync_list* lista_destino, bool(*condition)(void*))
{
	sync_mover_item_entre_listas(lista_origen->list, lista_origen->mutex, lista_destino->list, lista_destino->mutex, condition);
}

bool syncqueue_mover_item_entre_colas(t_sync_queue* cola_origen, t_sync_queue* cola_destino, bool(*condition)(void*))
{
	return sync_mover_item_entre_colas(cola_origen->queue, cola_origen->mutex, cola_destino->queue, cola_destino->mutex, condition);
}

void synclist_mover_item_de_lista_a_cola(t_sync_list* lista_origen, t_sync_queue* cola_destino, bool(*condition)(void*))
{
	sync_mover_item_de_lista_a_cola(lista_origen->list, lista_origen->mutex, cola_destino->queue, cola_destino->mutex, condition);
}

void syncqueue_mover_item_de_cola_a_lista(t_sync_queue* cola_origen, t_sync_list* lista_destino, bool(*condition)(void*))
{
	sync_mover_item_de_cola_a_lista(cola_origen->queue, cola_origen->mutex, lista_destino->list, lista_destino->mutex, condition);
}

void* syncqueue_mutex_cont_wait(t_sync_queue_mutex_cont* queue, void* (*func)()) {
	return syncronize_mutex_cont_wait(&(queue->sem_cont), func);
}

void syncqueue_mutex_cont_post(t_sync_queue_mutex_cont* queue, void (*func)())
{
	syncronize_mutex_cont_post(&(queue->sem_cont), func);
}


void* synclist_mutex_cont_wait(t_sync_list_mutex_cont* list, void* (*func)()) {
	return syncronize_mutex_cont_wait(&(list->sem_cont), func);
}

void synclist_mutex_cont_post(t_sync_list_mutex_cont* list, void (*func)())
{
	syncronize_mutex_cont_post(&(list->sem_cont), func);
}


void wait_pcp(t_req_sem_pcp* req_sem, uint32_t pid, void (*sleep_pcp)(), void (*wakeup_pcp)(t_req_sem_pcp*),void (*continue_pcp)())
{
	req_sem->sem->valor--;
	if(req_sem->sem->valor<0)
	{
		t_sem_pcp_proc_block *p_block = (t_sem_pcp_proc_block*)malloc(sizeof(t_sem_pcp_proc_block));
		p_block->pid = pid;
		p_block->wakeup_pcp = wakeup_pcp;
		p_block->req_sem = req_sem;
		syncqueue_push(req_sem->sem->bloqueados, p_block);
		sleep_pcp();
	}
	else
	{
		continue_pcp();
	}

}

void signal_pcp(t_sem_pcp* sem)
{
	sem->valor++;
//	if (sem->valor>=0)
//	{
		if(!syncqueue_is_empty(sem->bloqueados))
		{
			t_sem_pcp_proc_block *p_block = syncqueue_pop(sem->bloqueados);
			p_block->wakeup_pcp(p_block->req_sem);
		}
//	}

}



