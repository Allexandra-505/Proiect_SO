
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_USERNAME 64
#define MAX_CLUE 1024

typedef struct {
    int id;
    char username[MAX_USERNAME];
    double latitude;
    double longitude;
    char clue[MAX_CLUE];
    int value;
}Treasure;//structura pt o comoara cu informatii

typedef struct {
    char username[MAX_USERNAME];
    int total_score;
}Score;//retine scorul total a unui utilizator

int main(int argc, char *argv[]) {
    if (argc < 2) {//verifica daca utilizatorul a introdus numele hunt-ului
        //daca nu afiseaza mesaj de utilizare si iese cu eroare
        printf("Usage: %s <hunt_id>\n", argv[0]);
        return 1;
    }

    char path[256];//construieste calea catre fisierul treasure
    strcpy(path, argv[1]);//copiaza in path argumentul hunt_id 
    strcat(path, "/treasures");//adauga /treasure rezultatul final fiind hunt06/treasures

    int fd = open(path, O_RDONLY);//deschide fisierul pt citire,returneaza un descriptor de fisier
    if (fd == -1) {
        printf("Could not open treasures file: %s\n", path);
        return 1;
    }

    Score scores[100];//vector care va retine scorurile a maxim 100 de utilizatori
    int score_count = 0;//cati utilizatori am intalnit
    Treasure t;//variabila temporara in care vom citi fiecare comoara din fisier

    ssize_t bytes_read;//variabila care retine nr-ul de octeti la fiecare apel de read()
    while ((bytes_read = read(fd, &t, sizeof(Treasure))) == sizeof(Treasure)) {
        //parcurge fisierul binar treasure,la fiecare iteratie citeste o structura Treasure
        //completa in t,daca citirea reuseste complet,continua bucla
        int found = 0;//variabila pt a arata daca utilizatorul a fost gasit
        for (int i = 0; i < score_count; i++) {
            if (strcmp(scores[i].username, t.username) == 0) {//cauta daca t.username exista deja in scores
                scores[i].total_score = scores[i].total_score + t.value;//adauga valoarea comorii la scorul existent 
                found = 1;//setam
                break;
            }
        }//daca e nou
        if (found == 0){
            strcpy(scores[score_count].username, t.username);//se adauga o noua intrare in vectorul scores
            scores[score_count].total_score = t.value;//se seteaza scorul initial
            score_count++;//incrementam score_count
        }
    }

    close(fd);//dupa ce s-a terminat citirea inchidem fisierul treasures

    for (int i = 0; i < score_count; i++) {
        printf("%s: %d\n", scores[i].username, scores[i].total_score);
        //afiseaza fiecare utilizator si scorul lui total
    }

    return 0;
}
