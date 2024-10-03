#ifndef DATA2_H
#define DATA2_H

#include "room_structures.h"

#define PLAYERS_PER_GAME_R2 2
#define GAME_DURATION_R2 20*60
// sono diposti 7 token escluso il tesoro, 
// quindi metto il goal a 8 per assicurarmi che il tesoro venga preso per la vittoria
// ma consento anche di non raccogliere tutti i token prima del tesoro
#define TOKEN_GOAL_R2 10
#define INVENTORY_DIM_R2 3


// ************************************* Locazioni
#define R2_locations_count 6
const struct Location LOCATION_R2[R2_locations_count] = {
      {"",              "Vi trovate su una sperduta isola in mezzo al mare dei Caraibi. Dietro una nebbia perpetua, avvolto in un'atmosfera mistica, si nasconde il \033[1m++porto++\033[0m dei pirati. Una \033[1m++grotta++\033[0m nascosta, ricca di conchiglie e coralli, svela un labirinto di segreti, raggiungibile solo da un \033[1m++ponte++\033[0m sospeso costruito con materiali di naufragi. Sulla \033[1m++spiaggia++\033[0m deserta giace un antico \033[1m**relitto**\033[0m di una piccola scialuppa naufragata centinaia di anni fa"},
      {"porto",         "Isola circondata da acque turchesi tra le quali galleggia una vecchia \033[1m**bottiglia**\033[0m, con un porto naturale ai piedi di maestose scogliere. Nascosto tra i banchi di sabbia, il porto dei pirati è il rifugio perfetto per le navi che cercano riparo. Qualcuno sembra aver dimenticato un \033[1m**chiavistello**\033[0m."},
      {"ponte",         "Un ponte di corda teso tra due scogli giganti, offrendo una vista panoramica sull'isola. Leggende parlano di antiche battaglie tra pirati che si sfidavano per il controllo della grotta, dove si celava un tesoro immenso. Incastrata tra due assi di legno si scorge una \033[1m**lettera**\033[0m ormai sgualcita"},
      {"grotta",        "Una serie intricata di grotte sotterranee, con passaggi segreti, stalattiti illuminate da lucciole e uno stretto \033[1m++anfratto++\033[0m dal quale sembra provenire della luce. La Grotta Nascosta è nota solo ai pirati più audaci, che la utilizzano per nascondere tesori e segreti in una \033[1m**cassa**\033[0m ben riparata."},
      {"anfratto",      "All'interno di questa piccola parte della grotta sono stranamente disposti in ordine tre elementi: un \033[1m**bastone**\033[0m, una \033[1m**lanterna**\033[0m ed una \033[1m**torcia**\033[0m. In un angolo in penombra sono nascoste delle piccole \033[1m**sculture**\033[0m"},
      {"spiaggia",      "Una spiaggia dominata da un antico \033[1m**tesoro**\033[0m di una nave arenato sulla riva. Attorno al tesoro si distinguono i resti di un \033[1m**relitto**\033[0m, diventato un luogo di rituale per i pirati, che vi lasciano offerte per placare il mare tempestoso e ricordare le loro famiglie"},
};

// ************************************* Oggetti 

#define R2_objects_count 12
struct Object OBJECTS_R2[R2_objects_count] = {
      // oggetti del porto
      {
      .name = "bottiglia",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 1,
      .description = {
            "All'interno della bottiglia è stato messo un messaggio:\n\t\"Mia Amata,\nIn questa terra selvaggia e deserta, la mia mente si volge a te. Affronto l'ignoto con coraggio, sapendo che tu, con il tuo amore e la tua forza, proteggerai la nostra famiglia, Humphrey e il piccolo Percival. Sii la roccia su cui si infrangono le tempeste, mentre io sogno di ritornare a te.\nCon tutto il mio cuore,\n\t\tPatchy John\"",
            "All'interno della bottiglia è stato messo un messaggio:\n\t\"Mia Amata,\nIn questa terra selvaggia e deserta, la mia mente si volge a te. Affronto l'ignoto con coraggio, sapendo che tu, con il tuo amore e la tua forza, proteggerai la nostra famiglia, Humphrey e il piccolo Percival. Sii la roccia su cui si infrangono le tempeste, mentre io sogno di ritornare a te.\nCon tutto il mio cuore,\n\t\tPatchy John\"",}},
      {
      .name = "chiavistello",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = false, .blockingRiddle = 2, // nome del figlio
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 0,
      .description = {
            "-description_locked", 
            "-description_unlocked"}},    

