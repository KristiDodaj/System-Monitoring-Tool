#include <stdlib.h>
#include <stdio.h>
#include <sys/utsname.h>

// This function will print out the System Information using the <sys/utsname.h> C library
void getSystemInfo()
{

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

int main()
{
    getSystemInfo();
    return 0;
}
