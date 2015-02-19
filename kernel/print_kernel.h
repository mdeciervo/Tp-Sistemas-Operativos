#ifndef _PRINTPLANIFH_
#define _PRINTPLANIFH_

#include "glb_header_kernel.h"
char** console;
typedef struct CONVAL{
	int rows;
	int cols;
	int n_count;
	int r_count;
	int b_count;
	int x_count;
	int e_count;
	int cpus_tot;
	int cpus_uso;
	int mp; //multiprogramacion
	int mpu; //multiprogramacion usada
	int tem; //tamanio en memorio (peso de r+b+x)
	char* p_new;
	char* p_ready;
	char* p_exec;
	char* p_blocked;
	char* p_exit;
	char* ios;
	int last_500_chrs;
	t_list *procs;
	t_dictionary* sems;
	t_dictionary* v_c;
	bool enable;
} t_console_values;

t_console_values con_vals;

void print_mensaje(const char*, bool);
void print_estado();
void crear_printable_area();

#endif
