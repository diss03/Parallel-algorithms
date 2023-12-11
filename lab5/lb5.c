#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv) {

    double starttime, endtime;
    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int N = size / 2;
    int A = rank;

    // Создание декартовой топологии
    int dims[2] = {2, N};
    int periods[2] = {0, 1}; //для передачи по строке от последнего к главному
    int reorder = 0;
    starttime = MPI_Wtime();
    MPI_Comm cart_comm;
    // Аргументы: коммуникатор; размерность; массив размеров сетки; массив периодов; переупорядочивание; новый коммуникатор;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &cart_comm);

    // Получение координат процесса в декартовой топологии (размерность координат = 2)
    // int coords[2];
    // MPI_Cart_coords(cart_comm, rank, 2, coords);
    // printf("rank %d x %d y %d\n", rank, coords[0], coords[1]);

    int recv_data;
    int source;
    int destination;

    // Аргументы: коммуникатор; направление 0 - вверх, 1 - по у; размер смещения; источник; назначение;
    MPI_Cart_shift(cart_comm, 1, 1, &source, &destination);
    printf("send from %d to %d\n", source,  destination);
    MPI_Sendrecv(&A, 1, MPI_INT, destination, 0, &recv_data, 1, MPI_INT, source, 0, cart_comm, MPI_STATUS_IGNORE);
    endtime = MPI_Wtime();

    MPI_Comm_free(&cart_comm);

    printf("Процесс %d: было A = %d, получил A = %d\n", rank, A, recv_data);

    MPI_Finalize();

    printf("That took %.4f milliseconds, process number %d\n", (endtime-starttime)*1000, rank);
    return 0;
}
