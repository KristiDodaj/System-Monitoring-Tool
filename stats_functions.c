// Author: Kristi Dodaj
// stats_functions.c: Responsible for the functions that retrive the system information and structure the output

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <utmpx.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <math.h>

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

void getUsers(int write_pipe, int size_pipe)
{
    // This function will write to the write_pipe the list of users along with each of their connected sessions using the <utmpx.h> C library
    // and reading through the utmp user log file. It will also write the size of the total string through a second pipe (size_pipe).
    // Example Output:
    // getUsers() writes
    //
    // dodajkri      pts/1 (tmux(97972).%0)
    // dodajkri      pts/2 (tmux(97972).%2)
    // dodajkri      pts/0 (138.51.8.149)

    struct utmpx *users; // initialize utmpx struct

    // rewinds pointer to beginning of utmp file
    setutxent();

    // count how many lines
    int count = 0;
    while ((users = getutxent()) != NULL)
    {
        // validate that this is a user process
        if (users->ut_type == USER_PROCESS)
        {
            count++;
        }
    }

    // allocate memory for the buffer
    char *buf = (char *)malloc(count * 1024);
    if (!buf)
    {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    int offset = 0;

    // read through utmp file
    setutxent();
    while ((users = getutxent()) != NULL)
    {
        // validate that this is a user process
        if (users->ut_type == USER_PROCESS)
        {
            offset += sprintf(buf + offset, "%s      %s (%s) \n", users->ut_user, users->ut_line, users->ut_host);
        }
    }

    // close the utmp file
    endutxent();

    // resize the buffer to the actual size
    buf = (char *)realloc(buf, offset + 1);
    if (!buf)
    {
        perror("Error reallocating memory");
        exit(EXIT_FAILURE);
    }

    // add the null terminator
    buf[offset] = '\0';

    // send the buffer to the pipe
    write(write_pipe, buf, offset + 1);

    // send size
    char str[100];
    sprintf(str, "%d", offset + 1);
    write(size_pipe, str, strlen(str) + 1);

    // free the allocated memory
    free(buf);
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

void getCpuUsage(int write_pipe, int tdelay)
{
    // This function takes the second interval (int tdelay), and compares two measurements that are tdelay seconds apart done by reading the /proc/stat file.
    // The function will write the overall percent increase(ex. 0.18%) or decrease(ex. -0.18%) as a float rounded to 10 decimal places to the write_pipe.
    // FORMULA FOR CALCULATION: (U2-U1/T2-T1) * 100 WHERE T IS TOTAL TIME AND U IS TOTAL TIME WITHOUT IDLE TIME
    // Example Output:
    // getCpuUsage(1)
    //
    // writes: 1.1656951904

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

    // build output string
    char buf[1024];

    // Convert the float to a string with a specific format
    snprintf(buf, sizeof(buf), "%.10f", usage);

    // write output to pipe
    write(write_pipe, buf, strlen(buf) + 1);
}

char *getCpuUsageGraphic(float current_usage, float previous_usage, int previous_bars)
{
    // This function takes the current cpu usage (float current_usage) and previous usage (float previous_usage) as well as
    // the number of bars on the previous usage (int previous_bars) and formats a graphic for the current cpu usage.
    // The function will the return the properly formatted string that includes the current cpu usage and the graphic as well as the number of bars.
    // NOTE: The graphic convetions include 8 bars (|) and with every 1% change there will be one | less or more.
    // Also for the first graphic there will always be 8 bars as the there is nothing to compare the usage difference
    // Example Output:
    // getCpuUsageGraphic(5, 3, 11)
    //
    // returns:  "13 ||||||||||||| 5"
    // note that the 13 means the number of bars and will also be sent through the pipe

    // calculate number of bars needed
    int count = previous_bars;

    // find difference in usage
    int difference = (int)(current_usage - previous_usage);

    // create string to pass
    char *buf = (char *)malloc((1024) * sizeof(char));

    if (previous_usage != 0)
    {
        // update count
        count += difference;

        sprintf(buf, "%d ", count);

        // add the bars
        for (int i = 0; i < count; i++)
        {
            strcat(buf, "|");
        }

        // add the current usage
        sprintf(buf + strlen(buf), " %f", current_usage);
    }
    else
    {
        count = 8;
        sprintf(buf, "%d ", count);

        // add the bars
        for (int i = 0; i < count; i++)
        {
            strcat(buf, "|");
        }

        // add the current usage
        sprintf(buf + strlen(buf), " %f", current_usage);
    }

    return buf;
}

void getMemoryUsage(int write_pipe)
{
    // This function writes the value of total and used Physical RAM as well as the total and used Virtual Ram to the write_pipe.
    // This is being done by using the <sys/sysinfo.h> C library.
    // Note that this function defines 1Gb = 1024Kb (i.e the function uses binary prefixes)
    // Example Output:
    // getMemoryUsage() writes
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

    // build output string
    char *buf = calloc(1, 50);
    sprintf(buf, "%.2f GB / %.2f GB  --  %.2f GB / %.2f GB\n", usedPhysicalRam, totalPhysicalRam, usedVirtualRam, totalVirtualRam);

    // write output to pipe
    write(write_pipe, buf, strlen(buf) + 1);

    // free allocated memory
    free(buf);
}

char *getMemoryUsageGraphic(float current_usage, float previous_usage)
{
    // This function takes the current memory usage (float current_usage) and previous usage (float previous_usage) and
    // formats a graphic for the current memory usage. The function will return the properly formatted string that includes
    // the current memory usage and the graphic.
    // NOTE: The graphic convetion # represents +0.01 and : represents -0.01 in difference between usage. Additionally o means no change.
    // Also the first output will always be o since there is nothing to compare to
    // Example Output:
    // getMemoryUsageGraphic(9.85, 9.76)
    //
    // prints:  "|######### 0.09 (9.85)"

    // calculate number of bars needed
    int count = 0;

    // find difference in usage
    float difference = current_usage - previous_usage;

    // update count
    count = (int)round(difference / 0.01);

    // create string to pass
    // allocate memory for the buffer
    char *buf = (char *)malloc(512 * sizeof(char));
    strcpy(buf, "|");

    if (previous_usage > 0)
    {
        if (count < 0)
        {
            // add the bars
            for (int i = 0; i < abs(count); i++)
            {
                strcat(buf, ":");
            }
        }
        else if (count > 0)
        {
            // add the bars
            for (int i = 0; i < count; i++)
            {
                strcat(buf, "#");
            }
        }
        else
        {
            strcat(buf, "o");
        }
        // add the current usage
        sprintf(buf + strlen(buf), " %.2f (%.2f)", difference, current_usage);
    }
    else if (previous_usage == 0)
    {
        strcat(buf, "o");
        // add the current usage
        sprintf(buf + strlen(buf), " 0.00 (%.2f)", current_usage);
    }

    return buf;
}

void allInfoUpdate(int samples, int tdelay)
{
    // This function will take in int samples and tdelay and prints out all the system information that will update
    // in the specified time interval and the specified number of samples. The information given includes memory usage,
    // user logs, cpu information, system information and are implemented through the above listed functions.
    // NOTE: Cpu Usage, Memory Usage, and Users are individual processes that communicate through pipes
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
    int mem_pipe[2], cpu_pipe[2], user_pipe[2], size_pipe[2];
    if (pipe(mem_pipe) < 0 || pipe(cpu_pipe) < 0 || pipe(user_pipe) < 0 || pipe(size_pipe) < 0)
    {
        perror("Error creating pipes");
        exit(EXIT_FAILURE);
    }

    /////////////////////////////////
    //          CHILD
    /////////////////////////////////

    // child process for memory usage
    pid_t mem_pid = fork();
    if (mem_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (mem_pid == 0)
    {
        close(mem_pipe[0]); // close unused read end
        for (int i = 0; i < samples; i++)
        {
            getMemoryUsage(mem_pipe[1]); // write to pipe
            sleep(tdelay);               // sleep for tdelay seconds
        }

        exit(0); // exit child process
    }

    // child process for cpu usage
    pid_t cpu_pid = fork();
    if (cpu_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (cpu_pid == 0)
    {
        close(cpu_pipe[0]); // close unused read end
        for (int i = 0; i < samples; i++)
        {
            getCpuUsage(cpu_pipe[1], tdelay); // write to pipe
        }

        exit(0); // exit child process
    }

    // child process for users
    pid_t user_pid = fork();
    if (user_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (user_pid == 0)
    {
        close(user_pipe[0]); // close unused read end
        for (int i = 0; i < samples; i++)
        {
            getUsers(user_pipe[1], size_pipe[1]); // write to pipe
            sleep(tdelay);                        // sleep for tdelay seconds
        }

        exit(0); // exit child process
    }

    /////////////////////////////////
    //          PARENT
    /////////////////////////////////

    // close unused write ends of pipes
    close(mem_pipe[1]);
    close(cpu_pipe[1]);
    close(user_pipe[1]);
    close(size_pipe[1]);

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

    // wait for all child processes to finish
    fd_set read_fds;

    int max_fd = mem_pipe[0];
    if (cpu_pipe[0] > max_fd)
    {
        max_fd = cpu_pipe[0];
    }
    if (user_pipe[0] > max_fd)
    {
        max_fd = user_pipe[0];
    }

    // print all information
    for (int i = 0; i < samples; i++)
    {

        // wait for all child processes to finish
        FD_ZERO(&read_fds);
        FD_SET(mem_pipe[0], &read_fds);
        FD_SET(cpu_pipe[0], &read_fds);
        FD_SET(user_pipe[0], &read_fds);
        select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        // read and print output
        if (FD_ISSET(mem_pipe[0], &read_fds))
        {
            printf("\033[%d;0H", (memoryLineNumber)); // move cursor to memory
            char buf[100];
            read(mem_pipe[0], buf, sizeof(buf)); // read memory usage from pipe
            printf("%s", buf);
        }

        printf("\033[%d;0H", (usersLineNumber)); // move cursor to users
        printf("---------------------------------------\n");
        printf("### Sessions/users ###\n");
        printf("\033[J"); // clears everything below the current line

        // Read and print the user data from the user_pipe
        char size[100];
        read(size_pipe[0], size, sizeof(size));
        int length = atoi(size);
        char buf[length];
        read(user_pipe[0], buf, sizeof(buf)); // read memory usage from pipe
        printf("%s", buf);

        printf("---------------------------------------\n");
        getCpuNumber();

        if (i > 0)
        {
            // print usage
            printf(" total cpu use = %.10f %%\n", usage);
        }

        // Read and print the CPU usage data from the cpu_pipe
        char buf2[1024];
        read(cpu_pipe[0], buf2, sizeof(buf2)); // read memory usage from pipe
        usage = atof(buf2);

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

    // wait for processes to finish so no orphan or zombie cases
    int status;
    waitpid(mem_pid, &status, 0);
    waitpid(cpu_pid, &status, 0);
    waitpid(user_pid, &status, 0);

    // print usage
    printf(" total cpu use = %.10f %%\n", usage);

    // print the ending system details
    printf("---------------------------------------\n");
    printf("### System Information ### \n");
    getSystemInfo();
    printf("---------------------------------------\n");
}

void allInfoUpdateGraphic(int samples, int tdelay)
{
    // This function will take in int samples and tdelay and prints out all the system information including graphics that will update
    // in the specified time interval and the specified number of samples. The information given includes memory usage,
    // user logs, cpu information, system information and are implemented through the above listed functions.
    // NOTE: Cpu Usage, Memory Usage, and Users are individual processes that communicate through pipes
    // Example Output:
    // allInfoUpdateGraphic(10, 1) prints
    //
    // Nbr of samples: 10 -- every 1 secs
    // Memory usage: 4052 kilobytes
    //---------------------------------------
    // ### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)
    // 9.75 GB / 15.37 GB  -- 9.75 GB / 16.33 GB   |o 0.00 (9.75)
    // 9.75 GB / 15.37 GB  -- 9.75 GB / 16.33 GB   |* 0.00 (9.75)
    // 9.75 GB / 15.37 GB  -- 9.75 GB / 16.33 GB   |* 0.00 (9.75)
    // 9.76 GB / 15.37 GB  -- 9.76 GB / 16.33 GB   |* 0.00 (9.76)
    // 9.85 GB / 15.37 GB  -- 9.85 GB / 16.33 GB   |#########* 0.09 (9.85)
    // 10.06 GB / 15.37 GB  -- 10.06 GB / 16.33 GB   |####################* 0.20 (10.06)
    // 10.13 GB / 15.37 GB  -- 10.13 GB / 16.33 GB   |#######* 0.07 (10.13)
    // 10.16 GB / 15.37 GB  -- 10.16 GB / 16.33 GB   |##* 0.03 (10.16)
    // 10.28 GB / 15.37 GB  -- 10.28 GB / 16.33 GB   |###########* 0.12 (10.28)
    // 10.38 GB / 15.37 GB  -- 10.38 GB / 16.33 GB   |##########* 0.11 (10.38)
    //---------------------------------------
    // ### Sessions/users ###
    // marcelo       pts/0 (138.51.12.217)
    // marcelo       pts/1 (tmux(277015).%0)
    // alberto        tty7 (:0)
    // marcelo       pts/2 (tmux(277015).%1)
    // marcelo       pts/5 (138.51.12.217)
    //---------------------------------------
    // Number of cores: 4
    // total cpu use = 15.57%
    //         ||| 0.25
    //         ||||||||| 6.93
    //         ||||||||||||||| 12.08
    //         |||||||||||||||| 13.83
    //         ||||||||| 6.41
    //         |||||||||||||||| 13.97
    //         |||||||||||||||||| 15.37
    //         ||||||||||||||||| 14.91
    //         ||||||||||||||||||| 16.34
    //         |||||||||||||||||| 15.57
    //---------------------------------------
    // ### System Information ###
    // System Name = Linux
    // Machine Name = iits-b473-01
    // Version = #99-Ubuntu SMP Thu Sep 23 17:29:00 UTC 2021
    // Release = 5.4.0-88-generic
    // Architecture = x86_64
    //---------------------------------------

    // create pipes for communication
    int mem_pipe[2], cpu_pipe[2], user_pipe[2], size_pipe[2];
    if (pipe(mem_pipe) < 0 || pipe(cpu_pipe) < 0 || pipe(user_pipe) < 0 || pipe(size_pipe) < 0)
    {
        perror("Error creating pipes");
        exit(EXIT_FAILURE);
    }

    /////////////////////////////////
    //          CHILD
    /////////////////////////////////

    // child process for memory usage
    pid_t mem_pid = fork();
    if (mem_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (mem_pid == 0)
    {
        close(mem_pipe[0]); // close unused read end
        for (int i = 0; i < samples; i++)
        {
            getMemoryUsage(mem_pipe[1]); // write to pipe
            sleep(tdelay);               // sleep for tdelay seconds
        }

        exit(0); // exit child process
    }

    // child process for cpu usage
    pid_t cpu_pid = fork();
    if (cpu_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (cpu_pid == 0)
    {
        close(cpu_pipe[0]); // close unused read end
        for (int i = 0; i < samples; i++)
        {
            getCpuUsage(cpu_pipe[1], tdelay); // write to pipe
        }

        exit(0); // exit child process
    }

    // child process for users
    pid_t user_pid = fork();
    if (user_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (user_pid == 0)
    {
        close(user_pipe[0]); // close unused read end
        for (int i = 0; i < samples; i++)
        {
            getUsers(user_pipe[1], size_pipe[1]); // write to pipe
            sleep(tdelay);                        // sleep for tdelay seconds
        }

        exit(0); // exit child process
    }

    /////////////////////////////////
    //          PARENT
    /////////////////////////////////

    // close unused write ends of pipes
    close(mem_pipe[1]);
    close(cpu_pipe[1]);
    close(user_pipe[1]);
    close(size_pipe[1]);

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

    // wait for all child processes to finish
    fd_set read_fds;

    int max_fd = mem_pipe[0];
    if (cpu_pipe[0] > max_fd)
    {
        max_fd = cpu_pipe[0];
    }
    if (user_pipe[0] > max_fd)
    {
        max_fd = user_pipe[0];
    }

    // store previous cpu and memory results
    float cpu_usage[samples][2];
    float memory_usage[samples];

    // print all information
    for (int i = 0; i < samples; i++)
    {
        // wait for all child processes to finish
        FD_ZERO(&read_fds);
        FD_SET(mem_pipe[0], &read_fds);
        FD_SET(cpu_pipe[0], &read_fds);
        FD_SET(user_pipe[0], &read_fds);
        select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        // read and print output
        if (FD_ISSET(mem_pipe[0], &read_fds))
        {
            printf("\033[%d;0H", (memoryLineNumber)); // move cursor to memory
            char buf[100];
            read(mem_pipe[0], buf, sizeof(buf)); // read memory usage from pipe

            // get total usage
            float dummy;
            float dummy2;
            float usage;
            float dummy3;
            sscanf(buf, "%f GB / %f GB  --  %f GB / %f GB\n", &dummy, &dummy2, &usage, &dummy3);

            // add to array
            memory_usage[i] = usage;

            char print[500];

            if (i == 0)
            {
                strcpy(print, getMemoryUsageGraphic(usage, 0));
            }
            else
            {
                strcpy(print, getMemoryUsageGraphic(usage, memory_usage[i - 1]));
            }

            printf("%s", print);
        }

        printf("\033[%d;0H", (usersLineNumber)); // move cursor to users
        printf("---------------------------------------\n");
        printf("### Sessions/users ###\n");
        printf("\033[J"); // clears everything below the current line

        // Read and print the user data from the user_pipe
        char size[100];
        read(size_pipe[0], size, sizeof(size));
        int length = atoi(size);
        char buf[length];
        read(user_pipe[0], buf, sizeof(buf)); // read memory usage from pipe
        printf("%s", buf);

        printf("---------------------------------------\n");
        getCpuNumber();

        if (i > 0)
        {
            // print usage
            printf(" total cpu use = %.10f %%\n", usage);

            for (int j = 0; j < i; j++)
            {
                char print[1024];

                if (j != 0)
                {
                    strcpy(print, getCpuUsageGraphic(cpu_usage[j][1], cpu_usage[j - 1][1], cpu_usage[j - 1][0]));
                }
                else
                {
                    strcpy(print, getCpuUsageGraphic(cpu_usage[j][1], 0, 0));
                }
                // Scan the first number and the number of characters read
                int chars_read;
                int num1;
                sscanf(print, "%d%n", &num1, &chars_read);

                // Move the remaining part of the string to the left, starting after the first number
                memmove(print, print + chars_read, strlen(print + chars_read) + 1);

                printf("%s\n", print);
            }
        }

        // Read and print the CPU usage data from the cpu_pipe
        char buf2[1024];
        read(cpu_pipe[0], buf2, sizeof(buf2)); // read memory usage from pipe
        usage = atof(buf2);
        char str[150];

        if (i == 0)
        {
            stpcpy(str, getCpuUsageGraphic(usage, 0, 0));
        }
        else
        {
            stpcpy(str, getCpuUsageGraphic(usage, cpu_usage[i - 1][1], cpu_usage[i - 1][0]));
        }

        int bars;
        float dummy;

        sscanf(str, "%d |%*[^|]|%*[^|]|%*[^|] %f", &bars, &dummy);

        // update cpu_usage array
        cpu_usage[i][0] = bars;
        cpu_usage[i][1] = usage;

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

    // wait for processes to finish so no orphan or zombie cases
    int status;
    waitpid(mem_pid, &status, 0);
    waitpid(cpu_pid, &status, 0);
    waitpid(user_pid, &status, 0);

    // print usage
    for (int j = 9; j <= 10; j++)
    {
        char print[1024];

        strcpy(print, getCpuUsageGraphic(cpu_usage[j][1], cpu_usage[j - 1][1], cpu_usage[j - 1][0]));

        // Scan the first number and the number of characters read
        int chars_read;
        int num1;
        sscanf(print, "%d%n", &num1, &chars_read);

        // Move the remaining part of the string to the left, starting after the first number
        memmove(print, print + chars_read, strlen(print + chars_read) + 1);

        printf("%s\n", print);
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
    // NOTE: Getting users is an individual processes that communicates through pipes
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

    int user_pipe[2], size_pipe[2];
    if (pipe(user_pipe) < 0 || pipe(size_pipe) < 0)
    {
        perror("Error creating pipes");
        exit(EXIT_FAILURE);
    }

    /////////////////////////////////
    //          CHILD
    /////////////////////////////////

    // child process for memory usage
    pid_t user_pid = fork();
    if (user_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (user_pid == 0)
    {
        close(user_pipe[0]); // close unused read end
        for (int i = 0; i < samples; i++)
        {
            getUsers(user_pipe[1], size_pipe[1]); // write to pipe
            sleep(tdelay);                        // sleep for tdelay seconds
        }

        exit(0); // exit child process
    }

    /////////////////////////////////
    //         PARENT
    /////////////////////////////////

    // close unused write ends of pipes
    close(user_pipe[1]);
    close(size_pipe[1]);

    // clear terminal before starting
    printf("\033c");

    fd_set read_fds;

    // print all user information
    for (int i = 0; i < samples; i++)
    {

        // wait for all child processes to finish
        FD_ZERO(&read_fds);
        FD_SET(user_pipe[0], &read_fds);
        select(user_pipe[0] + 1, &read_fds, NULL, NULL, NULL);

        header(samples, tdelay);
        printf("---------------------------------------\n");
        printf("### Sessions/users ###\n");

        // read and print output
        if (FD_ISSET(user_pipe[0], &read_fds))
        {

            // Read and print the user data from the user_pipe
            char size[100];
            read(size_pipe[0], size, sizeof(size));
            int length = atoi(size);
            char buf[length];
            read(user_pipe[0], buf, sizeof(buf)); // read memory usage from pipe
            printf("%s", buf);
        }

        printf("---------------------------------------\n");

        if (i != samples - 1)
        {
            // clear buffer
            fflush(stdout);
            // clear screen
            printf("\033c");
        }
    }

    // wait for processes to finish so no orphan or zombie cases
    int status;
    waitpid(user_pid, &status, 0);

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
    // NOTE: Cpu Usage and Memory Usage are individual processes that communicate through pipes
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

    int mem_pipe[2], cpu_pipe[2];
    if (pipe(mem_pipe) < 0 || pipe(cpu_pipe) < 0)
    {
        perror("Error creating pipes");
        exit(EXIT_FAILURE);
    }

    /////////////////////////////////
    //          CHILD
    /////////////////////////////////

    // child process for memory usage
    pid_t mem_pid = fork();
    if (mem_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (mem_pid == 0)
    {
        close(mem_pipe[0]); // close unused read end
        for (int i = 0; i < samples; i++)
        {
            getMemoryUsage(mem_pipe[1]); // write to pipe
            sleep(tdelay);               // sleep for tdelay seconds
        }

        exit(0); // exit child process
    }

    // child process for cpu usage
    pid_t cpu_pid = fork();
    if (cpu_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (cpu_pid == 0)
    {
        close(cpu_pipe[0]); // close unused read end
        for (int i = 0; i < samples; i++)
        {
            getCpuUsage(cpu_pipe[1], tdelay); // write to pipe
        }

        exit(0); // exit child process
    }

    /////////////////////////////////
    //          PARENT
    /////////////////////////////////

    // close unused write ends of pipes
    close(mem_pipe[1]);
    close(cpu_pipe[1]);

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

    fd_set read_fds;
    int max_fd = (mem_pipe[0] > cpu_pipe[0]) ? mem_pipe[0] : cpu_pipe[0];

    // print all system info
    for (int i = 0; i < samples; i++)
    {

        // wait for all child processes to finish
        FD_ZERO(&read_fds);
        FD_SET(mem_pipe[0], &read_fds);
        FD_SET(cpu_pipe[0], &read_fds);
        select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        // read and print output
        if (FD_ISSET(mem_pipe[0], &read_fds))
        {
            printf("\033[%d;0H", (memoryLineNumber)); // move cursor to memory
            char buf[100];
            read(mem_pipe[0], buf, sizeof(buf)); // read memory usage from pipe
            printf("%s", buf);
        }

        printf("\033[%d;0H", (cpuLineNumber)); // move cursor to cpu usage
        getCpuNumber();

        if (i > 0)
        {
            // print usage
            printf(" total cpu use = %.10f %%\n", usage);
        }

        // Read and print the CPU usage data from the cpu_pipe
        char buf2[1024];
        read(cpu_pipe[0], buf2, sizeof(buf2)); // read memory usage from pipe
        usage = atof(buf2);

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

    // wait for processes to finish so no orphan or zombie cases
    int status;
    waitpid(mem_pid, &status, 0);
    waitpid(cpu_pid, &status, 0);

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
    // NOTE: Cpu Usage, Memory Usage, and Users are individual processes that communicate through pipes
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

    // create pipes for communication
    int mem_pipe[2], cpu_pipe[2], user_pipe[2], size_pipe[2];
    if (pipe(mem_pipe) < 0 || pipe(cpu_pipe) < 0 || pipe(user_pipe) < 0 || pipe(size_pipe) < 0)
    {
        perror("Error creating pipes");
        exit(EXIT_FAILURE);
    }

    /////////////////////////////////
    //          CHILD
    /////////////////////////////////

    // child process for memory usage
    pid_t mem_pid = fork();
    if (mem_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (mem_pid == 0)
    {
        close(mem_pipe[0]); // close unused read end
        for (int i = 0; i < samples; i++)
        {
            getMemoryUsage(mem_pipe[1]); // write to pipe
            sleep(tdelay);               // sleep for tdelay seconds
        }

        exit(0); // exit child process
    }

    // child process for cpu usage
    pid_t cpu_pid = fork();
    if (cpu_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (cpu_pid == 0)
    {
        close(cpu_pipe[0]); // close unused read end
        for (int i = 0; i < samples; i++)
        {
            getCpuUsage(cpu_pipe[1], tdelay); // write to pipe
        }

        exit(0); // exit child process
    }

    // child process for users
    pid_t user_pid = fork();
    if (user_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (user_pid == 0)
    {
        close(user_pipe[0]); // close unused read end
        for (int i = 0; i < samples; i++)
        {
            getUsers(user_pipe[1], size_pipe[1]); // write to pipe
            sleep(tdelay);                        // sleep for tdelay seconds
        }

        exit(0); // exit child process
    }

    /////////////////////////////////
    //          PARENT
    /////////////////////////////////

    // close unused write ends of pipes
    close(mem_pipe[1]);
    close(cpu_pipe[1]);
    close(user_pipe[1]);
    close(size_pipe[1]);

    // clear terminal before starting
    printf("\033c");

    float usage;

    // wait for all child processes to finish
    fd_set read_fds;

    int max_fd = mem_pipe[0];
    if (cpu_pipe[0] > max_fd)
    {
        max_fd = cpu_pipe[0];
    }
    if (user_pipe[0] > max_fd)
    {
        max_fd = user_pipe[0];
    }

    // print all info sequentially
    for (int i = 0; i < samples; i++)
    {

        // wait for all child processes to finish
        FD_ZERO(&read_fds);
        FD_SET(mem_pipe[0], &read_fds);
        FD_SET(cpu_pipe[0], &read_fds);
        FD_SET(user_pipe[0], &read_fds);
        select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        printf("\r"); // clear current line in case CTRL Z has been called
        printf(">>> Iteration: %d\n", i + 1);
        header(samples, tdelay);
        printf("---------------------------------------\n");
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");

        // create the needed spaces
        for (int j = 0; j < samples; j++)
        {
            if (j == i)
            {
                // read and print output
                if (FD_ISSET(mem_pipe[0], &read_fds))
                {
                    char buf[100];
                    read(mem_pipe[0], buf, sizeof(buf)); // read memory usage from pipe
                    printf("%s", buf);
                }
            }
            else
            {
                printf("\n");
            }
        }
        printf("---------------------------------------\n");
        printf("### Sessions/users ###\n");
        // Read and print the user data from the user_pipe
        char size[100];
        read(size_pipe[0], size, sizeof(size));
        int length = atoi(size);
        char buf[length];
        read(user_pipe[0], buf, sizeof(buf)); // read memory usage from pipe
        printf("%s", buf);
        printf("---------------------------------------\n");
        getCpuNumber();

        // Read and print the CPU usage data from the cpu_pipe
        char buf2[1024];
        read(cpu_pipe[0], buf2, sizeof(buf2)); // read memory usage from pipe
        usage = atof(buf2);
        printf(" total cpu use = %.10f %%\n", usage);

        printf("\n");

        fflush(stdout);
    }

    // wait for processes to finish so no orphan or zombie cases
    int status;
    waitpid(mem_pid, &status, 0);
    waitpid(cpu_pid, &status, 0);
    waitpid(user_pid, &status, 0);

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
    // NOTE: Getting users is an individual process that communicates through pipes
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

    int user_pipe[2], size_pipe[2];
    if (pipe(user_pipe) < 0 || pipe(size_pipe) < 0)
    {
        perror("Error creating pipes");
        exit(EXIT_FAILURE);
    }

    /////////////////////////////////
    //          CHILD
    /////////////////////////////////

    // child process for memory usage
    pid_t user_pid = fork();
    if (user_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (user_pid == 0)
    {
        close(user_pipe[0]); // close unused read end
        for (int i = 0; i < samples; i++)
        {
            getUsers(user_pipe[1], size_pipe[1]); // write to pipe
            sleep(tdelay);                        // sleep for tdelay seconds
        }

        exit(0); // exit child process
    }

    /////////////////////////////////
    //          PARENT
    /////////////////////////////////

    // close unused write ends of pipes
    close(user_pipe[1]);
    close(size_pipe[1]);

    // clear terminal before starting
    printf("\033c");

    fd_set read_fds;

    // print user info sequentially
    for (int i = 0; i < samples; i++)
    {
        // wait for all child processes to finish
        FD_ZERO(&read_fds);
        FD_SET(user_pipe[0], &read_fds);
        select(user_pipe[0] + 1, &read_fds, NULL, NULL, NULL);

        printf("\r"); // clear current line in case CTRL Z has been called
        printf(">>>Iteration: %d\n", i + 1);
        header(samples, tdelay);
        printf("---------------------------------------\n");
        printf("### Sessions/users ### \n");
        // read and print output
        if (FD_ISSET(user_pipe[0], &read_fds))
        {

            // Read and print the user data from the user_pipe
            char size[100];
            read(size_pipe[0], size, sizeof(size));
            int length = atoi(size);
            char buf[length];
            read(user_pipe[0], buf, sizeof(buf)); // read memory usage from pipe
            printf("%s", buf);
        }
        printf("\n");

        if (i != samples - 1)
        {
            // wait tdelay
            sleep(tdelay);
            // clear buffer
            fflush(stdout);
        }
    }

    // wait for processes to finish so no orphan or zombie cases
    int status;
    waitpid(user_pid, &status, 0);

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
    // NOTE: Cpu Usage and Memory Usage are individual processes that communicate through pipes
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

    int mem_pipe[2], cpu_pipe[2];
    if (pipe(mem_pipe) < 0 || pipe(cpu_pipe) < 0)
    {
        perror("Error creating pipes");
        exit(EXIT_FAILURE);
    }

    /////////////////////////////////
    //          CHILD
    /////////////////////////////////

    // child process for memory usage
    pid_t mem_pid = fork();
    if (mem_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (mem_pid == 0)
    {
        close(mem_pipe[0]); // close unused read end
        for (int i = 0; i < samples; i++)
        {
            getMemoryUsage(mem_pipe[1]); // write to pipe
            sleep(tdelay);               // sleep for tdelay seconds
        }

        exit(0); // exit child process
    }

    // child process for cpu usage
    pid_t cpu_pid = fork();
    if (cpu_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (cpu_pid == 0)
    {
        close(cpu_pipe[0]); // close unused read end
        for (int i = 0; i < samples; i++)
        {
            getCpuUsage(cpu_pipe[1], tdelay); // write to pipe
        }

        exit(0); // exit child process
    }

    /////////////////////////////////
    //          PARENT
    /////////////////////////////////

    // close unused write ends of pipes
    close(mem_pipe[1]);
    close(cpu_pipe[1]);

    // clear terminal before starting
    printf("\033c");

    float usage;

    fd_set read_fds;
    int max_fd = (mem_pipe[0] > cpu_pipe[0]) ? mem_pipe[0] : cpu_pipe[0];

    // print system info sequentially
    for (int i = 0; i < samples; i++)
    {

        // wait for all child processes to finish
        FD_ZERO(&read_fds);
        FD_SET(mem_pipe[0], &read_fds);
        FD_SET(cpu_pipe[0], &read_fds);
        select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        printf("\r"); // clear current line in case CTRL Z has been called
        printf(">>> Iteration: %d\n", i + 1);
        header(samples, tdelay);
        printf("---------------------------------------\n");
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");

        // create the needed spaces
        for (int j = 0; j < samples; j++)
        {
            if (j == i)
            {
                // read and print output
                if (FD_ISSET(mem_pipe[0], &read_fds))
                {
                    char buf[100];
                    read(mem_pipe[0], buf, sizeof(buf)); // read memory usage from pipe
                    printf("%s", buf);
                }
            }
            else
            {
                printf("\n");
            }
        }
        printf("---------------------------------------\n");
        getCpuNumber();

        // Read and print the CPU usage data from the cpu_pipe
        char buf2[1024];
        read(cpu_pipe[0], buf2, sizeof(buf2)); // read memory usage from pipe
        usage = atof(buf2);
        printf(" total cpu use = %.10f %%\n", usage);

        printf("\n");

        // clear buffer
        fflush(stdout);
    }

    // wait for processes to finish so no orphan or zombie cases
    int status;
    waitpid(mem_pid, &status, 0);
    waitpid(cpu_pid, &status, 0);

    // print the ending system details
    printf("\033[1A");
    printf("---------------------------------------\n");
    printf("### System Information ### \n");
    getSystemInfo();
    printf("---------------------------------------\n");
}