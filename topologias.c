#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define TAM_MATRIX 120
#define MAX_VALUE 9


/*
Escribe un programa sumaMatriz.c que calcule la
suma de los elementos de una matriz A de tamaño
nxn rellena con enteros aleatorios entre 0 y 9.
- 	Creamos una topología de procesadores de tamaño pxp,
	siendo n divisible por p.
-	El proceso P0 inicializa y reparte la matriz A por
	bloques bidimensionales entre los procesos Éstos
	efectúan la suma de los elementos del correspondiente
	bloque y devuelven el resultado a P0.
-	Para enviar las submatrices usaremos tipos derivados
	(MPI_Type_vector)
-	Ejecuta el programa con 4, 9, 16 procesos para valores
	de n múltiplos de 12 (120, 1200,12000…).
*/

void pintarMatriz(double **A, int rows, int cols);
void distribuir(double **IN, int rows_in, int cols_in, double **OUT, int rows_out, int cols_out);
void Recolectar(double **IN, int rows_in, int cols_in, double **OUT, int rows_out, int cols_out);

int main(int argc,char *argv[]){

	int rank, numprocs, i,j,k,m, blockSize, suma, sumaGlobal;
	double time; 
	double **A, **Apeq;
	int procs_dim;
	int dims[2], periods[2];
	suma = 0;
	sumaGlobal = 0;

	MPI_Comm cart;	
	
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	
	//trabajaremos con topolog�as cuadradas.. y matrices cuadradas!
	procs_dim = sqrt(numprocs);
	blockSize = TAM_MATRIX/procs_dim;
	//Creamos la topolog�a de procesadores
	dims[0] = procs_dim;
	dims[1] = procs_dim;
	periods[0] = periods[1] = 1;
	
	//Creamos la topologia de dos dimensiones
	MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cart);
	
	srand(time(NULL));
	
	// El master inicializa la matrices A
	if(rank == 0){

		//Reserva de filas
		A = (double **)malloc(TAM_MATRIX*sizeof(double*));

		printf("Master rellenando matriz...\n");
		for (i = 0; i < TAM_MATRIX; i++){
			A[i] = (float *) malloc (TAM_MATRIX*sizeof(double));
			for (j = 0; j < TAM_MATRIX; j++){
				A[i][j] = rand()%(MAX_VALUE + 1);
			}
		}
	}
	// cada proceso reserva memoria para sus matrices de bloques
	Apeq = (double **)malloc((blockSize)*sizeof(double*));

	printf("Proceso %d rellenando matriz...\n", rank);
	for (i = 0; i < blockSize; i++){
		Apeq[i] = (float *) malloc (blockSize*sizeof(double));
		for (j = 0; j < blockSize; j++){
			Apeq[i][j] = 0;
		}
	}
	
	
	//Sincronizamos los procesos para obtener tiempos de ejecucion precisos
	MPI_Barrier(MPI_COMM_WORLD);
	time = MPI_Wtime(); 
	
	
	//-------------------------------------INICIO ALGORITMO-----------------------------------------
	//Se distribuye A entre los procesadores, los cuales reciben su bloque de datos en Apeq
	distribuir(A,TAM_MATRIX,TAM_MATRIX, Apeq, blockSize, blockSize);
	
	for(i = 0;i < blockSize; i++){
		for(j = 0;j < blockSize; j++){
			suma = suma + Apeq[i][j];
		}
	}
	
	printf("El proceso %d, ha calculado la suma parcial: %d. \n", rank, suma);
	MPI_Barrier(MPI_COMM_WORLD);
	//El master recoge todas las sumas parciales de las submatrices
	MPI_Allreduce(&suma, &sumaGlobal, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	
	//-------------------------FIN ALGORITMO--------------------------------
	time = MPI_Wtime() - time;
	
	if (rank == 0) {
		printf("La suma total es: %d, %.16f segundos\n", sumaGlobal, time);
	}
    
    MPI_Finalize();
    return 0;
}



void pintarMatriz(double **A, int rows, int cols){
	int i,j;
	for(i=0; i<rows; i++){
		for(j=0; j<cols; j++)
			printf("%.0f ",A[i][j]);
		printf("\n");
	}
}



void distribuir(double **IN, int rows_in, int cols_in, double **OUT, int rows_out, int cols_out){
	int i, j, numprocs, rank, posicion = 0, procs_dim;
	//Buffer donde irán almacenados los bloques de datos
	double * buffer;

	MPI_Datatype bloque;
	
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	
	procs_dim = sqrt(numprocs);

	if(rank==0){
		buffer = malloc(sizeof(double)*rows_in*cols_in);
		//Creamos el tipo de datos bloque, con el tamaño correspondiente
		MPI_Type_vector(rows_out, cols_out, cols_in, MPI_DOUBLE, &bloque);
		MPI_Type_commit(&bloque);
		
		//Empaquetamos los datos de la matriz grande en bloques pequeños
		for(i=0;i<procs_dim;i++)
			for(j=0;j<procs_dim;j++){
				MPI_Pack(&IN[i*rows_out][j*cols_out], 1, bloque, buffer, sizeof(double)*rows_in*cols_in, &posicion, MPI_COMM_WORLD);	
			}
		MPI_Type_free(&bloque);
	}	
	//Enviamos a cada proceso
	MPI_Scatter(buffer, sizeof(double)*rows_out*cols_out ,MPI_PACKED, OUT[0], rows_out*cols_out, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	if(rank==0)	
		free(buffer);
}

