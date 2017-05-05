/* 02-14-2005 JROC -->
    -- In the Shared Files refresh code, added a 1 millisecond sleep call to help prevent there
    thread from starving other threads of CPU cycles.
*/

#include "stdafx.h"
#include "MFC_MUTE_GUI.h"
#include "MuteSharedFilesDlg.h"

#include "minorGems/io/file/File.h"
#include "minorGems/util/SettingsManager.h"
#include "minorGems/crypto/hashes/sha1.h"
#include "MUTE/otherApps/fileSharing/userInterface/common/formatUtils.h"
#include "MUTE/otherApps/fileSharing/fileShare.h"
#include <stdio.h>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMuteSharedFilesDlg dialog

bool CMuteSharedFilesDlg::m_bAppIsClosing = false;

CMuteSharedFilesDlg::CMuteSharedFilesDlg(CWnd* pParent /*=NULL*/)
: CResizableDialog(CMuteSharedFilesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMuteSharedFilesDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_vecSharedFileItems.resize(0);
}

//*******************************************************************
//  FUNCTION:   -	~CMuteSharedFilesDlg
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
CMuteSharedFilesDlg::~CMuteSharedFilesDlg()
{
	int i;
	
	for( i = 0; i < (int)m_vecSharedFileItems.size(); i++ )
	{
		delete m_vecSharedFileItems[i];
	}
}

void CMuteSharedFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMuteSharedFilesDlg)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_SHARED_FILES_LIST, m_SharedFilesList);
	DDX_Control(pDX, IDC_REFRESH_BUTTON, m_btnRefresh);
	DDX_Control(pDX, IDC_EXPORT_SHARED_FILES_LIST_BUTTON, m_btnExportlist);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMuteSharedFilesDlg, CResizableDialog)
//{{AFX_MSG_MAP(CMuteSharedFilesDlg)	
	ON_WM_CLOSE()
	ON_MESSAGE( SHARED_FILES_UPDATE_EVENT, OnSharedFileUpdate )	
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP	
	ON_BN_CLICKED(IDC_EXPORT_SHARED_FILES_LIST_BUTTON, OnBnClickedExportSearchResultsButton)
	ON_BN_CLICKED(IDC_REFRESH_BUTTON, OnBnClickedRefresh)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMuteSharedFilesDlg message handlers

BOOL CMuteSharedFilesDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( (pMsg->wParam == VK_ESCAPE) || (pMsg->wParam == VK_RETURN) )
		{
			// Don't close the window if the user hits the ESC key.
			return FALSE;
		}
	}
	
	return CResizableDialog::PreTranslateMessage(pMsg);
}

//*******************************************************************
//  FUNCTION:   -	OnInitDialog()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
BOOL CMuteSharedFilesDlg::OnInitDialog()
{
	CString	str;
	CResizableDialog::OnInitDialog();
	ShowSizeGrip(FALSE);
	// vector shadows the visible UploadList, maintaining an upload ID for
    // each list entry.
	m_SharedFilesList.SetParent((CWnd *) this );
	m_SharedFilesList.Init();
	
	AddAnchor( IDC_SHARED_FILES_LIST, TOP_LEFT, BOTTOM_RIGHT );
	AddAnchor( m_btnRefresh, BOTTOM_CENTER );
	AddAnchor( m_btnExportlist, BOTTOM_CENTER );
	
	PostMessage( SHARED_FILES_UPDATE_EVENT, NULL, NULL );
	((CStatic*)GetDlgItem(IDC_FILES_ICO))->SetIcon((HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SHARED_FILES_ICON), IMAGE_ICON, 16, 16, 0));
	    
	return TRUE; // return TRUE unless you set the focus to a control
}

/*
//*******************************************************************
//  FUNCTION:    -	OnClose
//  RETURNS:     -	
//  PARAMETERS:  -	
//  COMMENTS:	 -	
//*******************************************************************
*/
void CMuteSharedFilesDlg::OnClose()
{	
	m_bAppIsClosing = true;
}

