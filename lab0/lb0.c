#include <time.h>
#include <stdio.h>
#include "mpi.h"
int main(int argc, char* argv[])
{
	int ProcNum;
	int ProcRank;
	int RecvRank;
	double alltime = 0;
	double starttime, endtime, oneProcTime;

	MPI_Status Status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);

	starttime = MPI_Wtime();
	if (ProcRank != 0) {
		MPI_Send(&ProcRank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
	else if(ProcRank == 0){
		printf("Hello from process %3d\n", ProcRank);
		for(int i=1; i<ProcNum; i++){
			MPI_Recv(&RecvRank, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
			printf("Hello from process %3d\n", RecvRank);
		}
	}

	endtime = MPI_Wtime();
	MPI_Finalize();

	oneProcTime = (endtime-starttime)*1000;
	printf("Proc %d: %.4f\n", ProcRank, oneProcTime);
	//alltime += oneProcTime;

	//printf("V(p) = %.4f\n", alltime);
	//printf("V(1) = %.4f\n", alltime/ProcNum);
	return 0;
}
//mpiexec -n 100 .\test_mpi.exe
