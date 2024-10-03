#ifndef REQUEST_CODES_H
#define REQUEST_CODES_H

// si inviano solo stringhe brevi, meno di 255 caratteri
#define PIPE_MSG_DIM_MAX_LEN 1

#define USER_USERNAME_FIELD_DIM     63 + 1      // 63 caratteri + \0 
#define USER_PASSWORD_FIELD_DIM     63 + 1      // 63 caratteri + \0 


// ********************************************************************************************************

// Codici per ogni possibile tipo di richiesta da client e risposta da server
// fatte per usare solo la macro per maggiore leggibilità del codice
// ma permettere comunque di inviare un solo byte come primo messaggio

// NOTA
// Nella documentazione dei codici delle richieste il protocollo di comunicazione da utilizzare è elencato sotto i campi @note
// Si utilizzano le seguenti sigle:
// [C] - Client
// [S] - main Server
// [G] - Game server
// [R] - Room server

// ********************************************************************************************************


// ********************************************************************************************************
// *********************** Connessione, login e inizio partita (tra 0 e 15) ********************************
// ********************************************************************************************************

/**
 * Richiesta di Login
 * @attention Cominucazione tra [C] Client e [S] main Server
 * @note 1) [C] invia code: REQUEST_LOGIN
 * @note 2) [C] invia msg: <username>
 * @note 3) [C] invia msg: <password>
 * @note 4) [S] invia code: (risposta) RESPONSE_ACCESS_DENIED o RESPONSE_ACCESS_GRANTED
 * @note Se l'accesso è consentito (RESPONSE_ACCESS_GRANTED) 
 * @note 4) [S] invia msg: <game sever port>
*/
#define REQUEST_LOGIN               0x00

/**
 * Richiesta di Registrazione
 * @attention Cominucazione tra [C] Client e [S] main Server
 * @note 1) [C] invia code: REQUEST_REGISTRATION
 * @note 2) [C] invia msg: <username>
 * @note 3) [C] invia msg: <password>
 * @note 4) [S] invia code: (risposta) RESPONSE_ACCESS_DENIED o RESPONSE_ACCESS_GRANTED
 * @note Se l'accesso è consentito (RESPONSE_ACCESS_GRANTED) 
 * @note 4) [S] invia msg: <game sever port>
*/
#define REQUEST_REGISTRATION        0x01

// Accesso consentito in fase di login / registrazione
#define RESPONSE_ACCESS_DENIED      0x02
// Accesso negato in fase di login / registrazione (ad esempio per credenziali di login errate o username già occupato)
#define RESPONSE_ACCESS_GRANTED     0x03

/**
 * Richiesta di Logout
 * @attention Cominucazione tra [C] Client e [S] main Server
 * @note 1) [C] invia code: REQUEST_LOGOUT
*/
#define REQUEST_LOGOUT              0x04

/**
 * Richiesta di Disconnessione (da game o room server)
 * @attention Cominucazione tra [C] Client e [G] Game server
 * @attention Cominucazione tra [C] Client e [R] Rame server
 * @note 1) [C] invia code: COMMUNICATION_DISCONNECT
*/
#define COMMUNICATION_DISCONNECT    0x05

/**
 * Richiesta di Disconnessione
 * @attention Cominucazione tra [C] Client e [S] main Server
 * @note 1) [C] invia code: RESPONSE_ACCESS_DENIED
*/
#define REQUEST_END_CONNECTION      0x06


// ********************************************************************************************************
// *********************** Comandi di inizio gioco (tra 16 e 31) ******************************************
// ********************************************************************************************************

