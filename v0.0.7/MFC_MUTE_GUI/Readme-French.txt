VERSIONS DE MFC MUTE....

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
--- version 0.0.6 -- 08-13-2005 mise à jour---------------------------------------------------------
--- explication des corrections/ajouts/modifications
----------------------------------------------------------------------------------------------------
1.  Incorporation du code du patch DROP TREE de Jason Rohrer.
2.  Mise à jour de l'écran "A propos" intégrant l'adresse de sourceforge
     http://www.sourceforge.net/projects/mfc-mute-net
3.  L'utilisation du joker (wildcard) '*' permet maintenant de limiter les résultats. Ceux-ci sont
     randomizés afin d'augmenter la sécurité et de réduire les attaques du réseau par des recherches
     effectuées à partir de ce joker.
4.  Les fichiers tels que desktop.ini et Thumbs.db ne seront jamais inclus dans les résultats de
     recherches. Il n'y a aucune raison de pouvoir télécharger de tels fichiers, aussi ils sont retirés
    des résultats des reherches.
5.  Les icones dynamiques sont chargées au démarage à partir des fichiers d'icones contenues dans
    le repertoire icones situé sous le repertoire du programme principal. La liste des fichiers d'icones
    est donnée ci-dessous. Pour changer une icone il suffit de remplacer le ou les fichiers ci-dessous :
          icons\searchTab.ico
          icons\downloadsTab.ico
          icons\uploadsTab.ico
          icons\connectionsTab.ico
          icons\sharedfilesTab.ico
          icons\settingsTab.ico
6.  Les statistiques de traffic réseau figurent désormais sur de nombreux écrans  (en anglais seulement)


----------------------------------------------------------------------------------------------------
--- version 0.0.5 -- 03-04-2005 release:-----------------------------------------------------------------------
--- explanation of fixes/additions/alterations
----------------------------------------------------------------------------------------------------
1.  Added colored bar across bottom of GUI that shows connection status.
2.  Put the elapsed time in the caption bar format DAYS:HOURS:MINUTES:SECONDS
3.  Added some code from Nate that saves out of order downloaded chunks to help speed up D/Ls.


----------------------------------------------------------------------------------------------------
--- version 0.0.3 -- 02-09-2005-----------------------------------------------------------------------
--- Explication des corrections/ajouts/modifications
----------------------------------------------------------------------------------------------------

1.  Ajout de nouvelles icones et modifications des icones existantes(--NGLWARCRY--)
2.  Les fichiers sources du projet ont été modifiés pour être facilement compilés avec Visual Studio .NET.
3.  Nouveau code ajouté pour sauvergarder les IP hotes dans le fichier seedHosts.ini lors
    de la fermeture du programme.(--Nate--/TSAFA)
4.  Correction du statut du tri dans la fenêtre des téléchargements.
5.  Ajout d'un clic droit dans la fenêtre des téléchargements ce qui permet à l'utilisateur de
    de faire de l'item selectionné le prochain à être téléchargé.(--Tony--)
6.  Augmentation du délai entre chaque requête de recherche afin de réduire l'engorgement
    du réseau par trop de requêtes de recherche.
7.  Modification du code relatif au téléchargement en sorte que chaque fois qu'un fichier est
    annulé ou terminé le fichier Downloadqueue.ini file soit mis à jour pour refleter l'état de
    la file d'attente. Ceci améliore la persistance et l'intégrité de cette file en cas de
    de crash éventuel ou de fermeture rapide du programme.
8.  Si un utilisateur cherche un fichier qui est dans la file d'attente et le trouve dans
    la fenêtre de recherche il peut effectuer un double clic sur ce fichier ou cliquer sur
    téléchargement et forcer ainsi le telechargement à redemarrer.(--Tony--)
9.  Ajout d'un onglet "fichiers partagés" dans le GUI. Celui-ci montre les fichiers en partage
    et leurs HASH (hachage).
    -- on peut exporter pour un usage externe cette liste vers un fichier utilisant la virgule
    comme délimiteur.
