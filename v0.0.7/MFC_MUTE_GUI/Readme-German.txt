MFC MUTE-Versionen   -- translated by Markus

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
--- version 0.0.6 -- 08-13-2005 release:------------------------------------------------------------
--- Beschreibung der Bugfixes/Ergaenzungen/Veraenderungen
----------------------------------------------------------------------------------------------------

1. Jason Rohrer´s DROP TREE Sicherheitspatch-Code eingebaut

2. Die MUTE MFC Website wurde im About-Dialog angepasst: http://www.sourceforge.net/projects/mfc-mute-net

3. Die Suche nach Wildcards '*' liefert jetzt nur noch eine limitierte Anzahl an Ergebnissen zurueck. Die
   Resultate werden nach dem Zufallsprinzip zurueckgeliefert, um die Sicherheit zu erhoehen und um Wildcard-
   Suchattacken im Netzwerk zu vermeiden

4. Dateien wie desktop.ini und thumbs.db werden nicht mehr in Suchergebnisse aufgenommen. Es gibt nicht wirklich
   einen Grund, weshalb jemand diese Dateien downloaden sollte, deshalb wurden sie von den Suchresultaten
   ausgenommen

5. Dynamische Icons fuer die Tab-Reiter, die auf den Dateien basieren, die sich im Icons-Verzeichnis im MUTE MFC
   Programmverzeichnis befinden. Nachstehend gibt es eine Liste der Dateien; wenn ein Benutzer die Icons aendern
   moechte, die beim Programmstart angezeigt werden, muessen lediglich die vorhandenen Icon-Dateien durch andere
   gleichnamige ersetzt werden:

          icons\searchTab.ico
          icons\downloadsTab.ico
          icons\uploadsTab.ico
          icons\connectionsTab.ico
          icons\sharedfilesTab.ico
          icons\settingsTab.ico

6. Netzwerkverkehr-Statistiken zu verschiedenen Dialogen hinzugefuegt (derzeit nur in englisch verfuegbar)


----------------------------------------------------------------------------------------------------
--- version 0.0.5 -- 03-04-2005 release:-----------------------------------------------------------------------
--- Beschreibung der Bugfixes/Ergaenzungen/Veraenderungen
----------------------------------------------------------------------------------------------------
1.  Added colored bar across bottom of GUI that shows connection status.
2.  Put the elapsed time in the caption bar format DAYS:HOURS:MINUTES:SECONDS
3.  Added some code from Nate that saves out of order downloaded chunks to help speed up D/Ls.

----------------------------------------------------------------------------------------------------
--- version 0.0.4 -- 02-24-2005 release:-----------------------------------------------------------------------
--- Erklaerung der Bug fixes/Erweiterungen/Aenderungen
----------------------------------------------------------------------------------------------------

1. Tray icon tooltip von "MFC_MUTE_0.3" zu "MFC MUTE" geaendert
2. Zwei Fehler im Download Queue-Code beseitigt:
   -1-->  Fehler beim Laden der queued downloads aus der Datei Downloadqueue.ini beseitigt.
          In Version 0.0.3 wurden nicht alle der queued downloads in der Download.ini
          korrekt beim Restart hinzugefuegt
          
   -2-->  Fixed a bug where a queued download was automatically searched for and found, but
          in the process of attempting to resume the download, there were already 4 downloads
          active from the remote source.  Since only 4 active downloads are allowed from 1 VIP
          the queued item must be placed back into the queue.  The BUG was that the item was
          placed back into the queue, but the status text of the item was not properly updated.
          This gave the (invalid) appearance of many queued items searching at once.     
   
3. Timeout-Werte fuer Semaphoren im Download Queue Code erhoeht 
4. Code gefixed, der Downloads aus dem Suchen-Fenster hinzufuegt, die bereits in der Warteschlange
   vorhanden waren. Nach weiteren Tests mit Version 0.0.3 habe ich herausgefunden, dass der Code
   fehlerhaft war und doppelte Einträge in der Download-Liste aufgetaucht waeren
