#include "gamelib.h"
// Variabili globali
static Zona_segrete* pFirst = NULL;
static Zona_segrete* pLast = NULL;
static Giocatore* giocatori[4] = {NULL};  // Array di puntatori per i giocatori
static Giocatore* giocatore_corrente= NULL;
static int giocatori_in_ordine[4] = {0, 1, 2, 3}; // Fino a 4 giocatori, posizione nell'array rappresenta ordine per giocare, il valore è la posizone nell'array dei giocatori(cioè il giocatore,0 per il primo esempio)
static bool mappa_impostata = false;
static bool gioco_in_corso=false;
static bool turno_terminato = false;
static const char* classi_giocatore_str[] = {"Barbaro", "Nano", "Elfo", "Mago"};
static const char* TipoZonaStrings[] = {"Corridoio","Scala","Sala banchetto","Magazzino","Giardino","Posto di guardia","Prigione","Cucina","Armeria","Tempio"};
// Dichiarazioni funzioni
static bool apri_porta(Giocatore* giocatore);
static void imposta_giocatori();
static void imposta_mappa();
static bool combatti(Giocatore* giocatore);
static void scappa(Giocatore* giocatore);
static int gioca_potere_speciale(Giocatore* giocatore);
static int conta_zone();
static void genera_mappa();
static void inserisci_zona();
static void cancella_zona();
static void stampa_mappa();
static void stampa_zona(Giocatore* giocatore);
static void mescola_giocatori(int giocatori_in_ordine[], int num_giocatori);
static int ottieni_input_numerico(const char* prompt);
static void pulisci_buffer();
static int casuale(int min, int max);
// Funzione per generare un numero casuale tra min e max inclusi
static int casuale(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// Funzione per pulire il buffer di input
static void pulisci_buffer() {
    int c;
    while ((c = getchar())!= '\n' && c != EOF);
}
static int ottieni_input_numerico(const char* prompt) {
    int input;
    char extra;

    while (1) {
        printf("%s", prompt);

        // Usa scanf per leggere un intero
        if (scanf("%d", &input) == 1) {
            // Verifica se ci sono caratteri extra nel buffer
            if ((extra = getchar()) == '\n') {
                return input;  // Input valido, ritorna il numero
            } else {
                printf("Input non valido. Inserisci un numero.\n");
                // Pulisce il buffer in caso di neceessita
                while ((extra = getchar()) != '\n' && extra != EOF);
            }
        } else {
            printf("Input non valido. Inserisci un numero.\n");
            // Pulisce il buffer in caso di neceessita
            while ((extra = getchar()) != '\n' && extra != EOF);
        }
    }
}

static char ottieni_input_carattere(const char* prompt) {
    char input;
    char extra;

    while (1) {
        printf("%s", prompt);

        if (scanf(" %c", &input) == 1) {  // Legge un singolo carattere, e contrale converrsione risucita

            if ((extra = getchar()) == '\n') {  // Controlla se c'era solo un carattere nell'input
                return input;
            } else {
                printf("Input non valido. Inserisci un solo carattere.\n");
                while ((extra = getchar()) != '\n' && extra != EOF);  // Pulisce il buffer
            }

        } else {
            printf("Errore di input. Riprova.\n");
            while ((extra = getchar()) != '\n' && extra != EOF);  // Pulisce il buffer
        }
    }
}


// Funzione di supporto per lanciare dadi (simula dadi a 6 facce)
static int lancia_dadi(int numero_dadi) {
    int colpi = 0;
    for (int i = 0; i < numero_dadi; i++) {
        int dado = (rand() % 6) + 1;  // Dado a 6 facce
        if (dado <= 3) { // 3 teschi su un dado a 6 facce
            colpi++;
        }
    }
    return colpi;
}

// Funzione per lanciare dadi di difesa
static int lancia_dadi_difesa(int numero_dadi, int tipo) {
    int difese = 0;
    for (int i = 0; i < numero_dadi; i++) {
        int dado = (rand() % 6) + 1;  // Dado a 6 facce
        if ((tipo == 1 && dado <= 2) || (tipo == 0 && dado == 6)) { // Scudo bianco (1) o scudo nero (0)
            difese++;
        }
    }
    return difese;
}
// Funzione per il combattimento con l'abitante, torna true quando abirtante viene sconfitto
static bool combatti(Giocatore* giocatore) {
    printf("Combattimento iniziato tra %s e un abitante delle segrete!\n", giocatore->nome_giocatore);

    // Generazione  caratteristiche dell'abitante delle segrete
    int dadi_attacco_mostro = (rand() % 3) + 1;   // Da 1 a 3 dadi di attacco
    int dadi_difesa_mostro = (rand() % 3) + 1;    // Da 1 a 3 dadi di difesa
    unsigned char p_vita_mostro = (rand() % 6) + 3;         // Da 3 a 8 punti vita

    printf("Abitante delle segrete: %d dadi attacco, %d dadi difesa, %d punti vita.\n", dadi_attacco_mostro, dadi_difesa_mostro, p_vita_mostro);

    // Inizio  combattimento
    while (giocatore->p_vita > 0 && p_vita_mostro > 0) {
        // Tiro iniziale, determina chi attacca per primo
        int dado_iniziale_giocatore = (rand() % 6) + 1;
        int dado_iniziale_mostro = (rand() % 6) + 1;

        int attacca_giocatore_prima = dado_iniziale_giocatore >= dado_iniziale_mostro;
        // Esecuzione del turno di combattimento
        if (attacca_giocatore_prima) {
            // Caso in cui  inizia il giocatore
            printf("%s attacca per primo!\n", giocatore->nome_giocatore);
            int colpi_inflitti = lancia_dadi(giocatore->dadi_attacco);
            printf("%s tira i dadi\n Teschi usciti:%d\n",giocatore->nome_giocatore,colpi_inflitti);
            if (colpi_inflitti > 0) {
              int difese_mostro = lancia_dadi_difesa(dadi_difesa_mostro, 0); // 0 per l'abitante
              printf("Abitante si difende e tira i dadi\n Scudi Neri usciti:%d\n",difese_mostro);
              int danni = colpi_inflitti - difese_mostro;
              if (danni>p_vita_mostro){
                p_vita_mostro=0;
              }else{
                p_vita_mostro -= danni;
                printf("%s infligge %d danni all'abitante delle segrete!\n", giocatore->nome_giocatore, danni);
              }
            } else {
                printf("L'attacco di %s è stato bloccato!\n", giocatore->nome_giocatore);
            }

            if (p_vita_mostro == 0) {
                printf("L'abitante delle segrete è stato sconfitto!\n");
                return true;
            }

            // Turno dell'abitante
            printf("L'abitante delle segrete attacca!\n");
            colpi_inflitti = lancia_dadi(dadi_attacco_mostro);
            printf("Abitante tira i dadi\n Teschi usciti:%d\n",colpi_inflitti);

            int difese_giocatore = lancia_dadi_difesa(giocatore->dadi_difesa, 1); // 1 per il giocatore
            int danni = colpi_inflitti - difese_giocatore;

            if (colpi_inflitti > 0) {
              int difese_giocatore = lancia_dadi_difesa(giocatore->dadi_difesa, 1); // 1 per il giocatore
              printf("%s tira i dadi per difendersi\n Scudi Bianchi usciti:%d\n",giocatore->nome_giocatore,difese_giocatore);
              danni = colpi_inflitti - difese_giocatore;
              if(danni>giocatore->p_vita){
                giocatore->p_vita = 0;
              }else{
                giocatore->p_vita -= danni;
                printf("L'abitante delle segrete infligge %d danni a %s!\n", danni, giocatore->nome_giocatore);
              }
            } else {
                printf("L'attacco dell'abitante delle segrete è stato bloccato!\n");
            }
        } else {
            // Caso in cui invece iniza l'abitante inizia
            printf("L'abitante delle segrete attacca per primo!\n");
            int colpi_inflitti = lancia_dadi(dadi_attacco_mostro);
            printf("Abitante tira i dadi\n Teschi usciti:%d\n",colpi_inflitti);
            int difese_giocatore = lancia_dadi_difesa(giocatore->dadi_difesa, 1); // 1 per il giocatore
            int danni = colpi_inflitti - difese_giocatore;

            if (colpi_inflitti > 0) {
              int difese_giocatore = lancia_dadi_difesa(giocatore->dadi_difesa, 1); // 1 per il giocatore
              printf("%s tira i dadi per difendersi\n Scudi Bianchi usciti:%d\n",giocatore->nome_giocatore,difese_giocatore);
              int danni = colpi_inflitti - difese_giocatore;
              if(danni > giocatore->p_vita){
                giocatore->p_vita = 0;
              }else{
                giocatore->p_vita -= danni;
                printf("L'abitante delle segrete infligge %d danni a %s!\n", danni, giocatore->nome_giocatore);
              }
            } else {
                printf("L'attacco dell'abitante delle segrete è stato bloccato!\n");
            }

            if (giocatore->p_vita == 0) {
                printf("%s è stato sconfitto!\n", giocatore->nome_giocatore);
                return false;  // Il giocatore è morto
            }

            // Turno del giocatore
            colpi_inflitti = lancia_dadi(giocatore->dadi_attacco);
            printf("%s tira i dadi per attaccare\n Teschi usciti:%d\n",giocatore->nome_giocatore,colpi_inflitti);

            int difese_mostro = lancia_dadi_difesa(dadi_difesa_mostro, 0); // 0 per l'abitante
            danni = colpi_inflitti - difese_mostro;

            if (colpi_inflitti > 0) {
              int difese_mostro = lancia_dadi_difesa(dadi_difesa_mostro, 0); // 0 per l'abitante
              printf("Abitante si difende e tira i dadi\n Scudi Neri usciti:%d\n",difese_mostro);
              danni = colpi_inflitti - difese_mostro;
              if(danni > p_vita_mostro){
                p_vita_mostro=0;
              }else{
                p_vita_mostro -= danni;
                printf("%s infligge %d danni all'abitante delle segrete!\n", giocatore->nome_giocatore, danni);
              }
            } else {
                printf("L'attacco di %s è stato bloccato!\n", giocatore->nome_giocatore);
            }
        }
        if (p_vita_mostro == 0) {
              printf("L'abitante delle segrete è stato sconfitto da %s!\n", giocatore->nome_giocatore);
              return true;
          }
    }

    return false;  // Return di default nel caso non ci sia  un vincitore, al fine di evitare errori di compilazione
}
// Funzione per gestire l'avanzamento del giocatore nella mappa, torna true quando giocatore riesce ad avanzare
static bool avanza(Giocatore* giocatore) {
    // Controlla se la porta deve essere aperta
    if (giocatore->posizione->porta!=nessuna_porta) {
        printf("Non puoi avanzare finché la porta non viene aperta.\n");
        return false;
    }

    // Controlla se la stanza successiva è l'ultima stanza
    if (giocatore->posizione->zona_successiva != NULL) {
        bool fuggito=false;
        // Gestione della probabilità di apparizione di un abitante delle segrete (33% di probabilità)
        if (casuale(1, 100) <= 33) {
            printf("Un abitante delle segrete appare!\n");

            int scelta;
            int risultato = 0;
            bool incontro_concluso = false;

            // Menu di scelta dell'azione da parte del giocatore
            do {
                printf("\n--- Menu Azione ---\n");
                printf("1) Combatti\n");
                printf("2) Scappa\n");
                printf("3) Usa Potere Speciale\n");
                scelta=ottieni_input_numerico("Inserisci numero da 1 a 3: ");
                switch (scelta) {
                    case 1:
                        if (combatti(giocatore)) {
                            incontro_concluso = true;
                        } else {
                            return false;  // Il giocatore è morto durante il combattimento, termina il turno
                        }
                        break;
                    case 2:
                        scappa(giocatore);
                        incontro_concluso = true;
                        fuggito=true;
                        break;
                    case 3:
                        risultato = gioca_potere_speciale(giocatore);
                        if (risultato == 1) {
                            printf("Hai usato il potere speciale per sconfiggere l'abitante!\n");
                            incontro_concluso = true;
                        } else {
                            printf("Non hai poteri speciali disponibili.\n");
                        }
                        break;
                    default:
                        printf("Scelta non valida. Inserisci un'opzione valida (1-3).\n");
                        break;
                }
            } while (!incontro_concluso);
        }
        if(giocatore->p_vita > 0 && !fuggito){
        giocatore->posizione = giocatore->posizione->zona_successiva;
        printf("%s avanza nella zona successiva.\n", giocatore->nome_giocatore);
        return true;  // Continua il gioco
      }else
      return false; // non avanzo se  muoio oppure sono fuggito
    } else {
        printf("Errore: impossibile avanzare oltre l'ultima stanza.\n");
        return false;
    }
}


// Funzione per tornare alla stanzaa precedente
static void indietreggia(Giocatore* giocatore) {
    if (giocatore->posizione->zona_precedente != NULL) {
      if (casuale(1, 100) <= 33) { // probailità che appaia un abitante delle segrete
          printf("Appare un abitante delle segrete che si deve combatterre!\n");
          combatti(giocatore);
      }
      if(giocatore->p_vita>0){
        giocatore->posizione = giocatore->posizione->zona_precedente;
        printf("%s indietreggia nella zona precedente.\n", giocatore->nome_giocatore);
        if(giocatore->posizione->tesoro==nessun_tesoro) // tesoro si rigenera quando indietreggio
        giocatore->posizione->tesoro=casuale(0, 3);
    }
} else {
    printf("Non puoi indietreggiare oltre.\n");
}
}


//Funzione per aprire porta della stanza corrente, torna true quando si riesce ad aprire la porta
static bool apri_porta(Giocatore* giocatore) {
    if (giocatore->posizione->porta == porta_da_scassinare) {
        printf("Devi scassinare la porta.\n");
        int dado = (rand() % 6) + 1;
        printf("Tiro del dado: %d\n", dado);
        if (dado <= giocatore->mente) {
            printf("Porta scassinata con successo!\n");
            giocatore->posizione->porta=nessuna_porta;
            return true;
        } else {
            printf("Fallimento nello scassinare la porta.\n");
            int evento = rand() % 100;
            if (evento < 10) {
                printf("Ricomincia dalla prima stanza!\n");
                giocatore->posizione = pFirst;
                return false;
            } else if (evento < 60) {
                printf("Perdi un punto vita.\n");
                giocatore->p_vita--;
                return false;
            } else {
                printf("Appare un abitante delle segrete che si deve combattere!\n");
                if(combatti(giocatore)){
                giocatore->posizione->porta=nessuna_porta;
                printf("La porta si è aperta!\n");
                return true;
              }else{
                return false;
              }
            }
        }
    } else if(giocatore->posizione->porta == nessuna_porta){
        printf("Non c'è una porta da aprire.\n");
        giocatore->posizione->porta=nessuna_porta;
        return true;
    }else{
    printf("Porta aperta.\n");
    giocatore->posizione->porta=nessuna_porta;
    return true;
  }


}

static void prendi_tesoro(Giocatore* giocatore) {
    switch (giocatore->posizione->tesoro) {
        case nessun_tesoro:
            printf("Nessun tesoro nella zona.\n");
            break;
        case veleno:
            printf("Tesoro avvelenato! Perdi 2 punti vita.\n");
            giocatore->p_vita -= 2;
            giocatore->posizione->tesoro=nessun_tesoro;
            break;
        case guarigione:
            printf("Tesoro di guarigione! Guadagni 1 punto vita.\n");
            giocatore->p_vita += 1;
            giocatore->posizione->tesoro=nessun_tesoro;
            break;
        case doppia_guarigione:
            printf("Tesoro di doppia guarigione! Guadagni 2 punti vita.\n");
            giocatore->p_vita += 2;
            giocatore->posizione->tesoro=nessun_tesoro;
            break;
    }
}

static void scappa(Giocatore* giocatore) {
    int dado = (rand() % 6) + 1;
    if (dado <= giocatore->mente) {
        printf("Scappato con successo!\n");
        indietreggia(giocatore);
    } else {
        printf("Fuga fallita. Dovrai combattere con le difese dimezzate!\n");
        giocatore->dadi_difesa /= 2;  // Riduzione della difesa a metà
        combatti(giocatore);  // Combattimento
        giocatore->dadi_difesa *= 2;  // Riprisino difesa
    }
}
//funzion perusare potere, trona true quando viene usato
static int gioca_potere_speciale(Giocatore* giocatore) {
    if (giocatore->potere_speciale > 0) {
        printf("%s usa il potere speciale per uccidere immediatamente l'abitante delle segrete!\n", giocatore->nome_giocatore);
        giocatore->potere_speciale--;
        return 1;
    } else {
        printf("Nessun potere speciale disponibile.\n");
        return 0;
    }
}
static void inserisci_zona() {
    int posizione;
    printf("Inserisci la posizione in cui inserire la nuova zona (0 per prima, -1 per ultima): ");
    scanf("%d", &posizione);
    pulisci_buffer();

    Zona_segrete* nuova_zona = malloc(sizeof(Zona_segrete));
    if (!nuova_zona) {
        printf("Errore nell'allocazione della memoria per la zona.\n");
        return;
    }

    // Imposta attributi casuali per la nuova zona
    nuova_zona->tipo = casuale(0, 9);
    nuova_zona->tesoro = casuale(0, 3);
    nuova_zona->porta = casuale(0, 2);

    // Inserimento in testa
    if (posizione == 0) {
        nuova_zona->zona_successiva = pFirst;
        nuova_zona->zona_precedente = NULL;
        if (pFirst) {
            pFirst->zona_precedente = nuova_zona;
        } else {
            pLast = nuova_zona;  // Se la lista era vuota, aggiorna anche pLast
        }
        pFirst = nuova_zona;
    }
    // Inserimento in coda
    else if (posizione == -1 || posizione == conta_zone()) {
        nuova_zona->zona_precedente = pLast;
        nuova_zona->zona_successiva = NULL;
        if (pLast) {
            pLast->zona_successiva = nuova_zona;
        } else {
            pFirst = nuova_zona;  // Se la lista era vuota, aggiorna anche pFirst
        }
        pLast = nuova_zona;
    }
    // Inserimento in una posizione specifica
    else {
        Zona_segrete* corrente = pFirst;
        for (int i = 0; corrente != NULL && i < posizione; i++) {
            corrente = corrente->zona_successiva;
        }

        // Verifica se la posizione è valida
        if (!corrente && posizione != conta_zone()) {
            printf("Posizione non valida.\n");
            free(nuova_zona);
            return;
        }

        // Se corrente è NULL, si sta cercando di inserire alla fine della lista
        if (corrente == NULL && posizione == conta_zone()) {
            nuova_zona->zona_precedente = pLast;
            nuova_zona->zona_successiva = NULL;
            pLast->zona_successiva = nuova_zona;
            pLast = nuova_zona;
        } else {
            // Inserimento in una posizione specifica nel mezzo della lista
            nuova_zona->zona_successiva = corrente;
            nuova_zona->zona_precedente = corrente->zona_precedente;
            if (corrente->zona_precedente) {
                corrente->zona_precedente->zona_successiva = nuova_zona;
            } else {
                pFirst = nuova_zona;  // Se corrente era il primo nodo, aggiorna pFirst
            }
            corrente->zona_precedente = nuova_zona;
        }
    }

    printf("Zona inserita con successo.\n");
}


static void genera_mappa() {
  // Dealloca la mappa esistente prima di generare una nuova
 Zona_segrete* corrente = pFirst;
 while (corrente != NULL) {
     Zona_segrete* temp = corrente;
     corrente = corrente->zona_successiva;
     free(temp);  // Dealloca la memoria della zona corrente
 }
 pFirst = pLast = NULL;  // Reimposta la mappa vuota

 // Genera 15 nuove zone
 for (int i = 0; i < 15; i++) {
     Zona_segrete* nuova_zona = malloc(sizeof(Zona_segrete));
     if (!nuova_zona) {
         printf("Errore nell'allocazione della memoria per la zona.\n");
         return;
     }

     // Imposta attributi casuali per la nuova zona
     nuova_zona->tipo = casuale(0, 9);
     nuova_zona->tesoro = casuale(0, 3);
     nuova_zona->porta = casuale(0, 2);
     nuova_zona->zona_successiva = NULL;
     nuova_zona->zona_precedente = NULL;

     // Se la lista è vuota, la nuova zona è la prima
     if (pFirst == NULL) {
         pFirst = nuova_zona;
         pLast = nuova_zona;
     } else {
         // Altrimenti aggiungo alla lista
         pLast->zona_successiva = nuova_zona;
         nuova_zona->zona_precedente = pLast;
         pLast = nuova_zona;
     }
 }

 printf("15 zone generate e collegate correttamente!\n Zone precedentemente create sono state sovrascritte!\n");
}

static void cancella_zona() {
    int posizione;
    printf("Inserisci la posizione della zona da cancellare (0 per prima, -1 per ultima): ");
    scanf("%d", &posizione);
    pulisci_buffer();
    // Cancellazione in testa
    Zona_segrete* corrente;
    if (posizione == 0) {
        if (!pFirst) {
            printf("Mappa vuota.\n");
            return;
        }
        corrente = pFirst;
        pFirst = pFirst->zona_successiva;
        if (pFirst) {
            pFirst->zona_precedente = NULL;
        }
    } else if (posizione == -1) { //Cancellazione in coda
        if (!pLast) {
            printf("Mappa vuota.\n");
            return;
        }
        corrente = pLast;
        pLast = pLast->zona_precedente;
        if (pLast) {
            pLast->zona_successiva = NULL;
        }
    } else {
        // Cancellazione in posizione specifica
        corrente = pFirst;
        for (int i = 0; corrente != NULL && i < posizione; i++) {
            corrente = corrente->zona_successiva;
        }
        if (!corrente) {
            printf("Posizione non valida.\n");
            return;
        }

        if (corrente->zona_precedente) {
            corrente->zona_precedente->zona_successiva = corrente->zona_successiva;
        }
        if (corrente->zona_successiva) {
            corrente->zona_successiva->zona_precedente = corrente->zona_precedente;
        }
    }

    free(corrente);
    printf("Zona cancellata con successo.\n");
    if(conta_zone()>=15)
    mappa_impostata=true;
    else
    mappa_impostata=false;

}



static void stampa_mappa() {
  if(pFirst==NULL){
    printf("Mappa vuota\n");
  }
  Zona_segrete* corrente = pFirst;
    int posizione = 0;
    while (corrente) {
        printf("Posizione %d: Tipo %s, Tesoro: %d, Porta: %d\n",
               posizione,
               TipoZonaStrings[corrente->tipo],
               (corrente->tesoro != 0) ? 1 : 0,
               (corrente->porta != 0) ? 1 : 0);
        corrente = corrente->zona_successiva;
        posizione++;
    }
}
static void stampa_zona(Giocatore* giocatore) {
    if (giocatore->posizione != NULL) {
    Zona_segrete* corrente = giocatore->posizione;
    printf("Tipo %s, Tesoro: %d, Porta: %d\n",
           TipoZonaStrings[corrente->tipo],
           (corrente->tesoro != 0) ? 1 : 0,
           (corrente->porta != 0) ? 1 : 0);
}else{
  printf("Zona non valida");
}
}


static int conta_zone() {
    int count = 0;
    Zona_segrete* corrente = pFirst;
    while (corrente) {
        count++;
        corrente = corrente->zona_successiva;
    }
    return count;
}

static void imposta_giocatori() {
    int num_giocatori = 0;
    num_giocatori = ottieni_input_numerico("Inserisci il numero di giocatori (1-4): ");
    while (num_giocatori < 1 || num_giocatori > 4) {
        num_giocatori = ottieni_input_numerico("Numero di giocatori non valido. Per favore, inserisci un numero tra 1 e 4.\n");
    }

    // Libera eventuali giocatori precedentemente allocati
    for (int i = 0; i < 4; i++) {
        if (giocatori[i] != NULL) {
            free(giocatori[i]);
            giocatori[i] = NULL;
        }
    }

    // Alloca  la memoria per ogni giocatore
    for (int i = 0; i < num_giocatori; i++) {
        giocatori[i] = malloc(sizeof(Giocatore));
        if (giocatori[i] == NULL) {
            printf("Errore nell'allocazione della memoria per il giocatore %d.\n", i + 1);
            exit(1);
        }

      bool nome_valido = false;
      while (!nome_valido) {
          printf("Inserisci il nome del giocatore %d: ", i + 1);
          if (fgets(giocatori[i]->nome_giocatore, 64, stdin) != NULL) {
              size_t len = strlen(giocatori[i]->nome_giocatore);
          //soluzione che fa uso della libreria string.h per risolvere problema doppio invio casusato da puliscibuffer dopo la fgets
              // Rimuove il carattere di nuova linea, se presente
              if (len > 0 && giocatori[i]->nome_giocatore[len - 1] == '\n') {
                  giocatori[i]->nome_giocatore[len - 1] = '\0';
                  len--;
              }else // se non ho carattere fineline signifcia che input superava la dimensione
              pulisci_buffer();

              // Controlla se l'input è vuoto
              if (len == 0) {
                  printf("Nome non valido. Non puoi lasciare il nome vuoto.\n");
                  continue;
              }

              // Controlla se il nome è già stato utilizzato
              bool nome_usato = false;
              for (int j = 0; j < i; j++) {
                  if (strcmp(giocatori[i]->nome_giocatore, giocatori[j]->nome_giocatore) == 0) {
                      nome_usato = true;
                      printf("Nome già usato. Inserisci un nome diverso.\n");
                      break;
                  }
              }

              // Se il nome è valido e non usato, esce dal ciclo
              if (!nome_usato) {
                  nome_valido = true;
              }
          } else {
              printf("Errore nella lettura del nome. Riprova.\n");
          }
      }


        int classe = ottieni_input_numerico("\nScegli la classe del giocatore (0: Barbaro, 1: Nano, 2: Elfo, 3: Mago): ");
        if (classe < 0 || classe > 3) {
            printf("Classe non valida. Imposto Barbaro di default.\n");
            classe = barbaro;
        }
        giocatori[i]->classe = classe;

        switch (classe) {
            case barbaro:
                giocatori[i]->dadi_attacco = 3;
                giocatori[i]->dadi_difesa = 2;
                giocatori[i]->p_vita = 8;
                giocatori[i]->mente = casuale(1, 2);
                giocatori[i]->potere_speciale = 0;
                break;
            case nano:
                giocatori[i]->dadi_attacco = 2;
                giocatori[i]->dadi_difesa = 2;
                giocatori[i]->p_vita = 7;
                giocatori[i]->mente = casuale(2, 3);
                giocatori[i]->potere_speciale = 1;
                break;
            case elfo:
                giocatori[i]->dadi_attacco = 2;
                giocatori[i]->dadi_difesa = 2;
                giocatori[i]->p_vita = 6;
                giocatori[i]->mente = casuale(3, 4);
                giocatori[i]->potere_speciale = 1;
                break;
            case mago:
                giocatori[i]->dadi_attacco = 1;
                giocatori[i]->dadi_difesa = 2;
                giocatori[i]->p_vita = 4;
                giocatori[i]->mente = casuale(4, 5);
                giocatori[i]->potere_speciale = 3;
                break;
        }

        // Opzione per sacrificare 1 punto mente in cambio di 1 punto vita
        char scelta;
    scelta=ottieni_input_carattere("\nVuoi sacrificare 1 punto mente per 1 punto vita? (s/n): ");
        while (scelta != 's' && scelta != 'S' && scelta != 'n' && scelta != 'N') {
            scelta=ottieni_input_carattere("Scelta non valida. Inserisci 's' per sì o 'n' per no: ");
      }
        if (scelta == 's' || scelta == 'S') {
            if (giocatori[i]->mente > 1) {
                giocatori[i]->mente--;
                giocatori[i]->p_vita++;
                printf("Sacrificio fatto! Mente: %d, Vita: %d\n", giocatori[i]->mente, giocatori[i]->p_vita);
            } else {
                printf("Non puoi sacrificare più punti mente.\n");
                continue;
            }
        }
        if(scelta=='n'|| scelta=='N')
        printf("Non puoi piu sacrificare punti mente\n");

        // Opzione per sacrificare 1 punto vita in cambio di 1 punto mente
        scelta=ottieni_input_carattere("Vuoi invece sacrificare 1 punto vita per 1 punto mente? (s/n): ");

        while (scelta != 's' && scelta != 'S' && scelta != 'n' && scelta != 'N') {
            scelta=ottieni_input_carattere("Scelta non valida. Inserisci 's' per sì o 'n' per no: ");

        }
        if (scelta == 's' || scelta == 'S') {
            if (giocatori[i]->p_vita > 1) {
                giocatori[i]->mente++;
                giocatori[i]->p_vita--;
                printf("Sacrificio fatto! Mente: %d, Vita: %d\n", giocatori[i]->mente, giocatori[i]->p_vita);
            } else {
                printf("Non puoi sacrificare più punti vita.\n");
            }
        }if(scelta=='n' || scelta=='N'){
          printf("Non puoi piu sacrificare punti vita\n");
    }
    }

    printf("Giocatori impostati correttamente!\n");
}

// Funzione per impostare la mappa
static void imposta_mappa() {

    int scelta_mappa = 0;
    mappa_impostata = false;

    while (scelta_mappa != 5) {
        printf("\n--- Creazione Mappa ---\n");
        printf("1) Genera mappa\n");
        printf("2) Inserisci zona\n");
        printf("3) Cancella zona\n");
        printf("4) Stampa mappa\n");
        printf("5) Chiudi mappa\n");
        printf("Scelta: ");
        scelta_mappa=ottieni_input_numerico("Inserisci input (1-5):");

        switch (scelta_mappa) {
            case 1:
                genera_mappa();
                break;
            case 2:
                inserisci_zona();
                break;
            case 3:
                cancella_zona();
                break;
            case 4:
                stampa_mappa();
                break;
            case 5:
                // Verifica se ci sono almeno 15 zone
                if (conta_zone() >= 15) {
                    mappa_impostata = true;
                    printf("Mappa chiusa correttamente.\n");

                    // Imposta la posizione di tutti i giocatori su pFirst, cioè la prima stanza
                    for (int i = 0; i < 4; i++) {
                        if (giocatori[i] != NULL) {
                            giocatori[i]->posizione = pFirst;
                        }
                    }

                } else {
                    printf("Errore: la mappa deve avere almeno 15 zone per chiudere.\n");
                    scelta_mappa = 0;  // Forza a ripetere la scelta finché non si raggiunge il numero minimo di zone
                }
                break;
            default:
                printf("Scelta non valida.\n");
        }
    }

    if (!mappa_impostata) {
        printf("Errore: la mappa non è stata impostata correttamente.\n");
    }
}

// Funzione per impostare il gioco
 void imposta_gioco() {
        imposta_giocatori();
    // svuoto mappa nel caso imposti piu volte senza prima giocare
    if(mappa_impostata){
      Zona_segrete* corrente = pFirst;
      while (corrente != NULL) {
          Zona_segrete* temp = corrente;
          corrente = corrente->zona_successiva;
          free(temp);
          temp=NULL;
        }
        pFirst=NULL;
        pLast=NULL;
    }

    imposta_mappa();

}

// Funzione per mescolare i turni dei giocatori casualmente
static void mescola_giocatori(int giocatori_in_ordine[], int num_giocatori) {

    for (int i = num_giocatori - 1; i > 0; i--) {
        // Genera un numero casuale tra 0 e i
        int j = rand() % (i + 1);

        // Scambia giocatori_in_ordine[i] con giocatori_in_ordine[j]
        int temp = giocatori_in_ordine[i];
        giocatori_in_ordine[i] = giocatori_in_ordine[j];
        giocatori_in_ordine[j] = temp;
    }
}


// Funzione per stampare tutte le caratteristiche di un giocatore
void stampa_giocatore(Giocatore* giocatore) {
    // Verifica se il puntatore è valido
    if (giocatore == NULL) {
        printf("Giocatore non valido.\n");
        return;
    }

    // Stampa delle caratteristiche
    printf("Nome del Giocatore: %s\nClasse del Giocatore: %s\n",
           giocatore->nome_giocatore,
           classi_giocatore_str[giocatore->classe]);


    // Stampa delle altre caratteristiche
    printf("Punti Vita: %d\nDadi di Attacco: %d\nDadi di Difesa: %d\nPunti Mente: %d\nPoteri Speciali Disponibili: %d\n",
           giocatore->p_vita,
           giocatore->dadi_attacco,
           giocatore->dadi_difesa,
           giocatore->mente,
           giocatore->potere_speciale);
}

void gioca() {
    if (!mappa_impostata) {
        printf("Errore: imposta prima il gioco.\n");
        return;
    }
    int num_giocatori_attivi = 0;
    gioco_in_corso=true;

    // Conta i giocatori attivi
    for (int i = 0; i < 4; i++) {
        if (giocatori[i] != NULL) {
            num_giocatori_attivi++;
        }
    }

    // Ciclo di gioco
    while (gioco_in_corso && num_giocatori_attivi > 0) {
        mescola_giocatori(giocatori_in_ordine, num_giocatori_attivi);

        // Turno di gioco
        for (int i = 0; i < 4; i++) {
            if (giocatori[giocatori_in_ordine[i]] == NULL) //salta poszioni dei giocatori non allocati
             continue;

            giocatore_corrente = giocatori[giocatori_in_ordine[i]];
            printf("\n--- Turno di %s ---\n", giocatore_corrente->nome_giocatore);

            int scelta;
            turno_terminato = false;

            do {
                printf("\nScegli un'azione:\n");
                printf("1) Avanza\n");
                printf("2) Indietreggia\n");
                printf("3) Stampa giocatore\n");
                printf("4) Stampa zona\n");
                printf("5) Apri porta\n");
                printf("6) Prendi tesoro\n");
                printf("7) Termina turno\n");
                printf("Scelta: ");
                scelta=ottieni_input_numerico("Inserisci numero da 1 a 7: ");
                if(scelta<1||scelta>7)
                while(scelta<1||scelta>7){
                  scelta=ottieni_input_numerico("Inserisci numero da 1 a 7: ");

                }
                switch (scelta) {
                    case 1:  // Avanza
                        avanza(giocatore_corrente);
                            if (giocatore_corrente->posizione==pLast) { //condizione di vittoria
                                printf("Il giocatore %s ha vinto la partita!\n", giocatore_corrente->nome_giocatore);
                                gioco_in_corso = false;
                                mappa_impostata=false;
                                pFirst = NULL;
                                pLast = NULL;
                                return;
                            }
                        break;
                    case 2:
                        indietreggia(giocatore_corrente);
                        break;
                    case 3:
                        stampa_giocatore(giocatore_corrente);
                        break;
                    case 4:
                        stampa_zona(giocatore_corrente);
                        break;
                    case 5:
                        apri_porta(giocatore_corrente);
                        break;
                    case 6:
                        prendi_tesoro(giocatore_corrente);
                        break;
                    case 7:
                        printf("Turno terminato.\n");
                        turno_terminato = true;
                        break;
                    default:
                        printf("Scelta non valida. Riprova.\n");
                }

                // Se il giocatore è morto durante il turno, libero memoria del giocatore e termino turno
                if (giocatore_corrente->p_vita <= 0) {
                    printf("%s è morto.\n", giocatore_corrente->nome_giocatore);
                    free(giocatore_corrente);
                    giocatori[giocatori_in_ordine[i]] = NULL;
                    num_giocatori_attivi--;

                    turno_terminato = true;

                    // Se tutti i giocatori sono morti, termina il gioco
                    if (num_giocatori_attivi == 0) {
                        printf("Tutti i giocatori sono morti. Torna a 'Menu Principale'.\n");
                        printf("Imposta gioco per rigiocare.\n");
                        gioco_in_corso=false;
                        mappa_impostata=false;
                        pFirst = NULL;
                        pLast = NULL;
                        //libera memoria alllocata per i giocatori
                        for (int i = 0; i < 4; i++) {
                            if (giocatori[i] != NULL) {
                                free(giocatori[i]);
                                giocatori[i]=NULL;
                            }
                        }
                        //Libera la memoria allocata per la mappa
                        Zona_segrete* corrente = pFirst;
                        while (corrente != NULL) {
                            Zona_segrete* temp = corrente;
                            corrente = corrente->zona_successiva;
                            free(temp);
                            temp=NULL;
                          }
                        return;  // Termina il gioco quando sono  tutti morti
                    }
                }
            } while (!turno_terminato);
        }

        // Solo dopo che tutti i giocatori hanno giocato un turno chiedo se continuare
        char continua;

        do{
        continua=ottieni_input_carattere("Tutti i giocatori hanno completato un turno.\nVuoi continuare a giocare? ('s' per Si / 'n' per No):\n");

        if (continua == 'n' || continua == 'N') {
          gioco_in_corso=false;
        printf("prova a reimpostare per poter rigiocare.\n");
        mappa_impostata=false;
        pFirst = NULL;
        pLast = NULL;

        // Libera la memoria allocata per i giocatori
        for (int i = 0; i < 4; i++) {
            if (giocatori[i] != NULL) {
                free(giocatori[i]);
                giocatori[i]=NULL;
            }
        }
        //Libera la memoria allocata per la mappa
        Zona_segrete* corrente = pFirst;
        while (corrente != NULL) {
            Zona_segrete* temp = corrente;
            corrente = corrente->zona_successiva;
            free(temp);
            temp=NULL;
          }
            break;  // Esce dal ciclo di gioco se l'utente decide di smettere

    }else if (continua != 's' && continua != 'S') {
        printf("Input non valido. Inserisci 's' per continuare o 'n' per uscire.\n");
    }
  }while(continua!='s'&& continua!='S');
  }
}

void termina_gioco() {
    printf("Grazie per aver giocato! Alla prossima.\n");

    // Libera la memoria allocata per i giocatori
    for (int i = 0; i < 4; i++) {
        if (giocatori[i] != NULL) {
            free(giocatori[i]);
        }
    }
    //Libera la memoria allocata per la mappa
    Zona_segrete* corrente = pFirst;
    while (corrente != NULL) {
        Zona_segrete* temp = corrente;
        corrente = corrente->zona_successiva;
        free(temp);
      }
      mappa_impostata=false;
      pFirst = NULL;
      pLast = NULL;
    exit(0); //gioco terminato corretamente
  }