      // oggetti del ponte 
      {
      .name = "lettera",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 1,
      .description = {
            "\t\"Caro Capitano Aswin Compton,\nNella solitudine di quest'isola deserta, scrivo con cuore gravato dall'incertezza. La nostra nave, che un tempo danzava sulle onde, riposa qui su questa spiaggia desolata. Il destino ci ha portato a incontrare un naufragio inaspettato. Se tu mai dovessi recarti su questo sperduto angolo di mondo accendi e spegni la lanterna, accendi la torcia e alza il bastone: così il il ricordo delle nostre anime svelerà il mistero solo agli occhi che sanno vedere\nTi prego, Capitano, di prenderti cura della mia famiglia. Mia cara moglie Vivien, nelle tue mani forti e amorevoli, affido il mio bene più prezioso. Che la tua saggezza sia la bussola che guida il loro cammino, e che la tua forza sia il vento che gonfia le vele del loro coraggio.\n\t\tPatchy John\"",
            "\t\"Caro Capitano Aswin Compton,\nNella solitudine di quest'isola deserta, scrivo con cuore gravato dall'incertezza. La nostra nave, che un tempo danzava sulle onde, riposa qui su questa spiaggia desolata. Il destino ci ha portato a incontrare un naufragio inaspettato. Se tu mai dovessi recarti su questo sperduto angolo di mondo accendi e spegni la lanterna, accendi la torcia e alza il bastone: così il il ricordo delle nostre anime svelerà il mistero solo agli occhi che sanno vedere\nTi prego, Capitano, di prenderti cura della mia famiglia. Mia cara moglie Vivien, nelle tue mani forti e amorevoli, affido il mio bene più prezioso. Che la tua saggezza sia la bussola che guida il loro cammino, e che la tua forza sia il vento che gonfia le vele del loro coraggio.\n\t\tPatchy John\"", }},
      
      // oggetti dell'anfratto
      {
      .name = "bastone",
      .collectible = false,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = 4,
      .token = 0,
      .description = {
            "-description_locked", 
            "-description_unlocked"}},
      {
      .name = "lanterna",
      .collectible = false,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = 4,
      .token = 0,
      .description = {
            "-description_locked", 
            "-description_unlocked"}},  
      {
      .name = "torcia",
      .collectible = false,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = 4,
      .token = 0,
      .description = {
            "-description_locked", 
            "-description_unlocked"}},  
      {
      .name = "sculture",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = false, .blockingRiddle = 4, // sequenza di oggetti
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 1,
      .description = {
            "Per prendere queste sculture dovrai risolvere un enigma", 
            "Se illuminate dalla fioca luce della torcia poco distante queste creazioni ricordano le bianche scogliere di Dover, forse un ricordo dei mari spesso solcati dai pirati al rietro nella loro terra"}},  

      // oggetti della grotta
      {
      .name = "cassa",
      .collectible = false,
      .collected = false,
      .riddleUnlocked = false, .blockingRiddle = 3, // dove passavano i pirati
      .objectUnlocked = false, .blockingObject = 1, // chiavistello
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 0,
      .description = {
            "I segreti contenuto da questa cassa potranno essere rivelati solo a chi ben conosceva questi pirati segnati dal destino così crudele", 
            "La cassa è piena di collane dorate e perle splendenti, mentre sul fondo della cassa si distingue la forma della cattedrale di Rochester"}},           

