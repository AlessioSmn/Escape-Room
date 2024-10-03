#include <stdio.h> // stdin, stdout, stderr
#include <stdlib.h> // system
#include <string.h> // strcmp, strlen
#include <arpa/inet.h> // sockaddr_in, AF_INET, INADDR_ANY
#include <unistd.h> // close
#include <stdint.h> // tipi a dimensione fissa
#include <sys/socket.h> 
#include <netinet/in.h>
#include <sys/time.h> // gettimeofday
#include <errno.h>
#include "../common_utils.h"
#include "../request_codes.h"
#include "msg.h"

/**
 * Controlla se un socket, di file descriptor fd, è pronto
 * @param fd il File Descriptor del socket
 * @returns true se il socket è pronto, false altrimenti
*/
bool isSocketReady(int fd) {
      fd_set read_fds;
      struct timeval timeout = {0, 0};

      FD_ZERO(&read_fds);
      FD_SET(fd, &read_fds);

      // usa una select con timeout {0, 0}, polling
      bool ready = select(fd + 1, &read_fds, NULL, NULL, &timeout) > 0;

      return ready;
}

/**
 * Controlla che il comando e gli eventuali argomenti associato al comando sia del tipo e formato corretto
 * @param cmd Il comando target
 * @param argc Il numero di argomenti passati
 * @param arg1 Il primo argomento da controllare
 * @param arg2 Il secondo argomento da controllare
 * @returns Ritorna il comando inserito se il controllo ha successo, -1 altrimenti
*/
enum COMMANDS checkCommandArgs(char *command, int argc, char* arg1, char* arg2){
      int i = 0;
      // controlla su tutti i comandi
      for(; i < NUM_COMMANDS; i++){

            // match con il comando
            if(strcmp(command, COMMANDS_name[i]) == 0){

                  // controlla che il numero di argomenti sia corretto
                  if(argc < COMMANDS_minArgs[i] || argc > COMMANDS_maxArgs[i]){
                        printf("%s\n%s\n", error_command_wrong_argc, COMMANDS_printable[i]);
                        fflush(stdout);
                        return -1;
                  }
                  // in arg è contenuto l'eventuale argomento inserito
                  break;
            }
      }

      // nessun match con i comandi disponibili, quindi il comando è errato
      if(i == NUM_COMMANDS)
            return -1;

      enum COMMANDS cmd = i;

      // ***** Controllo la correttezza del tipo e formato dei parametri
      switch(cmd){
            // start <port>         -> arg1 numero
            case START:
                  if(isStringNumber(arg1))
                        return cmd;
                  else 
                        return -1;

            // look                 -> arg1 empty
            // look <location>      -> arg1 string
            // look <object>        -> arg1 string
            case LOOK:
            case TAKE:
            case DROP:
            case USE:
            case OBJS:
            case MSG:
            case LEAVE:
            case END:
            case LOGOUT:
            case HELP:
                  return cmd;

            default:
                  return -1;
      }
}

/**
 * legge una stringa da stdin e la analizza
 * @param arg Buffer per l'eventual argomento associato al comando inserito
 * @returns Ritorna il comando inserito se non ci sono errori, -1 altrimenti
*/
enum COMMANDS readCommand(char *arg1, char* arg2){
      char buffer[BUF_SIZE];
      char command[BUF_SIZE];
      char garbage[BUF_SIZE]; // per controllare che l'utente non inserisca più campi del dovuto (nb: non è un problema per il funzionamento dell'applicazione ma server se si vuole comunicare all'utente l'uso incorretto dell'applicazione)

      // si sovrascrive il primo byte, per evitare di leggere il comando precedente
      command[0] = arg1[0] = arg2[0] = garbage[0] = '\0';


      // leggo max BUF_SIZE caratteri dallo stdin
      // nb: se l'utente inserisce più di BUF_SIZE caratteri viene tagliato il messaggio
      fgets(buffer, BUF_SIZE, stdin);

      // scompongo l'input in quattro stringhe: comando arg1 arg2 garbage
      sscanf(buffer, "%s %s %s %s", command, arg1, arg2, garbage);

      // per evitare problemi con strcmp metto un marcatore di fine stringa nell'ultima posizione
      command[BUF_SIZE - 1] = arg1[BUF_SIZE - 1] = arg2[BUF_SIZE - 1] = garbage[BUF_SIZE - 1] ='\0'; 

      // conta il numero di parametri passati, oltre al comando
      int argc = 0;
      argc += strlen(arg1) > 0 ? 1 : 0;
      argc += strlen(arg2) > 0 ? 1 : 0;
      argc += strlen(garbage) > 0 ? 1 : 0;

      return checkCommandArgs(command, argc, arg1, arg2);
}

/**
 * Stampa a video i comandi disponibili con delle brevi descrizioni per l'utilizzo
 * @param managementCommands Se true stampa di comandi di gestione della fase iniziale
 * @param gameCommands Se true stampa di comandi di partita
*/
void printCommands(bool managementCommands, bool gameCommands){
      int i, index = 0;

      if(!managementCommands && !gameCommands) return;

      // riga iniziale per dividere
      printLineOfChars('#', true);

      if(managementCommands){
            printf("Comandi per la fase iniziale:\n");
            index = 0;
            for(i = 0; i < NUM_COMMANDS; i++) if(management_command[i]){

                  index++;

                  // stampo un indice
                  printf("(%d) ", index);

                  // stampo il comando
                  printf("%s:\t", COMMANDS_printable[i]);

                  // stampo la descrizioen
                  printf("%s ", COMMANDS_description[i]);

                  printf("\n");
            }

            if(gameCommands) printLineOfChars('-', false);
      }

      if(gameCommands){
            printf("Comandi di gioco:\n");
            index = 0;
            for(i = 0; i < NUM_COMMANDS; i++) if(game_command[i]){
                  
                  index++;
                  // stampo un indice
                  printf("(%d) ", index);

                  // stampo il comando
                  printf("%s:\t", COMMANDS_printable[i]);

                  // stampo la descrizioen
                  printf("%s ", COMMANDS_description[i]);

                  printf("\n");
            }
      }


      // riga finale per dividere
      printLineOfChars('#', true);

      fflush(stdout);
}


