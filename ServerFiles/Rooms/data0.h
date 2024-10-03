#ifndef DATA0_H
#define DATA0_H

#include "room_structures.h"

#define PLAYERS_PER_GAME_R0 1
#define GAME_DURATION_R0 10*60
#define TOKEN_GOAL_R0 2
#define INVENTORY_DIM_R0 3

// ************************************* Locazioni
#define R0_locations_count 5
const struct Location LOCATION_R0[R0_locations_count] = {
      {"",              "Ci sono solo due piccole stanze in questa casa abbandonata: il \033[1m++salotto++\033[0m e una \033[1m++camera++\033[0m"},
      {"salotto",       "Al centro del salotto, affiancato ad un grande \033[1m++divano++\033[0m, c`è un \033[1m++mobiletto++\033[0m di epoca barocca, mentre il lungo muro giallo è interrotto solo da una piccola porta protetta da una \033[1m**serratura**\033[0m"},
      {"divano",        "Nascosta sotto al divano si riesce a distinguere una piccola \033[1m**chiave**\033[0m rugginosa"},
      {"mobiletto",     "Appoggiato sopra al mobiletto c`è un \033[1m**giornale**\033[0m"},
      {"camera",        "Il letto ormai mangiato da chissà quali animali è accanto ad una piccola \033[1m**cassettiera**\033[0m ammuffita dalle infiltrazioni del muro alla quale è poggiata"},
};

// ************************************* Oggetti 

#define R0_objects_count 6
struct Object OBJECTS_R0[R0_objects_count] = {
      {
      .name = "giornale",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 0,
      .description = {
            "Il titolo del giornale, datato 17/02/1996, legge: EVACUATE LE VOSTRE CASE.", 
            "Il titolo del giornale, datato 17/02/1996, legge: EVACUATE LE VOSTRE CASE."}},
      {
      .name = "chiave",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = false, .blockingRiddle = 0, // Data di abbandono della casa
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 0,
      .description = {
            "Per prendere questa chiave avrai bisogno di risolvere un enigma", 
            "Una chiave vecchia e un po` rugginosa, ma forse ancora funzionante"}},
      {
      .name = "serratura",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = false, .blockingObject = 1, // chiave
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 1,
      .description = {
            "Ormai vecchia e rovinata ma ancora funzionante", 
            "E' finalmente aperta dopo tutti questi anni"}},

      {
      .name = "lettera",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 0,
      .description = {
            "12 maggio 1995\n\"Cara Silvia,\nRicordo con affetto quella giornata di vento e sole durante l'estate di tre anni fa quando decidemmo di affrontare un'avventura a bordo di una barca a vela. Ci svegliammo all'alba, prontamente caricate di provviste e di aspettative. La barca, con le sue vele svolazzanti, ci portò lungo la costa mentre ci concedevamo la libertà di esplorare acque sconosciute.\nIl riflesso del sole sull'acqua e il suono leggero delle onde ci accompagnarono mentre ci dirigevamo verso un'isolata baia nascosta. Lì gettammo l'ancora e decidemmo di fare un picnic sulla coperta della barca. Condividemmo racconti, sorseggiando caffè caldo e godendoci il panorama mozzafiato.\nCi lasciammo trasportare in un luogo dove il tempo sembrava scorrere più lentamente, un'oasi di pace e serenità. Passammo la giornata a navigare e a contemplare il cielo infinito, consapevoli di quanto il nostro viaggio fosse unico e prezioso.\nQuella barca a vela, con il suo stupendo nome \'Njord\' inciso nei ricordi di quell'estate, è diventata un simbolo delle nostre avventure condivise. Guardo indietro a quei momenti con un sorriso, grata di aver vissuto la magia di quella giornata e della tua amicizia.\nCon affetto,\n\nSofia\"\n", 
            "12 maggio 1995\n\"Cara Silvia,\nRicordo con affetto quella giornata di vento e sole durante l'estate di tre anni fa quando decidemmo di affrontare un'avventura a bordo di una barca a vela. Ci svegliammo all'alba, prontamente caricate di provviste e di aspettative. La barca, con le sue vele svolazzanti, ci portò lungo la costa mentre ci concedevamo la libertà di esplorare acque sconosciute.\nIl riflesso del sole sull'acqua e il suono leggero delle onde ci accompagnarono mentre ci dirigevamo verso un'isolata baia nascosta. Lì gettammo l'ancora e decidemmo di fare un picnic sulla coperta della barca. Condividemmo racconti, sorseggiando caffè caldo e godendoci il panorama mozzafiato.\nCi lasciammo trasportare in un luogo dove il tempo sembrava scorrere più lentamente, un'oasi di pace e serenità. Passammo la giornata a navigare e a contemplare il cielo infinito, consapevoli di quanto il nostro viaggio fosse unico e prezioso.\nQuella barca a vela, con il suo stupendo nome \'Njord\' inciso nei ricordi di quell'estate, è diventata un simbolo delle nostre avventure condivise. Guardo indietro a quei momenti con un sorriso, grata di aver vissuto la magia di quella giornata e della tua amicizia.\nCon affetto,\n\nSofia\"\n", 
            }},
      {
      .name = "gioiello",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 0,
      .description = {
            "Un meraviglioso gioiello azzurro, probabilmente di zaffiro o topazio, dalla forma esagonale", 
            "Un meraviglioso gioiello azzurro, probabilmente di zaffiro o topazio, dalla forma esagonale"}},
      {
      .name = "cassettiera",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = false, .blockingRiddle = 1,
      .objectUnlocked = false, .blockingObject = 4, // gioiello
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 1,
      .description = {
            "Sembra che tutto sia stato riposto al suo interno, chiuso da una strana serratura a forma esagonale, tranne un meraviglioso \033[1m**gioiello**\033[0m azzurro abbandonato sopra", 
            "Sul fondo si scorgono gioielli di una miriade di colori diversi"}},
};



// ************************************* Enigmi 

#define R0_riddles_count 2
struct Riddle RIDDLES_R0[R0_riddles_count] = {
      {
            .text = "Sembra che i vecchi abitanti di questa casa aggiornassero il codice della cassaforte ogni mese, impostandolo al mese e anno corrente nel formato MM/AA.\nQuale sarà quindi il codice per sbloccare la cassaforte?", 
            .riddleType = SingleAnswer,
            .structures.answer = "02/96",
            .token = 0,
      },
      {
            .text = "Come si chiamava la barca a vela sulla quale Sofia ha passato l'estate del '92?", 
            .riddleType = SingleAnswer,
            .structures.answer = "Njord",
            .token = 0,
      },
};

#endif