//Practica 2
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100
#define NUM_BLOQUES 100
#define TRUE 1
#define FALSE 0
#define RANGO_BLOQUES 25
#define EXISTENTE 2

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps); //Hecho
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2); //Hecho
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup); //Hecho
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre); //Hecho
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);//Hecho
void Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,char *nombreantiguo, char *nombrenuevo); //Hecho
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre); //Hecho
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
	const char espacioBarraN[] = " \n";	
	char* token = strtok(strcomando, espacioBarraN);
	
	
	if((strcmp(token,"info")==0) || (strcmp(token,"bytemaps")==0) || (strcmp(token,"dir")==0) || (strcmp(token,"salir")==0))
	{
		strcpy(orden, token);
	}
	else if((strcmp(token,"rename")==0) || (strcmp(token,"copy")==0))
	{
		strcpy(orden, token);
		strcpy(argumento1, strtok(NULL, espacioBarraN));
		strcpy(argumento2, strtok(NULL, espacioBarraN));
	}
	else if((strcmp(token,"imprimir")==0) || (strcmp(token,"remove")==0))
	{
		strcpy(orden, token);
		strcpy(argumento1, strtok(NULL, espacioBarraN));
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
	char* aux = (char*)malloc(sizeof(char)*LEN_NFICH);
	int comprobarNombres = 0;
	int indice = 0;
	int acum = 20;
	int i = 0;
	int j = 0;
	
	for(i = 0; i < MAX_FICHEROS; i++)
	{
		if(inodos->blq_inodos[i].size_fichero != 0)
		{
			for(j = 0; directorio->dir_nfich[j + acum] > MAX_FICHEROS; j++)
			{
				strncat(aux,&directorio->dir_nfich[j + acum], 1);
			}
			
			// for(j = 0; aux[j] != '\0'; j++)
			// {
				// printf("%c\tvs\t%c\n", aux[j], nombre[j]);
			// }
			
			// printf("%s\tvs\t%s\n", aux, nombre);
			comprobarNombres = strncmp(aux, nombre, sizeof(nombre));
			// printf("Comparacion: %d\n", comprobarNombres);
			memset(aux, '\0', sizeof(aux));
			
			if(comprobarNombres == 0)
			{
				indice = acum;
				// printf("Indice: %d\n", indice);
				return indice;
			}
			
			acum += 20;
		}
	}
	return -1;
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos)
{
	int i = 0;
	int j = 0;
	int acum = 20;
	for(i = 0; i < MAX_FICHEROS; i++)
	{
		if(inodos->blq_inodos[i].size_fichero != 0)
		{
			for(j = 0; directorio->dir_nfich[j + acum] > MAX_FICHEROS; j++)
			{
				printf("%c", directorio->dir_nfich[j + acum]);
			}
			acum += 20;
			
			printf("\t");
			printf("tamaño:%d\tinodo:%d\tbloques:",inodos->blq_inodos[i].size_fichero, i);
			
			// j debe ser menor que numero maximo de bloques por inodo, y el bloque debe ser distinto del bloque nulo
			for(j = 0; j < MAX_NUMS_BLOQUE_INODO && (inodos->blq_inodos[i].i_nbloque[j] != NULL_BLOQUE); j++)
			{
				printf("%d ", inodos->blq_inodos[i].i_nbloque[j]);
			}
			printf("\n");
		}
	}
}

void Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,char *nombreantiguo, char *nombrenuevo)
{
	int nombreValido = TRUE;
	int i = 0;
	// printf("El nombre que voy a pasar es: %s\n", nombreantiguo);
	int posicionParaRenombrar1 = BuscaFich(directorio, inodos, nombreantiguo);
	int posicionParaRenombrar2 = BuscaFich(directorio, inodos, nombrenuevo);
	
	if(posicionParaRenombrar1 == -1)
	{
		nombreValido = FALSE;
	}
	else if(posicionParaRenombrar2 != -1)
	{
		nombreValido = EXISTENTE;
	}
	
	// printf("El valor de nombre valido es: %d\n", nombreValido);
	//--------------------------------------------------------------------------
	if(nombreValido == TRUE)
	{
		for(i = 0; i < LEN_NFICH; i++)
		{
			memset(&directorio->dir_nfich[i + posicionParaRenombrar1], nombrenuevo[i], 1);
		}
		// memset(&directorio->dir_nfich[i], '\0', 1);
	}
	else if(nombreValido == EXISTENTE)
	{
		printf("ERROR: El fichero %s ya existe\n", nombrenuevo);
	}
	else
	{
		printf("ERROR: El fichero %s no encontrado\n", nombreantiguo);
	}
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre)
{
	//Quiero buscar el inodo correspondiente al fichero
	//Una vez tengo ese inodo busco los bloques correspondientes a ese inodo y los imprimo en orden.
	
	int ficheroExiste = BuscaFich(directorio, inodos, nombre);
	int inodoPorMirar;
	int i = 0;
	int j = 0;
	int acum = 20;
	char aux[LEN_NFICH];
	int comprobarNombres = 1;
	int operacionExitosa = FALSE;
	// printf("Fichero existe: %d\n", ficheroExiste);
	
	if(ficheroExiste == -1)
	{
		printf("ERROR: Fichero %s no encontrado\n", nombre);
	}
	else
	{		
		for(i = 0; i < MAX_FICHEROS; i++)
		{
			if(inodos->blq_inodos[i].size_fichero != 0)
			{
				for(j = 0; directorio->dir_nfich[j + acum] > MAX_FICHEROS; j++)
				{
					aux[j] = directorio->dir_nfich[j + acum];
				}
				
				printf("%s\tvs\t%s\n", aux, nombre);
				comprobarNombres = strncmp(aux, nombre, sizeof(nombre));
				
				if(comprobarNombres == 0)
				{
					inodoPorMirar = i;
				}
				
				memset(aux, '\0', sizeof(aux));
				
				acum += 20;
			}
		}
		// printf("Ver inodo: %d\n", inodoPorMirar);
		for(j = 0; j < MAX_NUMS_BLOQUE_INODO && (inodos->blq_inodos[inodoPorMirar].i_nbloque[j] != NULL_BLOQUE); j++)
		{
			// printf("Bloque:%d\n", inodos->blq_inodos[inodoPorMirar].i_nbloque[j]);
			for(i = 0; i < SIZE_BLOQUE; i++)
			{
				// printf("%X\n", inodos->blq_inodos[inodoPorMirar].i_nbloque[j] * SIZE_BLOQUE + i);
				printf("%c", memdatos->dato[(inodos->blq_inodos[inodoPorMirar].i_nbloque[j] - (NUM_BLOQUES-MAX_BLOQUES_DATOS)) * SIZE_BLOQUE + i]);
			}
		}
		printf("\n");
		
		operacionExitosa = TRUE;
	}
	
	return operacionExitosa;
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,char *nombre,FILE *fich)
{
	int operacionExitosa = FALSE;
	int posicionParaRenombrar = BuscaFich(directorio, inodos, nombre);
	char unCero[2] = "\0";
	int acum = 20;
	int i = 0;
	int j = 0;
	char* aux = (char*)malloc(sizeof(char)*LEN_NFICH);
	int comprobarNombres = 0;
	
	if(posicionParaRenombrar == -1)
	{
		printf("ERROR: Fichero %s no encontrado\n", nombre);
	}
	else
	{
		for(i = 0; i < MAX_FICHEROS; i++)
		{
			if(inodos->blq_inodos[i].size_fichero != 0)
			{
				for(j = 0; directorio->dir_nfich[j + acum] > MAX_FICHEROS; j++)
				{
					strncat(aux,&directorio->dir_nfich[j + acum], 1);
				}
				
				// for(j = 0; aux[j] != '\0'; j++)
				// {
					// printf("%c\tvs\t%c\n", aux[j], nombre[j]);
				// }
				
				// printf("%s\tvs\t%s\n", aux, nombre);
				comprobarNombres = strncmp(aux, nombre, sizeof(nombre));
				// printf("Comparacion: %d\n", comprobarNombres);
				memset(aux, '\0', sizeof(aux));
				
				if(comprobarNombres == 0)
				{
					strcpy(&ext_bytemaps->bmap_inodos[i], &unCero);
					ext_superblock->s_free_inodes_count++;
					inodos->blq_inodos[i].size_fichero = 0;
				}
				
				for(j = 0; j < MAX_NUMS_BLOQUE_INODO && (inodos->blq_inodos[i].i_nbloque[j] != NULL_BLOQUE) && comprobarNombres == 0; j++)
				{
					strcpy(&ext_bytemaps->bmap_bloques[inodos->blq_inodos[i].i_nbloque[j]], &unCero);
					ext_superblock->s_free_blocks_count++;
				}
				acum += 20;
			}
			
		}
		
		acum = 20;
		i = 0;
		
		printf("El texto que voy a copiar es: %X\n", directorio->dir_nfich[i + posicionParaRenombrar+acum]);
		printf("Me voy a posicionar en: %d\n", posicionParaRenombrar);
		while(directorio->dir_nfich[posicionParaRenombrar+acum] > MAX_FICHEROS)
		{
			for(i = 0; i < LEN_NFICH; i++)
			{
				memset(&directorio->dir_nfich[i + posicionParaRenombrar + acum - 20], directorio->dir_nfich[i + posicionParaRenombrar+acum], LEN_NFICH);
			}
			acum += 20;
		}
		operacionExitosa = TRUE;
	}
	
	return operacionExitosa;
}

