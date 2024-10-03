#ifndef ROOM_DATA_H
#define ROOM_DATA_H

#include "data0.h"
#include "data1.h"
#include "data2.h"

#include "room_structures.h"
#include "../codes.h"


const char ROOM_DESCRIPTION[ROOM_COUNT][ROOM_DES_MAX_LEN] = {
      "Stanza tutorial per prendere confidenza con i comandi",
      "Stanza tutorial per imparare le basi della comunicazione ed esplorare un nuovo tipo di enigma",
      "Stanza a tema 'Isola dei pirati'",
};

// Numero di giocatori necessario per partita
const int PLAYERS_PER_GAME[ROOM_COUNT] = {
      PLAYERS_PER_GAME_R0,
      PLAYERS_PER_GAME_R1,
      PLAYERS_PER_GAME_R2,
};

// Durata della partita espressa in secondi
const int GAME_DURATION[ROOM_COUNT] = {
      GAME_DURATION_R0,
      GAME_DURATION_R1,
      GAME_DURATION_R2,
};

// Numero di token necessari per la vittoria
const int TOKEN_GOAL[ROOM_COUNT] = {
      TOKEN_GOAL_R0,
      TOKEN_GOAL_R1,
      TOKEN_GOAL_R2,
};

// Dimensione dell'inventario
const int INVENTORY_DIM[ROOM_COUNT] = {
      INVENTORY_DIM_R0,
      INVENTORY_DIM_R1,
      INVENTORY_DIM_R2,
};


// Tabella di locazioni
const struct Location *LOCATIONS[ROOM_COUNT] = {
      LOCATION_R0,
      LOCATION_R1,
      LOCATION_R2,
};
const int LOCATIONS_count[ROOM_COUNT] = {
      R0_locations_count,
      R1_locations_count,
      R2_locations_count,
};


// Tabella di oggetti
struct Object *OBJECTS[ROOM_COUNT] = {
      OBJECTS_R0,
      OBJECTS_R1,
      OBJECTS_R2,
};
const int OBJECTS_count[ROOM_COUNT] = {
      R0_objects_count,
      R1_objects_count,
      R2_objects_count,
};


// Tabella di enigmi
struct Riddle *RIDDLES[ROOM_COUNT] = {
      RIDDLES_R0,
      RIDDLES_R1,
      RIDDLES_R2,
};
const int RIDDLES_count[ROOM_COUNT] = {
      R0_riddles_count,
      R1_riddles_count,
      R2_riddles_count,
};

#endif