#ifndef ROOM_STRUCTURES_H
#define ROOM_STRUCTURES_H
#include <stdbool.h>

/**
 * Rappresenta una locazione dell'escape room
 * @param name Il nome della locazione
 * @param description La descrizione della locazione
*/
struct Location{

      // Il nome della locazione
      const char *name;

      // La descrizione della locazione
      const char *description;
};

#define NO_BLOCKING_RIDDLE -1
#define NO_BLOCKING_OBJECT -1
#define NO_PART_OF_RIDDLE  -1
/**
 * Rappresenta un oggetto dell'escape room.
 * @attention Un oggetto potrà essere bloccato al massimo da UN enigma e UN altro oggetto
 * @param name il nome dell'oggetto
 * @param collectible definisce se l'oggetto può essere preso o meno (solo utilizzato)
 * @param collected definisce se l'oggetto è stato preso o meno
 * @param blockingRiddle Indice dell'eventuale engima bloccante
 * @param riddleUnlocked Se bloccato da enigma indica se l'enigma bloccante è stato risolto, altrimenti è true
 * @param blockingObject Indice dell'eventuale oggetto bloccante
 * @param objectUnlocked Se bloccato da oggetto indica se l'oggetto bloccante è stato utilizzato, altrimenti è true
 * @param partOfRiddle Indice dell'eventuale enigma a sequenza di oggetti del quale questo oggetto fa parte
 * @param token Numero di token dati al giocatore che raccoglie l'oggetto
 * @param description le due descrizioni dell'oggetto: [0] -> bloccato, [1] -> sbloccato
*/
struct Object{

      // il nome dell'oggetto
      const char *name;

      // definisce se l'oggetto può essere preso o meno (solo utilizzato)
      bool collectible;

      // definisce se l'oggetto è stato preso o meno
      bool collected;

      // Indice dell'eventuale engima bloccante
      // Se uguale a -1 non è bloccato da enigma
      const int blockingRiddle;

      // Se bloccato da enigma: indica se l'eventuale enigma bloccante è stato risolto
      // Altrimenti: true
      bool riddleUnlocked;

      // Indice dell'eventuale oggetto bloccante
      // Se uguale a -1 non è bloccato da oggetto
      const int blockingObject;

      // Se bloccato da oggetto: indica se l'eventuale oggetto bloccante è stato utilizzato
      // Altrimenti: true
      bool objectUnlocked;

      // Indice dell'eventuale enigma a sequenza di oggetti del quale questo oggetto fa parte
      const int partOfRiddle;

      // Numero di token dati al giocatore che raccoglie l'oggetto
      // Non const perchè va azzerato quando preso la prima volta, altrimenti si potrebbe 
      // alternare la sequenza TAKE - DROP per prendere token dallo stesso oggetto
      int token;

      // La descrizione dell'oggetto
      // Nella prima posizione c'è la descrizione da bloccato
      // Nella seconda posizione c'è la descrizione da sbloccato
      const char *description[2];
};


/**
 * Distingue i tipi di enigmi
*/
enum RiddleType{
      SingleAnswer,
      ObjectSequence
};

/**
 * Rappresenta un enigma dell'escape room
 * @param text Il testo dell'enigma
 * @param riddleType Il tipo dell'enigma
 * @param structures (union) Strutture dati distinte a seconda del tipo di enigma
 * @param token Numero di token assegnati al gioctaore che risolve l'enigma
*/
struct Riddle{

      // Il testo dell'enigma
      const char *text;

      // Il tipo dell'enigma
      enum RiddleType riddleType;

      // Struttre dati distinte a seconda del tipo di enigma
      union{

            // La risposta all'enigma
            const char *answer;

            // Informazioni realtive ad un enigma di tipo ObjectSequence
            struct SequenceRiddle{

                  // Il numero di oggetti bloccanti
                  const int objectCounter;

                  // Oggetti bloccanti elencati in ordine
                  const int* objectsSequence;

                  // Indice del primo slot libero della sequenza corrente
                  int currentIndex;

                  // 'Buffer' della sequenza correntemente inserita
                  int* currentSequence;

            } sequenceRiddle;
      } structures;

      // Token assegnati alla soluzione dell'enigma
      int token;
};

#endif