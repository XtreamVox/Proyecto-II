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

         if (ComprobarComando(comando, orden, argumento1, argumento2) != 0){
            printf("Comando desconocido\n");
            continue;
        }

	     if (strcmp(orden,"dir")==0) 
        {
            Directorio(&directorio,&ext_blq_inodos);
            continue;
        } 
        else if (strcmp(orden, "bytemaps") == 0)
        {
            Printbytemaps(&ext_bytemaps);
        } 
        else if (strcmp(orden, "info") == 0)
        {
            Directorio(directorio, &ext_blq_inodos);
        }
	    else if (strcmp(orden, "remove") == 0)
        {
            printf("Ingresa el nombre del archivo a remover: ");
            fgets(argumento1, LONGITUD_COMANDO, stdin);
            eliminarSaltoLinea(argumento1); // Eliminar el salto de línea

            if (Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent))
            {
                printf("Error eliminando el archivo\n");
            }
        }
        else if (strcmp(orden, "imprimir") == 0)
        {
            printf("Ingresa el nombre del archivo a imprimir: ");
            fgets(argumento1, LONGITUD_COMANDO, stdin);
            eliminarSaltoLinea(argumento1); // Eliminar el salto de línea
            if (Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1))
            {
                printf("Error imprimiendo el archivo\n");
            }
        }
	else if (strcmp(orden, "copy") == 0)
        {
            printf("Ingresa el nombre del archivo a donde se quiera copiar: ");
            fgets(argumento1, LONGITUD_COMANDO, stdin);
            eliminarSaltoLinea(argumento1); // Eliminar el salto de línea
            printf("Ingresa el nombre del archivo del que se quiere copiar: ");
            fgets(argumento2, LONGITUD_COMANDO, stdin);
            eliminarSaltoLinea(argumento2); // Eliminar el salto de línea

            if (Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent))
            {
                printf("Error copiando el archivo\n");
            }
        }
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

