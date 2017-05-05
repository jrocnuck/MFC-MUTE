// MuteAboutDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mfc_mute_gui.h"
#include "MuteAboutDlg.h"
#include "ColorNames.h"
#include "ExternString.h"
//#include ".\muteaboutdlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMuteAboutDlg dialog


CMuteAboutDlg::CMuteAboutDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CMuteAboutDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMuteAboutDlg)
	//}}AFX_DATA_INIT

	CString strCredits;
	strCredits =	"\tMFC MUTE v0.0.7\n"
					"Copyright © 2004,2005 JROC\n"
					"-->10-12-2005<--\n"
					"APPLICATION IS FREEWARE\n"
					"AND OPEN SOURCE.\n"
					"\rYOU ARE ALLOWED TO MODIFY,\n"
					"\rBUT NOT ALLOWED TO SELL\n"
					"\rFOR A PROFIT!\n\n"
					"\rDevelopers:\n"
					"JROC\n"
					"CINARCHESI\n"
                    "  - Languages, Options panes\n"
                    "DEFNAX\n"
                    "  -GUIs,ICONS\n"
					"MCoder\n"
					"  -GUIs\n"
					"PAOLO\n"    
                    "  -Visual Studio 2k3 fixes\n\n"
					"\rOther code contributors:\n"
					"NATE\n\n"
					"\rTesters:\n"
					"TSAFA\n"
					"TONY\n"
					"CINARCHESI\n"
					"NGLWARCRY\n"
                    "DEFNAX\n"
					"MARKUS\n\n"
					"\rTranslators:\n"
					"NGLWARCRY - ITALIAN\n"
					"MARKUS - GERMAN\n"
					"CINARCHESI - FRENCH\n"
					"TOBIAS - DANISH\n"
					"DA2DA - LITHUANIAN\n"
					"JUAN CARLOS - SPANISH\n"
					"DUIR - SPANISH\n"
					"DEFNAX - TURKISH\n\n"
					"\rWebCache Support:\n"					
                    "JOOP\n"             
                    "http://kr1z.mymute.info/wcman.php\n\n"
					"\rWeb Hosting:\n"
					"www.sourceforge.net/projects/mfc-mute-net\n"
					"MARKUS - www.planetpeer.de\n"
					"PAOLO - xybaron.altervista.org\n"
					"Jason Rohrer- mute-net.sourceforge.net\n\n"
					"\tMUTE CORE ENGINE\n"
					"\rDeveloper:\n"
					"Jason Rohrer\n\n"
					"\rCODE FOR THIS ABOUT CONTROL:\n"
					"Copyright Pablo Software Solutions 2002";					
	
	m_objCredits.SetCredits(strCredits);
}


void CMuteAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMuteAboutDlg)
	DDX_Control(pDX, IDC_CREDITS_CTRL, m_objCredits);
	DDX_Control(pDX, IDC_STATIC_WEBLINK_GROUP, m_oWebResourcesGroup);
	DDX_Control(pDX, IDC_STATIC_MUTE_HOME, m_oMuteHomePageLink);
	DDX_Control(pDX, IDC_STATIC_MUTE_SF_MAIN_HOME, m_oMuteSFMainHomePageLink);
	DDX_Control(pDX, IDC_STATIC_XYBARON_HOME, m_oXybaronHomePageLink);
	DDX_Control(pDX, IDC_STATIC_PLANETPEER_HOME, m_oPlanetPeerHomePageLink);	
	DDX_Control(pDX, IDC_STATIC_MYMUTEINFO_HOME, m_oMyMuteInfoHomePageLink);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMuteAboutDlg, CResizableDialog)
	//{{AFX_MSG_MAP(CMuteAboutDlg)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMuteAboutDlg message handlers

BOOL CMuteAboutDlg::OnInitDialog() 
{	
	CResizableDialog::OnInitDialog();

	ShowSizeGrip(FALSE);

	m_oMuteHomePageLink.SetLink( TRUE );
	m_oMuteHomePageLink.SetFontBold( TRUE );	
	m_oMuteHomePageLink.SetFontUnderline( TRUE );
	m_oMuteHomePageLink.SetTextColor( colBlue );
	m_oMuteHomePageLink.SetLinkCursor( AfxGetApp()->LoadCursor(IDC_HAND_CURSOR) );

	m_oPlanetPeerHomePageLink.SetLink( TRUE );
	m_oPlanetPeerHomePageLink.SetFontBold( TRUE );	
	m_oPlanetPeerHomePageLink.SetFontUnderline( TRUE );
	m_oPlanetPeerHomePageLink.SetTextColor( colBlue );
	m_oPlanetPeerHomePageLink.SetLinkCursor( AfxGetApp()->LoadCursor(IDC_HAND_CURSOR) );

	m_oXybaronHomePageLink.SetLink( TRUE );
	m_oXybaronHomePageLink.SetFontBold( TRUE );	
	m_oXybaronHomePageLink.SetFontUnderline( TRUE );
	m_oXybaronHomePageLink.SetTextColor( colBlue );
	m_oXybaronHomePageLink.SetLinkCursor( AfxGetApp()->LoadCursor(IDC_HAND_CURSOR) );

	m_oMuteSFMainHomePageLink.SetLink( TRUE );
	m_oMuteSFMainHomePageLink.SetFontBold( TRUE );	
	m_oMuteSFMainHomePageLink.SetFontUnderline( TRUE );
	m_oMuteSFMainHomePageLink.SetTextColor( colBlue );
	m_oMuteSFMainHomePageLink.SetLinkCursor( AfxGetApp()->LoadCursor(IDC_HAND_CURSOR) );
	
	m_oMyMuteInfoHomePageLink.SetLink( TRUE );
	m_oMyMuteInfoHomePageLink.SetFontBold( TRUE );	
	m_oMyMuteInfoHomePageLink.SetFontUnderline( TRUE );
	m_oMyMuteInfoHomePageLink.SetTextColor( colBlue );
	m_oMyMuteInfoHomePageLink.SetLinkCursor( AfxGetApp()->LoadCursor(IDC_HAND_CURSOR) );

	AddAnchor( m_objCredits, MIDDLE_CENTER );
	AddAnchor( m_oWebResourcesGroup, BOTTOM_CENTER );
	AddAnchor(m_oPlanetPeerHomePageLink,BOTTOM_CENTER);
	AddAnchor( m_oMuteHomePageLink, BOTTOM_CENTER );
	AddAnchor( m_oXybaronHomePageLink, BOTTOM_CENTER );
	AddAnchor( m_oMuteSFMainHomePageLink, BOTTOM_CENTER );
	AddAnchor( m_oMyMuteInfoHomePageLink, BOTTOM_CENTER );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMuteAboutDlg::SetStrings()
{
    CString strTemp;
	CExternStr  m_ExtStr;	

	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_WEB_GROUP_ENG + g_unStringLanguageIdOffset);
	m_oWebResourcesGroup.SetWindowText(strTemp);

}



void CMuteAboutDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CResizableDialog::OnShowWindow(bShow, nStatus);
		
	SetStrings();
	if( FALSE == bShow )
	{
		// WHEN THE WINDOW GOES AWAY, WE STOP THE ANIMATION
		m_objCredits.StopCredits();
	}
	else
	{
		// WHEN THE WINDOW COMES IN VIEW, WE START THE ANIMATION
		m_objCredits.StartCredits();		
	}	
}
