const char *client_usage = "Numero di argomenti errato e/o numero di porta inserito non valido, uso corretto: \n\n./client \033[3mport\033[m\n";
const char *error_command_not_found = "Comando non riconosciuto";
const char *error_command_wrong_argc = "Numero di argomenti errato per il comando scelto, uso corretto:";
const char *insert_username = "Inserisci il tuo username: ";
const char *insert_password = "Inserisci la tua password: ";

enum LOG_OPTIONS{
      LOGIN =           (int)'L',
      REGISTRATION =    (int)'R',
      QUIT =            (int)'q',
};
const int NUM_LOG_OPTIONS = 3;
const char *client_initial_options_head = "Effettua il login o la registrazione:";
const char *client_initial_options[] = {
      "(1) digita \033[1m", "\033[m per effettuare il login",
      "(2) digita \033[1m", "\033[m per effettuare la registrazione",
      "(3) digita \033[1m", "\033[m per chiudere l'applicazione"
};
const char client_initial_options_codes[] = {(char) LOGIN, (char) REGISTRATION, (char) QUIT};

const char *app_description = {
      "\n\t\033[3mBenvenuto nell'\033[1mescape room!\033[m\nL'obiettivo di questo gioco è fuggire dalla stanza completando varie azioni e risolvendo enigmi entro un tempo massimo. Nella tua avventura sarai accompagnato da altri utenti, con i quali dovrai comunicare e collaborare per risolvere i misteri più complicati.\nPer completare la tua missione dovrai esplorare la stanza (comando \033[1mlook\033[m), prendere gli oggetti che ti troverai davanti (comando \033[1mtake\033[m) e cercare di usarli con altri oggetti (comando \033[1muse\033[m) per sbloccarne di nuovi.\nLa missione sarà completata quando tu e gli gli altri partecipanti avrete raccolto tutti i \033[1mTOKEN\033[m presenti nella stanza, ma fallirete se il \033[1mTIMER\033[m finirà."
};

const int NUM_COMMANDS = 11;
const char *COMMANDS_name[] = { "start",  "look",     "take",     "drop",     "use",      "objs",     "msg",      "leave",     "end",      "logout",   "help"};
const int COMMANDS_minArgs[] = {1,        0,          1,          1,          1,          0,          1,          0,          0,          0,          0};
const int COMMANDS_maxArgs[] = {1,        1,          1,          1,          2,          0,          1,          0,          0,          0,          0};
const char* COMMANDS_head = "Elenco dei comandi disponibili:";
const char* COMMANDS_printable[] = {
      "\033[1mstart\033[m \033[3mroom\033[m",  
      "\033[1mlook\033[m \033[3m[location | object]\033[m", 
      "\033[1mtake\033[m \033[3mobject\033[m",  
      "\033[1mdrop\033[m \033[3mobject\033[m",  
      "\033[1muse\033[m \033[3mobject1 [object2]\033[m",
      "\033[1mobjs\033[m\033[0m\033[m",   
      "\033[1mmsg\033[m \033[3musername\033[m",   
      "\033[1mleave\033[m\033[0m\033[m",
      "\033[1mend\033[m\033[0m\033[m",
      "\033[1mlogout\033[m\033[0m\033[m",
      "\033[1mhelp\033[m\033[0m\033[m",
};
const char* COMMANDS_description[] = {
      "Avvia il gioco nella stanza il cui codice (\033[3mroom\033[m) è specificato come parametro. Il codice è un numero intero.",
      "Fornisce una breve descrizione della stanza con le sue locazioni. Se viene specificato un argomento (una locazione \033[3mlocation\033[m o un oggetto \033[3mobject\033[m) fornisce una descrizione più dettagliata.",
      "Raccogli l'oggetto specificato se sbloccato. Se l'oggetto è bloccato può fornire una breve descrizione del blocco oppure mostrare un enigma da risolvere.",
      "Rilascia l'oggetto specificato dall'inventario",
      "Utilizza un oggetto raccolto. Se viene specificato anche un secondo oggetto utlizza il primo oggetto sul secondo.",
      "Lista degli oggetti correntemente nall'inventario",
      "Permette di un messaggio ad un partecipante. Per inviare un messaggio a tutti '*' indicare come destinatario. Il messaggio sarà specificato dopo aver inserito il comando.",
      "Esce dalla partita corrente",
      "Termina la partita corrente ed effettua la disconnessione dal server",
      "Effettua il logout e torna alla schermata di login",
      "Stampa nuovamente i comandi a video"
};
const bool management_command[] = {
      true,
      false,
      false,
      false,
      false,
      false,
      false,
      false,
      true,
      true,
      true,
};
const bool game_command[] = {
      false,
      true,
      true,
      true,
      true,
      true,
      true,
      true,
      true,
      false,
      true,
};

enum COMMANDS{
      START,
      LOOK,
      TAKE,
      DROP,
      USE,
      OBJS,
      MSG,
      LEAVE,
      END,
      LOGOUT,
      HELP,
};
