#include "config_kernel.h"
#include "stdlib.h"
#include <commons/string.h>

t_config* config;

void open_config(char* path)
{
	config = config_create(path);
}

void close_current_config()
{
	config_destroy(config);
}

int PUERTO_PROG()
{
	return config_get_int_value(config, "PUERTO_PROG");
}

int PUERTO_UMV()
{
	return config_get_int_value(config, "PUERTO_UMV");
}

char* IP_UMV(){
	return config_get_string_value(config, "IP_UMV");
}

int PUERTO_CPU()
{
	return config_get_int_value(config, "PUERTO_CPU");
}
int QUANTUM()
{
	return config_get_int_value(config, "QUANTUM");
}
int RETARDO()
{
	return config_get_int_value(config, "RETARDO");
}
int MULTIPROGRAMACION()
{
	return config_get_int_value(config, "MULTIPROGRAMACION");
}

char** VALOR_SEMAFORO()
{
	return config_get_array_value(config, "VALOR_SEMAFORO");
}

char** SEMAFOROS()
{
	return config_get_array_value(config, "SEMAFOROS");
}

char** HIO()
{
	return config_get_array_value(config, "HIO");
}

char** ID_HIO()
{
	return config_get_array_value(config, "ID_HIO");
}

char** VARIABLES_COMPARTIDAS()
{
	return config_get_array_value(config, "VARIABLES_COMPARTIDAS");
}

int STACK_SIZE()
{
	return config_get_int_value(config, "STACK_SIZE");
}

int ENABLE_LOG()
{
	return config_get_int_value(config, "ENABLE_LOG");
}


