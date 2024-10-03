const char *server_usage = "Numero di argomenti errato e/o numero di porta inserito non valido, uso corretto: \n\n./server \033[3mport\033[m\n";
const char *server_init_error = "Errore nella fase di inizializzazione del server, provare con una porta diversa";
const char *error_command_not_found = "Comando non riconosciuto";
const char *error_command_wrong_argc = "Numero di argomenti errato per il comando scelto, uso corretto:";
const char *server_not_stoppable = "Il server non può essere interrotto in questo momento, ci sono ancora clienti connessi";
const char *server_stoppable = "Interruzione del server...";
const char *server_stopped = "Server terminato con successo!";
const char *server_newline_forcmd = "\n > ";

const char *COMMANDS_name[] = {"start", "stop", "info", "help"};
const char *COMMANDS_input_match[] = {"start\n", "stop\n", "info\n", "help\n"};
const int COMMANDS_minArgs[] = {1, 0, 0, 0};
const int COMMANDS_maxArgs[] = {1, 0, 0, 0};
const char* COMMANDS_head = "Elenco dei comandi disponibili:";
const char *COMMANDS_printable[] = {     
      "\033[1mstart\033[m\033[3m port\033[m",  
      "\033[1mstop \033[m\033[0m\033[m",
      "\033[1minfo \033[m\033[0m\033[m",
      "\033[1mhelp \033[m\033[0m\033[m"
};     
const char* COMMANDS_description[] = {
      "Avvia il server sulla porta \033[3mport\033[m",
      "Effettua la disconnessione del server se possibile",
      "Mostra varie informazioni sul server",
      "Elenca nuovamente la lista dei comandi"
};
const int NUM_COMMANDS = 4;

enum COMMANDS{
      START,
      STOP,
      INFO,
      HELP
};

