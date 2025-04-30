#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

pid_t monitor_pid = 0;//variabila care stocheaza PID-ul al proccesului de monitorizare
//initializat cu 0 inseamna ca nu este pornit
void start_monitor() {
    if (monitor_pid != 0) {//daca este deja activ,se afiseaza un mesaj si functia se opreste
        printf("Monitorul este deja pornit (PID: %d).\n", monitor_pid);
        return;
    }

    monitor_pid = fork();//se creeaza un procces copil
    if (monitor_pid == 0) {
        while (1) {//simuleaza infinit monitorizarea
            printf("Urmaresc vânătorile de comori.. \n");
            sleep(2); // afiseaza un mesaj la fiecare 2 sec
        }
        exit(0); // proccesul se termina daca bucla este intrerupta
    } else if (monitor_pid > 0) {//daca fork() retueneaza pozitiv,acesta este PID-ul procesului coopil 
        //stocat in monitor_pid
        printf("Procesul monitor a fost pornit cu PID %d.\n", monitor_pid);
    } else {
        perror("Eroare la pornirea procesului monitor");//daca returneza -1 atunci procesul nu s-a efectuat 
    }
}
void stop_monitor() {
    if (monitor_pid != 0) {//daca monitor_pid este 0 atunci procesul nu este activ
        kill(monitor_pid, SIGTERM); // trimite semnal pentru terminare
        printf("Procesul monitor PID %d a fost oprit.\n", monitor_pid);
        monitor_pid = 0;//punem pe 0 pt a indica ca procesul a fost oprit
    } else {
        printf("Monitorul nu este pornit.\n");
    }
}
void handle_exit(int sig) {
    printf("\nIesire program\n");//se afiseaza un mesaj de iesire
    if(monitor_pid !=0){
    stop_monitor(); // opreste procesul de monitorizare daca este activ
    }
    exit(0);//termina programul
}

int main() {
    char command[50];//array de carac care stocheaza comanda introdusa de utilizator 
    signal(SIGINT, handle_exit);//configureaza programul la apelul semnalului SIGINT,apasarea CTRL+C
    //atunci cand semnalul este primit functia handle_exit va fi apelata
    while (1) {//permite programul sa ruleze continuu asculand comenzi de la utilizator,programul
        //se opreste cand utilizatorul apasa CTRL+C sau comanda EXIT
        printf("Introdu comanda: ");//cere comanda
        scanf("%s", command);

        if (strcmp(command, "start_monitor") == 0) {//compara sirul de caractere stocat in variabila command cu 'start_monitor',
            //daca sunt identice strcmp retuneaza valoarea 0
            start_monitor();//daca este adevarat se apeleaza functia start_monito
        } else if (strcmp(command, "stop_monitor") == 0) {//compara sirul 'command' cu textul 'stop_monitor'
            stop_monitor();//daca este adevarat se apeleaza functia
        } else if (strcmp(command, "exit") == 0) {//compara sirul 'command' cu 'exit'
            handle_exit(0);//daca etse adevarat este apelata
        } else {
            printf("Comanda necunoscuta.\n");
        }
    }

    return 0;
}