#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <utmpx.h>
#include <unistd.h>

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
        printf("%s", "Unable to retrieve system information!");
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
    // Number of CPU's: 12       Number of Cores: 6

    int cpuNumber = 0;
    int coreNumber = 0;
    char line[256];
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
    printf("Number of CPU's: %d     Number of Cores: %d\n", cpuNumber, coreNumber);
    fclose(info);
}

int main()
{
    getCpuNumber();
    return 0;
}
