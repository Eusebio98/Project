# ProjectSO

Project Operating System 2018 - 2019

L'applicazione implementa un servizio di file transfer tra macchine client e server nel dominio AF_INET di IPV4. Nel file "main_server.c" è implementato il lato server che effettua prima una indicizzazione dei file all'interno della home, sfruttando le funzioni svilppate nella consegna 1 del progetto (file "step1.c" all'interno della repository), e poi si comporta come server TCP accettando connessioni da client remot e fornendo il servizio di file transfer. 

Come previsto nella consegna 1, l'indicizzazione dei file presenti nella home è stata implementata tramite l'utilizzo di N thread concorrenti, ove N è il numero di core della dell'eleboratore, e una lista globale contenente tutti i reativi pathname dei file presenti nella home a cui i vari thread accedono tramite un accesso mutuamente esclusivo. Nella consegna 2 è stato ripreso il codice, adattandolo alle esigenze della nuova applicazione.

Il file "main_client.c" implementa invece il lato client, il quale richiede una connessione TCP al server il cui indirizzo deve essere precisato (vi è un apposito #define SERVER_ADDRESS a inizio codice) e successivamente fornisce una interfaccia utente che, proprio come la shell di bash, accetta specifici comandi con una specifica sintassi per permettere una corretta comunicazione con il server.

Prima di commentare le funzionalità dell'applicazione, è bene precisare due importanti aspetti pratici, senza il quale non è possibile usufruire a pieno del servizio realizzato:
1) il server TCP deve essere mandato in esecuzione come amministratore tramite il comando sudo, in quanto nel servizio di upload di un file dal client verso il server, questò verrà caricato nella /home del server e se non si hanno gli opportuni permessi per la creazione di un file nella /home, il programma segnalerà un errore e chiudere il client.
2) l'autenticazione di un utente autorizzato avviene tramite l'invio di due stringhe senza spazi, valide come username e password che vengono confrontate dal server in un file "user_pass.txt" situato in /home (path --> "/home/user_pass.txt"). L'assenza di questo file .txt nel server preclude qualsiasi azione da parte del client che si vedrà rifiutata la connessione TCP. Sarà bene creare il file con le opportune autorizzazioni prima di avviare il server.

Come già anticipato, l'autenticazione utente è stata implementata con un meccanismo di confronto tra stringhe (non cifrate) risiedenti in un file di testo e oppurtunatamente inviate dal client al server all'atto di una connessione. 
Un utente non registrato può mandare un username e una password e se il server si accorge che si tratta di un nuovo utente, chiederà al client la sua volontà nella creazione di un nuovo profilo.
Un possibile futuro miglioramento dell'applicazione potrebbe consistere nella implementazione di un meccanismo di autenticazione più sicuro, con funzioni di hash e lo scambio di certificati per la comunicazione tra client e server, un pò come avviene nelle reti internet con il servizio HTTPS.

Il client può così mandare una serie di comandi al server, il quale li elaborerà e restituira le risposte in funzione della richiesta ricevuta. Una volta autenticato, il server manda al client la lista dei path disponibili conservata nella lista globale, ricavata dalle funzioni implementate nella consegna 1. Il client si mette in ascolto e ricevo fin quando il server non invia una particolare stringa di escape, "escape1234", che segnale la fine dell'invio di informazioni. Successivamente il client può usufruire delle seguenti funzionalità:

 - FUNZIONE DI SEARCH  --> [sintassi: search <file>] con cui il client invia una parola chiave <file> al server. Quest'ultimo ricercherà nella lista di file la parola chiave e manderà al client solo i pathname che la contengono.
  
- FUNZIONE DI DOWNLOAD --> [sintassi: download <file_path>] con cui il client seleziona un <path_name> tra quelli disponibili e richiede il trasferimento del file nella directory corrente. Il server invierà uno stream di byte che il client riceverà e scriverà su un file opportunatamente create nella working directory (dovrà essere inserito dal client il nome con cui verrà salvato il file scaricato).

- FUNZIONE DI UPLOAD --> [sintassi: upload <file_path>] con cui il client immette un path_name relativo ad un file presente nell'elaboratore e ne richiede l'invio al server. Se il pathname risulta corretto, il file viene aperto e verrà richiesto al client con quale nome memorizzare il file sul server (il file viene memorizzato nella /home, per questo il sever necessita delle opportune autorizzazione tramite il comando sudo quando viene lanciato). Alla fine il server riceverà uno stream di dati e creerà un nuobo file come richiesto in /home, aggiornando successivamente la lista dei file disponibili e inviandola nuovamente al client come conferma dell'avvenuto caricamento.

- FUNZIONE DI LIST --> [sintassi: list] con cui il client richiede un nuovo invio dei file disponibili per le operazioni di search e download così come descritto sopra.

- FUNZIONE DI EXIT --> [sintassi: exit] con il quale il client chiude la connessione e termina la propria esecuzione.

Altri comandi non sono accettati e se immessi verranno segnalati degli opportuni errori all'utente. Inoltre ogni volta che viene inserito un pathname nelle funzioni di download e upload, se questo non verrà riconosciuto o se il server riscontrerà dei problemi nell'invio o nella ricezione dei dati, degli opporuni errori verranno sengalati al client e se si tratta di fatal error, la connessione verrà abortita dal server, che chiudendo il socket di comunicazione si renderà disponibile per accettare altre connessioni da altri client.

Un aspetto da non trascurare  nella comunicazione tra client e server riguarda l'invio di messaggi consecutivi. L'applicazione è stata realizzata in maniera tale che ad ogni messaggio inviato dal server al client(o viceversa), questi si aspetti una risposta dal destinatario ("ok"), in maniera tale da sincronizzare i due lati della comunicazione ed evitare spiacevoli sovrapposizioni di dati in ricezione, che avrebbero alterato le funzionalità dell'applicazione.



