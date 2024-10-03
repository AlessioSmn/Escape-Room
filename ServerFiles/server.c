#include <stdio.h> // printf
#include <stdlib.h> // exit
#include <unistd.h> // close, getpid
#include <string.h> // memset
#include <arpa/inet.h> // sockaddr_in, AF_INET, INADDR_ANY
#include <stdint.h> // tipi a dimensione fissa
#include <stdbool.h>
#include <signal.h> // SIGKILL
#include "msg.h"
#include "../common_utils.h"
#include "../request_codes.h"
#include "server_utils.h"
#include "codes.h"
#include "game_server.h"

#define MAX_REGISTERED_USERS 2048

struct User{
      char username[USER_USERNAME_FIELD_DIM];
      char password[USER_PASSWORD_FIELD_DIM];
      bool logged;

      // File descriptor del socket legato all'utente, se loggato
      int fd;
};


/**
 * Ritorna l'indice di un utente dato il file descriptor del socket a lui relativo
 * @param registered_users_table Tabella contenente tutti gli utenti registrati
 * @param num_registered_users Numero di entrate nella tabella degli utenti registrati
 * @param fd File Descriptor da cercare nella tabella
 * @returns L'indice dell'utente nella tabella se presente, -1 altrimenti
*/
int getUserIndex_byFd(struct User registeredUsers[MAX_REGISTERED_USERS], const int num_registered_users, int fd){
      int i = 0;

      // scorro tutta la tabella
      for(; i < num_registered_users; i++)

            // se c'è match con il fd e l'utente è loggato
            if(registeredUsers[i].logged && registeredUsers[i].fd == fd)

                  // ritorno l'indice
                  return i;

      // se non ha trovato nessuno ritorna -1
      return -1;
}


/**
 * Controlla se l'utente specificato è presente e ritorna il suo indice nella tabella
 * @param registered_users_table Tabella contenente tutti gli utenti registrati
 * @param num_registered_users Numero di entrate nella tabella degli utenti registrati
 * @param username Buffer contenente lo username da cercare
 * @returns L'indice dell'utente nella tabella se presenta, -1 altrimenti
*/
int getUserIndex_byUsername(struct User registered_users_table[MAX_REGISTERED_USERS], const int num_registered_users, const char *username){
      int i = 0;
      // bool access_granted = false;

      // scorre la tabella degli utenti
      for(; i < num_registered_users; i++)

            // controlla se l'utente esiste
            if(strcmp(username, registered_users_table[i].username) == 0)

                  // ritorna l'indice nella tabella
                  return i;
      
      // se non ha trovato nessuno ritorna -1
      return -1;
}


/**
 * Stampa a video la lista dei comandi disponibili
*/
void printCommands(){
      int i = 0; 

      printLineOfChars('#', true);

      printf("Comandi disponibili:\n");
      for(i = 0; i < NUM_COMMANDS; i++){
            // stampo un indice
            printf("(%d) ", i+1);

            // stampo il nome del comando
            printf("%s ", COMMANDS_printable[i]);

            // stampo una breve descrizione
            printf("%s ", COMMANDS_description[i]);

            printf("\n");
      }

      printLineOfChars('#', true);

      fflush(stdout);
}


/**
 * Controlla che il comando e l'eventuale argomento associato al comando sia del tipo e formato corretto
 * @param cmd Il comando target
 * @param argc Il numero di argomenti passati
 * @param arg L'argomento da controllare
 * @returns Ritorna il comando inserito se il controllo ha successo, -1 altrimenti
*/
enum COMMANDS checkCommandArgs(char *command, int argc, char* arg){
      int i = 0;
      // controlla su tutti i comandi
      for(; i < NUM_COMMANDS; i++){

            // match con il comando
            if(strcmp(command, COMMANDS_name[i]) == 0){

                  // controlla che il numero di argomenti sia corretto
                  if(argc < COMMANDS_minArgs[i] || argc > COMMANDS_maxArgs[i]){
                        printf("%s\n%s\n%s", error_command_wrong_argc, COMMANDS_printable[i], server_newline_forcmd);
                        fflush(stdout);
                        return -1;
                  }
                  // in arg è contenuto l'eventuale argomento inserito
                  break;
            }
      }

      // nessun match con i comandi disponibili, quindi il comando è errato
      if(i == NUM_COMMANDS){
            printf("%s\n%s", error_command_not_found, server_newline_forcmd);
            fflush(stdout);
            return -1;
      }

