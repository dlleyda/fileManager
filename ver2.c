//Practica 2
#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100
#define TRUE 1
#define FALSE 0
#define RANGO_BLOQUES 25

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps)
{	
	int i = 0;
	printf("Inodos :");
	for(i = 0; i < MAX_INODOS; i++)
	{
		printf("%d ",ext_bytemaps->bmap_inodos[i]);
	}
	printf("\n");
	printf("Bloques [0-%d] :", RANGO_BLOQUES);
	for(i = 0; i <= RANGO_BLOQUES; i++)
	{
		printf("%d ",ext_bytemaps->bmap_bloques[i]);
	}
	printf("\n");
}

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2)
{
	int noValido = FALSE;
	
	if((strcmp(strcomando,"info\n")==0) || (strcmp(strcomando,"bytemaps\n")==0) || (strcmp(strcomando,"dir\n")==0) || (strcmp(strcomando,"rename\n")==0) || (strcmp(strcomando,"remove\n")==0) || (strcmp(strcomando,"copy\n")==0) || (strcmp(strcomando,"salir\n")==0))
	{
		strcpy(orden, strcomando);
	}
	else
	{
		printf("ERROR: Comando ilegal [bytemaps, copy, dir, info, imprimir, rename, remove, salir]\n");
		noValido = TRUE;
	}
	
	return noValido;
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup)
{
	printf("Bloque %d Bytes\ninodos particion = %d\ninodos libres = %d\nBloques particion = %d\nBloques libres = %d\nPrimer bloque de datos = %d\n",psup->s_block_size,psup->s_inodes_count,psup->s_free_inodes_count,psup->s_blocks_count,psup->s_free_blocks_count,psup->s_first_data_block);
}

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre)
{
	return 0;
}
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos)
{
	printf("Hola\n");
}
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,char *nombreantiguo, char *nombrenuevo)
{
	return 0;
}
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre)
{
	return 0;
}
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,char *nombre,  FILE *fich)
{
	return 0;
}
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich)
{
	return 0;
}
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich)
{
	printf("Hola\n");
}
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich)
{
	printf("Hola\n");
}
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich)
{
	printf("Hola\n");
}
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich)
{
	printf("Hola\n");
}

int main()
{
	// char *comando[LONGITUD_COMANDO];
	// char *orden[LONGITUD_COMANDO];
	// char *argumento1[LONGITUD_COMANDO];
	// char *argumento2[LONGITUD_COMANDO];
	char comando[LONGITUD_COMANDO];
	char orden[LONGITUD_COMANDO];
	char argumento1[LONGITUD_COMANDO];
	char argumento2[LONGITUD_COMANDO];
	
	int i,j;
	unsigned long int m;
    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
    int entradadir;
    int grabardatos;
    FILE *fichero;
    
    // Lectura del fichero completo de una sola vez
	
    
    fichero = fopen("particion.bin","r+b");
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fichero);    
    
    
    memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
    memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
    memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
    memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
    
    // Buce de tratamiento de comandos
    for (;;)
	{
		do 
		{
			printf (">> ");
			fflush(stdin);
			fgets(comando, LONGITUD_COMANDO, stdin);
			printf(comando);
		} while (ComprobarComando(comando,orden,argumento1,argumento2) !=0);
		
	    if (strcmp(orden,"dir\n")==0) 
		{
           Directorio(directorio,&ext_blq_inodos);
           continue;
        }
		else if (strcmp(orden,"info\n")==0) 
		{
           LeeSuperBloque(&ext_superblock);
           continue;
        }
		else if (strcmp(orden,"bytemaps\n")==0) 
		{
           Printbytemaps(&ext_bytemaps);
           continue;
        }
		
        // Escritura de metadatos en comandos rename, remove, copy     
        Grabarinodosydirectorio(directorio,&ext_blq_inodos,fichero);
        GrabarByteMaps(&ext_bytemaps,fichero);
        GrabarSuperBloque(&ext_superblock,fichero);
		
        if (grabardatos)
          GrabarDatos(memdatos,fichero);
	  
        grabardatos = 0;
        //Si el comando es salir se habrán escrito todos los metadatos
        //faltan los datos y cerrar
        if (strcmp(orden,"salir\n")==0)
		{
           GrabarDatos(memdatos,fichero);
           fclose(fichero);
           return 0;
        }
    }
	
	return 0;
}