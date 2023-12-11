#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv) {
    double starttime, endtime;

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Создаем новый коммуникатор только для процессов, ранг которых делится на 3
    starttime = MPI_Wtime();
    MPI_Comm new_comm;
    MPI_Comm_split(MPI_COMM_WORLD, (rank % 3 == 0) ? 0 : MPI_UNDEFINED, rank, &new_comm);

    if (rank % 3 == 0) {
        int n = 3; // Количество чисел в каждом процессе
        // Создаем буфер для хранения чисел
        int send_numbers[n];
        for (int i = 0; i < n; i++) {
            send_numbers[i] = rank + i; // Произвольные числа
        }

        // Создаем буфер для хранения пересланных чисел
        int received_numbers[size * n];

        // Осуществляем пересылку данных в главный процесс
        MPI_Gather(&send_numbers, n, MPI_INT, &received_numbers, n, MPI_INT, 0, new_comm);

        //освобождаем новый коммуникатор
        MPI_Comm_free(&new_comm);

        // Выводим пересланные числа в порядке возрастания рангов процессов
        if (rank == 0) {
            printf("Received numbers:\n");
        	for (int i = 0; i < size; i++){
                for (int j=0; j<size;j+=3){
                    if(received_numbers[i]==j){
                        printf("from process %d: %d, %d, %d\n", j, received_numbers[i], received_numbers[i+1], received_numbers[i+2]);
                    }
                }    
            }
        }
    }
    endtime = MPI_Wtime();

    MPI_Finalize();
    
    printf("That took %.4f milliseconds, process number %d\n", (endtime-starttime)*1000, rank);
    
    return 0;
}
