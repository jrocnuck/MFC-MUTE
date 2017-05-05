VERSIONE MFC/Win32 MUTE SOLO PER Windows .... -- translated by NGLWARCRY

----------------------------------------------------------------------------------------------------
--- version 0.0.7 -- 10-11-2005 release:------------------------------------------------------------
--- explanation of fixes/additions/alterations
----------------------------------------------------------------------------------------------------
1. Per coding help and motivation from Defnax and MCoder:
   a-> Changed tabs to XP Theme aware tabs.
   b-> Added status bar and removed old green/red bar from bottom of GUI.
   c-> Added headings above list boxes with counts.
   d-> Regrouped connections dialog controls
   e-> Added lots of nice looking button graphics
2. Increased performance for hashing by increasing chunk sizes being passed to SHA1 routines.
3. Decreased the time between automated queue searches.
4. Moved string resources out of MS Visual Studio automated resource files.  Now when new strings
   are added, the file won't get automatically modified by the compiler IDE.
5. When manually searching for a file and attempting to restart the download of a queued download,
   the remote HASH is also checked for a match against the queued file.  If the hash matches,
   then that file download is resumed immediately as long as the # of downloads from that VIP doesn't
   exceed the max downloads from 1 VIP.
   
----------------------------------------------------------------------------------------------------
--- versione 0.0.6 -- 08-13-2005 release:-----------------------------------------------------------
--- chiarimenti su bugfix/aggiunte/modifiche
----------------------------------------------------------------------------------------------------
1.  Aggiornato il core del programma con le patch di Jason Rohrer.
2.  Nell' About ora l' indirizzo web è http://www.sourceforge.net/projects/mfc-mute-net
3.  Per aumentare la sicurezza e per diminuire il rischio di attacchi, l' uso di wildcard '*' nelle ricerche
    è limitato e reso randomico.
4.  File come: desktop.ini e Thumbs.db non saranno più inclusi nei risultati delle ricerche.
    Non c'è nessun reale bisogno di scaricare questi file.
5.  Possibilità di cambiare le icone dei vari Tab all' avvio.  Per cambiare le icone è necessario copiarne di
    nuove a propria scelta nella cartella del programma e nominarle in base alla lista seguente:
          icons\searchTab.ico
          icons\downloadsTab.ico
          icons\uploadsTab.ico
          icons\connectionsTab.ico
          icons\sharedfilesTab.ico
          icons\settingsTab.ico
6.  Aggiunte le statistiche sul traffico della rete (per il momento solo in INGLESE)

----------------------------------------------------------------------------------------------------
--- version 0.0.5 -- 03-04-2005 release:-----------------------------------------------------------------------
--- chiarimenti su bugfix/aggiunte/modifiche
----------------------------------------------------------------------------------------------------
1.  Added colored bar across bottom of GUI that shows connection status.
2.  Put the elapsed time in the caption bar format DAYS:HOURS:MINUTES:SECONDS
3.  Added some code from Nate that saves out of order downloaded chunks to help speed up D/Ls.


----------------------------------------------------------------------------------------------------
--- versione 0.0.4 -- 02-24-2005 release:-----------------------------------------------------------------------
--- chiarimenti su bugfix/aggiunte/modifiche
----------------------------------------------------------------------------------------------------

1.  Cambiato il tooltip nel system tray da "MFC_MUTE_0.3" a "MFC MUTE"
2.  Sistemati 2 bug per il codice Download Queue.
    -1-->   bugfix: del caricamento dei file salvati in Downloadqueue.ini.
            Nella versione 0.0.3 non tutti i download in coda presenti nel file Downloadqueue.ini
            venivano caricati al successivo riavvio.
    -2-->   bugfix: quando un download in coda viene ricercato con successo, ma ci sono già 4
            download attivi dallo stesso VIP (Virtual IP), questo file verrà riposto nuovamente
            nello stato "in coda". Questo perchè solo 4 download contemporanei sono permessi da 1 VIP.
            In precedenza il file in questione si trovava "in coda" ma il suo stato non era aggiornato
            correttamente.            
3.  Aumentato il valore di timeout per il codice Download Queue.            
4.  Sistemato il codice nel Tab Cerca che aggiunge un file da scaricare quando questo è presente nei file
    in coda.  Dopo vari test della versione 0.0.3, ci si è resi conto che non funzionava e venivano 
    aggiunti file "doppioni" nella lista dei download.