//*******************************************************************
//  FUNCTION:   -	OnSharedFileUpdate()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
LRESULT CMuteSharedFilesDlg::OnSharedFileUpdate(WPARAM wParam, LPARAM lParam)
{
	int		i;
	char	*sharedPath		= NULL;
	char	*muteHashDir		= NULL;
	File	*sharedDirectory	= NULL;
	File	*hashDirectory		= NULL;
	int		numChildren;
	int		maxDepth;
	char	found;
	MSG		msg;	
	CString	strTemp;
	
	if( !m_SharedFilesList.LockList(100000) )
	{
		return (LRESULT) FALSE;
	}	

	m_SharedFilesList.LockWindowUpdate();
	m_btnRefresh.EnableWindow(FALSE);
	GetDlgItem(IDC_EXPORT_SHARED_FILES_LIST_BUTTON)->EnableWindow(FALSE);
	
	for( i = 0; i < (int)m_vecSharedFileItems.size(); i++ )
	{
		delete m_vecSharedFileItems[i];
	}

	m_SharedFilesList.DeleteAllItems();

	maxDepth = SettingsManager::getIntSetting( "maxSubfolderDepth", &found );

	if( !found || maxDepth < 0 ) 
	{
		// default max depth of 10
		maxDepth = 10;
	}

	sharedPath = muteShareGetSharingPath();	
    sharedDirectory = new File( NULL, sharedPath );
	delete [] sharedPath;

	muteHashDir = muteShareGetHashFilesPath();                
	hashDirectory = new File( NULL, muteHashDir );		
	delete [] muteHashDir;

	if( sharedDirectory->exists() && sharedDirectory->isDirectory() ) 
	{
		numChildren = 1;
		File **childFiles = sharedDirectory->getChildFilesRecursive( maxDepth, &numChildren );

		if( childFiles != NULL ) 
		{				
			m_vecSharedFileItems.resize(0);
			m_vecSharedFileItems.reserve( numChildren );						

			for( i=0; i < numChildren && (!m_bAppIsClosing); i++ ) 
			{
				YieldProcessor();
				// we need this to be able to allow other threads/windows to process
				// messages and not hang up!
				while( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
				{															
					::DispatchMessage( &msg );					
				}

				if( !childFiles[i]->isDirectory() )
				{
					CMuteSharedFileListItem		*listitem = new CMuteSharedFileListItem();

					char *fileName = muteShare_internalFileToEncodedPath( childFiles[i] );
					if( fileName != NULL ) 
					{
						// check for a cached hash
						// store hash of file contents using
						// hash of file name as the file name
						// we need to do this because file
						// names are now paths into subdirs

						// for example, if our file is
						// test/music/song.mp3
						// we cannot store the hash in
						// a file called "MUTE_test/music/song.mp3"
						// since this is not a valid file name
						// snag the hash of the file!
						char *hashFileName = computeSHA1Digest( fileName );
						File *hashFile = NULL;
						
						if( hashDirectory->exists() )
						{
							hashFile = hashDirectory->getChildFile( hashFileName );
						}

						delete [] hashFileName;

						if( NULL != hashFile )
						{
							long hashModTime = hashFile->getModificationTime();
							long fileModTime = childFiles[i]->getModificationTime();

							char *hashString = NULL;
							
							if( hashFile->exists() && hashModTime >= fileModTime ) 
							{
								// cached hash is up-to-date
								hashString = hashFile->readFileContents();
								listitem->m_strHash = hashString;
							}
							else
							{
								listitem->m_strHash.Empty();
							}

							if( NULL != hashString )
							{
								delete [] hashString;
							}
						
							delete hashFile;						
						}
						else
						{
							listitem->m_strHash.Empty();
						}

						listitem->m_strFileName = fileName;
						//listitem->ulSize = childFiles[i]->getLength(); -- future.. I'm sure somebody will request it.
						delete [] fileName;

						m_vecSharedFileItems.push_back( listitem );
						m_SharedFilesList.AddSharedFileItem( listitem );

						strTemp.Format( "%s (%d)",
								m_ExtStr.LoadString( IDS_TAB_TITLE_SHAREDFILES_TEXT_ENG + g_unStringLanguageIdOffset),
							    m_vecSharedFileItems.size() );
						GetDlgItem( IDC_SHARED_FILES_STATIC )->SetWindowText(strTemp);
					}
				}							
			}

			for( i=0; i<numChildren; i++ ) 
			{
				delete childFiles[i];
			}

			delete [] childFiles;
		}
	}

	delete sharedDirectory;
	delete hashDirectory;

	m_SharedFilesList.UnlockWindowUpdate();
	m_btnRefresh.EnableWindow(TRUE);
	GetDlgItem(IDC_EXPORT_SHARED_FILES_LIST_BUTTON)->EnableWindow(TRUE);

	m_SharedFilesList.UnlockList();
	m_SharedFilesList.Invalidate();	

	return (LRESULT) TRUE;
}


//*******************************************************************
//  FUNCTION:   -	SetStrings()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSharedFilesDlg::SetStrings()
{
	CString strTemp;

	m_SharedFilesList.SetStrings();

	m_btnRefresh.SetIcon( IDI_REFRESH_ICON );
	m_btnRefresh.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 200);	
	
	strTemp = m_ExtStr.LoadString( IDS_REFRESH_BUTTON_TEXT_ENG + g_unStringLanguageIdOffset );
	m_btnRefresh.SetWindowText( strTemp );

	m_btnExportlist.SetIcon( IDI_EXPORT_RESULTS_ICON );
	m_btnExportlist.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 200);
	strTemp = m_ExtStr.LoadString(IDS_SHARED_FILES_EXPORT_BUTTON_TEXT_ENG + g_unStringLanguageIdOffset);
	m_btnExportlist.SetWindowText( strTemp );
	
	strTemp.Format( "%s (%d)",
		          m_ExtStr.LoadString( IDS_TAB_TITLE_SHAREDFILES_TEXT_ENG + g_unStringLanguageIdOffset),
				  m_vecSharedFileItems.size() );
	GetDlgItem( IDC_SHARED_FILES_STATIC )->SetWindowText(strTemp);
}