      enum COMMANDS cmd = i;

      // ***** Controllo la correttezza del tipo e formato dei parametri

      // Start <port>
      if(cmd == START && !isStringNumber(arg)){
            printf("%s\n%s\n%s", error_command_wrong_argc, COMMANDS_printable[cmd], server_newline_forcmd);
            fflush(stdout);
            return -1;
      }

      // gli altri comandi non hanno argomenti, quindi se hanno passatto il controllo sul numero dei parametri vanno bene

      return cmd;
}


/**
 * legge una stringa da stdin e la analizza
 * @param arg Buffer per l'eventual argomento associato al comando inserito
 * @returns Ritorna il comando inserito se non ci sono errori, -1 altrimenti
*/
enum COMMANDS readCommand(char *arg){
      char buffer[BUF_SIZE];
      char command[BUF_SIZE];
      char garbage[BUF_SIZE]; // per controllare che l'utente non inserisca più campi del dovuto (nb: non è un problema per il funzionamento dell'applicazione ma server se si vuole comunicare all'utente l'uso incorretto dell'applicazione)

      // si sovrascrive il primo byte, per evitare di leggere il comando precedente
      command[0] = arg[0] = garbage[0] = '\0';


      // leggo max BUF_SIZE caratteri dallo stdin
      // nb: se l'utente inserisce più di BUF_SIZE caratteri viene tagliato il messaggio
      fgets(buffer, BUF_SIZE, stdin);

      // scompongo l'input in tre stringhe: comando arg1 garbage
      sscanf(buffer, "%s %s %s", command, arg, garbage);

      // per evitare problemi con strcmp metto un marcatore di fine stringa nell'ultima posizione
      command[BUF_SIZE - 1] = arg[BUF_SIZE - 1] = garbage[BUF_SIZE - 1] ='\0'; 

      // conta il numero di parametri passati, oltre al comando
      int argc = 0;
      argc += strlen(arg) > 0 ? 1 : 0;
      argc += strlen(garbage) > 0 ? 1 : 0;

      return checkCommandArgs(command, argc, arg);
}


// ****************************************************************************************************************************
// *************** MAIN SERVER ************************************************************************************************
// ****************************************************************************************************************************
// ****************************************************************************************************************************