10. L'utilisateur peut désormais reduire la fenêtre principale à l'aide d'une poignée barre (--Tony--)
11. Ajout de modifications dans le code source du noyau pour refleter les modifications et améliorations
    effectuées par Jason Rohrer's  dans le noyau de MUTE-0.4.
    -- Recherche d'hôtes améliorée
    -- Gestion du temps corrigée
    -- Correction des canaux d'entrée et de sortie
    -- Correction du StringBufferOutputStream (flux de sortie des chaines)
    -- corrections apportées au compilateur de chiffrement crytpo++
    -- corrections apportées a ChannelReceivingThreadManager (manager des canaux de reception)
    -- corrections apportées au ConnectionMaintainer (maintien des connexions)
    -- corrections du déroulant de Log
    -- corrections apportées à formatUtils.cpp (ajout de code -GiB)
12. Modifications du code gérant le décompte temporel et l'abandon (--Nate--)
13. Améliorations SIGNIFICATIVES du code de routage. Le code recherche et nettoie les modifications au fur et
    à mesure qu'elles sont découvertes (--NATE--).
    -- Ces changement devraient vraiment améliorer le réseau.
14. Ajout de code minimisant les requêtes de duplication de fichiers partiels fourni par (--Nate--)
    -->Auparavant les noeuds recevaient des requêtes de duplication de fichiers encombrant le réseau de
    paquets inutiles.
15. Ajout d'un mécanisme de cache associé au résultat de recherche reduisant TRES SIGNIFICATIVEMENT
    l'utilisation du processeur.La version précédente parcourait et  recréait la liste des fichiers
    partagés lors de chaque requête de recherche. La nouvelle version met à jour périodiquement un
    cache qui contient le nom de fichier, le hachage associé et la taille du fichier. Au
    démarrage l'application met à jour ce cache à l'occasion de la première requête de recherche. Lors
    des requêtes suivantes l'application recherche quand le cache a été mis à jour pour la dernière fois.
    Le premier délai entre deux mises à jour du cache est de 10 minutes,à chaque mise à jour successive
    l'intervalle augmente de 50 secondes jusqu'à atteindre un maximum de 30 minutes entre chaque mise
    à jour - Ceci non seulement reduit l'usage du processeur mais améliore également la performance
    générale du réseau car l'application dispose désormais de beaucoup plus de temps pour traiter les
    message du réseau plutôt que de consacrer son temps à rechercher dans les répertoires et les noms
    de fichiers.
16. Augmentation du temps de saturation du système de hachage. Cela signifie le temps alloué pour
    rechercher dans le répertoire des fichiers partagés de nouveau fichiers qui n'ont pas encore été
    hachés a été augmenté. Ceci permet au système de hachage de consommer moins de ressources processeur.
17. La fenêtre de fichiers en cours de partage se nettoie automatiquement après avoir montré 100 fichiers
    afin de limiter la consommation de mémoire ram et également d'accélérer le fonctionnement dans la mesure
    ou chaque fois que des tronçons de fichiers sont émis en sortie cette liste doit être parcourue afin
    d'être mise à jour.
18. Les dimensions et l'emplacement de la fenêtre sont désormais persistants et mémorisés dans un fichier
   "mfcWindowPlacement.ini". Ce fichier utilise 5 entiers délimités par une virgule. Le format est :
   champs 1 : 0== montrer la fenêtre dans la barre des taches
              1== affichage normal de la fenêtre
   champs 2 : x coordonnée du coin haut gauche de la fenêtre
   champs 3 : y coordonnée du coin haut droite de la fenêtre
   champs 4 : x coordonnée du coin bas gauche de la fenêtre
   champs 5 : y coordonnée du coin bas droite de la fenêtre
19. Ajout de statistiques dans la fenêtre de partage sur les partages effectués et les tronçons mis à jours.
    
----------------------------------------------------------------------------------------------------
--- version 0.0.2 -- 11-24-2004-----------------------------------------------------------------------
--- Description des corrections/ajouts/modifications
----------------------------------------------------------------------------------------------------

