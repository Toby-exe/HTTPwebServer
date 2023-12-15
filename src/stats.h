#ifndef STATS_H
#define STATS_H

#include <stdio.h>      // for FILE, perror, printf, snprintf, fgets
#include <string.h>     // for strncmp
#include <sys/time.h>   // for struct timeval, gettimeofday
#include <sys/resource.h> // for struct rusage, getrusage, RUSAGE_SELF
#include <unistd.h>     // for sysconf, _SC_NPROCESSORS_ONLN

#define BUF_SIZE 1024

long get_memory_usage();
double calculate_cpu_usage();
// void calculate_usage(struct timeval start, struct timeval wall_start);

#endif 