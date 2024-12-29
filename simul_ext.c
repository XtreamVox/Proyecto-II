#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
             EXT_DATOS *memdatos, char *nombre)
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main()
{
	 char *comando[LONGITUD_COMANDO];
	 char *orden[LONGITUD_COMANDO];
	 char *argumento1[LONGITUD_COMANDO];
	 char *argumento2[LONGITUD_COMANDO];
	 
	 int i,j;
	 unsigned long int m;
     EXT_SIMPLE_SUPERBLOCK ext_superblock;
     EXT_BYTE_MAPS ext_bytemaps;
     EXT_BLQ_INODOS ext_blq_inodos;
     EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
     EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
     EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
    int grabardatos = 0;
    FILE *fent = fopen("particion.bin", "r+b");
    if (!fent)
    {
        perror("Error abriendo el archivo de partición");
        return 1;
    }

    // Lectura del fichero y guardado de su contenido
     fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);        
     memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
     memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
     memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
     memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
     memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
     
     // Buce de tratamiento de comandos
     for (;;){
		 do {
		 printf (">> ");
		 fflush(stdin);
		 fgets(comando, LONGITUD_COMANDO, stdin);
		 } while (ComprobarComando(comando,orden,argumento1,argumento2) !=0);
	     if (strcmp(orden,"dir")==0) {
            Directorio(&directorio,&ext_blq_inodos);
            continue;
            }
         ...
         // Escritura de metadatos en comandos rename, remove, copy     
         Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fent);
         GrabarByteMaps(&ext_bytemaps,fent);
         GrabarSuperBloque(&ext_superblock,fent);
         if (grabardatos)
           GrabarDatos(&memdatos,fent);
         grabardatos = 0;
         //Si el comando es salir se habrÃ¡n escrito todos los metadatos
         //faltan los datos y cerrar
         if (strcmp(orden,"salir")==0){
            GrabarDatos(&memdatos,fent);
            fclose(fent);
            return 0;
         }
     }
}

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2)
{
    // Inicializa las cadenas
    memset(orden, 0, LONGITUD_COMANDO);
    memset(argumento1, 0, LONGITUD_COMANDO);
    memset(argumento2, 0, LONGITUD_COMANDO);

    // Extrae el comando y argumentos
    sscanf(strcomando, "%s %s %s", orden, argumento1, argumento2);

    // Lista de comandos válidos
    if (strcmp(orden, "dir") == 0 || strcmp(orden, "rename") == 0 ||
        strcmp(orden, "remove") == 0 || strcmp(orden, "copy") == 0 ||
        strcmp(orden, "salir") == 0 || strcmp(orden, "info") == 0 ||
        strcmp(orden, "bytemaps") == 0 || strcmp(orden, "imprimir") == 0)
    {
        return 0; // Comando válido
    }

    return -1; // Comando desconocido
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos)
{
    for (int i = 0; i < MAX_FICHEROS; i++)
    {
        if (directorio[i].dir_inodo != NULL_INODO && strcmp(directorio[i].dir_nfich, ".") != 0)
        {
            EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[directorio[i].dir_inodo];
            printf("Nombre: %s, Inodo: %u, Tamaño: %u, Bloques: ",
                   directorio[i].dir_nfich,
                   directorio[i].dir_inodo,
                   inodo->size_fichero);

            if (inodo->size_fichero > 0) // Mostrar bloques solo si el tamaño del archivo es mayor que 0
            {
                for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++)
                {
                    if (inodo->i_nbloque[j] != NULL_BLOQUE)
                    {
                        printf("%d ", inodo->i_nbloque[j]);
                    }
                }
            }
            else
            {
                printf("Ninguno");
            }
            printf("\n");
        }
    }
}