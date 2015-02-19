#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include "shared_strs.h"

typedef struct sockaddr_in INTERNET_SOCKET;

int enviar_mensaje_a_servidor(int socket_descriptor, int id_MSG, t_KER_PRO_CPU_UMV *message);
int conectar_a_servidor(char* unaIP, int unPuerto);
void desconectar_de_servidor(int sock_descriptor);
t_KER_PRO_CPU_UMV* recibir_mensaje_de_servidor(int sock_descriptor);

