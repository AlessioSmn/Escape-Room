
#include "room_server.h"

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h> // close
#include <time.h>
#include <sys/time.h>
#include <stdlib.h> // system, fd_set
#include <string.h>
#include "server_utils.h"
#include "../request_codes.h"
#include "../common_utils.h"
#include "server_utils.h"

#include "Rooms/room_structures.h"
#include "Rooms/room_data.h"


void printRoomheader(int roomIndex){
      printOrario();
      printf(" <#%03d>", roomIndex);
      printf(" [%04X] ", getpid());
}

/**
 * Cerca un oggetto con nome specificato nella tabella LOCATIONS
 * @param roomIndex indice della stanza gestita
 * @param Name nome della locazione della quale si vuole conoscere l'indice nella tabella
 * @returns Ritorna l'indice della locazione nella tabella se trovata, -1 altrimenti
*/
int getLocationIndex_byName(int roomIndex, char *Name){
      int i = 0;

      // scorre tutti le locazioni della stanza
      for(; i < LOCATIONS_count[roomIndex]; i++)

            // se torva un match
            if(strcmp(LOCATIONS[roomIndex][i].name, Name) == 0)

                  // ritorna
                  return i;

      return -1;
}


/**
 * Cerca un oggetto con nome specificato nella tabella OBJECTS
 * @param roomIndex indice della stanza gestita
 * @param Name nome dell'oggetto del quale si vuole conoscere l'indice nella tabella
 * @returns Ritorna l'indice dell'oggetto nella tabella se trovato, -1 altrimenti
*/
int getObjectIndex_byName(int roomIndex, char *Name){
      int i = 0;

      // scorre tutti gli oggetti della stanza
      for(; i < OBJECTS_count[roomIndex]; i++)

            // se trova un match
            if(strcmp(OBJECTS[roomIndex][i].name, Name) == 0)

                  // ritorna
                  return i;

      return -1;
}

/**
 * Cerca un utente con fd specificato nella tabella users
 * @param users tabella di utenti loggati (struct loggedUser)
 * @param numClients numero di clienti nella tabella users
 * @param fd fd del cliente del quale si vuole conoscere l'indice nella tabella
 * @returns Ritorna l'indice del cliente nella tabella se trovato, -1 altrimenti
*/
int getPlayerIndex_byFd(struct loggedUser *users, int numClients, int fd){
      int i = 0;

      // scorro tutti i clienti
      for(i = 0; i < numClients; i++)

            // se trovo un match con il fd
            if(users[i].fd == fd)

                  // ritorno
                  return i;

      return -1;
}

/**
 * Controlla se l'inventario di un utente è pieno. Se vuoto fornisce l'indice del primo slot libero
 * @param roomIndex indice della stanza gestita
 * @param inventory Inventario da controllare (array di indice di oggetti)
 * @param free_slot_index Intero che verrà sovrascritto con l'indice del primo slot libero se esiste 
 * @returns true se l'inventario è pieno, false altrimenti
*/
bool isInventoryFull(int roomIndex, int *inventory, int *free_slot_index){
      int i = 0;

      // scorro tutto l'inventario
      for(; i < INVENTORY_DIM[roomIndex]; i++)

            // se c'è almeno uno slot libero
            if(inventory[i] == -1){

                  // ritorno l'indice attraverso il puntatore
                  *free_slot_index = i;

                  // ritorno
                  return false;
            } 

      return true;
}

enum BlockType{
      Free,
      Riddle,
      Object,
      Both,
};

/**
 * Fornisce il tipo di blocco originale su un oggetto specificato
 * @param roomIndex L'indice della stanza gestita dal room server chiamante
 * @param objectIndex L'indice dell'oggetto all'interno della tabella OBJECTS
 * @returns Ritorna il tipo di blocco come <enum BlockType> 
*/
enum BlockType getObjectBlockTypeInfo(int roomIndex, int objectIndex){
      if(OBJECTS[roomIndex][objectIndex].blockingObject == NO_BLOCKING_OBJECT){
            if(OBJECTS[roomIndex][objectIndex].blockingRiddle == NO_BLOCKING_RIDDLE)
                  return Free;
            else
                  return Riddle;
      }
      else{
            if(OBJECTS[roomIndex][objectIndex].blockingRiddle == NO_BLOCKING_RIDDLE)
                  return Object;
            else
                  return Both;
      }
}

/**
 * Fornisce il tipo di blocco corrente sun un oggetto specificato, aggiornato secondo eventuali sblocchi
 * @param roomIndex L'indice della stanza gestita dal room server chiamante
 * @param objectIndex L'indice dell'oggetto all'interno della tabella OBJECTS
 * @returns Ritorna il tipo di blocco come <enum BlockType> 
*/
enum BlockType getObjectBlockTypeCurrent(int roomIndex, int objectIndex){
      if(OBJECTS[roomIndex][objectIndex].objectUnlocked){
            if(OBJECTS[roomIndex][objectIndex].riddleUnlocked)
                  return Free;
            else
                  return Riddle;
      }
      else{
            if(OBJECTS[roomIndex][objectIndex].riddleUnlocked)
                  return Object;
            else
                  return Both;
      }
}

bool isObjectUnlocked(int roomIndex, int objectIndex){
      return OBJECTS[roomIndex][objectIndex].riddleUnlocked && OBJECTS[roomIndex][objectIndex].objectUnlocked;
}

