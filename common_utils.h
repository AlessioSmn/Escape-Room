#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <stdbool.h>
#include <arpa/inet.h> // sockaddr_in, AF_INET, INADDR_ANY

#define ROOM_COUNT 3

#define BUF_SIZE_L      4096
#define BUF_SIZE        1024
#define BUF_SIZE_S      256
#define BUF_SIZE_XS     64

// Dimensione dei codici di richiesta (REQUEST) e risposta (RESPONSE)
#define REQRES_CODE_SIZE 1

// Massima dimensione delle descrizioni delle stanze
#define ROOM_DES_MAX_LEN BUF_SIZE_S

#define PIPE_WRITE 1
#define PIPE_READ  0

// si inviano solo stringhe brevi, meno di 255 caratteri
#define PIPE_MSG_DIM_MAX_LEN 1


/**
 * Controlla se la stringa passata rappresenta un numero naturale
 * @param str Stringa target, da controlloare se è un numero natruale o meno
 * @returns true se str rappresenta un numero, false altrimenti
*/
bool isStringNumber(char *str);


/**
 * Stampa una riga con il carattere c
 * @param c il caratttere con il quale formare la riga
 * @param isolate se True mette una riga vuota prima e dopo la riga piena
*/
void printLineOfChars(char c, bool isolate);


/**
 * Ritorna la larghezza della finestra del terminale
 * @returns Ritorna la larghezza della finestra del terminale
*/
int getTerminalWindowWidth();



// ************************ Inizializzazione degli indirizzi *********************

/**
 * @param Address Puntatore ad una struttra di tipo sockaddr_in dove salvare l'indirizzo
 * @param Port Porta
 * @param Addr Stringa contenente l'indirizzo IP in forma decimale puntata, se NULL si pone il server in ascolto su INADDR_ANY
 * @param printOutput Se true stampa varie informazioni
*/
void address_setting(struct sockaddr_in *address, in_port_t Port, const char* Addr, bool printOutput);


/**
 * @param Socket Puntatore al socket da collegare all'indirizzo specificato
 * @param Address Puntatore ad una struttra di tipo sockaddr_in dove è salvato l'indirizzo
 * @param returning Se true la funzione ritorna in ogni caso, se false esegue exit(1) in caso di fallimento
 * @param printOutput Se true stampa varie informazioni
 * @returns Ritorna 0 se l'operazione ha avuto successo, -1 altrimenti (se non sovrascritto da returning)
*/
int address_binding(int *Socket, struct sockaddr_in *address, bool returning, bool printOutput);


/**
 * @param Socket Puntatore al socket già collegato
 * @param queueLength Dimensione della coda di ascolto
 * @param returning Se true la funzione ritorna in ogni caso, se false esegue exit(1) in caso di fallimento
 * @param printOutput Se true stampa varie informazioni
 * @returns Ritorna 0 se l'operazione ha avuto successo, -1 altrimenti (se non sovrascritto da returning)
*/
int address_listening(int *Socket, int queueLength, bool returning, bool printOutput);


// ************************ TCP ***************************************************


/**
 * Funzione di utilità per una richiesta via socket sock
 * @note 1) Invia un solo carattere come da protocollo
 * @param sock socket dove inviare il messaggio
 * @param code codice della richiesta, utilizzare le macro
 * @param printError Se l'operazione non è completata con successo stampa una stringa di errore
 * @returns 1 se l'operazione è completata con successo, 0 se il socket ricevente è chiuso, -1 altrimenti
*/
int sendCodeTCP(int sock, const char code, bool printError);


/**
 * Funzione di utilità per una richiesta via socket sock
 * @note 1) Invia un solo carattere come da protocollo
 * @param sock socket dove inviare il messaggio
 * @param code codice della risposta da ricevere, utilizza le macro
 * @param printError Se l'operazione non è completata con successo stampa una stringa di errore
 * @returns 1 se l'operazione è completata con successo, 0 se il socket ricevente è chiuso, -1 altrimenti
*/
int recvCodeTCP(int sock, char *code, bool printError);


/**
 * Funzione di utilità per inviare un messaggio via socket sock
 * @note 1) Non fa alcun controllo sull'accesso al buffer
 * @note 2) Invia anche il marcatore di fine stringa
 * @param sock socket dove inviare il messaggio
 * @param buffer buffer contenente il messaggio
 * @param printError Se l'operazione non è completata con successo stampa una stringa di errore
 * @returns 1 se l'operazione è completata con successo, 0 se il socket ricevente è chiuso, -1 altrimenti
*/
int sendMessageTCP(int sock, const char *buffer, bool printError);


/**
 * Funzione di utilità per ricevere un messaggio via socket sock
 * @note 1) Non fa alcun controllo sull'accesso al buffer
 * @param sock socket attraverso il quale ricevere il messaggio
 * @param buffer buffer contenente il messaggio
 * @param printError Se l'operazione non è completata con successo stampa una stringa di errore
 * @returns 1 se l'operazione è completata con successo, 0 se il socket ricevente è chiuso, -1 altrimenti
*/
int recvMessageTCP(int sock, char *buffer, bool printError);


// ************************ Pipe **************************************************

