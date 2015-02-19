/*
 * config_cpu.c
 *
 *  Created on: 30/04/2014
 *      Author: utnso
 */

#include "config_cpu.h"
#include <stdlib.h>
#include <commons/string.h>

t_config* config;

void open_config(char* path){
	config = config_create(path);
}

void close_current_config(){
	config_destroy(config);
}

int puertoUMV(){
	return config_get_int_value(config, "PUERTO_UMV");
}

int puertoKernel(){
	return config_get_int_value(config, "PUERTO_KERNEL");
}

char* ipUMV(){
	return config_get_string_value(config,"IP_UMV");
}

char* ipKernel(){
	return config_get_string_value(config,"IP_KERNEL");
}
