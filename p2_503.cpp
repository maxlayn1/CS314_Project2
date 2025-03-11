/* ********************************************************** *
 * CS314 Project #2 solution                                  *
 *                                                            *
 * Your last-three:                                           *
 * Your course section #:                                     *
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

#include <semaphore.h> // ???????????????????????????????????????????????????????????
using namespace std;

// function prototypes
void bather_critical_section(int bather_id);
void boilerman_critical_section(int boilerman_id);
void safeguard_critical_section(int safeguard_id);

int main()
{
    pid_t process_id = fork();
    if (process_id == 0) // child process
    {
        safeguard_critical_section(process_id);
    }

    return 0;
}

void bather_critical_section(int bather_id)
{
    long int sleep_time;
    while (false) //wy why why why why why why hfasdlkjjadsfl;kjlk;l;khsas
    {
        sleep_time = rand() % (BATHER_TIME_01_A + 1); //sleep_time between 0 and BATHER_TIME_01_A
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
        sleep_time = rand() % (BOILERMAN_TIME_01_A + 1); //sleep_time between 0 and BOILERMAN_TIME_01_A
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
        int sleep_time = rand() % (SAFEGUARD_TIME_A + 1); //sleep_time between 0 and SAFEGUARD_TIME_A
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