void eliminarSaltoLinea(char *cadena)
{
    size_t longitud = strlen(cadena);
    if (longitud > 0 && cadena[longitud - 1] == '\n')
    {
        cadena[longitud - 1] = '\0';
    }
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

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps)
{
    printf("Bytemap de bloques:\n");
    for (int i = 0; i < 25; i++)
    {
        printf("%d", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\nBytemap de inodos:\n");
    for (int i = 0; i < MAX_INODOS; i++)
    {
        printf("%d", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup)
{
    printf("Inodos: %u\n", psup->s_inodes_count);
    printf("Bloques: %u\n", psup->s_blocks_count);
    printf("Bloques libres: %u\n", psup->s_free_blocks_count);
    printf("Inodos libres: %u\n", psup->s_free_inodes_count);
    printf("Primer bloque de datos: %u\n", psup->s_first_data_block);
    printf("Tamaño del bloque: %u\n", psup->s_block_size);
}

EXT_ENTRADA_DIR *BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre)
{
    for (int i = 0; i < MAX_FICHEROS; i++)
    {
        // Asegúrate de que dir_nfich no está vacío
        if (directorio[i].dir_nfich[0] != '\0' && strcmp(directorio[i].dir_nfich, nombre) == 0)
        {
            return &directorio[i];
        }
    }
    return NULL; // Si no lo encuentra, retorna NULL
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, FILE *fich)
{
    EXT_ENTRADA_DIR *entrada = BuscaFich(directorio, inodos, nombre);
    if (entrada == NULL) // Verificamos si el archivo fue encontrado
    {
        printf("Error: Archivo no encontrado.\n");
        return -1;
    }

    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[entrada->dir_inodo];
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO && inodo->i_nbloque[i] != NULL_BLOQUE; i++)
    {
        ext_bytemaps->bmap_bloques[inodo->i_nbloque[i]] = 0; // Liberar el bloque
        inodo->i_nbloque[i] = NULL_BLOQUE;
    }

    inodo->size_fichero = 0;
    ext_bytemaps->bmap_inodos[entrada->dir_inodo] = 0; // Liberar el inodo

    // Limpiar la entrada de directorio
    memset(entrada->dir_nfich, 0, LEN_NFICH);
    entrada->dir_inodo = NULL_INODO;

    ext_superblock->s_free_blocks_count++;
    ext_superblock->s_free_inodes_count++;
    return 0;
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre)
{
    EXT_ENTRADA_DIR *entrada = BuscaFich(directorio, inodos, nombre);
    if (entrada == NULL) // Verificamos si el archivo fue encontrado
    {
        printf("Error: Archivo no encontrado.\n");
        return -1;
    }

    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[entrada->dir_inodo];
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO && inodo->i_nbloque[i] != NULL_BLOQUE; i++)
    {
        printf("%.*s", SIZE_BLOQUE, memdatos[inodo->i_nbloque[i] - PRIM_BLOQUE_DATOS].dato);
    }
    printf("\n");
    return 0;
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich)
{
    EXT_ENTRADA_DIR *origen = BuscaFich(directorio, inodos, nombreorigen);
    if (origen == NULL)
    {
        printf("Error: Archivo origen no encontrado.\n");
        return -1;
    }

    // Verificar si el archivo destino ya existe
    EXT_ENTRADA_DIR *destino = BuscaFich(directorio, inodos, nombredestino);
    if (destino == NULL)
    {
        // Si no existe, crear una nueva entrada en el directorio
        for (int i = 0; i < MAX_FICHEROS; i++)
        {
            if (directorio[i].dir_inodo == NULL_INODO)
            {
                destino = &directorio[i];
                strncpy(destino->dir_nfich, nombredestino, LEN_NFICH - 1);
                destino->dir_nfich[LEN_NFICH - 1] = '\0';

                // Asignar un inodo libre
                for (int j = 0; j < MAX_INODOS; j++)
                {
                    if (ext_bytemaps->bmap_inodos[j] == 0)
                    {
                        destino->dir_inodo = j;
                        ext_bytemaps->bmap_inodos[j] = 1; // Marcar inodo como ocupado
                        break;
                    }
                }
                break;
            }
        }

        if (destino == NULL)
        {
            printf("Error: No hay espacio en el directorio para el archivo destino.\n");
            return -1;
        }
    }
    else
    {
        // Si el archivo ya existe, limpiar su contenido actual
        EXT_SIMPLE_INODE *inodo_destino = &inodos->blq_inodos[destino->dir_inodo];
        for (int i = 0; i < MAX_NUMS_BLOQUE_INODO && inodo_destino->i_nbloque[i] != NULL_BLOQUE; i++)
        {
            int bloque = inodo_destino->i_nbloque[i];
            ext_bytemaps->bmap_bloques[bloque] = 0; // Marcar bloque como libre
            memset(memdatos[bloque - PRIM_BLOQUE_DATOS].dato, 0, SIZE_BLOQUE);
            inodo_destino->i_nbloque[i] = NULL_BLOQUE;
        }
        inodo_destino->size_fichero = 0;
    }

    // Copiar datos desde el origen al destino
    EXT_SIMPLE_INODE *inodo_origen = &inodos->blq_inodos[origen->dir_inodo];
    EXT_SIMPLE_INODE *inodo_destino = &inodos->blq_inodos[destino->dir_inodo];
    inodo_destino->size_fichero = inodo_origen->size_fichero;

    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++)
    {
        if (inodo_origen->i_nbloque[i] == NULL_BLOQUE)
            break;

        for (int j = PRIM_BLOQUE_DATOS; j < MAX_BLOQUES_PARTICION; j++)
        {
            if (ext_bytemaps->bmap_bloques[j] == 0)
            {
                ext_bytemaps->bmap_bloques[j] = 1; // Marcar bloque como ocupado
                inodo_destino->i_nbloque[i] = j;

                memcpy(memdatos[j - PRIM_BLOQUE_DATOS].dato,
                       memdatos[inodo_origen->i_nbloque[i] - PRIM_BLOQUE_DATOS].dato,
                       SIZE_BLOQUE);
                break;
            }
        }
    }

    return 0;
}


void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich)
{
    fseek(fich, 3 * SIZE_BLOQUE, SEEK_SET);
    fwrite(directorio, SIZE_BLOQUE, 1, fich);
    fseek(fich, 2 * SIZE_BLOQUE, SEEK_SET);
    fwrite(inodos, SIZE_BLOQUE, 1, fich);
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich)
{
    fseek(fich, SIZE_BLOQUE, SEEK_SET);
    fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich);
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich)
{
    fseek(fich, 0, SEEK_SET);
    fwrite(ext_superblock, SIZE_BLOQUE, 1, fich);
}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich)
{
    fseek(fich, 4 * SIZE_BLOQUE, SEEK_SET);
    fwrite(memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fich);
}