5.  Nel Tab Download, quando è selezionato un file, con il tasto Cancella verrà usata la funzionalità annulla/elimina.
6.  Aumentato il valore di timeout per il codice Downloads Dialog.
7.  Aggiunto 1 millisecondo di attesa nel codice di aggiornamento nel Tab File condivisi per risparmio di CPU.
8.  Nel Tab Upload, quando è selezionato un file, con il tasto Cancella verrà usata la funzionalità annulla/elimina.    
9.  Bugfix relativo ai timeout/retries delle richieste dei chunk. Questo cambiamento dovrebbe aiutare la rete e
    prevenire che troppe richieste di chunk vengano inviate senza che sia lasciarto tempo sufficiente alle risposte
    di ritornare al client.
10. Tobiash ha aggiornato la traduzione danese.
11. Traduzione turca ad opera di Defnax.

----------------------------------------------------------------------------------------------------
--- versione 0.0.3 -- 02-09-2005 release:-----------------------------------------------------------------------
--- chiarimenti su bugfix/aggiunte/modifiche
----------------------------------------------------------------------------------------------------
1.  Aggiunte nuove icone e fatti alcuni cambiamenti alle altre. (--NGLWARCRY--) 
2.  Aggiornato il progetto per essere compilato con Visual Studio .NET.
3.  Quando viene chiuso il programma gli Ip degli Host vengono salvati nel file seedHosts.ini. (--Nate--/TSAFA)
4.  Sistemata la disposizione dello "stato" dei download.
5.  Aggiunta una nuova opzione nel menu contestuale nei download che permette ad un utente di scegliere un file "in coda"
    come il prossimo elemento ad essere cercato. (--Tony--)
6.  Aumentato il tempo tra la ricerca automatizzata dei file in download che si trovano "In Coda" e le richieste di
    ricerca per prevenire il flooding (ingolfo) della rete.
7.  Ogni volta che un file viene cancellato o completato, il file Downloadqueue.ini viene aggiornato al 
    nuovo stato. In questo modo si mantiene l' integrità del file nell' eventualità di crash del programma o 
    veloci chiusure dello stesso.
8.  Se nel Tab Cerca viene trovato un file attualmente presente nei download "In Coda", cliccando due volte
    sul file in questione, nel Tab Cerca, o cliccando il pulsante Scarica File si forzerà il programma
    a scaricarlo (resume). (--Tony--)
9.  Nuovo Tab "File Condivisi". Potrete così vedere i vostri file e relativi hash.
    -- possibilità di esportare la lista per uso esterno.
10. Possibilità di ridurre le dimensioni della GUI. (--Tony--)
11. Implementate le modifiche al core fatte da Jason Rohrer in MUTE-0.4.
    -- Host Catcher fixes
    -- Time fixes
    -- Inbound/Outbound Channel fixes
    -- StringBufferOutputStream fixes
    -- crytpo++ compiler related fixes
    -- ChannelReceivingThreadManager fixes
    -- ConnectionMaintainer fixes
    -- Log rollover fixes
    -- formatUtils.cpp fix (aggiunta la dimensione GB)
12. Cambiato il codice delle connessioni per i time out/rifiutato.. (--Nate--)
13. IMPORTANTI ottimizzazioni / ripulita del protocollo di routing da parte di (--NATE--)
    -- questi cambiamenti dovrebbero veramente migliorare la rete
14. Diminuita la richiesta di duplicati dei chunk dei file (--Nate--)
    -->in precedenza, i nodi venivano contattati da tali richieste, stivando la rete di pacchetti 
    non necessari.
15. Inclusa una cache nel codice di invio dei risultati delle ricerche per ridurre drasticamente l 'uso 
    della CPU. Le versioni precedenti ad ogni richiesta di ricerca ricreavano la lista dei file condivisi. 
    Questa nuova versione periodicamente effettua un aggiornamento della cache che contiene: nome del file,
    hash e dimensione. Durante l 'avvio l' applicazione aggiorna la cache alla prima richiesta di
    ricerca. Alle successive richieste il programma verifica l' ultima volta che la cache è stata
    aggiornata. Il primo intervallo è di 10 minuti, dopo il quale gli intervalli dell' aggiornamento
    aumentano di 50 secondi fino ad arrivare a 30 minuti. --Non solo viene raggiunto così una diminuzione
    dell' uso di risorse da parte del programma ma anche un miglior utilizzo della rete in quanto
    l' applicazione avrà più tempo per processare i messaggi della rete stessa.
