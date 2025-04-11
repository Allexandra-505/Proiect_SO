#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

#define MAX_PATH 256
#define MAX_CLUE 1024
#define MAX_USERNAME 64
#define MAX_LOG_ENTRY 2048

typedef struct {
    int id;
    char username[MAX_USERNAME];
    double latitude;
    double longitude;
    char clue[MAX_CLUE];
    int value;
} Treasure;

void log_action(const char* hunt_id, const char* action);
int add_treasure(const char* hunt_id);
int list_treasures(const char* hunt_id);
int view_treasure(const char* hunt_id, int treasure_id);

void build_path(char* dest, const char* dir, const char* file) {
    strcpy(dest, dir);
    strcat(dest, "/");
    strcat(dest, file);
}

void format_time(time_t time_val, char* buffer, size_t buffer_size) {
    struct tm* time_info = localtime(&time_val);
    char temp[20];

    sprintf(temp, "%d", time_info->tm_year + 1900);
    strcpy(buffer, temp);

    strcat(buffer, "-");
    sprintf(temp, "%02d", time_info->tm_mon + 1);
    strcat(buffer, temp);

    strcat(buffer, "-");
    sprintf(temp, "%02d", time_info->tm_mday);
    strcat(buffer, temp);

    strcat(buffer, " ");
    sprintf(temp, "%02d", time_info->tm_hour);
    strcat(buffer, temp);

    strcat(buffer, ":");
    sprintf(temp, "%02d", time_info->tm_min);
    strcat(buffer, temp);

    strcat(buffer, ":");
    sprintf(temp, "%02d", time_info->tm_sec);
    strcat(buffer, temp);
}

void log_action(const char* hunt_id, const char* action) {
    char log_path[MAX_PATH];
    time_t now = time(NULL);
    char time_str[64];
    char log_entry[MAX_LOG_ENTRY];

    format_time(now, time_str, sizeof(time_str));

    strcpy(log_entry, "[");
    strcat(log_entry, time_str);
    strcat(log_entry, "] ");
    strcat(log_entry, action);
    strcat(log_entry, "\n");

    build_path(log_path, hunt_id, "logged_hunt");

    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Failed to open log file");
        return;
    }

    write(fd, log_entry, strlen(log_entry));
    close(fd);
}

int add_treasure(const char* hunt_id) {
    char dir_path[MAX_PATH];
    char treasures_path[MAX_PATH];
    Treasure treasure;
    struct stat st = {0};
    char log_message[MAX_LOG_ENTRY];

    strcpy(dir_path, hunt_id);

    if (stat(dir_path, &st) == -1) {
        if (mkdir(dir_path, 0755) == -1) {
            perror("Failed to create hunt directory");
            return -1;
        }
    }

    build_path(treasures_path, dir_path, "treasures");

    int fd = open(treasures_path, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("Failed to open treasures file");
        return -1;
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        perror("Failed to get file stats");
        close(fd);
        return -1;
    }

    int num_treasures = file_stat.st_size / sizeof(Treasure);
    treasure.id = num_treasures + 1;
    printf("Enter username: ");
    scanf("%63s", treasure.username);

    printf("Enter latitude: ");
    scanf("%lf", &treasure.latitude);

    printf("Enter longitude: ");
    scanf("%lf", &treasure.longitude);

    printf("Enter clue (max 1023 chars): ");
    getchar(); // consume newline
    fgets(treasure.clue, MAX_CLUE, stdin);
    if (treasure.clue[strlen(treasure.clue) - 1] == '\n') {
        treasure.clue[strlen(treasure.clue) - 1] = '\0';
    }

    printf("Enter value: ");
    scanf("%d", &treasure.value);

    if (lseek(fd, 0, SEEK_END) == -1) {
        perror("Failed to seek to end of file");
        close(fd);
        return -1;
    }

    if (write(fd, &treasure, sizeof(Treasure)) != sizeof(Treasure)) {
        perror("Failed to write treasure to file");
        close(fd);
        return -1;
    }

    close(fd);

    strcpy(log_message, "Added treasure ID ");
    char id_str[16];
    sprintf(id_str, "%d", treasure.id);
    strcat(log_message, id_str);
    strcat(log_message, " by user ");
    strcat(log_message, treasure.username);

    log_action(hunt_id, log_message);

    printf("Treasure added successfully with ID %d\n", treasure.id);
    return 0;
}

int list_treasures(const char* hunt_id) {
    char treasures_path[MAX_PATH];
    Treasure treasure;
    struct stat st;
    char log_message[MAX_LOG_ENTRY];
    time_t mod_time;
    char time_str[64];

    build_path(treasures_path, hunt_id, "treasures");

    if (stat(treasures_path, &st) == -1) {
        printf("Hunt '%s' does not exist", hunt_id);
        return -1;
    }

    int fd = open(treasures_path, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open treasures file");
        return -1;
    }

    if (fstat(fd, &st) == -1) {
        perror("Failed to get file stats");
        close(fd);
        return -1;
    }

    mod_time = st.st_mtime;
    format_time(mod_time, time_str, sizeof(time_str));

    printf("Hunt: %s\n", hunt_id);
    printf("File size: %ld bytes\n", st.st_size);
    printf("Last modified: %s\n\n", time_str);

    printf("ID | Username | Coordinates | Value\n");
    while (read(fd, &treasure, sizeof(Treasure)) == sizeof(Treasure)) {
        printf("%d | %s | (%.6f, %.6f) | %d\n",
               treasure.id, treasure.username,
               treasure.latitude, treasure.longitude, treasure.value);
    }

    close(fd);

    strcpy(log_message, "Listed treasures in hunt ");
    strcat(log_message, hunt_id);
    log_action(hunt_id, log_message);

    return 0;
}
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Operations:\n");
        printf("  add <hunt_id>\n");
        printf("  list <hunt_id>\n");
        return 1;
    }

    if (strcmp(argv[1], "add") == 0) {
        if (argc < 3) {
            printf("Usage: %s add <hunt_id>\n", argv[0]);
            return 1;
        }
        return add_treasure(argv[2]);
    } else if (strcmp(argv[1], "list") == 0) {
        if (argc < 3) {
            printf("Usage: %s list <hunt_id>\n", argv[0]);
            return 1;
        }
        return list_treasures(argv[2]);
    } else {
        printf("Unknown operation: %s\n", argv[1]);
        printf("Only 'add' and 'list' are implemented.\n");
        return 1;
    }

    return 0;
}

