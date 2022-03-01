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

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2)
{
	//Declaramos variables
	int noValido = FALSE;
	//Separamos en tokens
	separar(strcomando, orden, argumento1, argumento2);
	
	int contador = 0;
	
	//Comparamos la orden con todas las posibilidades
	if((strcmp(orden,"info")!=0) && (strcmp(orden,"bytemaps")!=0) && (strcmp(orden,"dir")!=0) && (strcmp(orden,"salir")!=0) && (strcmp(orden,"rename")!=0)
		&& (strcmp(orden,"copy")!=0) && (strcmp(orden,"imprimir")!=0) && (strcmp(orden,"remove")!=0))
	{
		printf("ERROR: Comando ilegal [bytemaps, copy, dir, info, imprimir, rename, remove, salir]\n");
		noValido = TRUE; //El booleano no de que no sea valida la orden se pone a true
	}
	
	return noValido;
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup) //Imprimimos todo lo necesario del superbloque
{
	printf("Bloque %d Bytes\ninodos particion = %d\ninodos libres = %d\nBloques particion = %d\nBloques libres = %d\nPrimer bloque de datos = %d\n",psup->s_block_size,psup->s_inodes_count,psup->s_free_inodes_count,psup->s_blocks_count,psup->s_free_blocks_count,psup->s_first_data_block);
}

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre)
{
	//Declaramos variables
	char* aux = (char*)malloc(sizeof(char)*LEN_NFICH);
	int acum = 20;
	int i = 0;
	int j = 0;
	//Inicializamos todas las variables
	for(i = 0; i < MAX_FICHEROS; i++)
	{
		aux[i] = '\0';
	}
	//Recorremos todos los ficheros 
	for(i = 0; i < MAX_FICHEROS; i++)
	{
		if(inodos->blq_inodos[i].size_fichero > 0) //Comprobamos que el tamaño del fichero correspondiente sea mayor que 0
		{
			for(j = 0; directorio->dir_nfich[j + acum] > MAX_FICHEROS; j++)
			{
				strncpy(&aux[j],&directorio->dir_nfich[j + acum], 1); //Vamos copiando caracter a caracter de lo leído del fichero.bin a una variable auxiliar
			}

			if(strncmp(aux, nombre, sizeof(nombre)) == 0) //Comparamos dicha variable auxiliar con el nombre que nos pasan por argumentos, si coincide liberamos memoria y devolvemos el indice donde ha coincidido
			{
				free(aux);
				return acum;
			}
			
			memset(aux, '\0', sizeof(aux)); //Seteamos cada vez la variable aux a '\0' en todos los indices
			acum += 20;
		}
	}
	free(aux); //Saldrmos aquí si no hemos encontrado un nombre igual a aux, liberamos memoria y devolvemos -1 para simbolizar que no han coincidido
	return -1;
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos)
{
	//Declaramos variables
	int i = 0;
	int j = 0;
	int acum = 20;
	
	for(i = 0; i < MAX_FICHEROS; i++) //Recorremos todos los ficheros
	{
		if(inodos->blq_inodos[i].size_fichero != 0)
		{
			for(j = 0; directorio->dir_nfich[j + acum] != '\0'; j++)
			{
				printf("%c", directorio->dir_nfich[j + acum]); //Vamos imprimiendo caracter a caracter
			}
			acum += 20;
			
			printf("\t");
			printf("tamaño:%d\tinodo:%d\tbloques:",inodos->blq_inodos[i].size_fichero, i); //Imprimimos los demás detalles
			
			// j debe ser menor que numero maximo de bloques por inodo, y el bloque debe ser distinto del bloque nulo
			for(j = 0; j < MAX_NUMS_BLOQUE_INODO && (inodos->blq_inodos[i].i_nbloque[j] != NULL_BLOQUE); j++)
			{
				printf("%d ", inodos->blq_inodos[i].i_nbloque[j]);
			}
			printf("\n");
		}
	}
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,char *nombreantiguo, char *nombrenuevo) //Para renombrar tenemos que encontrar la posicion donde se encuentre el nombre del fichero antiguo
{
	int nombreValido = TRUE;
	int operacionExitosa = FALSE;
	int i = 0;
	int posicionParaRenombrar1 = BuscaFich(directorio, inodos, nombreantiguo); //Vemos si existen para el control de errores
	int posicionParaRenombrar2 = BuscaFich(directorio, inodos, nombrenuevo);
	
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
		for(i = 0; i < LEN_NFICH; i++)
		{
			memset(&directorio->dir_nfich[i + posicionParaRenombrar1], nombrenuevo[i], 1);
		}
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

int buscarInodo(EXT_ENTRADA_DIR* directorio, EXT_BLQ_INODOS* inodos, char* nombre) //Funcion para encontrar qué inodo es el correspondiente al fichero con ese nombre
{
	//Declaramos variables
	int i = 0;
	int j = 0;
	char* aux = (char*)malloc(sizeof(char) * LEN_NFICH);
	int acumulador = 20;
	//Inicializamos variables
	for(i = 0; i < MAX_FICHEROS; i++)
	{
		aux[i] = '\0';
	}
	
	for (i = 0; i < MAX_FICHEROS; i++) //Recorremos todos los ficheros y vemos uno a uno si el tamaño es mayor que cero
	{
		if (inodos->blq_inodos[i].size_fichero > 0)
		{
			for (j = 0; directorio->dir_nfich[j + acumulador] != '\0'; j++) //Copiamos caracter a caracter en la variable aux
			{
				strncpy(&aux[j], &directorio->dir_nfich[j + acumulador], 1);
			}
			if (strcmp(aux, nombre) == 0) //En el caso de coincidan la variable aux con el nombre, devolvemos i (el inodo asignado a ese fichero)
			{
				free(aux);
				return i;
			}
			memset(aux, '\0', LEN_NFICH); //Limpiamos la variable aux
			acumulador += 20;
		}
	}
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) //Para imprimir debemos ver qué bloques imprimir
{
	//Antes de nada vemos si el fichero existe, y declaramos variables
	int ficheroExiste = BuscaFich(directorio, inodos, nombre);
	int inodoPorMirar;
	int i = 0;
	int j = 0;
	int operacionExitosa = FALSE;
	
	if(ficheroExiste == -1) //Control de errores
	{
		printf("ERROR: Fichero %s no encontrado\n", nombre);
	}
	else
	{		
		inodoPorMirar = buscarInodo(directorio, inodos, nombre); //Miramos qué inodo es el que corresponde al fichero

		for(j = 0; j < MAX_NUMS_BLOQUE_INODO && (inodos->blq_inodos[inodoPorMirar].i_nbloque[j] != NULL_BLOQUE); j++) //Recorremos todos los bloques del inodo "calculado"
		{
			for(i = 0; i < SIZE_BLOQUE; i++)
			{
				printf("%c", memdatos->dato[(inodos->blq_inodos[inodoPorMirar].i_nbloque[j] - (PRIM_BLOQUE_DATOS)) * SIZE_BLOQUE + i]); //Imprimimos caracter a caracter de dicho bloque (debemos restar PRIM_BLOQUE_DATOS, porque son los bloques reservados para el superbloque, bmaps, inodos y directorio)
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
	int indice_exists = BuscaFich(directorio, inodos, nombre); //Buscamos si el fichero existe
	int operacionExitosa = FALSE;
	int inodo = 0;
	int k = 0;
	char cero[2] = { "\0" }; //Necesistamos esta variable para poder ponerla en los bytemaps
	int acumulador = 20;
	if (indice_exists != -1) //En el caso de que exista:
	{
		inodo = buscarInodo(directorio, inodos, nombre); //Buscamos qué inodo le corresponde ese fichero
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
		memset(&directorio->dir_nfich[indice_exists], '\0', LEN_NFICH); //Seteamos en la direccion del nombre del fichero todo a '\0', y shifteamos todos los nombres a la derecha
		while (directorio->dir_nfich[indice_exists+acumulador] > MAX_FICHEROS) //Recorremos los indices hasta que el carcater observado este entre 0 y 20--> los cuales corresponden para indicar qué fichero va dónde
		{
			for (k = 0; k < LEN_NFICH; k++) //Copiamos caracter a caracter de las posiciones más adelantadas en una anterior
				memset(&directorio->dir_nfich[indice_exists + acumulador+k - 20], directorio->dir_nfich[indice_exists + acumulador + k], 1);
			acumulador += 20;
		}
		memset(&directorio->dir_nfich[indice_exists + acumulador-20], '\0', LEN_NFICH); //Finalmente se nos quedará el último nombre de fichero duplicado, asi que lo borramos
		inodos->blq_inodos[inodo].size_fichero = 0; //Seteamos el tamaño del fichero a 0
		ext_superblock->s_free_inodes_count++; //Actualizamos contadores
		
		for(int i = 0; i < MAX_FICHEROS; i++) //Seteamos la posicion que ocupase en el array el inodo calculado a NULL_INODO
		{
			if(directorio[i].dir_inodo == inodo)
			{
				directorio[i].dir_inodo = NULL_INODO;
			}
		}
		
		operacionExitosa = TRUE; //La operacion ha tenido exito
	}
	else
	{
		printf("ERROR: Fichero %s no encontrado\n", nombre);
	}
	
	return operacionExitosa;
}

int primerIndiceDeFicheroLibre(EXT_BLQ_INODOS *inodos) //Buscamos primer indice del fichero que esté libre (hasta que posicion hay nombres de ficheros)
{
	int i = 0;
	int acum = 0;
	
	for(i = 0; i < MAX_FICHEROS; i++)
	{
		if(inodos->blq_inodos[i].size_fichero != 0)
		{			
			acum += 20;
		}
	}
	return acum;
}

int buscarInodoLibre(EXT_BYTE_MAPS *ext_bytemaps) //Buscamos el primer inodo libre
{
	int i = 0;
	
	for(i = 0; i < MAX_INODOS; i++)
	{
		// Para un inodo libre compruebo
		if(ext_bytemaps->bmap_inodos[i] == 0)
		{
			return i;
		}
	}
}

void crearFichero(EXT_ENTRADA_DIR* directorio, EXT_BLQ_INODOS* inodos, char* nombre, int inodoLibre) //Para implementar el fichero deberemos poner el nombre de forma ordenada:
{
	int acumulador = primerIndiceDeFicheroLibre(inodos); //Buscamos el primer indice que no esté ocupada para setear ahí el nombre, en otras palabras, buscamos hasta qué posicion ocupan los nombres de los ficheros
	int i = 0;
	
	for(i = MAX_FICHEROS-1; i >= 0; i--) //Recorremos de atrás hacia delante para poder mover los ficheros existentes hacia la derecha en caso de que sea necesario
	{
		if(inodos->blq_inodos[i].size_fichero > 0)
		{
			if(i > inodoLibre) //En el caso de que el inodo que estemos evaluando sea mayor que el inodoLibre, deberemos mover su nombre a la izquierda
			{
				memcpy(&directorio->dir_nfich[acumulador], &directorio->dir_nfich[acumulador - 20], LEN_NFICH);
				
				acumulador -= 20; //Y seguimos llendo hacia atrás
			}
			else if(i == inodoLibre) //En el caso de que el inodo evaluado sea el mismo, ponemos el nombre
			{
				memcpy(&directorio->dir_nfich[acumulador], nombre, LEN_NFICH);
				directorio[i].dir_inodo = inodoLibre; //Y en el inodo evaluado seteamos que deje de ser NULL_INODO y pase a ser el inodo correspondiente
			}
		}
	}
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
	char unCero[2] = "\0";
	
	//Variables necesarias para bucles principalmente
	int inodoLibre = 0;
	int inodoPorMirar = 0;
	int j = 0;
	int contador = 0;
	int acum = 20;
	
	//Variables para comprobar la validez de los argumentos que nos han pasado
	int posicionParaCopiar1 = BuscaFich(directorio, inodos, nombreorigen);
	int posicionParaCopiar2 = BuscaFich(directorio, inodos, nombredestino);
	
	if(posicionParaCopiar1 == -1)
	{
		nombreValido = FALSE;
	}
	else if(posicionParaCopiar2 != -1)
	{
		nombreValido = EXISTENTE;
	}
	
	//--------------------------------------------------------------------------
	if(nombreValido == TRUE)
	{
		inodoPorMirar = buscarInodo(directorio, inodos, nombreorigen);
		inodoLibre = buscarInodoLibre(ext_bytemaps);
		
		// Contamos cuantos bloques tenemos que copiar
		for(j = 0; j < MAX_NUMS_BLOQUE_INODO && (inodos->blq_inodos[inodoPorMirar].i_nbloque[j] != NULL_BLOQUE); j++)
		{
			contador++;
			ext_superblock->s_free_blocks_count--; //Actualizamos contadores
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
				memcpy(&memdatos->dato[(inodos->blq_inodos[inodoLibre].i_nbloque[x] - (PRIM_BLOQUE_DATOS)) * SIZE_BLOQUE], &memdatos->dato[(inodos->blq_inodos[inodoPorMirar].i_nbloque[x] - (PRIM_BLOQUE_DATOS)) * SIZE_BLOQUE], SIZE_BLOQUE);
				
				//Actualizamos contadores que nos sirven para ir rellenando más con cierto limite, en este caso el limite está en el numero de bloques que hallamos contado en la variable "contador"
				x++;
				contador--;
			}
		}
		
		crearFichero(directorio, inodos, nombredestino, inodoLibre); //Creamos el fichero donde corresponda

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
			if(strcmp(argumento1, "\0") != 0 && strcmp(argumento2, "\0") != 0)
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
			if(strcmp(argumento1, "\0") != 0)
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
			if(strcmp(argumento1, "\0") != 0)
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
			if(strcmp(argumento1, "\0") != 0 && strcmp(argumento2, "\0") != 0)
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
		//comando = "\0";---------------------------------------------------------------------------------------------------------------------------
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