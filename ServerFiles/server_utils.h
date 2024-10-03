#ifndef server_utils
#define server_utils
#include <time.h>
#include <stdbool.h>
#include "../request_codes.h"

/**
 * Stampa l'orario corrente a fini di debugging
 * @note Formato: [HH:MM:SS]
*/
void printOrario();

/**
 * Struttura dati per mantenere l'informazione di username e socket di comunicazione di ogni utente collegato
 * Fondamentale per il comando <msg>, per effettuare la traduzione username-socket
 * @param fd File Descriptor del socket di comunicazione relativo all'utente
 * @param username Username dell'utente
 * @param logged Tiene traccia di una eventuale disconnessione dell'utente
*/
struct loggedUser{
      // File Descriptor del socket di comunicazione relativo all'utente
      int fd;

      // Username dell'utente
      char username[USER_USERNAME_FIELD_DIM];

      // Tiene traccia di una eventuale disconnessione dell'utente
      bool logged;
};

#endif