/**
 * Funzione di utilità per inviare un messaggio via pipe, attraverso l'estremo pipeFD.
 * Invia prima la dimensione del messaggio su 8 bit, poi il messaggio
 * @note 1) Non fa alcun controllo su pipeFD
 * @note 2) Non fa alcun controllo sull'accesso al buffer
 * @note 3) Non fa alcun controllo sulla dimensione del buffer
 * @note 4) Invia anche il marcatore di fine stringa
 * @param pipeFD estremo della pipe dove inviare il messaggio
 * @param buffer buffer contenente il messaggio
 * @param printError Se l'operazione non è completata con successo stampa una stringa di errore
 * @returns 1 se l'operazione è completata con successo, -1 altrimenti
*/
int sendMessagePIPE(int pipeFD, const char *buffer, bool printError);


/**
 * Funzione di utilità per ricevere un messaggio via pipe, attraverso l'estremo pipeFD.
 * Riceve prima la dimensione del messaggio su 8 bit, poi il messaggio
 * @note 1) Non fa alcun controllo su pipeFD
 * @note 2) Non fa alcun controllo sull'accesso al buffer
 * @note 3) Non fa alcun controllo sulla dimensione del buffer
 * @note 4) Riceve anche il marcatore di fine stringa
 * @param pipeFD estremo della pipe dove ricevere il messaggio
 * @param buffer buffer dove copiare il messaggio
 * @param printError Se l'operazione non è completata con successo stampa una stringa di errore
 * @returns 1 se l'operazione è completata con successo, 0 se la pipe è chiusa dall'altro lato, -1 altrimenti
*/
int recvMessagePIPE(int pipeFD, char *buffer, bool printError);


/**
 * Funzione di utilità per inviare un codice via pipe
 * @note 1) Invia un solo carattere come da protocollo
 * @param pipeFD estremo della pipe dove inviare il messaggio
 * @param code codice della richiesta, utilizzare le macro
 * @param printError Se l'operazione non è completata con successo stampa una stringa di errore
 * @returns 1 se l'operazione è completata con successo, -1 altrimenti
*/
int sendCodePIPE(int pipeFD, const char code, bool printError);



/**
 * Funzione di utilità per ricevere un codice via pipe
 * @note 1) Invia un solo carattere come da protocollo
 * @param pipeFD estremo della pipe attraverso il quale ricevere il messaggio
 * @param code codice della richiesta, utilizzare le macro
 * @param printError Se l'operazione non è completata con successo stampa una stringa di errore
 * @returns 1 se l'operazione è completata con successo, 0 se la pipe è chiusa dall'altro lato, -1 altrimenti
*/
int recvCodePIPE(int pipeFD, char *code, bool printError);


// ************************ UDP ***************************************************

/**
 * Funzione di utilità per inviare un messaggio via socket sock
 * @note 1) Non fa alcun controllo sull'accesso al buffer
 * @note 2) Invia anche il marcatore di fine stringa
 * @param sock socket dove inviare il messaggio
 * @param buffer buffer contenente il messaggio
 * @param receiver_address indirizzo del destinatario, mettere a NULL se il socket è già connesso ad un indirizzo
 * @returns 1 se l'operazione è completata con successo, 0 se il socket ricevente è chiuso, -1 altrimenti
*/
/*
int sendMessageUDP(int sock, const char* buffer, struct sockaddr_in* receiver_address){
      socklen_t addrlen = sizeof(receiver_address);
      int ret;

      // ***** Invio la dimensione del messaggio
      int msglen = strlen(buffer) + 1; // includo anche '/0'
      uint16_t msglen_n = htons(msglen);
      sendto(sock, (void*)&msglen_n, sizeof(uint16_t), 0, (struct sockaddr*)receiver_address, &addrlen);

      // ***** Invio il messaggio
      int start = 0;
      while(start < msglen){
            ret = sendto(sock, (void*)buffer, msglen, 0, (struct sockaddr*)receiver_address, &addrlen);
            if(ret <= 0) goto error;
            start += ret;
      }

      return 1;

error:
      if(ret == -1){
            perror("Errore in fase di sendto: ");
            fflush(stdout);
            return -1;
      }
      printf("Socket ricevente chiuso\n");
      fflush(stdout);
      return 0;
}

int sendMessageUDP(int sock, const char* buffer){
      socklen_t addrlen = sizeof(receiver_address);
      int ret;

      // ***** Invio la dimensione del messaggio
      int msglen = strlen(buffer) + 1; // includo anche '/0'
      uint16_t msglen_n = htons(msglen);
      sendto(sock, (void*)&msglen_n, sizeof(uint16_t), 0);

      // ***** Invio il messaggio
      int start = 0;
      while(start < msglen){
            sendto(sock, (void*)buffer, msglen, 0);
            if(ret <= 0) goto error;
            start += ret;
      }

      return 1;
      
error:
      if(ret == -1){
            perror("Errore in fase di sendto (connesso): ");
            fflush(stdout);
            return -1;
      }
      printf("Socket ricevente chiuso\n");
      fflush(stdout);
      return 0;
}

int recvMessageUDP(int sock, char* buffer, struct sockaddr_in* sender_address){
      
}
*/
#endif