/*
 * config_cpu.h
 *
 *  Created on: 30/04/2014
 *      Author: utnso
 */

#ifndef CONFIG_CPU_H_
#define CONFIG_CPU_H_

#include <commons/config.h>

//LECTURA ARCHIVO CONFIG
void open_config(char* path);
void close_current_config();
int puertoUMV();
int puertoKernel();
char* ipUMV();
char* ipKernel();


#endif /* CONFIG_CPU_H_ */
