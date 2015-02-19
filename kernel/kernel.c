#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/log_app.h>
#include <commons/shared_strs.h>
#include "config_kernel.h"
#include "kernel.h"
#include "plp.h"
#include "pcp.h"
#include "print_kernel.h"

t_PLP* plp;
t_PCP* pcp;

int main(void) {
	con_vals.b_count = 0;
	con_vals.cols = 0;
	con_vals.cpus_tot = 0;
	con_vals.cpus_uso=0;
	con_vals.e_count=0;
	con_vals.mpu = 0;
	con_vals.mp=0;
	con_vals.n_count=0;
	con_vals.r_count=0;
	con_vals.rows = 0;
	con_vals.tem=0;
	con_vals.x_count=0;
	con_vals.ios=string_new();
	con_vals.procs = list_create();
	con_vals.enable = false;


	char log_text[500];
	inicializar_log(PATH_LOG, "kernel");

	deshabilitar_log_trace();
	//habilitar_log_debug();
	log_info_message(

					"Kernel Iniciado"
					);

	plp = (t_PLP*) malloc(sizeof(t_PLP));

	plp->ready = syncqueue_mutex_cont_create(0);
	plp->ultimo_PID_programa = 0;

	pcp = (t_PCP*) malloc(sizeof(t_PCP));
	pcp->ready = plp->ready; //compartida (es la misma)
	pcp->block = syncqueue_create();
	pcp->exec = syncqueue_create();
	pcp->exit = syncqueue_create();
	pcp->semaforos = dictionary_create();
	pcp->ios = dictionary_create();
	pcp->reqs_ios = dictionary_create();
	pcp->variables_compartidas = dictionary_create();
	pcp->CPUs = syncqueue_mutex_cont_create(0);
	pcp->CPUs_en_uso = synclist_create();
	pcp->ultimo_PID_cpu = 0;
	pthread_mutex_init(&pcp->mutex_ios, NULL);
	pcp->callback_programa_finalizado = &callback_programa_finalizado;
	log_debug_message("Se creo la estructura PCP");

	open_config(PATH_CONFIG);

	//PLP
	plp->puerto = PUERTO_PROG();
	plp->puerto_umv = PUERTO_UMV();
	plp->stack_size = STACK_SIZE();
	plp->multiprogramacion = MULTIPROGRAMACION();
	sprintf(log_text, "[KERNEL] CONFIG PUERTO_PLP con valor %d",
			plp->puerto);
	log_debug_message(log_text);
	con_vals.mp = plp->multiprogramacion;
	sprintf(log_text, "[KERNEL] CONFIG PUERTO_UMV con valor %d",
			plp->puerto_umv);
	log_debug_message(log_text);

	sprintf(log_text, "[KERNEL] CONFIG STACK_SIZE con valor %d",
			plp->stack_size);
	log_debug_message(log_text);


	sprintf(log_text, "[KERNEL] CONFIG MULTI con valor %d",
			plp->multiprogramacion);
	log_debug_message(log_text);

	char* ip_umv = IP_UMV();
	plp->ip_umv = string_new();

	string_append(&plp->ip_umv, ip_umv);
	
	sem_init(&plp->nivel_multiprogramacion,0, plp->multiprogramacion);
	plp->new = synclist_mutex_cont_create(0);
	log_debug_message("Se creo la estructura PLP");

	//PCP
	pcp->puerto = PUERTO_CPU();
	pcp->quantum = QUANTUM();
	pcp->retardo = RETARDO();
	char** array_ids = SEMAFOROS();
	char** array_valores_init = VALOR_SEMAFORO();
	char* key;
	char* valor_ptr;
	uint32_t  valor = 0;
	t_sem_pcp* sem_cont;
	while (*array_ids != NULL && *array_valores_init!=NULL)
	{
		key = *array_ids++;
		valor_ptr = *array_valores_init++;
		valor = atoi(valor_ptr);
		sem_cont=(t_sem_pcp*)malloc(sizeof(t_sem_pcp));
		sem_cont->valor = valor;
		sem_cont->bloqueados = syncqueue_create();
		sem_cont->id_sem = (char*)malloc(strlen(key));
		strcpy(sem_cont->id_sem, key);
		dictionary_put(pcp->semaforos, key, sem_cont);
		sprintf(log_text, "[KERNEL] CONFIG %s con valor init %d",
				key, valor);
		log_debug_message(log_text);
	}

	array_ids = ID_HIO();
	array_valores_init = HIO();
	valor = 0;
	t_HIO* hio;
	char* io_nombre;
	while (*array_ids != NULL && *array_valores_init!=NULL)
	{

		key = *array_ids++;
		valor_ptr = *array_valores_init++;
		valor = atoi(valor_ptr);

		hio = (t_HIO*)malloc(sizeof(t_HIO));
		hio->id = key;
		io_nombre = malloc(sizeof(char)*strlen(key)+4);
		sprintf(io_nombre,"%s(%c);",key,key[0]);
		string_append(&con_vals.ios,io_nombre);
		free(io_nombre);
		hio->ms = valor;

		dictionary_put(pcp->ios, key, (t_HIO*)hio);
		dictionary_put(pcp->reqs_ios, key, syncqueue_mutex_cont_create(0));

		sprintf(log_text, "[KERNEL] CONFIG %s con valor %d",
				key, ((t_HIO*)dictionary_get(pcp->ios,key))->ms);
		log_debug_message(log_text);
	}

	array_ids = VARIABLES_COMPARTIDAS();
	t_VC* vc;
	while (*array_ids != NULL)
	{
		vc = (t_VC*)malloc(sizeof(t_VC));
		pthread_mutex_init(&vc->mutex,NULL);
		vc->valor = 0;
		key = *array_ids++;
		dictionary_put(pcp->variables_compartidas, key, vc);

		sprintf(log_text, "[KERNEL] CONFIG %s con valor %d",
				key, ((t_VC*)dictionary_get(pcp->variables_compartidas,key))->valor);
		log_debug_message(log_text);
	}
	con_vals.v_c = pcp->variables_compartidas;
	con_vals.sems = pcp->semaforos;
	if(ENABLE_LOG()==1)
	{
		crear_printable_area();
		con_vals.enable = true;
		cambiar_output(print_mensaje);

	}
	close_current_config();

	print_estado();

	pthread_t thread_plp;
	pthread_t thread_pcp;
	int result;
	result = pthread_create(&thread_plp, NULL, (void*) lanzar_plp,
			(void *) plp); //Hilo Planificacion Largo Plazo
	if (result) {
		//begin log
		log_error_message(
						"[KERNEL] Error al lanzar Planificacion Largo Plazo"
);
		//end log
		return EXIT_FAILURE;

	} else {
		//begin log
		log_info_message(

						"[KERNEL] Planificacion Largo Plazo lanzada EXITOSAMENTE "
);
		//end log
	}

	result = pthread_create(&thread_pcp, NULL, (void*) lanzar_pcp, (void*) pcp); //Hilo Planificacion Corto Plazo
	if (result) {
		//begin log
		log_error_message(

						"[KERNEL] Error al lanzar Planificacion Corto Plazo"
						);
		//end log
		return EXIT_FAILURE;

	} else {
		//begin log
		log_info_message(

						"[KERNEL] Planificacion Corto Plazo lanzada EXITOSAMENTE "
						);
		//end log
	}
	pthread_join(thread_plp, NULL );
	pthread_join(thread_pcp, NULL );
	return EXIT_SUCCESS;
}

