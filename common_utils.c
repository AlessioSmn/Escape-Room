#include "common_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset
#include <stdbool.h>
#include <arpa/inet.h> // sockaddr_in, AF_INET, INADDR_ANY
#include <stdbool.h>
#include <sys/ioctl.h> // ioctl
#include <unistd.h>

bool isStringNumber(char *str){
      int i = 0;
      for(; str[i] != '\0'; i++)
            if(str[i] < '0' || str[i] > '9')
                  return false;
      return true;
}

void printLineOfChars(char c, bool isolate){
      // Prende le dimensioni correnti della finestra del terminale
      struct winsize terminal_window;
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal_window); // terminal_window.ws_row and terminal_window.ws_col
      int terminal_width = terminal_window.ws_col, i;

      if(isolate) printf("\n");
      for(i = 0; i < terminal_width; i++) printf("%c", c);
      printf("\n");
      if(isolate) printf("\n");
}


int getTerminalWindowWidth(){
      // Prende le dimensioni correnti della finestra del terminale
      struct winsize terminal_window;
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal_window); // terminal_window.ws_row and terminal_window.ws_col
      return terminal_window.ws_col;
}



// ************************ Inizializzazione degli indirizzi *********************

void address_setting(struct sockaddr_in *address, in_port_t Port, const char* Addr, bool printOutput){

      // Pulizia della zona di memoria
      memset(address, 0, sizeof(struct sockaddr_in));

      address->sin_family = AF_INET;
      
      // Porta
      address->sin_port = htons(Port);
      
      // Indirizzo
      if(Addr == NULL) address->sin_addr.s_addr = INADDR_ANY;
      else inet_pton(AF_INET, (void*)Addr, &address->sin_addr);

      if(printOutput){
            printf("[%03X] [init] Indirizzo: [%s : %05d]\n",  getpid(), Addr == NULL ? "ANY" : Addr, Port);
            fflush(stdout);
      }
}


int address_binding(int *Socket, struct sockaddr_in *address, bool returning, bool printOutput){

      // Aggancio del socket all'indirizzo
      int ret = bind(*Socket, (struct sockaddr*)address, sizeof(*address) );
      
      // controllo del valore di ritorno
      if(ret < 0){
            if(printOutput){
                  printf("[%03X] [init] ", getpid());
                  fflush(stdout);
            }
            perror("Errore in fase di bind: ");
            if(returning)
                  return -1;
            else
                  exit(1);
      }

      if(printOutput){
	      printf("[%03X] [init] Bind completata con successo\n", getpid());
            fflush(stdout);
      }

      return 0;
}


int address_listening(int *Socket, int queueLength, bool returning, bool printOutput){
      
      // Inizio dell'ascolto, coda da [queueLength] connessioni
      int ret = listen(*Socket, queueLength);

      // controllo del valore di ritorno
      if(ret < 0){
            if(printOutput){
                  printf("[%03X] [init] ", getpid());
                  fflush(stdout);
            }
            perror("Errore in fase di listen: ");

            if(returning)
                  return -1;
            else
                  exit(1);
      }

      if(printOutput){
            printf("[%03X] [init] Listen completata con successo\nServer in ascolto...\n", getpid());
            fflush(stdout);
      }

      return 0;
}


// ************************ TCP ***************************************************

int sendCodeTCP(int sock, const char code, bool printError){

      int ret = send(sock, (void*)&code, sizeof(char), 0);

      if(ret == 1) return 1;
      if(ret == -1){
            if(printError){
                  perror("Errore in fase di send: ");
                  fflush(stdout);
            }
            return -1;
      }
      if(printError){
            printf("Socket ricevente chiuso\n");
            fflush(stdout);
      }
      return 0;
}


int recvCodeTCP(int sock, char *code, bool printError){

      int ret = recv(sock, (void*)code, sizeof(char), 0);

      if(ret == 1) return 1;
      if(ret == -1){
            if(printError){
                  perror("Errore in fase di recv: ");
                  fflush(stdout);
            }
            return -1;
      }
      if(printError){
            printf("Socket ricevente chiuso\n");
            fflush(stdout);
      }
      return 0;
}


