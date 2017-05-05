//this file is part was converted from the eMule SearchListCtrl by JROC
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

#include <vector>
#include "MuleListCtrl.h"
#include "ExternString.h"

using namespace std;

class CMUTEConnectionListItem
{
public:
	CString	m_strAddress;
	UINT32	m_unPort;
	UINT32	m_unSent;
	UINT32	m_unQueued;
	UINT32	m_unDropped;	
};


// CMUTEConnectionListCtrl
class CMUTEConnectionListCtrl: public CMuleListCtrl
{
	DECLARE_DYNAMIC(CMUTEConnectionListCtrl)

public:
	void SetStrings();
	CMUTEConnectionListCtrl();
	virtual ~CMUTEConnectionListCtrl();
	void	Init();
	void	AddConnectionItem( CMUTEConnectionListItem * pListItem );
	vector<bool> m_vecOldSortAscending;
	

protected:
	int				m_iColumns;
	CSize			m_sizSmallSystemIcon;
	CExternStr      m_ExtStr;


	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	afx_msg	void OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnDeleteallitems(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()
};
