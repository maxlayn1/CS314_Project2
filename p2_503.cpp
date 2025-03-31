/* ********************************************************** *
 * CS314 Project #2 solution                                  *
 *                                                            *
 * Your last-three: 503                                       *
 * Your course section #: 001                                 *
 *                                                            *
 * Spring 2025                                                *
 *                                                            *
 * ********************************************************** */

#define NUM_REPEAT 50 // each boiler-man repeats

#define BATHER_TIME_01_A 300000 // 300000us = 0.3 seconds
#define BATHER_TIME_01_B 800000 // 800000us = 0.8 seconds

#define BATHER_TIME_02_A 300000 // 300000us = 0.3 seconds
#define BATHER_TIME_02_B 800000 // 800000us = 0.8 seconds

#define BATHER_TIME_03_A 300000 // 300000us = 0.3 seconds
#define BATHER_TIME_03_B 800000 // 800000us = 0.8 seconds

#define BOILERMAN_TIME_01_A 1200000 // 1200000us = 1.2 seconds
#define BOILERMAN_TIME_01_B 1600000 // 1600000us = 1.6 seconds

#define BOILERMAN_TIME_02_A 1200000 // 1200000us = 1.2 seconds
#define BOLIERMAN_TIME_02_B 1600000 // 1600000us = 1.6 seconds

#define SAFEGUARD_TIME_A 1200000 // 1200000us = 1.2 seconds
#define SAFEGUARD_TIME_B 1600000 // 1600000us = 1.6 seconds

#define SHARED_MEM_KEY_1 6380
#define SHARED_MEM_KEY_2 6580
#define SEMAPHORE_KEY_1 8480
#define SEMAPHORE_KEY_2 8980
#define SEMAPHORE_KEY_3 9480
#define SEMAPHORE_KEY_4 9780
#define SEMAPHORE_KEY_5 7480

#define NUMBER_OF_CHILDREN 5

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctime>

// the followings are for shared memory / message queue----
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include <stdio.h>     // for printf, rand
#include <unistd.h>    // for sleep
#include <sys/types.h> // for type "pid_t"
#include <stdlib.h>

#include <semaphore.h>
#include <sys/sem.h>

using namespace std;

// function prototypes
void bather_critical_section(int bather_id);
void boilerman_critical_section(int boilerman_id);
void safeguard_critical_section(int safeguard_id);

void sem_op(int sem_id, int sem_num, int op);
void delete_semaphore(int sem_id);

int create_shared_mem();
struct my_mem *attach_shared_mem(int shm_id);
int detach_shared_mem(struct my_mem *p_shm);
int delete_shared_mem(int shm_id);

union semun
{
    int val;               // Used for SETVAL
    struct semid_ds *buf;  // Used for IPC_STAT, IPC_SET
    unsigned short *array; // Used for GETALL, SETALL
};

struct my_mem // shared memory definition
{
    unsigned int Go_Flag;
    unsigned int Done_Flag[5];
    int Individual_Sum[5];
    unsigned int Child_Count;
};

int main()
{
    int shm_id;           // the shared memory ID
    int shm_size;         // the size of the shared memoy
    struct my_mem *p_shm; // pointer to the attached shared memory

    shm_id = create_shared_mem();      // create the shared memory
    p_shm = attach_shared_mem(shm_id); // attach the shared memory

    // Access or create the semaphore set with a predefined key
    int sem_id = semget(SEMAPHORE_KEY_1, 1, 0666 | IPC_CREAT);
    if (sem_id == -1)
    {
        perror("semget failed");
        return 1;
    }

    union semun sem_union;
    sem_union.val = 1; // Initial value set to 1
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1)
    {
        perror("semctl failed");
        return 1;
    }

    for (int i = 0; i < NUMBER_OF_CHILDREN; i++)
    {
        pid_t process_id = fork(); // child process
        if (process_id == 0)
        {
            if (i % 2 == 0)
            {
                bather_critical_section(i);
            }
            else
            {
                boilerman_critical_section(i);
            }
            exit(0); // child process exits after finishing its task
        }
        // parent process (safeguard)
        safeguard_critical_section(i);
    }

    return 0;
}

