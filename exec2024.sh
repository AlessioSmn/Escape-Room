# ************************************************************************************
# NOTA: di seguito, tutti i device sono lanciati passando un numero di porta
# a linea di comando, per generalità. Se si decide che un device si comporta da client
# non è necessario recuperare la porta nel codice, né effettuare la bind().
# Supporre che i dispositivi contattino il server sulla porta 4242, inserendola
# staticamente nel codice sorgente.
# ************************************************************************************

# 1. COMPILAZIONE
# Il comando 'make' necessita del makefile, che deve essere
# creato come descritto nella guida sulla pagina Elearn

  make

  read -p "Compilazione eseguita. Premi invio per eseguire..."

# 2. ESECUZIONE
# I file eseguibili devono chiamarsi come descritto in specifica, e cioè:
#    a) 'server' per il server;
#    b) 'client' per il client;
#    c) 'other' per il terzo device.
# I file eseguibili devono essere nella current folder

# 2.1 esecuzione del server sulla porta 4242
  gnome-terminal -x sh -c "./server 4242; exec bash"

# Date le specifiche di progetto per l'avvio del server:
# start port 
# avvia il server sulla porta port. Dopo l'avvio, il server si mette in attesa di connessioni da parte dei giocatori
# E' necessario eseguire il comando start <port> prima di lanciare i due client,
# altrimenti la loro esecuzione termina immediatamente a causa di Connection refused
  echo "Main server creato in una nuova finestra di terminale, digitare in questa il comando:"
  echo " > start <port>"
  read -p  "Per creare il server di gioco ed accettare nuove connessioni, poi premi invio per eseguire i due client"

# 2.2 esecuzione del client sulla porta 6000
	gnome-terminal -x sh -c "./client 4242; exec bash"

# 2.3 esecuzione del terzo device sulla porta 6100
	gnome-terminal -x sh -c "./client 4242; exec bash"