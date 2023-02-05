#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <utmpx.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>

// MAKE SURE THAT EACH FUNCTION EXPLAINS THE PARAMETERS(ALSO TYPE OF PARAMETER) BEING PASSED AND ITS FUNCTION

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
    getrusage(RUSAGE_SELF, &usage);
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
    int res = uname(&info);

    // check if function worked and print result
    if (res == 0)
    {
        printf("System Name = %s \n", info.sysname);
        printf("Machine Name = %s \n", info.nodename);
        printf("Version = %s \n", info.version);
        printf("Release = %s \n", info.release);
        printf("Architecture = %s \n", info.machine);
    }
    else
    {
        printf("%s", "Unable to retrieve system information!\n");
    }
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
    setutxent();         // rewinds pointer to beginning of utmp file

    // read through utmp file
    while ((users = getutxent()) != NULL)
    {
        // validate that this is a user process
        if (users->ut_type == USER_PROCESS)
        {
            printf("%s      %s (%s) \n", users->ut_user, users->ut_line, users->ut_host);
        }
    }
    endutxent(); // close the utmp file
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
    while (fgets(line, sizeof(line), info))
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

    // print final output
    printf("Number of CPU's: %d     Total Number of Cores: %d\n", cpuNumber, coreNumber);
    fclose(info);
}

long int getCpuUsage(long int previousMeasure)
{
    // This function takes the previous cpu time measurement (long int previousMeasure), and compares it to the current measurement done by reading the /proc/stat file.
    // The function will print the overall percent increase(ex. 0.18%) or decrease(ex. -0.18%) rounded to 10 decimal places and return the current measurement as a long int.
    // FORMULA FOR CALCULATION: ((current measure - previous measure) / previous measure) * 100 where each measurement = (user + nice + system + irq + softirq + steal) - (idle + iowait)
    // Note: We consider iowait to be idle time so it also subtracted from the overall time spent. We are also considering irq, softirq, and steal as
    // time spent by the CPU. Lastly, guest and guest_nice values are included in the value of user and nice, so they will be subtracted from the overall calculation.
    // Example Output:
    // getCpuUsage(921263)
    //
    // prints: total cpu use = 0.0219400000 %
    // returns: 921265

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
    fscanf(info, "cpu %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);
    fclose(info);

    // get the final measure
    long int totalMeasure = (user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice);
    long int downTime = idle + iowait;
    long int accountedFor = guest + guest_nice;
    long int currentMeasure = totalMeasure - downTime - accountedFor;

    float usage = ((float)(currentMeasure - previousMeasure) / (float)previousMeasure) * 100;

    if (previousMeasure != 0)
    {
        printf(" total cpu use = %.10f %%\n", usage);
    }

    return currentMeasure;
}

void getCpuUsageGraphic(double usage)
{
    // This function will recieve double usage which is a percentage change in cpu usage and print put a graphical represantation of the difference.
    // GRAPHIC CONVENTIONS: ::::::  = total relative negative change, ||||||| = total relative positive change, |o = zero change, where | and : mean a change of 0.0001%.
    // Example Output:
    // getCpuUsageGraphic(0.00032) prints
    //
    // |||  0.0003200000  %

    // get amount of graphic components needed
    int graphicElementCount = (int)(abs(usage) / 0.0001);

    // print graphic output
    if (usage < 0)
    {
        for (int i = 0; i < graphicElementCount; i++)
        {
            printf(":");
        }
        printf("  %0.10f %% \n", usage);
    }
    else if (usage > 0)
    {
        for (int i = 0; i < graphicElementCount; i++)
        {
            printf("|");
        }
        printf("  %0.10f %% \n", usage);
    }
    else
    {
        printf("|o  %0.10f %% \n", usage);
    }
}

void getMemoryUsage()
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
    sysinfo(&info);

    // find the used and total physical RAM
    double totalPhysicalRam = (double)info.totalram / (1073741824);
    double usedPhysicalRam = (double)(info.totalram - info.freeram) / (1073741824);

    // find the used and total virtual RAM (total virtual RAM = physical memory + swap memory)
    double totalVirtualRam = (double)(info.totalram + info.totalswap) / (1073741824);
    double usedVirtualRam = (double)(info.totalram + info.totalswap - info.freeram - info.freeswap) / (1073741824);

    // print final results
    printf("%.2f GB / %.2f GB  --  %.2f GB / %.2f GB\n", usedPhysicalRam, totalPhysicalRam, usedVirtualRam, totalVirtualRam);
}