/**
 * Richiesta di Inizio partita. Il game server risponde RESPONSE_START_OK se la richiesta è avvenuta con successo
 * ma non ci sono ancora abbastanza clienti per iniziare la partita, mentre risponde con RESPONSE_START_OK_NOW.
 * se la partita può essere iniziata in quell'istante
 * @attention Cominucazione tra [C] Client e [G] Game server
 * @note 1) [C] invia code: REQUEST_START_GAME
 * @note 2) [C] invia msg: <codice della stanza>
 * @note 3) [C] invia msg: <username>
 * @note 4) [G] invia code: (risposta) RESPONSE_START_OK_NOW | RESPONSE_START_OK | RESPONSE_START_ERR
*/
#define REQUEST_START_GAME          0x10
// REQUEST_START_GAME avvenuta con successo
#define RESPONSE_START_OK           0x11
// REQUEST_START_GAME avvenuta con successo, avvio istantaneo
#define RESPONSE_START_OK_NOW       0x12
// REQUEST_START_GAME errore nell'inizio di una partita (ad esempio codice non valido)
#define RESPONSE_START_ERR          0x13

/**
 * Richiesta di annullamento inizio partita
 * @attention Cominucazione tra [C] Client e [G] Game server
 * @note 1) [C] invia code: REQUEST_START_ABORT
 * @note 2) [C] invia msg: <codice della stanza>
*/
#define REQUEST_START_ABORT         0x14

/**
 * Richiesta di informazioni sulle stanze disponibili
 * @attention Cominucazione tra [C] Client e [G] Game server
 * @note 1) [C] invia code: REQUEST_ROOM_INFO
 * @note 2) [G] invia msg: <numero di stanze N>
 * @note 3.1) [G] invia msg: <descrizione della prima stanza>
 * @note 3.2) [G] invia msg: <descrizione della seconda stanza>
 * @note 3.i) [G] invia msg: <descrizione della i-esima stanza>
 * @note 3.N) [G] invia msg: <descrizione della N-esima stanza>
*/
#define REQUEST_ROOM_INFO           0x15

// ********************************************************************************************************
// *********************** Comandi di gioco (tra 32 e 63) *************************************************
// ********************************************************************************************************

/**
 * Richiesta di Look
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note 1) [C] invia code: REQUEST_LOOK
 * @note 2) [C] invia msg: <nome della locazione / oggetto>
 * @note 3) [R] invia code: (risposta) (OK | not_poss | ERR)
 * @note Se la locazione / oggetto è presente: (RESPONSE_LOOK_OK) 
 * @note 4) [R] invia msg: <descrizione della locazione / oggetto>
*/
#define REQUEST_LOOK                0x20
// REQUEST_LOOK avvenuta con successo
#define RESPONSE_LOOK_OK            0x21
// REQUEST_LOOK su un oggetto collezionabile non posseduto
#define RESPONSE_LOOK_not_poss      0x22
// REQUEST_LOOK error (ad esempio locazione non esistente)
#define RESPONSE_LOOK_ERR           0x23

/**
 * Richiesta di Take
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note 1) [C] invia code: REQUEST_TAKE
 * @note 2) [C] invia msg: <nome dell'oggetto>
 * @note 3) [R] invia code: (risposta) [OK | inv_full | alr_taken | blocked | riddle | ERR]
 * @note Si vedano le varie possibili risposte per il prosieguo. Nota: tutte tranne riddle consistono in semplice invio di un codice da parte del room server
*/
#define REQUEST_TAKE                0x24
/**
 * Risposta a Take:
 * REQUEST_TAKE avvenuta con successo
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note Parte finale del protocollo di REQUEST_TAKE
*/
#define RESPONSE_TAKE_OK            0x25
/**
 * Risposta a Take:
 * REQUEST_TAKE possibile ma intentario pieno
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note Parte finale del protocollo di REQUEST_TAKE
*/
#define RESPONSE_TAKE_inv_full      0x26
/**
 * Risposta a Take:
 * REQUEST_TAKE possibile ma oggetto già preso
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note Parte finale del protocollo di REQUEST_TAKE
*/
#define RESPONSE_TAKE_alr_taken     0x27
/**
 * Risposta a Take:
 * REQUEST_TAKE su un oggetto non collezionabile
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note Parte finale del protocollo di REQUEST_TAKE
*/
#define RESPONSE_TAKE_not_coll      0x28
/**
 * Risposta a Take:
 * REQUEST_TAKE oggetto bloccato da altri oggetti
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note Parte finale del protocollo di REQUEST_TAKE
*/
#define RESPONSE_TAKE_blocked       0x29
/**
 * Risposta a Take:
 * REQUEST_TAKE error (ad esempio oggetto non esistente)
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note Parte finale del protocollo di REQUEST_TAKE
*/
#define RESPONSE_TAKE_ERR           0x2A
/**
 * Risposta a Take:
 * REQUEST_TAKE oggetto bloccato da un enigma a risposta singola
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note 1) Protocollo di REQUEST_TAKE
 * @note 2) [R] invia code: RESPONSE_TAKE_riddle_ans (Parte finale del protocollo di REQUEST_TAKE)
 * @note 3) [R] invia msg: <identificatore dell'enigma>
 * @note 4) [R] invia msg: <identificatore dell'oggetto>
 * @note 5) [R] invia msg: <testo dell'enigma>
*/
#define RESPONSE_TAKE_riddle_ans    0x2B

