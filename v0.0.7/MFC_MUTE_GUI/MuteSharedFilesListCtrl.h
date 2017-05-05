//this file (the list control part) is derived from part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#pragma once
#include "stdafx.h"
#include <vector>
#include "MuleListCtrl.h"
#include "TitleMenu.h"
#include "ExternString.h"

using namespace std;

class CMuteSharedFileListItem
{
public:
	CMuteSharedFileListItem()
	{
		memset( m_szFileType, NULL, 80 );		
	}

public:		
	CString			m_strFileName;
	CString			m_strFileExt;
	CString			m_strHash;
//	__uint64		m_unFileSize;
	char			m_szFileType[80];	
};


// CMuteSharedFilesListCtrl
class CMuteSharedFilesListCtrl : public CMuleListCtrl
{
	DECLARE_DYNAMIC(CMuteSharedFilesListCtrl)

public:
	void SetStrings();
	CMuteSharedFilesListCtrl();
	virtual			~CMuteSharedFilesListCtrl();
	void			Init();
	void			AddSharedFileItem( CMuteSharedFileListItem * pListItem );	
	void			UpdateInfo( CMuteSharedFileListItem * pListItem );	
	vector<bool>	m_vecOldSortAscending;

	inline int		LockList( int nTimeOutMS )
	{
		int nRetVal = FALSE;
		if( m_semListOperations.Lock(nTimeOutMS) )
		{
			nRetVal = TRUE;
			m_bStopUpdating = true;
		}
		return( nRetVal );
	}

	inline void		UnlockList()
	{
		m_bStopUpdating = false;
		m_semListOperations.Unlock();
	}

protected:
	bool			m_bStopUpdating;
	int				m_iColumns;
	bool			m_bSetImageList;
	HIMAGELIST		m_hSystemImageList;
	CMapStringToPtr m_aExtToSysImgIdx;
	CSize			m_sizSmallSystemIcon;
	CSemaphore		m_semListOperations;
	CExternStr      m_ExtStr;
	//void	CreateMenu();
	
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	
	afx_msg void OnCustomdraw ( NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg	void OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void UpdateToolTips(void);
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()
private:
	int	 GetFileTypeSystemImageIdx( LPCTSTR pszFilePath, int iLength, CString &strExtension, char * szFileType  );
	CTitleMenu		m_RightClkMenu;
	CToolTipCtrl	m_ToolTip;
	CString			m_strToolTip;
};
