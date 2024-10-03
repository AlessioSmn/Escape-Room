
#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include <stdbool.h>
#include <arpa/inet.h> // in_port_t

/**
 * Programma eseguito dal server di gioco (GAME SERVER)
 * @param Game_Server_Port Porta, specificata da start <port>, dove fare ascoltare le richieste 
 * @param pipe_GameToMain_W file descriptor dell'estremo della pipe GameToMain in scrittura
 * @param pipe_MainToGame_R file descriptor dell'estremo della pipe MainToGame in lettura
 * @param redirect_stdout Se true reindirizza lo stdout su unu file di log
*/
void GAME_SERVER(in_port_t Game_Server_Port, int pipe_GameToMain_W, int pipe_MainToGame_R, bool redirect_stdout);


#endif