#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

void generate_numbers(int* info, int ProcRank, int procNum){
	for (int i = 0; i< 4; i++){
		int new_num = rand() %100 + 100 + ProcRank;
		info[i] = new_num;
	}
	info[4] = ProcRank;
}

int main(int argc, char* argv[]) {
	double starttime, endtime;
	int procNum, ProcRank;
	int send_buf[5];
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

	generate_numbers(send_buf,ProcRank,procNum);
	int recv_buf[5 * procNum];
	
	starttime = MPI_Wtime();
	MPI_Allgather(send_buf, 5, MPI_INT, recv_buf, 5, MPI_INT, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	endtime = MPI_Wtime();
	
	printf("Process %d recive:\n", ProcRank);
	for (int i = 0; i < procNum; i++){
		for(int j = 0; j < sizeof(recv_buf)/sizeof(int); j++){
			if(recv_buf[j]==i){
				printf("from process %d: %d, %d, %d, %d\n", i, recv_buf[j-4], recv_buf[j-3], recv_buf[j-2], recv_buf[j-1]);
			}
		}
	}

	printf("That took %.4f milliseconds, process number %d\n", (endtime-starttime)*1000, ProcRank);
	MPI_Finalize();
	return 0;
}