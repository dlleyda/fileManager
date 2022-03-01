#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100
#define TRUE 1
#define FALSE 0
#define RANGO_BLOQUES 25
#define EXISTENTE 2

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

int buscarInodoLibre(EXT_BYTE_MAPS *ext_bytemaps) //Buscamos el primer inodo libre
{
	int i = 0;
	for(i = 0; i < MAX_INODOS; i++)
	{
		if(ext_bytemaps->bmap_inodos[i] == 0) // Para un inodo libre compruebo
		{
			return i;
		}
	}
}

int buscarHuecoDir(EXT_ENTRADA_DIR* directorio)
{
	//Declaramos variables
	int i = 0;
	int contador = 0;
	for(i = 0; i < MAX_FICHEROS; i++) //Recorremos todos los ficheros y contamos todos los que no tengan un inodo nulo
	{
		if((directorio[i].dir_nfich[0] < 'a' || directorio[i].dir_nfich[0] > 'z') && directorio[i].dir_nfich[0] != '.') //No debe contener letras ni puntos
		{
			if(directorio[i].dir_nfich[0] < 'A' || directorio[i].dir_nfich[0] > 'Z')
				return i; //Devolvemos el hueco
		}
	}
}

void separar(char* strcomando, char *orden, char *argumento1, char *argumento2)
{ //Separamos strcomando en orden, argumento1 y argumento2
	char* token = strtok(strcomando, " \n"); //Separamos por ' ' y por '\n'
	int contador = 0;
	
	while(token!=NULL)
	{
		if(contador == 0)
		{
			strcpy(orden,token);
			strcpy(argumento1, "\0"); //Seteamos argumento 1 y 2 en \0 para posteriormente poder compararlo y ver si hay suficientes argumentos
			strcpy(argumento2, "\0");
		}
		else if(contador == 1)
		{
			strcpy(argumento1, token);
		}
		else
		{
			strcpy(argumento2, token);
		}
		contador++;
		token = strtok(NULL, " \n");
	}
}

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps)
{	
	//Declaramos variables
	int i = 0;
	printf("Inodos: ");
	//Recorremos todos los inodos imprimiendo uno a uno de los bytemaps
	for(i = 0; i < MAX_INODOS; i++)
	{
		printf("%d ",ext_bytemaps->bmap_inodos[i]);
	}
	printf("\n");
	printf("Bloques [0-%d] :", RANGO_BLOQUES);
	//Recorremos tantos bloques como los definidos, imprimiendo uno a uno de los bytemaps
	for(i = 0; i <= RANGO_BLOQUES; i++)
	{
		printf("%d ",ext_bytemaps->bmap_bloques[i]);
	}
	printf("\n");
}

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2)
{
	//Declaramos variables
	int noValido = FALSE;
	//Separamos en tokens
	separar(strcomando, orden, argumento1, argumento2);
	int contador = 0;
	
	//Comparamos la orden con todas las posibilidades
	if(strcmp(strcomando,"\n")==0)
	{
		noValido = TRUE;
	}
	else if((strcmp(orden,"info")!=0) && (strcmp(orden,"bytemaps")!=0) && (strcmp(orden,"dir")!=0) && (strcmp(orden,"salir")!=0) && (strcmp(orden,"rename")!=0)
		&& (strcmp(orden,"copy")!=0) && (strcmp(orden,"imprimir")!=0) && (strcmp(orden,"remove")!=0))
	{
		printf("ERROR: Comando ilegal [bytemaps, copy, dir, info, imprimir, rename, remove, salir]\n");
		noValido = TRUE; //El booleano se pone en false
	}
	
	return noValido;
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup) //Imprimimos todo lo necesario del superbloque
{
	printf("Bloque %d Bytes\ninodos particion = %d\ninodos libres = %d\nBloques particion = %d\nBloques libres = %d\nPrimer bloque de datos = %d\n",psup->s_block_size,psup->s_inodes_count,psup->s_free_inodes_count,psup->s_blocks_count,psup->s_free_blocks_count,psup->s_first_data_block);
}