5. Im Downloads-Fenster uebernimmt die ENTF-Taste die abbrechen/loeschen-Funktion
6. Timeout-Werte fuer Semaphoren im Downloads Dialog Code erhoeht   
7. Im Share Files refresh code wurde eine Pause mit einer Millisekunde Laenge eingebaut, um zu 
   verhindern, dass dieser Thread anderen Threads keine CPU-Zyklen mehr uebrig laesst
8. Im Uploads-Fenster uebernimmt die ENTF-Taste die abbrechen/loeschen-Funktion
9. Fehler im darunterliegenden (Original MUTE-Code) bezueglich der internen file chunk request timeouts
   und Wiederholungen beseitigt. Dieser Aenderung sollte sich auch positiv auf das gesamte MUTE-Netz
   auswirken; mit der Aenderung wird naemlich verhindert, dass zu viele file chunk requests rausgehen, 
   bevor der Client die eingehenden Antworten verarbeiten kann
10. Daenische Uebersetzung von Tobias ueberarbeitet
11. Tuerkische Uebersetzung von defnax hinzugefuegt


----------------------------------------------------------------------------------------------------
--- version 0.0.3 -- 02-09-2005 release:-----------------------------------------------------------------------
--- Erklaerung der Bug fixes/Erweiterungen/Aenderungen
----------------------------------------------------------------------------------------------------

1. Neue Icon-Resourcen hinzugefuegt und einige Aenderungen (--NGLWARCRY--) an den Icons vorgenommen
2. Projektdateien wurden aktualisiert und koennen nun auch unter Visual Studio .NET compiliert werden
3. Code hinzugefuegt, um IP-Adressen von Hosts in der seedHosts.ini bei Programmende zu speichern (--Nate--/TSAFA)
4. Sortieren nach "Status" im Downloads-Dialog gefixed
5. Menueeintrag hinzugefuegt (erreichbar mit rechter Maustaste im Downloads-Bildschirm), mit dem ein Eintrag in der Warteschlange
   zum naechsten Eintrag fuer die automatische Suche gemacht werden kann (--Tony--)   
6. Zeitintervall zwischen automatischen Suchabfragen fuer die Download-Queue erhoeht, um das "Ueberfluten" des Netzes mit
   Suchanfragen zu verhindern   
7. Der download queue code wurde dahingehend angepasst, dass jedesmal, wenn eine Datei entweder fertig heruntergeladen oder
   gecancelt wurde, diese Aenderungen sofort in der Datei Downloadqueue.ini gespeichert werden. Dies verbessert die Bestaendigkeit
   und Integritaet der Download-Queue auch in Faellen von potentiellen Abstuerzen oder schnellen Programmbeendigungen   
8. Wenn man nach einer Datei sucht, die sich bereits in der Warteschlange (Queue) befindet und diese im Suchen-Fenster
   entdeckt, dann kann man diese Datei doppelklicken bzw. den Download anklicken, um das Resuming fuer diesen Eintrag in 
   der Warteschlange zu forcieren (--Tony--)   
9. "Shared Files"-Reiter zum GUI hinzugefuegt. Dieser zeigt die Datei sowie den Hash-Wert der Datei (Du kannst die Liste zur
   weiteren externen Verwendung im CSV-Format speichern)   
10. Die Oberflaeche kann mit Hilfe eines Kontrollelements noch weiter verkleinert werden (--Tony--)
11. Einige der Aenderungen und Fixes von Jason Rohrer´s MUTE 0.4 core code uebernommen:
    -- Host Catcher fixes
    -- Time fixes
    -- Inbound/Outbound Channel fixes
    -- StringBufferOutputStream fixes
    -- crytpo++ compiler related fixes
    -- ChannelReceivingThreadManager fixes
    -- ConnectionMaintainer fixes
    -- Log rollover fixes
    -- formatUtils.cpp fix (added -GiB code)    
12. Aenderungen am connections code fuer das Handling von Timeouts vorgenommen (--Nate--)
13. Sehr wichtige Aenderungen am Routing bzw. am Routing Code vorgenommen, die von Nate entdeckt und erforscht wurden (--NATE--)
    (Diese Aenderungen sollten das Netzwerk wirklich enorm verbessern!)
