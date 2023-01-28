#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <utmpx.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>

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

    printf("### System Information ###\n");

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

    printf("### Sessions/users ### \n");

    struct utmpx *users;
    setutxent(); // rewinds pointer to beginning of utmpx file

    // ASK IF CHECKING THAT THIS IS A USER_PROCESS

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
    // This function will print out the number of cpu's as well as the number of cores per cpu using the /proc/cpuinfo to scrape the information.
    // Example Ouput:
    // getCpuNumber() returns
    //
    // Number of CPU's: 12       Number of Cores for each CPU: 6

    int cpuNumber = 0;
    int coreNumber = 0;
    char line[256];

    // open cpuinfo file and scrape cpu and core numbers
    FILE *info = fopen("/proc/cpuinfo", "r");
    while (fgets(line, sizeof(line), info))
    {
        if (strstr(line, "cpu cores"))
        {
            coreNumber += atoi(strchr(line, ':') + 1);
        }
        if (strstr(line, "processor"))
        {
            cpuNumber++;
        }
    }

    // divide the number of cores by number of cpu's to get number of cores for each cpu
    int corePerCpu = coreNumber / cpuNumber;

    printf("Number of CPU's: %d     Number of Cores for each CPU: %d\n", cpuNumber, corePerCpu);
    fclose(info);
}

void getCpuUsage(int secondInterval)
{
    // This function will print the percentage increase of cpu usage by making two seperate measurements from the /proc/stat
    // file and calculating the % increase between the two.
    // Example Output:
    // getCpuUsage() returns
    //
    // total cpu use = 0.18%

    unsigned long long firstMeasure;
    unsigned long long secondMeasure;

    // open the stat file and scrape inital cpu usage
    FILE *info = fopen("/proc/stat", "r");
    fscanf(info, "cpu %llu", &firstMeasure);
    fclose(info);

    // wait until tdelay/2 to take the second measurement
    sleep((double)secondInterval / 2);

    // take the second measurement
    info = fopen("/proc/stat", "r");
    fscanf(info, "cpu  %llu", &secondMeasure);
    fclose(info);

    // calculate the percentage
    double usage = ((double)(secondMeasure - firstMeasure) / firstMeasure) * 100;

    printf("total cpu use = %.2f%%\n", usage);
}

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
    printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay);

    // find the memory usage
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("Memory Usage: %ld kilobytes \n", usage.ru_maxrss);

    // ASK WHY MAC USING MORE RAM THAN LINUX 950272 VS 2408
}

void getMemoryUsage()
{
    // This function prints the value of total and used Physical RAM as well as the total and used Virtual Ram.
    // Note that this function defines 1Gb = 1024Kb (i.e the function uses binary prefixes)
    // Example Outpiut:
    // getMemoryUsage() returns
    //
    // 7.18 GB / 7.77 GB  --  7.30 GB / 9.63

    // find the used and total physical RAM
    struct sysinfo info;
    sysinfo(&info);

    double totalPhysicalRam = (double)info.totalram / (1024 * 1024 * 1024);
    double usedPhysicalRam = (double)(info.totalram - info.freeram) / (1024 * 1024 * 1024);

    // find the used and total virtual RAM (total virtual RAM = physical memory + swap memory)
    double totalVirtualRam = (double)(info.totalram + info.totalswap) / (1024 * 1024 * 1024);
    double usedVirtualRam = (double)(info.totalram + info.totalswap - info.freeram - info.freeswap) / (1024 * 1024 * 1024);

    printf("%.2f GB / %.2f GB  --  %.2f GB / %.2f GB\n", usedPhysicalRam, totalPhysicalRam, usedVirtualRam, totalVirtualRam);
}

int main()
{
    header(10, 1);
    printf("---------------------------------------\n");
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
    getMemoryUsage();
    printf("---------------------------------------\n");
    printf("### Sessions/users ### \n");
    getUsers();
    printf("---------------------------------------\n");
    getCpuNumber();
    getCpuUsage(1);
    printf("---------------------------------------\n");
    printf("### System Information ### \n");
    getSystemInfo();
    return 0;
}
