#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_COMMAND_LENGTH 256

sig_atomic_t running = 1;
char received_command[MAX_COMMAND_LENGTH];
char hunt_id[MAX_COMMAND_LENGTH];
int treasure_id;

void read_command_from_file(char *buffer, size_t size) {
    int fd = open("command.txt", O_RDONLY);
    if (fd == -1) {
        perror("Failed to open command.txt");
        return;
    }
    ssize_t len = read(fd, buffer, size - 1);
    if (len > 0) {
        buffer[len] = '\0';
    }
    close(fd);
}

void sigusr1_handler(int sig) {
    (void)sig;

    read_command_from_file(received_command, MAX_COMMAND_LENGTH);
    printf("Received command: %s\n", received_command);

    char *command = strtok(received_command, " ");
    if (!command) return;

    if (strcmp(command, "list_hunts") == 0) {
        printf("Listing hunts:\n");
        DIR *dir = opendir(".");
        if (dir) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
                    printf("  - %s\n", entry->d_name);
                }
            }
            closedir(dir);
        } else {
            perror("Failed to open directory");
        }
    } else if (strcmp(command, "list_treasures") == 0) {
        char *hunt = strtok(NULL, " ");
        if (!hunt) {
            printf("Hunt ID missing.\n");
            return;
        }
        if (fork() == 0) {
            execl("./treasure_manager", "treasure_manager", "list", hunt, NULL);
            perror("Exec list error");
            exit(1);
        } else {
            wait(NULL);
        }
    } else if (strcmp(command, "view_treasure") == 0) {
        char *hunt = strtok(NULL, " ");
        char *treasure = strtok(NULL, " ");
        if (!hunt || !treasure) {
            printf("Hunt or Treasure ID missing.\n");
            return;
        }
        if (fork() == 0) {
            execl("./treasure_manager", "treasure_manager", "view", hunt, treasure, NULL);
            perror("Exec view error");
            exit(1);
        } else {
            wait(NULL);
        }
    } else if (strcmp(command, "stop_monitor") == 0) {
        usleep(1000000);
        running = 0;
    } else {
        printf("Unknown command: %s\n", command);
    }
}

void sigterm_handler(int sig) {
    (void)sig;
    printf("SIGTERM received. Exiting..\n");
    running = 0;
}

int main() {
    struct sigaction sa_usr1;
    sa_usr1.sa_handler = sigusr1_handler;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa_usr1, NULL);

    struct sigaction sa_term;
    sa_term.sa_handler = sigterm_handler;
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = 0;
    sigaction(SIGTERM, &sa_term, NULL);
    while (running) {
        pause();
    }

    printf("Monitor stopped.\n");
    return 0;
}
