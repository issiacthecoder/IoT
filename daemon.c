#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// Define constants for return codes and daemon name
#define OK 0
#define ERR_FORK 1
#define ERR_SETSID 2
#define ERR_CHDIR 3
#define DAEMON_NAME "sampled"
#define ERROR_FORMAT "Error: %s"

static void _signal_handler(int signal) {
    switch(signal) {
        case SIGHUP:
            // Handle SIGHUP if needed
            break;

        case SIGTERM:
            syslog(LOG_INFO, "Received SIGTERM, exiting.");
            closelog();
            exit(OK);

        default:
            syslog(LOG_INFO, "Received unhandled signal: %d", signal);
            break;
    }
}

static void _do_work(void) {
    int i = 0;
    while (1) {
        syslog(LOG_INFO, "Iteration: %d", i++);
        sleep(1);
    }
}

int main(void) {
    openlog(DAEMON_NAME, LOG_PID | LOG_NDELAY | LOG_NOWAIT, LOG_DAEMON);
    syslog(LOG_INFO, "Starting daemon");

    pid_t pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, ERROR_FORMAT, strerror(errno));
        return ERR_FORK;
    }

    if (pid > 0) {
        // Parent process exits
        return OK;
    }

    // Child process continues
    if (setsid() < 0) {
        syslog(LOG_ERR, ERROR_FORMAT, strerror(errno));
        return ERR_SETSID;
    }

    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Set umask
    umask(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    // Change working directory
    if (chdir("/") < 0) {
        syslog(LOG_ERR, ERROR_FORMAT, strerror(errno));
        return ERR_CHDIR;
    }

    // Set up signal handlers
    signal(SIGTERM, _signal_handler);
    signal(SIGHUP, _signal_handler);

    // Do the daemon's work
    _do_work();

    return 0; // This line is never reached
}