14. Code von Nate hinzugefuegt (duplicate file chunk request mitigation code), um das Netzwerk bei doppelten file chunk requests
    nicht durcheinander zu bringen und es mit unnoetigen Paketen zu belasten    
15. Einen Cache-Mechanismus zum Search Result sending code hinzugefuegt, der die CPU-Auslastung durch MUTE MFC drastisch 
    reduziert (Anmerkung: Aus Zeitgruenden verzichte ich auf die komplette Uebersetzung des Algorithmus)    
16. Wenn das "Shared Files"-Verzeichnis auf neue Dateien ueberprueft wird, die noch nicht gehashed wurden, dann wird der
    Polling-Intervall erhoeht. Dadurch werden weniger CPU-Resourcen benoetigt.    
17. Der Uploads-Dialog loescht die Liste automatisch, nachdem 100 Eintraege angezeigt wurden. Diese Massnahme verhindert,
    dass MUTE MFC grosse Mengen an Hauptspeicher verbraucht und hilft ausserdem, den ganzen Vorgang zu beschleunigen     
18. Die Fensterposition sowie die Fenstergroesse bleiben auch nach einem Restart erhalten. Wenn MUTE MFC
    beendet wird, wird eine Datei namens "mfcWindowPlacement.ini" erzeugt und die Einstellungen werden
    dort gespeichert. Diese .ini-Datei liegt im CSV-Format und ist wie folgt aufgebaut:
    Feld 1: 0 == Fenster im Tray anzeigen
            1 == Fenster normal anzeigen
    Feld 2: x-Koordinate linke obere Ecke des Fensters
    Feld 3: y-Koordinate linke obere Ecke des Fensters
    Feld 4: x-Koordinate rechte untere Ecke des Fensters
    Feld 5: y-Koordinate rechte untere Ecke des Fensters
19. Statistik im Uploads-Dialog hinzugefuegt, in der die Anzahl der Uploads sowie die Gesamtanzahl der
    hochgeladenen Dateifragmente angezeigt werden.

----------------------------------------------------------------------------------------------------
--- version 0.0.2 -- 11-24-2004 release:-----------------------------------------------------------------------
--- Erklärung zu den Fixes/Erweiterungen/Veränderungen
----------------------------------------------------------------------------------------------------

1. *** Neue Download-Queue ***, die aktiv/selbständig nach neuen Quellen fuer Downloads
   nach dem Neustart von MUTE sucht.

   Die Queue handhabt die Wiederaufnahme von Downloads (resuming) wesentlich besser als
   in der Vergangenheit. Nun kannst du Downloads starten und das Programm eine Weile 
   laufen lassen; die Downloads werden automatisch fortgesetzt, sobald Quellen verfuegbar
   sind.
   
   Die Daten der Download-Queue werden im Verzeichnis MUTE\settings in der Datei Downloadqueue.ini
   gespeichert. Diese Datei ist eine Textdatei mit folgender Struktur:
   
   DATEINAME * HASH, wobei das '*' ein Trennzeichen und DATEINAME die ASCII-Version des
   lokalen Dateinamens im MUTE\incoming-Verzeichnis ist. HASH entspricht dem ASCII-HASH
   der Remote-Datei, die für den initialen Download verwendet wurde. Die Datei Downloadqueue.ini
   wird beim Programmstart geladen und beim Beenden von MUTE mitsamt den neuen Queue-Informationen
   ueberschrieben.
   
   Wenn ein User Dateien in seinem MUTE\incoming-Verzeichnis hat, die er zur Downloadqueue.ini
   hinzufuegen moechte, dann ist das sogar ohne den HASH-Wert moeglich. Um dies zu bewerkstelligen,
   muß MUTE beendet sein; dann kann man mit einem Texteditor den kompletten Dateinamen der Datei(en) 
   - gefolgt von einem '*' - einfuegen.
   
   Beispiel: Wenn ich die Datei "supercoolmusic.mp3" zu meiner Queue hinzufuegen moechte, dann
   kann ich die Datei Downloadqueue.ini oeffnen und diese Zeile der Datei hinzufuegen:
   
   supercoolmusic.mp3 *
   
