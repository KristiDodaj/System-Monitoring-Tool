
// BY: KRISTI DODAJ
// COURSE: CSCB09

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "stats_functions.h"

void parseArguments(int argc, char *argv[], bool *system, bool *user, bool *sequential, int *samples, int *tdelay)
{
    // This function will take in int argc and char *argv[] and will update the boolean pointers (user, sequential, system, graphics) and int
    // pointers (samples, tdelay) according to the command line arguments inputted.
    // Note: We assume that positional arguments for samples and tdelay are in this order (samples, tdelay), and will ALWAYS be the first two arguments inputted.
    // Example Output 1:
    // Suppose we execute as follows: ./a.out 5 2 --user
    // parseArguments(argc, argv, system, user, sequential, samples, tdelay) will set
    //
    // samples = 5
    // tdelay = 2
    // user = true
    //
    //// Example Output 2:
    // Suppose we execute as follows: ./a.out --sequential --tdelay=3 --samples=2
    // parseArguments(argc, argv, system, user, sequential, samples, tdelay) will set
    //
    // samples = 2
    // tdelay = 3
    // sequential = true

    for (int i = 0; i < argc; i++)
    {
        // find if --sequential was called
        if (strcmp(argv[i], "--sequential") == 0)
        {
            *sequential = true;
        }
        // check if --system was called
        if (strcmp(argv[i], "--system") == 0)
        {
            *system = true;
        }
        // check if --user was called
        if (strcmp(argv[i], "--user") == 0)
        {
            *user = true;
        }
        // check for flag --samples
        int sampleNumber;
        if (sscanf(argv[i], "--samples=%d", &sampleNumber) == 1 && sampleNumber > 0)
        {
            *samples = sampleNumber;
        }
        // check for flag --tdelay
        int tdelayNumber;
        if (sscanf(argv[i], "--tdelay=%d", &tdelayNumber) == 1 && tdelayNumber > 0)
        {
            *tdelay = tdelayNumber;
        }
        // check for samples positional argument
        int samplesPostionalArgument;
        if (argc > 1)
        {
            if (sscanf(argv[1], "%d", &samplesPostionalArgument) == 1 && samplesPostionalArgument > 0)
            {
                *samples = samplesPostionalArgument;
            }
        }
        // check for tdelay positional argument
        int tdelayPositionalArgument;
        if (argc > 2)
        {
            if (sscanf(argv[2], "%d", &tdelayPositionalArgument) == 1 && tdelayPositionalArgument > 0)
            {
                *tdelay = tdelayPositionalArgument;
            }
        }
    }
}

bool validateArguments(int argc, char *argv[])
{
    // This functions takes int argc and char *argv[] and validates the command line arguments inputted by returning true if they are correct and
    // false if they are not. The validation includes checking for repeated arguments, mistyped arguments, too many argumenys, and correct use of positional arguments.
    // Note: We assume that positional arguments for samples and tdelay are in this order (samples, tdelay), and will ALWAYS be the first two arguments inputted.
    // Example Output 1:
    // Suppose we execute as follows: ./a.out --sequential --tdelay=3 --samples=2
    // validateArguments(argc, argv[]) returns true
    // Example Output 2:
    // Suppose we execute as follows: ./a.out --sequential --sequential
    // validateArguments(argc, argv[]) returns true and prints: REPEATED ARGUMENTS. TRY AGAIN!

    // keep track of how many times each arg is called
    int sequentialArgCount = 0;
    int systemArgCount = 0;
    int userArgCount = 0;
    int samplesArgCount = 0;
    int tdelayArgCount = 0;
    int positionalArgCount = 0;

    // check number of arguments
    if (argc > 6)
    {
        printf("TOO MANY ARGUMENTS. TRY AGAIN!\n");
        return false;
    }

    int dummyValue = 0;

    // iterate argv to check for correctness
    for (int i = 1; i < argc; i++)
    {

        // check if all the flags are correctly formated
        if (argc >= 3)
        {
            if (strcmp(argv[i], "--sequential") != 0 && strcmp(argv[i], "--system") != 0 && strcmp(argv[i], "--user") != 0 && sscanf(argv[1], "%d", &dummyValue) != 1 && sscanf(argv[2], "%d", &dummyValue) != 1 && sscanf(argv[i], "--samples=%d", &dummyValue) != 1 && sscanf(argv[i], "--tdelay=%d", &dummyValue) != 1)
            {
                printf("ONE OR MORE ARGUMENTS ARE MISTYPED OR IN THE WRONG ORDER. TRY AGAIN!\n");
                return false;
            }
        }

        if (argc < 3)
        {
            if (strcmp(argv[i], "--sequential") != 0 && strcmp(argv[i], "--system") != 0 && strcmp(argv[i], "--user") != 0 && sscanf(argv[1], "%d", &dummyValue) != 1 && sscanf(argv[i], "--samples=%d", &dummyValue) != 1 && sscanf(argv[i], "--tdelay=%d", &dummyValue) != 1)
            {
                printf("ONE OR MORE ARGUMENTS ARE MISTYPED OR IN THE WRONG ORDER. TRY AGAIN!\n");
                return false;
            }
        }

        // check if there are repeated arguments
        if (strcmp(argv[i], "--sequential") == 0)
        {
            sequentialArgCount++;
            if (sequentialArgCount > 1)
            {
                printf("REPEATED ARGUMENTS. TRY AGAIN!\n");
                return false;
            }
        }
        else if (strcmp(argv[i], "--system") == 0)
        {
            systemArgCount++;
            if (systemArgCount > 1)
            {
                printf("REPEATED ARGUMENTS. TRY AGAIN!\n");
                return false;
            }
        }
        else if (strcmp(argv[i], "--user") == 0)
        {
            userArgCount++;
            if (userArgCount > 1)
            {
                printf("REPEATED ARGUMENTS. TRY AGAIN!\n");
                return false;
            }
        }
        else if (sscanf(argv[i], "--samples=%d", &dummyValue) == 1)
        {
            samplesArgCount++;
            if (samplesArgCount > 1)
            {
                printf("REPEATED ARGUMENTS. TRY AGAIN!\n");
                return false;
            }
        }
        else if (sscanf(argv[i], "--tdelay=%d", &dummyValue) == 1)
        {
            tdelayArgCount++;
            if (tdelayArgCount > 1)
            {
                printf("REPEATED ARGUMENTS. TRY AGAIN\n!");
                return false;
            }
        }
        else if (sscanf(argv[1], "%d", &dummyValue) == 1)
        {
            positionalArgCount++;
            if (positionalArgCount > 2)
            {
                printf("REPEATED ARGUMENTS. TRY AGAIN!\n");
                return false;
            }
        }
    }

    return true;
}

