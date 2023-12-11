#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define PACKET_SIZE 2

int main(int argc, char** argv) {
    int rank, size;
    MPI_Request request;
    // double starttime, endtime;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        printf("This program should be run with at least 2 processes\n");
        MPI_Finalize();
        return 0;
    }

    // starttime = MPI_Wtime();
    if (rank == 0) {
        // Процесс 0 реализует функцию центрального узла
        int total_packets = size - 1; // Количество пакетов для отправки
        int i, source, dest, packet[PACKET_SIZE];
        MPI_Status status;

        printf("Process 0 is the central node\n");

        for (i = 0; i < total_packets; i++) {
            // Принимаем пакет от других процессов
            MPI_Recv(&packet, PACKET_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            source = status.MPI_SOURCE;
            dest = packet[0];

            printf("Process %d received packet from process %d with destination %d\n", rank, source, dest);

            if (dest >= 1 && dest < size) {
                // Пересылаем пакет адресату
                MPI_Send(&packet, PACKET_SIZE, MPI_INT, dest, 0, MPI_COMM_WORLD);
                printf("Process %d sent packet to process %d\n", rank, dest);
                // Получаем подтверждение об отправке
                MPI_Recv(&packet, PACKET_SIZE, MPI_INT, dest, 2, MPI_COMM_WORLD, &status);
                printf("Process 0 received confirmation from process %d\n", dest);
                // Посылаем ответ отправителю о доставке
                MPI_Send(&packet, PACKET_SIZE, MPI_INT, source, 1, MPI_COMM_WORLD);
               
            }
        }
    } else {
        // Процессы 1, 2, ..., size-1 генерируют пакеты и отправляют их процессу 0
        int packet[PACKET_SIZE];
        packet[0] = (rank+1)%size; // Адресат
        if(packet[0]==0){
            packet[0] = 1;
        }
        packet[1] = rand()/rank; // Информационная часть пакета

        printf("Process %d generated packet with destination: %d, information: %d\n", rank, packet[0], packet[1]);

        // Отправляем пакет процессу 0
        MPI_Send(&packet, PACKET_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("Process %d sent packet to process 0\n", rank);

        //получаем пакет с информацией от цетрального процесса;
        if(MPI_Recv(&packet, PACKET_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) == MPI_SUCCESS){
            printf("Process %d received information from process 0: %d\n", rank, packet[1]);
            //отправляем подтверждение о получении сообщения от цетрального процесса;
            MPI_Send(&packet, PACKET_SIZE, MPI_INT, 0, 2, MPI_COMM_WORLD);
        }

        // Получаем подтверждение от процесса 0 о доставке пакета процессу-адресату
        MPI_Recv(&packet, PACKET_SIZE, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process %d received delivery confirmation from process 0\n", rank);
    }
    // endtime = MPI_Wtime();

    MPI_Finalize();
    // printf("That took %.4f milliseconds, process number %d\n", (endtime-starttime)*1000, rank);
    return 0;
}