1.  *** Nouvelle file d'attente des téléchargements *** après un redémarrage 
    du programme de nouvelles sources sont désormais recherchées en vue de continuer le téléchargement.
    
    La reprise des téléchargements a été considérablement améliorée. Maintenant le programme
    est capable de redémarrer des téléchargements quand de nouvelles sources deviennent disponibles.
    
    Les données de la file d'attente sont stockées dans le fichier downloadqueue.ini dans le
    répertoire MUTE\Settings. Il s'agit d'un fichier texte dans lequel les lignes ont le format 
    suivant :  NOM DE FICHIER * HACHAGE.
    
    '*' est le délimiteur. NOM DE FICHIER estle nom de la version ascii de la version locale
    du fichier dans le répertoire des entrées.HACHAGE est le hachage du 
    fichier distant tel qu'enregistré lors du début du téléchargement. Le fichier 
    Downloadqueue.ini est lu au démarrage de l'application et mis à jour avec les nouvelles données de 
    synchronisation de la file d'attente lors de sa fermeture.
    
    Si un utilisateur veut ajouter manuellement à Downloadqueue.ini des 
    fichiers présents dans le répertoire des fichiers entrants il peut le faire même sans en connaitre 
    la valeur de hachage. Il suffit, alors que le logiciel est arrêté,  d'éditer le fichier 
    Downloadqueue.ini avec un éditeur de texte standard et d'ajouter le nom des fichiers suivi par 
    un '*'.  Par exemple pour ajouter le fichier "supercoolmusic.mp3" il suffit 
    d'ouvrir le fichier
    Downloadqueue.ini et d'ajouter la ligne :
    
    supercoolmusic.mp3 *
    
2.  Francis a revu l'écran de réglages en le dotant à gauche d'un panneau d'options
    à partir duquel on peut appeler les écrans de réglages, d'aide HTML ( 
    qui pointe sur la page de MUTE WIKI, une actualisation restant à faire) ainsi qu'un écran 
    A propos.

3.  Francis a restructuré le code pour permettre de changer de langue 
    sans avoir à redémarrer le programme.
    
4.  Francis a ajouté une prise en charge de fichiers de langues qui peuvent être créés
    et placés dans le répertoire settings pour permettre la reconnaissance d'autres
    langues que celles incorporées dans le logiciel (Anglais-Italien-Allemand-Francais).
    voir : http://board.planetpeer.de/viewtopic.php?t=464

5.  Nouvel écran A Propos avec un déroulant sympa pour les crédits.

6.  Résolution du problème des instances multiples de MFC-MUTE. Lorsque deux 
    instances de MFC-MUTE utilisaient le même port le programme plantait.

7.  Les recherches "hash_" fonctionnent correctement dans cette version.

8.  Le programme se ferme plus rapidement, grace à un code plus épuré.

9.  Traduction en francais

10. Nouvel écran de fermeture qui permet de savoir quand le programme est vraiment terminé.

11. Correction de bugs internes relatifs à l'exportation des résultats de recherche.

12. Amélioration du processus de téléchargement. Les messages sont désormais traités
    par la routine de traitement appropriée. Plus de passage à l'aveugle.

13. Utilisation de la librairie pthreads32 pour éviter le gel des téléchargements. Avec
    cette version Vous ne vous trouverez plus dans la situation de ne pouvoir arrêter un
    téléchargement.

14. Annulation des recherches plus rapide.

15. Correction d'un bug dans le code de hachage qui causait l'arrêt du programme quand
    l'utilisateur démarrait avec un répertoire de hachage invalide.
    
16. Pour les clients MFC-MUTE seulement, ajout de code pour permettre les recherches
    avec '*. Ceci devrait retourner tous les fichiers dans le répertoire des fichiers
    partagés. A utiliser avec modération parce que cela monopolise la bande passante. 
    Mais si vous avez de la chance comme avec GOOGLE, alors essayez.
    
17. Suppression de la limitation à 28 kbytes de la chaine de recherche. Désormais les
    résultats d'une recherche correspondent à tous les critères de la requête.

