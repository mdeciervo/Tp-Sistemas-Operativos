#include "shared_strs.h"

char* id_a_mensaje(int id) {
	switch (id) {
	case HANDSHAKE_PROGRAMA_PLP: {
		return "HANDSHAKE_PROGRAMA_PLP";
	}
	case HANDSHAKE_CPU_PCP: {
		return "HANDSHAKE_CPU_PCP";
	}
	case HANDSHAKE_CPU_UMV: {
		return "HANDSHAKE_CPU_UMV";
	}
	case HANDSHAKE_PLP_UMV: {
		return "HANDSHAKE_PLP_UMV";
	}
	case IMPRIMIR_TEXTO: {
		return "IMPRIMIR_TEXTO";
	}
	case IMPRIMIR: {
		return "IMPRIMIR";
	}
	case CREAR_SEGMENTOS: {
		return "CREAR_SEGMENTOS";
	}
	case EJECUTAR_PROGRAMA: {
		return "EJECUTAR_PROGRAMA";
	}
	case OBTENER_VALOR: {
		return "OBTENER_VALOR";
	}
	case GRABAR_VALOR: {
		return "GRABAR_VALOR";
	}
	case WAIT: {
		return "WAIT";
	}
	case SIGNAL: {
		return "SIGNAL";
	}
	case ENTRADA_SALIDA: {
		return "ENTRADA_SALIDA";
	}
	case SOLICITAR_BYTES: {
		return "SOLICITAR_BYTES";
	}
	case ENVIAR_BYTES: {
		return "ENVIAR_BYTES";
	}
	case SOLICTUD_INSTRUCCION: {
		return "SOLICTUD_INSTRUCCION";
	}
	case CAMBIO_PROCESO_ACTIVO: {
		return "CAMBIO_PROCESO_ACTIVO";
	}
	case FINALIZAR_EJECUCION: {
		return "FINALIZAR_EJECUCION";
	}
	case DESTRUIR_SEGMENTOS: {
		return "DESTRUIR_SEGMENTOS";
	}
	case NOTIFICACION_QUANTUM: {
		return "NOTIFICACION_QUANTUM";
	}
	case ENTRADA_INDICE: {
		return "ENTRADA_INDICE";
	}
	case FINALIZACION_EJECUCION_PROGRAMA: {
		return "FINALIZACION_EJECUCION_PROGRAMA";
	}
	case DESCONEXION_CPU: {
		return "DESCONEXION_CPU";
	}
	default: {
		return "DESCONOCIDOOOO(?)";
	}
	}
}

void calcular_tamanio_shared_str(t_KER_PRO_CPU_UMV *shared_str) {
	shared_str->buffer_long = shared_str->tamanio_bytes + 1;
	shared_str->codigo_long = strlen(shared_str->codigo) + 1;
	shared_str->id_dispositivo_long = strlen(shared_str->id_dispositivo) + 1;
	shared_str->id_sem_long = strlen(shared_str->id_sem) + 1;
	shared_str->id_var_comp_long = strlen(shared_str->id_var_comp) + 1;
	shared_str->mensaje_long = strlen(shared_str->mensaje) + 1;
	shared_str->texto_long = strlen(shared_str->texto) + 1;

	shared_str->gen_msg.size_str = sizeof(shared_str->buffer_long)
			+ shared_str->buffer_long + sizeof(shared_str->codigo_long)
			+ shared_str->codigo_long + sizeof(shared_str->id_dispositivo_long)
			+ shared_str->id_dispositivo_long + sizeof(shared_str->id_sem_long)
			+ shared_str->id_sem_long + sizeof(shared_str->id_var_comp_long)
			+ shared_str->id_var_comp_long + sizeof(shared_str->mensaje_long)
			+ shared_str->mensaje_long + sizeof(shared_str->texto_long)
			+ shared_str->texto_long +

			sizeof(shared_str->OK) + sizeof(shared_str->PCB)
			+ sizeof(shared_str->base_bytes) + sizeof(shared_str->base_segmento)
			+ sizeof(shared_str->gen_msg) + sizeof(shared_str->identificador_cpu)
			+ sizeof(shared_str->indice) + sizeof(shared_str->offset_bytes)
			+ sizeof(shared_str->quantum) + sizeof(shared_str->retardo)
			+ sizeof(shared_str->tamanio_bytes)
			+ sizeof(shared_str->tamanio_segmento)
			+ sizeof(shared_str->utilizacion) + sizeof(shared_str->valor)
			+ sizeof(shared_str->PID);
}

