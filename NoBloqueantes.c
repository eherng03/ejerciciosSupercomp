#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define VEC_SLAVE_SIZE 1000
#define VEC_MASTER_SIZE 100000
#define MESSAGE_TAG_0 0
#define MESSAGE_TAG_1 1

#define ID_MAESTRO 0
#define MAX_VALUE 10000

int getMax(int * vector, int size);

int main(int argc, char *argv[])
{

	/*=====================================================================================*
	*		  							Datos necesarios 								   *
	=======================================================================================*/
	int size, rank, i, from, to, ndat, part, tag, restElems, max, source, globMax, seguir, datasize, maxIndex;
	int * vector, * maximos;

	restElems = VEC_MASTER_SIZE;
	max = 0;
	globMax = 0;
	source = 0;
	seguir = 1;
	maxIndex = 0;
	MPI_Status st_0, st_1, st_2, st_3, status, stStave;
	MPI_Request req_0, req_1, req_2, req_3;
	
	srand(time(NULL));

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);


	/*=====================================================================================*
	*		  			Rellenado de vectores con valores aleatorios 					   *
	=======================================================================================*/
	if (rank == ID_MAESTRO){
		vector = (int *) malloc(VEC_MASTER_SIZE * sizeof(int));
		if(vector == NULL){
			printf("Error de reserva de memoria para el vector del maestro.\n");
			return -1;
		}
		
		//Rellena el vector con valores aleatorios
		for (i = 0; i < VEC_MASTER_SIZE; ++i)
			vector[i] = rand()%(MAX_VALUE + 1);
		printf("Vector del maestro rellenado.\n");
		
		maximos = (int *) malloc((VEC_MASTER_SIZE/size) * sizeof(int));
		if(maximos == NULL){
			printf("Error de reserva de memoria para el vector del maestro.\n");
			return -1;
		}	
		//Rellena el vector con valores aleatorios
		for (i = 0; i < (VEC_MASTER_SIZE/size); ++i)
			maximos[i] = rand()%(MAX_VALUE + 1);	
		

	}else{
		vector = (int *) malloc(VEC_SLAVE_SIZE * sizeof(int));
		if(vector == NULL){
			printf("Error de reserva de memoria para el vector esclavo.\n");
			return -1;
		}
		//Los esclavos inicializan el vector a 0
		for (i = 0; i < VEC_SLAVE_SIZE; ++i)
			vector[i] = 0;
		printf("Vector del proceso %d rellenado.\n", rank);
	}

	//Espero a que todos los procesos rellenen su vector
	MPI_Barrier(MPI_COMM_WORLD); 

	/*=====================================================================================*
	*		  							Envio y proceso de datos 						   *
	=======================================================================================*/
	
	if(rank == ID_MAESTRO){
		printf("----------------------------------------------------------------------------\n");
		//el master manda los primeros elementos a los procesos
		
		for(i = 1; i < size; i++){
			//Envia 1000 elementos de su vector
			MPI_Isend(&vector[(i-1)*VEC_SLAVE_SIZE], VEC_SLAVE_SIZE, MPI_INT, i, 0, MPI_COMM_WORLD, &req_0);
			MPI_Wait(&req_0, &st_0);
			max = getMax(&vector[(i-1)*VEC_SLAVE_SIZE], VEC_SLAVE_SIZE);
			printf("El master ha calculado el maximo del vector enviado al proceso %d: %d. \n", i, max);
			restElems = restElems - VEC_SLAVE_SIZE;
		}
	}else{
		//Los esclavos esperan a recibir vectores del maestro
		
		while(seguir){
			//Comprobamos si hay algún mensaje pendiente
			MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &stStave);
			
			MPI_Get_count(&stStave, MPI_INT, &datasize);
			
			//si no recibo datos, y el tag es 1 significa que el maestro ha acabado de repartir sus datos
			if(datasize == 0 && stStave.MPI_TAG == 1){
				printf("================================= PROCESO %d TERMINA ====================== \n", rank);
				seguir = 0;
				break;
			}
			
			//Recibimos los datos enviados por el maestro
			MPI_Irecv(vector, VEC_SLAVE_SIZE, MPI_INT, ID_MAESTRO, stStave.MPI_TAG, MPI_COMM_WORLD, &req_1);
			MPI_Wait(&req_1, &st_1);
			printf("El proceso %d ha recibido datos.\n", rank);	
					
			//calcula el maximo del vector recibido
			max = getMax(vector, VEC_SLAVE_SIZE);
			printf("El proceso %d calcula el maximo %d.\n", rank, max);
			
			MPI_Isend(&max, 1, MPI_INT, ID_MAESTRO, 0, MPI_COMM_WORLD, &req_2);
			MPI_Wait(&req_2, &st_2);
			printf("El proceso %d envia el maximo al master.\n", rank);
			
			//El tag 1 indica que no hay mas datos por mandar
			if(stStave.MPI_TAG == 1){
				printf("================================= PROCESO %d TERMINA ====================== \n", rank);
				seguir = 0;
				break;
			}
		}
	}	
	
		

	if(rank == ID_MAESTRO){
		while(restElems > 0){
			//Comprobamos si hay algún mensaje pendiente
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			
			//Obtenemos el proceso fuente del mensaje
			source = status.MPI_SOURCE;
			
			MPI_Irecv(&max, 1, MPI_INT, source, 0, MPI_COMM_WORLD, &req_3);
			MPI_Wait(&req_3, &st_3);
			
			printf("El maestro recibe del proceso %d el maximo %d. \n", source, max);
			maximos[maxIndex] = max;
			maxIndex++;
			//Si solo queda un bloque de elementos por mandar, envio el tag 1
			if(restElems - VEC_SLAVE_SIZE <= 0){
				//Envia 1000 elementos de su vector
				MPI_Isend(&vector[VEC_MASTER_SIZE - restElems], restElems, MPI_INT, source, 1, MPI_COMM_WORLD, &req_0);
				MPI_Wait(&req_0, &st_0);
				
				printf("El maestro envia LOS ULTIMOS datos al proceso%d. \n", source);
				max = getMax(&vector[VEC_MASTER_SIZE - restElems], VEC_SLAVE_SIZE);
				printf("El master ha calculado el maximo del vector enviado al proceso %d: %d. \n", i, max);
				//El maestro envia un mensaje vacio y con tag 1 al resto de procesos para que sepan que no quedan datos por mandar
				for(i = 1; i < size; i++){
					if(i != source){
						MPI_Isend(0, 0, MPI_INT, i, 1, MPI_COMM_WORLD, &req_0);
						MPI_Wait(&req_0, &st_0);
					}	
				}
				
			}else{
				//Envia 1000 elementos de su vector
				MPI_Isend(&vector[VEC_MASTER_SIZE - restElems], VEC_SLAVE_SIZE, MPI_INT, source, 0, MPI_COMM_WORLD, &req_0);
				MPI_Wait(&req_0, &st_0);
				printf("El maestro envia nuevos datos al proceso%d. \n", source);
			}
			
			restElems = restElems - VEC_SLAVE_SIZE;
		}
		printf("========================= El maestro acaba de mandar sus  datos =========================\n");
		
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	
	if(rank == ID_MAESTRO){
		globMax = getMax(maximos, VEC_MASTER_SIZE/size);
		printf("========================= El maximo global es %d =========================\n", globMax);
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
	return max;
}