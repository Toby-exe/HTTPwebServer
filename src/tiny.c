#include "server.h"

void print_logo()
{
    printf("\033[1;31m"); // Set the text color to red
    printf("  ______ _                             \n");
    printf(" /_  __/(_)____   __  __               \n");
    printf("  / /  / // __ \\ / / / /               \n");
    printf(" / /  / // / / // /_/ /                \n");
    printf("/_/  /_//_/ /_/ \\__, /                 \n");
    printf("               /____/                  \n");

    printf("\033[1;33m"); // Set the text color to bright orange
    printf("   _____                                \n");
    printf("  / ___/ ___   _____ _   __ ___   _____\n");
    printf("  \\__ \\ / _ \\ / ___/| | / // _ \\ / ___/\n");
    printf(" ___/ //  __// /    | |/ //  __// /    \n");
    printf("/____/ \\___//_/     |___/ \\___//_/     \n");
    printf("\033[0m"); // Reset the text color

    return;
}
int main(int argc, char *argv[])
{
    print_logo();

    Http_server server;
    start_server(&server, argc, argv);

    create_mime_db();
    ThreadPool *pool = thread_pool_create(8);
    int connection_count = 0;

    while (1)
    {
        struct rusage usage;
        struct timeval start, wall_start;

        // Get the start time
        getrusage(RUSAGE_SELF, &usage);
        start = usage.ru_utime;
        gettimeofday(&wall_start, NULL); // Get the wall start time

        accept_client(&server, pool, &connection_count);
        calculate_usage(start, wall_start);
    }

    destroy_mime_db();
    thread_pool_wait(pool);
    thread_pool_destroy(pool);
    close(server.sockfd);

    return 0;
}