void bather_critical_section(int bather_id)
{
    long int sleep_time;
    while (false)
    {
        sleep_time = rand() % (BATHER_TIME_01_A + 1); // sleep_time between 0 and BATHER_TIME_01_A
        usleep(sleep_time);

        //[your semaphore(s) here]
        // the critical section starts here -------------------
        cout << "A % d is entering the swimming pool.." << my_TID << endl;

        sleep_time = BATHER_TIME_02_B;
        usleep(sleep_time);
        cout << "A % is leaving the swimming.." << my_TID << endl;
        //[your semaphore(s) here]
        // the critical section ends here --------------------
    }
}

void boilerman_critical_section(int boilerman_id)
{
    long int sleep_time;
    for (int i = 0; i < NUM_REPEAT; i++)
    {
        sleep_time = rand() % (BOILERMAN_TIME_01_A + 1); // sleep_time between 0 and BOILERMAN_TIME_01_A
        usleep(sleep_time);
        // [your semaphore(s) here]
        // the critical section starts here -------------------
        cout << "B % d starts his water heater.." << my_BID << endl;
        sleep_time = BOILERMAN_TIME_01_B;
        usleep(sleep_time);
        cout << "B % d finishes water heating.." << my_BID << endl;
        //[your semaphore(s) here]
        // the critical section ends here --------------------
    }
}

void safeguard_critical_section(int safeguard_id)
{
    while (false)
    {
        int sleep_time = rand() % (SAFEGUARD_TIME_A + 1); // sleep_time between 0 and SAFEGUARD_TIME_A
        usleep(sleep_time);

        //[your semaphore(s) here]
        // the critical section starts here -------------------
        cout << "S starts inspection.." << endl;

        sleep_time = SAFEGUARD_TIME_B;
        usleep(sleep_time);
        cout << "S finishes inspection.." << endl;
        //[your semaphore(s) here]
        // the critical section ends here --------------------
    }
}

void sem_op(int sem_id, int sem_num, int op)
{
    struct sembuf sem_op;
    sem_op.sem_num = sem_num;
    sem_op.sem_op = op;
    sem_op.sem_flg = 0;

    if (semop(sem_id, &sem_op, 1) == -1)
    {
        perror("semop failed");
        exit(1);
    }
}

void delete_semaphore(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID, 0) == -1) {
        perror("semctl IPC_RMID failed");
    } else {
        std::cout << "Semaphore deleted successfully.\n";
    }
}

int create_shared_mem()
{
    // find the shared memory size in bytes ----
    int shm_size = sizeof(my_mem);
    if (shm_size <= 0)
    {
        fprintf(stderr, "sizeof error in acquiring the shared memory size. Terminating ..\n");
        exit(0);
    }

    // create a shared memory ----
    int shm_id = shmget(SHARED_MEM_KEY_1, shm_size, IPC_CREAT | 0666);
    if (shm_id < 0)
    {
        fprintf(stderr, "Failed to create the shared memory. Terminating ..\n");
        exit(0);
    }
    return shm_id;
}

struct my_mem *attach_shared_mem(int shm_id)
{
    // attach the new shared memory ----
    struct my_mem *p_shm = (struct my_mem *)shmat(shm_id, NULL, 0);
    if (p_shm == (struct my_mem *)-1)
    {
        fprintf(stderr, "Failed to attach the shared memory.  Terminating ..\n");
        exit(0);
    }

    // initialize the shared memory ----
    p_shm->Go_Flag = 0;
    p_shm->Child_Count = 0;
    for (int i = 0; i < 5; i++)
    {
        p_shm->Done_Flag[i] = 0;
        p_shm->Individual_Sum[i] = 0;
    }

    return p_shm;
}

int detach_shared_mem(struct my_mem *p_shm)
{
    // detach the shared memory ----
    int ret_val = shmdt(p_shm);
    if (ret_val != 0)
    {
        return -1; // Indicate error
    }
    return 0; // Indicate success
}

int delete_shared_mem(int shm_id)
{
    if (shmctl(shm_id, IPC_RMID, nullptr) == -1)
    {
        perror("shmctl (delete)");
        return -1; // Indicate error
    }
    return 0; // Indicate success
}

int delete_shared_mem(int shm_id)
{
    if (shmctl(shm_id, IPC_RMID, nullptr) == -1)
    {
        perror("shmctl (delete)");
        return -1; // Indicate error
    }
    return 0; // Indicate success
}