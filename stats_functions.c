// BY: KRISTI DODAJ
// COURSE: CSCB09

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <utmpx.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>

void header(int samples, int tdelay)
{
    // This function will take in int samples and int tdelay as parameters and print the header of the program which displays the
    // number of samples and the second delay as well as the memory usage of the program using the <sys/resources.h> C library
    // Example Output:
    // header(10, 1) prints
    //
    // Nbr of samples: 10 -- every 1 secs
    // Memory Usage: 4092 kilobytes

    // print sampe and tdelay
    printf("\nNbr of samples: %d -- every %d secs\n", samples, tdelay);

    // find and print the memory usage
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == -1)
    {
        // error checking for system resources
        perror("getrusage: failed to fetch the program memory usage");

        // NOTE: The program will exit since printing from usage would fail given the usage object is not populated
    }

    printf("Memory Usage: %ld kilobytes \n", usage.ru_maxrss);
}

void getSystemInfo()
{
    // This function will print out the System Information using the <sys/utsname.h> C library
    // Examle Output:
    // getSystemInfo() prints
    //
    // System Name = Darwin
    // Machine Name = Kristis-MacBook-Air.local
    // Version = Darwin Kernel Version 22.1.0: Sun Oct  9 20:14:30 PDT 2022; root:xnu-8792.41.9~2/RELEASE_ARM64_T8103
    // Release = 22.1.0
    // Architecture = arm64

    // create and populate the struct
    struct utsname info;

    if (uname(&info) == -1)
    {
        // error checking for system resources
        perror("uname: Unable to retrieve system information");

        // NOTE: The program will exit since printing from info would fail given the info object is not populated
    }

    printf("System Name = %s \n", info.sysname);
    printf("Machine Name = %s \n", info.nodename);
    printf("Version = %s \n", info.version);
    printf("Release = %s \n", info.release);
    printf("Architecture = %s \n", info.machine);
}

void getUsers()
{
    // This function will print out the list of users along with each of their connected sessions using the <utmpx.h> C library
    // and reading through the utmp user log file.
    // Example Output:
    // getUsers() prints
    //
    // dodajkri      pts/1 (tmux(97972).%0)
    // dodajkri      pts/2 (tmux(97972).%2)
    // dodajkri      pts/0 (138.51.8.149)

    struct utmpx *users; // initialize utmpx struct

    // rewinds pointer to beginning of utmp file
    setutxent();

    // read through utmp file
    while ((users = getutxent()) != NULL)
    {
        // validate that this is a user process
        if (users->ut_type == USER_PROCESS)
        {
            printf("%s      %s (%s) \n", users->ut_user, users->ut_line, users->ut_host);
        }

        // NOTE: No need to error check since it returns NULL when there are no entries
        //       thus not causing any issues to the program.
    }

    // close the utmp file
    endutxent();
}

void getCpuNumber()
{
    // This function will print out the number of cpu's as well as the total number of cores using the /proc/cpuinfo file to scrape the information.
    // Example Ouput:
    // getCpuNumber() prints
    //
    // Number of CPU's: 12     Total Number of Cores: 72

    // initialize variables to store information
    int cpuNumber = 0;
    int coreNumber = 0;
    char line[200];

    // open the /proc/cpuinfo file and scrape the cpu and core numbers
    FILE *info = fopen("/proc/cpuinfo", "r");

    // error checking for system resources
    if (info == NULL)
    {
        perror("fopen: Failed to open /proc/cpuinfo");

        // NOTE: The program will exit since fgets will fail given the file is not opened
    }

    while (fgets(line, sizeof(line), info) != NULL)
    {
        if (strstr(line, "processor") != NULL)
        {
            cpuNumber++;
        }
        if (strstr(line, "cpu cores") != NULL)
        {
            char *ptr = strchr(line, ':'); // pointer to first occurrence of ':'
            coreNumber += atoi((ptr + 1)); // converts string to int
        }
    }

    // error checking for system resources
    if (ferror(info))
    {
        perror("fgets: Failed to read /proc/cpuinfo");
        fclose(info);
    }

    if (fclose(info) != 0)
    {
        perror("fclose: Failed to close /proc/cpuinfo");
    }

    // print final output
    printf("Number of CPU's: %d     Total Number of Cores: %d\n", cpuNumber, coreNumber);
}

