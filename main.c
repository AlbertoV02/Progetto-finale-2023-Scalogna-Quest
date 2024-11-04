#include "gamelib.h"

int main() {
  time_t t;
  unsigned short scelta = 0;
  char extra;
  srand((unsigned)time(&t));  //Inizializza il generatoredi numeri casuali utilizzando il tempo attuale

    do {
        // Stampa del menu principale
        printf("\n--- Scalogna Quest ---\n");
        printf("1) Imposta gioco\n");
        printf("2) Gioca\n");
        printf("3) Termina gioco\n");

        // Ciclo per ottenere un input valido
        while (true) {
            printf("Scelta: ");
            if ((scanf("%hd",&scelta))==1) {
                // Verifica se l'input è un numero valido senza caratteri extra
                if ((extra=getchar()) == '\n') {
                    break; // Input valido, esce dal ciclo
                } else {
                    printf("Input non valido. Per favore, inserisci un numero tra 1 e 3.\n");
                    while ((extra = getchar()) != '\n' && extra != EOF);  // Pulisce il buffer

                }
            } else {
                printf("Errore di input. Riprova.\n");
                while ((extra = getchar()) != '\n' && extra != EOF);  // Pulisce il buffer
            }
        }

        switch (scelta) {
            case 1:
            imposta_gioco();
                break;
            case 2:
            gioca();
                break;
            case 3:
            termina_gioco();
                break;
            default:
                printf("Comando non valido. Inserisci 1, 2, o 3.\n");
        }
    } while (scelta != 3);

    return 0;
}