int BuscaFich2(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre)
{
	//Declaramos variables
	int i = 0;
	
	for(i = 0; i < MAX_FICHEROS; i++) //Recorremos todos los ficheros
	{
		if(directorio[i].dir_inodo != NULL_INODO && inodos->blq_inodos[directorio[i].dir_inodo].size_fichero > 0)
		{
			if(strcmp(directorio[i].dir_nfich, nombre) == 0)
			{
				return i;
			}
		}
	}
	return -1;
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos)
{
	//Declaramos variables
	int i = 0;
	int j = 0;
	
	for(i = 0; i < MAX_FICHEROS; i++) //Recorremos todos los ficheros
	{
		if(directorio[i].dir_inodo != NULL_INODO && inodos->blq_inodos[directorio[i].dir_inodo].size_fichero > 0)
		{
			printf("%s\t", directorio[i].dir_nfich); //Vamos imprimiendo caracter a caracter
			
			printf("tamaño:%d\tinodo:%d\tbloques:",inodos->blq_inodos[directorio[i].dir_inodo].size_fichero, directorio[i].dir_inodo); //Imprimimos los demás detalles
			
			// j debe ser menor que numero maximo de bloques por inodo, y el bloque debe ser distinto del bloque nulo
			for(j = 0; j < MAX_NUMS_BLOQUE_INODO && (inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j] != NULL_BLOQUE); j++)
			{
				printf("%d ", inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]);
			}
			printf("\n");
		}
	}
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,char *nombreantiguo, char *nombrenuevo) //Para renombrar tenemos que encontrar la posicion donde se encuentre el nombre del fichero antiguo
{
	int nombreValido = TRUE;
	int operacionExitosa = FALSE;
	int posicionParaRenombrar1 = BuscaFich2(directorio, inodos, nombreantiguo); //Vemos si existen para el control de errores
	int posicionParaRenombrar2 = BuscaFich2(directorio, inodos, nombrenuevo);
	
	if(posicionParaRenombrar1 == -1) //Este es el caso de que el fichero antiguo no existe/ no se ha encontrado
	{
		nombreValido = FALSE;
	}
	else if(posicionParaRenombrar2 != -1) //Este es el caso de que el nombre nuevo de fichero ya existe
	{
		nombreValido = EXISTENTE;
	}
	
	if(nombreValido == TRUE) //En el caso de que no haya habido ningun cambio, sustituimos letra a letra
	{
		strcpy(directorio[posicionParaRenombrar1].dir_nfich,nombrenuevo);
		operacionExitosa = TRUE; //La operacion ha sido exitosa
	}
	else if(nombreValido == EXISTENTE) //Printf correspondientes para el tratamiento de errores
	{
		printf("ERROR: El fichero %s ya existe\n", nombrenuevo);
	}
	else
	{
		printf("ERROR: El fichero %s no encontrado\n", nombreantiguo);
	}
	
	return operacionExitosa; //Devolvemos si la operacion ha tenido exito o no
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) //Para imprimir debemos ver qué bloques imprimir
{
	//Antes de nada vemos si el fichero existe, y declaramos variables
	int ficheroExiste = BuscaFich2(directorio, inodos, nombre);
	int i = 0;
	int j = 0;
	int operacionExitosa = FALSE;
	
	if(ficheroExiste == -1) //Control de errores
	{
		printf("ERROR: Fichero %s no encontrado\n", nombre);
	}
	else
	{
		int inodo = directorio[ficheroExiste].dir_inodo;
		
		for(j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) //Recorremos todos los bloques del inodo "calculado"
		{
			if(inodos->blq_inodos[inodo].i_nbloque[j] != NULL_BLOQUE)
			{
				for(i = 0; i < SIZE_BLOQUE; i++)
				{
					printf("%c", memdatos[inodos->blq_inodos[inodo].i_nbloque[j] - (PRIM_BLOQUE_DATOS)].dato[i]); //Imprimimos caracter a caracter de dicho bloque (debemos restar PRIM_BLOQUE_DATOS, porque son los bloques reservados para el superbloque, bmaps, inodos y directorio)
				}				
			}
		}
		printf("\n");
		
		operacionExitosa = TRUE; //La operacion ha sido exitosa
	}
	
	return operacionExitosa;
}

