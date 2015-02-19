#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include "collections/list.h"
#include "shared_strs.h"

char identidad_socket_server[256][100];
int espera_activa;

typedef struct sockaddr_in INTERNET_SOCKET;

void delegar_conexion(int descriptor);
void eliminar_una_conexion(fd_set *master, int fd);
int aceptar_una_nueva_conexion(fd_set *master, int current_fdmax, int newfd);
int abrir_conexion_servidor (int puerto, t_KER_PRO_CPU_UMV* (*handler_msgs_ptr)(t_KER_PRO_CPU_UMV*));
int abrir_conexion_servidor_local (int puerto, t_KER_PRO_CPU_UMV* (*handler_msgs_ptr)(t_KER_PRO_CPU_UMV*));
int enviar_mensaje(int socket_descriptor_server, int id_MSG, t_KER_PRO_CPU_UMV *message);
void desconectar(int sock_or_port);

void deshabilitar_espera_activa();


t_KER_PRO_CPU_UMV* recibir_mensaje(int sock_descriptor);
