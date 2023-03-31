// BY: KRISTI DODAJ
// COURSE: CSCB09

#ifndef STATS
#define STATS

// define the function signatures

void header(int samples, int tdelay);
void getSystemInfo();
void getUsers(int write_pipe);
void getCpuNumber();
float getCpuUsage(int tdelay);
void getMemoryUsage();
void allInfoUpdate(int samples, int tdelay);
void usersUpdate(int samples, int tdelay);
void systemUpdate(int samples, int tdelay);
void allInfoSequential(int samples, int tdelay);
void usersSequential(int samples, int tdelay);
void systemSequential(int samples, int tdelay);

#endif /* STATS */