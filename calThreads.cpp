#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

void* monthRunner(void* p_data);

sem_t runnersSync[12];

struct RunnerInfo
{
    int numMonth;
    int startingWeekday;
    int numDays;
    int numMonthsPerRow;
};

int daysInMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        cout << "Wrong number of command line arguments" << endl;
        exit(0);
    }

    int numMonthsPerRow = atoi(argv[1]);
    int startingWeekday = atoi(argv[2]);
    int year = atoi(argv[3]);

    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
    {
        daysInMonth[1] = 29;
    }

    pthread_t p_runners[12];

    sem_init(&runnersSync[0], 0, 1);

    // Create 12 month threads
    for (int i = 1; i < 12; i++)
    {
        sem_init(&runnersSync[i], 0, 0);
    }

    // pthread_t* p_runners = new pthread_t[12];

    for(int i  = 0; i < 12 ;i++)
    {
        RunnerInfo* runnerInfo = new RunnerInfo;
        runnerInfo->numMonth = i;
        runnerInfo->numMonthsPerRow = numMonthsPerRow;
        runnerInfo->startingWeekday = startingWeekday;
        runnerInfo->numDays = daysInMonth[i];
    
        pthread_create(&p_runners[i], 0, monthRunner, runnerInfo);

        runnerInfo->startingWeekday = (startingWeekday + daysInMonth[i]) % 7;
    }

    for (int i = 0; i < 12; i++)
    {
        pthread_join(p_runners[i], nullptr);
    }

    // Join threads
    // Clean up
}

void* monthRunner(void* p_data)
{
    RunnerInfo* info = (RunnerInfo*) p_data;

    cout << "Month " << info->numMonth
         << " thread started!" << endl;

    delete info;

    return nullptr;
}