/* ********************************************************** *
 * CS314 Project #2 solution                                  *
 *                                                            *
 * Your last-three: 503                                       *
 * Your course section #: 001                                 *
 *                                                            *
 * Spring 2025                                                *
 *                                                            *
 * ********************************************************** */

#define NUM_REPEAT 10 // each boiler-man repeats

#define BATHER_TIME_01_A 300000 // 300000us = 0.3 seconds
#define BATHER_TIME_01_B 800000 // 800000us = 0.8 seconds

#define BATHER_TIME_02_A 300000 // 300000us = 0.3 seconds
#define BATHER_TIME_02_B 800000 // 800000us = 0.8 seconds

#define BATHER_TIME_03_A 300000 // 300000us = 0.3 seconds
#define BATHER_TIME_03_B 800000 // 800000us = 0.8 seconds

#define BOILERMAN_TIME_01_A 1200000 // 1200000us = 1.2 seconds
#define BOILERMAN_TIME_01_B 1600000 // 1600000us = 1.6 seconds

#define BOILERMAN_TIME_02_A 1200000 // 1200000us = 1.2 seconds
#define BOILERMAN_TIME_02_B 1600000 // 1600000us = 1.6 seconds

#define SAFEGUARD_TIME_A 1200000 // 1200000us = 1.2 seconds
#define SAFEGUARD_TIME_B 1600000 // 1600000us = 1.6 seconds

#define SHARED_MEM_KEY_1 6380
#define SHARED_MEM_KEY_2 6580
#define SEMAPHORE_KEY_1 8480
#define SEMAPHORE_KEY_2 8980
#define SEMAPHORE_KEY_3 9480
#define SEMAPHORE_KEY_4 9780
#define SEMAPHORE_KEY_5 7480

#define NUMBER_OF_BATHERS 3
#define NUMBER_OF_BOILERMEN 2
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
void bather_critical_section(int bather_id, int sync_sem_id, int SAFETY, int MUTEX, struct my_mem *p_shm);
void boilerman_critical_section(int boilerman_id, int sync_sem_id, int SAFETY, int MUTEX, struct my_mem *p_shm);
void safeguard_critical_section(int safeguard_id, int sync_sem_id, int SAFETY, int MUTEX, struct my_mem *p_shm);

void sem_op(int sem_id, short op);
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
    int ba_count;     // number of bathers in the swimming pool
    int done_counter; // number of finished child processes
    int boiler_done_counter; // number of finished boilermen
};

int main()
{
    int shm_id;           // the shared memory ID
    int shm_size;         // the size of the shared memoy
    struct my_mem *p_shm; // pointer to the attached shared memory

    shm_id = create_shared_mem();      // create the shared memory
    p_shm = attach_shared_mem(shm_id); // attach the shared memory

    // Create a semaphore for synchronization
    int sync_sem_id = semget(SEMAPHORE_KEY_1, 1, 0666 | IPC_CREAT);
    if (sync_sem_id == -1)
    {
        perror("semget failed for sync semaphore");
        return 1;
    }

    union semun sem_union;
    sem_union.val = 0; // Initialize semaphore to 0
    if (semctl(sync_sem_id, 0, SETVAL, sem_union) == -1)
    {
        perror("semctl failed for sync semaphore");
        return 1;
    }

    // Create a semaphore for safety
    int SAFETY = semget(SEMAPHORE_KEY_4, 1, 0666 | IPC_CREAT);
    if (SAFETY == -1)
    {
        perror("semget failed for safety semaphore");
        return 1;
    }

    sem_union.val = 1; // Initialize semaphore to 1 (allow first process to enter)
    if (semctl(SAFETY, 0, SETVAL, sem_union) == -1)
    {
        perror("semctl failed for safety semaphore");
        return 1;
    }

    // Create a semaphore for mutual exclusion
    int MUTEX = semget(SEMAPHORE_KEY_3, 1, 0666 | IPC_CREAT);
    if (MUTEX == -1)
    {
        perror("semget failed for mutex semaphore");
        return 1;
    }

    sem_union.val = 1; // Initialize semaphore to 1 (allow first process to enter)
    if (semctl(MUTEX, 0, SETVAL, sem_union) == -1)
    {
        perror("semctl failed for mutex semaphore");
        return 1;
    }

    for (int i = 0; i < NUMBER_OF_CHILDREN; i++)
    {
        pid_t process_id;
        process_id = fork(); // create a child process

        if (process_id < 0)
        {
            perror("fork failed");
            exit(1);
        }
        if (process_id == 0) // child process
        {
            if (i < NUMBER_OF_BATHERS)
            {
                bather_critical_section(i + 1, sync_sem_id, SAFETY, MUTEX, p_shm);
                p_shm->done_counter = p_shm->done_counter + 1;
                exit(0);
            }
            else
            {
                boilerman_critical_section(i - 2, sync_sem_id, SAFETY, MUTEX, p_shm);
                p_shm->done_counter = p_shm->done_counter + 1;
                p_shm->boiler_done_counter = p_shm->boiler_done_counter + 1;
                exit(0);
            }
        }
    }

    safeguard_critical_section(1, sync_sem_id, SAFETY, MUTEX, p_shm); // parent process

    delete_semaphore(sync_sem_id);
    delete_semaphore(SAFETY);
    delete_semaphore(MUTEX);
    detach_shared_mem(p_shm);
    delete_shared_mem(shm_id);

    return 0;
}