/**
 * Risposta a Take:
 * REQUEST_TAKE oggetto bloccato da un enigma di tipo sequenza di oggetti
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note 1) Protocollo di REQUEST_TAKE
 * @note 2) [R] invia code: RESPONSE_TAKE_riddle_seq (Parte finale del protocollo di REQUEST_TAKE)
 * @note 3) [R] invia msg: <identificatore dell'enigma>
 * @note 4) [R] invia msg: <identificatore dell'oggetto>
 * @note 5) [R] invia msg: <testo dell'enigma>
*/
#define RESPONSE_TAKE_riddle_seq    0x2C

/**
 * Richiesta di Risoluzione di un enigma a risposta singola
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note 1) [C] invia code: REQUEST_RIDDLE_ANS
 * @note 2) [C] invia msg: <identificatore dell'enigma>
 * @note 3) [C] invia msg: <identificatore dell'oggetto>
 * @note 4) [C] invia msg: <risposta proposta>
 * @note 5) [R] invia code: risposta (RESPONSE_RIDDLE_OK | RESPONSE_RIDDLE_ERR)
*/
#define REQUEST_RIDDLE_ANS          0x2D
// REQUEST_RIDDLE_* avvenuta con successo, risposta giusto e oggetto sbloccato
#define RESPONSE_RIDDLE_OK          0x2E
// REQUEST_RIDDLE_* error (ad esempio risposta sbagliata)
#define RESPONSE_RIDDLE_ERR         0x2F

/**
 * Richiesta di Risoluzione di un enigma di tipo sequenza di oggetti
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note 1) [C] invia code: REQUEST_RIDDLE_SEQ
 * @note 2) [C] invia msg: <identificatore dell'enigma>
 * @note 3) [C] invia msg: <identificatore dell'oggetto>
 * @note 4) [R] invia code: risposta (RESPONSE_RIDDLE_OK | RESPONSE_RIDDLE_ERR)
*/
#define REQUEST_RIDDLE_SEQ          0x30

/**
 * Richiesta di utilizzo di un oggetto
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note 1) [C] invia code: REQUEST_USE_ONE
 * @note 2) [C] invia msg: <nome dell'oggetto>
 * @note 3) [R] invia code: risposta (OK | denied | no_effect | ERR)
*/
#define REQUEST_USE_ONE             0x31

/**
 * Richiesta di utilizzo di un oggetto su un secondo oggetto
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note 1) [C] invia code: REQUEST_USE_TWO
 * @note 2) [C] invia msg: <nome dell'oggetto 1>
 * @note 3) [C] invia msg: <nome dell'oggetto 2>
 * @note 4) [R] invia code: risposta (OK | denied | no_effect | ERR)
*/
#define REQUEST_USE_TWO             0x32
// REQUEST_USE / TWO avvenuta con successo
#define RESPONSE_USE_OK             0X33
// REQUEST_USE / TWO chiamata con il primo oggetto collezionabile non posseduto dal chiamante
#define RESPONSE_USE_denied       0x34
// REQUEST_USE / TWO corretta ma senza alcun effetto
#define RESPONSE_USE_no_effect      0x35
// REQUEST_USE_ONE / TWO error (ad esempio l'utilizzo non ha avuto alcun effetto)
#define RESPONSE_USE_ERR            0x36

