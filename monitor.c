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

#define MAX_COMMAND_LENGTH 256//defineste lungimea max pt comanda primita

sig_atomic_t running = 1;//variabila sigura pt semnale care controleaza rularea buclei 
//principale,cand devine 0 programul se opreste
char received_command[MAX_COMMAND_LENGTH];//buffer in care va fi salvata comanda citita din fisierul txt
char hunt_id[MAX_COMMAND_LENGTH];//id-ul vanatorii
int treasure_id;//id pt comoara

void read_command_from_file(char *buffer, size_t size) {//functie care citeste comanda din fisierul 
    //txt si o salveaza in buffer
    int fd = open("command.txt", O_RDONLY);//deschide fisierul txt pt citire
    if (fd == -1) {
        perror("Failed to open command.txt");
        return;
    }
    ssize_t len = read(fd, buffer, size - 1);//citeste din fisier max size-1 octeti si saleaza in buffer
    if (len > 0) {
        buffer[len] = '\0';//daca s-a citit ceva,adauga \0 la finalul comenzii
    }
    close(fd);//inchide fisierul
}

void sigusr1_handler(int sig) {//este apelata atunci cand monitorul primeste semnalul SIGUSR1
    (void)sig;
    read_command_from_file(received_command, MAX_COMMAND_LENGTH);//citeste comanda trimisa din fisierul txt
    printf("Received command: %s\n", received_command);//afiseaza comanda primita

    char *command = strtok(received_command, " ");//extrage primul cuvant din comanda,ex:list_treasures
    if (!command) return;//daca comanda este goala atunci se opreste

    if (strcmp(command, "list_hunts") == 0) {//verifica daca comanda este list_hunts
        printf("Listing hunts:\n");
        DIR *dir = opendir(".");//deschide directorul curent
        if (dir) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {//parcurge fiecare intrare din director
                if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {//afiseaza doar directoarele 
                    //care nu sunt ascunse(nu incep cu .),presupuse sesiuni bde vanatoare
                    printf("  - %s\n", entry->d_name);
                }
            }
            closedir(dir);//inchide directorul la final
        } else {
            perror("Failed to open directory");
        }
    } else if (strcmp(command, "list_treasures") == 0) {//daca comanda este list_treasures extrage
        //urmatorul token care trb sa fie huntID
        char *hunt = strtok(NULL, " ");
        if (hunt == NULL) {//verifica daca utilizatorul a introdus si un huntID
            printf("Hunt ID missing.\n");
            return;
        }
        if (fork() == 0) {
            execl("./treasure_manager", "treasure_manager", "list", hunt, NULL);
            //creeaza un procces copil care ruleaza comanda ./treasure_manager list hunt01

            perror("Exec list error");
            exit(1);
        } else {
            wait(NULL);//proccesul parinte asteapta terminarea copilului
        }
    } else if (strcmp(command, "view_treasure") == 0) {
        char *hunt = strtok(NULL, " ");
        char *treasure = strtok(NULL, " ");
        if (hunt == 0 || treasure == 0) {//verifica daca ambele argumeste au fost introduse
            printf("Hunt or Treasure ID missing.\n");
            return;
        }
        if (fork() == 0) {//creeaza un copil care ruleaza ./treasure_manager view hunt01 2

            execl("./treasure_manager", "treasure_manager", "view", hunt, treasure, NULL);
            perror("Exec view error");
            exit(1);
        } else {
            wait(NULL);
        }
    } else if (strcmp(command, "stop_monitor") == 0) {//daca este STOP,opreste bucla principala 
        //dupa o intarziere de 1 secunda
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
    struct sigaction sa_usr1;//avem o structura sigaction care va defini comportamentul
    //atunci cand primeste semnalul SIGUSR1
    sa_usr1.sa_handler = sigusr1_handler;//setam functia care va fi apelata automat cand se primeste semnalul
    //SIGUSR1,adica sigusr1_handler()
    sigemptyset(&sa_usr1.sa_mask);//initializeaza masca de semnale adica setul de semnale care vor fi 
    //blocate temporar in timpul executiei handler-ului,sigemptyset inseamna ca nici un semnal 
    //nu va fi blocatg temporar cand handle-ul ruleaza
    sa_usr1.sa_flags = SA_RESTART;//SA_RESTART inseamna ca daca un apel de sistem este intrerupt de semnal
    //el va fi reluat automat
    sigaction(SIGUSR1, &sa_usr1, NULL);//inregistram handle-ul pentru SIGUSR1,acum cand primeste SIGUSR1
    //se va executa sigusr1_handler()

    struct sigaction sa_term;//creem o structura pt semnalul SIGTERM
    sa_term.sa_handler = sigterm_handler;//atribuim handle-ul care se va apela cand procesul
    //primeste SIGTERM
    sigemptyset(&sa_term.sa_mask);//nu blocam alte semnale in timpul executiei handler-ului
    sa_term.sa_flags = 0;//nu setam niciun flag special aici
    sigaction(SIGTERM, &sa_term, NULL);//inregistram handle-ul pentru SIGTERM,la primirea
    //acesti semnal se va apela sigterm_handler()
    while (running) {//cat timp running este 1 programul se blocheaza in pause(),asteptand un semnal
        pause();//cand primeste un semnal se apeleaza handle-ul corespunzator SIGUSR1 sau SIGTERM apoi 
        //revine la pause 
    }

    printf("Monitor stopped.\n");
    return 0;
}
