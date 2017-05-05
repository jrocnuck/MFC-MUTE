; -- MUTEInnoInstall.iss --
; Developed by JROC
; 02-09-2005 added Spanish, Danish, Lithuanian
; 08-20-2005 added icon installs, updated for version 0.0.7, etc..etc.. 

[Setup]
AppName={cm:MyAppName}
AppId=fileSharingMUTE
AppVerName={cm:MyAppVerName,0.0.7}
DefaultDirName={pf}\{cm:MyAppName}
DefaultGroupName={cm:MyAppName}
UninstallDisplayIcon={app}\fileSharingMUTE-MFC.exe
VersionInfoDescription=MFC MUTE v0.0.7 Setup
ShowLanguageDialog=yes
WindowVisible=true
AppPublisher=JROC
AppPublisherURL=sourceforge.net/projects/mfc-mute-net
AppSupportURL=www.planetpeer.de
AppUpdatesURL=www.planetpeer.de
AppVersion=MFC MUTE v0.0.7
UninstallDisplayName=MUTE
InternalCompressLevel=max
OutputBaseFilename=MFC-MUTE-0.0.7-Setup
SolidCompression=true
VersionInfoVersion=0.0.7
VersionInfoCompany=(C)2005 JROC - www.planetpeer.de
VersionInfoTextVersion=0.0.7

[Languages]
Name: en; MessagesFile: compiler:Default.isl
Name: de; MessagesFile: compiler:Languages\German.isl
Name: it; MessagesFile: compiler:Languages\Italian.isl
Name: fr; MessagesFile: compiler:Languages\French.isl
Name: sp; MessagesFile: compiler:Languages\Spanish.isl
Name: da; MessagesFile: compiler:Languages\Danish.isl
Name: lith; MessagesFile: compiler:Languages\Lithuanian.isl
Name: turk; MessagesFile: compiler:Languages\Turkish.isl

[Messages]
en.BeveledLabel=English
de.BeveledLabel=Deutsch
it.BeveledLabel=Italian
fr.BeveledLabel=French
sp.BeveledLabel=Spanish
da.BeveledLabel=Danish
lith.BeveledLabel=Lithuanian
turk.BeveledLabel=Turkish

[CustomMessages]
en.MyMsgStartAutomatically=&Start Automatically when Windows Starts
en.MyAppName=MUTE
en.MyAppVerName=MUTE-MFC File Sharing V%1
en.MyMsgStartmenu=&Create Program Group in Start Menu

de.MyMsgStartAutomatically=&Automatisch mit Windows starten
de.MyAppName=MUTE
de.MyAppVerName=MUTE-MFC File Sharing V%1
de.MyMsgStartmenu=&Erzeuge Ordner im Startmenue

it.MyMsgStartAutomatically=&Esegui automaticamente all' avvio di Windows
it.MyAppName=MUTE
it.MyAppVerName=MUTE-MFC File Sharing V%1
it.MyMsgStartmenu=Aggiungi la cartella del programma nel Menu &Start

fr.MyMsgStartAutomatically=&Lancez automatiquement quand Windows commence
fr.MyAppName=MUTE
fr.MyAppVerName=MUTE-MFC File Sharing V%1
fr.MyMsgStartmenu=&Créer le groupe de programme dans le menu Démarrer

sp.MyMsgStartAutomatically=&Lanza automáticamente Cuando Windows comienza
sp.MyAppName=MUTE
sp.MyAppVerName=MUTE-MFC File Sharing V%1
sp.MyMsgStartmenu=&Crear el grupo de programma en el Menú Inicio

da.MyMsgStartAutomatically=&Opståen Automatisk hvor Windows Starter
da.MyAppName=MUTE
da.MyAppVerName=MUTE-MFC File Sharing V%1
da.MyMsgStartmenu=&Skabe Plan Sammenstille i Opståen Menu

