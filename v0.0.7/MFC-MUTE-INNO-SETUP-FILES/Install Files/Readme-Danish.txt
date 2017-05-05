MFC MUTE VERSIONER....

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
--- forklaring af fixes/tilføjelser/ændringer
----------------------------------------------------------------------------------------------------
1. Jason Rohrer's DROP TREE sikkerhedspatch er blevet integreret.
2. Om Mute side (under indstillinger) er blevet opdateret med 
http://www.sourceforge.net/projects/mfc-mute-net adressen.
3. Der er blevet lagt begrænsning af antallet af resultater som 
kan modtages på baggrund af wildcard søgninger '*'. Antallet af 
resultater som kan modtages er bestemt af en tilfældighed funktion.
Dette er gjordt for at øge sikkerheden og  reducere antallet 
af 'Network search attacks'.
4. Forbedret søgefunktion. Desktop.ini og Thumbs.db filer vil ikke 
længere blive vist som søge resultater, da der ikke er nogen fornuftig 
grund til at downloade dem.
5. Dynamiske faneblads ikoner. Nu er det mulig tilføje egne faneblads-
ikoner. Kopier dine ikoner ned i 'icons'-mappen, filnavnene skal matche
understående:
    icons\searchTab.ico
    icons\downloadsTab.ico
    icons\uploadsTab.ico
    icons\connectionsTab.ico
    icons\sharedfilesTab.ico
    icons\settingsTab.ico

6. Netværks traffik statistik på relevate faneblade (Download, upload, 
forbindelser).


----------------------------------------------------------------------------------------------------
--- version 0.0.5 -- 03-04-2005 release:-----------------------------------------------------------------------
--- forklaring af fixes/tilføjelser/ændringer
----------------------------------------------------------------------------------------------------
1. Tilføjet en farvet status-bar i bunden af GUI'en over forbindelse 
status.
2. Sat timer (over forløben tid) i caption-baren med formatet 
DAGE:TIMER:MINUTER:SEKUNDER.
3. Tilføjet noget kode som holder styr på download rækkefølgen af
hentede blokke. Dette øger download hastigheden.

----------------------------------------------------------------------------------------------------
--- version 0.0.4 -- 02-24-2005 release:-----------------------------------------------------------------------
--- forklaring af fixes/tilføjelser/ændringer
----------------------------------------------------------------------------------------------------
1.  Ændret den gule tips tekst, som høre til MUTE MFC, i proceslinien fra 
    "MFC_MUTE_0.3" til "MFC MUTE".
2.  Har ordnet 2 fejl i "Download kø"-koden.
    -1-->   Har ordet en fejl som opstår når en download fra Downloadqueue.ini 
    genoptages. I ver. 0.0.3 blev ikke alle af downloads'ene genoptaget korrekt
    efter genstart, dette er nu rettet.
     -2-->  Fik rettet en fejl hvor en download i køen blev genoptaget. Processen
     søgte efter og fandt emnet, og forsøgte på at begynde download
     også selvom der allerede var 4 aktive downloads i gang fra denne VIP. 
     Da 4 er det max. antal downloads som må være aktive fra en VIP, skal download's
     forsøget lægges tilbage i kø'en. Fejlen var at selvom downloadet blive lagt
     tilbage i køen, så blev status teksten ved med at være den samme (forkerte).
3.  Forøget semaphore timeout værdier i "Download-Kø"-koden.
4.  Rettet den program-kode som tilføjer downloads fra søge-vinduet. Fejlen var
    at program-koden tilføjede downloads som allerede var valgt og at der derfor kom dubletter.
5.  I Download-vinduet kan valgte filer nu afbrydes/fjernes med DELETE tasten.
6.  Forøget semaphore timeout værdier in "Download Dialog"-koden.
7.  Har tilføjet 1 millisekunds hvile for at tage alt CPU kraften fra andre tråde.
8.  I upload-vinduet kan valgte filer nu afbrydes/fjernes med DELETE tasten.
9.  Har rettet fejl I den program-kode som styre filblok-forspørgselernes timeout/retries. 
    Denne rettelse skulle forbedre netværksydelsen ved at begrænse antallet af filblok-forspørgseler
    som genudsendes fordi der ikke er nok tid til at klienten kan nå at få et svar.
10. Der er blevet rettet nogle fejl i den danske sprog pakke. (TOBIAS)