int main(int argc, char** argv){
      // equivalente di clear da terminale
      system("clear");


      // ********************************* Controllo del corretto uso dell'applicazione da terminale **************************

      // controllo che sia stato passato il numero corretto di argomenti
      if(argc != 2 || !isStringNumber(argv[1])){
            printf("%s\n", server_usage);
            exit(1);
      }
      

      // ********************************* Inizializzazione del server primario ***********************************************

      int listener = socket(AF_INET, SOCK_STREAM, 0);
      bool isGameServerStarted = false;

      // Creo il socket listener e aggancio l'indirizzo
      // nb: faccio solo fino alla bind, farò listen solo dopo che verrà inserito il comando start
      struct sockaddr_in main_server_addr;
      address_setting(&main_server_addr, atoi(argv[1]), NULL, false);
      (void) address_binding(&listener, &main_server_addr, false, false);
      

      // ********************************* Strutture e files ******************************************************************

      // *********************** Files ******************************

      // Utenti registrati
      FILE *reg_users_file;
      int NUM_REG_USERS = 0;

      // Tabella contenente i vari utenti registrati
      struct User registered_users[MAX_REGISTERED_USERS];

      // apro il file
      reg_users_file = fopen("ServerFiles/registrated_user.txt", "a+"); // apro in READ e APPEND
      if(reg_users_file == NULL){
            printf("\nERRORE: file %s non esistente\n", "ServerFiles/registrated_user.txt");
            close(listener);
            exit(1);
      }
      
      // Leggo tutti gli utenti e li metto nella tabella registered_users
      while( NUM_REG_USERS < MAX_REGISTERED_USERS && 
            // ulteriore controllo sui 63 caratteri massimo
            // dovrebbe già essere garantito se il login è fatto correttamente
            EOF != fscanf(reg_users_file, "%63s %63s", registered_users[NUM_REG_USERS].username, registered_users[NUM_REG_USERS].password)){
            
            registered_users[NUM_REG_USERS].logged = false;

            NUM_REG_USERS++;
      }
      
      // long token_sequence = 0;

      // *********************** Strutture e variabili **************
      
      int fd = 0;
      int ret;

      // per controllare l'uscita dal principale ciclo di esecuzione
      bool stop_server = false;

      // salvo la porta del game server, la stampo su una eventuale seconda chiamata di start
      int game_server_port;
      pid_t game_server_pid;

      // fd per due pipe di comunicazione da e verso il game server
      int pipe_GameToMain[2];
      int pipe_MainToGame[2];

      // Set di descrittori di file
      fd_set master; // set di descrittori di file gestito del programmatore
      fd_set fds_read; // set usato dalla select
      int fdmax = 0;

      // pulisco e inizializzo i set di descrittori di file
      FD_ZERO(&master);
      FD_ZERO(&fds_read);
      FD_SET(0, &master); // inserisco lo stdin
      fdmax = 0;

      int connected_clients = 0; // contatore dei client connessi, da controllare per la terminazione del server


      // Buffer per leggere l'eventuale argomento del comando inserito da stdin
      char arg[BUF_SIZE];
      

      // ********************************* Ciclo del server primario *********************************************************

      // ***** Stampo i comandi disponibili per il server (definiti in ServerFiles/msg.h)
      printCommands();
      printf("%s", server_newline_forcmd);
      fflush(stdout);

      // ciclo infinito, esce se si inserisce il comando stop
      while(!stop_server){

            // copio il fs_set master nel set dedicato alla select
            fds_read = master;

            // chiamo la select
            // timeout infinito, se non ci sono fd pronti non è necessario fare nulla, quindi mi blocco
            (void) select(fdmax + 1, &fds_read, NULL, NULL, NULL);

            // scorro tutti i fd cercando quelli pronti
            for(fd = 0; fd <= fdmax; fd++) if(FD_ISSET(fd, &fds_read)){

                  // **************************************************** stdin pronto ***************************************
                  
                  if(fd == 0) {

                        // *************************** Lettura e analisi dello stdin

                        // funzione di utlità per leggere e analizzare l'input da terminale
                        enum COMMANDS cmd = readCommand(arg);

                        // se il comando è errato esce
                        if(cmd == -1) continue;


                        // *************************** Esecuzione del comando scelto
                        switch(cmd){

                        // Start fa partire il server di gioco
                        case START: {
                              
                              // se il comando è già stato chiamato una volta
                              if(isGameServerStarted){
                                    printf("Game Server già attivo sulla porta %d\n", game_server_port);
                                    break;
                              }
                              if(atoi(arg) == atoi(argv[1])){
                                    printf("Il game server non può essere attivato sulla stessa porta del main server (%d)\n", atoi(arg));
                                    break;
                              }

                              // creo una pipe per la comunicazione tra MAIN e GAME server
                              // necessaria per continuare con l'esecuzione del main server solo dopo che il game server è stato correttamente inizializzato
                              pipe(pipe_GameToMain);
                              pipe(pipe_MainToGame);


                              // Salvo la porta del game server, la stampo su altra chiamata di start
                              game_server_port = atoi(arg);

                              game_server_pid = fork();

                              if(game_server_pid == 0){ // figlio, GAME SERVER

                                    // ********************************************* Server di gioco

                                    // chiudo il socket di ascolto
                                    close(listener);

                                    // Lascio aperti solo i corretti lati delle due pipe
                                    close(pipe_GameToMain[PIPE_READ]);
                                    close(pipe_MainToGame[PIPE_WRITE]);
                                          
                                    // faccio eseguire il codice del server di gioco
                                    GAME_SERVER(game_server_port, pipe_GameToMain[PIPE_WRITE], pipe_MainToGame[PIPE_READ], true);
                              }

                              if(game_server_pid > 0){ // padre, server primario

                                    // ********************************************* Inizializzo il server primario

                                    // Lascio aperti solo i corretti lati delle due pipe
                                    close(pipe_GameToMain[PIPE_WRITE]);
                                    close(pipe_MainToGame[PIPE_READ]);

                                    char gameServer_init_response = GAME_SERVER_INIT_ERR;
                                    // Attendo che il game server sia partito
                                    // nb read è bloccante
                                    (void) recvCodePIPE(pipe_GameToMain[PIPE_READ], &gameServer_init_response, false);


                                    if(gameServer_init_response != GAME_SERVER_INIT_OK){
                                          // printf("Inizializzazione del game server, gameServer_init_response: [%02X]\n", gameServer_init_response);
                                          printf("Errore nell'inizializzazione del game server, prova di nuovo\n\n");
                                          break;
                                    }
                                    
                                    // metto in ascolto il server primario su localhost:porta specificata
                                    // printLineOfChars('#', true);
                                    if(address_listening(&listener, 10, true, false)){
                                          printf("%s\n", server_init_error);
                                          goto end_phase;
                                    }
                                    // printLineOfChars('#', true);

                                    // Aggiungo il listener alla lista dei file descriptor da controllare
                                    FD_SET(listener, &master);
                                    if(fdmax < listener) fdmax = listener;

                                    // Aggiungo il descrittore della pipe in lettura ai descrittori da controllare
                                    FD_SET(pipe_GameToMain[PIPE_READ], &master);
                                    if(fdmax < pipe_GameToMain[PIPE_READ]) fdmax = pipe_GameToMain[PIPE_READ];

                                    printf("Il game server è adesso disponibile sulla porta %d\n", game_server_port);
                                    fflush(stdout);

                                    isGameServerStarted = true;
                              }
                              
                              else{
                                    printf("Errore nella fork()\n");
                                    fflush(stdout);
                                    close(listener);
                                    exit(1);
                              }
                              
                              break;
                        }


                        // Termina il server se possibile
                        case STOP: {
                              if(connected_clients != 0){
                                    printf("%s\n", server_not_stoppable);
                                    fflush(stdout);
                              }
                              else{
                                    printf("%s\n", server_stoppable);
                                    close(listener);

                                    // fermo il main server (uscirà dal ciclo principale)
                                    stop_server = true;

                                    // se il game server non è stato lanciato basta questo
                                    if(!isGameServerStarted)
                                          break;

                                    // altrimenti devo comunicare anche a lui la richiesta di terminazione
                                    // impongo al game server di terminare
                                    (void) sendCodePIPE(pipe_MainToGame[PIPE_WRITE], GAME_SERVER_STOP, false);
                                    
                                    // controllo la risposta del game server
                                    char response;
                                    int ret = recvCodePIPE(pipe_GameToMain[PIPE_READ], &response, false);
                                    if(ret <= 0 || response != GAME_SERVER_STOP_OK){

                                          printf("Hard kill del game server necessaria\n");
                                          // se per qualche motivo non riesco più a comunicare con il game server lo devo in qualche modo interrompere comunque
                                          kill(game_server_pid, SIGKILL);
                                    }
                                    printf("%s\n\n", server_stopped);
                                    fflush(stdout);
                              }
                              break;
                        }


                        // Stampa di nuovo i comandi disponibili
                        case HELP:{
                              printCommands();
                              break;
                        }

                        
                        // Stampo varie informazioni sul server
                        case INFO: {
                              printf("\r      \n");
                              printf("Server principale: INADDR_ANY:%d\n", atoi(argv[1]));
                              if(isGameServerStarted)
                                    printf("Server di gioco: INADDR_ANY:%d\n", game_server_port);
                              else
                                    printf("Server di gioco: non ancora attivato\n");
                              printf("Utenti registrati: %d\n", NUM_REG_USERS);
                              printf("Clienti connessi: %d\n", connected_clients);
                              printf("\n");
                              fflush(stdout);
                              break;
                        }
                        

                        // comando non riconosciuto
                        default:{
                                    printf("%s\n", error_command_not_found);
                                    fflush(stdout);
                                    break;
                              }
                        }

                        // necessario per uscire dal for() che analizza tutti i fd in read_fds,
                        // altrimenti si rischia di accettare una nuova connessione sul server chiuso
                        // nb stdin è sempre analizzato prima di ogni socket se più file sono pronti, quindi fare break in questo punto evita di ascoltare nuove connnessioni
                        if(stop_server) break;

                        printf("%s", server_newline_forcmd);
                        fflush(stdout);
                        continue;
                  }


                  // ********************************************** messaggio dal Game Server ********************************
                  
                  if(fd == pipe_GameToMain[PIPE_READ]){
                        // Non ho ritenuto necessario alcun tipo di messaggio da game server a main server
                        // ma per una eventuale espansione del progetto la struttra per farlo è già pronta.
                        // Un esempio potrebbe essere quello di comunicare le vittorie e sconfitte dei vari utenti
                        // per tenerne traccia su un file / DB / altro elemento persistente e creare quindi delle
                        // classifiche o fare varie analisi dei giochi.
                        printf("%s", server_newline_forcmd);
                        fflush(stdout);
                        continue;
                  }
                  

                  // ************************************************* Main socket pronto ************************************
                  
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

                        // aggiorno il numero dei clienti connessi
                        connected_clients++;

                        printf("\rNuovo client connesso: (%s:%d), clienti totali: %d", inet_ntoa(client_addr.sin_addr), client_addr.sin_port, connected_clients);
                        fflush(stdout);
                        
                        printf("%s", server_newline_forcmd);
                        fflush(stdout);
                        continue;
                  }


                  // ************************************************* altro socket pronto ***********************************
                  
                  else {
                        char request_code;

                        // ***** leggo il codice della richiesta
                        ret = recvCodeTCP(fd, &request_code, false);

                        // se il socket è chiuso o c'è stato un errore lo tolgo dalla lista master
                        if(ret <= 0){
                              FD_CLR(fd, &master);
                              connected_clients--;

                              int user_index = getUserIndex_byFd(registered_users, NUM_REG_USERS, fd);

                              if(user_index == -1){
                                    printf("\rClient non loggato crashato, clienti connessi rimanenti: %d%s", connected_clients, server_newline_forcmd);
                                    fflush(stdout);
                                    continue;
                              }

                              registered_users[user_index].logged = false;

                              printf("\rClient crashato (%s), clienti connessi rimanenti: %d%s", registered_users[user_index].username, connected_clients, server_newline_forcmd);
                              fflush(stdout);
                              continue;
                        }


                        // ***** Eseguo la richiesta
                        switch(request_code){

                        // Richiesta di login al server
                        case REQUEST_LOGIN: {
                              char username[BUF_SIZE];
                              char password[BUF_SIZE];
                              bool access_granted = false;

                              // Riceve username e password
                              if(recvMessageTCP(fd, username, false) <= 0
                              || recvMessageTCP(fd, password, false) <= 0){

                                    // se il socket è chiuso o c'è stato un errore lo toglo dalla lista master
                                    FD_CLR(fd, &master);
                                    connected_clients--;

                                    int user_index = getUserIndex_byFd(registered_users, NUM_REG_USERS, fd);
                                    if(user_index == -1);
                                          break;

                                    registered_users[user_index].logged = false;

                                    printf("\rClient crashato (%s), clienti connessi rimanenti: %d%s", registered_users[user_index].username, connected_clients, server_newline_forcmd);
                                    fflush(stdout);

                                    break;
                              }

                              // controlla username e password
                              int user_index = getUserIndex_byUsername(registered_users, NUM_REG_USERS, username);

                              // controlla che l'utente non sia già loggato
                              // controlla che la password sia corretta
                              if(user_index != -1 
                              && !registered_users[user_index].logged 
                              && strcmp(password, registered_users[user_index].password) == 0)
                                    access_granted = true;

                              // Invia risposta
                              if(!access_granted){
                                    // Invio codice di access DENIED
                                    (void) sendCodeTCP(fd, RESPONSE_ACCESS_DENIED, false);
                                    printf("\rRichiesta di login per username (%s): accesso negato", username);
                                    fflush(stdout);
                                    break;
                              }

                              // segnalo il login dell'utente
                              registered_users[user_index].logged = true;

                              // salvo il fd 
                              registered_users[user_index].fd = fd;
                              
                              // Invio codice di access GRANTED al client
                              (void) sendCodeTCP(fd, RESPONSE_ACCESS_GRANTED, false);

                              // Invio anche la porta dove trovare il game server
                              // char game_server_port_msg[sizeof(game_server_port)];
                              char game_server_port_msg[sizeof(game_server_port)];
                              sprintf(game_server_port_msg, "%d", game_server_port);
                              (void) sendMessageTCP(fd, game_server_port_msg, false);

                              printf("\rRichiesta di login per username (%s): accesso consentito", username);
                              fflush(stdout);

                              break;
                        }


                        // Richiesta di registrazione
                        case REQUEST_REGISTRATION: {
                              char username[BUF_SIZE];
                              char password[BUF_SIZE];
                              bool free_username = false;
                              bool first_char_letter = false;

                              // Riceve username e password
                              if(recvMessageTCP(fd, username, false) <= 0
                              || recvMessageTCP(fd, password, false) <= 0){

                                    // se il socket è chiuso o c'è stato un errore lo toglo dalla listamaster
                                    FD_CLR(fd, &master);
                                    connected_clients--;

                                    int user_index = getUserIndex_byFd(registered_users, NUM_REG_USERS, fd);
                                    if(user_index == -1);
                                          break;

                                    registered_users[user_index].logged = false;

                                    printf("\rClient crashato (%s), clienti connessi rimanenti: %d%s", registered_users[user_index].username, connected_clients, server_newline_forcmd);
                                    fflush(stdout);

                                    break;
                              }

                              // controllo che non ci siano già altri utenti con lo stesso username
                              free_username = getUserIndex_byUsername(registered_users, NUM_REG_USERS, username) == -1 ? true : false;

                              // controllo che lo username inizi con una lettera
                              // così posso utilizzare i caratteri per comunicazioni speciali (es: MSG * (broadcast))
                              if((username[0] >= 'A' && username[0] <= 'Z')
                              || (username[0] >= 'a' && username[0] <= 'z'))
                                    first_char_letter = true;

                              if(!free_username || !first_char_letter){
                                    (void) sendCodeTCP(fd, RESPONSE_ACCESS_DENIED, false);
                                    printf("\rRichiesta di registrazione per username (%s): accesso negato", username);
                                    fflush(stdout);
                                    break;
                              }

                              // ****** Se lo username è libero registro l'utente

                              // aggiorno il file
                              fprintf(reg_users_file, "%s %s\n", username, password);

                              // aggiorno la tabella
                              strcpy(registered_users[NUM_REG_USERS].username, username);
                              strcpy(registered_users[NUM_REG_USERS].password, password);
                              registered_users[NUM_REG_USERS].logged = true; // registrazione e login contremporanei
                              registered_users[NUM_REG_USERS].fd = fd;

                              NUM_REG_USERS++;

                              // Invia risposta
                              (void) sendCodeTCP(fd, RESPONSE_ACCESS_GRANTED, false);

                              // Invio anche la porta dove trovare il game server
                              char game_server_port_msg[sizeof(game_server_port)];
                              sprintf(game_server_port_msg, "%d", game_server_port);
                              (void) sendMessageTCP(fd, game_server_port_msg, false);

                              printf("\rRichiesta di registrazione per username (%s): accesso consentito", username);
                              fflush(stdout);

                              break;
                        }


                        // Cliente si disconnette dal server
                        case REQUEST_END_CONNECTION: {

                              // chiudo il socket relativo
                              close(fd);

                              // lo toglo dalla lista master
                              FD_CLR(fd, &master);

                              // aggiorno il counter dei client collegati a questo processo server
                              connected_clients--;

                              // segnalo l'utente che non è più loggato
                              int user_index = getUserIndex_byFd(registered_users, NUM_REG_USERS, fd);

                              if(user_index == -1){
                                    printf("\rDisconnessione di un cliente non loggato, clienti connessi: %d", connected_clients);
                                    fflush(stdout);
                              }
                              else{
                                    // segnalo il logout nella tabella
                                    registered_users[user_index].logged = false;

                                    printf("\rDisconnessione di un cliente (%s), clienti connessi: %d", registered_users[user_index].username, connected_clients);
                                    fflush(stdout);
                              }

                              break;
                        }


                        // Cliente effettua il logout, ma rimane connesso
                        case REQUEST_LOGOUT: {
                              
                              // segnalo l'utente che non è più loggato
                              int user_index = getUserIndex_byFd(registered_users, NUM_REG_USERS, fd);

                              if(user_index == -1)
                                    break;

                              registered_users[user_index].logged = false;

                              printf("\rLogout di un cliente (%s), clienti connessi: %d", registered_users[user_index].username, connected_clients);
                              break;
                        }


                        default:
                              printf("\nRequest CODE not handled: [%c]/[%02X]", request_code, request_code);
                        }


                        printf("%s", server_newline_forcmd);
                        fflush(stdout);
                        continue;

                  } // else (ultima casistica del for)

            } // for(fd = 0; fd <= fdmax; fd++) 

      } // while(!stop_server)


      // ********************************* Chiusura di tutte le strutture allocate / utlizzate *******************************
end_phase:

      // ************** Chiusura dei file
      fclose(reg_users_file);

      // Chiudo le pipe aperte
      close(pipe_GameToMain[PIPE_WRITE]);
      close(pipe_MainToGame[PIPE_READ]);

      // ************** Chiusura dei socket
      close(listener);

      FD_ZERO(&master);
      FD_ZERO(&fds_read);

      return 0;
}