lith.MyMsgStartAutomatically=&Start Automatically when Windows Starts
lith.MyAppName=MUTE
lith.MyAppVerName=MUTE-MFC File Sharing V%1
lith.MyMsgStartmenu=&Create Program Group in Start Menu

turk.MyMsgStartAutomatically=&Start Automatically when Windows Starts
turk.MyAppName=MUTE
turk.MyAppVerName=MUTE-MFC File Sharing V%1
turk.MyMsgStartmenu=&Create Program Group in Start Menu

[Dirs]
Name : {app}\MUTE_incoming
Name : {app}\MUTE_hashes
Name : {app}\icons
Name : {app}\icons\defnax16
Name : {app}\icons\defnax32
Name : {app}\icons\defnax32\mfctabicons1-32x32
Name : {app}\icons\defnax32\mfctabicons2-32x32

[Files]
Source: Install Files\fileSharingMUTE-MFC.exe; DestDir: {app}
Source: Install Files\pthreadVSE.dll; DestDir: {app}
Source: Install Files\snudotnet.exe; DestDir: {app}\settings; DestName: SeedNodeUpdater.exe
Source: Install Files\updateWebCaches.bat; DestDir: {app}\settings
Source: Install Files\updateSeedNodes.bat; DestDir: {app}\settings
Source: Install Files\Readme.txt; DestDir: {app}; Languages: en; Flags: isreadme
Source: Install Files\Readme-German.txt; DestName: Liesmich.txt; DestDir: {app}; Languages: de; Flags: isreadme
Source: Install Files\Readme-Italian.txt; DestName: Readme.txt; DestDir: {app}; Languages: it; Flags: isreadme
Source: Install Files\Readme-French.txt; DestName: Readme.txt; DestDir: {app}; Languages: fr; Flags: isreadme
Source: Install Files\Readme-Spanish.txt; DestName: Readme.txt; DestDir: {app}; Languages: sp; Flags: isreadme
Source: Install Files\Readme-Danish.txt; DestName: Readme.txt; DestDir: {app}; Languages: da; Flags: isreadme
Source: Install Files\Readme-Lithuanian.txt; DestName: Readme.txt; DestDir: {app}; Languages: lith; Flags: isreadme
Source: Install Files\Readme-Turkish.txt; DestName: Readme.txt; DestDir: {app}; Languages: turk; Flags: isreadme
Source: Install Files\mfc-mute_language_instructions.htm; DestDir: {app}
Source: Install Files\mfc-mute_language_instructions.htm; DestDir: {app}\Sample Language files
Source: Install Files\Sample Language files\langage1.ico; DestDir: {app}\Sample Language files
Source: Install Files\Sample Language files\langage1.lng; DestDir: {app}\Sample Language files
Source: Install Files\Sample Language files\language2.lng; DestDir: {app}\Sample Language files
Source: Install Files\Sample Language files\language3_lLHT.lng; DestDir: {app}\Sample Language files
Source: Install Files\settings\language.ini; DestDir: {app}\settings; Languages: en
Source: Install Files\settings\GERMAN\language.ini; DestDir: {app}\settings; Languages: de
Source: Install Files\settings\ITALIAN\language.ini; DestDir: {app}\settings; Languages: it
Source: Install Files\settings\FRENCH\language.ini; DestDir: {app}\settings; Languages: fr
Source: Install Files\settings\SPANISH\language.ini; DestDir: {app}\settings; Languages: sp
Source: Install Files\settings\DANISH\language.ini; DestDir: {app}\settings; Languages: da
Source: Install Files\settings\LITHUANIAN\language.ini; DestDir: {app}\settings; Languages: lith
Source: Install Files\settings\TURKISH\language.ini; DestDir: {app}\settings; Languages: turk
Source: Install Files\settings\broadcastProbability.ini; DestDir: {app}\settings
Source: Install Files\settings\continueForwardProbability.ini; DestDir: {app}\settings
Source: Install Files\settings\deleteCanceled.ini; DestDir: {app}\settings;Flags: onlyifdoesntexist
Source: Install Files\settings\downloadChunkRetries.ini; DestDir: {app}\settings
Source: Install Files\settings\downloadFileInfoRetries.ini; DestDir: {app}\settings
Source: Install Files\settings\downloadRetryFreshRouteProbability.ini; DestDir: {app}\settings
Source: Install Files\settings\downloadTimeoutCurrentTimeoutWeight.ini; DestDir: {app}\settings
Source: Install Files\settings\downloadTimeoutMilliseconds.ini; DestDir: {app}\settings
Source: Install Files\settings\downloadTimeoutRecentChunkWeight.ini; DestDir: {app}\settings
Source: Install Files\settings\hashesPath.ini; DestDir: {app}\settings; Flags: onlyifdoesntexist
Source: Install Files\settings\haveGoodSharePath.ini; DestDir: {app}\settings; Flags: onlyifdoesntexist
Source: Install Files\settings\inboundMessageLimit.ini; DestDir: {app}\settings;Flags: onlyifdoesntexist
Source: Install Files\settings\incomingPath.ini; DestDir: {app}\settings; Flags: onlyifdoesntexist
Source: Install Files\settings\logConnectionContents.ini; DestDir: {app}\settings;Flags: onlyifdoesntexist
Source: Install Files\settings\logConnections.ini; DestDir: {app}\settings;Flags: onlyifdoesntexist
Source: Install Files\settings\logDownloadTimeoutChanges.ini; DestDir: {app}\settings
Source: Install Files\settings\logLevel.ini; DestDir: {app}\settings;Flags: onlyifdoesntexist
Source: Install Files\settings\logMessageHistory.ini; DestDir: {app}\settings
Source: Install Files\settings\logRoutingHistory.ini; DestDir: {app}\settings
Source: Install Files\settings\maxConnectionCount.ini; DestDir: {app}\settings ;Flags: onlyifdoesntexist
Source: Install Files\settings\maxDroppedMessageFraction.ini; DestDir: {app}\settings ;Flags: onlyifdoesntexist
Source: Install Files\settings\maxMessageUtility.ini; DestDir: {app}\settings ;Flags: onlyifdoesntexist
Source: Install Files\settings\maxSubfolderDepth.ini; DestDir: {app}\settings
Source: Install Files\settings\maxUploads.ini; DestDir: {app}\settings ; Flags: onlyifdoesntexist
Source: Install Files\settings\maxUploadsPerVip.ini; DestDir: {app}\settings;Flags: onlyifdoesntexist
Source: Install Files\settings\mime.ini; DestDir: {app}\settings
Source: Install Files\settings\muteVersion.ini; DestDir: {app}\settings
Source: Install Files\settings\outboundMessageLimit.ini; DestDir: {app}\settings
Source: Install Files\settings\port.ini; DestDir: {app}\settings; Flags: onlyifdoesntexist
Source: Install Files\settings\README.txt; DestDir: {app}\settings
Source: Install Files\settings\seedHosts.ini; DestDir: {app}\settings;Flags: onlyifdoesntexist
Source: Install Files\settings\sharingPath.ini; DestDir: {app}\settings; Flags: onlyifdoesntexist
Source: Install Files\settings\tailChainDropProbability.ini; DestDir: {app}\settings
Source: Install Files\settings\targetConnectionCount.ini; DestDir: {app}\settings
Source: Install Files\settings\useMajorityRouting.ini; DestDir: {app}\settings
Source: Install Files\settings\utilityAlpha.ini; DestDir: {app}\settings
Source: Install Files\settings\utilityBeta.ini; DestDir: {app}\settings
Source: Install Files\settings\utilityGamma.ini; DestDir: {app}\settings
Source: Install Files\settings\webHostCachePostIntervalSeconds.ini; DestDir: {app}\settings
Source: Install Files\settings\webHostCaches.ini; DestDir: {app}\settings; AfterInstall: AfterMyProgInstall({app})
Source: Install Files\icons\*; DestDir: {app}\icons
Source: Install Files\icons\defnax16\*; DestDir: {app}\icons\defnax16
;Source: Install Files\icons\defnax32\*; DestDir: {app}\icons\defnax32
Source: Install Files\icons\defnax32\mfctabicons1-32x32\*; DestDir: {app}\icons\defnax32\mfctabicons1-32x32
Source: Install Files\icons\defnax32\mfctabicons2-32x32\*; DestDir: {app}\icons\defnax32\mfctabicons2-32x32