void getMemoryUsageGraphic(double previousUsedMemory)
{
    // This function takes in double previousMemory and prints the value of used/total Physical RAM, used/total Virtual RAM as well as
    // a graphic for the net change in memory usage compared to the previous measure. This is being done by using the <sys/sysinfo.h> C library.
    // GRAPHIC CONVENTIONS: ::::::@  = total relative negative change, ######* = total relative positive change, |o = zero change, where # and : mean a change of 0.01.
    // Note that this function defines 1Gb = 1024Kb (i.e the function uses binary prefixes)
    // Example Output:
    // getMemoryUsageGraphic(3.03) prints
    //
    // 3.27 GB / 15.32 GB  --  3.27 GB / 16.28 GB   |########################* 0.24 (3.27)

    // find the used and total physical RAM
    struct sysinfo info;
    sysinfo(&info);

    // find the used and total physical RAM
    double totalPhysicalRam = (double)info.totalram / (1073741824);
    double usedPhysicalRam = (double)(info.totalram - info.freeram) / (1073741824);

    // find the used and total virtual RAM (total virtual RAM = physical memory + swap memory)
    double totalVirtualRam = (double)(info.totalram + info.totalswap) / (1073741824);
    double usedVirtualRam = (double)(info.totalram + info.totalswap - info.freeram - info.freeswap) / (1073741824);

    // find difference
    double difference = usedPhysicalRam - previousUsedMemory;
    int graphicElementCount = (int)(abs(difference) / 0.01);

    // print final results
    printf("%.2f GB / %.2f GB  --  %.2f GB / %.2f GB   |", usedPhysicalRam, totalPhysicalRam, usedVirtualRam, totalVirtualRam);

    if (difference > 0)
    {
        for (int i = 0; i < graphicElementCount; i++)
        {
            printf("#");
        }
        printf("* %.2f (%.2f)\n", difference, usedVirtualRam);
    }
    else if (difference < 0)
    {
        for (int i = 0; i < graphicElementCount; i++)
        {
            printf(":");
        }
        printf("@ %.2f (%.2f)\n", difference, usedVirtualRam);
    }
    else
    {
        printf("o %.2f (%.2f)\n", difference, usedVirtualRam);
    }
}