16. Aumentato il tempo di ritardo per la creazione degli hash. -- Dopo che la cartella dei file
    condivisi è stata scandagliata alla ricerca di file senza hash, la pausa per una nuova ricerca
    viene aumentata. La CPU in questa maniera lavorerà molto meno.
17. La lista dei file in upload è ripulita alla presenza di 100 elementi. Questo per prevenire un uso 
    eccessivo della ram e per velocizzazione in quanto ogni volta che vengono inviati chunk, la lista
    viene ricreata per aggiornare gli elementi presenti.
18. La dislocazione della finestra e la dimensione viene ora ricordata al riavvio. Alla chiusura del
    programma il file "mfcWindowPlacement.ini" viene creato. Questo file è caratterizzato da 5 
    componenti. Nel seguente formato:
    campo 1: 0 == mostra la finestra nel tray system
             1 == mostra la finestra normale
    campo 2: x coordinata superiore sinistra della finestra
    campo 3: y coordinata superiore sinistra della finestra
    campo 4: x coordinata inferiore destra della finestra
    campo 5: y coordinata inferiore destra della finestra
19. Aggiunte statistiche nella finestra Upload: # Numero di Upload Visualizzati e Chunk Totali.


    
----------------------------------------------------------------------------------------------------
--- versione 0.0.2 -- 11-24-2004 release:-----------------------------------------------------------------------
--- chiarimenti su bugfix/aggiunte/modifiche
----------------------------------------------------------------------------------------------------
1.	*** Nuova funzione: DOWNLOAD IN CODA *** che cercherà nuove fonti per i download dopo un riavvio dell' applicazione.
        Questa funzione si occuperà anche del resume dei file in modo migliore che in passato. Dopo aver iniziato un 
        download, il programma ricercherà automaticamente una nuova fonte nel caso la precedente venga meno, riprendendo a 
        scaricare il file. 

	I dati relativi alla nuova funzione vengono salvati nel file Downloadqueue.ini nella cartella settings. E' un file
        di testo con questa sintassi: NOMEFILE * HASH, dove '*' è il separatore; NOMEFILE è in codice ascii, il file è
        presente nella cartella incoming, HASH è in codice ascii, e viene rilevato e salvato nel momento in cui iniziate a
        scaricare. Il file Downloadqueue.ini viene caricato all' avvio del programma, e quando questo viene terminato, il 
        file viene sovrascritto con le nuove informazioni relative alla coda dei file in download, aggiornando in questo 
        modo i dati presenti prima della chiusura del programma.

	Nel caso un utente voglia inserire dei file che sta scaricando nella coda dei download, questo è possibile anche se
        non si conosce il valore HASH. E' sufficiente aprire il file Downloadqueue.ini, quando MUTE è chiuso, con un text
        editor ed aggiungere il nome del file seguito da '*'. Come esempio se si volesse aggiungere il brano
        "supercoolmusic.mp3" alla coda, si dovrebbe inserire questa linea di testo al file Downloadqueue.ini:

        supercoolmusic.mp3 *

2.	Franc ha ridisegnato il Tab delle Preferenze avendo in questo modo un pannello laterale sinistro in cui è possibile
        scegliere tra Preferenze, Aiuto in linea collegato all' attuale pagina WIKI di MUTE ( da aggiornare),
        e Informazioni su MUTE.

3.	Revisionato da Franc il codice per evitare il riavvio del programma dopo aver cambiato lingua.

4.	Franc ha aggiunto il supporto per file esterni con estensione .lng, questi file andranno inseriti nella cartella
        settings. In questo modo si possono aggiungere lingue oltre a quelle interne (INGLESE, ITALIANO, TEDESCO e FRANCESE).
        -- descrizione nel documento mfc-mute_language_instructions.htm 

5.	Aggiunto il nuovo riquadro in Preferenze: Informazioni su Mute con "Riconoscimenti in Movimento".

