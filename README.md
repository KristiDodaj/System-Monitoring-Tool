# System-Monitoring-Tool

NOTE: The actual function descriptions are provided within the source code.

## GENERAL OVERVIEW:

This project was approached in a very modular structure where smaller functions serve as building blocks for bigger ones.
I was able to break things down into two classes of functions, low-level and high-level. Below is a list of each category
and how the pieces interconnect.

## LOW-LEVEL FUNCTIONS:
1. header(int samples, int tdelay) //prints header info
2. getSystemInfo() //prints system info
3. getUsers() //prints user info
4. getCpuNumber() //prints cpu and core numbers
5. getCpuUsage(long int previousMeasure) //prints cpu usage based on two different measurements taken at an interval of tdelay seconds
6. getMemoryUsage() //prints memory info
7. parseArguments(int argc, char *argv[], bool *system, bool *user, bool *sequential, int *samples, int *tdelay) //parses command line arguments passed
8. validateArguments(int argc, char *argv[]) //validates the command line arguments passed

Notice that all these functions are responsible for getting the information and each has a singular responsibility.

## HIGH-LEVEL FUNCTIONS:
1. allInfoUpdate(int samples, int tdelay) //prints all info by updating itself
2. usersUpdate(int samples, int tdelay) //prints user info by updating itself
3. systemUpdate(int samples, int tdelay) //print system info by updating itself
4. allInfoSequential(int samples, int tdelay) //prints all info sequentially
5. usersSequential(int samples, int tdelay) //prints user info sequentially
6. systemSequential(int samples, int tdelay) //print system info sequentially
7. navigate(int argc, char *argv[]) //navigates to needed output given the command line arguments

Notice the first 6 functions are split into two versions (update and sequential). Every 3 functions for both sequential and updating cases handle the different flags (everything, --system, --user). Lastly, the seventh function (navigate) serves as a method to navigate to each one of these 6 versions depending on the given command line arguments. 

## NOTES
When displaying the CPU usage in the first iteration, the program will take two measurements back to back with no time being waited. This choice is made because if tdelay was set to 10, we would have to wait 10 seconds to see the first output for the CPU usage. This is obviously quite                inefficient, thus the first output will not abide by the tdelay interval to carry the calculations but the rest will.

FOR FURTHER INFORMATION ON EACH FUNCTION'S ROLE AND DESCRIPTION PLEASE REFER TO THE SOURCE CODE FILE.

## HOW TO RUN

RUN THE FOLLOWING COMMANDS IN SEQUENCE:
<br /> ``` gcc -o mySystemStats mySystemStats.C ```
<br /> ``` ./mySystemStats [insert flags or positional args here] ```

