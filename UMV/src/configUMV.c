/*
 * configUMV.c
 *
 *  Created on: 22/04/2014
 *      Author: utnso
 */
#include "configUMV.h"

configUmv* levantarConfiguracion(char *rutaArchivo) {

	configuracionUmv = malloc(sizeof(configUmv));
	t_config *config = config_create(rutaArchivo);
	configuracionUmv->algoritmo = config_get_string_value(config, "Algoritmo");
	configuracionUmv->retardo = config_get_int_value(config, "Retardo");
	configuracionUmv->tamanioMemoria = config_get_int_value(config,
			"TamanioMemoria");
	configuracionUmv->puerto = config_get_int_value(config, "Puerto");
	configuracionUmv->ip = config_get_string_value(config, "Ip");
	return configuracionUmv;

}

void mostrarConfiguracion(void) {
	sprintf(log_text,
			"Configuracion levantada correctamente:\n-Tamanio Memoria: %i\n-Retardo: %i\n-Algoritmo: %s\n-Ip: %s\n-Puerto: %i",
			configuracionUmv->tamanioMemoria, configuracionUmv->retardo,
			configuracionUmv->algoritmo, configuracionUmv->ip,
			configuracionUmv->puerto);
	log_info_message(log_text);
	return;
}

