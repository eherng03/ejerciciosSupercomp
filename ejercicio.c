#include <stdio.h>
#include <mpi.h>

#define N 10

int main(int argc, char* argv[]){

	int size, rank, i, from, to, ndat, tag, len;
	char name[100];
	char nameRec[100];
	MPI_Status info;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Get_processor_name(name, &len);
//Inicializo vector donde almaceno el nombre del procesador
	for (i=0; i<N; i++) {
		name[i] = 0;
		nameRec[i] = 0;
	}
//Enviar los datos al rank + 1 y recibirlos del rank -1 modulo 10
	prev = (rank - 1)%size;
	next= (rank + 1)%size;
//Dependiendo de que arquitecturA, ESTO PUEDE NO FUNCIONAR
//Tiene que haber un proceso que haga algo distinto para que no se produzca un bloqueo
	MPI_Send(name, len, MPI_CHAR, next, 0, MPI_COMM_WORLD)
	MPI_Recv(nameRec, len, MPI_CHAR, prev, 0, MPI_COMM comm, &info);
	printf("Proceso %s (%d) recibe el mensaje del proceso %s (%d) \n",name, rank, nameRec, prev);
	MPI_Finalize();
	return 0;
}

