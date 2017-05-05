//*******************************************************************
//          
//  ExternStr.h : Header file              
//
//*******************************************************************

#ifndef EXTERNSTR_H_
#define EXTERNSTR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000




class CExternStr 
{	
public:
	CExternStr();
	~CExternStr();

	void LoadStringsFromFile(CString strFilePath);	
	CString LoadString(UINT Id);		
};


#endif // !defined(EXTERNSTR_H
