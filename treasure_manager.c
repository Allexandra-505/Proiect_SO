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
int list_treasures(const char* hunt_id) {//primeste ca param un string care reprez
    //numele sesiunii de vanatoare,directorul unde sunt stocate
    char treasures_path[MAX_PATH];//buffer care contine calea comppleta catre fisierul unde sunt salvate 
    //toate comorile despre acel hunt 
    struct stat st;//structura pt a obt info despre fisier
    build_path(treasures_path, hunt_id, "treasures");//crfeeaza calea catre fisierul de comori
    if (stat(treasures_path, &st) == -1) {//verif daca exista si preia info despre el
        perror("Could not access treasures file");
        return -1;
    }
    int fd = open(treasures_path, O_RDONLY);//deschide fisierul treasure pt citire
    if (fd == -1) {
        perror("Failed to open treasures file");
        return -1;
    }

    printf("Hunt: %s\n", hunt_id);//afiseaza informatiile
    printf("Total File Size: %ld bytes\n", st.st_size);//afiseaza dimensiunea totala in bytes

    char time_str[64];
    format_time(st.st_mtime, time_str, sizeof(time_str));//st.st_mtime e timpul ultimei modificari
    //si il converteste intr-un string
    printf("Last Modified: %s\n", time_str);//af data ultimei modificari
    printf("Treasures:\n");
    Treasure t;//o structura pt a citi fiecare comoara din fisier
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {//citeste o structura de fisier pana la final
        printf("  ID: %d | User: %s | Lat: %.5f | Long: %.5f | Value: %d\n",
               t.id, t.username, t.latitude, t.longitude, t.value);
    }
    close(fd);
    log_action(hunt_id, "Listed all treasures.");//inregistreaza in jurnal ca toate comorile au fost listate
    return 0;
}

// Vizualizeaza o comoara după id
int view_treasure(const char* hunt_id, int treasure_id) {//numele dir in care se afla fisierul treasure,
    //id-ul comorii pe care vrem sa o cautam si sa o vedem
    char treasures_path[MAX_PATH];//buffer in c are constr calea completa catre fisierul de comori
    build_path(treasures_path, hunt_id, "treasures");//combina numele dir cu treasure  

    int fd = open(treasures_path, O_RDONLY);//deschide fisierul pt citire
    if (fd == -1) {
        perror("Could not open treasures file");
        return -1;
    }

    Treasure t;//structura pt a citi pe rand fiecare comoara
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {//read citeste o comoara,un bloc de dim sizeof..,
        //din fisier in structura t
        if (t.id == treasure_id) {//daca id-ul comorii citite e egal cu id-ul cautat
            printf("Treasure ID: %d\nUser: %s\nLatitude: %.5f\nLongitude: %.5f\nClue: %s\nValue: %d\n",
                   t.id, t.username, t.latitude, t.longitude, t.clue, t.value);
            close(fd);//daca comoara este gasita afiseaza info din structura
            char log_msg[MAX_LOG_ENTRY];
            sprintf(log_msg, "Viewed treasure ID %d", treasure_id);
            log_action(hunt_id, log_msg);//se construieste un mesaj de log apoi se apeleaza log_action pt a salva 
            //in jurnalul vanatorii aceasta actiune
            return 0;
        }
    }

    printf("Treasure with ID %d not found.\n", treasure_id);
    close(fd);
    return -1;
}

// Sterge o comoara după id
int remove_treasure(const char* hunt_id, int treasure_id) {
    char path[MAX_PATH];//calea completa catre fis de comori
    char temp_path[MAX_PATH];//fis temporar pt a rescrie comorile fara cea stersa
    build_path(path, hunt_id, "treasures");//concateneaza numele dir
    build_path(temp_path, hunt_id, "temp");//concateneaza numele fis 

    int fd = open(path, O_RDONLY);//fis de comori pt citire
    int temp_fd = open(temp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1 || temp_fd == -1) {//fis temorar se deschide pt scriere,se creeaza
        //daca nu exista si se goleste daca exista deja 
        perror("Failed to open files");
        return -1;
    }

    Treasure t;
    int found = 0;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {//citeste fiecare comoara din fis original
        if (t.id != treasure_id) {//daca nu e cel cautat,comoara se scrie in fis temporar
            write(temp_fd, &t, sizeof(Treasure));//daca da at nu se scrie,o stergem si marcam ca a fost found=1
        } else {
            found = 1;
        }
    }

    close(fd);
    close(temp_fd);

    if (!found) {//daca nu a fost gasita 
        printf("Treasure ID %d not found.\n", treasure_id);
        remove(temp_path);//stergem fis temorar
        return -1;
    }

    remove(path);//stergem fis original
    rename(temp_path, path);//se redenumeste fis temporar in locul lui acum fis treasure contine comorile
    //mai purin cea stearsa

    char log_msg[MAX_LOG_ENTRY];//se construieste un mesaj de jurnal
    sprintf(log_msg, "Removed treasure ID %d", treasure_id);
    log_action(hunt_id, log_msg);////se apeleaza functia log_action care salveaza actiunea 
    //intr-un fisier logged_hunt

    printf("Treasure %d removed successfully.\n", treasure_id);
    return 0;
}

// Sterge complet o vanatoare de comori (directorul)
int remove_hunt(const char* hunt_id) {
    char treasures_path[MAX_PATH], log_path[MAX_PATH];//calea completa catre fis cu comori
    build_path(treasures_path, hunt_id, "treasures");//calea completa catre fis de loguri
    build_path(log_path, hunt_id, "logged_hunt");//concateneaza id-ul vanatorii cu numele fis respectiv
    //pt a obt caile complete

    remove(treasures_path);//sterge fis treasure si logged_hunt de pe disc
    remove(log_path);

    if (rmdir(hunt_id) == -1) {//sterge directorul asociat vanatorii,hunt_id
        perror("Failed to remove hunt directory");
        return -1;
    }

    printf("Hunt '%s' deleted successfully.\n", hunt_id);
    return 0;
}

// Functia principala care interpreteaza comenzile din linia de comanda
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Operations:\n");
        printf("  add <hunt_id>\n");
        printf("  list <hunt_id>\n");
        printf("  view <hunt_id> <treasure_id>\n");
        printf("  remove_treasure <hunt_id> <treasure_id>\n");
        printf("  remove_hunt <hunt_id>\n");
        return 1;
    }

    if (strcmp(argv[1], "add") == 0 && argc >= 3) {//verifica daca primul arg este comanda add,si daca 
        //un al doilea arg este necesat pt a adauga o comoara
        return add_treasure(argv[2]);//daca este ok trecem id-ul vanatorii

    } else if (strcmp(argv[1], "list") == 0 && argc >= 3) {
        return list_treasures(argv[2]);

    } else if (strcmp(argv[1], "view") == 0 && argc >= 4) {
        return view_treasure(argv[2], atoi(argv[3]));

    } else if (strcmp(argv[1], "remove_treasure") == 0 && argc >= 4) {
        return remove_treasure(argv[2], atoi(argv[3]));//daca este ok convertim id-ul la int comorii 

    } else if (strcmp(argv[1], "remove_hunt") == 0 && argc >= 3) {
        return remove_hunt(argv[2]);

    } else {
        printf("Unknown command.\n");
        return 1;
    }

    return 0;
}
