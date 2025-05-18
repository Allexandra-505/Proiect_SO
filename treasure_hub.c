#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>

pid_t monitor_pid = 0;//pid_t este folosit pentru a stoca ID-ul unui procces/
//monitor_pid va retine PID-ul procesului monitor creat de treasure_hub,
//0 adica nu este pornit

void write_command_to_file(const char *command) {//scrie comanda primita ca text,este folosit pt 
    //a comunica intre treasure_hub si monitor
    int fd = open("command.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    //incearca sa deschida fisierul txt,se deschide pt scriere,daca nu exista se creeaza
    //daca exista se sterge continutul,are permisuni de scriere si citire pentru proprietar
    //si de citire pentru grup si utilizatori
    if (fd == -1) {
        perror("Failed to write command to file");
        return;
    }
    write(fd, command, strlen(command));//scrie comanda text in fisierul deschis,cu strlen
    //returnam nr de caractere ce trebuie scrise 
    close(fd);
}

void sigchld_handler(int sig) {//functia trateaza semnalul SIGCHLD primit de la proces copil care
    // s-a terminal,parametrul sig retine nr semnalului SIGCHLD
    (void)sig;//evitam un warning pentru ca nu folosim aceasta variabila
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);//waitpid este folosit pentru a astepta terminarea 
    //procces copil ,-1 inseamna ca asteapta orice proces copil,&status va contine codul de 
    //terminare al procesului,WOHANG spune sa nu blocheze daca niciun copil nu s-a terminat inca 
    if (pid == monitor_pid) {///verificam daca procesul care s-a terminat este chiar procesul monitor 
        //pe care l-am pornit mai devreme 
        monitor_pid = 0;//daca monitor nu s-a terminat resetam la 0 pt ca nu mai e activ
        printf("Monitor has stopped.\n");
    }
}

void stop_monitor() {//opreste procesul monitor daca e activ
    if (monitor_pid == 0) {//e oprit
        printf("Monitor is not running.\n");
        return;
    }

    write_command_to_file("stop_monitor");//apeleaza functia care scrie textul stop_monitor
    //in fisierul txt
    kill(monitor_pid, SIGUSR1);//trimite semnalul SIGUSR1 catre procesul monitor
    //el are un handler pt SIGUSR1 care va citi fisierul txt si va reactiona
    waitpid(monitor_pid, NULL, 0);//asteapta ca monitorul sa se termine,blocheaza executia pana cand s-a incheiat
    monitor_pid = 0;//seteaza la 0 adica nu mai e monitor activ
    printf("Monitor has been stopped.\n");
}

void start_monitor() {//porneste procesul monitor
    if (monitor_pid != 0) {//monitor_pid contine PID-ul procesului monitor daca e activ
        //daca e diferit de 0 inseamna ca deja ruleaza
        printf("Monitor is already running (PID: %d).\n", monitor_pid);
        return;
    }

    monitor_pid = fork();//creeaza un procces copil,monitor_pid va fi 0
    if (monitor_pid == 0) {//daca suntem in procesul copill monitor_pid=0,se 
        //foloseste execl pt a inlocui procesul copil cu programul ./monitor
        execl("./monitor", "monitor", NULL);
        perror("Failed to launch monitor");//daca esueaza se afiseaza eroare si copilul iese cu codul 1
        exit(1);
    } else {
        printf("Monitor has been started(PID: %d).\n", monitor_pid);
        //daca suntem in procesul parinte afisam un mesaj cum ca monitorul a fost pornit si ii afisam PID-ul
    }
}

void send_command(const char *command) {//primeste ca parametru un sir de caractere,
    //adica comanda pe care utilizatorul vrea sa o btrimita monitorului,nu intoarce nici un rezultat
    if (monitor_pid == 0) {//daca monitorPID=0 at nu s-a pornit nici un proces monitor
        printf("Monitor is not running. Please start it first.\n");
        return;
    }
    write_command_to_file(command);//salveaza comanda in fisierul txt,foloseste functia
    //write_command_to_file care deschide fisierul il scrie si il inchide
    kill(monitor_pid, SIGUSR1);//trimite semnalul SIGUSR1 catre procesor monitor,asta declanseaza in
    // monitor functia sigusr1_handler(),care citeste comanda din fisier si o executa
     printf("Command sent: %s\n", command);
}


void calculate_scores() {
    DIR *dir = opendir(".");
    if (!dir) {
        perror("Failed to open current directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
            char *hunt_id = entry->d_name;

            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("Failed to create pipe");
                continue;
            }

            pid_t pid = fork();
            if (pid < 0) {
                perror("Failed to fork");
                close(pipefd[0]);
                close(pipefd[1]);
                continue;
            }

            if (pid == 0) {
                close(pipefd[0]); 
                dup2(pipefd[1], STDOUT_FILENO); 
                close(pipefd[1]);

                execl("./score_calculator", "score_calculator", hunt_id, NULL);
                perror("Failed to exec score_calculator");
                exit(1);
            }

            close(pipefd[1]); 

            printf("Scores for hunt '%s':\n", hunt_id);
            char buffer[1024];
            ssize_t bytes_read;
            while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytes_read] = '\0';
                printf("%s", buffer);
            }
            close(pipefd[0]);
            waitpid(pid, NULL, 0); 
            printf("\n");
        }
    }
    closedir(dir);
}

int main() {
     struct sigaction sa_chld;//se declara o structura pt a configura un handler de semnal
    sa_chld.sa_handler = sigchld_handler;//la receptia semnalului SIGHLD se apeleaza sigchld_handler
    sigemptyset(&sa_chld.sa_mask);//nu se blocheaza niciun semnal suplimentar in timpul tratarii
    sa_chld.sa_flags = SA_RESTART;//daca o functie este blocata este intrerupta si va fi reluata automat
    sigaction(SIGCHLD, &sa_chld, NULL);//se aplica configuratia pt SIGHLD

    char command[256];//buffer pt a citit comenzile introduse de utilizator
    printf("Please enter a command: \n");

    while (1) {//se intra intr-o bucla infinita,interfata de utilizator
        printf("> ");
        if (fgets(command, sizeof(command), stdin) != NULL) {//citeste linia de comanda introdusa de utilizator
            command[strcspn(command, "\n")] = '\0';//elimina caracterul newline la sfarsitul liniei
            if (strcmp(command, "start_monitor") == 0) {//daca comanda este start_monitor se lanseaza procesul pt functie
                start_monitor();
            } else if (strcmp(command, "stop_monitor") == 0) {//daca este stop_monitor se opreste monitor
                stop_monitor();
            } else if (strcmp(command, "list_hunts") == 0) {//trimite comanda list_hunts catre monitor
                send_command("list_hunts");
            } else if (strncmp(command, "list_treasures", 14) == 0) {//daca comanda incepe cu list_treasures se trimite exact 
                //asa cum a fost introdusa ist_treasures <hunt_id>
                send_command(command);
            } else if (strncmp(command, "view_treasure", 13) == 0) {
                send_command(command);
             } else if (strcmp(command, "calculate_score") == 0) {
                calculate_scores(); 
            } else if (strcmp(command, "exit") == 0) {//iese din aplicatie doar daca monitorul a fost deja orpit
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
   

