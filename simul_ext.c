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

//Definicion de las estructuras

typedef struct EXT_SIMPLE_SUPERBLOCK {
    unsigned int s_inodes_count;
    unsigned int s_blocks_count;
    unsigned int s_free_blocks_count;
    unsigned int s_free_inodes_count;
    unsigned int s_first_data_block;
    unsigned int s_block_size;
    unsigned char s_relleno[SIZE_BLOQUE - 6 * sizeof(unsigned int)];
} EXT_SIMPLE_SUPERBLOCK;

typedef struct EXT_BYTE_MAPS {
    unsigned char bmap_bloques[MAX_BLOQUES_PARTICION];
    unsigned char bmap_inodos[MAX_INODOS];
    unsigned char bmap_relleno[SIZE_BLOQUE - (MAX_BLOQUES_PARTICION + MAX_INODOS) * sizeof(char)];
} EXT_BYTE_MAPS;

typedef struct EXT_SIMPLE_INODE {
    unsigned int size_fichero;
    unsigned short int i_nbloque[MAX_NUMS_BLOQUE_INODO];
} EXT_SIMPLE_INODE;

typedef struct EXT_ENTRADA_DIR {
    char dir_nfich[LEN_NFICH];
    unsigned short int dir_inodo;
} EXT_ENTRADA_DIR;