/*
//*******************************************************************
//  FUNCTION:    -	OnShowWindow()
//  RETURNS:     -	
//  PARAMETERS:  -	
//  COMMENTS:	 -	
//*******************************************************************
*/
void CMuteSharedFilesDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CResizableDialog::OnShowWindow(bShow, nStatus);

	SetStrings();
}


//*******************************************************************
//  FUNCTION:   -	OnBnClickedExportSearchResultsButton()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Open up a dialog to help folks save the current
//					shared files to a text file. (useful for getting hash)
//*******************************************************************
void CMuteSharedFilesDlg::OnBnClickedExportSearchResultsButton()
{
	CString		strFileNameWithPath;
	CFileDialog oFileDlg( FALSE, "", NULL, NULL ,"Text Files (*.txt)|*.txt||", NULL );
	ofstream	out;
	int			i;	
	CString		strTemp, strAtLeast, strAtMost;
		
	if( m_vecSharedFileItems.size() <= 0 )
		return;

	strTemp = m_ExtStr.LoadString( IDS_SHARED_FILES_EXPORT_DLG_CAPTION_TEXT_ENG + g_unStringLanguageIdOffset );
	oFileDlg.m_ofn.lpstrTitle = (LPCSTR) strTemp;
	
	if( IDOK != oFileDlg.DoModal() )
	{
		return;
	}

	if( !m_SharedFilesList.LockList(100000) )
	{
		return;
	}	

	strFileNameWithPath = oFileDlg.GetPathName();
	
	out.open( (LPCSTR) strFileNameWithPath );
	if( out.is_open() )
	{
		for( i = 0; i < (int) m_vecSharedFileItems.size(); i++ )
		{			
			out << (LPCSTR) m_vecSharedFileItems[i]->m_strFileName;
			out << " , ";			
			out << (LPCSTR) m_vecSharedFileItems[i]->m_strHash;			
			out << endl;		 
		}

		out.close();

		// reset current path // for some reason this export search
		// results causes future downloads to say cannot create
		// apparently the CFileDialog class screws stuff up by
		// changing the current directory.. theoretically... 
		// this program should use absolute directories rather than
		// relative dirs.. and this wouldn't be a problem.
		if( ! ( (CMUTEMFC2App *) AfxGetApp() )->m_strStartingPath.IsEmpty() )
		{
			SetCurrentDirectory( (LPCSTR) ( (CMUTEMFC2App *) AfxGetApp() )->m_strStartingPath );
		}
	}

	m_SharedFilesList.UnlockList();
}

void CMuteSharedFilesDlg::OnBnClickedRefresh()
{
	PostMessage( SHARED_FILES_UPDATE_EVENT, NULL, NULL );
}
