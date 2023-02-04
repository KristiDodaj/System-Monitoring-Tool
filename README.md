# System-Monitoring-Tool

NOTE: The actual function descriptions are provided within the source code.

GENERAL OVERVIEW:

This project was approached in a very modular structure where smaller functions serve as building blocks for bigger ones.
I was able to break things down into two classes of functions, low-level and high-level. Below is a list of each category
and how the pieces interconnect.

LOW-LEVEL FUNCTIONS:
1. header(int samples, int tdelay)
2. getSystemInfo()
3. getUsers()
4. getCpuNumber()
5. getCpuUsage(long int previousMeasure)
6. getMemoryUsage()
7. parseArguments(int argc, char *argv[], bool *system, bool *user, bool *sequential, int *samples, int *tdelay)
8. validateArguments(int argc, char *argv[])

Notice that all these functions are responsible for getting the information and each has a singular responsibility.

HIGH-LEVEL FUNCTIONS:
1. allInfoUpdate(int samples, int tdelay)
2. usersUpdate(int samples, int tdelay)
3. systemUpdate(int samples, int tdelay)
4. allInfoSequential(int samples, int tdelay)
5. usersSequential(int samples, int tdelay)
6. systemSequential(int samples, int tdelay)
7. navigate(int argc, char *argv[])

Notice the first 6 functions are split into two versions (update and sequential). Every 3 functions for both sequential and updating cases handle the different flags (everything, --system, --user). Lastly, the seventh function (navigate) serves as a method to navigate to each one of these 6 versions depending on the given command line arguments. 

FOR FURTHER INFORMATION ON EACH FUNCTION'S ROLE AND COMPOSITION PLEASE REFER TO THE SOURCE CODE FILE.
