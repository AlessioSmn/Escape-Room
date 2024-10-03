#include "game_server.h"

#include <stdio.h> // printf
#include <stdlib.h> // exit
#include <unistd.h> // close, getpid
#include <string.h> // memset
#include <arpa/inet.h> // sockaddr_in, AF_INET, INADDR_ANY
#include <stdint.h> // tipi a dimensione fissa
#include <stdbool.h>
#include <time.h>
#include "codes.h"
#include "../request_codes.h"
#include "../common_utils.h" // BUF_SIZE, sendCodeTCP, isStringNumber, address_setting, ...
#include "server_utils.h"
#include "room_server.h"


extern const int PLAYERS_PER_GAME[ROOM_COUNT];
extern const int GAME_DURATION[ROOM_COUNT];
extern const char ROOM_DESCRIPTION[ROOM_COUNT][ROOM_DES_MAX_LEN];


void GAME_SERVER(in_port_t Game_Server_Port, int pipe_GameToMain_W, int pipe_MainToGame_R, bool redirect_stdout){
      int i, j;

      // ********************************* Inzializzazione del server di gioco ***********************************************

      // Socket di ascolto
      int listener = socket(AF_INET, SOCK_STREAM, 0);

      // imposto indirizzo e porta
      struct sockaddr_in game_server_addr;
      address_setting(&game_server_addr, Game_Server_Port, NULL, false);
      
      // bind()
      if(address_binding(&listener, &game_server_addr, true, false) == -1){

            // Comunico che la fase iniziale è terminata con un errore
            (void) sendCodePIPE(pipe_GameToMain_W, GAME_SERVER_INIT_ERR, false);
            close(listener);
            exit(1);
      }

      // listen()
      if(address_listening(&listener, 10, true, false) == -1){

            // Comunico che la fase iniziale è terminata con un errore
            (void) sendCodePIPE(pipe_GameToMain_W, GAME_SERVER_INIT_ERR, false);
            close(listener);
            exit(1);
      }

      // Comunico al main server che la fase iniziale è terminata con successo
      (void) sendCodePIPE(pipe_GameToMain_W, GAME_SERVER_INIT_OK, false);


      // ********************************* Strutture e files *****************************************************************

      // **************** Strutture per l'IO multiplexing
      
      int fd = 0, ret;

      // Set di descrittori di file
      fd_set master; // set di descrittori di file gestito del programmatore
      fd_set fds_read; // set usato dalla select
      int fdmax = 0;

      // pulisco e inizializzo i set di descrittori di file
      FD_ZERO(&master);
      FD_ZERO(&fds_read);

      // Aggiungo il descrittore della pipe in lettura ai descrittori da controllare
      FD_SET(pipe_MainToGame_R, &master);
      if(fdmax < pipe_MainToGame_R) fdmax = pipe_MainToGame_R;

      // Aggiungo il listener alla lista dei file descriptor da controllare
      FD_SET(listener, &master);
      if(fdmax < listener) fdmax = listener;

      // **************** Strutture per la gestione degli utenti

      bool stop_server = false;

      // Tiene traccia del numero dei client loggati con il GAME SERVER
      // Ha solamente scopo di debug, il controllo sul numero di client collegati per 
      // poter terminare il server o meno è gestito dal MAIN SERVER.
      int num_logged_users = 0;

      // counter dei clienti in attesa per una partita, uno per room
      int waiting_client_counter[ROOM_COUNT];
      for(i = 0; i < ROOM_COUNT; i++) 
            waiting_client_counter[i] = 0;

      // array di giocatori in attesa per una partita
      // Un array per ogni room disponibile
      struct loggedUser *waiting_client_id[ROOM_COUNT];
      for(i = 0; i < ROOM_COUNT; i++)
            waiting_client_id[i] = malloc(sizeof(struct loggedUser) * PLAYERS_PER_GAME[i]);


      // ********************************* Reindirizzamento dello standard output ********************************************

      // reindirizzamento dello stdout su file di log se possibile
      // se non è possibile non stampo la parte iniziale
      FILE *log_file;

      if(redirect_stdout){
            log_file = fopen("ServerFiles/game_server_log.txt", "a");
            if(NULL == log_file){
                  printf("Errore nell'apertura del file %s\n", "ServerFiles/game_server_log.txt");
            }
            else{
                  if(-1 == dup2(fileno(log_file), STDOUT_FILENO)){
                        printf("Errore nella redirezione dello stdout su file\n");
                  }
                  else{
                        time_t mytime = time(NULL);
                        struct tm *tmp = gmtime(&mytime);
                        printf("***** Nuova esecuzione ************\n");
                        printf("***** Indirizzo IP: INADDR_ANY ****\n");
                        printf("***** Porta: %- 10d ***********\n", Game_Server_Port);
                        printf("***** Data: %02d/%02d/%04d ************\n", tmp->tm_mday, tmp->tm_mon+1, tmp->tm_year + 1900);
                        printf("***** Ora: ");
                        printOrario();
                        printf(" *************");
                        printf("\n***** File descriptor socket listener: %d\n", listener);
                        printf("***** File descriptor dell'estremo in lettura della pipe in ingresso da main: %d\n", pipe_MainToGame_R);
                        fflush(stdout);
                  }
            }
      }


      // ********************************* Ciclo di esecuzione principale ****************************************************

      while(!stop_server){
            
            // copio il fs_set master nel set dedicato alla select
            fds_read = master;

            // chiamo la select con timeout infinito: se non ci sono fd pronti non è necessario fare nulla, quindi mi blocco
            (void) select(fdmax + 1, &fds_read, NULL, NULL, NULL);

            // scorro tutti i fd cercando quelli pronti
            for(fd = 0; fd <= fdmax; fd++) if(FD_ISSET(fd, &fds_read)){

                  // *************************************** messaggio dal Main Server ***************************************

                  if(fd == pipe_MainToGame_R) {

                        char msg_code;
                        
                        ret = recvCodePIPE(pipe_MainToGame_R, &msg_code, true);
                        // Se è persa la comunicazione con il server principale (ad esempio è stato interrotto)
                        // interrompo anche il game server, altrimenti non c'è modo di terminarlo
                        if(ret <= 0){
                              stop_server = true;
                              break;
                        }

                        switch(msg_code){

                        // messaggio di stop dal server primario
                        // interrompo il ciclo di esecuzione del server
                        case GAME_SERVER_STOP: {
                              printOrario();
                              printf(" [main] GAME_SERVER_STOP\n");

                              sendCodePIPE(pipe_GameToMain_W, GAME_SERVER_STOP_OK, true);

                              // interrompo il processo
                              stop_server = true;
                              break;
                        }
                        
                        
                        default: {
                              printOrario();
                              printf(" [main] Received code not handled: [%02X]\n", msg_code);
                              break;
                        }

                        }

                        // (controllo per primo sempre il fd della pipe_MainToGame_R)
                        // se è arrivato il comando di GAME_SERVER_STOP interrompo immediatamente l'esecuzione, 
                        // per evitare di ascoltare eventuali nuove richieste di connessioni
                        if(stop_server)
                              break;

                        continue;
                  }
                  

                  // **************************************** socket Listener pronto *****************************************
                  
                  if(fd == listener) {
                        struct sockaddr_in client_addr;
                        socklen_t addrlen;
                        int comSocket;

                        addrlen = sizeof(client_addr);

                        // nuovo socket di comunicazione
                        comSocket = accept(listener, (struct sockaddr*)&client_addr, &addrlen);

                        // aggiungo il nuovo socket alla lista master
                        FD_SET(comSocket, &master);
                        if(fdmax < comSocket) fdmax = comSocket;
                        
                        num_logged_users++;

                        printOrario();
                        printf(" [list] Nuovo client connesso: [%s:%d], totali loggati: %d, socket creato: %d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port, num_logged_users, comSocket);
                        fflush(stdout);

                        continue;
                  }
                  

                  // ****************************************** altro socket pronto ******************************************

                  else {  
                        char request_code;
                        char arg[BUF_SIZE];
                        arg[BUF_SIZE - 1] = '\0';
                        
                        // ***** leggo il codice della richiesta
                        ret = recvCodeTCP(fd, &request_code, false);

                        if(ret <= 0){
                              // se il socket è chiuso o c'è stato un errore lo toglo dalla listamaster
                              FD_CLR(fd, &master);
                              num_logged_users--;
                              break;
                        }

                        // ***** Eseguo la richiesta
                        switch(request_code){

                        // Client richiede informazioni sulle stanze
                        case REQUEST_ROOM_INFO: {

                              // Invio il numero di stanze
                              char room_count_string[8];
                              sprintf(room_count_string, "%d", ROOM_COUNT);
                              (void) sendMessageTCP(fd, room_count_string, true);

                              // Invio le descrizioni delle stanze
                              for(i = 0; i < ROOM_COUNT; i++)
                                    (void) sendMessageTCP(fd, ROOM_DESCRIPTION[i], true);

                              break;
                        }

                        // Client richiede una partita nella stanza [arg] -> roomCode
                        case REQUEST_START_GAME: {

                              int ROOM;

                              char username[USER_USERNAME_FIELD_DIM];

                              // leggo il codice della stanza richiesta e lo username dell'utente
                              if(recvMessageTCP(fd, arg, true) <= 0
                              || recvMessageTCP(fd, username, true) <= 0){

                                    // se il socket è chiuso o c'è stato un errore lo toglo dalla listamaster
                                    FD_CLR(fd, &master);
                                    num_logged_users--;
                                    break;
                              }

                              // Converto il codice della stanza a intero
                              sscanf(arg, "%d", &ROOM);

                              // se il codice della stanza non è tra quelli ammessi lo comunico
                              if(ROOM >= ROOM_COUNT){

                                    // Invio il codice
                                    (void) sendCodeTCP(fd, RESPONSE_START_ERR, true);
                              
                                    // log
                                    printOrario();
                                    printf(" [%04d] REQUEST_START_GAME: room %d - not present\n", fd, ROOM);
                                    fflush(stdout);

                                    break;
                              }

                              // metto il client in coda per la stanza
                              waiting_client_id[ROOM][waiting_client_counter[ROOM]].fd = fd;
                              strcpy(waiting_client_id[ROOM][waiting_client_counter[ROOM]].username, username);
                              waiting_client_id[ROOM][waiting_client_counter[ROOM]].logged = true;

                              // aggiorno il counter dei clienti in attesa
                              waiting_client_counter[ROOM]++;
                              
                              // Log
                              printOrario();
                              printf(" [%04d] REQUEST_START_GAME: room %d, counter = %d\n", fd, ROOM, waiting_client_counter[ROOM]);
                              fflush(stdout);

                              // se il numero di clienti in attesa non è ancora sufficiente esco dalla gestione di start
                              // tutti i client verranno 'risvegliati' dall'ultimo arrivato
                              if(waiting_client_counter[ROOM] != PLAYERS_PER_GAME[ROOM]){
                              
                                    // Codice della stanza OK, lo comunico
                                    // il client può così mettersi in attesa dell'inizio della partita
                                    (void) sendCodeTCP(fd, RESPONSE_START_OK, true);

                                    break;
                              }
                              
                              // Comunico codice della stanza OK e avvio imminente
                              (void) sendCodeTCP(fd, RESPONSE_START_OK_NOW, true);


                              // *************** Questo era l'ultimo, faccio partire la partita

                              // creo un processo per la partita
                              pid_t server_gestore;
                              server_gestore = fork();

                              if(server_gestore == 0){ // figlio ROOM_SERVER

                                    // chiudo tutto ciò che non è usato
                                    close(listener);
                                    close(pipe_GameToMain_W);
                                    close(pipe_MainToGame_R);

                                    // Faccio eseguire il codice apposito
                                    ROOM_SERVER(ROOM, waiting_client_id[ROOM]);

                                    exit(1);
                              }
                              if(server_gestore > 0){ // padre, GAME_SERVER
                                    
                                    // I clienti che entrano in partita si disconnettono dal GAME SERVER
                                    num_logged_users -= waiting_client_counter[ROOM];

                                    // resetto il counter dei client in attesa di una partita
                                    waiting_client_counter[ROOM] = 0;

                                    for(j = 0; j < PLAYERS_PER_GAME[ROOM]; j++){

                                          // tolgo i socket dalla lista master
                                          FD_CLR(waiting_client_id[ROOM][j].fd, &master);

                                          // e li chiudo
                                          close(waiting_client_id[ROOM][j].fd);
                                    }
                              }

                              // fork fallita
                              else{
                                    // log
                                    printOrario();
                                    printf("ERRORE: fork for ROOM_SERVER failed\n");
                                    fflush(stdout);

                                    // lo comunico ai client
                                    for(j = 0; j < PLAYERS_PER_GAME[ROOM]; j++)
                                          (void) sendCodeTCP(waiting_client_id[ROOM][j].fd, COMMUNICATION_START_FAILED, false);
                              }

                              break;
                        }


                        // Client annulla la richiesta di start
                        case REQUEST_START_ABORT: {

                              // ricevo anche il codice della stanza
                              int ROOM;

                              // leggo il codice della stanza richiesta
                              ret = recvMessageTCP(fd, arg, true);

                              // se il socket è chiuso o c'è stato un errore lo toglo dalla listamaster
                              if(ret <= 0){
                                    FD_CLR(fd, &master);
                                    num_logged_users--;
                                    break;
                              }

                              sscanf(arg, "%d", &ROOM);
                              printOrario();
                              printf(" [%04d] REQUEST_START_ABORT from room %d: ", fd, ROOM);

                              // controllo che il codice della stanza sia tra quelli ammissibili
                              if(ROOM >= ROOM_COUNT || ROOM < 0){
                                    printf("DENIED\n");
                                    fflush(stdout);
                                    break;
                              }
                              
                              // controllo che il mittente avesse effettivamente richiesto una partita
                              bool game_requested = false;
                              for(j = 0; j < PLAYERS_PER_GAME[ROOM]; j++){
                                    if(waiting_client_id[ROOM][j].fd == fd){
                                          game_requested = true;
                                          break;
                                    }
                              }
      
                              if(game_requested){
                                    // sposto eventuali clienti successivi indietro nella lista
                                    // così tengo la coda dei clienti in attesa unita, senza buchi
                                    int k = j + 1; // parto dal successivo
                                    for(; k < PLAYERS_PER_GAME[ROOM]; k++)
                                          waiting_client_id[ROOM][k-1] = waiting_client_id[ROOM][k];

                                    waiting_client_counter[ROOM]--;
                                    printf("GRANTED, index was %d, %d remaining\n", j, waiting_client_counter[ROOM]);
                                    fflush(stdout);
                              }
                              else{
                                    printf("DENIED\n");
                                    fflush(stdout);
                              }

                              break;
                        }

                                   
                        // Client si è disconnesso / ha effettuato il logout (gestito dal main server)
                        case COMMUNICATION_DISCONNECT: {

                              num_logged_users--;

                              // chiudo il socket
                              close(fd);

                              // lo elimino dalla lista di fd
                              FD_CLR(fd, &master);

                              printOrario();
                              printf(" [%04d] Disconnecting, remaining: %d\n", fd, num_logged_users); 
                              fflush(stdout);

                              break;
                        }


                        default: {
                              (void) sendCodeTCP(fd, 0x00, true);
                              printOrario();
                              printf(" [%04d] Request CODE not handled: [%02X]\n", fd, request_code);
                              fflush(stdout);
                              break;
                        }

                        } // switch(request_code)

                  } // else (ultimo caso del for)

            } // for(fd = 0; fd <= fdmax; fd++)

      } // while(!stop_server)


      for(i = 0; i< ROOM_COUNT; i++)
            free(waiting_client_id[i]);

      close(pipe_GameToMain_W);
      close(pipe_MainToGame_R);

      close(listener);

      FD_ZERO(&master);
      FD_ZERO(&fds_read);
      
      printOrario();
      printf(" Shutdown completed successfully\n\n");

      if(redirect_stdout)
            fclose(log_file);
      
      exit(1);
}
