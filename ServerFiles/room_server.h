#ifndef ROOM_SERVER_H
#define ROOM_SERVER_H
#include "server_utils.h"

/**
 * Server gestore di una singola partita con due clienti
 * @param roomIndex Il codice della stanza che questo processo dovrà gestire
 * @param LoggedUsersTable Tabella di fd-username dei client connessi
*/
void ROOM_SERVER(int roomIndex, struct loggedUser *LoggedUsersTable);

#endif