#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include "print_kernel.h"
#include <unistd.h>

void clear_screen()
{
  const char* CLEAR_SCREE_ANSI = "\e[1;1H\e[2J";
  write(STDOUT_FILENO,CLEAR_SCREE_ANSI,12);
}

void crear_sep_line(int from, int row, char chr) {
	int j;
	for (j = from; j < con_vals.cols; j++)
		console[row][j] = chr;
}
void crear_template() {
	int i, j, queues_from, cap_q;

	cap_q = con_vals.cols - 11;
	queues_from = 6;
	con_vals.tem = 0;
	con_vals.mpu = 0;
	con_vals.cpus_uso=0;
	con_vals.n_count = 0;
	con_vals.r_count = 0;
	con_vals.b_count = 0;
	con_vals.x_count = 0;
	con_vals.e_count = 0;
	con_vals.p_new = string_new();
	con_vals.p_ready = string_new();
	con_vals.p_blocked = string_new();
	con_vals.p_exec = string_new();
	con_vals.p_exit = string_new();

	char aux[100];
	for (i = 0; i < con_vals.procs->elements_count; i++) {
		t_Programa* proc = (t_Programa*)list_get(con_vals.procs,i);

		if (proc->cq == 'R' || proc->cq == 'B' || proc->cq == 'X') {
			con_vals.tem += proc->peso;
			con_vals.mpu++;
		}
		sprintf(aux, "%d-%s(%d);", proc->PID, proc->nombre,
				proc->peso);

		if (proc->cq == 'N') {
			con_vals.n_count++;
			string_append(&con_vals.p_new, aux);
		}

		if (proc->cq == 'R') {
			con_vals.r_count++;
			string_append(&con_vals.p_ready, aux);
		}

		if (proc->cq == 'B') {
			con_vals.b_count++;

			string_append(&con_vals.p_blocked, aux);
		}

		if (proc->cq == 'X') {
			con_vals.x_count++;

			con_vals.cpus_uso=con_vals.cpus_uso+1;
			sprintf(aux, "%d-%s(%d)|c:%d;", proc->PID, proc->nombre,
					proc->peso, proc->cpu);
			string_append(&con_vals.p_exec, aux);
		}

		if (proc->cq == 'E') {
			con_vals.e_count++;
			sprintf(aux, "%d-%s(%d)|q:%llu;", proc->PID, proc->nombre,
					proc->peso, proc->q);

			string_append(&con_vals.p_exit, aux);
		}

	}


	sprintf(console[1], "CPUs: u[%d] - i[%d] | MP: %d/%d | TEM: %d",
			con_vals.cpus_uso, con_vals.cpus_tot - con_vals.cpus_uso,
			con_vals.mpu, con_vals.mp, con_vals.tem);
	sprintf(console[2], "iOS:%s", con_vals.ios);

	char* sems = string_new();
	char* vcs = string_new();
	void each_sem(char* key, t_sem_pcp* sem) {
		sprintf(aux, "%s(%d);",sem->id_sem,sem->valor);
		string_append(&sems,aux);
	}
	void each_vc(char* key, t_VC* vc) {
		sprintf(aux, "%s(%d);",key,vc->valor);
		string_append(&vcs,aux);
	}

	dictionary_iterator(con_vals.sems, (void*)each_sem);
	sprintf(console[3],"SEMs: %s",sems);
	free(sems);
	dictionary_iterator(con_vals.v_c, (void*)each_vc);
	sprintf(console[4],"VCs: %s",vcs);
	free(vcs);

	for (j = 0; j < con_vals.cols; j++) {
		console[queues_from][j] = ' ';
		console[queues_from + 1][j] = ' ';
		console[queues_from + 2][j] = ' ';
		console[queues_from + 3][j] = ' ';
		console[queues_from + 4][j] = ' ';
	}
	sprintf(console[queues_from], "N:%d [%s]", con_vals.n_count,
			con_vals.p_new);
	sprintf(console[queues_from + 1], "R:%d [%s]", con_vals.r_count,
			con_vals.p_ready);
	sprintf(console[queues_from + 2], "B:%d [%s]", con_vals.b_count,
			con_vals.p_blocked);
	sprintf(console[queues_from + 3], "X:%d [%s]", con_vals.x_count,
			con_vals.p_exec);
	sprintf(console[queues_from + 4], "E:%d [%s]", con_vals.e_count,
			con_vals.p_exit);
	free(con_vals.p_new);
	free(con_vals.p_ready);
	free(con_vals.p_blocked);
	free(con_vals.p_exec);
	free(con_vals.p_exit);

	sprintf(console[queues_from + 6], "%s|%s|", "#Pro", "#CPU");
	int count_r_x_b = 0;
	int prev_row_aux = 0;


	for (i = 0; i < con_vals.procs->elements_count; i++) {
		t_Programa* proc = (t_Programa*)list_get(con_vals.procs,i);

		if (queues_from + 7 + count_r_x_b * 2 + 2 >= con_vals.last_500_chrs - 1)
			break;

		if (proc->cq == 'X'
				|| ((proc->cq == 'B' || proc->cq == 'R') && proc->q > 0)) {
			prev_row_aux = proc->r;
			proc->r = queues_from + 7 + (count_r_x_b * 2);

			console[proc->r][9] = '|';

			sprintf(console[proc->r+1],"%d    %d   ",proc->PID,proc->cpu);
			console[proc->r+1][9]='|';
			if (proc->q - ((proc->q / cap_q) * cap_q) == 0) {
				for (j = 0; j < cap_q; j++) {
					console[proc->r + 1][j + 12] = ' ';
				}
			}
			else if (prev_row_aux > 0 && prev_row_aux!=proc->r) // si no es inicio de time line y el row donde estaba antes tenia data
					{
				//copio el pasado
				for (j = 0; j < cap_q; j++) {
					console[proc->r + 1][j + 12] =
							console[prev_row_aux + 1][j + 12];
				}
			}

			for (j = 0; j < cap_q; j++) {
				console[proc->r][j + 12] = ' ';
			}

			sprintf(console[proc->r] + 10, "%llu-%llu",
					(proc->q / cap_q) * cap_q, proc->q - proc->ios);
			if ((proc->cq == 'X'
					|| proc->cq == 'R'
					|| (proc->cq == 'B' && proc->io == ' '))
					&& proc->q > 0) {
				console[proc->r + 1][proc->q
						- ((proc->q / cap_q) * cap_q) + 9] = '_';
			} else if (proc->cq == 'B' && proc->io != ' ') {
				console[proc->r + 1][proc->q
						- ((proc->q / cap_q) * cap_q) + 9] = proc->io;
				proc->io = ' ';
			}
			count_r_x_b++;
		}
	}
	console[queues_from + 7 + (count_r_x_b * 2)][9] = '|';
	crear_sep_line(10, queues_from + 7 + count_r_x_b * 2, '_');

//    /* clean footer*/
     for (i = queues_from+7+count_r_x_b*2+1; i < con_vals.last_500_chrs - 1; i++)
    	 for (j = 0; j < con_vals.cols;j++)
    		 console[i][j]=' ';

}

