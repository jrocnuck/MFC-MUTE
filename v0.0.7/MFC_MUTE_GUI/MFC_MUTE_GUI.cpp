// MUTEMFC2.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MFC_MUTE_GUI.h"
#include "MFC_MUTE_GUIDlg.h"
#include "GatherRandomnessDlg.h"
#include "KeyLengthDlg.h"
#include "GenerateKeyPairDlg.h"
#include "TimedDlg.h"

#include <Winsock.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// MUTE includes
#include "MUTE/layers/messageRouting/messageRouter.h"
#include "MUTE/otherApps/fileSharing/fileShare.h"
#include "MUTE/common/CryptoUtils.h"

#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/system/Thread.h"
#include "minorGems/util/log/AppLog.h"

#include "ExternString.h"


/////////////////////////////////////////////////////////////////////////////
// CMUTEMFC2App

BEGIN_MESSAGE_MAP(CMUTEMFC2App, CWinApp)
	//{{AFX_MSG_MAP(CMUTEMFC2App)
	//}}AFX_MSG
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMUTEMFC2App construction

CMUTEMFC2App::CMUTEMFC2App()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMUTEMFC2App object

CMUTEMFC2App theApp;

/**
 * Thread that shuts down this MUTE node in the background behind
 * a modal dialog box.
 */

class ShutDownThread : public Thread 
{

    public:
		CExternStr m_ExtStr;
        
        ShutDownThread( CDialog *inDialog )
            : mDialog( inDialog ) 
		{

            start();
        }

        
        
        ~ShutDownThread() 
		{

        }

        
        // implements the Thread interface
        // when thread returns from run method (if joined)
        // then all layers of this MUTE node will have been shut down.
        void run() 
		{
			CWaitCursor wait;
            CString	strTemp;
			CTimedDlg timedDlg;

			// .. ok so the Sleep calls makes you think.. this guy is crazy..
			// well I want the GUI to be on the screen long enough for users to know
			// that the program is actually shutdown.. and long enough to read/see the close
			// down GUI... 
		
			strTemp = m_ExtStr.LoadString( IDS_TRACE_STOPPING_FILE_SHARING_LAYER_ENG + g_unStringLanguageIdOffset );
            TRACE( strTemp );			
			timedDlg.m_objTimedDlgText.SetText(strTemp);
			timedDlg.DisplayWindow();			
			timedDlg.m_objProgress.SetPos(1);
			Sleep(150);		
			muteShareStop();
			Sleep(50);		

			strTemp = m_ExtStr.LoadString( IDS_TRACE_SAVING_RANDOMNESS_ENG + g_unStringLanguageIdOffset );
            TRACE( strTemp );					
			timedDlg.m_objTimedDlgText.SetText(strTemp);
			timedDlg.m_objProgress.SetPos(2);
			Sleep(50);
			            
			char *randState = muteGetRandomGeneratorState();
            if( NULL != randState )
			{
				SettingsManager::setSetting( "randomSeed", randState );
				delete [] randState;
			}
            
			strTemp = m_ExtStr.LoadString( IDS_TRACE_STOPPING_MSG_ROUTER_ENG + g_unStringLanguageIdOffset );
            TRACE( strTemp );
			timedDlg.m_objTimedDlgText.SetText(strTemp);
			timedDlg.m_objProgress.SetPos(3);
			Sleep(50);
			muteStop();
			

			strTemp = m_ExtStr.LoadString( IDS_TRACE_ALL_LAYERS_STOPPED_ENG + g_unStringLanguageIdOffset );
            TRACE( strTemp );
			timedDlg.m_objTimedDlgText.SetText(strTemp);
			timedDlg.m_objProgress.SetPos(4);
			Sleep(500);			
			timedDlg.m_objProgress.SetPos(5);			
			Sleep(50);
			timedDlg.CloseDlgWindow();
		}


    protected:
		CDialog *mDialog;            
    };

/////////////////////////////////////////////////////////////////////////////
// CMUTEMFC2App initialization