----------------------------------------------------------------------------------------------------
--- version 0.0.3 -- 01-09-2005 release:-----------------------------------------------------------------------
--- forklaring af fixes/tilføjelser/ændringer
----------------------------------------------------------------------------------------------------
1.  Har tilføjet nye Ikoner og ændret nogen af de gamle (tak til --NGLWARCRY--).
2.  Har opgraderet projekt filer, så det bliver nemmere at kompile under Visual Studio .NET.
3.  Tilføjet noget kode som saver host IP adresser i seedhost.ini filen når Mute lukkes 
    ned (Tak til --Nate--/TSAFA).
4.  Fixer Status sorteringen in download dialog skærmen.
5.  Har tilføjen en funktion, så man ,ved at højre klikke på et i downloads skærmen, kan bestemme 
    hvilket emne i download-køen man vil downloade næste gang (tak til --Tony--).
6.  Har øget tiden mellem søgningerne for automatiske download for at ungå overbelastning af netværket
7.  Tilrettet download-kø-koden så hver gang en fil blive afbrudt eller færdig downloadet, så opdateres
    indholdet af Downloadqueue.ini filen. Dette forbedre download-kø-funktionen i tilfælde af et crash
    eller en hurtig afslutning af programmet.
8.  En bruger kan nu dobbelt-klikke (eller trykke på download-knappen) på en fil i søge-vinduet for at
    starte en download. Hvis downloaden går i stå eller programet lukkes kan man søge efter filen og
    klikke på den igen for at genstarte download processen (tak til --Tony--).
9.  Der er blevet tilføjet et Delte Filer fag. Her kan man se filen samt HASH-data for filen. Her ud 
    over kan du eksporter listen til en koma delt fil til ekstren brug. 
10. Brugerne kan nu formindste GUI til en mindre størrelse med resizer-gripbar. (tak til --Tony--)
11. Der er blevet lavet nogle ændringer af kernen samt blevet rettet nogle bugs i kernen. Disse kommer
    fra Jason Rohrer's MUTE-0.4 core code.
    -- Host Catcher fixes
    -- Time fixes
    -- Inbound/Outbound Channel fixes
    -- StringBufferOutputStream fixes
    -- crytpo++ compiler related fixes
    -- ChannelReceivingThreadManager fixes
    -- ConnectionMaintainer fixes
    -- Log rollover fixes
    -- formatUtils.cpp fix (added -GiB code)
12. Koden som tager sig af netforbindelsen time out/dropping er blevet ændret (tak til --Nate--)
13. Så er der blevet lavet betydelige forbedringer af routing koden. (Mange tak til --Nate--)
14. Der er blevet tilføjet kode som forhindre duplet blokke i at oversømme netværket. Dette har
    tidligere været et problem.
15. Har tilføjet en cache mekanisme til Search Result sending kode som dramatisk redukser 
    programmets CPU belastning. Tidligere versioner gensøgte mappen Delte Filer hver gang der kom
    og en forespørgsel og lavede hver gang en ny list af mappen. Den nye version laver periodiske
    søgninger og gemmer resultatet i en cache som indeholder filnavn, filhash, and filestørrelse.
    Ved program opstart opdateres cachen ved første søge forespørgsel. Herefter opdateres cachen
    hver 10 minut, der ligges dog 50 sekunder til denne periode for hvert gang en søgeforespørgsel
    lykkes, Dog max. 30 minuter. Dette giver ikke bare en forbedret CPU udnyttelse, men også en
    bedre udnyttelse af netværket fordi der så er mere CPU kraft til at tage sig af netværks fore-
    spørgsler.
16. Forøget hash genereringstid  -- Dette betyder programmet giver mere tid til at undersøge 
    Delte Filer mappen for filer som ikke har fået lavet hash-data, lave disse hash-data. Dette for
    programmet til at bruge mindre CPU kraft.   
17. Upload skærmen vil automatisk blive tømt efter 100 uploads. Dette søge for at der ikke bliver 
    brugt så meget ram og at programmet vil blive afviklet hurtigere fordi der ikke skal bruges 
    resurser på opdatering af Uploadlisten.
18. Program vinduets position vil nu være den samme ved opstart som den var da det blev lukket 
    sidst. Når programmet lukkes gemmes positions data i filen "mfcWindowPlacement.ini". Filen er
    koma delt og indeholder 5 integer værdier. Formatet er:
    
    field 1:    0 == show window in tray
                1 == show window normal
    field 2: x coordinate top left of window
    field 3: y coordinate top left of window
    field 4: x coordinate bottom right of window
    field 5: y coordinate bottom right of window
19. Tilføjet fuktioner : Visning af antal uploads og statistik over mængden af blokke hver enkelt
    upload har sendt.