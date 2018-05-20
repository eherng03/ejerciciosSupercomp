#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define VECTOR_SLAVE_SIZE 1000
#define VECTOR_MASTER_SIZE 100000
#define MESSAGE_TAG_0 0

#define ID_MAESTRO 0
#define VECTOR_MAX_VALUE 10000


int main(int argc, char *argv[])
{
	int size, rank, i, from, to, ndat, part, tag, elementosRestantes, process_id, indiceCola;
	int * vectorEsclavo;
	int * vectorMaestro;
	//servirá para saber que proceso es el siguiente al que le tego que enviar la información
	int * colaProcesos;
	long acumm = 0;
	process_id = 1;
	elementosRestantes = VECTOR_MASTER_SIZE;
	indiceCola = 0;

	MPI_Status info_0, info_1, info_2;
	MPI_Request request_0, request_1, request_2;
	
	srand(time(NULL));

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	/*======================================================================================																				   *
	*		  				    Reserva de memoria para los vectores 					   *
	=======================================================================================*/
	vectorEsclavo = (int *) malloc(VECTOR_SLAVE_SIZE * sizeof(int));
	if(vectorEsclavo == NULL){
		printf("Error de reserva de memoria para el vector esclavo.\n");
		return -1;
	}


	colaProcesos = (int *) malloc(size * sizeof(int));
	if(colaProcesos == NULL){
		printf("Error de reserva de memoria para la cola de procesos.\n");
		return -1;
	}

	/*======================================================================================																				   *
	*		  			Rellenado de vectores con valores aleatorios 					   *
	=======================================================================================*/
	if (rank == ID_MAESTRO){
		vectorMaestro = (int *) malloc(VECTOR_MASTER_SIZE * sizeof(int));
		if(vectorMaestro == NULL){
			printf("Error de reserva de memoria para el vector del maestro.\n");
			return -1;
		}	
				//Rellena el vector con valores aleatorios
		for (i = 0; i < N; ++i)
			vectorMaestro[i] = rand()%VECTOR_MAX_VALUE;

	}else{
		//Los esclavos inicializan el vector a 0
		for (i = 0; i < N; ++i)
			vectorEsclavo[i] = 0;
	}

	//inicializo la cola de procesos con 0, se almacenaran los procesos finalizados
	for (i = 1; i <= size; ++i)
			colaProcesos[i] = 0 ;

	//Espero a que todos los procesos rellenen su vector
	MPI_Barrier(MPI_COMM_WORLD); 

	//Mientras queden elementos que mandar, los reparto entre los vectores
	while(elementosRestantes != 0){
		if(rank == ID_MAESTRO){
		int isend_status = 0;
		if(process_id > size){
			process_id
			indiceCola = (indiceCola + 1) % size;
		}
		//Envia 1000 elementos del vector del maestro
		isend_status = MPI_Isend(vectorMaestro, VECTOR_SLAVE_SIZE, MPI_INT, process_id, MESSAGE_TAG_0, MPI_COMM_WORLD, &request_0);
		MPI_Wait(&request_0, &info_0);
		printf("El master envia elementos al proceso %d\n", colaProcesos[indiceCola]);
		elementosRestantes = elementosRestantes - VECTOR_SLAVE_SIZE;
		//paso al siguiente proceso
		process_id = process_id + 1;
		}else{
			int max = 0;
			int irec_status = 0;
			int isend_status = 0;

			//Recibimos los datos enviados por el maestro
			irec_status = MPI_Irecv(vectorEsclavo, VECTOR_SLAVE_SIZE, MPI_INT, ID_MAESTRO, MESSAGE_TAG_0, MPI_COMM_WORLD, &request_1);
			MPI_Wait(&request_1, &info_1);
			
			//calcula el maximo del vector recibido
			max = getMax(vectorEsclavo, VECTOR_SLAVE_SIZE);

			//Envia el maximo calculado hacia el maestro.
			isend_status = MPI_Isend(max, 1, MPI_INT, ID_MAESTRO, MESSAGE_TAG_0, MPI_COMM_WORLD, &request_2);
			MPI_Wait(&request_2, &info_2);
			//En este punto, este proceso ha terminado por lo que lo introducimos en la cola
			colaProcesos[(indiceCola - 1) % size] = rank;

			printf("El proceso %d ha terminado.\n", rank);
		}

		if(rank == ID_MAESTRO){
			//TODO hacer algo para que reciba los maximos
			int irec_status = 0;
			irec_status = MPI_Irecv(vectorEsclavo, VECTOR_SLAVE_SIZE, MPI_INT, colaProcesos[(indiceCola - 1) % size], MESSAGE_TAG_0, MPI_COMM_WORLD, &request_1);
		}
	}
	
	
	MPI_Finalize();
	return 0;
}


int getMax(int * vector, int size){
	int max = 0;
	int i;
	for(i = 0; i < size; i++){
		if(vector[i] > max){
			max = vector[i];
		}
	}
}