2. Francis hat den Einstellungen-Dialog renoviert. Links kann nun zwischen den Einstellungen fuer 
   MUTE, einer Hilfe im HTML-Format (die auf das Wiki verweist und aktualisiert werden muß)
   sowie einem neuen About-Button waehlen.
   
3. Francis hat den Code ueberarbeitet, der fuer das dynamische Umschalten zwischen den einzelnen
   Sprachen *ohne* Neustart von MUTE MFC zustaendig ist.
   
4. Francis hat Unterstuetzung fuer externe Sprachdateien hinzugefuegt, die erzeugt und im
   MUTE\settings-Verzeichnis abgelegt werden koennen, um weitere Sprachen fuer MUTE MFC zu
   unterstuetzen (außer den internen Sprachen wie englisch, italienisch, deutsch und franzoesisch).
   
5. Einen neuen und coolen About Screen mit scrollenden Credits hinzugefuegt   

6. Wenn zwei Instanzen von MUTE MFC geoeffnet wurden, die denselben lokalen Port benutzten,
   dann konnte es zu einem Crash kommen. Dieses Problem wurde gefixed.
   
7. "hash_"-Suchen funktionieren nun richtig bei dieser Version

8. Schnellere Shutdowns aufgrund von ueberarbeitetem Code

9. Danke an Francis fuer die franzoesische Uebersetzung

10. Neues Shutdown-Fenster, das dir mitteilt, wenn das Programm *wirklich* geschlossen wurde

11. Interne Bugfixes bezueglich der Bereitstellung von Suchergebnisse fuer andere

12. Effizienz bei der Verarbeitung von Downloads wurde verbessert. Nachdem inbound messages 
    einmal von den entsprechenden message handlern gehandled wurden, werden die messages
    nicht mehr blindlings an andere message handler weitergereicht.
    
13. Verwendung der pthreads32 library, um "eingefrorene" Downloads zu verhindern bzw. zu fixen.
    Mit dieser Version kannst einen Download *immer* abbrechen.
    
14. Schnellerer Abbruch der Suche

15. Einen Fehler im Hashing-Code beseitigt, der einen Crash verursachen konnte, wenn vom User
    ein ungueltiges Hash-Verzeichnis spezifiziert wurde
    
16. Nur fuer MUTE MFC: Code hinzugefuegt, der auf ein '*' bei der Suchabfrage reagiert. Diese
    Suche sollte *alle* Dateien im MUTE\shared-Verzeichnis zurueckliefern. Benutze diese Suche
    mit Bedacht, denn sie wird die Bandbreite stark auslasten...
    
17. Der Code fuer search results sending wurde modifiziert, so daß er nicht mehr auf eine
    Zeichenkette mit einer Groesse von 28 KBytes limitiert ist. Aus diesem Grund werden die 
    User Resultate fuer alle Dateien erhalten, die ihrem Suchkriterium entsprechen.
   
18. (GOOGLE translation: Zusätzliche IKONEN zu den rechten Klickenmenüs auf allen Schirmen)

----------------------------------------------------------------------------------------------------
--- 08-21-2004 release:-----------------------------------------------------------------------
----------------------------------------------------------------------------------------------------

1. Eine große Änderung an der Art und Weise der eingehenden Suchen vorgenommen. Nun wird für eine
   eingehende Suche kein Hash mehr erzeugt, wenn er für eine Datei nicht existiert. Ein separater
   Thread ist nun für die Erzeugung von Hash-Dateien für das benutzerdefinierte Shared-Verzeichnis
   zuständig.
   
   Der neue Hashing-Thread startet zusammen mit MUTE. Dieser Thread läuft iterativ über alle Dateien
   im Shared-Verzeichnis und überprüft, ob für jede Datei in diesem Verzeichnis eine Hash-Datei existiert.
   
   Wenn der Hash noch nicht existiert, wird er vom Thread erstellt; anschließend wartet der Thread
   10 Sekunden, bis er den nächsten Hash erstellen kann. Zusätzlich enthält die für die Erstellung der
   Hashes zuständige Funktion einen Begrenzer, so daß große Dateien MUTE nicht mehr "herunterziehen"
   und ein weiteres arbeiten damit unmöglich machen (Hänger, Abstürze usw.)
   
   Innerhalb der Funktion (während der Hashing-Code durchläuft) wird alle drei Sekunden das Generieren
   des Hashes für eine Sekunde verzögert. Dies stellt sicher, daß das Hashing keine anderen Threads 
   "verhungern" läßt.
   
