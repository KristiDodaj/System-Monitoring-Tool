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
        // printf(" FIRST: %ld, SECOND: %ld, ", previousMeasure, currentMeasure);
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
        printf("\033[%d;0H", (memoryLineNumber)); // move cursor to memory
        getMemoryUsage();
        printf("\033[%d;0H", (usersLineNumber)); // move cursor to users
        printf("---------------------------------------\n");
        printf("### Sessions/users ###\n");
        printf("\033[J"); // clears everything below the current line
        getUsers();
        printf("---------------------------------------\n");
        getCpuNumber();
        previousMeasure = getCpuUsage(previousMeasure);

        // update line numbers
        memoryLineNumber = memoryLineNumber + 1;

        if (i != samples - 1)
        {
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

void parseArguments(int argc, char *argv[], bool *system, bool *user, bool *sequential, int *samples, int *tdelay)
{
    // Note: Assume that positional arguments for samples and tdelay are in this order and will be the first two arguments
    // Also, If --tdelay and --samples are repeated the last call will override the first

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
    // This wil validate the inputed args

    // check number of arguments
    if (argc > 4)
    {
        printf("TOO MANY ARGUMENTS. TRY AGAIN!") return false;
    }

    for (int i = 0; i < argc; i++)
    {
        // check if all the flags are correctly formated
        if (strcmp(argv[i], "--sequential") != 0 || strcmp(argv[i], "--system") != 0 || strcmp(argv[i], "--user") != 0)
        {
            return false;
        }
        else if (sscanf(argv[i], "--samples=%d") != 1 || sscanf(argv[i], "--tdelay=%d") != 1 || sscanf(argv[1], "%d") != 1)
        {
            return false;
        }
    }

    return true;
}

int main(int argc, char *argv[])
{
    bool answer = validateArguments(argc, argv);

    if (answer == true)
    {
        printf("CORRECRT \n");
    }
    else
    {
        printf("WRONG\n");
    }

    return 0;
}
