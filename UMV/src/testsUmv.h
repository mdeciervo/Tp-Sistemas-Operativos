/*
 * testsUmv.h
 *
 *  Created on: 24/04/2014
 *      Author: utnso
 */

#ifndef TESTSUMV_H_
#define TESTSUMV_H_
#include <errno.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include "memoria.h"

int correrTests(void);
void test1(void);
void test2(void);
int limpiar();
int inicializar();
void agregar_tests();

#endif /* TESTSUMV_H_ */