int main(int argc, char** argv){
      int ret, i = 0;
      char buffer[BUF_SIZE];

      // ********************************** Controllo degli input da terminale ********************************************

      // equivalente di clear da terminale
      system("clear");

      // controllo che sia stato passato il numero corretto di argomenti
      if(argc != 2 || !isStringNumber(argv[1])){
            printf("%s\n", client_usage);
            exit(1);
      }


      // ********************************** Connessione con il server primario ********************************************

      in_port_t main_server_port = atoi(argv[1]);
      in_port_t game_server_port = -1;
      int mainSocket = -1;
      int gameSocket = -1;
      struct sockaddr_in main_server_addr;
      struct sockaddr_in game_server_addr;

      mainSocket = socket(AF_INET, SOCK_STREAM, 0);

	// Indirizzo del server, porta specificata da riga di comando
      address_setting(&main_server_addr, main_server_port, NULL, false);
      
      // Connession con il server
	ret = connect(mainSocket, (struct sockaddr*)&main_server_addr, sizeof(main_server_addr));
	if(ret < 0){
		perror("Errore in fase di connessione: ");
            printf("\n");
		goto error_handling_0;
	}



      // ********************************** Fase iniziale di login / registrazione ****************************************

      char username[USER_USERNAME_FIELD_DIM];
      char password[USER_PASSWORD_FIELD_DIM];

      // controlla che l'utente scelga una delle opzioni disponibili
      bool accesschoice;
      
      // controlla che l'accesso sia consentito dal server
      bool access_granted;

      // ***************************** Cicla finchè il server non permette l'accesso
      
login_phase:
      accesschoice = false;
      access_granted = false;
      memset(buffer, 0, BUF_SIZE);

      // Ciclo finchè non mi viene consentito l'accesso dal server
      while(!access_granted){

            // ***************** Richiede una scelta tra registrazione e login (e quit)
            printLineOfChars('#', true);

            accesschoice = false;
            char chosen_option;

            // cicla finchè l'tente non sceglie una delle scelte disponibili
            while(!accesschoice){
                  chosen_option = 'x';

                  // propone le scelte all'utente (L - login, R - registrazione, q - quit)
                  printf("%s\n", client_initial_options_head);
                  for(i = 0; i < NUM_LOG_OPTIONS; i++)
                        printf("%s%c%s\n", client_initial_options[2*i], client_initial_options_codes[i], client_initial_options[2*i + 1]);
                  printf("Nota bene: username e password devono essere lunghe la massimo 63 caratteri\n\n > ");
                  fflush(stdout);

                  // legge l'input dell'utente
                  fgets(buffer, BUF_SIZE, stdin);
                  sscanf(buffer, "%c", &chosen_option); // isolo solamente un carattere

                  if(chosen_option == LOGIN || chosen_option == REGISTRATION || chosen_option == QUIT)
                        accesschoice = true;
                  else
                        printf("Comando non riconosciuto, riprova\n");
            }

            // Se l'utente decide
            if(chosen_option == QUIT){
                  (void) sendCodeTCP(mainSocket, REQUEST_END_CONNECTION, true);
                  // (void) sendMessageTCP(mainSocket, username, true);
                  // mi disconnetto in ogni caso, non mi interessa il valore di ritorno

                  printf("Bye bye...\n\n");
                  fflush(stdout);

                  return -1;
            }

            char req_code = chosen_option == LOGIN ? REQUEST_LOGIN : REQUEST_REGISTRATION;


            // *************************** Richiede una username e password

            printLineOfChars('*', true);

            // ********** Username dell'utente

            // richiedo lo username
            printf("%s", insert_username);
            fflush(stdout);

            // leggo la stringa inserita
            fgets(buffer, BUF_SIZE, stdin);
            sscanf(buffer, "%63s", username); // taglio eventuali caratteri oltre il 64esimo
            username[USER_USERNAME_FIELD_DIM - 1] = '\0'; // per evitare access out of boundaries

            
            // ********** Password dell'utente

            // richiedo la password
            printf("%s", insert_password);
            fflush(stdout);

            // leggo la stringa inserita
            fgets(buffer, BUF_SIZE, stdin);
            sscanf(buffer, "%63s", password); // taglio eventuali caratteri oltre il 64esimo
            password[USER_PASSWORD_FIELD_DIM - 1] = '\0'; // per evitare access out of boundaries

            
            // *************************** Invio dati al server

            // Invio richiesta di login / registrazione
            (void) sendCodeTCP(mainSocket, req_code, true);

            // Invio username
            (void) sendMessageTCP(mainSocket, username, true);
            
            // Invio password
            (void) sendMessageTCP(mainSocket, password, true);


            // attendo risposta del server
            char server_response = RESPONSE_ACCESS_DENIED;

            ret = recvCodeTCP(mainSocket, &server_response, true);
            if(ret <= 0) goto error_handling_1;

            access_granted = (server_response == RESPONSE_ACCESS_GRANTED);

            if(!access_granted){
                  if(chosen_option == LOGIN)
                        printf("Credenziali di login errate, prova di nuovo\n");
                  else
                        printf("Username già in uso, provare con un altro username\n");
                  fflush(stdout);
            }
      }

      printf("Login effettuato correttamente, benvenuto\n");
      fflush(stdout);

      // Leggo la porta del game server
      ret = recvMessageTCP(mainSocket, buffer, true);
      if(ret <= 0) goto error_handling_1;
      sscanf(buffer, "%hu", &game_server_port);

      // Imposto l'indirizzo
      address_setting(&game_server_addr, game_server_port, NULL, false);

      // stampo i comandi disponibili
      printCommands(true, true);

      // Stampo una breve descrizione dell'applicazione
      printf("%s\n", app_description);
      fflush(stdout);

      // serve per evitare di farsi reinviare la informazioni sulle stanze se ne sono già in possesso
      bool first_connection = true;

      int room_count;
      char (*room_description)[ROOM_DES_MAX_LEN];


      // ******************************************************************************************************************
      // ********************************** Ciclo di esecuzione principale ************************************************
      // ******************************************************************************************************************

      // Ciclo interrotto solo da END o LOGOUT
      // Nel caso di end si termina l'esecuzione
      // Nel caso di logout si torna alla fase di login

      bool continue_main_loop = true;
      while(continue_main_loop){
            
            // ******************************* Connessione con il server di gioco *****************************************

            // Socket apposito
            gameSocket = socket(AF_INET, SOCK_STREAM, 0);

            // printf("Tentativo di connessione al server di gioco: %s:%d\n", inet_ntoa(game_server_addr.sin_addr), ntohs(game_server_addr.sin_port));
            // Connession con il server
            
            ret = connect(gameSocket, (struct sockaddr*)&game_server_addr, sizeof(game_server_addr));
            if(ret < 0){
                  perror("Errore in fase di connessione: ");
                  printf("\n");
                  goto error_handling_1;
            }

            // Mi faccio inviare le informazioni sulle stanze se è la prima iterazione
            if(first_connection){

                  // Invio il codice di richiesta
                  (void) sendCodeTCP(gameSocket, REQUEST_ROOM_INFO, true);

                  // Ricevo il numero delle stanze
                  char room_count_string[8];
                  ret = recvMessageTCP(gameSocket, room_count_string, true);
                  if(ret <= 0) goto error_handling_2;
                  sscanf(room_count_string, "%d", &room_count);

                  // Alloco room_count stringhe per le descrizioni
                  room_description = malloc(room_count * sizeof(char[ROOM_DES_MAX_LEN]));

                  // leggo tutte le descrizioni
                  for(i = 0; i < room_count; i++){
                        ret = recvMessageTCP(gameSocket, room_description[i], true);
                        if(ret <= 0) goto error_handling_2;
                  }

                  first_connection = false;
            }


            // ******************************* Attesa dell'inizio della partita *******************************************

            // buffer per il primo argomento dei comandi
            char arg1[BUF_SIZE];
            // buffer per il secondo argomento dei comandi
            char arg2[BUF_SIZE];

            // per controllare l'uscita dal ciclo di attesa di una partita
            bool wait_for_game = true;

            // per capire, dopo [while(wait_for_game)], se tornare alla fase di login
            bool goto_login = false;

            // In questa fase l'utente può solo inserire
            //  > start <room>
            //  > logout
            //  > end
            //  > help

            // stampo i comandi disponibili per questa fase
            printCommands(true, false);

            // **************** Stampo le informazioni sulle stanze disponibili

            // stampo le stanze disponibili
            printf("Stanze disponibili:\n");

            for(i = 0; i < room_count; i++)
                  printf("codice \033[1m%d\033[m: %s\n", i, room_description[i]);
            
            printf("\n");
            fflush(stdout);
    

            while(wait_for_game){

                  printf("\nInserisci un comando:\n > ");
                  fflush(stdout);

                  // legge il comando inserito ed eventuali argomenti
                  // effettua tutti i controlli sul numero e tipo corretto degli argomenti
                  enum COMMANDS cmd = readCommand(arg1, arg2);

                  // se l'input è errato non si valuta il comando
                  if(cmd == -1) {
                        printf("Comando non riconosciuto, digita <help> per la lista dei comandi\n");
                        continue;
                  }

                  char response_code;

                  // comando valido: lo si esegue
                  switch(cmd){

                  // Il client si mette in lista di attesa per un gioco
                  case START: {

                        // invio il codice di richiesta di inzio gioco
                        (void) sendCodeTCP(gameSocket, REQUEST_START_GAME, true);

                        // invio il codice della stanza
                        (void) sendMessageTCP(gameSocket, arg1, true);

                        // invio il mio username
                        (void) sendMessageTCP(gameSocket, username, true);

                        // server mi comunica se la stanza è una di quelle disponibili o meno
                        ret = recvCodeTCP(gameSocket, &response_code, true);
                        if(ret <= 0) goto error_handling_2;

                        // Se non ho ricevuto RESPONSE_START_OK
                        if(response_code != RESPONSE_START_OK){
                              
                              // Se sono l'ultimo client e la partita inizia a breve
                              if(response_code == RESPONSE_START_OK_NOW){

                                    printf("La partita sta per iniziare\n"); 
                                    fflush(stdout);

                                    // attendo COMMUNICATION_STARTING_GAME, è dopo una fork quindi potrebbe esserci un po' di delay

                                    // codice ricevuto dal server relativo alla start
                                    ret = recvCodeTCP(gameSocket, &response_code, true);
                                    if(ret <= 0) goto error_handling_2;
                                    
                                    if(response_code == COMMUNICATION_STARTING_GAME)
                                          wait_for_game = false;
                                    

                                    else{
                                          printf("Errore nella fase iniziale della creazione di una stanza, il server potrebbe essere sovraccaricato\n");
                                          fflush(stdout);
                                    }

                                    // In entrambi i casi esco da [case START:] :
                                    //   Nel caso di COMMUNICATION_STARTING_GAME esco da while(wait_for_game) e vado alla fase iniziale della partita
                                    //   Nel caso di COMMUNICATION_START_FAILED rimango in while(wait_for_game)
                                    break;
                              }

                              // Se la richiesta di start era errata esco da [case START]
                              else {
                                    printf("Codice della stanza non valido, prova di nuovo\n"); 
                                    fflush(stdout);
                                    break; // esco da [case START]
                              }
                        }

                        // Richiesta di start corretta, addesso serve attende il numero corretto di giocatori
                        // E' l'unico caso in cui devo eseguire il codice successivo di attesa di COMMUNICATION_STARTING_GAME
                        printf("Attendi che altri utenti scelgano questa stanza, premi INVIO se vuoi interrompere l'attesa e uscire\n");
                        fflush(stdout);

                        // ************************************** Attesa dell'inizio della partita *********************************

                        // variabile per controllare se l'uscita dal ciclo di select è avvenuta per partita iniziata o abort start
                        bool game_started = false;

                        // bloccante finchè il server non riceve abbastanza richieste 
                        // per quella stanza per poter iniziare una partita

                        // Gestisco questa fase con IO multiplexing per poter ascoltare 
                        // sia il server che lo stdin, nel caso in cui il client voglia annullare la richiesta

                        // Campi per la gestione in IO multiplexing
                        fd_set fds_read, master;
                        FD_ZERO(&fds_read);
                        FD_ZERO(&master);

                        // Ci saranno solo questi due descrittori
                        // quindi non serivrà nemmeno un ciclo per controllare tutti i descrittori
                        FD_SET(0, &master);
                        FD_SET(gameSocket, &master);
                        int fdmax = gameSocket > 0 ? gameSocket : 0;

                        // stampo a video una barra di caricamento
                        // inutile ai fini della comunicazione, solo per non tenere il terminale visualmente bloccato durante l'attesa
                        int progress_bar_index = 0, j;

                        while(true){
                              fds_read = master;

                              // aggiorno la barra ogni 0.075 secondi / 75msec
                              struct timeval waiting_time;
                              waiting_time.tv_sec = 0;
                              waiting_time.tv_usec = 75000; // 0.075 secondi
                              
                              int pronti = select(fdmax + 1, &fds_read, NULL, NULL, &waiting_time);

                              // stdin vuoto e server ancora in attesa di altri client
                              if(pronti == 0){
                                    // Dimensione della barra di caricamento
                                    int full_progress_bar_width = getTerminalWindowWidth() - 8; // meno i due caratteri di apertura e chiusura e 3 spazi per lato
                                    
                                    // Stampo la barra di caricamento
                                    printf("\r   [");
                                    for(j = 0; j < full_progress_bar_width; j++){
                                          if(j == progress_bar_index){
                                                printf("#");
                                                continue;
                                          }
                                          if(j == (progress_bar_index + 1) % full_progress_bar_width){
                                                printf("#");
                                                continue;
                                          }
                                          if(j == (progress_bar_index + 2) % full_progress_bar_width){
                                                printf("#");
                                                continue;
                                          }
                                          printf(" ");
                                    }
                                    printf("]");
                                    fflush(stdout);
                                    progress_bar_index = (progress_bar_index + 1) % full_progress_bar_width;
                              }

                              // uno tra STDIN e gameSocket è pronto
                              else{

                                    // è pronta la risposta dal server
                                    if(FD_ISSET(gameSocket, &fds_read)){
                                          // pulisco il terminale (sovrascrivo la barra di caricamento)
                                          printf("\r");
                                          printLineOfChars(' ', false);

                                          // codice ricevuto dal server relativo alla start
                                          ret = recvCodeTCP(gameSocket, &response_code, true);
                                          if(ret <= 0) goto error_handling_2;
                                          
                                          if(response_code == COMMUNICATION_STARTING_GAME){
                                                game_started = true;
                                                printf("Numero necessario di utenti raggiunto, la partita può iniziare\n");
                                                fflush(stdout);
                                          }
                                          else{
                                                printf("Errore nella fase iniziale della creazione di una stanza, il server potrebbe essere sovraccaricato\n");
                                                fflush(stdout);
                                          }
                                    }

                                    // stdin pronto
                                    else{
                                          // abortisco la richiesta di una nuova partita
                                          (void) sendCodeTCP(gameSocket, REQUEST_START_ABORT, true);

                                          // invio il codice della stanza per la quale mi ero prenotato
                                          (void) sendMessageTCP(gameSocket, arg1, true);

                                          // pulisco lo stdin, altrimenti viene trovato pronto dalla select e interpretato come comando
                                          char c;
                                          do{
                                                c = getchar();
                                          } while(c != '\n' && c != EOF);
                                    }

                                    // in entrambi i casi esco dal ciclo di attesa
                                    break;
                              }
                        }

                        // se la partita è iniziata
                        if(game_started)
                              wait_for_game = false;

                        break;
                  }
                  

                  // Effettua la disconnessione dal server 
                  case END: {

                        // invio al server la richiesta di disconnessione (in realtà messaggio e basta, non attendo risposta)
                        (void) sendCodeTCP(mainSocket, REQUEST_END_CONNECTION, true); 

                        (void) sendCodeTCP(gameSocket, COMMUNICATION_DISCONNECT, true);

                        // per uscire dal ciclo di attesa della partita (while(wait_for_game))
                        wait_for_game = false;

                        // per terminare l'applicazione, andare nella parte finale di codice che chiuse socket e dealloca le struttture dinamiche
                        continue_main_loop = false;

                        break;
                  }


                  // Effettua il logout dal server ma rimane connesso, riporta l'utente alla fase iniziale di login/registrazione
                  case LOGOUT: {

                        // invio al server la richiesta di logout (in realtà messaggio e basta, non attendo risposta)
                        (void) sendCodeTCP(mainSocket, REQUEST_LOGOUT, true);
                        
                        // termino il ciclo principale
                        wait_for_game = false;

                        // segnale per ritornare alla fase di login
                        goto_login = true;

                        (void) sendCodeTCP(gameSocket, COMMUNICATION_DISCONNECT, true);
                        close(gameSocket);
                        
                        break;
                  }
                        

                  // Stampa nuovamente la lista dei comandi
                  case HELP: {
                        // stampo nuovamente la lista dei comandi
                        printCommands(true, true);
                        break;
                  }


                  default: {
                        printf("Comando %s non valido in questa fase\n", COMMANDS_name[cmd]);
                        break;
                  }

                  }

            }

            // Torna alla fase di login nel caso di LOGOUT
            if(goto_login) goto login_phase;

            // esce dal ciclo while(continue_main_loop) nel caso di END
            if(!continue_main_loop) break;


            // ******************************* Fase iniziale della partita ************************************************
            
            // stampo i comandi disponibili per questa fase
            printCommands(false, true);

            char token_total_string[8];

            // leggo il numero di token totali
            ret = recvMessageTCP(gameSocket, token_total_string, true);
            if(ret <= 0) goto error_handling_2;

            // Numero di token totali da raccogliere per vincere la partita
            int token_total = 0;
            sscanf(token_total_string, "%d", &token_total);

            // Leggo il numero di partecipanti
            int num_players = -1;
            char num_players_string[8];
            ret = recvMessageTCP(gameSocket, num_players_string, true);
            if(ret <= 0) goto error_handling_2;

            printf("\tInformazioni sulla partita:\n");
            fflush(stdout);

            sscanf(num_players_string, "%d", &num_players);
            printf("Numero di partecipanti: %d\n", num_players);
            fflush(stdout);

            // Leggo i nomi di ognuno
            printf("Username di tutti i partecipanti: ");
            fflush(stdout);

            for(i = 0; i < num_players - 1; i++){

                  char player_username[USER_USERNAME_FIELD_DIM];
                  
                  // Leggo lo username
                  ret = recvMessageTCP(gameSocket, player_username, true);
                  if(ret <= 0) goto error_handling_2;
                  
                  if(i != 0) printf(", ");
                  printf("%s", player_username);
                  fflush(stdout);

                  // Possibile modifica
                  // Salvare tutti i nomi dei partecipanti per mostrarli a video su richiesta dell'utente
                  // Ho scelto creare questo comando perchè è comunque possibile inviare un messaggio in 
                  // broadcast per chiedere ai vari utenti il proprio username (in realtà è sufficiente un 
                  // qualsiasi messaggio di risposta, verrà automaticamente stampato a video il mittente)
            }
            printf("\n");
            fflush(stdout);

            // Leggo la durata della partita
            int game_duration_sec = 0;
            char game_duration_sec_string[8];
            ret = recvMessageTCP(gameSocket, game_duration_sec_string, true);
            if(ret <= 0) goto error_handling_2;
            sscanf(game_duration_sec_string, "%d", &game_duration_sec);

            printf("Durata del gioco: %d'%02d\"\n", game_duration_sec / 60, game_duration_sec % 60);
            fflush(stdout);

            printf("Buona fortuna!\n");
            fflush(stdout);

            printLineOfChars('-', false);

            // ******************************* Gestione della partita *****************************************************

            // utilizzo IO multiplezing per controllare sia lo stdin che eventuali messaggi dal server

            // Variabile per controllare l'uscita dal ciclo di gestione della partita
            bool game_over = false;

            // numero di token guadagnati dal client
            int token_count = 0;

            // Per mostrare il timer a video
            struct timeval elapsed;
            elapsed.tv_sec = 0;
            elapsed.tv_usec = 0;

            // variabili per salvare gli istanti temporali di esecuzione della select e di terminazione
            struct timeval last_iteration_time, current_iteration_time;

            fd_set master;
            fd_set fds_read;
            int fdmax;

            FD_ZERO(&master);
            FD_ZERO(&fds_read);

            FD_SET(0, &master); // stdin
            FD_SET(gameSocket, &master); 
            fdmax = gameSocket > 0 ? gameSocket : 0;


            while(!game_over){
                  
                  printf("\n > ");
                  fflush(stdout);

                  fds_read = master;

                  // Salvo l'istante in cui chiamo la select (e quindi quando parte il timer)
                  gettimeofday(&last_iteration_time, NULL);
                  
                  // Controllo il timeout solo per mostrare a video il passare del tempo, 
                  // non ha alcuno scopo per la gestione temporale della partita
                  // Ho scelto di gestirla in maniera centralizzata sul server
                  // nb: C'è la possibilità che sia leggermente sfasato rispetto a quello del server che controlla il gioco
                  (void) select(fdmax + 1, &fds_read, NULL, NULL, NULL);

                  // calcolo il tempo passato
                  gettimeofday(&current_iteration_time, NULL);

                  // tengo traccia del tempo totale scorso
                  elapsed.tv_sec += (current_iteration_time.tv_sec - last_iteration_time.tv_sec);
                  elapsed.tv_usec += (current_iteration_time.tv_usec - last_iteration_time.tv_usec);

                  // aggiusto eventuali overflow dei microsecondi
                  if (elapsed.tv_usec > 1000000) {
                        elapsed.tv_sec += elapsed.tv_usec / 1000000;
                        elapsed.tv_usec %= 1000000;
                  }
                  if (elapsed.tv_usec < 0) {
                        elapsed.tv_usec += 1000000;
                        if(elapsed.tv_sec > 0) elapsed.tv_sec--;
                  }

                  // controllo prima il gameSocket
                  // nel caso in cui arrivassero in contemporanea un messaggio dal server 
                  // e un comando da stdin è necessario prima elaborare il messaggio nel server nel caso
                  // sia necessario interrompere il gioco in caso di gameover 
                  if(FD_ISSET(gameSocket, &fds_read)) {

                        char msg_code;
gameSocket_ready:
                        // leggo il messaggio
                        ret = recvCodeTCP(gameSocket, &msg_code, true);
                        if(ret <= 0) goto gameover_timeout;

                        switch (msg_code)
                        {

                        // Assegnamento di un token
                        case COMMUNICATION_TOKEN: {

                              // E' separato da una eventuale TAKE così da essere più versatile per una eventuale
                              // modifica delle strutture di gioco che comporti l'assegnamento di token anche per 
                              // altri tipi di azioni.
                              // Ad esempio si potrebbero assegnare token anche per LOOK per invitare l'utente ad
                              // esplorare tutte le parti della escape room

                              char token_string[8];
                              int given_token = 0;

                              // Leggo il numero di token
                              ret = recvMessageTCP(gameSocket, token_string, true);
                              if(ret <= 0) goto gameover_timeout;

                              // Riconverto a intero
                              sscanf(token_string, "%d", &given_token);

                              // Aggiorno il counter dei token
                              token_count += given_token;

                              printf("Hai ottenuto %d nuov%c token\n", given_token, (given_token > 1 ? 'i' : 'o'));
                              fflush(stdout);

                              break;
                        }

                        // Messaggio da un altro partecipante
                        case COMMUNICATION_MSG: {

                              char sender_username[USER_USERNAME_FIELD_DIM];
                              char message[BUF_SIZE];

                              // leggo mittente e messaggio
                              if(recvMessageTCP(gameSocket, sender_username, true) <= 0
                              || recvMessageTCP(gameSocket, message, true) <= 0)
                                    goto gameover_timeout;

                              // li mostro a video
                              printf("\rMessaggio dal giocatore %s:\n\"%s\"\n", sender_username, message);
                              fflush(stdout);

                              break;
                        }

                        // La partita è terminata in vittoria
                        case COMMUNICATION_GAME_WIN: {

                              // Stampo a video
                              printLineOfChars('#', true);
                              printf("\t\033[1mVITTORIA\033[m\n");
                              printf("Hai raccolto %d token su un totale di %d\n", token_count, token_total);
                              printf("Avete impiegato un totale di %ld minuti e %ld secondi\n", elapsed.tv_sec / 60, elapsed.tv_sec % 60);
                              printLineOfChars('#', true);
                              fflush(stdout);

                              game_over = true;
                              break; // esce dallo switch (msg_code)
                        }

                        // La partita è terminata in sconfitta
                        case COMMUNICATION_GAMEOVER: {

                              // Stampo a video
                              printLineOfChars('#', true);
                              printf("\t\033[1mGAMEOVER\033[m\n");
                              printf("Hai raccolto %d token su un totale di %d\n", token_count, token_total);
                              printLineOfChars('#', true);
                              fflush(stdout);

                              game_over = true;
                              break; // esce dallo switch (msg_code)
                        }
                        
                        
                        default: {
                              printf("\nComunicazione da server non riconosciuta [%02X]\n", msg_code);
                              fflush(stdout);
                              break;
                        }

                        }
                  }

                  // Se gameover esco senza processare un eventuale stdin pronto
                  // altrimenti invierei richieste ad un server non più attivo
                  if(game_over)
                        break;


                  // stampo il timer dei secondi passati
                  printf("\rTIMER: [%02d:%02d]\n", (int)(game_duration_sec - elapsed.tv_sec) / 60, (int)((game_duration_sec - elapsed.tv_sec) % 60));
                  fflush(stdout);

                  // stampo il numero dei token
                  printf("\rTOKEN: [%02d/%02d]\n", token_count, token_total);
                  fflush(stdout);


                  // stdin pronto, invio richiesta al server
                  if(FD_ISSET(0, &fds_read)){
                        
                        // legge il comando inserito ed eventuali argomenti
                        // effettua tutti i controlli sul numero e tipo corretto degli argomenti
                        enum COMMANDS cmd = readCommand(arg1, arg2);

                        // se l'input è errato non si valuta il comando
                        if(cmd == -1) {
                              printf("Comando non riconosciuto, digita <help> per la lista dei comandi\n");
                              continue;
                        }

                        char response_code;

                        switch (cmd)
                        {

                        // richiesta della descrizione di un oggetto / locazione
                        case LOOK: {

                              // invio la richiesta di LOOK
                              (void) sendCodeTCP(gameSocket, REQUEST_LOOK, true);

                              // invio la locazione (solo \0 se non è specificato l'argomento)
                              (void) sendMessageTCP(gameSocket, arg1, true);
                              
                              // Attendo la risposta
                              ret = recvCodeTCP(gameSocket, &response_code, true);
                              if(ret <= 0) goto gameover_timeout;

                              switch(response_code){

                              case RESPONSE_LOOK_OK: {

                                    char description[BUF_SIZE_L];

                                    // Leggo la descrizione
                                    ret = recvMessageTCP(gameSocket, description, true);
                                    if(ret <= 0) goto gameover_timeout;

                                    // mostro la risposta a video
                                    printf("%s\n", description);
                                    fflush(stdout);

                                    break;
                              }

                              case RESPONSE_LOOK_not_poss: {
                                    // oggetto non posseduto

                                    // lo comunico a video
                                    printf("Oggetto collezionabile non posseduto, prendilo con <take %s>\n", arg1);
                                    fflush(stdout);

                                    break;
                              }

                              case RESPONSE_LOOK_ERR: {
                                    printf("Locazione / oggetto non presente, prova di nuovo\n");
                                    fflush(stdout);
                                    break;
                              }

                              }
                              break;
                        }


                        // richiesta: prendere un oggetto
                        case TAKE: {

                              // invio la richiesta di TAKE
                              // printf("SENDING REQUEST_TAKE\n"); fflush(stdout);
                              (void) sendCodeTCP(gameSocket, REQUEST_TAKE, true);

                              // invio l'oggetto (solo \0 se non è specificato l'argomento)
                              // printf("SENDING NAME\n"); fflush(stdout);
                              (void) sendMessageTCP(gameSocket, arg1, true);

                              // Attendo la risposta;
                              ret = recvCodeTCP(gameSocket, &response_code, true);
                              if(ret <= 0) goto gameover_timeout;


                              switch (response_code)
                              {

                              // Oggetto preso
                              case RESPONSE_TAKE_OK: {

                                    // lo comunico all'utente a video
                                    printf("Oggetto %s preso, adesso è parte del tuo inventario\n", arg1);
                                    fflush(stdout);

                                    break;
                              }

                              // Inventario pieno
                              case RESPONSE_TAKE_inv_full: {

                                    // lo comunico all'utente a video
                                    printf("Inventario pieno, lascia un oggetto per poter prendere %s\n", arg1);
                                    fflush(stdout);

                                    break;
                              }

                              // Oggeto già preso
                              case RESPONSE_TAKE_alr_taken: {

                                    // lo comunico all'utente a video
                                    printf("Oggetto %s già preso da qualcuno, se non lo possiedi puoi chiedere informazioni agli altri partecipanti con il comando <msg>\n", arg1);
                                    fflush(stdout);

                                    break;
                              }
                              
                              // Oggetto già preso
                              case RESPONSE_TAKE_not_coll: {

                                    // lo comunico all'utente a video
                                    printf("L'oggetto %s non è collezionabile\n", arg1);
                                    fflush(stdout);

                                    break;
                              }

                              // Oggetto bloccato
                              case RESPONSE_TAKE_blocked: {

                                    // lo comunico all'utente a video
                                    printf("Oggetto %s bloccato, cerca nell'escape room una maniera per liberarlo\n", arg1);
                                    fflush(stdout);

                                    break;
                              }

                              // Oggetto bloccato da un enigma a risposta singola
                              case RESPONSE_TAKE_riddle_ans: {

                                    char riddle_identifier[8]; // è un intero in forma di stringa, su circa 2 cifre
                                    char object_identifier[8]; // è un intero in forma di stringa, su circa 2 cifre
                                    char riddle[BUF_SIZE_S];

                                    // Leggo id dell'enigma, id dell'oggetto e testo dell'enigma
                                    if(recvMessageTCP(gameSocket, riddle_identifier, true) <= 0
                                    || recvMessageTCP(gameSocket, object_identifier, true) <= 0
                                    || recvMessageTCP(gameSocket, riddle, true) <= 0)
                                          goto gameover_timeout;

                                    // lo comunico all'utente a video
                                    printf("Oggetto %s bloccato da un enigma:\n", arg1);
                                    printf("%s\n > ", riddle);
                                    fflush(stdout);

                                    // Attendo la risposta dall'utente
                                    fgets(buffer, BUF_SIZE, stdin);

                                    // Tolgo il \n finale, lo sostituisco con '\0'
                                    buffer[strlen(buffer) - 1] = '\0';

                                    // E' possibile che il tempo sia terminato mentre l'utente scriveva la risposta,
                                    // in tal caso esco dal ciclo di esecuzione
                                    if(isSocketReady(gameSocket))
                                          goto gameSocket_ready;

                                    // invio al server il codice di risposta ad un enigma
                                    (void) sendCodeTCP(gameSocket, REQUEST_RIDDLE_ANS, true);

                                    // Invio id dell'enigma, id dell'oggetto e risposta all'enigma
                                    (void) sendMessageTCP(gameSocket, riddle_identifier, true);
                                    (void) sendMessageTCP(gameSocket, object_identifier, true);
                                    (void) sendMessageTCP(gameSocket, buffer, true);

                                    // Attendo la risposta
                                    ret = recvCodeTCP(gameSocket, &response_code, true);
                                    if(ret <= 0) goto gameover_timeout;

                                    // Stampo a video la risposta del server
                                    switch (response_code)
                                    {
                                    case RESPONSE_RIDDLE_OK:
                                          printf("Enigma risolto!\n");
                                          break;

                                    case RESPONSE_RIDDLE_ERR:
                                          printf("Risposta sbagliata\n");
                                          break;
                                    
                                    default:
                                          printf("ERRORE nella risposta all'enigma");
                                          break;
                                    }

                                    fflush(stdout);
                                    break;
                              }

                              // Oggetto bloccato da un enigma a sequenza di oggetti
                              case RESPONSE_TAKE_riddle_seq: {

                                    char riddle_identifier[8]; // è un intero in forma di stringa, su circa 2 cifre
                                    char object_identifier[8]; // è un intero in forma di stringa, su circa 2 cifre
                                    char riddle[BUF_SIZE_S];

                                    // Leggo id dell'enigma, id dell'oggetto e testo dell'enigma
                                    if(recvMessageTCP(gameSocket, riddle_identifier, true) <= 0
                                    || recvMessageTCP(gameSocket, object_identifier, true) <= 0
                                    || recvMessageTCP(gameSocket, riddle, true) <= 0)
                                          goto gameover_timeout;

                                    // lo comunico all'utente a video
                                    printf("Oggetto %s bloccato da un enigma:\n", arg1);
                                    printf("%s\n", riddle);
                                    fflush(stdout);

                                    
                                    // invio al server il codice di risposta ad un enigma
                                    (void) sendCodeTCP(gameSocket, REQUEST_RIDDLE_SEQ, true);

                                    // Invio id dell'enigma e id dell'oggetto
                                    (void) sendMessageTCP(gameSocket, riddle_identifier, true);
                                    (void) sendMessageTCP(gameSocket, object_identifier, true);

                                    // Attendo la risposta
                                    ret = recvCodeTCP(gameSocket, &response_code, true);
                                    if(ret <= 0) goto gameover_timeout;

                                    // Stampo a video la risposta del server
                                    switch (response_code)
                                    {
                                    case RESPONSE_RIDDLE_OK:
                                          printf("La sequenza era corretta!\n");
                                          break;

                                    case RESPONSE_RIDDLE_ERR:
                                          printf("Sequenza errata\n");
                                          break;
                                    
                                    default:
                                          printf("ERRORE nella risposta all'enigma");
                                          break;
                                    }

                                    fflush(stdout);

                                    break;
                              }

                              // Oggetto non trovato
                              case RESPONSE_TAKE_ERR: {

                                    // lo comunico all'utente a video
                                    printf("Oggetto %s non presente nell'escape room\n", arg1);
                                    fflush(stdout);

                                    break;
                              }   

                              // Tutti i casi possibili correti sono quelli precedenti 
                              default: {

                                    // lo comunico all'utente a video
                                    printf("ERRORE nel codice di risposta;\n");
                                    fflush(stdout);

                                    break;
                              }
                              
                              }
                              
                              break;
                        }


                        // richiesta: rilasciare un oggetto
                        case DROP: {
                              
                              // Invio il codice di richiesta
                              (void) sendCodeTCP(gameSocket, REQUEST_DROP, true);

                              // Invio il nome dell'oggetto
                              (void) sendMessageTCP(gameSocket, arg1, true);

                              // Attendo la risposta
                              ret = recvCodeTCP(gameSocket, &response_code, true);
                              if(ret <= 0) goto gameover_timeout;

                              // stampo a video il risultato dell'operazione
                              if(response_code != RESPONSE_DROP_OK)
                                    printf("Errore nel rilascio dell'oggetto\n");
                              
                              else
                                    printf("Oggetto rilasciato correttamente\n");
                              
                              fflush(stdout);

                              break;
                        }


                        // richiesta: utilizzo di un oggetto    
                        case USE: {

                              // Invio il codice di richiesta
                              if(strlen(arg2) == 0)
                                    (void) sendCodeTCP(gameSocket, REQUEST_USE_ONE, true);
                              else
                                    (void) sendCodeTCP(gameSocket, REQUEST_USE_TWO, true);

                              // Invio il nome dell'oggetto
                              (void) sendMessageTCP(gameSocket, arg1, true);

                              // Invio il nome del secondo oggetto se presente
                              if(strlen(arg2) > 0)
                                    (void) sendMessageTCP(gameSocket, arg2, true);

                              // Attendo il risultato dell'operazione
                              ret = recvCodeTCP(gameSocket, &response_code, true);
                              if(ret <= 0) goto gameover_timeout;

                              switch(response_code){

                              // La use ha prodotto qualche effetto
                              case RESPONSE_USE_OK: {
                                    printf("Qualcosa si è mosso...\n");
                                    fflush(stdout);
                                    break;
                              }

                              // La use non ha avuto alcun effetto
                              case RESPONSE_USE_no_effect: {
                                    printf("Non sembra sia cambiato niente\n");
                                    fflush(stdout);
                                    break;
                              }

                              // Utilizzo dell'oggetto negato
                              case RESPONSE_USE_denied: {
                                    printf("Non puoi utilizzare l'oggetto %s\n", arg1);
                                    fflush(stdout);
                                    break;
                              }

                              case RESPONSE_USE_ERR: {
                                    printf("Errore nella use, controlla i nomi degli oggetti\n");
                                    fflush(stdout);
                                    break;
                              }

                              default:
                                    break;
                                    
                              }

                              break;
                        }
                              

                        // richiesta: visualizzare il proprio inventario
                        case OBJS: {
                              (void) sendCodeTCP(gameSocket, REQUEST_OBJS, true);

                              // Ricevo l'inventario sotto forma di stringa
                              ret = recvMessageTCP(gameSocket, buffer, true);
                              if(ret <= 0) goto gameover_timeout;

                              // stampo l'inventario
                              printf("INVENTARIO:\n%s\n", buffer);
                              fflush(stdout);

                              break;
                        }


                        // richiesta: messaggia un altro partecipante
                        case MSG: {

                              char message[BUF_SIZE];

                              // richiedo il messaggio da inviare
                              printf("Messaggio da inviare:\n_\r");
                              fflush(stdout);

                              // leggo il messaggio
                              fgets(message, BUF_SIZE, stdin);

                              // tolgo l'ultimo \n
                              message[strlen(message) - 1] = '\0';

                              // E' possibile che il tempo sia terminato mentre l'utente scriveva il messaggio,
                              // in tal caso esco dal ciclo di esecuzione
                              if(isSocketReady(gameSocket))
                                    goto gameSocket_ready;

                              // invio il codice della richiesta, username del destinatario e messaggio
                              (void) sendCodeTCP(gameSocket, REQUEST_MSG, true);
                              (void) sendMessageTCP(gameSocket, arg1, true);
                              (void) sendMessageTCP(gameSocket, message, true);

                              // attendo la risposta
                              ret = recvCodeTCP(gameSocket, &response_code, true);
                              if(ret <= 0) goto gameover_timeout;

                              // stampo a video l'esito dell'operazione
                              switch (response_code)
                              {
                              case RESPONSE_MSG_OK:
                                    printf("Messaggio recapitato correttamente\n");
                                    break;

                              case RESPONSE_MSG_self:
                                    printf("Hai scritto un messaggio a te stesso\n");
                                    break;

                              case RESPONSE_MSG_disconnected:
                                    printf("L'utente %s si è disconnesso\n", arg1);
                                    break;
                              
                              case RESPONSE_MSG_ERR:
                                    printf("Utente %s non presente nella partita\n", arg1);
                                    break;
                              }

                              fflush(stdout);

                              break;
                        }


                        // Stampa nuovamente la lista dei comandi
                        case HELP: {
                              // stampo nuovamente la lista dei comandi
                              printCommands(false, true);
                              break;
                        }

                        
                        // Esce dalla partita corrente
                        case LEAVE: {

                              // invio al server di gioco corrente la comunicazione di disconnnessione
                              (void) sendCodeTCP(gameSocket, COMMUNICATION_DISCONNECT, true);

                              // esco dal ciclo di gestione della partita
                              game_over = true;

                              break;
                        }


                        // Effettua la disconnessione dal server 
                        case END: {

                              // invio al server primario la richiesta di disconnessione (in realtà messaggio e basta, non attendo risposta)
                              (void) sendCodeTCP(mainSocket, REQUEST_END_CONNECTION, true);

                              // invio anche al server di gioco corrente la comunicazione di disconnnessione
                              (void) sendCodeTCP(gameSocket, COMMUNICATION_DISCONNECT, true);

                              // esco dal ciclo di gestione della partita
                              game_over = true;

                              // esce dal ciclo di esecuzione dell'applicazione cliente (connessione con server di gioco - partita)
                              continue_main_loop = false;

                              break;
                        }


                        default: {
                              printf("Comando %s non valido in questa fase\n", COMMANDS_name[cmd]);
                              break;
                        }
                        
                        }
                                
                        // Caso estremamente più probabile: gameover per timeout durante l'attesa dello stdin
                        // Può essere avvenuto per la risposta ad un enigma o per la scrittura di un messaggio.
                        // Potrebbe anche avvenire durante l'invio di una serie di messaggi di richiesta, ad
                        // esempio tra l'invio di una sendCodeTCP() e le varie sendMessageTCP successive, di conseguenza
                        // il codice del client attenderebbe un codice di risposta con il ROOM_SERVER non più attivo
                        if(game_over){
gameover_timeout: 
                              // Per non appesantire il codice in ogni recvCode/MessageTCP con 
                              // {
                              //    game_over = true;
                              //    goto gameover_timeout;
                              // }
                              game_over = true;

                              // Stampo a video
                              printLineOfChars('#', true);
                              printf("\t\033[1mGAMEOVER\033[m\n");
                              printf("Hai raccolto %d token su un totale di %d\n", token_count, token_total);
                              printLineOfChars('#', true);
                              fflush(stdout);
                        }

                  } // if(FD_ISSET(0, &fds_read))
            
            } // while(!game_over)

            // chiudo il gameSocket, lo riapro ricollegandomi al gam server
            close(gameSocket);

      } // while(continue_main_loop)
      
      // **************************************** Chiusura dell'applicazione ********************************************

      // le due etichette sono utilizzate in varie parti del codiec da [goto error_handling_1/2]
      // per terminare l'applicazione in caso di terminazione del server non recuperabile
      // Il caso in cui il socket del room server viene trovato chiuso è gestito a parte perchè
      // può essere dovuto semplicemente al timeout della partita in corso

error_handling_2:
      free(room_description);

error_handling_1:
      close(gameSocket);

error_handling_0:
      close(mainSocket);

      exit(1);
}