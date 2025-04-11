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

void format_time(time_t time_val, char* buffer, size_t buffer_size) {//timpul format
  //  in time_t,un sir de cacartere unde va fi salvat rezultatul,dim max
    struct tm* time_info = localtime(&time_val);//tranf time_val intr-o 
    //strcuctura tm care contine anul,luna,ziua,...
    //local time ret un pointer la o structura tm,anii de la 1900,
    //luni de la 0 la 11,ziua(1-31),min (0-59),..
    char temp[20];//buffer in care se pune fiecare parte a timpului

    sprintf(temp, "%d", time_info->tm_year + 1900);//scrie nr. in string si
    //adaugam la tm_year 1900 sa obtinem anul real
    strcpy(buffer, temp);//ex:copiaza 2025 in buffer
    //folosin o singura daca strcpy pentru a nu sterge tot ce avem pana acum 
    //si sa se salveze doar luna,ora,..

    strcat(buffer, "-");//linie intre fiecare,an,luna,zi
    sprintf(temp, "%02d", time_info->tm_mon + 1);//tm_mon apartine 0-11,deci adaugam 1
    //02 ca sa luam luna cu 2 cifre
    strcat(buffer, temp);//adauga in buffer

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

void log_action(const char* hunt_id, const char* action) {//numele dir,un mesaj care
    //contine actiunea ex: Added treasure ID1
    char log_path[MAX_PATH];//creeaza un buffer pentru calea completa a fisierului logged_hunt
    time_t now = time(NULL);//obt timpul curent
    char time_str[64];//buffer pentru a stoca timpul intr-un format 
    char log_entry[MAX_LOG_ENTRY];//buffer pt mesajul final 

    format_time(now, time_str, sizeof(time_str));//apeleaza functia si converteste
    // now intr-un string mai frumos 

    strcpy(log_entry, "[");
    strcat(log_entry, time_str);
    strcat(log_entry, "] ");
    strcat(log_entry, action);
    strcat(log_entry, "\n");//dupa aceste apelari o sa imi afiseze timpul
    //intr-un format frumos apoi un mesaj cu actiunea

    build_path(log_path, hunt_id, "logged_hunt");//crfeeaza calea completa catre fisier de log
    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    //deschide fisir pt screire,il creaza daca nu exista,scrie la final fara sa
    //stearga continutul,proprietarul scrie,ceilalti doar citesc
    if (fd == -1) {
        perror("Failed to open log file");
        return;
    }

    write(fd, log_entry, strlen(log_entry));//scrie continutul lui log_entry in fisierul deschis
    close(fd);
}

int add_treasure(const char* hunt_id) {
    //folosim id-ul pt a determina directorul in care se salveaza inform vanatorii,0 succes,-1 eroare
    char dir_path[MAX_PATH];//stocheaza calea catre director
    char treasures_path[MAX_PATH];//calea completa catre fisierul in care sunt salvate comorile
    Treasure treasure;//structura c are cont info despre comoara
    struct stat st = {0};//strcut stat obt info despre fisiere
    char log_message[MAX_LOG_ENTRY];//mesajul de log ce va fi scris in jurnalul actiunilor 

    strcpy(dir_path, hunt_id);//copiez id in path pt a indica locul unde sunt salvate fisierele vanatorii
    if (stat(dir_path, &st) == -1) {//verifica daca directorul exista ,daca nu return -1
        if (mkdir(dir_path, 0755) == -1) {//daca da at il creeaza cu permisiuni,prop scrie,ceilalti pot citi/executa
            perror("Failed to create hunt directory");
            return -1;
        }
    }

    build_path(treasures_path, dir_path, "treasures");//construiec calea completa catre fisieruk
    //de comori,adaugand comorile la calea directorului vanatorii
    //ex: hunt123->hunt123/treasures

    int fd = open(treasures_path, O_RDWR | O_CREAT, 0644);//deschidem fisierul de comori
    //daca nu exista il creeaza,prop scrie,ceilalti citesc
    if (fd == -1) {
        perror("Failed to open treasures file");
        return -1;
    }

    struct stat file_stat;//obt info despre fisier
    if (fstat(fd, &file_stat) == -1) {//obt dimensiunea 
        perror("Failed to get file stats");
        close(fd);
        return -1;
    }

    int num_treasures = file_stat.st_size / sizeof(Treasure);//calc nr de comori
    //imparte dim tot in octeti la dim unui obiect Treasure pt a obt nr de comori
    treasure.id = num_treasures + 1;//id-ul pt noua comoar,o valoare unica care se 
    //bazeaza pe nr de comori deja existente
    printf("Enter username: ");
    scanf("%63s", treasure.username);//numele care adauga comoara
    //colecteaza inform de la utilizator 
    printf("Enter latitude: ");
    scanf("%lf", &treasure.latitude);//coordon. geografice ale comorii

    printf("Enter longitude: ");
    scanf("%lf", &treasure.longitude);

    printf("Enter clue (max 1023 chars): ");
    getchar(); // consume newline
    fgets(treasure.clue, MAX_CLUE, stdin);
    if (treasure.clue[strlen(treasure.clue) - 1] == '\n') {
        treasure.clue[strlen(treasure.clue) - 1] = '\0';
    }

    printf("Enter value: ");//colectarea comorii
    scanf("%d", &treasure.value);

    if (lseek(fd, 0, SEEK_END) == -1) {//deplaseaza pointerul de fisier la sfarsit pt a adauga
        //la finalul fisierului
        perror("Failed to seek to end of file");
        close(fd);
        return -1;
    }

    if (write(fd, &treasure, sizeof(Treasure)) != sizeof(Treasure)) {//screierea comorii in fisier 
        perror("Failed to write treasure to file");
        close(fd);
        return -1;
    }

    close(fd);

    strcpy(log_message, "Added treasure ID ");//crearea si scrierea unui mesaj de log 
    //creeaza un mesaj care indica faptul ca o comoara a fost adaugata,continand ID-ul comorii
    //si utilizatorul care a adaugat-o 
    char id_str[16];
    sprintf(id_str, "%d", treasure.id);
    strcat(log_message, id_str);
    strcat(log_message, " by user ");
    strcat(log_message, treasure.username);

    log_action(hunt_id, log_message);//scrie mesajul de log in fisierul de log folosind functia log_action

    printf("Treasure added successfully with ID %d\n", treasure.id);
    return 0;
}
int main(int argc, char *argv[]) {
    if (argc < 2) {//daca nr de arg <2 at prog a fost lansat fara param sau cu less
        printf("Operations:\n");
        printf("  add <hunt_id>\n");//afis un mesaj cu operatiile disponibile
        return 1;
    }

    if (strcmp(argv[1], "add") == 0) {//daca primul argumeste este add,daca e adv at continua 
        //cu adaugarea unuei comori
        if (argc < 3) {
            printf("Usage: %s add <hunt_id>\n", argv[0]);
            return 1;
        }
        return add_treasure(argv[2]); 
    } else {
        printf("Unknown operation: %s\n", argv[1]);
        printf("Only 'add' and 'list' are implemented.\n");
        return 1;
    }

    return 0;
}