void crear_printable_area() {
	int i;

	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	con_vals.rows = w.ws_row - 2;
	if (con_vals.rows <= 0)
		con_vals.rows = 70;
	con_vals.cols = w.ws_col;
	if (con_vals.cols <= 0)
		con_vals.cols = 70;

	con_vals.last_500_chrs = con_vals.rows - (500 / con_vals.cols);

	console = (char**) malloc(con_vals.rows * sizeof(char*));
	for (i = 0; i < con_vals.rows; i++) {
		console[i] = (char*) malloc(con_vals.cols * sizeof(char));
	}

	clear_screen();

	crear_sep_line(0, 5, '-');
	crear_sep_line(0, 11, '-');
	crear_sep_line(0,con_vals.last_500_chrs-1,'-');

}

//positioning
void gotoxy(int x, int y) {
	printf("%c[%d;%df", 0x1B, y, x);
}

void print_estado() {
	if(!con_vals.enable)
		return;
	int i,j;
	crear_template();
	/* print each row of the array */
	for (i = 0; i < con_vals.last_500_chrs - 1; i++) {
		gotoxy(0, i);
		printf("%c[2K", 27);
		for(j=0;j<con_vals.cols;j++)
		{
			if(console[i][j]=='\0')
				printf("%c",' ');
			else
				printf("%c",console[i][j]);
		}
		printf("\n");
	}
}

void print_mensaje(const char* message, bool is_error) {

	int i;

	sprintf(console[con_vals.last_500_chrs],"%s",message);
    for(i=con_vals.last_500_chrs-1;i<con_vals.rows;i++)
	{
    	gotoxy(0, i);
    	printf("%c[2K", 27);
		printf("%s\n",console[i]);
	}
}
