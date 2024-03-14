#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

#define MAX_NAZW_UZYT 50
#define MAX_WPIS 200

typedef struct {
    int maks_rekordy;
    char nazwa_uzyt[MAX_NAZW_UZYT];
    char wpis[MAX_WPIS];
    int polubienia;
} Rekord;

char *nazwa_pliku;
int shmid, maks_rekordy;
key_t key;
Rekord *rekordy;

// Obsługa sygnałów
void handler(int sig) {
    int i;
    if (sig == SIGINT) {
        // Sprzątam i kończę
        if (shmdt(rekordy) == -1) {
            perror("shmdt");
            exit(1);
        }

        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            perror("shmctl");
            exit(1);
        }

        printf("\n[Serwer]: dostałem SIGINT => kończę i sprzątam... (odłączenie: OK, usunięcie: OK)\n");

        exit(0);
    }
    if (sig == SIGTSTP) {
        // Wypisuję rekordy
        if (strlen(rekordy[0].wpis) == 0) {
            printf("\nBrak wpisów\n");
        } else {
            printf("\n___________  Twitter 2.0:  ___________\n");
            for (i = 0; i < maks_rekordy; i++) {
                if (strlen(rekordy[i].wpis) > 0) {
                    printf("[%s]: %s [Polubienia: %d]\n", rekordy[i].nazwa_uzyt, rekordy[i].wpis, rekordy[i].polubienia);
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "./serwer [path] [n]\n");
        exit(1);
    }

    nazwa_pliku = argv[1];
    maks_rekordy = atoi(argv[2]);

    printf("[Serwer]: Twitter 2.0 (wersja C)\n");

    // Tworzę klucz
    if ((key = ftok(nazwa_pliku, 1)) == -1) {
        perror("ftok");
        exit(1);
    }

    // Tutaj postanowiłem usunąć id klucza z powodu braku kompatybilności pomiędzy FreeBSD a Linux
    printf("[Serwer]: tworzę klucz na podstawie pliku %s... OK\n", nazwa_pliku);

    // Tworzę SHM
    if ((shmid = shmget(key, maks_rekordy * sizeof(Rekord), 0644 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }

    printf("[Serwer]: tworzę segment pamięci wspólnej na %d wpisów po %ldB... OK (id: %d, rozmiar: %ldB)\n", maks_rekordy, sizeof(Rekord), shmid, maks_rekordy * sizeof(Rekord));

    // Dołączam SHM
    if ((rekordy = shmat(shmid, NULL, 0)) == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    rekordy[0].maks_rekordy = maks_rekordy;

    printf("[Serwer]: dołączam pamięć wspólną... OK (adres: %p)\n", (void *)rekordy);

    // Sygnały
    signal(SIGINT, handler);
    signal(SIGTSTP, handler);

    printf("[Serwer]: naciśnij Ctrl^Z, aby wyświetlić stan serwisu\n");
    printf("[Serwer]: naciśnij Ctrl^C, aby zakończyć program\n");

    // Czekam na sygnał
    while (1) {
        pause();
    }

    return 0;
}