float getCpuUsage(int tdelay)
{
    // This function takes the second interval (int tdelay), and compares two measurements that are tdelay seconds apart done by reading the /proc/stat file.
    // The function will return the overall percent increase(ex. 0.18%) or decrease(ex. -0.18%) as a float rounded to 10 decimal places.
    // FORMULA FOR CALCULATION: (U2-U1/T2-T1) * 100 WHERE T IS TOTAL TIME AND U IS TOTAL TIME WITHOUT IDLE TIME
    // Example Output:
    // getCpuUsage(1)
    //
    // returns: 1.1656951904

    // declare and populate all the desired times spent by the CPU
    long int user;
    long int nice;
    long int system;
    long int irq;
    long int softirq;
    long int steal;
    long int guest;
    long int guest_nice;

    long int idle;
    long int iowait;

    // open file and retrieve each value to do the first measurement
    FILE *info = fopen("/proc/stat", "r");

    // error checking for system resources
    if (info == NULL)
    {
        perror("fopen: Error opening /proc/stat for first cpu usage calculation");
    }

    if (fscanf(info, "cpu %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice) != 10)
    {
        perror("fscanf: Error reading from /proc/stat for first cpu usage calculation");
        fclose(info);
    }

    if (fclose(info) != 0)
    {
        perror("fclose: Error closing /proc/stat for first cpu usage calculation");
    }

    // calculate first measure
    long int T1 = (user + nice + system + idle + iowait + irq + softirq);
    long int U1 = T1 - idle;

    // calculate time to be waited and sleep
    sleep(tdelay);

    // open file and retrieve each value to do the second measurement
    FILE *info2 = fopen("/proc/stat", "r");

    // error checking for system resources
    if (info2 == NULL)
    {
        perror("fopen: Error opening /proc/stat for second cpu usage calculation");
    }

    if (fscanf(info2, "cpu %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice) != 10)
    {
        perror("fscanf: Error reading from /proc/stat for second cpu usage calculation");
        fclose(info2);
    }

    if (fclose(info2) != 0)
    {
        perror("fclose: Error closing /proc/stat for second cpu usage calculation");
    }

    // NOTE: The program will exit given a failure to read or open the file since adding unassigned integers will cause a failure

    // calculate second measure
    long int T2 = (user + nice + system + idle + iowait + irq + softirq);
    long int U2 = T2 - idle;

    // measure and print percentage
    float usage = ((float)(U2 - U1) / (float)(T2 - T1)) * 100;

    return usage;
}

void getMemoryUsage(int write_pipe)
{
    // This function prints the value of total and used Physical RAM as well as the total and used Virtual Ram.
    // This is being done by using the <sys/sysinfo.h> C library.
    // Note that this function defines 1Gb = 1024Kb (i.e the function uses binary prefixes)
    // Example Output:
    // getMemoryUsage() prints
    //
    // 7.18 GB / 7.77 GB  --  7.30 GB / 9.63

    // find the used and total physical RAM
    struct sysinfo info;

    // error checking for system resources
    if (sysinfo(&info) == -1)
    {
        perror("sysinfo: Error getting sysinfo on RAM");

        // NOTE: This program will exit given sysinfo fails since we are calling value from an unpopulated object
    }

    // find the used and total physical RAM
    double totalPhysicalRam = (double)info.totalram / (1073741824);
    double usedPhysicalRam = (double)(info.totalram - info.freeram) / (1073741824);

    // find the used and total virtual RAM (total virtual RAM = physical memory + swap memory)
    double totalVirtualRam = (double)(info.totalram + info.totalswap) / (1073741824);
    double usedVirtualRam = (double)(info.totalram + info.totalswap - info.freeram - info.freeswap) / (1073741824);

    / build output string char *buf = calloc(1, 50);
    sprintf(buf, "%.2f GB / %.2f GB  --  %.2f GB / %.2f GB\n", usedPhysicalRam, totalPhysicalRam, usedVirtualRam, totalVirtualRam);

    // write output to pipe
    write(write_pipe, buf, strlen(buf) + 1);

    // free allocated memory
    free(buf);

    void allInfoUpdate(int samples, int tdelay)
    {
        // This function will take in int samples and tdelay and prints out all the system information that will update
        // in the specified time interval and the specified number of samples. The information given includes memory usage,
        // user logs, cpu information, system information and are implemented through the above listed functions.
        // Example Output:
        // allInfoUpdate(10, 1) prints
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

        // create pipes for communication
        int memory_pipe[2];
        if (pipe(memory_pipe) < 0)
        {
            perror("Error creating pipes");
            exit(EXIT_FAILURE);
        }

        // close unused write ends of pipes
        close(memory_pipe[1]);

        // clear terminal before starting and take an intial measurement for the cpu usage calculation
        printf("\033c");

        // print headers
        header(samples, tdelay);
        printf("---------------------------------------\n");
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");

        // keep track of lines
        int usersLineNumber = samples + 6;
        int memoryLineNumber = 6;

        float usage;

        // print all information
        for (int i = 0; i < samples; i++)
        {

            // create child processes
            pid_t memory_pid;
            memory_pid = fork();
            if (pids[0] == 0)
            {
                // child process for memory usage
                close(memory_pipe[0]);
                print_memory_usage(memory_pipe[1]);
                exit(EXIT_SUCCESS);
            }

            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(memory_pipe[0], &read_fds);
            select(FD_SETSIZE, &read_fds, NULL, NULL, NULL);

            // read and print output
            if (FD_ISSET(memory_pipe[0], &read_fds))
            {
                printf("\033[%d;0H", (memoryLineNumber)); // move cursor to memory
                char buf[100];
                read(memory_pipe[0], buf, sizeof(buf)); // read memory usage from pipe
                printf("%s", buf);
            }

            printf("\033[%d;0H", (usersLineNumber)); // move cursor to users
            printf("---------------------------------------\n");
            printf("### Sessions/users ###\n");
            printf("\033[J"); // clears everything below the current line
            getUsers();
            printf("---------------------------------------\n");
            getCpuNumber();

            if (i > 0)
            {
                // print usage
                printf(" total cpu use = %.10f %%\n", usage);
            }

            usage = getCpuUsage(tdelay); // get current measurement for cpu usage

            if (i == samples - 1)
            {
                printf("\033[1A"); // move the cursor up one line
                printf("\033[2K"); // clear the entire line
            }

            // update line numbers
            memoryLineNumber = memoryLineNumber + 1;

            // clear buffer
            fflush(stdout);
        }

        // print usage
        printf(" total cpu use = %.10f %%\n", usage);

        // print the ending system details
        printf("---------------------------------------\n");
        printf("### System Information ### \n");
        getSystemInfo();
        printf("---------------------------------------\n");
    }

    void usersUpdate(int samples, int tdelay)
    {
        // This function will take in int samples and tdelay and prints out all the user information that will update
        // in the specified time interval and the specified number of samples. The information given includes users logged in,
        // their individual sessions, and system information.
        // Example Output:
        // usersUpdate(10, 1) prints
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

        // clear terminal before starting
        printf("\033c");

        // print all user information
        for (int i = 0; i < samples; i++)
        {
            header(samples, tdelay);
            printf("---------------------------------------\n");
            printf("### Sessions/users ###\n");
            getUsers();
            printf("---------------------------------------\n");

            if (i != samples - 1)
            {
                // wait tdelay
                sleep(tdelay);
                // clear buffer
                fflush(stdout);
                // clear screen
                printf("\033c");
            }
        }

        // print the ending system details
        printf("### System Information ### \n");
        getSystemInfo();
        printf("---------------------------------------\n");
    }

    void systemUpdate(int samples, int tdelay)
    {
        // This function will take in int samples and tdelay and prints out the system information that will update
        // in the specified time interval and the specified number of samples. The information given includes memory usage,
        // cpu information, system information and are implemented through the above listed functions.
        // Example Output:
        // systemUpdate(10, 1) prints
        //
        // Nbr of samples: 10 -- every 1 secs
        // Memory Usage: 3924 kilobytes
        // ---------------------------------------
        // 2.98 GB / 15.32 GB  --  2.98 GB / 16.28 GBsed/Tot)
        // 2.98 GB / 15.32 GB  --  2.98 GB / 16.28 GB
        // 2.98 GB / 15.32 GB  --  2.98 GB / 16.28 GB
        // 2.98 GB / 15.32 GB  --  2.98 GB / 16.28 GB
        // 2.98 GB / 15.32 GB  --  2.98 GB / 16.28 GB
        // 2.98 GB / 15.32 GB  --  2.98 GB / 16.28 GB
        // 2.98 GB / 15.32 GB  --  2.98 GB / 16.28 GB
        // 2.98 GB / 15.32 GB  --  2.98 GB / 16.28 GB
        // 2.98 GB / 15.32 GB  --  2.98 GB / 16.28 GB
        // 2.97 GB / 15.32 GB  --  2.97 GB / 16.28 GB
        // Number of CPU's: 12     Total Number of Cores: 72
        //  total cpu use = 0.0002161870 %
        // ---------------------------------------
        // ### System Information ###
        // System Name = Linux
        // Machine Name = iits-b473-27
        // Version = #62-Ubuntu SMP Tue Nov 22 19:54:14 UTC 2022
        // Release = 5.15.0-56-generic
        // Architecture = x86_64
        // ---------------------------------------

        // clear terminal before starting
        printf("\033c");

        // print headers
        header(samples, tdelay);
        printf("---------------------------------------\n");
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");

        // keep track of lines
        int cpuLineNumber = samples + 6;
        int memoryLineNumber = 6;
        float usage;

        // print all system info
        for (int i = 0; i < samples; i++)
        {

            printf("\033[%d;0H", (memoryLineNumber)); // move cursor to memory
            // getMemoryUsage();
            printf("\033[%d;0H", (cpuLineNumber)); // move cursor to cpu usage
            getCpuNumber();

            if (i > 0)
            {
                // print usage
                printf(" total cpu use = %.10f %%\n", usage);
            }

            usage = getCpuUsage(tdelay); // get current measurement for cpu usage

            if (i == samples - 1)
            {
                printf("\033[1A"); // move the cursor up one line
                printf("\033[2K"); // clear the entire line
            }

            // update line numbers
            memoryLineNumber = memoryLineNumber + 1;

            // clear buffer
            fflush(stdout);
        }

        // print usage
        printf(" total cpu use = %.10f %%\n", usage);

        // print the ending system details
        printf("---------------------------------------\n");
        printf("### System Information ### \n");
        getSystemInfo();
        printf("---------------------------------------\n");
    }

    void allInfoSequential(int samples, int tdelay)
    {
        // This function will take in int samples and tdelay and prints out all the system information that will print sequentially
        // in the specified time interval and the specified number of samples. The information given includes memory usage,
        // user logs, cpu information, system information and are implemented through the above listed functions.
        // Example Output:
        // allInfoSequential(2, 2) prints
        //
        // >>> Iteration: 1
        //
        // Nbr of samples: 2 -- every 2 secs
        // Memory Usage: 3924 kilobytes
        // ---------------------------------------
        // ### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)
        // 3.00 GB / 15.32 GB  --  3.00 GB / 16.28 GB
        //
        // ---------------------------------------
        // ### Sessions/users ###
        // dodajkri      pts/1 (tmux(97972).%0)
        // dodajkri      pts/0 (138.51.8.149)
        // ---------------------------------------
        // Number of CPU's: 12     Total Number of Cores: 72
        // total cpu use = 0.0000000000 %
        //
        // >>> Iteration: 2
        //
        // Nbr of samples: 2 -- every 2 secs
        // Memory Usage: 3924 kilobytes
        // ---------------------------------------
        // ### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)
        //
        // 3.00 GB / 15.32 GB  --  3.00 GB / 16.28 GB
        // ---------------------------------------
        // ### Sessions/users ###
        // dodajkri      pts/1 (tmux(97972).%0)
        // dodajkri      pts/0 (138.51.8.149)
        // ---------------------------------------
        // Number of CPU's: 12     Total Number of Cores: 72
        //  total cpu use = 0.0004304505 %
        // ---------------------------------------
        // ### System Information ###
        // System Name = Linux
        // Machine Name = iits-b473-27
        // Version = #62-Ubuntu SMP Tue Nov 22 19:54:14 UTC 2022
        // Release = 5.15.0-56-generic
        // Architecture = x86_64
        // ---------------------------------------

        // clear terminal before starting
        printf("\033c");

        float usage;

        // print all info sequentially
        for (int i = 0; i < samples; i++)
        {
            usage = getCpuUsage(tdelay); // get current measurement for cpu usage
            printf("\r");                // clear current line in case CTRL Z has been called
            printf(">>> Iteration: %d\n", i + 1);
            header(samples, tdelay);
            printf("---------------------------------------\n");
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");

            // create the needed spaces
            for (int j = 0; j < samples; j++)
            {
                if (j == i)
                {
                    // getMemoryUsage();
                }
                else
                {
                    printf("\n");
                }
            }
            printf("---------------------------------------\n");
            printf("### Sessions/users ###\n");
            getUsers();
            printf("---------------------------------------\n");
            getCpuNumber();

            // print usage
            printf(" total cpu use = %.10f %%\n", usage);

            printf("\n");

            fflush(stdout);
        }

        // print the ending system details
        printf("\033[1A");
        printf("---------------------------------------\n");
        printf("### System Information ### \n");
        getSystemInfo();
        printf("---------------------------------------\n");
    }

    void usersSequential(int samples, int tdelay)
    {
        // This function will take in int samples and tdelay and prints out all the user information that will print sequentially
        // in the specified time interval and the specified number of samples. The information given includes users logged in,
        // their individual sessions, and system information.
        // Example Output:
        // usersSequential(2, 2) prints
        //
        // >>>Iteration: 1
        //
        // Nbr of samples: 2 -- every 2 secs
        // Memory Usage: 3924 kilobytes
        // ---------------------------------------
        // ### Sessions/users ###
        // dodajkri      pts/1 (tmux(97972).%0)
        // dodajkri      pts/0 (138.51.8.149)
        //
        // >>>Iteration: 2
        //
        // Nbr of samples: 2 -- every 2 secs
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

        // clear terminal before starting
        printf("\033c");

        // print user info sequentially
        for (int i = 0; i < samples; i++)
        {
            printf("\r"); // clear current line in case CTRL Z has been called
            printf(">>>Iteration: %d\n", i + 1);
            header(samples, tdelay);
            printf("---------------------------------------\n");
            printf("### Sessions/users ### \n");
            getUsers();
            printf("\n");

            if (i != samples - 1)
            {
                // wait tdelay
                sleep(tdelay);
                // clear buffer
                fflush(stdout);
            }
        }

        // print the ending system details
        printf("\033[1A");
        printf("---------------------------------------\n");
        printf("### System Information ### \n");
        getSystemInfo();
        printf("---------------------------------------\n");
    }

    void systemSequential(int samples, int tdelay)
    {
        // This function will take in int samples and tdelay and prints out the system information that will print sequentially
        // in the specified time interval and the specified number of samples. The information given includes memory usage,
        // cpu information, system information and are implemented through the above listed functions.
        // Example Output:
        // systemSequential(2, 2) prints
        //
        // >>> Iteration: 1
        //
        // Nbr of samples: 2 -- every 2 secs
        // Memory Usage: 3924 kilobytes
        // ---------------------------------------
        // ### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)
        // 3.00 GB / 15.32 GB  --  3.00 GB / 16.28 GB
        //
        // ---------------------------------------
        // Number of CPU's: 12     Total Number of Cores: 72
        // total cpu use = 0.0000000000 %
        //
        // >>> Iteration: 2
        //
        // Nbr of samples: 2 -- every 2 secs
        // Memory Usage: 3924 kilobytes
        // ---------------------------------------
        // ### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)
        //
        // 3.00 GB / 15.32 GB  --  3.00 GB / 16.28 GB
        // ---------------------------------------
        // Number of CPU's: 12     Total Number of Cores: 72
        //  total cpu use = 0.0004301672 %
        // ---------------------------------------
        // ### System Information ###
        // System Name = Linux
        // Machine Name = iits-b473-27
        // Version = #62-Ubuntu SMP Tue Nov 22 19:54:14 UTC 2022
        // Release = 5.15.0-56-generic
        // Architecture = x86_64
        //---------------------------------------

        // clear terminal before starting
        printf("\033c");

        float usage;

        // print system info sequentially
        for (int i = 0; i < samples; i++)
        {

            usage = getCpuUsage(tdelay); // get current measurement for cpu usage
            printf("\r");                // clear current line in case CTRL Z has been called
            printf(">>> Iteration: %d\n", i + 1);
            header(samples, tdelay);
            printf("---------------------------------------\n");
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");

            // create the needed spaces
            for (int j = 0; j < samples; j++)
            {
                if (j == i)
                {
                    // getMemoryUsage();
                }
                else
                {
                    printf("\n");
                }
            }
            printf("---------------------------------------\n");
            getCpuNumber();
            // print usage
            printf(" total cpu use = %.10f %%\n", usage);

            printf("\n");

            // clear buffer
            fflush(stdout);
        }

        // print the ending system details
        printf("\033[1A");
        printf("---------------------------------------\n");
        printf("### System Information ### \n");
        getSystemInfo();
        printf("---------------------------------------\n");
    }