int Borrar(EXT_ENTRADA_DIR* directorio, EXT_BLQ_INODOS* inodos,EXT_BYTE_MAPS* ext_bytemaps, EXT_SIMPLE_SUPERBLOCK* ext_superblock,char* nombre, FILE* fich)
{
	//Declaramos las variables
	int indice_exists = BuscaFich2(directorio, inodos, nombre); //Buscamos si el fichero existe
	int operacionExitosa = FALSE;
	int inodo = 0;
	int k = 0;
	char cero[2] = { "\0" }; //Necesistamos esta variable para poder ponerla en los bytemaps

	if (indice_exists != -1) //En el caso de que exista:
	{
		int inodo = directorio[indice_exists].dir_inodo;
		
		strncpy(&ext_bytemaps->bmap_inodos[inodo], &cero, 1); //Ponemos en ese inodo el 0
		for (k = 0; k < MAX_NUMS_BLOQUE_INODO; k++)
		{
			if (inodos->blq_inodos[inodo].i_nbloque[k] != NULL_BLOQUE)
			{
				strncpy(&ext_bytemaps->bmap_bloques[inodos->blq_inodos[inodo].i_nbloque[k]], &cero, 1); //Ponemos en los bloques correspondientes el 0 para el bytempap
				inodos->blq_inodos[inodo].i_nbloque[k] = NULL_BLOQUE; //Además de setear dichos bloques a NULL_BLOQUE
				ext_superblock->s_free_blocks_count++; //Actualizamos contadores del superbloque
			}
		}
		memset(directorio[indice_exists].dir_nfich, '\0', LEN_NFICH); //Finalmente se nos quedará el último nombre de fichero duplicado, asi que lo borramos
		inodos->blq_inodos[inodo].size_fichero = 0; //Seteamos el tamaño del fichero a 0
		ext_superblock->s_free_inodes_count++; //Actualizamos contadores
		
		directorio[indice_exists].dir_inodo = NULL_INODO; //Seteamos el inodo a NULL_INODO
	
		operacionExitosa = TRUE; //La operacion ha tenido exito
	}
	else
	{
		printf("ERROR: Fichero %s no encontrado\n", nombre);
	}
	
	return operacionExitosa;
}

