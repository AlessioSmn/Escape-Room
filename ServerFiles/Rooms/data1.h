#ifndef DATA1_H
#define DATA1_H

#include "room_structures.h"

#define PLAYERS_PER_GAME_R1 2
#define GAME_DURATION_R1 10*60
#define TOKEN_GOAL_R1 6
#define INVENTORY_DIM_R1 1


// ************************************* Locazioni
#define R1_locations_count 3
const struct Location LOCATION_R1[R1_locations_count] = {
      {"",              "Siete nella stanza di controllo di un edificio sconosciuto, ci sono solo due elementi: un \033[1m++pannello++\033[0m di controllo, affiancato da un piccolo box chiuso contentente un \033[1m**trasmettitore**\033[0m, e un computer, dotato di \033[1m++tastiera++\033[0m e di una strana chiavetta \033[1m**usb**\033[0m bloccata al suo interno"},
      {"tastiera",      "Su questa strana tastiera ci sono solo 4 tasti: \033[1m**alt**\033[0m, \033[1m**shift**\033[0m, \033[1m**canc**\033[0m e \033[1m**ctrl**\033[0m. Attaccato poco sopra c'è un piccolo \033[1m**post-it**\033[0m"},
      {"pannello",      "Gli unici pulsanti presenti sono: \033[1m**accensione**\033[0m, \033[1m**reset**\033[0m e \033[1m**regolatore**\033[0m di velocità, accanto al \033[1m**manuale**\033[0m del pannello"},
};


// ************************************* Oggetti 
#define R1_objects_count 11
struct Object OBJECTS_R1[R1_objects_count] = {
      {
      .name = "alt",
      .collectible = false,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = 0,
      .token = 0,
      .description = {
            "Spesso utilizzato come modificatore insieme ad altri tasti per attivare scorciatoie da tastiera o per inserire caratteri speciali", 
            "Spesso utilizzato come modificatore insieme ad altri tasti per attivare scorciatoie da tastiera o per inserire caratteri speciali"}},
      {
      .name = "shift",
      .collectible = false,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = 0,
      .token = 0,
      .description = {
            "Grande e posizionato ai margini inferiori, per attivare maiuscole o caratteri alternativi", 
            "Grande e posizionato ai margini inferiori, per attivare maiuscole o caratteri alternativi"}},
      {
      .name = "canc",
      .collectible = false,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = 0,
      .token = 0,
      .description = {
            "Usato per eliminare caratteri o elementi selezionati.", 
            "Usato per eliminare caratteri o elementi selezionati."}},
      {
      .name = "ctrl",
      .collectible = false,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = 0,
      .token = 0,
      .description = {
            "Usato in combinazione con altri tasti per attivare funzioni speciali", 
            "Usato in combinazione con altri tasti per attivare funzioni speciali"}},

      {
      .name = "accensione",
      .collectible = false,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = 1,
      .token = 0,
      .description = {
            "Per accendere o spegnere qualche strano dispositivo",
            "Per accendere o spegnere qualche strano dispositivo"}},
      {
      .name = "reset",
      .collectible = false,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = 1,
      .token = 0,
      .description = {
            "Pulsante per ripristinare la macchina dopo un arresto di emergenza o una segnalazione di errore", 
            "Pulsante per ripristinare la macchina dopo un arresto di emergenza o una segnalazione di errore"}},
      {
      .name = "regolatore",
      .collectible = false,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = 1,
      .token = 0,
      .description = {
            "Manopola per regolare la velocità di funzionamento della macchina.", 
            "Manopola per regolare la velocità di funzionamento della macchina"}},
      {
      .name = "usb",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = false, .blockingRiddle = 0,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 1,
      .description = {
            "Una misteriosa chiavetta USB bloccata dentro al computer", 
            "Una misteriosa chiavetta USB"}},
      {
      .name = "post-it",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_OBJECT,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 1,
      .description = {
            "Sotto il sottile strato di polvere si legge:\"\nIn un libro d'emozioni, controllare il destino,\nsotto cieli stellati, spostare ogni confine.\nCancellare gli affanni, come onde che ritornano,\nalternare i sogni, su pagine d'aurora.\"", 
            "Sotto il sottile strato di polvere si legge:\"\nIn un libro d'emozioni, controllare il destino,\nsotto cieli stellati, spostare ogni confine.\nCancellare gli affanni, come onde che ritornano,\nalternare i sogni, su pagine d'aurora.\""}},
      {
      .name = "trasmettitore",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = false, .blockingRiddle = 1,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 1,
      .description = {
            "Un trasmettitore radio a corto raggio per inviare segnali a dispositivi nella sua portata", 
            "Un trasmettitore radio a corto raggio per inviare segnali a dispositivi nella sua portata"}},
      {
      .name = "manuale",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_OBJECT,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 1,
      .description = {
            "Alla pagina 23, paragrafo 'utilizzo prolungato', è scritto: \"...\n dopo anni di utilizzo è spesso necessario, dopo aver acceso il macchinario, ruotare tre volte la manopola di regolazione della velocità.\"",
            "Alla pagina 23, paragrafo 'utilizzo prolungato', è scritto: \"...\n dopo anni di utilizzo è spesso necessario, dopo aver acceso il macchinario, ruotare tre volte la manopola di regolazione della velocità.\"",
      }},
};


// ************************************* Enigmi 

#define R1_riddles_count 2
struct Riddle RIDDLES_R1[R1_riddles_count] = {
      {
            .text = "Premi i pulsanti nell'ordine corretto",
            .riddleType = ObjectSequence,
            .structures.sequenceRiddle = {
                  .objectCounter = 4,
                  .objectsSequence = (const int[4]){3, 1, 2, 0},
                  .currentIndex = 0,
                  .currentSequence = (int[4]){-1, -1, -1, -1}
            },
            .token = 1,
      },
      {
            .text = "Usa i pulsanti nell'ordine corretto",
            .riddleType = ObjectSequence,
            .structures.sequenceRiddle = {
                  .objectCounter = 4,
                  .objectsSequence = (const int[4]){4, 6, 6, 6},
                  .currentIndex = 0,
                  .currentSequence = (int[4]){-1, -1, -1, -1}
            },
            .token = 1,
      },
};

#endif