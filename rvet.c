/**
 * Código base (incompleto) para implementação de relógios vetoriais.
 * Meta: implementar a interação entre três processos ilustrada na figura
 * da URL: 
 * 
 * https://people.cs.rutgers.edu/~pxk/417/notes/images/clocks-vector.png
 * 
 * Compilação: mpicc -o rvet rvet.c
 * Execução:   mpiexec -n 3 ./rvet
 */
 
#include <stdio.h>
#include <string.h>
#include <mpi.h>

typedef struct Clock {
    int p[3];
} Clock;

int max(int a, int b) {
    if (b > a) {
        return b;
    }
    return a;
}

MPI_Datatype clock_type;

void PrintEventState(int pid, Clock *clock) {
    printf("Process: %d, Clock: (%d, %d, %d)\n", pid, clock->p[0], clock->p[1], clock->p[2]);
}

void Event(int pid, Clock *clock) {
    clock->p[pid]++;
    PrintEventState(pid, clock);
}

void Send(int s_pid, int r_pid, Clock *clock) {
    Event(s_pid, clock);
    MPI_Send(clock, 1, clock_type, r_pid, 0, MPI_COMM_WORLD);
    PrintEventState(s_pid, clock);
}

void Receive(int s_pid, int r_pid, Clock *clock) {
    Event(r_pid, clock);
    Clock rcv_clock;
    MPI_Recv(&rcv_clock, 1, clock_type, s_pid, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    for (int i = 0; i < 3; i++) {
        clock->p[i] = max(clock->p[i], rcv_clock.p[i]);
    }
    PrintEventState(r_pid, clock);
}

void process0() {
    Clock clock = {{0, 0, 0}};
    Event(0, &clock);
    Send(0, 1, &clock);
    Receive(1, 0, &clock);
    Send(0, 2, &clock);
    Receive(2, 0, &clock);
    Send(0, 1, &clock);
    Event(0, &clock);
}

void process1() {
    Clock clock = {{0, 0, 0}};
    Send(1, 0, &clock);
    Receive(0, 1, &clock);
    Receive(0, 1, &clock);
}

void process2() {
    Clock clock = {{0, 0, 0}};
    Event(2, &clock);
    Send(2, 0, &clock);
    Receive(0, 2, &clock);
}

int main(void) {
    int my_rank;
    MPI_Init(NULL, NULL);

    int lengths[1] = {3};
    MPI_Aint displacements[1];
    Clock dummy_clock;
    MPI_Aint base_address;
    MPI_Get_address(&dummy_clock, &base_address);
    MPI_Get_address(&dummy_clock.p[0], &displacements[0]);
    displacements[0] -= base_address;

    MPI_Datatype types[1] = {MPI_INT};
    MPI_Type_create_struct(1, lengths, displacements, types, &clock_type);
    MPI_Type_commit(&clock_type);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == 0) {
        process0();
    } else if (my_rank == 1) {
        process1();
    } else if (my_rank == 2) {
        process2();
    }

    MPI_Finalize();
    return 0;
}