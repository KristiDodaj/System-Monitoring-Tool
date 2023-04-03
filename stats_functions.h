// Author: Kristi Dodaj
// stats_functions.h: Responsible for defining the function definitions that are within the stats_functions.c file

#ifndef STATS
#define STATS

// define the function signatures

void header(int samples, int tdelay);
void getSystemInfo();
void getUsers(int write_pipe, int size_pipe);
void getCpuNumber();
void getCpuUsage(int write_pipe, int tdelay);
void getCpuUsageGraphic(float current_usage, float previous_usage, int previous_bars);
void getMemoryUsage(int write_pipe);
void getMemoryUsageGraphic(float current_usage, float previous_usage);
void allInfoUpdate(int samples, int tdelay);
void usersUpdate(int samples, int tdelay);
void systemUpdate(int samples, int tdelay);
void allInfoSequential(int samples, int tdelay);
void usersSequential(int samples, int tdelay);
void systemSequential(int samples, int tdelay);

#endif /* STATS */