[Icons]
Name: {group}\{cm:MyAppName}; Filename: {app}\fileSharingMUTE-MFC.exe; WorkingDir: {app}; Tasks: startmenu
Name: {group}\MUTE updateWebCaches; Filename: {app}\settings\updateWebCaches.bat; WorkingDir: {app}\settings; Tasks: startmenu
Name: {group}\MUTE updateSeedNodes; Filename: {app}\settings\updateSeedNodes.bat; WorkingDir: {app}\settings; Tasks: startmenu
Name: {group}\{cm:UninstallProgram,{cm:MyAppName}}; Filename: {uninstallexe}; Tasks: startmenu
Name: {userstartup}\{cm:MyAppName}; Filename: {app}\fileSharingMUTE-MFC.exe; WorkingDir: {app}; Tasks: startupmenu
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{cm:MyAppName}; Filename: {app}\fileSharingMUTE-MFC.exe; WorkingDir: {app}; Tasks: quicklaunchmenu
Name: {userdesktop}\{cm:MyAppName}; Filename: {app}\fileSharingMUTE-MFC.exe; WorkingDir: {app}; Tasks: desktopicon
Name: {userdesktop}\MUTE updateWebCaches; Filename: {app}\settings\updateWebCaches.bat; WorkingDir: {app}\settings; Tasks: desktopicon
Name: {userdesktop}\MUTE updateSeedNodes; Filename: {app}\settings\updateSeedNodes.bat; WorkingDir: {app}\settings; Tasks: desktopicon

