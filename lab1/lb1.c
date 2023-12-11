#include <stdio.h> 
#include <stdlib.h> 
#include <mpi.h> 
#include <time.h> 
 
#define FIELD_SIZE 10
#define EMPTY_CELL 1
#define SHIP_CELL 2
#define MISS_CELL 3
#define HIT_CELL 4
#define GAME_END 5
#define HIT_TARGET 6
#define MISS_TARGET 7
 
void place_ships(int board[FIELD_SIZE][FIELD_SIZE]) {
    for (int i = 0; i < FIELD_SIZE; ++i) {
        int x = rand() % FIELD_SIZE;
        int y = rand() % FIELD_SIZE;
        board[y][x] = SHIP_CELL;
    }
}
 
int check_end(int board[FIELD_SIZE][FIELD_SIZE], int rank) {
    for (int i = 0; i < FIELD_SIZE; ++i) {
        for (int j = 0; j < FIELD_SIZE; ++j) {
            if (board[i][j] == SHIP_CELL) {
                return 0; 
            }
        }
    }
    // printf("Игрок %d выбыл!", rank);
    return 1;
}
 
void play_game_2(int rank, int size) { 
    int board[FIELD_SIZE][FIELD_SIZE];
    place_ships(board);
    int end = 0;	
 
    int target_send_x, target_send_y;
    int target_recv_x, target_recv_y;
    // int next_proc_end; 
    int hit_target = 0;
    int result;

    while (1) {
	    target_send_x = rand() %FIELD_SIZE; 
	    target_send_y = rand() %FIELD_SIZE; 
 
	    MPI_Send(&target_send_x, 1, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD); 
	    MPI_Send(&target_send_y, 1, MPI_INT, (rank + 1) % size, 1, MPI_COMM_WORLD);
        // printf("target:%d %d (from %d to %d)\n",target_send_x, target_send_y, rank, (rank - 1 + size) % size);
 
	    MPI_Recv(&target_recv_x, 1, MPI_INT, (rank - 1 + size) % size, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
	    MPI_Recv(&target_recv_y, 1, MPI_INT, (rank - 1 + size) % size, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
 
	    if (board[target_recv_x][target_recv_y] == SHIP_CELL) {
	    	hit_target = HIT_TARGET;
	    	board[target_recv_x][target_recv_y] = HIT_CELL;
	    }
        else if(board[target_recv_x][target_recv_y] != SHIP_CELL){
	    	board[target_recv_x][target_recv_y] = MISS_CELL;
            hit_target = MISS_TARGET;
	    }


	    if (end != GAME_END)
	    	MPI_Send(&hit_target, 1, MPI_INT, (rank - 1 + size) % size, 2, MPI_COMM_WORLD); 
 
	    else {
	        MPI_Send(&end, 1, MPI_INT, (rank - 1 + size) % size, 2, MPI_COMM_WORLD);
            break;
        }

        
        MPI_Recv(&result, 1, MPI_INT, (rank + 1) % size, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 

        if(result==HIT_TARGET){
            // printf("Hit in proc %d from proc %d!\n", rank, (rank + 1) % size);
        }
        else if (result==MISS_TARGET){   
            // printf("Miss in proc %d from proc %d!\n", rank, (rank + 1) % size);
        }

	    if (check_end(board, rank) || result == GAME_END) {
	    	end = GAME_END;
            break;
        }
    }
}
 
int main(int argc,  char *argv[]) { 
    int rank, recv, size; 
    MPI_Status status; 
    double starttime, endtime;
 
    MPI_Init(&argc, &argv); 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &size); 

    if(size < 2){
        printf("to few processes");
    }
    else if(size>=2){
        starttime = MPI_Wtime();
        play_game_2(rank, size);
        endtime = MPI_Wtime();
    }
    MPI_Finalize(); 
    
    printf("That took %.4f milliseconds, process number %d\n", (endtime-starttime)*1000, rank);
    return 0; 
}
