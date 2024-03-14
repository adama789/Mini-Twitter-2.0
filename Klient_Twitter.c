// Adam Cedro twitter klient
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAZW_UZYT 50
#define MAX_WPIS 200

typedef struct {

    int maks_rekordy;
    char nazwa_uzyt[MAX_NAZW_UZYT];
    char wpis[MAX_WPIS];
    int polubienia;

} Rekord;

char *nazwa_pliku, *nazwa_uzyt;
key_t key;
Rekord *rekordy;
int shmid;


int main(int argc, char *argv[]) {
    int wolne_sloty = 0, i, maks_rekordy, numer_wpisu;
    char typ;
    char komunikat[255];


    if (argc != 3) {
        fprintf(stderr, "./klient [path] [user]\n");
        exit(1);
    }

    nazwa_pliku = argv[1];
    nazwa_uzyt = argv[2];

    // Tworzę klucz od serwera
    if ((key = ftok(nazwa_pliku, 1)) == -1) {
        perror("ftok");
        exit(1);
    }

    // Znajduję segment SHM
    if ((shmid = shmget(key, 0, 0)) == -1) {
        perror("shmget");
        exit(1);
    }

    // Dołączam SHM do klienta
    if ((rekordy = shmat(shmid, NULL, 0)) == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    maks_rekordy = rekordy[0].maks_rekordy;

    printf("Twitter 2.0 wita! (wersja C)\n");

    // Obliczam ilość wolnych slotów
    for (i = 0; i < maks_rekordy; i++) {
        if (strlen(rekordy[i].wpis) == 0) {
            wolne_sloty++;
        }
    }

    printf("[Wolnych %d wpisow (na %d)]\n", wolne_sloty, maks_rekordy);

    // Wypisuję istniejące wpisy
    for (i = 0; i < maks_rekordy - wolne_sloty; i++) {

        if (i == 0) {
            printf("Istniejace wpisy\n");
        }
        printf("\t%d. %s [Autor: %s, Polubienia: %d]\n", i + 1, rekordy[i].wpis, rekordy[i].nazwa_uzyt, rekordy[i].polubienia);
    }

    printf("Podaj akcje (N)owy wpis, (L)ike\n");

    // Sprawdzam poprawność wprowadzonych danych
    while (1) {

        scanf("%c", &typ);

        if ((typ == 'N' || typ == 'L') && isspace(getchar())) {
            break;
        }
    }

    if (typ == 'N') {

        if (wolne_sloty <= 0) {
            printf("Brak slotow na wpisy\n");
            exit(0);
        }

        printf("Napisz co ci chodzi po głowie:\n");

        fgets(komunikat, 255, stdin);

        if (strlen(komunikat) > 0 && komunikat[strlen(komunikat) - 1] == '\n') {
            komunikat[strlen(komunikat) - 1] = '\0'; // Usuwam znak nowej lini z bufora
        }   

        strcpy(rekordy[maks_rekordy - wolne_sloty].wpis, komunikat);
        strcpy(rekordy[maks_rekordy - wolne_sloty].nazwa_uzyt, nazwa_uzyt);
        rekordy[maks_rekordy - wolne_sloty].polubienia = 0;

    } else if (typ == 'L') {

        if (maks_rekordy - wolne_sloty == 0) {
            printf("Brak wpisow aby zostawić like\n");
            exit(0);
        }

        printf("Który wpis chcesz polubic\n");
        scanf("%d", &numer_wpisu);

        if (numer_wpisu < 1 || numer_wpisu > maks_rekordy || strlen(rekordy[numer_wpisu - 1].wpis) == 0) {
            printf("Nieprawidłowy numer wpisu\n");
            exit(0);
        }

        rekordy[numer_wpisu - 1].polubienia++;

    }

    printf("Dziekuje za skorzystanie z aplikacji Twitter 2.0\n");

    return 0;

}