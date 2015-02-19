/*
 * sincro.c
 *
 *  Created on: 20/06/2014
 *      Author: utnso
 */
#include "sincro.h"

void preLectura() {
	pthread_mutex_lock(&mutex);
	readcount++;
	if (readcount == 1) {
		pthread_mutex_lock(&wrt);
	}
	pthread_mutex_unlock(&mutex);
	return;
}
void postLectura() {
	pthread_mutex_lock(&mutex);
	readcount--;
	if (readcount == 0) {
		pthread_mutex_unlock(&wrt);
	}
	pthread_mutex_unlock(&mutex);
	return;
}
