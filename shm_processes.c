#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define SHM_SIZE 2 * sizeof(int) // Shared memory size for two integers

void ParentProcess(int *ShmPTR);
void ChildProcess(int *ShmPTR);

int main(int argc, char *argv[]) {
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    srand(time(NULL)); // Seed the random number generator

    ShmID = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error (server) ***\n");
        exit(1);
    }
    printf("Server has received a shared memory of two integers...\n");

    ShmPTR = (int *)shmat(ShmID, NULL, 0);
    if (ShmPTR == (int *)-1) {
        printf("*** shmat error (server) ***\n");
        exit(1);
    }
    printf("Server has attached the shared memory...\n");

    ShmPTR[0] = 0; // Initialize BankAccount
    ShmPTR[1] = 0; // Initialize Turn
    printf("Server has initialized BankAccount and Turn in shared memory...\n");

    printf("Server is about to fork a child process...\n");
    pid = fork();
    if (pid < 0) {
        printf("*** fork error (server) ***\n");
        exit(1);
    } else if (pid == 0) {
        ChildProcess(ShmPTR);
        exit(0);
    } else {
        ParentProcess(ShmPTR);
    }

    wait(&status);
    printf("Server has detected the completion of its child...\n");
    shmdt((void *)ShmPTR);
    printf("Server has detached its shared memory...\n");
    shmctl(ShmID, IPC_RMID, NULL);
    printf("Server has removed its shared memory...\n");
    printf("Server exits...\n");
    exit(0);
}

void ParentProcess(int *ShmPTR) {
    for (int i = 0; i < 25; i++) {
        sleep(rand() % 6); // Sleep for 0-5 seconds

        int account = ShmPTR[0]; // Copy BankAccount to local variable

        while (ShmPTR[1] != 0); // Busy wait for Turn

        if (account <= 100) {
            int balance = rand() % 101; // Random amount between 0-100
            if (balance % 2 == 0) {
                account += balance;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }

        ShmPTR[0] = account; // Update BankAccount
        ShmPTR[1] = 1; // Set Turn to 1
    }
}

void ChildProcess(int *ShmPTR) {
    for (int i = 0; i < 25; i++) {
        sleep(rand() % 6); // Sleep for 0-5 seconds

        int account = ShmPTR[0]; // Copy BankAccount to local variable

        while (ShmPTR[1] != 1); // Busy wait for Turn

        int balance = rand() % 51; // Random amount between 0-50
        printf("Poor Student needs $%d\n", balance);

        if (balance <= account) {
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        ShmPTR[0] = account; // Update BankAccount
        ShmPTR[1] = 0; // Set Turn to 0
    }
}