6.	Sistemato un problema quando si aprono due istanze con MUTE e si utilizzano le stesse porte locali...
        ora non si ha più un crash del programma come in precedenza.

7.	Ricerca tramite "hash_" completamente funzionante.

8.	Chiusure più veloci, dovute ad una pulizia del codice.

9.  	Un ringraziamento a Franc per la traduzione francese.

10.	La finestra di chiusura permette ora di sapere se il programma è effettivamente terminato.

11. 	Bug fix riguardante l' invio dei risultati della ricerca verso altri client.

12. 	Efficienza migliorata nei download, i messaggi in arrivo vengono trattati dal giusto processo, ora i messaggi
        non vengono bypassati ad altri processi.

13.	Implementato l' uso della libreria pthreads32. Sarà impossibile non riuscire ad annullare un download ora.  

14.	Migliorata la cancellazione delle ricerche effettuate.

15. 	Sistemato un bug nel codice che causava crash del programma se all' avvio era selezionata una cartella hash
        non valida.

16. 	Aggiunta la ricerca tramite il simbolo '*'. Vale solo per le versioni MFC MUTE. Facendo una ricerca con detto 
        simbolo, si otterranno come risultati tutti i file in condivisione dai client. Va usato con prudenza, in quanto 
        può saturare la banda, ma se ti "senti fortunato" (vedi GOOGLE!), allora è da provare.

17. 	Cambiato il codice per l' invio dei risultati: non è più limitato ad una stringa di 28KB.
        Perciò, si otterranno tutti i risultati che soddisfano il criterio di ricerca.
	
18.	ICONE aggiunte nei menu contestuali.

----------------------------------------------------------------------------------------------------
--- 08-21-2004 release:-----------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
1. Realizzato un grande cambiamento al modo di operare delle ricerche. Ora quando una ricerca viene ricevuta,
   se per un file non esiste un hash associato, questo non viene generato. Un processo separato genera gli hash
   in base alla cartella condivisa dall' utente.  Il nuovo processo di generazione degli hash inizia insieme al 
   programma. Questo nuovo processo analizza tutti i file condivisi e controlla se esiste un hash per ogni file. 
   Se un hash non esiste, viene generato e il processo attende 10 secondi prima di poterne generare un altro.
   Inoltre, la funzione che genera gli hash dei file ha un limitatore interno, in modo tale che un file di grandi 
   dimensioni non possa rallentare l' applicazione rendendola inutilizzabile. Mentre avviene la generazione degli 
   hash, ogni 3 secondi, la funzione ritarda la creazione degli hash di un secondo. Questo avviene per impedire 
   che il calcolo degli hash possa ostacolare ogni altro processo attivo del programma.

2. Creato un pacchetto di installazione con Inno Setup 4.2.7 ISPP 1.2.1.295 e ISTool 4.2.7.
   -- La nuova installazione comprende il Tedesco, l' Italiano e l' Inglese.
   -- La nuova installazione genera un' icona del programma sul desktop e crea un gruppo di programma nel menu start.
   -- La nuova installazione comprende il nuovo Seed Node Updater (SNU) nella cartella settings.
   -- La nuova installazione genera due file batch sul desktop per il Seed Node Updater con i quali è possibile aggiornare
      entrambi seed node e web cache.

3. Sistemato un bug in fileShare.cpp associato ai controlli per l' 
   upload.(memory leak)

----------------------------------------------------------------------------------------------------       
--- 08-07-2004 release:-----------------------------------------------------------------------
----------------------------------------------------------------------------------------------------

