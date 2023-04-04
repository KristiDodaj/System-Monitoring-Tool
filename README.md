# System-Monitoring-Tool 📈

NOTE: The actual function descriptions with further details on their design are provided within the source code.

## GENERAL OVERVIEW:

This project was approached in a very modular structure where smaller functions serve as building blocks for bigger ones.
I was able to break things down into two classes of functions, low-level and high-level.

## FILES BREAKDOWN:

1. stats_functions.c: contains all the functions responsible for retrieving as well as displaying the info
   <br /> • header(int samples, int tdelay);
   <br />• getSystemInfo();
   <br />• getUsers(int write_pipe, int size_pipe);
   <br />• getCpuNumber();
   <br />• getCpuUsage(int write_pipe, int tdelay);
   <br />• getCpuUsageGraphic(float current_usage, float previous_usage, int previous_bars);
   <br />• getMemoryUsage(int write_pipe);
   <br />• getMemoryUsageGraphic(float current_usage, float previous_usage);
   <br />• allInfoUpdate(int samples, int tdelay);
   <br />• allInfoUpdateGraphic(int samples, int tdelay);
   <br />• usersUpdate(int samples, int tdelay);
   <br />• systemUpdate(int samples, int tdelay);
   <br />• systemUpdateGraphic(int samples, int tdelay);
   <br />• allInfoSequential(int samples, int tdelay);
   <br />• allInfoSequentialGraphic(int samples, int tdelay);
   <br />• usersSequential(int samples, int tdelay);
   <br />• systemSequential(int samples, int tdelay);
   <br />• handle_ctrl_c(int signal_number);
   <br />• systemSequentialGraphic(int samples, int tdelay);

2. main.c: contains all the functions responsible for parsing/validating the CLAs as well as navigate to the right output.
   <br />• parseArguments(int argc, char *argv[], bool *system, bool *user, bool *sequential, int *samples, int *tdelay);
   <br />• validateArguments(int argc, char \*argv[]);
   <br />• navigate(int argc, char \*argv[]);

3. stats_functions.h: header file containing all the function signatures of stats_functions.c so it can be linked to main.c

## LOW-LEVEL FUNCTIONS:

1. header(int samples, int tdelay) //prints header info
2. getSystemInfo() //prints system info
3. getUsers() //writes user info to the write pipe
4. getCpuNumber() //prints cpu and core numbers
5. getCpuUsage(long int previousMeasure) //writes to the pipe the cpu usage based on two different measurements taken at an interval of tdelay seconds
6. getCpuUsageGraphic(float current_usage, float previous_usage, int previous_bars) //returns the graphical string version of the given cpu usage
7. getMemoryUsage() //writes memory info to the write pipe
8. getMemoryUsageGraphic(float current_usage, float previous_usage) //returns the graphical string version of the given memory usage
9. parseArguments(int argc, char *argv[], bool *system, bool *user, bool *sequential, int *samples, int *tdelay) //parses command line arguments passed
10. validateArguments(int argc, char \*argv[]) //validates the command line arguments passed
11. handle_ctrl_c(int signal_number) //handles the case when CTRL C is pressed

Notice that all these functions are responsible for getting the information and each has a singular responsibility.

## HIGH-LEVEL FUNCTIONS:

1. allInfoUpdate(int samples, int tdelay) //prints all info by updating itself
2. allInfoUpdateGraphic(int samples, int tdelay) //prints all graphical info by updating itself
3. usersUpdate(int samples, int tdelay) //prints user info by updating itself
4. systemUpdate(int samples, int tdelay) //print system info by updating itself
5. systemUpdateGraphic(int samples, int tdelay) //print system info graphically by updating itself
6. allInfoSequential(int samples, int tdelay) //prints all info sequentially
7. allInfoSequentialGraphic(int samples, int tdelay) //prints all graphic info sequentially
8. usersSequential(int samples, int tdelay) //prints user info sequentially
9. systemSequential(int samples, int tdelay) //print system info sequentially
10. systemSequentialGraphic(int samples, int tdelay) //print all graphical system info sequentially
11. navigate(int argc, char \*argv[]) //navigates to needed output given the command line arguments

Notice the first 10 functions are split into two versions (update and sequential). Every 5 functions for both sequential and updating cases handle the different flags (everything, --system, --user) as well as graphical or not graphical. Lastly, the 11th function (navigate) serves as a method to navigate to each one of these 10 versions depending on the given command line arguments.

## CONCURRENCY

Each of the beginning 10 high-level functions stated above utilize forking to create separate processes that handle the retrieving of users, CPU usage, and memory usage. When any or all of these three processes are activated, the above-stated functions also set up pipelines to communicate between the main and forked processes. The forked processes use lower-level functions (getUsers, getCpuUsage, getMemoryUsage) to retrieve the needed info and write back to the main process with the wanted information through pipes.

FORE MORE INFO ON HOW THIS IS IMPLEMENTED REFER TO THE stats_functions.c FILE

## SIGNALS & ERROR CHECKING

1. The program will ignore the users CTRL-Z input and ......

2. The code has been fully error-checked using perror statements that report to STDERR. This means that the program will report if there was any failure in retrieving or accessing wanted information from the system. (see the codebase for further details)

## NOTES

1. When displaying the CPU usage in the first iteration, the program will take tdelay seconds for the first cpu result as it takes tdelay seconds to make the measurement.

2. The convetion for graphics is as follows:
   <br />• For CPU usage, the first iteration will start with 8 bars (|) and will lose or gain a bar for each 1% decrease or increase relative to the next iteration
   <br />• For memory usage, '#' represents +0.01 and ':' represents -0.01 in difference between usage. Additionally 'o' means no change. Note that the first iteration will be 'o' as there is nothing to compare to.

FOR FURTHER INFORMATION ON EACH FUNCTION'S ROLE/DESCRIPTION AS WELL AS ASSUMPTIONS PLEASE REFER TO THE SOURCE CODE FILES.

## HOW TO RUN

The program also provides a makefile titles 'makefile' which provides the necessary rules to compile the program.

RUN THE FOLLOWING COMMANDS IN SEQUENCE:
<br /> `make`
<br /> `./monitor [insert flags or positional args here]`

Note: You can run "make clean" to erase all the .o files produced from the compilation process

THE ARGUMENT OPTIONS INCLUDE:

1. --system (prints system info)
2. --user (prints user info)
3. --tdeleay=N (sets second interval between outputs)
4. --samples=N (sets number of sample outputs)
5. --sequential (prints samples sequentially)
6. --graphics (prints the graphical version)
7. You can also set tdelay and samples by simply inputing two seperate integers as your first two arguments (ex ./monitor 10 1)

NOTE: Calling the program with no arguments will deafult to samples=10, tdelay=1, and prints both system and user info by updating itself.
