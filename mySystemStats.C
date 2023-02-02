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
    // This function will print the header of the program which displays the number of samples and the second delay
    // as well as the memeory usage of the program using the <sys/resources.h> C library
    // Example Output:
    // header(10, 1) returns
    //
    // Nbr of samples: 10 -- every 1 secs
    // Meory Usage: 4092 kilobytes

    // print sampe and tdelay
    printf("\nNbr of samples: %d -- every %d secs\n", samples, tdelay);

    // find the memory usage
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("Memory Usage: %ld kilobytes \n", usage.ru_maxrss);
}

void getSystemInfo()
{
    // This function will print out the System Information using the <sys/utsname.h> C library
    // Examle Output:
    // getSystemInfo() returns
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
        printf("Architecture = %s \n\n", info.machine);
    }
    else
    {
        printf("%s", "Unable to retrieve system information!\n");
    }
}

void getUsers()
{
    // This function will print out the list of users along with each of their connected sessions using the <utmpx.h> C library
    // and reading through the utmpx user log file.
    // Example Output:
    // getUsers() returns
    //
    // marcelo       pts/0 (138.51.12.217)
    // marcelo       pts/1 (tmux(3773782).%0)
    // alberto       tty7 (:0)
    // marcelo       pts/2 (tmux(3773782).%1)
    // marcelo       pts/3 (tmux(3773782).%3)
    // marcelo       pts/4 (tmux(3773782).%4)

    struct utmpx *users;
    setutxent(); // rewinds pointer to beginning of utmpx file

    while ((users = getutxent()) != NULL)
    {
        // validate that this is a user process
        if (users->ut_type == USER_PROCESS)
        {
            printf("%s      %s (%s) \n", users->ut_user, users->ut_line, users->ut_host);
        }
    }
    endutxent(); // close the utmpx file
}

void getCpuNumber()
{
    // This function will print out the number of cpu's as well as the total number of cores using the /proc/cpuinfo file to scrape the information.
    // Example Ouput:
    // getCpuNumber() returns
    //
    // Number of CPU's: 3       Number of Cores for each CPU: 3

    int cpuNumber = 0;
    int coreNumber = 0;
    char segment[150];

    // open cpuinfo file and scrape cpu and core numbers
    FILE *info = fopen("/proc/cpuinfo", "r");
    while (fgets(segment, sizeof(segment), info))
    {
        if (strstr(segment, "processor") != NULL)
        {
            cpuNumber++;
        }
        if (strstr(segment, "cpu cores") != NULL)
        {
            char *ptr = strchr(segment, ':');
            coreNumber += atoi((ptr + 1));
        }
    }

    printf("Number of CPU's: %d     Total Number of Cores: %d\n", cpuNumber, coreNumber);
    fclose(info);

    // ASK IF OUTPUTING TOTAL NUMBER OF CORES IS OK
}

long int getCpuUsage(long int previousMeasure)
{
    // This function takes the previous cpu time measurement and compares it to the current measurement done by reading the /proc/stat file and returns
    // an overall percent increase(ex. 0.18%) or decrease(ex. -0.18%).
    // Note: We consider iowait to be idle time so it also subtracted from the overall time spend. We are also condering irq, softirq, and steal as
    // time spent by the CPU. Lastly, guest and guest_nice values are included in the value of user and nice, so they will be subtracted from the overall calculation.
    // Example Output:
    // getCpuUsage() returns
    //
    // total cpu use = 0.18%

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

    long int totalMeasure = (user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice);
    long int downTime = idle + iowait;
    long int accountedFor = guest + guest_nice;
    long int currentMeasure = totalMeasure - downTime - accountedFor;

    float usage = ((float)(currentMeasure - previousMeasure) / (float)previousMeasure) * 100;

    if (previousMeasure != 0)
    {
        printf(" FIRST: %ld, SECOND: %ld, ", previousMeasure, currentMeasure);
        printf(" total cpu use = %.10f %%\n", usage);
    }

    return currentMeasure;
}

void getMemoryUsage()
{
    // This function prints the value of total and used Physical RAM as well as the total and used Virtual Ram.
    // This is being done by using the <sys/sysinfo.h> C library.
    // Note that this function defines 1Gb = 1024Kb (i.e the function uses binary prefixes)
    // Example Outpiut:
    // getMemoryUsage() returns
    //
    // 7.18 GB / 7.77 GB  --  7.30 GB / 9.63

    // find the used and total physical RAM
    struct sysinfo info;
    sysinfo(&info);

    double totalPhysicalRam = (double)info.totalram / (1073741824);
    double usedPhysicalRam = (double)(info.totalram - info.freeram) / (1073741824);

    // find the used and total virtual RAM (total virtual RAM = physical memory + swap memory)
    double totalVirtualRam = (double)(info.totalram + info.totalswap) / (1073741824);
    double usedVirtualRam = (double)(info.totalram + info.totalswap - info.freeram - info.freeswap) / (1073741824);

    printf("%.2f GB / %.2f GB  --  %.2f GB / %.2f GB\n", usedPhysicalRam, totalPhysicalRam, usedVirtualRam, totalVirtualRam);
}

void allInfoUpdate(int samples, int tdelay)
{
    // clear terminal before starting
    printf("\033c");
    long int previousMeasure = getCpuUsage(0);

    // print all information
    for (int i = 0; i < samples; i++)
    {
        header(samples, tdelay);
        printf("---------------------------------------\n");
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");
        getMemoryUsage();
        printf("---------------------------------------\n");
        printf("### Sessions/users ###\n");
        getUsers();
        printf("---------------------------------------\n");
        getCpuNumber();
        previousMeasure = getCpuUsage(previousMeasure);

        if (i != samples - 1)
        {
            sleep(tdelay);
            // clear buffer
            fflush(stdout);
            // clear screen
            printf("\033c");
        }
    }
    printf("---------------------------------------\n");
    printf("### System Information ### \n");
    getSystemInfo();
}

void usersUpdate(int samples, int tdelay)
{
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
    printf("### System Information ### \n");
    getSystemInfo();
}

void systemUpdate(int samples, int tdelay)
{
    // clear terminal before starting
    printf("\033c");

    // store a first per measure for the cpu usage
    long int previousMeasure = getCpuUsage(0);

    // print headers
    header(samples, tdelay);
    printf("---------------------------------------\n");
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");

    // keep track of lines
    int lineNumber = samples + 6;
    int backUpNumber = 6;

    // print all system info
    for (int i = 0; i < samples; i++)
    {

        printf("\033[%d;0H", (backUpNumber)); // move cursor to memory
        getMemoryUsage();
        printf("\033[%d;0H", (lineNumber)); // move cursor to cpu usage
        getCpuNumber();
        previousMeasure = getCpuUsage(previousMeasure);

        // update line numbers
        backUpNumber = backUpNumber + 1;

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
}

int main()
{
    systemUpdate(2, 4);
    return 0;
}