18. Ajout d'icônes aux menus contextuels dans chaque fenêtre


----------------------------------------------------------------------------------------------------
--- 08-21-2004 version:-----------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
1.  Une modification substantielle de la façon dont les recherches entrantes sont traitées a été
    effectuée. Maintenant, en présence d'une recherche entrante, s'il n'existe pas de hachage pour
    un fichier, aucun hachage n'est créé. Un thread séparé prend en charge la génération de fichiers
    de hachage a partir du répertoire des fichiers partagés de l'utilisateur. 

    Le nouveau thread démarre quand le programme démarre. Ce thread parcourt tous les fichiers
    du répertoire des fichiers partagés et vérifie au cours de cette itération qu'à chaque fichier
    correspond bien un hachage. Si un hachage n'existe pas il en crée un et attend 10 secondes jusqu'à
    ce qu'il puisse créer le fichier de hachage suivant. De plus, la fonction qui assure concrètement
    le hachage est désormais dotée d'un limiteur, de telle sorte que les gros fichiers ne plantent pas 
    totalement l'application. Alors que le code de hachage boucle toutes les trois
    secondes, cette fonction introduit une pause d'une seconde. Ceci garantit que le hachage ne porte
    pas préjudice aux threads d'autres applications.    
    
2.  Création d'un installateur complet avec Inno Setup 4.2.7 ISPP 1.2.1.295 et ISTool 4.2.7.
    -- Le nouvel installateur génère des installations en allemand, italien et anglais.
    -- Le nouvel installateur place une icone sur le bureau et dans le groupe de programmes.
    -- Le nouvel installateur place le nouveau Seed Node Updater (qui met à jour les noeuds de départ)
       dans le répertoire settings.
    -- Le nouvel installateur place les fichiers batch sur le bureau pour le seed node updater qui mettra à 
       jours les préférences de l'utilisateur s'agissant des noeuds de départ et des caches web.
       
----------------------------------------------------------------------------------------------------       
--- 08-07-2004 version:-----------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
1.  Support pour plusieurs langues ajouté à partir de l'éditeur de ressources de MS Visual C++. Maintenant
    MUTE MFC est disponible en anglais,allemand et italien.

--- 7-29-2004 version de débogage : executable seulement (source disponible sur demande) -------
1.  la version 7-28 avait un bug concernant le téléchargement quand des fichiers en lecture seule étaient partagés...
    Le programme plantait lors de la tentative de transférer les fichiers à partir du répertoire des fichiers entrants
    vers un répertoire en lecture seule (par exemple un lecteur CD) parce que les pointeurs de fichiers n'étaient pas
    pas validés dans le code sous-jacent du coeur MUTE. Cela a été résolu rapidemment... Désolé pour ces versions
    consécutives à intervalles rapprochés.

	-- Aucune nouvelle fonctionnalité dans cette version
  
----------------------------------------------------------------------------------------------------
--- 7-28 version de débogage : exécutable seulement (source disponible sur demande) -------
----------------------------------------------------------------------------------------------------
1. La fenêtre des préférences a changé. Elle propose de nouveaux réglages et des liens vers des sites web
   importants concernant MUTE
2. Les utilisateurs peuvent désormais choisir leurs répertoires de partage, téléchargement et entrants.
   - Cela permet de partager des fichiers stockés sur des supports en lecture seule comme des  CDs. 
   Cependant... pour le moment, les fichiers entrants ne seront pas transférés après achèvement
   vers le répertoire des fichiers partagés. (Cela sera résolu dans une prochaine version après que
   l'on m'aura donné quelques idées pour remédier à cela).
3. Nouveau bouton sur la fenêtre téléchargement pour parcourir le répertoire des fichiers entrants.

----------------------------------------------------------------------------------------------------
--- 7-24 version de débogage : exécutable seulement (source disponible sur demande) -------
----------------------------------------------------------------------------------------------------
1. Réglages du partage désormais possibles à partir de la fenêtre de préférences.
2. Bouton de purge du hachage sur la fenêtre de recherche.
3. En cas de téléchargement déjà présent et en cours, le même fichier ne peut plus être ajouté pour être téléchargé
   une deuxième fois.
