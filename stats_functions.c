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

    // measure and print percentage
    float usage = ((float)(currentMeasure - previousMeasure) / (float)previousMeasure) * 100;

    // if previousMeausure is 0 it means this is our very first measurement which means nothing is printed yet
    if (previousMeasure != 0)
    {
        printf(" total cpu use = %.10f %%\n", usage);
    }

    return currentMeasure;
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

    // print all information
    for (int i = 0; i < samples; i++)
    {
        // print output
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