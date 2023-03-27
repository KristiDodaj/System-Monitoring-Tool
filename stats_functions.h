// BY: KRISTI DODAJ
// COURSE: CSCB09

#ifndef STATS
#define STATS

// define the function signatures

void header(int samples, int tdelay);
void getSystemInfo();
void getUsers();
void getCpuNumber();
long int getCpuUsage(long int previousMeasure);
void getMemoryUsage();
void allInfoUpdate(int samples, int tdelay);
void usersUpdate(int samples, int tdelay);
void systemUpdate(int samples, int tdelay);
void allInfoSequential(int samples, int tdelay);
void usersSequential(int samples, int tdelay);
void systemSequential(int samples, int tdelay);

#endif /* STATS */