      // oggetti della spiaggia
      {
      .name = "relitto",
      .collectible = false,
      .collected = false,
      .riddleUnlocked = true, .blockingRiddle = NO_BLOCKING_RIDDLE,
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 0,
      .description = {
            "Il relitto di una scialuppa, risvegliato dall'incessante ritmo delle onde, sembra conservare storie dimenticate. Frammenti di \033[1m**vele**\033[0m lacerate e \033[1m**attrezzi**\033[0m di navigazione rotti fanno rivivere il passato, mentre il vento sussurra le gesta di marinai scomparsi", 
            "Il relitto di una scialuppa, risvegliato dall'incessante ritmo delle onde, sembra conservare storie dimenticate. Frammenti di \033[1m**vele**\033[0m lacerate e \033[1m**attrezzi**\033[0m di navigazione rotti fanno rivivere il passato, mentre il vento sussurra le gesta di marinai scomparsi"}},
      {
      .name = "vele",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = false, .blockingRiddle = 1, // nome del capitano
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 0,
      .description = {
            "Queste vele ingiallite furono un tempo usate per tracciare delle rotte di navigazione. In un angolo si legge: \"Otto furono le giornate di tempesta e otto furono gli uomini mai tornati\"", 
            "Queste vele ingiallite furono un tempo usate per tracciare delle rotte di navigazione. In un angolo si legge: \"Otto furono le giornate di tempesta e otto furono gli uomini mai tornati\"", }},
      {
      .name = "attrezzi",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = false, .blockingRiddle = 0, // sequenza di numeri 
      .objectUnlocked = true, .blockingObject = NO_BLOCKING_OBJECT,
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 1,
      .description = {
            "Dovrai risolvere un enigma per accedere a questi attrezzi", 
            "Un sestante di acciaio, lucidato dal vento marino, giace tra le sabbie della costa. Le sue linee eleganti ricordano le navigazioni intraprese lungo le coste dell'Inghilterra"}},
      
      // oggetto finale
      {
      .name = "tesoro",
      .collectible = true,
      .collected = false,
      .riddleUnlocked = false, .blockingRiddle = 5, // origine dei pirati 
      .objectUnlocked = false, .blockingObject = 10, // attrezzi
      .partOfRiddle = NO_PART_OF_RIDDLE,
      .token = 10,
      .description = {
            "Il prezioso tesoro custodito fino alla morte da Patchy John", 
            "Il prezioso tesoro custodito fino alla morte da Patchy John"}},
};



// ************************************* Enigmi 

#define R2_riddles_count 6
struct Riddle RIDDLES_R2[R2_riddles_count] = {
      {
            .text = "Il cofanetto che blocca gli attrezzi è bloccato da un codice: i numeri presenti sono 7, 15, 23, 31. Qual è il prossimo numero della sequenza?", 
            .riddleType = SingleAnswer,
            .structures.answer = "39",
            .token = 0,
      },
      {
            .text = "Come si chiamava il capitano del pirata spiaggiato?", 
            .riddleType = SingleAnswer,
            .structures.answer = "Aswin Compton",
            .token = 0,
      },
      {
            .text = "Come si chiamava il figlio minore del pirata spiaggiato?", 
            .riddleType = SingleAnswer,
            .structures.answer = "Percival",
            .token = 0,
      },
      {
            .text = "Dove passavano spesso i pirati al rientro dai loro viaggi verso terre remote?", 
            .riddleType = SingleAnswer,
            .structures.answer = "Dover",
            .token = 1,
      },
      {
            .text = "Sembra che gli elementi stranamente disposti sul lato dell'anfratto possano attivare qualche sorta di meccanismo...",
            .riddleType = ObjectSequence,
            .structures.sequenceRiddle = {
                  .objectCounter = 4,
                  .objectsSequence = (const int[4]){4, 4, 5, 3},
                  .currentIndex = 0,
                  .currentSequence = (int[4]){-1, -1, -1, -1}
            },
            .token = 1,
      },
      {
            .text = "Con ogni probabilità, di dove erano originari i pirati che qua hanno passato i loro ultimi giorni?", 
            .riddleType = SingleAnswer,
            .structures.answer = "Rochester",
            .token = 1,
      },
};

#endif