int Copiar(EXT_ENTRADA_DIR *directorio,EXT_BLQ_INODOS *inodos,EXT_BYTE_MAPS *ext_bytemaps,EXT_SIMPLE_SUPERBLOCK *ext_superblock,EXT_DATOS *memdatos,char *nombreorigen,char *nombredestino,FILE *fich)
{
	//Variables booleanas
	int operacionExitosa = FALSE;
	int nombreValido = TRUE;
	
	//Variables para poder comparar si está vacio o no el bloque o inodo en cuestion
	int ocupado = 1;
	char *ocupadoS = (char*)malloc(sizeof(char)*2);
	ocupadoS = memset(ocupadoS, ocupado, 1);
	
	//Variables necesarias para bucles principalmente
	int inodoLibre = 0;
	int inodoPorMirar = 0;
	int j = 0;
	int i = 0;
	int contador = 0;
	
	//Variables para comprobar la validez de los argumentos que nos han pasado
	int posicionParaCopiar1 = BuscaFich2(directorio, inodos, nombreorigen);
	int posicionParaCopiar2 = BuscaFich2(directorio, inodos, nombredestino);
	
	if(posicionParaCopiar1 == -1)
	{
		nombreValido = FALSE;
	}
	else if(posicionParaCopiar2 != -1)
	{
		nombreValido = EXISTENTE;
	}
	
	if(nombreValido == TRUE)
	{
		inodoPorMirar = directorio[posicionParaCopiar1].dir_inodo;
		inodoLibre = buscarInodoLibre(ext_bytemaps);
		
		// Contamos cuantos bloques tenemos que copiar
		for(j = 0; j < MAX_NUMS_BLOQUE_INODO; j++)
		{
			if(inodos->blq_inodos[inodoPorMirar].i_nbloque[j] != NULL_BLOQUE)
			{
				contador++;
				ext_superblock->s_free_blocks_count--; //Actualizamos contadores	
			}
		}	
		
		int x = 0;		//Para llevar los indices de: inodos->blq_inodos[i].i_nbloque[x]
		//Copiamos todos los datos y actualizamos el superbloque
		//Marcamos que en el bytemap de inodos que está ocupado, admeás de actualizar los distintos contadores.
		strncpy(&ext_bytemaps->bmap_inodos[inodoLibre], ocupadoS,1);
		inodos->blq_inodos[inodoLibre].size_fichero = inodos->blq_inodos[inodoPorMirar].size_fichero;
		ext_superblock->s_free_inodes_count--;
		
		//Hacemos lo mismo para los bloques
		for(j = 0; j < MAX_BLOQUES_PARTICION && contador > 0; j++)
		{
			if(ext_bytemaps->bmap_bloques[j] == 0) //Si encontramos un sitio libre
			{
				inodos->blq_inodos[inodoLibre].i_nbloque[x] = j; //Añadimos ese bloque al inodo
				strncpy(&ext_bytemaps->bmap_bloques[j], ocupadoS,1); //Marcamos en el bytemap qeu está ocupado
				//Copiamos los datos del bloque del fichero origen al bloque del fichero destino
				for(i = 0; i < SIZE_BLOQUE; i++)
				{
					strncpy(&memdatos[inodos->blq_inodos[inodoLibre].i_nbloque[x] - (PRIM_BLOQUE_DATOS)].dato[i], &memdatos[inodos->blq_inodos[inodoPorMirar].i_nbloque[x] - (PRIM_BLOQUE_DATOS)].dato[i], 1);
				}
				//Actualizamos contadores que nos sirven para ir rellenando más con cierto limite, en este caso el limite está en el numero de bloques que hallamos contado en la variable "contador"
				x++;
				contador--;
			}
		}
		int buscarHuecoEnDir = buscarHuecoDir(directorio); //Buscamos un hueco en el array de directorio
		directorio[buscarHuecoEnDir].dir_inodo = inodoLibre; //Seteamos en la posicion libre el inodo
		strcpy(directorio[buscarHuecoEnDir].dir_nfich, nombredestino); //Seteamos en esa posicion el nombre
		operacionExitosa = TRUE; //La operacion ha tenido exito
	}
	else if(nombreValido == EXISTENTE)
	{
		printf("ERROR: El fichero %s ya existe\n", nombredestino);
	}
	else
	{
		printf("ERROR: El fichero %s no encontrado\n", nombreorigen);
	}
	
	free(ocupadoS); //Liberamos memoria
	return operacionExitosa;
}

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich)
{
	fseek(fich, SIZE_BLOQUE*3, SEEK_SET); //Nos ponemos el posicion correspondiente al directorio
	fwrite(directorio, SIZE_BLOQUE, 1, fich); //Escribimos la inforamcion de directorio
	fseek(fich, SIZE_BLOQUE*2, SEEK_SET); //Nos ponemos el posicion correspondiente de inodos
	fwrite(inodos, SIZE_BLOQUE, 1, fich); //Escribimos la inforamcion de inodos
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich)
{
	fseek(fich, SIZE_BLOQUE, SEEK_SET); //Nos ponemos en la posicion correspondiente a los bytemaps
	fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich); //Escribimos la inforamcion de los bytemaps
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich)
{
	fseek(fich, 0, SEEK_SET); //Nos ponemos en la posicion correspondiente al superbloque
	fwrite(ext_superblock, SIZE_BLOQUE, 1, fich); //Escribimos la inforamcion del superbloque
}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich)
{
	fseek(fich, SIZE_BLOQUE*4, SEEK_SET); //Nos ponemos en la posicion correspondiente de todos los datos
	fwrite(memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fich); //Escribimos la inforamcion de los datos
}

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
    int entradadir;
    int grabardatos = FALSE;
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
		} while (ComprobarComando(comando,orden,argumento1,argumento2) != 0);
		
	    if (strcmp(orden,"dir")==0) 
		{
			if(strcmp(argumento1, "\0") == 0 && strcmp(argumento2, "\0") == 0) //No debe haber argumentos
			{
				Directorio(&directorio,&ext_blq_inodos);
			}
			else
			{
				printf("Demasiados argumentos...\n");
			}
			continue;
        }
		else if (strcmp(orden,"info")==0) 
		{
			if(strcmp(argumento1, "\0") == 0 && strcmp(argumento2, "\0") == 0) //No debe haber argumentos
			{
				LeeSuperBloque(&ext_superblock);
			}
			else
			{
				printf("Demasiados argumentos...\n");
			}
			continue;
        }
		else if (strcmp(orden,"bytemaps")==0) 
		{
			if(strcmp(argumento1, "\0") == 0 && strcmp(argumento2, "\0") == 0) //No debe haber argumentos
			{
				Printbytemaps(&ext_bytemaps);
			}
			else
			{
				printf("Demasiados argumentos...\n");
			}
			continue;
        }
		else if (strcmp(orden,"rename")==0) 
		{
			if(strcmp(argumento1, "\0") != 0 && strcmp(argumento2, "\0") != 0) //Debe haber 2 argumentos
			{
				grabardatos = Renombrar(&directorio,&ext_blq_inodos,argumento1,argumento2);
			}
			else
			{
				printf("Faltan parámetros de entrada\n");
			}
			continue;
        }
		else if (strcmp(orden,"imprimir")==0) 
		{
			if(strcmp(argumento1, "\0") != 0) //Debe haber 1 argumento
			{
				grabardatos = Imprimir(&directorio, &ext_blq_inodos, memdatos, argumento1);
			}
			else
			{
				printf("Faltan parámetros de entrada\n");
			}
			continue;
        }
		else if (strcmp(orden,"remove")==0) 
		{
			if(strcmp(argumento1, "\0") != 0) //Debe haber 1 argumento
			{
				grabardatos = Borrar(&directorio,&ext_blq_inodos,&ext_bytemaps, &ext_superblock, argumento1, fichero);
			}
			else
			{
				printf("Faltan parámetros de entrada\n");
			}
			continue;
        }
		else if (strcmp(orden,"copy")==0) 
		{
			if(strcmp(argumento1, "\0") != 0 && strcmp(argumento2, "\0") != 0) //Debe haber 2 argumentos
			{
				grabardatos = Copiar(&directorio,&ext_blq_inodos,&ext_bytemaps, &ext_superblock,memdatos,argumento1,argumento2,fichero);				
			}
			else
			{
				printf("Faltan parámetros de entrada\n");
			}
			continue;
        }
        // Escritura de metadatos en comandos rename, remove, copy     
        GrabarSuperBloque(&ext_superblock,fichero);
        Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fichero);
        GrabarByteMaps(&ext_bytemaps,fichero);
		
        if (grabardatos = TRUE)
          GrabarDatos(&memdatos,fichero);
	  
        grabardatos = FALSE;
        //Si el comando es salir se habrán escrito todos los metadatos
        //faltan los datos y cerrar
        if (strcmp(orden,"salir")==0)
		{
			GrabarDatos(&memdatos,fichero);
			fclose(fichero);
			return 0;
        }
    }
	return 0;
}