[Tasks]
; The following task doesn't do anything and is only meant to show [CustomMessages] usage
Name: startupmenu; Description: "{cm:MyMsgStartAutomatically}";
Name: startmenu; Description: "{cm:MyMsgStartmenu}";
Name: desktopicon; Description: "{cm:CreateDesktopIcon}";
Name: quicklaunchmenu; Description: "{cm:CreateQuickLaunchIcon}";


[Code]
procedure AfterMyProgInstall(appDirString: String);
var
  hashPathFile: String;
  incomingPathFile: String;
  hashPathDir: String;
  incomingPathDir: String;

begin
  // want to add the default hashes path and incoming path
  // so we create a variable for both of the files hashesPath.ini and incomingPath.ini
	hashPathFile := appDirString + '\settings\hashesPath.ini';
	incomingPathFile := appDirString + '\settings\incomingPath.ini';
	
	// set up the actual string that will be used as the default paths for HASHES and INCOMING
	hashPathDir := appDirString + '\MUTE_hashes';
	incomingPathDir := appDirString + '\MUTE_incoming';

  // replace backslash (\) characters with ascii-hex code %5C
	StringChange(hashPathDir, '\', '%5C');
	StringChange(incomingPathDir, '\', '%5C');
	
  // replace space characters with ascii-hex code %5C
	StringChange(hashPathDir, ' ', '%20');
	StringChange(incomingPathDir, ' ', '%20');

  // update the paths for both the HASH and INCOMING directory inside the files hashesPath.ini and incomingPath.ini
	SaveStringToFile(hashPathFile, hashPathDir, False);
	SaveStringToFile(incomingPathFile, incomingPathDir, False );
end;
