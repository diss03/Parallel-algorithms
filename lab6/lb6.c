#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>


int *createMatrix (int size) {
    int *matrix;
    if (( matrix = malloc(size*size*sizeof(int))) == NULL) {
        printf("Malloc error");
        exit(1);
    }

    for (int i=0; i<size*size; i++) {
        matrix[i] = rand() % 10;
    }

    return matrix;
}

void sequentialMatrixMultiplication(int MatrixA[], int MatrixB[], int MatrixC[], int size) {
    int index;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            index = i*size+j;
            MatrixC[index] = 0;
            for (int k = 0; k < size; k++) {
                MatrixC[index] += MatrixA[i*size+k] * MatrixB[k*size+j];
            }
        }
    }
}

void printMatrix (int *matrix, int dim) {
    for (int i=0; i<dim; i++) {
        for (int j=0; j<dim; j++) {
            printf("%d ", matrix[i*dim+j]);
        }
        printf("\n");
    }
}

void transpose(int Matrix[], int size)
{
    int t;
    for(int i = 0; i < size; ++i){
        for(int j = i; j < size; ++j){
            t = Matrix[i*size+j];
            Matrix[i*size+j] = Matrix[j*size+i];
            Matrix[j*size+i] = t;
        }
    }
}


int main(int argc, char** argv) {
    int dim = atoi(argv[argc - 1]);
    int size, rank;
    double starttime = 0;
    double endtime = 0;

    //для генерации
    srand(time(NULL));

    //генерация матриц
    int *MatrixA  = createMatrix(dim);
    int *MatrixB  = createMatrix(dim);
    int *MatrixC  = createMatrix(dim);


    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    if (size == 1) {
        //последовательный алгоритм
        // printf("Matrix A:\n");
        // printMatrix(MatrixA, dim);
        
        // printf("Matrix B:\n");
        // printMatrix(MatrixB, dim);

        starttime = MPI_Wtime();
        sequentialMatrixMultiplication(MatrixA, MatrixB, MatrixC, dim);
        endtime = MPI_Wtime();

        // printf("Matrix C:\n");
        // printMatrix(MatrixC, dim);
    }

    else if (size > 1 && dim%size==0) {
        //параллельный ленточный алгоритм
        if(rank ==0){  
            // printf("Matrix A:\n");
            // printMatrix(MatrixA, dim);
            
            // printf("Matrix B:\n");
            // printMatrix(MatrixB, dim);

            transpose(MatrixB, dim); 
        }

        MPI_Status Status;
        // сколько раз придется задействовать каждый буфер
        int procUseCount = dim/size; 
        // размер всех ячеек из матрицы А и В для одного процесса
        int elemetsPerBlock = procUseCount*dim; 
        
        // буферы
        int bufA[elemetsPerBlock];
        int bufB[elemetsPerBlock];
        int bufC[elemetsPerBlock];
        for (int i = 0; i < elemetsPerBlock; ++i){
            bufA[i] = 0;
            bufB[i] = 0;
            bufC[i] = 0;
        }

        starttime = MPI_Wtime();
        //рассыдка каждому поцессу своей части матриц А и В
        MPI_Scatter(MatrixA, elemetsPerBlock, MPI_INT, &bufA, elemetsPerBlock, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatter(MatrixB, elemetsPerBlock, MPI_INT, &bufB, elemetsPerBlock, MPI_INT, 0, MPI_COMM_WORLD);

        //создание нового коммуникатора (одномерное кольцо)
        MPI_Comm ring_comm;
        int dims[1] = {size}; 
        int periods[1] = {1};
        int reorder = 1;
        MPI_Cart_create(MPI_COMM_WORLD, 1, dims, periods, reorder, &ring_comm);
        MPI_Comm_rank(ring_comm, &rank);

        int source, destination, index;        
        for (int p=0; p < size; p++) {
            //расчет результата
            for (int i=0; i < procUseCount; i++) {
                for (int j=0; j < procUseCount; j++) {
                    index = i*dim+j+((rank+size-p)*procUseCount)%dim;
                    for (int k=0; k < dim; k++) {
                        bufC[index] += bufA[i*dim+k]*bufB[j*dim+k];
                    }
                }
            }
            //получение ранка процесса для передачи блока-столбца
            MPI_Cart_shift(ring_comm, 0, 1, &source, &destination);
            //пересылка столбца
            MPI_Sendrecv_replace(&bufB, elemetsPerBlock, MPI_INT, destination, 0, source, 0, ring_comm, &Status);
        }

        //Сбор всех частей матрицы С в единую
        MPI_Gather(&bufC, elemetsPerBlock, MPI_INT, MatrixC, elemetsPerBlock, MPI_INT, 0, ring_comm);
        endtime = MPI_Wtime();

        if (rank == 0) {
            // printf("Matrix C:\n");
            // printMatrix(MatrixC, dim);
        }
        MPI_Comm_free(&ring_comm);
    }
    else{
        if (rank == 0)
            printf("Incorrect data: check the divisibility of the matrix size by the number of processes\n");
    }

    MPI_Finalize();
    printf("That took %.4f milliseconds, process number %d\n", (endtime-starttime)*1000, rank);
    return 0;
}
        