char* serializar_shared_str(t_KER_PRO_CPU_UMV *shared_str) {
	char *serializedPackage = malloc(shared_str->gen_msg.size_str);

	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(shared_str->buffer_long);
	memcpy(serializedPackage + offset, &(shared_str->buffer_long),
			size_to_send);
	offset += size_to_send;

	size_to_send = shared_str->buffer_long;
	memcpy(serializedPackage + offset, shared_str->buffer, size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->codigo_long);
	memcpy(serializedPackage + offset, &(shared_str->codigo_long),
			size_to_send);
	offset += size_to_send;

	size_to_send = shared_str->codigo_long;
	memcpy(serializedPackage + offset, shared_str->codigo, size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->id_dispositivo_long);
	memcpy(serializedPackage + offset, &(shared_str->id_dispositivo_long),
			size_to_send);
	offset += size_to_send;

	size_to_send = shared_str->id_dispositivo_long;
	memcpy(serializedPackage + offset, shared_str->id_dispositivo,
			size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->id_sem_long);
	memcpy(serializedPackage + offset, &(shared_str->id_sem_long),
			size_to_send);
	offset += size_to_send;

	size_to_send = shared_str->id_sem_long;
	memcpy(serializedPackage + offset, shared_str->id_sem, size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->id_var_comp_long);
	memcpy(serializedPackage + offset, &(shared_str->id_var_comp_long),
			size_to_send);
	offset += size_to_send;

	size_to_send = shared_str->id_var_comp_long;
	memcpy(serializedPackage + offset, shared_str->id_var_comp, size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->mensaje_long);
	memcpy(serializedPackage + offset, &(shared_str->mensaje_long),
			size_to_send);
	offset += size_to_send;

	size_to_send = shared_str->mensaje_long;
	memcpy(serializedPackage + offset, shared_str->mensaje, size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->texto_long);
	memcpy(serializedPackage + offset, &(shared_str->texto_long), size_to_send);
	offset += size_to_send;

	size_to_send = shared_str->texto_long;
	memcpy(serializedPackage + offset, shared_str->texto, size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->OK);
	memcpy(serializedPackage + offset, &(shared_str->OK), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->PCB);
	memcpy(serializedPackage + offset, &(shared_str->PCB), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->base_bytes);
	memcpy(serializedPackage + offset, &(shared_str->base_bytes), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->base_segmento);
	memcpy(serializedPackage + offset, &(shared_str->base_segmento),
			size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->gen_msg);
	memcpy(serializedPackage + offset, &(shared_str->gen_msg), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->identificador_cpu);
	memcpy(serializedPackage + offset, &(shared_str->identificador_cpu),
			size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->indice);
	memcpy(serializedPackage + offset, &(shared_str->indice), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->offset_bytes);
	memcpy(serializedPackage + offset, &(shared_str->offset_bytes),
			size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->quantum);
	memcpy(serializedPackage + offset, &(shared_str->quantum), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->retardo);
	memcpy(serializedPackage + offset, &(shared_str->retardo), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->tamanio_bytes);
	memcpy(serializedPackage + offset, &(shared_str->tamanio_bytes),
			size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->tamanio_segmento);
	memcpy(serializedPackage + offset, &(shared_str->tamanio_segmento),
			size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->utilizacion);
	memcpy(serializedPackage + offset, &(shared_str->utilizacion),
			size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->valor);
	memcpy(serializedPackage + offset, &(shared_str->valor), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(shared_str->PID);
	memcpy(serializedPackage + offset, &(shared_str->PID), size_to_send);

	return serializedPackage;
}

int deserializar_buffer_a_shared_str(t_KER_PRO_CPU_UMV* package,
		int socket_cliente) {
	int status;
	int nbytes = 0;
	int buffer_size;
	char *buffer = malloc(buffer_size = sizeof(uint32_t));

	//-----------------
	uint32_t buffer_long;
	status = recv(socket_cliente, buffer, sizeof(package->buffer_long), 0);
	nbytes += status;
	memcpy(&(buffer_long), buffer, buffer_size);
	if (!status)
		return 0;
	package->buffer = (char*) malloc(sizeof(char) * buffer_long);
	status = recv(socket_cliente, package->buffer, buffer_long, 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	uint32_t codigo_long;
	status = recv(socket_cliente, buffer, sizeof(package->codigo_long), 0);
	nbytes += status;
	memcpy(&(codigo_long), buffer, buffer_size);
	if (!status)
		return 0;
	package->codigo = (char*) malloc(sizeof(char) * codigo_long);
	status = recv(socket_cliente, package->codigo, codigo_long, 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	uint32_t id_dispositivo_long;
	status = recv(socket_cliente, buffer, sizeof(package->id_dispositivo_long),
			0);
	nbytes += status;
	memcpy(&(id_dispositivo_long), buffer, buffer_size);
	if (!status)
		return 0;
	package->id_dispositivo = (char*) malloc(
			sizeof(char) * id_dispositivo_long);
	status = recv(socket_cliente, package->id_dispositivo, id_dispositivo_long,
			0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	uint32_t id_sem_long;
	status = recv(socket_cliente, buffer, sizeof(package->id_sem_long), 0);
	nbytes += status;
	memcpy(&(id_sem_long), buffer, buffer_size);
	if (!status)
		return 0;
	package->id_sem = (char*) malloc(sizeof(char) * id_sem_long);
	status = recv(socket_cliente, package->id_sem, id_sem_long, 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	uint32_t id_var_comp_long;
	status = recv(socket_cliente, buffer, sizeof(package->id_var_comp_long), 0);
	nbytes += status;
	memcpy(&(id_var_comp_long), buffer, buffer_size);
	if (!status)
		return 0;
	package->id_var_comp = (char*) malloc(sizeof(char) * id_var_comp_long);
	status = recv(socket_cliente, package->id_var_comp, id_var_comp_long, 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	uint32_t mensaje_long;
	status = recv(socket_cliente, buffer, sizeof(package->mensaje_long), 0);
	nbytes += status;
	memcpy(&(mensaje_long), buffer, buffer_size);
	if (!status)
		return 0;
	package->mensaje = (char*) malloc(sizeof(char) * mensaje_long);
	status = recv(socket_cliente, package->mensaje, mensaje_long, 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	uint32_t texto_long;
	status = recv(socket_cliente, buffer, sizeof(package->texto_long), 0);
	nbytes += status;
	memcpy(&(texto_long), buffer, buffer_size);
	if (!status)
		return 0;
	package->texto = (char*) malloc(sizeof(char) * texto_long);
	status = recv(socket_cliente, package->texto, texto_long, 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	status = recv(socket_cliente, buffer, sizeof(package->OK), 0);
	nbytes += status;
	memcpy(&(package->OK), buffer, buffer_size);
	if (!status)
		return 0;
	//-----------------

	//-----------------
	status = recv(socket_cliente, &(package->PCB), sizeof(package->PCB), 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	status = recv(socket_cliente, &(package->base_bytes),
			sizeof(package->base_bytes), 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	status = recv(socket_cliente, &(package->base_segmento),
			sizeof(package->base_segmento), 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	status = recv(socket_cliente, &(package->gen_msg), sizeof(package->gen_msg),
			0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	status = recv(socket_cliente, &(package->identificador_cpu),
			sizeof(package->identificador_cpu), 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	status = recv(socket_cliente, &(package->indice), sizeof(package->indice),
			0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	status = recv(socket_cliente, &(package->offset_bytes),
			sizeof(package->offset_bytes), 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	status = recv(socket_cliente, &(package->quantum), sizeof(package->quantum),
			0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	status = recv(socket_cliente, &(package->retardo), sizeof(package->retardo),
			0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	status = recv(socket_cliente, &(package->tamanio_bytes),
			sizeof(package->tamanio_bytes), 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	status = recv(socket_cliente, &(package->tamanio_segmento),
			sizeof(package->tamanio_segmento), 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	status = recv(socket_cliente, &(package->utilizacion),
			sizeof(package->utilizacion), 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	status = recv(socket_cliente, &(package->valor), sizeof(package->valor), 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	//-----------------
	status = recv(socket_cliente, &(package->PID), sizeof(package->PID), 0);
	nbytes += status;
	if (!status)
		return 0;
	//-----------------

	free(buffer);

	return nbytes;
}

t_KER_PRO_CPU_UMV obtener_nueva_shared_str() {
	t_KER_PRO_CPU_UMV nueva;

	nueva.buffer = "\0";
	nueva.codigo = "\0";
	nueva.id_dispositivo = "\0";
	nueva.id_sem = "\0";
	nueva.id_var_comp = "\0";
	nueva.mensaje = "\0";
	nueva.texto = "\0";
	nueva.PID = 0;
	nueva.OK = 0;
	nueva.PCB.PC = 0;
	nueva.PCB.codSeg = 0;
	nueva.PCB.cursorStack = 0;
	nueva.PCB.id = 0;
	nueva.PCB.indCod = 0;
	nueva.PCB.indEtiq = 0;
	nueva.PCB.stackSeg = 0;
	nueva.PCB.tamContext = 0;
	nueva.base_bytes = 0;
	nueva.base_segmento = 0;
	nueva.gen_msg.disconnected = 0;
	nueva.gen_msg.id_MSJ = 0;
	nueva.gen_msg.socket_descriptor = 0;
	nueva.gen_msg.socket_descriptor_server = 0;
	nueva.identificador_cpu = 0;
	nueva.indice = 0;
	nueva.offset_bytes = 0;
	nueva.quantum = 0;
	nueva.retardo = 0;
	nueva.tamanio_bytes = 0;
	nueva.tamanio_segmento = 0;
	nueva.utilizacion = 0;
	nueva.valor = 0;
	return nueva;
}