void ROOM_SERVER(int roomIndex, struct loggedUser *users){
      int ret, i, j;

      // ********************************************** Variabili iniziali *********************************************

      // *************** Strutture necessarie per l'IO multiplexing

      fd_set master; // set di descrittori di file gestito del programmatore
      fd_set fds_read; // set usato dalla select
      int fd, fdmax = 0;

      // pulisco e inizializzo i set di descrittori di file
      FD_ZERO(&master);
      FD_ZERO(&fds_read);

      // inserisco i clienti
      for(i = 0; i < PLAYERS_PER_GAME[roomIndex]; i++){
            FD_SET(users[i].fd, &master);
            if(fdmax < users[i].fd) fdmax = users[i].fd;
      }

      // tempo massimo per una partita
      struct timeval elapsed;
      elapsed.tv_sec = 0;
      elapsed.tv_usec = 0;

      // variabili per salvare gli istanti temporali di esecuzione della select e di terminazione
      struct timeval last_iteration_time, current_iteration_time;


      // *************** Strutture necessarie per la gestione del gioco

      int remaining_players = PLAYERS_PER_GAME[roomIndex];

      // inventario dei clienti
      int inventories[PLAYERS_PER_GAME[roomIndex]][INVENTORY_DIM[roomIndex]];

      // token totali presi
      int token_total = 0;

      // inizializzo gli inventari come vuoti
      for(i = 0; i < PLAYERS_PER_GAME[roomIndex]; i++)
            for(j = 0; j < INVENTORY_DIM[roomIndex]; j++)
                  inventories[i][j] = -1;

      bool game_over = false;
      bool game_win = false;


      // ************************************************ Fase iniziale ************************************************
      
      // ****************** Log della creazione del room server 
      printRoomheader(roomIndex);
      printf("NEW ROOM_SERVER: room:<#%03d> pid:[%04X]\n", roomIndex, roomIndex);

      printRoomheader(roomIndex);
      printf("%d clienti in questa partita: ", PLAYERS_PER_GAME[roomIndex]);
      for(i = 0; i < PLAYERS_PER_GAME[roomIndex]; i++)
            printf("%d (%s), ", users[i].fd, users[i].username);
      printf("\n");

      fflush(stdout);

      // ****************** Comunicazioni della fase iniziale
      // Si invano tutte le informazioni specificate nel protocollo di COMMUNICATION_STARTING_GAME
      
      // Invio ai client i codici di fork avvenuta con successo
      for(i = 0; i < PLAYERS_PER_GAME[roomIndex]; i++)
            (void) sendCodeTCP(users[i].fd, COMMUNICATION_STARTING_GAME, false);

      // Numero di partecipanti (intero in Text protocol)
      char numClientsString[8];
      sprintf(numClientsString, "%d", PLAYERS_PER_GAME[roomIndex]);

      // Numero di token totali (intero in Text protocol)
      char tokenTotalString[8];
      sprintf(tokenTotalString, "%d", TOKEN_GOAL[roomIndex]);

      // Durata della partita (intero in Text protocol)
      char gameDurationString[8];
      sprintf(gameDurationString, "%d", GAME_DURATION[roomIndex]);

      // Invio tutte le informazioni necessarie da protocollo COMMUNICATION_STARTING_GAME
      for(i = 0; i < PLAYERS_PER_GAME[roomIndex]; i++){

            // Invio il numero di token necessari per la vittoria
            (void) sendMessageTCP(users[i].fd, tokenTotalString, false);

            // Invio il numero di partecipanti totali
            (void) sendMessageTCP(users[i].fd, numClientsString, false);

            // Invio gli username (evito di inviare il proprio nome)
            for(j = 0; j < PLAYERS_PER_GAME[roomIndex]; j++) if(i != j)
                  (void) sendMessageTCP(users[i].fd, users[j].username, false);

      }

      // Invio la durata della partita in secondi
      // Lo faccio alla fine per evitare che ci sia troppo delay tra il primo client
      // che ha ricevuto tutti i messaggi iniziali e l'ultimo: questo potrebbe portare, 
      // nel caso di una partita con molti partecipanti, ad avere il primo client che inizia
      // ad inviare richiesta prima che il room server abbia terminato questa fase iniziale,
      // rendendo l'applicazione meno responsiva.
      // (Se mettessi tutto in un unico ciclo il numero di messaggi inviati tra primo e ultimo client sarebbe O(n^2))
      for(i = 0; i < PLAYERS_PER_GAME[roomIndex]; i++)
            (void) sendMessageTCP(users[i].fd, gameDurationString, false);


      // ****************************************** Ciclo principale di esecuzione *************************************

      // ciclo finchè il gioco è in corso e sono rimasti giocatori
      while(!game_over && remaining_players > 0) {

            fds_read = master;

            // ricalcolo il tempo di attesa
            struct timeval total_time;
            if(elapsed.tv_usec != 0){
                  total_time.tv_sec = GAME_DURATION[roomIndex] - elapsed.tv_sec - 1; // prendo uno per darlo ai microsecondi
                  total_time.tv_usec = 1000000 - elapsed.tv_usec; // prendo uno per darlo ai microsecondi
            }
            else{
                  total_time.tv_sec = GAME_DURATION[roomIndex] - elapsed.tv_sec;
                  total_time.tv_usec = 0; 
            }

            // Salvo l'istante in cui chiamo la select (e quindi quando parte il timer)
            gettimeofday(&last_iteration_time, NULL);
            
            int fd_ready_count = select(fdmax + 1, &fds_read, NULL, NULL, &total_time);

            // ***************************************** Gameover ******************************************************
            
            // Se la select è andata in timeout allora il tempo è finito e la partita è terminata
            if(fd_ready_count == 0){

                  game_over = true;

                  printRoomheader(roomIndex);
                  printf("Gameover for timeout\n");
                  fflush(stdout);

                  break; // esco dal ciclo della gestione del gioco
            }

            // *************************************** Socket pronto ***************************************************
            
            for(fd = 0; fd <= fdmax; fd++) if(FD_ISSET(fd, &fds_read)){

                  char request_code;

                  ret = recvCodeTCP(fd, &request_code, false);

                  if(ret <= 0){
                        // se il socket è chiuso o c'è stato un errore lo toglo dalla lista master e segnalo la disconnessione
                        FD_CLR(fd, &master);
                        remaining_players--;
                        users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;

                        printf("CLIENT CRASHED\n");
                        break;
                  }

                  // tengo traccia di eventuali token assegnati da una richiesta
                  // Mi permette di gestire l'assegnamento dei token in una singola parte di codice dopo lo switch()
                  int given_token = 0;
                  
                  // rispondo alla richiesta del client
                  switch(request_code){

                  // Richiesta di look
                  case REQUEST_LOOK: {

                        char name[BUF_SIZE_XS];

                        // leggo il nome della locazione / oggetto
                        ret = recvMessageTCP(fd, name, false);

                        if(ret <= 0){
                              // se il socket è chiuso o c'è stato un errore lo toglo dalla listamaster
                              FD_CLR(fd, &master);
                              remaining_players--;
                              users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;
                              break;
                        }

                        printRoomheader(roomIndex);
                        printf("[%04d] REQUEST_LOOK: <%s>", fd, name);

                        int index = getLocationIndex_byName(roomIndex, name);

                        // locazione trovata
                        if(index != -1){

                              // Invio OK
                              (void) sendCodeTCP(fd, RESPONSE_LOOK_OK, false);
                              
                              // Invio descrizione
                              (void) sendMessageTCP(fd, LOCATIONS[roomIndex][index].description, false);

                              printf(" - location, index %d\n", index);
                              fflush(stdout);

                              break;
                        }

                        // Locazione non trovata, cerco tra gli oggetti
                        index = getObjectIndex_byName(roomIndex, name);

                        // nome non trovato nè in locazioni nè in oggetti
                        if(index == -1){
                              (void) sendCodeTCP(fd, RESPONSE_LOOK_ERR, false);

                              printf(" - not found\n");
                              fflush(stdout);
                              
                              break;
                        }

                        // Trovato tra gli oggetti

                        // Controllo che l'oggetto non sia stato preso / sia nell'inventario del richiedente
                        bool look_object_feasible = false;

                        // se l'oggetto non è collezionabile posso sempre fare la look
                        if(!OBJECTS[roomIndex][index].collectible)
                              look_object_feasible = true;

                        // altrimenti devo controllare che sia nell'inventario del richiedente
                        else{

                              // recupero l'indice dell'utente chiamante
                              int user_index = getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd);
                              if(user_index == -1){
                                    printRoomheader(roomIndex);
                                    printf("\nERROR getPlayerIndex_byFd returned -1\n");
                                    fflush(stdout);

                                    // se non lo trovo l'errore non è recuperabile,
                                    // chiudo il socket per evitare altri problemi
                                    FD_CLR(fd, &master);
                                    remaining_players--;
                                    users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;
                                    break;
                              }

                              // scorro l'inventario dell'utente cercando l'oggetto
                              for(i = 0; i < INVENTORY_DIM[roomIndex]; i++){

                                    int object_index = inventories[user_index][i];

                                    if(object_index == -1) continue;

                                    // se trovo un match
                                    if(strcmp(OBJECTS[roomIndex][object_index].name, name) == 0){

                                          // la look è possibile
                                          look_object_feasible = true;

                                          // esco dal for
                                          break;
                                    }
                              }
                        }

                        if(!look_object_feasible){
                              // Invio codice di risposta
                              (void) sendCodeTCP(fd, RESPONSE_LOOK_not_poss, false);

                              printf(" - object not possessed\n");
                              fflush(stdout);
                              
                              break;
                        }

                        // Invio OK
                        (void) sendCodeTCP(fd, RESPONSE_LOOK_OK, false);

                        // controllo se l'oggetto è bloccato o meno
                        bool object_unlocked = isObjectUnlocked(roomIndex, index);

                        // Invio la corretta descrizione
                        (void) sendMessageTCP(fd, OBJECTS[roomIndex][index].description[object_unlocked], false);

                        printf(" - object\n");
                        fflush(stdout);
                        
                        break;
                  }


                  // Richiesta di Take
                  case REQUEST_TAKE: {

                        char name[BUF_SIZE_XS];

                        // leggo il nome dell' oggetto
                        ret = recvMessageTCP(fd, name, false);

                        if(ret <= 0){
                              // se il socket è chiuso o c'è stato un errore lo toglo dalla lista master
                              FD_CLR(fd, &master);
                              remaining_players--;
                              users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;
                              break;
                        }

                        printRoomheader(roomIndex);
                        printf("[%04d] REQUEST_TAKE: <%s>", fd, name);

                        int object_index = getObjectIndex_byName(roomIndex, name);

                        // nome non trovato 
                        if(object_index == -1){
                              (void) sendCodeTCP(fd, RESPONSE_TAKE_ERR, false);

                              printf(" - not found\n");
                              fflush(stdout);
                              
                              break;
                        }

                        // recupero l'indice dell'utente
                        int user_index = getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd);
                        if(user_index == -1){

                              (void) sendCodeTCP(fd, RESPONSE_TAKE_ERR, false);
                              
                              printRoomheader(roomIndex);
                              printf("\nERROR getPlayerIndex_byFd returned -1\n");
                              fflush(stdout);

                              // se non lo trovo  l'errore non è recuperabile,
                              // chiudo il socket per evitare altri problemi
                              FD_CLR(fd, &master);
                              remaining_players--;
                              users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;
                              break;
                        }

                        // controllo che l'oggetto non sia stato già preso
                        if(OBJECTS[roomIndex][object_index].collected){

                              // invio il codice di risposta
                              (void) sendCodeTCP(fd, RESPONSE_TAKE_alr_taken, false);

                              // log
                              printf(" - already taken\n");
                              fflush(stdout);

                              break;
                        }

                        // controllo che l'oggetto sia collezionabile
                        if(!OBJECTS[roomIndex][object_index].collectible){

                              // invio il codice di risposta
                              (void) sendCodeTCP(fd, RESPONSE_TAKE_not_coll, false);

                              // log
                              printf(" - not collectible\n");
                              fflush(stdout);

                              break;
                        }


                        // Oggetto trovato, in base al tipo di blocco corrente faccio azioni diverse
                        enum BlockType block_type = getObjectBlockTypeCurrent(roomIndex, object_index);
                        switch (block_type)
                        {

                        case Free: {

                              // Controlla che l'inventario non sia pieno
                              int free_slot_index;
                              if(isInventoryFull(roomIndex, inventories[user_index], &free_slot_index)){

                                    // invio il codice di risposta
                                    (void) sendCodeTCP(fd, RESPONSE_TAKE_inv_full, false);

                                    // log
                                    printf(" - inventory full\n");
                                    fflush(stdout);

                                    break;
                              }

                              // assegno l'oggetto all'inventario dell'utente
                              inventories[user_index][free_slot_index] = object_index;

                              // assegno eventuali token
                              given_token += OBJECTS[roomIndex][object_index].token;

                              // e azzero quelli dell'oggetto, per evitare che possa essere lasciato e ripreso
                              OBJECTS[roomIndex][object_index].token = 0;

                              // segnalo l'oggetto come preso
                              OBJECTS[roomIndex][object_index].collected = true;

                              // invio il codice di risposta
                              (void) sendCodeTCP(fd, RESPONSE_TAKE_OK, false);

                              // log
                              printf(" - free\n");
                              fflush(stdout);

                              break;
                        }

                        case Object: {

                              // invio il codice di risposta
                              (void) sendCodeTCP(fd, RESPONSE_TAKE_blocked, false);

                              // log
                              printf(" - blocked: access denied\n");
                              fflush(stdout);
                              
                              break;
                        }
                        
                        case Both: // Nel caso di blocco doppio faccio innanzitutto risolvere l'enigma 
                        case Riddle: {
                              // nb: non blocco l'oggetto soggetto ad enigma nell'istante di richiesta,
                              // quindi permetto l'accesso all'enigma potenzialemente a più utenti
                              // Quando un utente lo risolve e prende l'oggetto anche l'enigma diventa non più disponibile,
                              // ma almeno un utente ha la risposta corretta, quindi l'accesso alla risposta non è perso.
                              // Questo favorisce anche la cooperazione tra utenti, nel caso in cui un'informazione 
                              // contenuta nell'enigma sia necessaria in qualche altro campo dell'escape room

                              int riddle_index = OBJECTS[roomIndex][object_index].blockingRiddle;

                              // A seconda del tipo di enigma bloccante invio due codici distinti
                              // Enigma a risposta singola
                              if(RIDDLES[roomIndex][riddle_index].riddleType == SingleAnswer){

                                    // log
                                    printf(" - riddle single answer, index = %d\n", riddle_index);
                                    fflush(stdout);

                                    // invio il codice di risposta
                                    (void) sendCodeTCP(fd, RESPONSE_TAKE_riddle_ans, false);
                              }

                              // Enigma a sequenza di oggetti
                              // Nota: E' abbastanza inefficiente questa procedura di controllo per questo tipo di enigma
                              // si potrrebbe tranquillamente fare in questa parte di codice evitando l'invio di tre messaggi
                              // L'ho lasciato per due motivi:
                              // 1) Uniformità con l'altro tipo di enigma già presente (e quindi anche migliore leggibilità)
                              // 2) Modularità del codice: nelle specifiche di progetto sono menzionati vari tipi di enigmi, io
                              //    ho realizzato solo queste due macrocategorie ma con il codice così strutturato è molto semplice
                              //    aggiungere una nuova tipologia creando un nuovo codice di risposta e del codice di gestione e 
                              //    analisi della stessa a fianco di quelle due già presenti
                              else{

                                    // log
                                    printf(" - riddle object sequence, index = %d\n", riddle_index);
                                    fflush(stdout);

                                    // invio il codice di risposta
                                    (void) sendCodeTCP(fd, RESPONSE_TAKE_riddle_seq, false);
                              }

                              // Invio l'indice in text protocol
                              char riddle_identifier[8];
                              sprintf(riddle_identifier, "%d", riddle_index);
                              (void) sendMessageTCP(fd, riddle_identifier, false);

                              // invio l'id dell'oggetto
                              char object_identifier[8];
                              sprintf(object_identifier, "%d", object_index);
                              (void) sendMessageTCP(fd, object_identifier, false);

                              // Invio il testo dell'enigma
                              (void) sendMessageTCP(fd, RIDDLES[roomIndex][riddle_index].text, false);

                              break;
                        }

                        default: {

                              // invio il codice di risposta
                              (void) sendCodeTCP(fd, RESPONSE_TAKE_ERR, false);
                              break;
                        }

                        }

                        break;
                  
                  }


                  // Richiesta di Use object
                  case REQUEST_USE_ONE: {

                        char object_name[BUF_SIZE_XS];

                        // leggo il nome dell'oggetto
                        ret = recvMessageTCP(fd, object_name, false);

                        if(ret <= 0){
                              // se il socket è chiuso o c'è stato un errore lo toglo dalla listam aster
                              FD_CLR(fd, &master);
                              remaining_players--;
                              users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;
                              break;
                        }

                        printRoomheader(roomIndex);
                        printf("[%04d] REQUEST_USE_ONE: <%s>", fd, object_name);

                        int object_index = getObjectIndex_byName(roomIndex, object_name);

                        // oggetto non trovato 
                        if(object_index == -1){
                              (void) sendCodeTCP(fd, RESPONSE_USE_ERR, false);

                              printf(" - not found\n");
                              fflush(stdout);
                              
                              break;
                        }

                        bool use_permitted = false;

                        // controllo che il giocatore possieda l'oggetto se collezionabile
                        if(OBJECTS[roomIndex][object_index].collectible){

                              // recupero l'indice dell'utente chiamante
                              int user_index = getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd);
                              if(user_index == -1){

                                    (void) sendCodeTCP(fd, RESPONSE_USE_ERR, false);
                                    
                                    printRoomheader(roomIndex);
                                    printf("\nERROR getPlayerIndex_byFd returned -1\n");
                                    fflush(stdout);

                                    // se non lo trovo  l'errore non è recuperabile,
                                    // chiudo il socket per evitare altri problemi
                                    FD_CLR(fd, &master);
                                    remaining_players--;
                                    users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;
                                    break;
                              }

                              // scorro l'inventario dell'utente cercando l'oggetto
                              for(i = 0; i < INVENTORY_DIM[roomIndex]; i++){

                                    int object_index_swipe = inventories[user_index][i];

                                    if(object_index_swipe == -1) continue;

                                    // se trovo un match
                                    if(strcmp(OBJECTS[roomIndex][object_index_swipe].name, object_name) == 0){
                                          use_permitted = true;
                                          break;
                                    }
                              }
                        }
                        else use_permitted = true;

                        // use non ammessa 
                        if(!use_permitted){
                              (void) sendCodeTCP(fd, RESPONSE_USE_denied, false);

                              printf(" - use denied\n");
                              fflush(stdout);
                              
                              break;
                        }

                        // controllo se l'oggetto è parte di una sequenza di un enigma
                        if(OBJECTS[roomIndex][object_index].partOfRiddle == NO_PART_OF_RIDDLE){

                              (void) sendCodeTCP(fd, RESPONSE_USE_no_effect, false);

                              printf(" - not part of riddle\n");
                              fflush(stdout);
                              
                              break;
                        }

                        // altrimenti aggiorno la sequenza dell'enigma
                        int riddle_index = OBJECTS[roomIndex][object_index].partOfRiddle;
                        struct SequenceRiddle *riddle_structures = &RIDDLES[roomIndex][riddle_index].structures.sequenceRiddle;

                        // aggiorno la sequenza
                        riddle_structures->currentSequence[riddle_structures->currentIndex] = object_index;

                        // aggiorno la posizione nella sequenza
                        riddle_structures->currentIndex = (riddle_structures->currentIndex + 1) % riddle_structures->objectCounter;

                        // invio il codice di risposta
                        (void) sendCodeTCP(fd, RESPONSE_USE_OK, false);

                        printf(" - OK, riddle %d\n", riddle_index);
                        fflush(stdout);

                              
                        break;
                  }


                  // Richiesta di Use object1 object2
                  case REQUEST_USE_TWO: {

                        char object1_name[BUF_SIZE_XS];
                        char object2_name[BUF_SIZE_XS];

                        // leggo il nome degli oggett1
                        if(recvMessageTCP(fd, object1_name, false) <= 0
                        || recvMessageTCP(fd, object2_name, false) <= 0){
                              // se il socket è chiuso o c'è stato un errore lo toglo dalla listam aster
                              FD_CLR(fd, &master);
                              remaining_players--;
                              users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;
                              break;
                        }

                        // log
                        printRoomheader(roomIndex);
                        printf("[%04d] REQUEST_USE_TWO: <%s on %s>", fd, object1_name, object2_name);

                        int object1_index = getObjectIndex_byName(roomIndex, object1_name);
                        int object2_index = getObjectIndex_byName(roomIndex, object2_name);

                        // oggetto non trovato 
                        if(object1_index == -1
                        || object2_index == -1){
                              (void) sendCodeTCP(fd, RESPONSE_USE_ERR, false);

                              printf(" - not found\n");
                              fflush(stdout);
                              
                              break;
                        }

                        bool use_permitted = false;

                        // controllo che il giocatore possieda l'oggetto 1 se collezionabile
                        if(OBJECTS[roomIndex][object1_index].collectible){

                              // recupero l'indice dell'utente chiamante
                              int user_index = getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd);
                              if(user_index == -1){

                                    (void) sendCodeTCP(fd, RESPONSE_USE_ERR, false);
                                    
                                    printRoomheader(roomIndex);
                                    printf("\nERROR getPlayerIndex_byFd returned -1\n");
                                    fflush(stdout);

                                    // se non lo trovo  l'errore non è recuperabile,
                                    // chiudo il socket per evitare altri problemi
                                    FD_CLR(fd, &master);
                                    remaining_players--;
                                    users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;
                                    break;
                              }

                              // scorro l'inventario dell'utente cercando l'oggetto
                              for(i = 0; i < INVENTORY_DIM[roomIndex]; i++){

                                    int object_index_swipe = inventories[user_index][i];

                                    if(object_index_swipe == -1) continue;

                                    // se trovo un match
                                    if(strcmp(OBJECTS[roomIndex][object_index_swipe].name, object1_name) == 0){
                                          use_permitted = true;
                                          break;
                                    }
                              }
                        }
                        else use_permitted = true;

                        // use non ammessa 
                        if(!use_permitted){
                              (void) sendCodeTCP(fd, RESPONSE_USE_denied, false);

                              // log
                              printf(" - use denied\n");
                              fflush(stdout);
                              
                              break;
                        }

                        // Use ammessa

                        // Controllo se il secondo oggetto è bloccato dal primo
                        if(OBJECTS[roomIndex][object2_index].blockingObject == NO_BLOCKING_OBJECT // se non è bloccato
                        || OBJECTS[roomIndex][object2_index].blockingObject != object1_index){ // oppure se è bloccato ma non dall'oggetto 1
                        
                              (void) sendCodeTCP(fd, RESPONSE_USE_no_effect, false);

                              // log
                              printf(" - no change\n");
                              fflush(stdout);
                              
                              break;
                        }

                        // Sblocco il secondo oggetto
                        OBJECTS[roomIndex][object2_index].objectUnlocked = true;

                        // log
                        printf(" - unlocked\n");
                        fflush(stdout);

                        // invio il codice di risposta
                        (void) sendCodeTCP(fd, RESPONSE_USE_OK, false);
                              
                        break;
                  }

                  
                  // Richiesta di Drop
                  case REQUEST_DROP: {

                        char object_name[BUF_SIZE_XS];

                        // leggo il nome dell'oggetto
                        ret = recvMessageTCP(fd, object_name, false);

                        if(ret <= 0){
                              // se il socket è chiuso o c'è stato un errore lo toglo dalla listam aster
                              FD_CLR(fd, &master);
                              remaining_players--;
                              users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;
                              break;
                        }
                        
                        printRoomheader(roomIndex);
                        printf("[%04d] REQUEST_DROP: <%s>", fd, object_name);

                        // recupero l'indice dell'utente
                        int user_index = getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd);
                        if(user_index == -1){
                              printRoomheader(roomIndex);
                              printf("\nERROR getPlayerIndex_byFd returned -1\n");
                              fflush(stdout);

                              // se non lo trovo  l'errore non è recuperabile,
                              // chiudo il socket per evitare altri problemi
                              FD_CLR(fd, &master);
                              remaining_players--;
                              users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;
                              break;
                        }

                        bool object_found = false;
                        int object_index;

                        // scorro l'inventario dell'utente cercando l'oggetto
                        for(i = 0; i < INVENTORY_DIM[roomIndex]; i++){

                              object_index = inventories[user_index][i];

                              if(object_index == -1) continue;

                              // se trovo un match
                              if(strcmp(OBJECTS[roomIndex][object_index].name, object_name) == 0){
                                    
                                    // tolgo l'oggetto
                                    inventories[user_index][i] = -1;

                                    object_found = true;

                                    printf(" - OK, index was %d\n", i);
                                    fflush(stdout);

                                    break;
                              }
                        }
                        
                        if(object_found){
                              // Comunico al client il successo dell'operazione
                              (void) sendCodeTCP(fd, RESPONSE_DROP_OK, false);

                              // segnalo l'oggetto come disponibile
                              OBJECTS[roomIndex][object_index].collected = false;
                        }
                        else {
                              // Comunico al client il fallimento dell'operazione
                              (void) sendCodeTCP(fd, RESPONSE_DROP_ERR, false);
                              
                              printf(" - object not found\n");
                              fflush(stdout);
                        }

                        break;
                  }


                  // Richiesta di riposta ad un enigma a risposta singola
                  case REQUEST_RIDDLE_ANS: {

                        char riddle_identifier[8];
                        char object_identifier[8];
                        char answer[BUF_SIZE_XS];
                        int riddle_index;
                        int object_index;

                        // leggo l'id dell'enigma, l'id dell'oggetto e la risposta proposta
                        if(recvMessageTCP(fd, riddle_identifier, false) <= 0
                        || recvMessageTCP(fd, object_identifier, false) <= 0
                        || recvMessageTCP(fd, answer, false) <= 0){
                              // se il socket è chiuso o c'è stato un errore lo toglo dalla lista master
                              FD_CLR(fd, &master);
                              remaining_players--;
                              users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;
                              break;
                        }

                        // riporto i due indici come interi
                        sscanf(riddle_identifier, "%d", &riddle_index);
                        sscanf(object_identifier, "%d", &object_index);

                        // controllo che la risposta sia corretta
                        bool riddle_solved = strcmp(RIDDLES[roomIndex][riddle_index].structures.answer, answer) == 0;
                        
                        // log
                        printRoomheader(roomIndex);
                        printf("[%04d] REQUEST_RIDDLE (single answer): <%d> - %s\n", fd, riddle_index, riddle_solved ? "correct" : "wrong");
                        fflush(stdout);
                        
                        // Se l'enigma non è stato risolto lo comunico
                        if(!riddle_solved){
                              (void) sendCodeTCP(fd, RESPONSE_RIDDLE_ERR, false);
                              break;
                        }

                        // Altrimenti segno l'oggetto come sbloccato dall'enigma
                        OBJECTS[roomIndex][object_index].riddleUnlocked = true;

                        // assegno eventuali token dell'enigma
                        given_token += RIDDLES[roomIndex][riddle_index].token;

                        // e azzero quelli dell'enigma, per evitare che possa essere nuovamente chiesto e ottenuto
                        // nb: al momento gli enigmi sono forniti solo se l'enigma non è stato risolto, quindi non si possono avere più volte gli stessi token di un enigma
                        RIDDLES[roomIndex][riddle_index].token = 0;

                        // E comunico il successo dell'operazione
                        (void) sendCodeTCP(fd, RESPONSE_RIDDLE_OK, false);
                        
                        break;
                  }


                  // Richiesta di riposta ad un enigma di tipo sequenza di oggetti
                  case REQUEST_RIDDLE_SEQ: {

                        char riddle_identifier[8];
                        char object_identifier[8];
                        int riddle_index;
                        int object_index;

                        // leggo l'id dell'enigma e l'id dell'oggetto
                        if(recvMessageTCP(fd, riddle_identifier, false) <= 0
                        || recvMessageTCP(fd, object_identifier, false) <= 0){
                              // se il socket è chiuso o c'è stato un errore lo toglo dalla lista master
                              FD_CLR(fd, &master);
                              remaining_players--;
                              users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;
                              break;
                        }

                        // riporto i due indici come interi
                        sscanf(riddle_identifier, "%d", &riddle_index);
                        sscanf(object_identifier, "%d", &object_index);

                        bool riddle_solved = false;

                        struct SequenceRiddle *riddle_structures = &RIDDLES[roomIndex][riddle_index].structures.sequenceRiddle;
                        
                        // controllo che la sequenza sia corretta
                        int sequence_len = riddle_structures->objectCounter;

                        riddle_solved = true;
                        // scorro tutti gli elementi della sequenza
                        for(i = 0; i < sequence_len; i++){
                              
                              // fino a trovarne almeno uno errato
                              if(riddle_structures->objectsSequence[i] != riddle_structures->currentSequence[i]){
                                    riddle_solved = false;
                                    break;
                              }
                        }

                        // log
                        printRoomheader(roomIndex);
                        printf("[%04d] REQUEST_RIDDLE (object sequence): <%d> - %s\n", fd, riddle_index, riddle_solved ? "solved" : "wrong");
                        fflush(stdout);

                        // azzero in ogni caso la sequenza
                        for(i = 0; i < sequence_len; i++)
                              riddle_structures->currentSequence[i] = -1;
                        riddle_structures->currentIndex = 0;

                        // Se l'enigma non è stato risolto lo comunico
                        if(!riddle_solved){
                              (void) sendCodeTCP(fd, RESPONSE_RIDDLE_ERR, false);
                              break;
                        }

                        // Altrimenti segno l'oggetto come sbloccato dall'enigma
                        OBJECTS[roomIndex][object_index].riddleUnlocked = true;

                        // assegno eventuali token dell'enigma
                        given_token += RIDDLES[roomIndex][riddle_index].token;

                        // e azzero quelli dell'enigma, per evitare che possa essere nuovamente chiesto e ottenuto
                        // nb: al momento gli enigmi sono forniti solo se l'enigma non è stato risolto, quindi non si possono avere più volte gli stessi token di un enigma
                        RIDDLES[roomIndex][riddle_index].token = 0;

                        // E comunico il successo dell'operazione
                        (void) sendCodeTCP(fd, RESPONSE_RIDDLE_OK, false);

                        break;
                  }


                  // Richiesta di mostrare l'inventario
                  case REQUEST_OBJS: {

                        // log
                        printRoomheader(roomIndex);
                        printf("[%04d] REQUEST_OBJS\n", fd);
                        fflush(stdout);

                        // recupero l'indice dell'utente
                        int user_index = getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd);
                        if(user_index == -1){
                              
                              printRoomheader(roomIndex);
                              printf("\nERROR getPlayerIndex_byFd returned -1\n");
                              fflush(stdout);

                              // se non lo trovo  l'errore non è recuperabile,
                              // chiudo il socket per evitare altri problemi
                              FD_CLR(fd, &master);
                              remaining_players--;
                              users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;
                              break;
                        }

                        // costruisco una stringa con tutti i nomi degli oggetti che possiede
                        char buffer[BUF_SIZE_S];
                        int start = 0;

                        // scorro tutto l'inventario
                        for(i = 0; i < INVENTORY_DIM[roomIndex]; i++){

                              // se non e' presente alcun oggetto stampo "/"
                              if(inventories[user_index][i] == -1){
                                    buffer[start] = '/';
                                    buffer[start + 1] = '\n';
                                    buffer[start + 2] = '\0';
                                    start += 2;
                              }

                              else{
                                    // stampo il nome dell'oggetto nel buffer
                                    int object_index = inventories[user_index][i];
                                    sprintf(buffer + start, "%s\n", OBJECTS[roomIndex][object_index].name);

                                    // Aggiorno start e non includo \0, 
                                    // lo voglio sovrascrivere alla prossima iterazione
                                    // il +1 è relativo al \n inserito
                                    start += strlen(OBJECTS[roomIndex][object_index].name) + 1;
                              }
                        }

                        // il marcatore di fine stringa è già presente dall'ultima iterazione

                        // invio l'inventario
                        (void) sendMessageTCP(fd, buffer, false);

                        break;
                  }


                  // Richiesta di messaggiare un partecipante
                  case REQUEST_MSG: {

                        char receiver_username[USER_USERNAME_FIELD_DIM];
                        char message[BUF_SIZE];

                        // leggo lo username e il messaggio
                        if(recvMessageTCP(fd, receiver_username, false) <= 0
                        || recvMessageTCP(fd, message, false) <= 0){
                              // se il socket è chiuso o c'è stato un errore lo toglo dalla listam aster
                              FD_CLR(fd, &master);
                              remaining_players--;
                              users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;
                              break;
                        }

                        // log
                        printRoomheader(roomIndex);
                        printf("[%04d] REQUEST_MSG: <%s>", fd, receiver_username);
                        fflush(stdout);

                        // Se è una MSG in broadcast
                        if(strcmp(receiver_username, "*") == 0){

                              // log
                              printf(" - broadcast\n");
                              fflush(stdout);

                              // ricavo lo username del mittente per comunicarlo ai destinatari
                              int sender_index = getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd);

                              // scorro tutti i clienti
                              for(i = 0; i < PLAYERS_PER_GAME[roomIndex]; i++){
                                    
                                    // invio solo a quelli loggati e non al mittente
                                    if((users[i].fd != fd) && users[i].logged){

                                          // invio il messaggio secondo protocollo
                                          (void) sendCodeTCP(users[i].fd, COMMUNICATION_MSG, false);
                                          (void) sendMessageTCP(users[i].fd, users[sender_index].username, false);
                                          (void) sendMessageTCP(users[i].fd, message, false);
                                    }

                                    fflush(stdout);
                              }
                              
                              // invio il codice di risposta al mittente
                              (void) sendCodeTCP(fd, RESPONSE_MSG_OK, false);

                              break;
                        }

                        // Altrimenti è una MSG diretta verso <receiver_username>
                        int received_fd = -1;

                        // Cerco l'utente
                        for(i = 0; i < PLAYERS_PER_GAME[roomIndex]; i++){
                              if(strcmp(users[i].username, receiver_username) == 0){
                                    received_fd = users[i].fd;
                                    break;
                              }
                        }

                        if(received_fd == -1){

                              // invio il codice di risposta
                              (void) sendCodeTCP(fd, RESPONSE_MSG_ERR, false);

                              // log
                              printf(" - not found\n");
                              fflush(stdout);

                              break;
                        }

                        // se il mittente è anche destinatario
                        if(received_fd == fd){

                              // invio il codice di risposta
                              (void) sendCodeTCP(fd, RESPONSE_MSG_self, false);

                              // log
                              printf(" - self msg\n");
                              fflush(stdout);

                              break;
                        }

                        // se il destinatario si è disconnesso
                        if(!users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], received_fd)].logged){

                              // invio il codice di risposta
                              (void) sendCodeTCP(fd, RESPONSE_MSG_disconnected, false);

                              // log
                              printf(" - receiver disconnected\n");
                              fflush(stdout);

                              break;
                        }

                        // log
                        printf(" - found: %d, sending\n", received_fd);
                        fflush(stdout);

                        // ricavo lo username del mittente per comunicarlo al destinatario
                        int sender_index = getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd);

                        // invio al destinatario il messaggio secondo protocollo
                        (void) sendCodeTCP(received_fd, COMMUNICATION_MSG, false);
                        (void) sendMessageTCP(received_fd, users[sender_index].username, false);
                        (void) sendMessageTCP(received_fd, message, false);

                        // invio il codice di risposta al mittente
                        (void) sendCodeTCP(fd, RESPONSE_MSG_OK, false);

                        break;
                  }


                  // Disconnessione di un utente
                  case COMMUNICATION_DISCONNECT: {

                        // tolgo il socket dalla lista master
                        FD_CLR(fd, &master);

                        // Aggiorno il counter
                        remaining_players--;

                        // Segno il logout
                        users[getPlayerIndex_byFd(users, PLAYERS_PER_GAME[roomIndex], fd)].logged = false;

                        // stampa
                        printRoomheader(roomIndex);
                        printf("[%04d] Client disconnected\n", fd);
                        fflush(stdout);
                        break;
                  }
                  

                  default: {
                        sendCodeTCP(fd, '$', true);

                        // stampa
                        printRoomheader(roomIndex);
                        printf("[%04d] DEFAULT (%02X), sending '$': %02X\n", fd, request_code, '$');
                        fflush(stdout);
                        break;
                  }

                  } // switch(request_code)

                  
                  // Se sono stati dati token
                  if(given_token > 0){
                        // Incremento il counter totale
                        token_total += given_token;

                        // Comunico l'assegnamento del token
                        (void) sendCodeTCP(fd, COMMUNICATION_TOKEN, false);

                        char token_string[8];
                        sprintf(token_string, "%d", given_token);
                        (void) sendMessageTCP(fd, token_string, false);
                  }

                  // Se il limite di token è stato raggiunto interrompo il gioco
                  if(token_total >= TOKEN_GOAL[roomIndex]){
                        game_over = true;
                        game_win = true;
                  }

                  // azzero il counter per la prossima iterazione
                  given_token = 0;
            }

            // *********************************** Calcolo tempo passato ***********************************************
            
            // calcolo il tempo passato: lo faccio dopo aver controllato i fd pronti, così da non contare quel tempo
            // Ho scelto di farlo qui e non subito dopo la select così da non 'togliere' tempo ai client nel caso di un server sovraccarico
            gettimeofday(&current_iteration_time, NULL);

            // Tengo traccia del tempo totale scorso
            // Probabilmente si possono evitare tutti questi controlli sui tempi ma 
            // ho avuto problemi senza questi controlli e non so bene come la 
            // select gestisca eventuali microsecondi < 0 o > 1000000

            struct timeval current_elapsed;
            current_elapsed.tv_sec = (current_iteration_time.tv_sec - last_iteration_time.tv_sec);
            current_elapsed.tv_usec = (current_iteration_time.tv_usec - last_iteration_time.tv_usec);

            // aggiusto eventuali microsecondi negativi
            if(current_elapsed.tv_usec < 0){
                  current_elapsed.tv_usec += 1000000; // +1 sec
                  if(current_elapsed.tv_sec > 0) current_elapsed.tv_sec--; // -1sec
            }

            // aggiusto eventuali 'overflow' dei microsecondi
            if(current_elapsed.tv_usec >= 1000000){
                  current_elapsed.tv_sec += current_elapsed.tv_usec / 1000000;
                  current_elapsed.tv_usec %= 1000000;
            }

            elapsed.tv_sec += current_elapsed.tv_sec;
            elapsed.tv_usec += current_elapsed.tv_usec;

            if(elapsed.tv_usec >= 1000000){
                  elapsed.tv_sec += elapsed.tv_usec / 1000000;
                  elapsed.tv_usec %= 1000000;
            }

      } // while(!game_over && remaining_players > 0)


      // ********************************************** Partita terminata **********************************************

      // Comunico il risultato della partita
      if(game_win){
            // notifico solo i client rimasti alla fine della partita
            for(j = 0; j <= fdmax; j++)
                  if(FD_ISSET(j, &master)) // lo mando solo a quelli rimasti
                        (void) sendCodeTCP(j, COMMUNICATION_GAME_WIN, true);
      }
      else{
            // notifico solo i client rimasti alla fine della partita
            for(j = 0; j <= fdmax; j++)
                  if(FD_ISSET(j, &master)) // lo mando solo a quelli rimasti
                        (void) sendCodeTCP(j, COMMUNICATION_GAMEOVER, true);
      }

      // chiudo i socket con i clienti
      for(i = 0; i < PLAYERS_PER_GAME[roomIndex]; i++)
            close(users[i].fd);

      
      printRoomheader(roomIndex);
      printf("ROOM_SERVER shutting down: %s\n", game_over ? (game_win ? "GAMEWIN" : "GAMEOVER") : "NO CLIENTS REMAINING");
      fflush(stdout);

      exit(1);
}
