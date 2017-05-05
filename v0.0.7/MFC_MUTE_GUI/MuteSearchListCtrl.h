//this file is derived from part of eMule
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
#include "TitleMenu.h"
#include "MuleListCtrl.h"
#include "ExternString.h"

using namespace std;

enum SEARCH_LIST_RCLK_MENU_ENUMS
{	
	MUTE_SEARCH_MENU_DOWNLOAD = MP_CANCEL + 1
};

class CMuteSearchListItem
{
public:
	CMuteSearchListItem() : m_bIsDownloading(false) {}

private:
	CExternStr      m_ExtStr;

public:
	CString	m_strFileName;
	unsigned __int64 m_unFileSize;
	CString	m_strFileSize;
	CString	m_strFileExt;
	CString	m_strFileType;
	CString	m_strHash;
	CString m_strHostVirtualAddress;
	bool	m_bIsDownloading;
};


// CMuteSearchListCtrl
class CMuteSearchListCtrl : public CMuleListCtrl
{
	DECLARE_DYNAMIC(CMuteSearchListCtrl)

public:
	void SetStrings();
	CMuteSearchListCtrl();
	virtual ~CMuteSearchListCtrl();
	//virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void	Init();
	void	AddSearchResult( CMuteSearchListItem * pListItem );
	vector<bool> m_vecOldSortAscending;
	inline int TakeSemaphore() { return (m_semListOperations.Lock(1500)); }
	inline void ReleaseSemaphore() { m_semListOperations.Unlock(); }
	inline void	ResetCaches() 
	{
		m_aExtToSysImgIdx.RemoveAll();
		m_aExtToSysFileType.RemoveAll();
	}
protected:
	int				m_iColumns;
	bool			m_bSetImageList;
	HIMAGELIST		m_hSystemImageList;
	CMapStringToPtr m_aExtToSysImgIdx;
	CMapStringToPtr m_aExtToSysFileType;
	CSize			m_sizSmallSystemIcon;
	CSemaphore		m_semListOperations;
	CExternStr      m_ExtStr;

	void	CreateMenu();

	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	afx_msg void SearchDownloadClick( WPARAM wParam, LPARAM lParam );
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnCustomdraw ( NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg	void OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnDeleteallitems(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg	void OnNMDblclkSearchlist( NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	
	void UpdateToolTips(void);
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()
private:
	int		GetFileTypeSystemImageIdx( LPCTSTR pszFilePath, int iLength, CString &strExtension, CString &strFileType );
	CTitleMenu		m_RightClkMenu;
	CToolTipCtrl	m_ToolTip;
	CString			m_strToolTip;
};