int Copiar(EXT_ENTRADA_DIR *directorio,EXT_BLQ_INODOS *inodos,EXT_BYTE_MAPS *ext_bytemaps,EXT_SIMPLE_SUPERBLOCK *ext_superblock,EXT_DATOS *memdatos,char *nombreorigen,char *nombredestino,FILE *fich)
{
	int operacionExitosa = FALSE;
	int nombreValido = TRUE;
	
	int ocupado = 1;
	char *ocupadoS = memset(ocupadoS, ocupado, 1);
	
	int comprobarNombres;
	int indiceDelQueCopio = 0;
	int i = 0;
	int j = 0;
	int contador = 0;
	int acum = 20;
	char* aux = (char*)malloc(sizeof(char)*LEN_NFICH);
	int posicionParaCopiar1 = BuscaFich(directorio, inodos, nombreorigen);
	int posicionParaCopiar2 = BuscaFich(directorio, inodos, nombredestino);
	int* bloques = (int*)malloc(sizeof(int));
	
	if(posicionParaCopiar1 == -1)
	{
		nombreValido = FALSE;
	}
	else if(posicionParaCopiar2 != -1)
	{
		nombreValido = EXISTENTE;
	}
	
	// printf("El valor de nombre valido es: %d\n", nombreValido);
	//--------------------------------------------------------------------------
	if(nombreValido == TRUE)
	{
		for(i = 0; i < MAX_FICHEROS; i++)
		{
			if(inodos->blq_inodos[i].size_fichero != 0)
			{
				for(j = 0; directorio->dir_nfich[j + acum] > MAX_FICHEROS; j++)
				{
					strncat(aux,&directorio->dir_nfich[j + acum], 1);
				}
				
				printf("%s vs %s\n", aux, nombreorigen);
				comprobarNombres = strncmp(aux, nombreorigen, sizeof(nombreorigen));
				memset(aux, '\0', sizeof(aux));
				
				if(comprobarNombres == 0)
				{
					indiceDelQueCopio = i;
					for(j = 0; j < MAX_NUMS_BLOQUE_INODO && (inodos->blq_inodos[indiceDelQueCopio].i_nbloque[j] != NULL_BLOQUE); j++)
					{
						bloques[contador] = inodos->blq_inodos[indiceDelQueCopio].i_nbloque[j];
						contador++;
						bloques = (int*)realloc(bloques, sizeof(int)*(contador+1));
					}
				}
				acum += 20;
			}
		}
		
		bloques[contador] = '\0';
		
		//Copiamos los datos sin el contenido
		//--------------------------------------------------------------------------------------
		//Cuento cuantos bloques debo copiar
		
		int x = 0;		//Para llevar los indices de: inodos->blq_inodos[i].i_nbloque[x]
		
		for(i = 0; i < MAX_INODOS; i++)
		{
			// Para un inodo libre compruebo
			if(ext_bytemaps->bmap_inodos[i] == 0)
			{
				strcpy(&ext_bytemaps->bmap_inodos[i], ocupadoS);
				inodos->blq_inodos[i].size_fichero = inodos->blq_inodos[indiceDelQueCopio].size_fichero;
				
				printf("Prueba de que esta ocupado: %d\n", ext_bytemaps->bmap_inodos[i]);
				printf("El inodo %d esta libre\n", i);
				
				for(j = 0; j < MAX_BLOQUES_PARTICION && contador >= 0; j++)
				{
					if(ext_bytemaps->bmap_bloques[j] == 0)
					{
						printf("El bloque %d esta libre\n", j);
						inodos->blq_inodos[i].i_nbloque[x] = j;
						strcpy(&ext_bytemaps->bmap_bloques[j], ocupadoS);
						ext_superblock->s_free_blocks_count--;
						
						memcpy(&memdatos->dato[(inodos->blq_inodos[i].i_nbloque[x] - (NUM_BLOQUES-MAX_BLOQUES_DATOS)) * SIZE_BLOQUE], &memdatos->dato[(inodos->blq_inodos[indiceDelQueCopio].i_nbloque[bloques[x]] - (NUM_BLOQUES-MAX_BLOQUES_DATOS)) * SIZE_BLOQUE], SIZE_BLOQUE);
						printf("HOLaaaaaaaaaa\n");
						x++;
					}
				}
				i = MAX_INODOS;
			}
		}
		
		for(i = 0; i < MAX_NUMS_BLOQUE_INODO; i++)
		{
			inodos->blq_inodos[i].i_nbloque[x] = NULL_BLOQUE;
		}
		//--------------------------------------------------------------------------------------
		
		//Rellenamos los bloques con los datos correspondientes
		
		
		memset(&directorio->dir_nfich[acum], nombredestino, 1);
		
		operacionExitosa = TRUE;
	}
	else if(nombreValido == EXISTENTE)
	{
		printf("ERROR: El fichero %s ya existe\n", nombredestino);
	}
	else
	{
		printf("ERROR: El fichero %s no encontrado\n", nombreorigen);
	}
	
	return operacionExitosa;
}

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich)
{
	printf("Grabarinodosydirectorio\n");
}
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich)
{
	printf("GrabarByteMaps\n");
}
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich)
{
	printf("GrabarSuperBloque\n");
}
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich)
{
	printf("GrabarDatos\n");
}

