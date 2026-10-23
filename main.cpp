#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <string>

using namespace std;

int difference(string code1, string code2);
void filterProcess(int inputFd, string myCode, int countWriteFd);

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "error" << endl;
        return 1;
    }

    cout << "Finding maximally distinct codes..." << endl;

    int fd[2];
    int countFd[2];

    pipe(fd);
    pipe(countFd);

    string firstCode = argv[1];

    if (fork() == 0) // first child
    {
        close(fd[1]);
        close(countFd[0]);

        filterProcess(fd[0], firstCode, countFd[1]);

        _exit(0);
    }
    else // first parent
    {
        close(fd[0]);
        close(countFd[1]);

        for (int i = 2; i < argc; i++)
        {
            string code = argv[i];
            write(fd[1], code.c_str(), code.length() + 1);
        }

        close(fd[1]);

        int totalCount = 0;

        read(countFd[0], &totalCount, sizeof(totalCount));
        close(countFd[0]);

        wait(NULL);

        cout << totalCount << " distinct codes found" << endl;
    }

    return 0;
}


// Count how many characters differ between two codes
int difference(string code1, string code2)
{
    if (code1.length() != code2.length())
    {
        return 2;
    }

    int differences = 0;

    for (int i = 0; i < code1.length(); i++)
    {
        if (code1[i] != code2[i])
        {
            differences++;
        }
    }

    return differences;
}


// Each process filters codes and may create another filter
void filterProcess(int inputFd, string myCode, int countWriteFd)
{
    char buffer[100];
    string pending = "";

    bool hasNextFilter = false;

    int nextFd[2];
    int countFd[2];

    ssize_t bytesRead;

    while ((bytesRead = read(inputFd, buffer, sizeof(buffer))) > 0)
    {
        for (int i = 0; i < bytesRead; i++)
        {
            if (buffer[i] == '\0')
            {
                int diff = difference(myCode, pending);

                if (diff > 1)
                {

                    if (!hasNextFilter)
                    {
                    
                        pipe(nextFd);
                        pipe(countFd);

                        if (fork() == 0) 
                        {
                            close(nextFd[1]);
                            close(countFd[0]);

                            close(inputFd);

                            close(countWriteFd);

                            string newCode = pending;

                            filterProcess(nextFd[0], newCode, countFd[1]);

                            _exit(0);
                        }
                        else 
                        {
                            close(nextFd[0]);
                            close(countFd[1]);

                            hasNextFilter = true;
                        }
                    }
                    else
                    {
                        write(nextFd[1],
                              pending.c_str(),
                              pending.length() + 1);
                    }
                }

                pending = "";
            }
            else
            {
                pending += buffer[i];
            }
        }
    }

    close(inputFd);

    int count = 1;

    if (hasNextFilter)
    {
        close(nextFd[1]);

        int childCount = 0;

        read(countFd[0], &childCount, sizeof(childCount));
        close(countFd[0]);

        count += childCount;

        wait(NULL);
    }

    cout << "Process: " << getpid()
         << " my code is \"" << myCode << "\"" << endl;
         
    write(countWriteFd, &count, sizeof(count));
    close(countWriteFd);
}