1. Versione Multilingua: Italiano e Tedesco (supporto per tutte le 
   lingue che si basano sull'alfabeto latino)

----------------------------------------------------------------------------------------------------        
--- 7-29-2004 debug release: soltanto binari (sorgenti disponibili su richiesta) -------
----------------------------------------------------------------------------------------------------       
1. La release 7-28 ha  un bug per quanto riguarda il trasferimento da una directory sola lettura...
   Il programma va in crash quando tenta di spostare i file dalla cartella incoming
   verso una directory di sola lettura (cioè lettore CD) perché i file non sono stati convalidati
   nel codice di mute. Questo è stato sistemato rapidamente...
   spiacente per la nuova release...

----------------------------------------------------------------------------------------------------       
--- 7-28 debug release: soltanto binari (sorgenti disponibili su richiesta) -------
----------------------------------------------------------------------------------------------------       
1. Tab Preferenze cambiato, offre nuovi regolazioni e collegamenti a importanti siti su MUTE. 
2. ora gli utenti possono scegliere diverse cartelle per i file condivisi, in download e per gli hash  
   - questo permette che gli utenti condividano file da fonti di sola lettura come CD.. tuttavia... attualmente 
   i file in download (una volta completati) non saranno spostati nella cartella condivisa 
   (sarà sistemato in una release futura dopo che avrò ricevuto alcune idee su come ovviare a questo) 
3. nuovo pulsante in Tab Download per esplorare la cartella incoming 

----------------------------------------------------------------------------------------------------       
--- 7-24 debug release: soltanto binari (sorgenti disponibili su richiesta) ------- 
----------------------------------------------------------------------------------------------------       
1. controlli per upload nel Tab Preferenze 
2. pulsante svuota cartella hash nel Tab Cerca  
3. se un file è già presente ed attivo nel Tab Download, non verrà aggiunto lo stesso file due volte 
4. riparato il problema chiave Alt-XXX nel Tab Cerca in modo che se state cercando e cliccate 
   Alt-T ora realmente sarà uguale ad usare il pulsante Ferma, mentre prima non funzionava. 

----------------------------------------------------------------------------------------------------       
--- 7-14 debug release: soltanto binari (sorgenti disponibili su richiesta) ------- 
----------------------------------------------------------------------------------------------------       
1. Tab Preferenze, nuovo tasto per "Cancella File Annullati" -- permette di forzare il programma per cancellare i file annullati/falliti dalla cartella incoming . 
2. Tab Preferenze, non viene più visualizzato un messaggio di riavvio una volta salvate le preferenze. 
3. Tab Cerca, nuovo filtro di ricerca. Ora potete fare una ricerca più seria e accurata di prima. 
4. Tab Download, ora si possono fare i  resume dei file
5. Tab Download, tasto per eliminare i file nella cartella Incoming in modo da non dover agire manualmente. -- può persino essere aggiunto un tasto per eliminare gli hash dalla cartella Hash 

----------------------------------------------------------------------------------------------------       
--- 6-15 release: disponibile con il codice sorgente ------
----------------------------------------------------------------------------------------------------       
0. icone COLORATE (un po' più di vita!) 
1. programma si apre nel Tab Connessioni 
2. Bug ip invalido (quando l' indirizzo IP digitato è sbagliato) Sistemato 
3. Tab Connessioni si può spostare il separatore delle colonne 
4. Tab Cerca, colori diversi per i file (è basato approssimativamente sul tipo. colore rosso per audio, verde per video, blu per i documenti,nero per sconosciuto 
5. Tab Cerca, aggiunta l'icona di sistema ed il tipo di file alla lista di ricerca (quando disponibile)  
6. Tab Cerca, tutte le colonne sono regolabili 
7. Tab Cerca, right-click menu download
8. Tab Cerca, i file aggiunti ai download sono evidenziati in giallo e segnalati in download
9. Tab Cerca, possibilità di salvare le ricerche in file di testo
10. Tab Cerca, tasto per cancellare i risultati della ricerca 
11. Tab Cerca, si può manovrare attraverso la lista di ricerca con i tasti cursore e premere Invio per scaricare il file selezionato. 
12. Tab Download, right-click menu: elimina completi, annulla, elimina fermi ecc. 
13. Tab Download, tutte le colonne sono regolabili
14. Tab Download, tasto esplora file condivisi
15. Tab Download, pulsante per eliminare file: completi, fermi e per annullare...
16. Tab Download, significato colori... blu == completo, == rosso fermo/fallito/annullamento in corso.../annullato, verde == download attivo
17. Tab Upload, right-click menu: elimina
18. Tab Upload, tutte le colonne sono regolabili
19. Tab Upload, tasto rimuovi ed elimina ( elimina solo completati/fermi) upload correnti riappariranno alla successiva richiesta del chunk
20. Tab Upload,significato colori... blu == completo, rosso == fermo/fallito, verde == attivo 
21  TOOLTip con Informazioni aggiuntive