int main()
{
	char *comando[LONGITUD_COMANDO];
	char *orden[LONGITUD_COMANDO];
	char *argumento1[LONGITUD_COMANDO];
	char *argumento2[LONGITUD_COMANDO];
	// char comando[LONGITUD_COMANDO];
	// char orden[LONGITUD_COMANDO];
	// char argumento1[LONGITUD_COMANDO];
	// char argumento2[LONGITUD_COMANDO];
	
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
		} while (ComprobarComando(comando,orden,argumento1,argumento2) !=0);
		
	    if (strcmp(orden,"dir")==0) 
		{ //Arreglar lo del nombre
			Directorio(&directorio,&ext_blq_inodos);
			continue;
        }
		else if (strcmp(orden,"info")==0) 
		{
			LeeSuperBloque(&ext_superblock);
			continue;
        }
		else if (strcmp(orden,"bytemaps")==0) 
		{
			Printbytemaps(&ext_bytemaps);
			continue;
        }
		else if (strcmp(orden,"rename")==0) 
		{
			Renombrar(&directorio,&ext_blq_inodos,argumento1,argumento2);
			continue;
        }
		else if (strcmp(orden,"imprimir")==0) 
		{
           Imprimir(&directorio, &ext_blq_inodos, memdatos, argumento1);
           continue;
        }
		else if (strcmp(orden,"remove")==0) 
		{
           Borrar(&directorio,&ext_blq_inodos,&ext_bytemaps, &ext_superblock, argumento1, fichero);
           continue;
        }
		else if (strcmp(orden,"copy")==0) 
		{
           Copiar(&directorio,&ext_blq_inodos,&ext_bytemaps, &ext_superblock,memdatos,argumento1,argumento2,fichero);
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
        if (strcmp(orden,"salir")==0)
		{
           GrabarDatos(memdatos,fichero);
           fclose(fichero);
           return 0;
        }
    }
	
	return 0;
}