#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

	/*======================================================================================*
	*		                    		VARIABLES GLOBALES		                            *
	*=======================================================================================*/
//Numero que representa las filas y las columnas de la matriz del maestro
#define MATRIX_SIZE 10000
//ID del proceso maestro
#define MASTER_ID 0
//Maximo valor que pueden tomar los elementos de la matriz
#define MAX_VALUE 100


	/*======================================================================================*
	*		               			 DECLARACION DE FUNCIONES				                *
	*=======================================================================================*/

void normalizeVetor(float * V, int length, int maxValue);
int findMax(float * vec, int size);

int main(int argc, char *argv[])
{
	//printf("Iniciando programa...\n");
	MPI_Status info;
	int rank, size, nRowsPerProcess, sendCount, i, j;
	float ** matrix; 
	double init_time, end_time;
	float result, globalMax;
	time_t t;
	
	result = 0.0;
	globalMax = 0;

	srand((unsigned) time(&t));

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	
	//       Cada proceso va a recibir (MATRIX_SIZE/Nº of process) columnas.
	
	//Columnas que le tocan a cada proceso
	nRowsPerProcess = MATRIX_SIZE/size;
	//Numero de elementos que recibira cada proceso
	sendCount = nRowsPerProcess * MATRIX_SIZE;


	// El master rellena su matriz con numeros aleatorios entre 0 y 100
	if(rank == MASTER_ID){
		//Reserva de filas
		matrix = (float **)malloc(MATRIX_SIZE*sizeof(float*));

		printf("Master rellenando matriz...\n");
		for (i = 0; i < MATRIX_SIZE; i++){
			matrix[i] = (float *) malloc (MATRIX_SIZE*sizeof(float));
			for (j = 0; j < MATRIX_SIZE; j++){
				matrix[i][j] = rand()%(MAX_VALUE + 1);
			}
		}

	}else{
		printf("Procesador %d creando matriz...\n", rank);	
		//Reserva de filas
		matrix = (float **)malloc(MATRIX_SIZE*sizeof(float*));
			
		//Si el proceso no es el master inicializa su matriz con ceros	
		
		for (i = 0; i < MATRIX_SIZE; i++){
			matrix[i] = (float *) malloc (nRowsPerProcess*sizeof(float));
		}
		
	}

	//Espero a todos los procesos
	MPI_Barrier(MPI_COMM_WORLD); 
	
	init_time = MPI_Wtime();
	
	//Reparto la matriz del maestro entre todos los procesos 				
	MPI_Scatter(&matrix[0][0], sendCount, MPI_FLOAT, &matrix[0][0], sendCount, MPI_FLOAT, MASTER_ID, MPI_COMM_WORLD);
	
	//busco el maximo en el trozo que he recibido datos
	result = findMax(&matrix[0][0], sendCount);
	printf("El proceso %d  ha calculado el maximo: %f ", rank, result);

	//Espero a todos los procesos
	MPI_Barrier(MPI_COMM_WORLD); 
	
	//El maximo global
	MPI_Allreduce(&result, &globalMax, 1 , MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
	printf("El maximo global es: %f \n", globalMax);

	normalizeVetor(&matrix[0][0], sendCount, globalMax);
	
	//Vuelo a juntar todos los datos en la matriz del maestro
			//Matriz del maestro					//Matriz de los esclavos
	MPI_Gather(&matrix[0][0], sendCount, MPI_FLOAT, &matrix[0][0], sendCount, MPI_FLOAT, MASTER_ID, MPI_COMM_WORLD);
	
	//Espero a todos los procesos
	MPI_Barrier(MPI_COMM_WORLD); 
	end_time = MPI_Wtime();	
	
	if(rank == MASTER_ID){
		printf("Numero de procesos: [%d]. Tiempo empleado: [%f].\n", size, (end_time-init_time));
	}
	//free(matrix);
	MPI_Finalize();
	return 0;
}


void normalizeVetor(float * V, int length, int maxValue){
	int i;
	for (i = 0; i < length; i++){
		V[i] = V[i]/maxValue;
	}
}

int findMax(float * vec, int size){
	long max = 0;
	int i;
	for (i = 0; i < size; i++){
		if(vec[i] > max){
			max = vec[i];
		}
	}
	return max;
}