4. Résolution du problème de la touche Alt-XXX dans la fenêtre de recherche de telle sorte qu'en appuyant sur 
   Alt-T lors d'une recherche cela sera réellement équivalent à appuyer sur le bouton stop, tandis qu'avant cela ne
   fonctionnait pas.

----------------------------------------------------------------------------------------------------
--- 7-14 version de débogage : exécutable seulement (source disponible sur demande) -------
----------------------------------------------------------------------------------------------------
1. Fenêtre des preférences : nouveau bouton pour effacer les fichiers annulés.
   Permet d'éviter la suppression des téléchargements annulés/en echec du répertoire entrant de Mute.
2. Fenêtre des preférences : plus de message demandant de redémarrer le programme après avoir enregistré les préférences.
3. Fenêtre de recherche : nouvelle boite de filtre.
   Maintenant vous pouvez chercher sans avoir à éliminer les débris des recherches précédentes.
4. Fenêtre de téléchargement : maintenant il est possible reprendre le cours des téléchargements
5. Fenêtre de téléchargement : bouton pour purger le répertoire des fichiers entrants de Mute.
   Il n'est plus nécessaire de l'ouvrir pour supprimer les fichiers -- vais peut-être faire de même pour le
   répertoire de hachage.

----------------------------------------------------------------------------------------------------
--- 6-15 version release: code source disponible -------
----------------------------------------------------------------------------------------------------
0. Icones en couleurs (Cela met un peu de piment dans la vie)
1. Le programme démarre sur la fenêtre de connection
2. Résolution du bug écran de connection invalide (en cas de saisie d'une mauvaise adresse IP)
3. Boite de liste avec possibilité de tri et entêtes déplaçables
4. Fenêtre de recherche avec fichiers colorées (selon le type...audio en rouge,vidéo en vert, documents en bleu,
   noir en cas de type inconnu)
5. Fenêtre de recherche : ajout d'icones système et du type de fichier dans la boite de liste (quand disponible)
6. Fenêtre de recherche : toutes les colonnes supportent le tri et les entêtes sont désormais mobiles
7. Fenêtre de recherche : menu contextuel proposant le téléchargement (click droit sur la souris)
8. Les fichiers ajoutés aux téléchargements sont surlignées en jaune et marqués comme ajoutés aux téléchargements.
9. Fenêtre de recherche : possibilité d'exporter les résultats dans un fichier utilisant la virgule comme délimiteur
10. Fenêtre de recherche : bouton pour effacer les résultats de la recherche
11. Fenêtre de recherche : possibilité de naviguer dans la liste de recherche avec les touches de déplacement et de selectionner
    les fichiers à télécharger en appuyant sur la touche entrée.
12. Fenêtre de téléchargement : sur click droit menu contextuel : éffacé, terminé, annulé, en attente, etc... 
13. Fenêtre de téléchargement : toutes les colonnes supportent le tri et les entêtes sont désormais mobiles
14. Fenêtre de téléchargement : bouton pour parcourir les fichiers partagés
15. Fenêtre de téléchargement : boutons pour effacement terminé, en attente et annulation...
16. Fenêtre de téléchargement : items colorés... bleu == terminé, rouge == en attente/échec/en cours d'annulation/annulé,
    vert == téléchargement en cours
17. Fenêtre de partage : effacement des selections dans le  menu contextuel accessible par un click droit
18. Fenêtre de partage : toutes les colonnes supportent le tri et les entêtes sont désormais mobiles
19. Fenêtre de partage : boutons pour retirer et effacer (efface seulement si terminé ou bloqué) les partages en cours
    réapparaitront lors de la prochaine requête de tronçon.
20. Fenêtre de partage : items colorés... bleu == terminé, rouge == en attente/échec,vert == en cours
21. Info-bulles avec informations faciles à lire