void allInfoUpdate(int samples, int tdelay, bool graphic)
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

    // clear terminal before starting and take an intial measurement for the cpu usage calculation
    printf("\033c");
    long int previousMeasure = getCpuUsage(0);

    // print headers
    header(samples, tdelay);
    printf("---------------------------------------\n");
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");

    // keep track of lines
    int usersLineNumber = samples + 6;
    int memoryLineNumber = 6;
    int cpuGraphic = samples + 7;

    // print all information
    for (int i = 0; i < samples; i++)
    {
        // print graphical or non graphical output
        if (!graphic)
        {
            printf("\033[%d;0H", (memoryLineNumber)); // move cursor to memory
            getMemoryUsage();
            printf("\033[%d;0H", (usersLineNumber)); // move cursor to users
            printf("---------------------------------------\n");
            printf("### Sessions/users ###\n");
            printf("\033[J"); // clears everything below the current line
            getUsers();
            printf("---------------------------------------\n");
            getCpuNumber();
            previousMeasure = getCpuUsage(previousMeasure); // take and print current measurement for cpu usage which becomes previousMeasure in the next iteration

            // update line numbers
            memoryLineNumber = memoryLineNumber + 1;
        }
        else
        {
            usersLineNumber = samples + 16;

            printf("\033[%d;0H", (memoryLineNumber)); // move cursor to memory
            getMemoryUsage();
            printf("\033[%d;0H", (4 + samples)); // move cursor to cpu
            printf("---------------------------------------\n");
            getCpuNumber();
            previousMeasure = getCpuUsage(previousMeasure); // take and print current measurement for cpu usage which becomes previousMeasure in the next iteration
            printf("\033[A");

            char line[35];
            float percent;
            fgets(line, 35, stdin);
            sscanf(line, " total cpu use = %f %%", &percent);
            printf("\033[%d;0H", (cpuGraphic)); // move cursor to cpu graphic
            getCpuUsageGraphic(percent);

            printf("\033[%d;0H", (usersLineNumber)); // move cursor to users
            printf("---------------------------------------\n");
            printf("### Sessions/users ###\n");
            printf("\033[J"); // clears everything below the current line
            getUsers();

            // update line numbers

            memoryLineNumber = memoryLineNumber + 1;
            cpuGraphic = cpuGraphic + 1;
        }

        if (i != samples - 1)
        {
            // wait for given amount
            sleep(tdelay);
            // clear buffer
            fflush(stdout);
        }
    }

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

    // store a first per measure for the cpu usage
    long int previousMeasure = getCpuUsage(0);

    // print headers
    header(samples, tdelay);
    printf("---------------------------------------\n");
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");

    // keep track of lines
    int cpuLineNumber = samples + 6;
    int memoryLineNumber = 6;

    // print all system info
    for (int i = 0; i < samples; i++)
    {

        printf("\033[%d;0H", (memoryLineNumber)); // move cursor to memory
        getMemoryUsage();
        printf("\033[%d;0H", (cpuLineNumber)); // move cursor to cpu usage
        getCpuNumber();
        previousMeasure = getCpuUsage(previousMeasure);

        // update line numbers
        memoryLineNumber = memoryLineNumber + 1;

        if (i != samples - 1)
        {
            // wait tdelay
            sleep(tdelay);
            // clear buffer
            fflush(stdout);
        }
    }

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
    long int previousMeasure = getCpuUsage(0);

    // print all info sequentially
    for (int i = 0; i < samples; i++)
    {
        printf(">>> Iteration: %d\n", i + 1);
        header(samples, tdelay);
        printf("---------------------------------------\n");
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");

        // create the needed spaces
        for (int j = 0; j < samples; j++)
        {
            if (j == i)
            {
                getMemoryUsage();
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
        getCpuUsage(previousMeasure);
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
    long int previousMeasure = getCpuUsage(0);

    // print system info sequentially
    for (int i = 0; i < samples; i++)
    {
        printf(">>> Iteration: %d\n", i + 1);
        header(samples, tdelay);
        printf("---------------------------------------\n");
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");

        // create the needed spaces
        for (int j = 0; j < samples; j++)
        {
            if (j == i)
            {
                getMemoryUsage();
            }
            else
            {
                printf("\n");
            }
        }
        printf("---------------------------------------\n");
        getCpuNumber();
        getCpuUsage(previousMeasure);
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

void parseArguments(int argc, char *argv[], bool *system, bool *user, bool *sequential, bool *graphics, int *samples, int *tdelay)
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
        // check if --graphics was called
        if (strcmp(argv[i], "--graphics") == 0)
        {
            *graphics = true;
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
    // validateArguments(int argc, char *argv[]) returns true
    // Example Output 2:
    // Suppose we execute as follows: ./a.out --sequential --sequential
    // validateArguments(int argc, char *argv[]) returns true and prints: REPEATED ARGUMENTS. TRY AGAIN!

    // keep track of how many times each arg is called
    int sequentialArgCount = 0;
    int graphicsArgCount = 0;
    int systemArgCount = 0;
    int userArgCount = 0;
    int samplesArgCount = 0;
    int tdelayArgCount = 0;
    int positionalArgCount = 0;

    // check number of arguments
    if (argc > 7)
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
            if (strcmp(argv[i], "--sequential") != 0 && strcmp(argv[i], "--graphics") != 0 && strcmp(argv[i], "--system") != 0 && strcmp(argv[i], "--user") != 0 && sscanf(argv[1], "%d", &dummyValue) != 1 && sscanf(argv[2], "%d", &dummyValue) != 1 && sscanf(argv[i], "--samples=%d", &dummyValue) != 1 && sscanf(argv[i], "--tdelay=%d", &dummyValue) != 1)
            {
                printf("ONE OR MORE ARGUMENTS ARE MISTYPED OR IN THE WRONG ORDER. TRY AGAIN!\n");
                return false;
            }
        }

        if (argc < 3)
        {
            if (strcmp(argv[i], "--sequential") != 0 && strcmp(argv[i], "--graphics") != 0 && strcmp(argv[i], "--system") != 0 && strcmp(argv[i], "--user") != 0 && sscanf(argv[1], "%d", &dummyValue) != 1 && sscanf(argv[i], "--samples=%d", &dummyValue) != 1 && sscanf(argv[i], "--tdelay=%d", &dummyValue) != 1)
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
        if (strcmp(argv[i], "--graphics") == 0)
        {
            graphicsArgCount++;
            if (graphicsArgCount > 1)
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
        bool graphics = false;
        int samples = 10;
        int tdelay = 1;
        parseArguments(argc, argv, &system, &user, &sequential, &graphics, &samples, &tdelay);

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
                allInfoUpdate(samples, tdelay, graphics);
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

int main(int argc, char *argv[])
{
    // call the navigate function which will redirect to the right output depeneding on the arguments
    navigate(argc, argv);

    return 0;
}
