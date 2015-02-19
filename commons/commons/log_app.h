#include "log.h"

t_log* log_i;
t_log* log_d;
t_log* log_t;
t_log* log_w;
t_log* log_e;

void inicializar_log(char* file_name, char *app_name);
void finalizar_log();
void cambiar_output(void (*render_output)(const char*,bool));
void log_debug_message (const char* message);
void log_info_message (const char* message);
void log_trace_message (const char* message);
void log_warning_message (const char* message);
void log_error_message (const char* message);
void habilitar_log_error ();
void deshabilitar_log_error ();
void habilitar_log_debug ();
void deshabilitar_log_debug ();
void habilitar_log_info ();
void deshabilitar_log_info ();
void habilitar_log_warn ();
void deshabilitar_log_warn ();
void habilitar_log_trace ();
void deshabilitar_log_trace ();
void habilitar_print_console ();
void deshabilitar_print_console ();
