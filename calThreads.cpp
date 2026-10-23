#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <iomanip>

using namespace std;

void* monthRunner(void* p_data);

sem_t runnersSync[12];

struct CalendarInfo
{
    int threadIndex;          // 0-11
    int neighborIndex;
    bool isLastThreadInRow;
    int rowNumber;
    int monthsPerRow;
    int numRows;
    int numWeeksInRow;

    string monthName;
    int year;
    int numDaysInMonth;
    int startDayOfMonth;      // 0-6
};

int daysInMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
string monthNames[12] = { "January", "February", "March", "April", "May", "June",
                          "July", "August", "September", "October", "November", "December" };

int getNumWeeks(int startingDay, int numDays)
{
    return (startingDay + numDays + 6) / 7;
}

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

    //throw error messages for exceptions
    if (numMonthsPerRow < 1 || numMonthsPerRow > 12 ||
        12 % numMonthsPerRow != 0)
    {
        cout << "Months per row must divide evenly into 12." << endl;
        return 1;
    }

    if (startingWeekday < 0 || startingWeekday > 6)
    {
        cout << "Starting weekday must be between 0 and 6." << endl;
        return 1;
    }

    //leap year
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
    {
        daysInMonth[1] = 29;
    }

    pthread_t p_runners[12];

    sem_init(&runnersSync[0], 0, 1);

    // Calculate num of weeks needed for each row
    int weeksPerRow[12] = {0};
    int day = startingWeekday;

    for (int i = 0; i < 12; i++)
    {
        int row = i / numMonthsPerRow;
        int weeks = getNumWeeks(day, daysInMonth[i]);

        if (weeks > weeksPerRow[row])
        {
            weeksPerRow[row] = weeks;
        }

        day = (day + daysInMonth[i]) % 7;
    }

    // 12 motnh threads
    for (int i = 1; i < 12; i++)
    {
        sem_init(&runnersSync[i], 0, 0);
    }

    for(int i  = 0; i < 12 ;i++)
    {
        CalendarInfo* calInfo = new CalendarInfo;

        // initializing variables in struct
        calInfo->threadIndex = i;
        calInfo->rowNumber = i / numMonthsPerRow;
        calInfo->monthsPerRow = numMonthsPerRow;
        calInfo->numRows = 12 / numMonthsPerRow;
        calInfo->numWeeksInRow = weeksPerRow[calInfo->rowNumber];

        calInfo->isLastThreadInRow = ((i + 1) % numMonthsPerRow == 0);

        int rowStart = (i / numMonthsPerRow) * numMonthsPerRow;

        calInfo->neighborIndex = rowStart + ((i + 1) % numMonthsPerRow);

        calInfo->monthName = monthNames[i];
        calInfo->year = year;
        calInfo->numDaysInMonth = daysInMonth[i];
        calInfo->startDayOfMonth = startingWeekday;

        pthread_create(&p_runners[i], nullptr, monthRunner, calInfo);

        // Calculate starting weekday of next month
        startingWeekday = (startingWeekday + daysInMonth[i]) % 7;
    }

    for (int i = 0; i < 12; i++)
    {
        pthread_join(p_runners[i], nullptr);
    }

    for (int i = 0; i < 12; i++)
    {
        sem_destroy(&runnersSync[i]);
    }
}



void* monthRunner(void* p_data)
{
    CalendarInfo* calInfo = (CalendarInfo*) p_data;

    int myNum = calInfo->threadIndex;
    int totalTurns = 2 + calInfo->numWeeksInRow;

    for (int turn = 0; turn < totalTurns; turn++)
    {
        sem_wait(&runnersSync[myNum]);

        if (turn == 0)
        {
            string title = calInfo->monthName + " " + to_string(calInfo->year);

            int leftPadding = (21 - title.length()) / 2;

            cout << string(leftPadding, ' ')
                 << title
                 << string(21 - leftPadding - title.length(), ' ');
        }
        else if (turn == 1)
        {
            cout << " S  M  T  W  R  F  S ";
        }
        else
        {
            int week = turn - 2;

            for (int weekday = 0; weekday < 7; weekday++)
            {
                int date = week * 7 + weekday
                         - calInfo->startDayOfMonth + 1;

                if (date < 1 || date > calInfo->numDaysInMonth)
                {
                    cout << setw(3) << " ";
                }
                else
                {
                    cout << setw(3) << date;
                }
            }
        }

        if (calInfo->isLastThreadInRow)
        {
            cout << endl;
        }
        else
        {
            cout << "   ";
        }

        if (turn == totalTurns - 1 && calInfo->isLastThreadInRow)
        {
            cout << endl;

            if (calInfo->rowNumber < calInfo->numRows - 1)
            {
                sem_post(&runnersSync[myNum + 1]);
            }
        }
        else
        {
            // Pass baton 
            sem_post(&runnersSync[calInfo->neighborIndex]);
        }
    }

    delete calInfo;

    return nullptr;
}