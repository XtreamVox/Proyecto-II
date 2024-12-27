/*
Simulador de ficheros tipo Linux
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define SIZE_BLOQUE 512
#define MAX_BLOQUES_PARTICION 100
#define MAX_INODOS 24
#define MAX_NUMS_BLOQUE_INODO 7
#define LEN_NFICH 32
