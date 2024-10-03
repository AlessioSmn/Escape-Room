#ifndef DATA2_H
#define DATA2_H

#include "room_structures.h"

#define PLAYERS_PER_GAME_Ri 1
#define GAME_DURATION_Ri 600 // 10 minuti
#define TOKEN_GOAL_Ri 0
#define INVENTORY_DIM_Ri 0


// ************************************* Locazioni
#define Ri_locations_count 2
const struct Location LOCATION_Ri[Ri_locations_count] = {
      {"",              "-main_description"},
      {"-name",         "-location_description"},
};

// ************************************* Oggetti 

#define Ri_objects_count 1
struct Object OBJECTS_Ri[Ri_objects_count] = {
      {
      .name = "-name",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 0,
      .description = {
            "-description_locked", 
            "-description_unlocked"}},
};



// ************************************* Enigmi 

#define Ri_riddles_count 2
struct Riddle RIDDLES_Ri[Ri_riddles_count] = {
      {
            .text = "-text", 
            .riddleType = SingleAnswer,
            .structures.answer = "-answer",
            .token = 0,
      },
      {
            .text = "-text",
            .riddleType = ObjectSequence,
            .structures.sequenceRiddle = {
                  .objectCounter = 3,
                  .objectsSequence = (const int[3]){0, 1, 2},
                  .currentIndex = 0,
                  .currentSequence = (int[3]){-1, -1, -1}
            },
            .token = 0,
      },
};

#endif