#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

pid_t monitor_pid = 0;

void write_command_to_file(const char *command) {
    int fd = open("command.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Failed to write command to file");
        return;
    }
    write(fd, command, strlen(command));
    close(fd);
}

void sigchld_handler(int sig) {
    (void)sig;
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);
    if (pid == monitor_pid) {
        monitor_pid = 0;
        printf("Monitor has stopped.\n");
    }
}

void stop_monitor() {
    if (monitor_pid == 0) {
        printf("Monitor is not running.\n");
        return;
    }

    write_command_to_file("stop_monitor");
    kill(monitor_pid, SIGUSR1);
    waitpid(monitor_pid, NULL, 0);
    monitor_pid = 0;
    printf("Monitor has been stopped.\n");
}

void start_monitor() {
    if (monitor_pid != 0) {
        printf("Monitor is already running (PID: %d).\n", monitor_pid);
        return;
    }

    monitor_pid = fork();
    if (monitor_pid == 0) {
        execl("./monitor", "monitor", NULL);
        perror("Failed to launch monitor");
        exit(1);
    } else {
        printf("Monitor has been started(PID: %d).\n", monitor_pid);
    }
}

void send_command(const char *command) {
    if (monitor_pid == 0) {
        printf("Monitor is not running. Please start it first.\n");
        return;
    }
    write_command_to_file(command);
    kill(monitor_pid, SIGUSR1);
    printf("Command sent: %s\n", command);
}

int main() {
    struct sigaction sa_chld;
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa_chld, NULL);

    char command[256];
    printf("Introdu o comanda: \n");

    while (1) {
        printf("> ");
        if (fgets(command, sizeof(command), stdin) != NULL) {
            command[strcspn(command, "\n")] = '\0';

            if (strcmp(command, "start_monitor") == 0) {
                start_monitor();
            } else if (strcmp(command, "stop_monitor") == 0) {
                stop_monitor();
            } else if (strcmp(command, "list_hunts") == 0) {
                send_command("list_hunts");
            } else if (strncmp(command, "list_treasures", 14) == 0) {
                send_command(command);
            } else if (strncmp(command, "view_treasure", 13) == 0) {
                send_command(command);
            } else if (strcmp(command, "exit") == 0) {
                if (monitor_pid != 0) {
                    printf("Please stop the monitor before exiting..\n");
                } else {
                    printf("Exiting program.\n");
                    break;
                }
            } else {
                printf("Unknown command: %s\n", command);
            }
        }
    }
    return 0;
}