/**
 * Richiesta di Objs, visualizzazione dell'inventario
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note 1) [C] invia code: REQUEST_OBJS
 * @note 2) [R] invia msg: <inventario>
*/
#define REQUEST_OBJS                0x37

/**
 * Richiesta di Drop, rilascio di un oggetto dall'inventario
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note 1) [C] invia code: REQUEST_DROP
 * @note 2) [C] invia msg: <nome dell'oggetto>
 * @note 3) [R] invia code: risposta (RESPONSE_DROP_OK o RESPONSE_DROP_ERR)
*/
#define REQUEST_DROP                0x38
// REQUEST_DROP avvenuta con successo
#define RESPONSE_DROP_OK            0x39
// REQUEST_DROP error (ad esempio oggetto non posseduto)
#define RESPONSE_DROP_ERR           0x3A

/**
 * Richiesta di Message
 * @attention Cominucazione tra [C] Client e [R] Room server
 * @note 1) [C] invia code: REQUEST_MSG
 * @note 2) [C] invia msg: <username del destinatario>
 * @note 3) [C] invia msg: <messaggio>
 * @note 4) [R] invia code: risposta (OK | self | disconnected | ERR)
*/
#define REQUEST_MSG                 0x3B
// REQUEST_MSG avvenuta con successo, messaggio inviato al destinatario
#define RESPONSE_MSG_OK             0x3C
// REQUEST_MSG con mittente coiincidente con il destinatario
#define RESPONSE_MSG_self           0x3D
// REQUEST_MSG verso un destinatario disconnesso
#define RESPONSE_MSG_disconnected   0x3E
// REQUEST_MSG error (ad esempio destinatario non esistente)
#define RESPONSE_MSG_ERR            0X3F

// ********************************************************************************************************
// ******************** Comunicazioni da server a client (tra 64 e 79) ************************************
// ********************************************************************************************************

/**
 * Comunicazione di start avvenuta con successo
 * @attention Cominucazione tra [R] Room server e [C] Client
 * @note 1) [R] invia code: COMMUNICATION_STARTING_GAME
 * @note 2) [R] invia msg: <numero di token per la vittoria>
 * @note 3) [R] invia msg: <numero di partecipanti totali N+1>
 * @note 4) [R] invia N msg: username dei partecipanti:
 * @note 4.1) [R] invia msg: <username partecipante 1>
 * @note 4.2) [R] invia msg: <username partecipante 2>
 * @note 4.i) ...
 * @note 4.N) [R] invia msg: <username partecipante N>
 * @note 5) [R] invia msg: <durata della partita (in secondi)>
*/
#define COMMUNICATION_STARTING_GAME 0x40
/**
 * Comunicazione di start fallita per errore nella creazione del processo gestore della partita
 * @attention Cominucazione tra [G] Game server e [C] Client
 * @note 1) [G] invia code: COMMUNICATION_START_FAILED
*/
#define COMMUNICATION_START_FAILED  0x41

/**
 * Comunicazione di token guadagnati
 * @attention Cominucazione tra [R] Room server e [C] Client
 * @note 1) [R] invia code: COMMUNICATION_TOKEN
 * @note 2) [R] invia msg: <numero di token>
*/
#define COMMUNICATION_TOKEN         0x42

/**
 * Comunicazione di messaggio in ingresso
 * @attention Cominucazione tra [R] Room server e [C] Client
 * @note 1) [R] invia code: COMMUNICATION_MSG
 * @note 2) [R] invia msg: <username del mittente>
 * @note 3) [R] invia msg: <messaggio>
*/
#define COMMUNICATION_MSG           0x43

/**
 * Comunicazione di partita terminata per vittoria
 * @attention Cominucazione tra [R] Room server e [C] Client
 * @note 1) [R] invia code: COMMUNICATION_GAME_WIN
*/
#define COMMUNICATION_GAME_WIN      0x4E

/**
 * Comunicazione di partita terminata per timeout
 * @attention Cominucazione tra [R] Room server e [C] Client
 * @note 1) [R] invia code: COMMUNICATION_GAMEOVER
*/
#define COMMUNICATION_GAMEOVER      0x4F

#endif