2. Komplette Setuproutine mit Inno Setup 4.27, ISPP 1.2.1.295 und ISTool 4.2.7 erstellt:
   -- Die neue Installation bietet Unterstützung für deutsch, italienisch und englisch
   -- plaziert ein Icon auf dem Desktop und legt eine Programmgruppe an
   -- enthält SNU (SeedNode Updater) 0.5 Beta im settings-Verzeichnis
   -- legt Batchdateien auf dem Desktop an, mit denen jederzeit mittels SNU die Seed 
      Nodes und die Web Caches auf einen aktuellen Stand gebracht werden können
      
3. Speicherleck in fileShare.cpp beseitigt, das mit den Upload-Kontrollen verbunden war

----------------------------------------------------------------------------------------------------       
--- 08-07-2004 release:-----------------------------------------------------------------------
----------------------------------------------------------------------------------------------------

1. Unterstützung von Mehrsprachigkeit mit dem Resource Editor von MS Visual C++ hinzugefügt. Nun 
   unterstützt MUTE MFC englisch, deutsch und italienisch.
   
----------------------------------------------------------------------------------------------------       
--- 7-29-2004 debug release: binary only currently (source available upon request) -------
----------------------------------------------------------------------------------------------------       

1. Das Release 07-28 hatte einen Fehler bezüglich downloaden von einem freigegebenen Read Only-
   Verzeichnis. Das Programm stürzt bei dem Versuch ab, Dateien aus dem Incoming-Verzeichnis in
   ein Read Only-Verzeichnis (z.B. CD-Laufwerk) zu verschieben, da Dateipointer im darunterliegenden
   MUTE Kerncode nicht überprüft wurden. Dies wurde schnell gefixed...sorry für die rasch aufeinander-
   folgenden Releases...
   
   -- keine neuen Features in diesem Release
   
----------------------------------------------------------------------------------------------------
--- 7-28 debug release: binary only currently (source available upon request) -------
----------------------------------------------------------------------------------------------------

1. Der Settings-Dialog hat sich geändert und bietet neue Settings und Weblinks zu wichtigen MUTE
   Sites an
   
2. Nun können User verschiedene Verzeichnisse für Share\Incoming\Hash benutzen:
   - Das erlaubt Usern Dateien von Read Only-Quellen wie CDs zu sharen, jedoch werden eingehende
     Dateien (nachdem sie fertig sind) nicht in das Shared-Verzeichnis verschoben (das wird in
     einem zukünftigen Release gefixed werden, sobald User Ideen haben, wie ich das erfolgreich
     in den Griff bekomme)
     
3. Neuer Button auf der Downloads-Seite, um das Incoming-Verzeichnis zu erforschen

----------------------------------------------------------------------------------------------------
--- 7-24 debug release: binary only currently (source available upon request) -------
----------------------------------------------------------------------------------------------------

1. Upload-Kontrollen einstellbar im Settings-Bildschirm

2. "Hashes löschen"-Button im Suchen-Dialog

3. Eine Datei wird nicht zweimal zu den Downloads hinzugefügt, wenn sie bereits vorhanden und bei den
   Downloads aktiv ist
   
4. Das ALT-XXX Problem im Suchen-Dialog wurde beseitigt. Wenn du nun suchst und ALT-T anklickst, dann
   entspricht das tatsächlich dem Klick auf den Stop-Button, was vorher nicht funktioniert hat.
   
  
