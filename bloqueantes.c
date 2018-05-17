#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

	/*======================================================================================
	*																					   *
	*		                            VARIABLES GLOBALES		                    	   *
	*																					   *
	======================================================================================*/

#define MATRIX_SIZE 10000
#define MASTER_ID 0
#define MAX_VALUE 100

	/*======================================================================================
	*																					   *
	*		                          DECLARACION DE FUNCIONES		                    	   *
	*																					   *
	======================================================================================*/

void normalizeVetor(int * V, int length, int maxValue);
int findMax(int * vec, int size);
void printMatrix(int M[MATRIX_SIZE][MATRIX_SIZE]);
void fillMatrix(int M[MATRIX_SIZE][MATRIX_SIZE]);


int main(int argc, char *argv[])
{
	MPI_Status info;
	int rank, size, nRowsPerProcess, sendCount;
	float matrix[MATRIX_SIZE][MATRIX_SIZE];
	float init_time, end_time, result, globalMax;

	result = 0;
	globalMax = 0;

	srand(time(NULL));

	/*======================================================================================																				   *
	*		      Cada proceso va a recivir (MATRIX_SIZE/Nº of process) columnas.			   *
	=======================================================================================*/

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	//Columnas que le tocan a cada proceso
	nRowsPerProcess = MATRIX_SIZE/size;

	//Numero de elementos que recibira cada proceso
	sendCount = nRowsPerProcess * MATRIX_SIZE;


	// El master rellena su matriz con numeros aleatorios entre 0 y 100
	if(rank == MASTER_PROCESS_ID){
		fillMatrix(&matrix);
	}else{
		//Si el proceso no es el master inicializa su matriz con ceros
		for (i = 0; i < MATRIX_SIZE; i++){
			for (j = 0; j < MATRIX_SIZE; j++){
				matrix[i][j] = 0;
			}
		}
	}

	print("La matriz a repartir es: \n");
	for(i = 0; i < MATRIX_SIZE; i++){
		for(j = 0; j < MATRIX_SIZE; j++){
			printf(" %f ", matrix[i][j]);
		}
		print("\n");
	}

	//Espero a todos los procesos
	MPI_Barrier(MPI_COMM_WORLD); 

	init_time = MPI_Wtime();
	
	//Reparto la matriz del maestro entre todos los procesos 				
	MPI_Scatter(&matrix[nRowsPerProcess * rank][0], sendCount, MPI_FLOAT, &matrix[nRowsPerProcess * rank][0], MATRIX_SIZE*MATRIX_SIZE, MPI_FLOAT, MASTER_ID, MPI_COMM_WORLD);
	
	//busco el maximo en el trozo que he recibido datos
	result = findMax(&matrix[nRowsPerProcess * rank][0], sendCount);
	printf("El proceso %d mola, y ha calculado el maximo: %f ", rank, result);

	//El maximo global
	MPI_Allreduce(&result, &globalMax, 1 , MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
	printf("El maximo global es: %f \n", globalMax);

	normalizeVetor(&matrix[nRowsPerProcess*rank][0], sendCount, globalMax);
	
	//Vuelo a juntar todos los datos en la matriz del maestro
	MPI_Gather(&matrix[nRowsPerProcess * rank][0], sendCount, MPI_FLOAT, &matrix[nRowsPerProcess*rank][0], sendCount, MPI_FLOAT, MASTER_ID, MPI_COMM_WORLD);
	

	end_time = MPI_Wtime();	
	print("La matriz final es: \n");
	for(i = 0; i < MATRIX_SIZE; i++){
		for(j = 0; j < MATRIX_SIZE; j++){
			printf(" %f ", matrix[i][j]);
		}
		print("\n");
	}

	if(rank == MASTER_PROCESS_ID)
		printf("Numero de procesos: [%d]. Tiempo empleado: [%f].\n", size, (end_time-init_time));
	
	MPI_Finalize();
	return 0;
}


void fillMatrix(int * M[MATRIX_SIZE][MATRIX_SIZE]){
	int i,j;

	for (i = 0; i < MATRIX_SIZE; i++){
		for (j = 0; j < MATRIX_SIZE; j++){
			M[i][j] = rand()%MAX_VALUE;
		}
	}
}

void normalizeVetor(float * V, int length, int maxValue){
	int i;
	for (i = 0; i < length; ++i){
		V[i] = V[i]/maxValue;
	}
}

int findMax(int * vec, int size){
	long max = 0;
	int i;
	for (i = 0; i < size; ++i){
		if(vec[i] > max){
			max = vec[i];
		}
	}
	return max;
}

