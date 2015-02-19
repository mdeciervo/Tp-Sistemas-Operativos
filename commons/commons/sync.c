#include "sync.h"

// Sincronizacion mutex
void* syncronize_pthread_mutex(pthread_mutex_t* t_m, void* (*func)()) {
	pthread_mutex_lock(t_m);
	void* s = func();
	pthread_mutex_unlock(t_m);
	return s;
}

// Sincronizacion para productor consumidor (contador)
void* syncronize_mutex_cont_wait(sem_t* m, void* (*func)()) {
	sem_wait(m);
	return func();
}
void syncronize_mutex_cont_post(sem_t* m, void (*func)()) {
	func();
	sem_post(m);
}

//Sincronizacion de listas y colas

void sync_list_add(t_list* lista, pthread_mutex_t* m, void* item) {
	void* syncronize_add() {
		list_add(lista, item);
		return NULL ;
	}

	syncronize_pthread_mutex(m, syncronize_add);

}

void* sync_queue_pop(t_queue* queue, pthread_mutex_t* m) {
	void* syncronize_pop() {
		return queue_pop(queue);
	}

	return syncronize_pthread_mutex(m, syncronize_pop);
}

void* sync_queue_peek(t_queue* queue, pthread_mutex_t* m) {
	void* syncronize_peek() {
		return queue_peek(queue);
	}

	return syncronize_pthread_mutex(m, syncronize_peek);
}


void* sync_list_remover(t_list* list, pthread_mutex_t* m,
		bool (*condition)(void*)) {
	void* syncronize_remover() {
		return list_remove_by_condition(list, condition);
	}

	return syncronize_pthread_mutex(m, syncronize_remover);

}

void* sync_queue_remover(t_queue* queue, pthread_mutex_t* m,
		bool (*condition)(void*)) {
	return sync_list_remover(queue->elements, m, condition);
}

void sync_queue_push(t_queue* queue, pthread_mutex_t* m, void* item) {
	void* syncronize_push() {
		queue_push(queue, item);
		return NULL ;
	}

	syncronize_pthread_mutex(m, syncronize_push);

}

bool sync_queue_is_empty(t_queue* queue, pthread_mutex_t* m) {
	void* syncronize_is_empty() {
		return queue_is_empty(queue);
	}

	return syncronize_pthread_mutex(m, syncronize_is_empty);

}

void* sync_list_find(t_list* lista, pthread_mutex_t* m,
		bool (*condition)(void*)) {

	void* syncronize_find() {

		void* item = list_find(lista, condition);

		return item;
	}

	return syncronize_pthread_mutex(m, (void*) syncronize_find);
}

void* sync_list_get(t_list* lista, pthread_mutex_t* m,
		int index) {

	void* syncronize_get() {

		void* item = list_get(lista, index);

		return item;
	}

	return syncronize_pthread_mutex(m, (void*) syncronize_get);
}

void sync_list_sort(t_list *lista, pthread_mutex_t* m, bool (*comparator)(void *, void *)) {

	void* syncronize_sort() {

		list_sort(lista, comparator);

		return NULL;
	}

	syncronize_pthread_mutex(m, (void*) syncronize_sort);
}


void sync_mover_item_entre_listas(t_list* lista_origen, pthread_mutex_t mu_o,
		t_list* lista_destino, pthread_mutex_t mu_d, bool (*condition)(void*)) {

	void* syncronize_obtener_remover() {

		void* item = list_find(lista_origen, condition);
		list_remove_by_condition(lista_origen, condition);

		return item;
	}

	void* item = syncronize_pthread_mutex(&mu_o,
			(void*) syncronize_obtener_remover);

	void syncronize_add() {
		list_add(lista_destino, item);
	}

	syncronize_pthread_mutex(&mu_d, (void*) syncronize_add);

}

bool sync_mover_item_entre_colas(t_queue* cola_origen, pthread_mutex_t mu_o,
		t_queue* cola_destino, pthread_mutex_t mu_d, bool (*condition)(void*)) {

	void* syncronize_obtener_remover() {
		void* item =  list_remove_by_condition(cola_origen->elements, condition);
		return item;
	}

	void* item = syncronize_pthread_mutex(&mu_o,
			(void*) syncronize_obtener_remover);
	if(item==NULL)
		return false;
	void syncronize_push() {
		queue_push(cola_destino, item);
	}

	syncronize_pthread_mutex(&mu_d, (void*) syncronize_push);
	return true;

}

void sync_mover_item_de_lista_a_cola(t_list* lista_origen, pthread_mutex_t mu_o,
		t_queue* cola_destino, pthread_mutex_t mu_d, bool (*condition)(void*)) {

	void* syncronize_obtener_remover() {

		void* item = list_remove_by_condition(lista_origen, condition);

		return item;
	}

	void* item = syncronize_pthread_mutex(&mu_o,
			(void*) syncronize_obtener_remover);

	void syncronize_push() {
		queue_push(cola_destino, item);
	}
	if (item != NULL )
		syncronize_pthread_mutex(&mu_d, (void*) syncronize_push);

}

void sync_mover_item_de_cola_a_lista(t_queue* cola_origen, pthread_mutex_t mu_o,
		t_list* lista_destino, pthread_mutex_t mu_d, bool (*condition)(void*)) {

	void* syncronize_obtener_remover() {

		void* item = list_find(cola_origen->elements, condition);
		list_remove_by_condition(cola_origen->elements, condition);

		return item;
	}

	void* item = syncronize_pthread_mutex(&mu_o,
			(void*) syncronize_obtener_remover);

	void syncronize_add() {
		list_add(lista_destino, item);
	}

	syncronize_pthread_mutex(&mu_d, (void*) syncronize_add);

}