int sendMessageTCP(int sock, const char *buffer, bool printError){
      // Text protocol con messaggi a dimensione variabile:
      // 1) Invio la dimensione del messaggio su 16 bit
      // 2) Invio il messaggio
      int ret;

      // ***** Invio la dimensione del messaggio
      int msglen = strlen(buffer) + 1; // includo anche '/0'
      uint16_t msglen_n = htons(msglen);

      ret = send(sock, (void*)&msglen_n, sizeof(uint16_t), 0);
      if(ret <= 0) goto error;
      // invio del secondo byte nel caso non fosse stato inviato, estremamente improbabile
      if(ret == 1){
            ret = send(sock, (void*)(&msglen_n + 1), sizeof(uint8_t), 0);
            if(ret <= 0) goto error;
      }

      // ***** Invio il messaggio
      int start = 0;
      while(start < msglen){
            ret = send(sock, (void*)(buffer + start), (msglen - start), 0);
            if(ret <= 0) goto error;
            start += ret;
      }
      
      return 1;

error:
      if(ret == -1){
            if(printError){
                  perror("Errore in fase di send: ");
                  fflush(stdout);
            }
            return -1;
      }
      if(printError){
            printf("Socket ricevente chiuso\n");
            fflush(stdout);
      }
      return 0;
}


int recvMessageTCP(int sock, char *buffer, bool printError){
      // Text protocol con messaggi a dimensione variabile:
      // 1) Ricevo la dimensione del messaggio su 16 bit
      // 2) Ricevo il messaggio
      int ret;

      // ***** Ricevo la dimensione del messaggio
      uint16_t msglen_n;

      ret = recv(sock, (void*)&msglen_n, sizeof(uint16_t), 0);
      if(ret <= 0) goto error;
      // invio del secondo byte nel caso non fosse stato inviato, estremamente improbabile
      if(ret == 1){
            ret = send(sock, (void*)(&msglen_n + 1), sizeof(uint8_t), 0);
            if(ret <= 0) goto error;
      }

      int msglen = ntohs(msglen_n);

      // ***** Ricevo il messaggio
      int start = 0;
      while(start < msglen){
            ret = recv(sock, (void*)(buffer + start), (msglen - start), 0);
            if(ret <= 0) goto error;
            start += ret;
      }
      
      return 1;

error:
      if(ret == -1){
            if(printError){
                  perror("Errore in fase di recv: ");
                  fflush(stdout);
            }
            return -1;
      }
      if(printError){
            printf("Socket ricevente chiuso\n");
            fflush(stdout);
      }
      return 0;
}


// ************************ Pipe **************************************************

int sendMessagePIPE(int pipeFD, const char *buffer, bool printError){
      int ret;
      uint8_t msglen = strlen(buffer) + 1; // non controllo che strlen() sia minore di 2^8

      // ***** Invio la dimensione del messaggio
      ret = write(pipeFD, (void*)&msglen, sizeof(uint8_t));
      if(ret <= 0) goto error;
      
      // ***** Invio il messaggio
      int start = 0;
      while(start < msglen){
            ret = write(pipeFD, (void*)(buffer + start), (msglen - start));
            if(ret <= 0) goto error;
            start += ret;
      }

      return 1;

error:
      if(printError){
            printf("Pipe chiusa\n");
            fflush(stdout);
      }
      return -1;
}


int recvMessagePIPE(int pipeFD, char *buffer, bool printError){
      int ret;
      uint8_t msglen;

      // ***** Ricevo la dimensione del messaggio
      ret = read(pipeFD, (void*)&msglen, sizeof(uint8_t));
      if(ret <= 0) goto error;
      
      // ***** Invio il messaggio
      int start = 0;
      while(start < msglen){
            ret = read(pipeFD, (void*)(buffer + start), (msglen - start));
            if(ret <= 0) goto error;
            start += ret;
      }

      return 1;
error:
      if(ret == 0){
            if(printError){
                  printf("Pipe chiusa dall'altro lato\n");
                  fflush(stdout);
            }
            return 0;
      }
      if(printError){
            printf("Errore nella lettura da pipe\n");
            fflush(stdout);
      }
      return -1;
}


int sendCodePIPE(int pipeFD, const char code, bool printError){
      int ret = write(pipeFD, (void*)&code, 1);
      if(ret == 1) return 1;

      if(printError){
            printf("Pipe chiusa\n");
            fflush(stdout);
      }
      return 0;
}


int recvCodePIPE(int pipeFD, char *code, bool printError){
      int ret = read(pipeFD, (void*)code, 1);
      if(ret == 1) return 1;

      if(ret == 0){
            if(printError){
                  printf("Pipe chiusa dall'altro lato\n");
                  fflush(stdout);
            }
            return 0;
      }
      if(printError){
            printf("Errore nella lettura da pipe\n");
            fflush(stdout);
      }
      return -1;
}