BOOL CMUTEMFC2App::InitInstance()
{
	// Standard initialization

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	AfxEnableControlContainer(); // NEW  Enable ActiveX Control 

	char		*szStartingPath = NULL;
	DWORD		dwPathSize;
	CExternStr  objExtStr;	

	// will replace these #ifdefs with reading from a file... 
	char *szLanguage = SettingsManager::getStringSetting( "language" );
	
	if( NULL != szLanguage )
	{
		CString strLanguage = szLanguage;
		delete [] szLanguage;
		
		if( strLanguage.Find( "ENGLISH" ) > - 1 )
		{
			// default to ENGLISH!
			g_unStringLanguageIdOffset = MUTE_ENGLISH;
		}
		if( strLanguage.Find( "ITALIAN" ) > - 1 )
		{
			g_unStringLanguageIdOffset = MUTE_ITALIAN;
		}
		else if( strLanguage.Find( "GERMAN" ) > - 1 )
		{
			g_unStringLanguageIdOffset = MUTE_GERMAN;
		}
		else if( strLanguage.Find( "FRENCH" ) > - 1 )
		{
			g_unStringLanguageIdOffset = MUTE_FRENCH;
		}
		else if( strLanguage.Find( "SPANISH" ) > - 1 )
		{
			g_unStringLanguageIdOffset = MUTE_SPANISH;
		}
		else if( strLanguage.Find( "DANISH" ) > - 1 )
		{
			g_unStringLanguageIdOffset = MUTE_DANISH;
		}
		else if( strLanguage.Find( "LITHUANIAN" ) > - 1 )
		{
			g_unStringLanguageIdOffset = MUTE_LITHUANIAN;
		}
		else if( strLanguage.Find( "TURKISH" ) > - 1 )
		{
			g_unStringLanguageIdOffset = MUTE_TURKISH;
		}
		else if( strLanguage.Right(4) == ".lng" )
		{
			// Maybe we have found a language file ...		
			g_unStringLanguageIdOffset = 0;
			
			// In the affirmative we must verify if we can find it...
			CFileFind file;
			if(file.FindFile(strLanguage,0) )			
			{			
				objExtStr.LoadStringsFromFile(strLanguage);
			}
			else
			{
				g_unStringLanguageIdOffset = MUTE_ENGLISH;			
				SettingsManager::setSetting( "language","ENGLISH" );
			}				
		}
		else
		{
			// default to ENGLISH!
			g_unStringLanguageIdOffset = MUTE_ENGLISH;
		}
	}
	else
	{
		// default to ENGLISH!
		g_unStringLanguageIdOffset = MUTE_ENGLISH;
	}

	m_strStartingPath.Empty();

	dwPathSize = GetCurrentDirectory( NULL, NULL );
	if( dwPathSize > 0 )
	{
		szStartingPath = new char[ dwPathSize + 1 ];
		if( NULL != szStartingPath )
		{
			if( GetCurrentDirectory(  dwPathSize,  szStartingPath ) > 0 )
			{
				m_strStartingPath = szStartingPath;
			}

			delete [] szStartingPath;
		}
	}


	// check if firewall setting exists
    char valueFound;
    SettingsManager::getIntSetting( "behindFirewall", &valueFound );
    
	if( !valueFound ) 
	{
        // ask user about firewall status
		CString	strTemp, strTemp2;
		strTemp = objExtStr.LoadString( IDS_BEHIND_FIREWALL_POPUP_CAPTION_ENG + g_unStringLanguageIdOffset );
		strTemp2 = objExtStr.LoadString( IDS_BEHIND_FIREWALL_QUESTION_ENG + g_unStringLanguageIdOffset );
        int answer = MessageBox( NULL, strTemp2, strTemp,MB_YESNO );
        
        // save the firewall setting
        if( answer == IDYES  ) 
		{
            SettingsManager::setSetting( "behindFirewall", 1 );
		}
        else 
		{
            SettingsManager::setSetting( "behindFirewall", 0 );
		}
	}
	
    char *randomSeed = NULL;
    randomSeed = SettingsManager::getStringSetting( "randomSeed" );
    
    if( randomSeed == NULL ) 
	{
		CGatherRandomnessDlg	dlgRandomness(NULL);
		dlgRandomness.DoModal();
		if( !dlgRandomness.m_strRandomnessString.IsEmpty() )
		{
			randomSeed = stringDuplicate( (LPCSTR) dlgRandomness.m_strRandomnessString );
		}
		else
		{
			randomSeed = new char [100];
			sprintf( randomSeed, "Using randomness saved from last time %d\n", GetTickCount() );
		}
    }
    
	if( NULL != randomSeed )
	{
		muteSeedRandomGenerator( randomSeed );
		delete [] randomSeed;
	}
	
    
    // we've used this seed, so mark it as blank
    // (will be NULL if getStringSetting called again) 
    SettingsManager::setSetting( "randomSeed", "" );
	  	
    int portNumber = SettingsManager::getIntSetting( "port", &valueFound );
	
    if( !valueFound ) 
	{
        portNumber = 4900;
	}
	
    // make sure node RSA keys exist before starting node
    // the node will generate keys itself upon startup, but then
    // we cannot display a proper progress dialog
    
    char *nodePublicKey = SettingsManager::getStringSetting( "nodePublicKey" );
    char *nodePrivateKey = SettingsManager::getStringSetting( "nodePrivateKey" );
	
    if( nodePublicKey == NULL || strcmp( nodePublicKey, "" ) == 0 ||
        nodePrivateKey == NULL || strcmp( nodePrivateKey, "" ) == 0 ) 
	{
		
        if( nodePublicKey != NULL ) {
            delete [] nodePublicKey;
		}
        if( nodePrivateKey != NULL ) {
            delete [] nodePrivateKey;
		}
		
        char keyLengthFound;
        int keyLength = SettingsManager::getIntSetting( "nodeKeySize", &keyLengthFound );
		
        if( !keyLengthFound ) {

			CKeyLengthDlg klengthDlg(NULL);

			klengthDlg.DoModal();
			
			int selection = klengthDlg.m_nKeyLength;
			
            switch( selection ) 
			{
			case 0:
				keyLength = 512;
				break;
			case 1:
				keyLength = 1024;
				break;
			case 2:
				keyLength = 1536;
				break;
			case 3:
				keyLength = 2048;
				break;
			case 4:
				keyLength = 4096;
				break;
			default:
				keyLength = 1024;
				break;
			}						
		}
			
		CGenerateKeyPairDlg keygenDLG(NULL);
		keygenDLG.m_keyLength = keyLength;
		keygenDLG.DoModal();
		
		SettingsManager::setSetting( "nodePublicKey", keygenDLG.m_NodePublicKey );
        SettingsManager::setSetting( "nodePrivateKey", keygenDLG.m_NodePrivateKey );    
	}

	if( NULL != nodePublicKey )
	{
		delete [] nodePublicKey;
	}

	if( NULL != nodePrivateKey )
	{
		delete [] nodePrivateKey;
	}
	
    muteStart( portNumber );
        
    muteShareStart();


    char *logLevelString = SettingsManager::getStringSetting( "logLevel" );

    if( logLevelString != NULL ) {
        // change the default log level

        int newLogLevel = -1;
        
        if( strcmp( logLevelString, "DEACTIVATE_LEVEL" ) == 0 ) {
            newLogLevel = Log::DEACTIVATE_LEVEL;
            }
        else if( strcmp( logLevelString, "CRITICAL_ERROR_LEVEL" ) == 0 ) {
            newLogLevel = Log::CRITICAL_ERROR_LEVEL;
            }
        else if( strcmp( logLevelString, "ERROR_LEVEL" ) == 0 ) {
            newLogLevel = Log::ERROR_LEVEL;
            }
        else if( strcmp( logLevelString, "WARNING_LEVEL" ) == 0 ) {
            newLogLevel = Log::WARNING_LEVEL;
            }
        else if( strcmp( logLevelString, "INFO_LEVEL" ) == 0 ) {
            newLogLevel = Log::INFO_LEVEL;
            }
        else if( strcmp( logLevelString, "DETAIL_LEVEL" ) == 0 ) {
            newLogLevel = Log::DETAIL_LEVEL;
            }
        else if( strcmp( logLevelString, "TRACE_LEVEL" ) == 0 ) {
            newLogLevel = Log::TRACE_LEVEL;
            }

        if( newLogLevel != -1 ) {
            AppLog::setLoggingLevel( newLogLevel );
            }

        delete [] logLevelString;
        }


	CMUTEMFC2Dlg dlg(NULL);
	m_pMainWnd = &dlg;
	dlg.DoModal();
	
	// jroc.. not sure we need this anymore.. 
	// some how.. some way.. it still seems like threads are having a hard time stopping...
	Sleep(500); 
    // start shutdown thread    	

	ShutDownThread * thread = new ShutDownThread( NULL );
	if( NULL != thread )
	{
		thread->join();
		delete thread;
	}

	char *randState = muteGetRandomGeneratorState();
	if( NULL != randState )
	{
		SettingsManager::setSetting( "randomSeed", randState );
		delete [] randState;
	}

	Sleep(300);
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
