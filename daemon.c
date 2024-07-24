/*
I compiled the code using:
/home/issiac/Downloads/buildroot-2024.05/output/host/usr/bin/arm-linux-gcc --sysroot=/home/issiac/Downloads/buildroot-2024.05/output/staging -c daemon.c -o daemon.o
/home/issiac/Downloads/buildroot-2024.05/output/host/usr/bin/arm-linux-gcc --sysroot=/home/issiac/Downloads/buildroot-2024.05/output/staging -o daemon daemon.o -uClibc -lc
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <syslog.h>

void create_daemon() {
    pid_t pid;

    // Fork the parent process
    pid = fork();

    // If fork failed, exit
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    // If we got a good PID, exit the parent process
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Change the file mode mask
    umask(0);

    // Create a new SID for the child process
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    // Change the current working directory
    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }

    // Close out the standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void write_log(const char *message) {
    FILE *logfile;
    logfile = fopen("/tmp/chronos.log", "a");
    if (logfile == NULL) {
        exit(EXIT_FAILURE);
    }
    fprintf(logfile, "%s\n", message);
    fclose(logfile);
}

int main() {
    create_daemon();

    // Open a connection to the syslog
    openlog("chronos", LOG_PID, LOG_DAEMON);

    while (1) {
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        timestamp[strlen(timestamp) - 1] = '\0'; // Remove newline character

        // Write timestamp to log file
        write_log(timestamp);

        // Log to syslog
        syslog(LOG_NOTICE, "Daemon is running: %s", timestamp);

        // Sleep for 10 seconds
        sleep(1);
    }

    closelog();
    return EXIT_SUCCESS;
}

