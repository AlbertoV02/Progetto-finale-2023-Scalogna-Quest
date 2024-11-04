#ifndef GAMELIB_H
#define GAMELIB_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// definizioni nuovin tipi
typedef enum { barbaro, nano, elfo, mago } classe_giocatore;
typedef enum { corridoio, scala, sala_banchetto, magazzino, giardino, posto_guardia, prigione, cucina, armeria, tempio } Tipo_zona;
typedef enum { nessun_tesoro, veleno, guarigione, doppia_guarigione } Tipo_tesoro;
typedef enum { nessuna_porta, porta_normale, porta_da_scassinare } Tipo_porta;

// Struttura  Giocatore
typedef struct {
    char nome_giocatore[64];
    classe_giocatore classe;
    struct Zona_segrete* posizione;
    unsigned char p_vita;
    unsigned char dadi_attacco;
    unsigned char dadi_difesa;
    unsigned char mente;
    unsigned char potere_speciale;
} Giocatore;

// Struttura  Zone Segrete
typedef struct Zona_segrete {
    Tipo_zona tipo;
    Tipo_tesoro tesoro;
    Tipo_porta porta;
    struct Zona_segrete* zona_successiva;
    struct Zona_segrete* zona_precedente;
} Zona_segrete;
// Dichiarazioni delle funzioni pubbliche
void imposta_gioco();
void gioca();
void termina_gioco();
#endif