void navigate(int argc, char *argv[])
{
    // This function will take in int argc, char *argv[] and will validate/parse the inputted arguments as well as help navigate to the right output
    // depening on the command line arguments given.
    // Example Output 1:
    // Suppose we execute as follows: ./a.out
    // navigate(int argc, char *argv[]) prints
    //
    // Nbr of samples: 10 -- every 1 secs
    // Memory Usage: 3924 kilobytes
    // ---------------------------------------
    // 2.98 GB / 15.32 GB  --  2.98 GB / 16.28 GBsed/Tot)
    // 2.99 GB / 15.32 GB  --  2.99 GB / 16.28 GB
    // 2.99 GB / 15.32 GB  --  2.99 GB / 16.28 GB
    // 2.99 GB / 15.32 GB  --  2.99 GB / 16.28 GB
    // 2.99 GB / 15.32 GB  --  2.99 GB / 16.28 GB
    // 2.99 GB / 15.32 GB  --  2.99 GB / 16.28 GB
    // 2.99 GB / 15.32 GB  --  2.99 GB / 16.28 GB
    // 2.99 GB / 15.32 GB  --  2.99 GB / 16.28 GB
    // 2.99 GB / 15.32 GB  --  2.99 GB / 16.28 GB
    // 2.99 GB / 15.32 GB  --  2.99 GB / 16.28 GB
    // ---------------------------------------
    // ### Sessions/users ###
    // dodajkri      pts/1 (tmux(97972).%0)
    // dodajkri      pts/0 (138.51.8.149)
    // ---------------------------------------
    // Number of CPU's: 12     Total Number of Cores: 72
    //  total cpu use = 0.0001081957 %
    // ---------------------------------------
    // ### System Information ###
    // System Name = Linux
    // Machine Name = iits-b473-27
    // Version = #62-Ubuntu SMP Tue Nov 22 19:54:14 UTC 2022
    // Release = 5.15.0-56-generic
    // Architecture = x86_64
    // ---------------------------------------
    //
    // Example Output 1:
    // Suppose we execute as follows: ./a.out --user
    // navigate(int argc, char *argv[]) prints
    //
    // Nbr of samples: 10 -- every 1 secs
    // Memory Usage: 3924 kilobytes
    // ---------------------------------------
    // ### Sessions/users ###
    // dodajkri      pts/1 (tmux(97972).%0)
    // dodajkri      pts/0 (138.51.8.149)
    // ---------------------------------------
    // ### System Information ###
    // System Name = Linux
    // Machine Name = iits-b473-27
    // Version = #62-Ubuntu SMP Tue Nov 22 19:54:14 UTC 2022
    // Release = 5.15.0-56-generic
    // Architecture = x86_64
    // ---------------------------------------

    if (validateArguments(argc, argv) == true)
    {

        // interpret the called arguments
        bool system = false;
        bool user = false;
        bool sequential = false;
        int samples = 10;
        int tdelay = 1;
        parseArguments(argc, argv, &system, &user, &sequential, &samples, &tdelay);

        // check if sequential
        if (sequential)
        {
            // redirect to the right output depending on booleans
            if ((!system && !user) || (system && user))
            {
                allInfoSequential(samples, tdelay);
            }
            else if (user)
            {
                usersSequential(samples, tdelay);
            }
            else if (system)
            {
                systemSequential(samples, tdelay);
            }
        }
        else
        {
            // redirect to the right output depending on booleans
            if ((!system && !user) || (system && user))
            {
                allInfoUpdate(samples, tdelay);
            }
            else if (user)
            {
                usersUpdate(samples, tdelay);
            }
            else if (system)
            {
                systemUpdate(samples, tdelay);
            }
        }
    }
}

// This function will dicatate what will occur when the signal from CTRL Z is activated. Since we want to ingnore this signal
// the function will do nothing.
void handle_ctrl_z(int signal_number)
{
    // do nothing
}

int main(int argc, char *argv[])
{
    // redirect incoming signals
    if (signal(SIGTSTP, handle_ctrl_z) == SIG_ERR)
    {
        // error check whether signal() worked
        perror("singal: Error registering SIGTSTP handler");
        exit(1);
    }

    // call the navigate function which will redirect to the right output depeneding on the arguments
    navigate(argc, argv);

    return 0;
}
