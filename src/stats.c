#include "stats.h"

/**
 * @brief Retrieves the memory usage of the process
 *
 * This function opens the /proc/self/status file and reads its contents to find the resident set size (VmRSS), 
 * which is the portion of the process's memory that is held in RAM. It then returns this value.
 * 
 * @param[in] None
 * @param[out] None
 * @return The memory usage of the process in kilobytes (kB). If the file fails to open, it returns -1.
 */
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

/**
 * @brief Calculates the CPU usage
 *
 * This function calculates the CPU usage by first getting the number of CPU cores and the start time. 
 * It then measures the CPU time and wall time, and finally calculates the CPU usage.
 * 
 * @param[in] None
 * @param[out] None
 * @return The CPU usage as a double
 */
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
