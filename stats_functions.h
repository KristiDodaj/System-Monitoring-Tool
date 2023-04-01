// BY: KRISTI DODAJ
// COURSE: CSCB09

#ifndef STATS
#define STATS

// define the function signatures

void header(int samples, int tdelay);
void getSystemInfo();
void getUsers(int write_pipe);
void getCpuNumber();
void getCpuUsage(int write_pipe, int tdelay);
void getMemoryUsage(int write_pipe);
void allInfoUpdate(int samples, int tdelay);
void usersUpdate(int samples, int tdelay);
void systemUpdate(int samples, int tdelay);
void allInfoSequential(int samples, int tdelay);
void usersSequential(int samples, int tdelay);
void systemSequential(int samples, int tdelay);

#endif /* STATS */