void bather_critical_section(int bather_id, int sync_sem_id, int SAFETY, int MUTEX, struct my_mem *p_shm)
{
    cout << "A" << bather_id<< " is born ..." << endl;
    cout << "A" << bather_id<< " is waiting for other children to be active ..." << endl;
    // Wait for the parent to signal
    sem_op(sync_sem_id, -1);
    cout << "A" << bather_id << " starts working now ..." << endl;

    while (p_shm->boiler_done_counter != NUMBER_OF_BOILERMEN)
    {
        sem_op(MUTEX, -1);                     // wait
        p_shm->ba_count = p_shm->ba_count + 1; // increment the number of bathers in the swimming pool
        if (p_shm->ba_count == 1)
        {
            sem_op(SAFETY, -1); // wait
        }
        // the critical section starts here -------------------
        cout << "A" << bather_id << " is entering the swimming pool.." << endl;
        sem_op(MUTEX, 1); // signal

        long int sleep_time = rand() % (BATHER_TIME_02_B + 1); // sleep_time between 0 and BATHER_TIME_02_B
        usleep(sleep_time);

        sem_op(MUTEX, -1);
        p_shm->ba_count = p_shm->ba_count - 1;
        if (p_shm->ba_count == 0)
        {
            sem_op(SAFETY, 1); // signal
        }
        cout << "A" << bather_id << " is leaving the swimming pool.." << endl;
        sem_op(MUTEX, 1); // signal
        // the critical section ends here --------------------

        // missing more delay????????????????????
    }
}

void boilerman_critical_section(int boilerman_id, int sync_sem_id, int SAFETY, int MUTEX, struct my_mem *p_shm)
{
    cout << "B" << boilerman_id << " is born ..." << endl;
    cout << "B" << boilerman_id << " is waiting for other children to be active ..." << endl;
    // Wait for the parent to signal
    sem_op(sync_sem_id, -1);
    cout << "B" << boilerman_id << " starts working now ..." << endl;

    for (int i = 0; i < NUM_REPEAT; i++)
    {
        // the critical section starts here -------------------
        sem_op(SAFETY, -1); // wait

        cout << "B" << boilerman_id << " starts his water heater.." << endl;
        long int sleep_time = rand() % (BOILERMAN_TIME_01_A + 1); // sleep_time between 0 and BOILERMAN_TIME_01_A
        usleep(sleep_time);
        cout << "B" << boilerman_id << " finishes water heating.." << endl;

        sem_op(SAFETY, 1); // signal
        // the critical section ends here --------------------

        sleep_time = rand() % (BOILERMAN_TIME_01_B + 1); // sleep_time between 0 and BOILERMAN_TIME_01_B
        usleep(sleep_time);
    }
}

void safeguard_critical_section(int safeguard_id, int sync_sem_id, int SAFETY, int MUTEX, struct my_mem *p_shm)
{
    // Signal all children to proceed
    for (int i = 0; i < NUMBER_OF_CHILDREN; i++)
    {
        sem_op(sync_sem_id, 1);
    }
    cout << "Now all five children are active!" << endl;

    while (p_shm->done_counter != NUMBER_OF_CHILDREN)
    {
        // the critical section starts here -------------------
        sem_op(SAFETY, -1); // wait

        cout << "S starts inspection. Everyone get out!!" << endl;
        int sleep_time = rand() % (SAFEGUARD_TIME_A + 1); // sleep_time between 0 and SAFEGUARD_TIME_A
        usleep(sleep_time);
        cout << "S finishes inspection.." << endl;

        sem_op(SAFETY, 1); // signal
        // the critical section ends here --------------------

        sleep_time = rand() % (SAFEGUARD_TIME_B + 1); // sleep_time between 0 and SAFEGUARD_TIME_B
        usleep(sleep_time);
    }
}

void sem_op(int sem_id, short op)
{
    struct sembuf operation[1];
    operation[0].sem_num = 0; // one semaphore in the set
    operation[0].sem_op = op; // -1 for P (wait), +1 for V (signal)
    operation[0].sem_flg = 0; // Default behavior (blocking)

    if (semop(sem_id, operation, 1) == -1)
    {
        perror("semop failed");
        exit(1);
    }
}

void delete_semaphore(int sem_id)
{
    if (semctl(sem_id, 0, IPC_RMID, 0) == -1)
    {
        perror("semctl IPC_RMID failed");
    }
    else
    {
        cout << "Semaphore deleted successfully.\n";
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
    p_shm->ba_count = 0;
    p_shm->done_counter = 0;
    p_shm->boiler_done_counter = 0;

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