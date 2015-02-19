/*
 * testsUmv.c
 *
 *  Created on: 24/04/2014
 *      Author: utnso
 */
#include "testsUmv.h"

int correrTests() {

	CU_initialize_registry();
	agregar_tests();
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}

void test1() {
	printf("Test 1!, Crear Memoria\n");
	int cant = 1000;
	memoria = crear_memoria(cant);
	int cantDeSegmentos = list_size(listaDeSegmentos);
	CU_ASSERT_EQUAL(cantDeSegmentos, 1);
	t_segmento* seg = list_get(listaDeSegmentos, 0);
	CU_ASSERT_EQUAL(seg->size, cant);
	CU_ASSERT_EQUAL(seg->offset, 0);
}

void test2() {
	printf("Test 2!, crear segmento\n");
	int a;
	a = crearSegmento(23, 200);
	a = crearSegmento(2, 100);
	a = crearSegmento(23, 200);
	a = crearSegmento(2, 100);
	a = crearSegmento(23, 400);
	CU_ASSERT_NOT_EQUAL(a, -1)
	int b = crearSegmento(5, 200);
	CU_ASSERT_EQUAL(b, -1);
	destruirSegmentosDePrograma(2);
	compactacion();
	b = crearSegmento(5, 200);
	mostrarSegmentos(listaDeSegmentos);
	CU_ASSERT_NOT_EQUAL(a, -1)

}

void agregar_tests() {
	CU_pSuite archivo = CU_add_suite("Archivo", inicializar, limpiar);
	CU_add_test(archivo, "test_contar_devuelve_menos1_si_el_archivo_no_existe",
			test1);
	CU_add_test(archivo, "test_contar_devuelve_0_si_no_hay_ocurrencias", test2);
	return;
}

int inicializar() {
	inicializar_log("UMV.log", "UMV");
	habilitar_log_debug();
	levantarConfiguracion("/home/utnso/git/tp-2014-1c-alloca2/UMV/UMV.conf");
	return EXIT_SUCCESS;
}

int limpiar() {
	return EXIT_SUCCESS;
}
