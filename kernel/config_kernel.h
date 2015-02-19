#include <commons/config.h>

//LECTURA ARCHIVO CONFIG
void open_config(char* path);
void close_current_config();
int PUERTO_PROG();
int PUERTO_CPU();
int PUERTO_UMV();
char* IP_UMV();
int QUANTUM();
int RETARDO();
int MULTIPROGRAMACION();
int STACK_SIZE();
char** VALOR_SEMAFORO();
char** SEMAFOROS();
char** HIO();
char** ID_HIO();
char** VARIABLES_COMPARTIDAS();
int ENABLE_LOG();
