#include "stats.h"

long get_memory_usage()
{
    FILE *fp;
    char buf[BUF_SIZE];
    long mem_usage;

    /* Open the /proc/self/status file. */
    fp = fopen("/proc/self/status", "r");
    if (fp == NULL)
    {
        perror("Failed to open /proc/self/status");
        return -1;
    }

    /* Read the entire contents. */
    while (fgets(buf, BUF_SIZE, fp) != NULL)
    {
        /* The VmRSS value contains the resident set size, i.e., the portion of the process's memory that is held in RAM. */
        if (strncmp(buf, "VmRSS:", 6) == 0)
        {
            sscanf(buf, "%*s %ld", &mem_usage);
            break;
        }
    }

    fclose(fp);

    return mem_usage; /* kB */
}


// https://stackoverflow.com/questions/8501706/how-to-get-the-cpu-usage-in-c - this helped with figuring out how to get the CPU usage
double calculate_cpu_usage()
{
    struct rusage usage;
    struct timeval start, end;
    long sec, usec;
    double cpu_time, wall_time;

    // Get the number of CPU cores
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

    // Get the start time
    getrusage(RUSAGE_SELF, &usage);
    start = usage.ru_utime;

    // TODO: Insert the code you want to measure here

    // Get the end time
    getrusage(RUSAGE_SELF, &usage);
    end = usage.ru_utime;

    // Calculate the CPU time (in seconds)
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    cpu_time = sec + usec / 1e6;

    // Calculate the wall time (in seconds)
    // TODO: Replace this with the actual wall time
    wall_time = 1.0;

    // Calculate the CPU usage
    double cpu_usage = cpu_time / num_cores / wall_time * 100;

    return cpu_usage;
}

void calculate_usage(struct timeval start, struct timeval wall_start)
{
    struct rusage usage;
    struct timeval end, wall_end;
    long sec, usec;
    double cpu_time, wall_time;

    // Get the number of CPU cores
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

    // Get the end time
    getrusage(RUSAGE_SELF, &usage);
    end = usage.ru_utime;
    gettimeofday(&wall_end, NULL); // Get the wall end time

    // Calculate the CPU time (in seconds)
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    cpu_time = sec + usec / 1e6;

    // Calculate the wall time (in seconds)
    wall_time = (wall_end.tv_sec - wall_start.tv_sec) + (wall_end.tv_usec - wall_start.tv_usec) / 1e6;

    // Calculate the CPU usage
    double cpu_usage = cpu_time / num_cores / wall_time * 100;
    long mem_usage = get_memory_usage();
    printf("CPU Usage: %.2lf%%\n", cpu_usage);
    printf("Memory Usage: %ld kB\n", mem_usage);
}