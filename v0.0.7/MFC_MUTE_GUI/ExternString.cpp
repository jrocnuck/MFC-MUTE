/////////////////////////////////////////////////////////////////////////////////////////////
//
// ExternString.cpp : implementation file
//
// This small class is dedicated to the external resource strings 
// 2 functions :
// 
// LoadStringsFromFile : Reads all lines from a language file
//						 parse each line 
//						 add the resulting string to  the global CstringArray
// LoadString          : Read the requested string in the global CStringArray
//                       if there is no CStringArray calls default Cstring::Loadstring
//						 function and reads the string in the .exe embedded string table
// 
//
//////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ExternString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExternStr
/////////////////////////////////////////////////////////////////////////////

CExternStr::CExternStr()
{
	
}

CExternStr::~CExternStr()
{
	
}


/////////////////////////////////////////////////////////////////////////////////
// Function : LoadStringsFromFile
// 
// Action :  1) Load all strings of a langague file (.lng) 
//           2) parse them
//			 3) populate the application global CStringArray
//
// Parameters : CString strFilePath :  File to read in.	
//
// Return value : 
/////////////////////////////////////////////////////////////////////////////////
void CExternStr::LoadStringsFromFile(CString strFilePath)
{
	CString strId;
	CString strLine;
	CString strCaption;
	int index=0;

	strArray.RemoveAll();

	CStdioFile file(strFilePath,CFile::modeRead | CFile::typeText);
	while (file.ReadString(strLine) != NULL)
	{
	    int delpos[4];
		int index=0;
		
		

		for (int pos=0;pos<=strLine.GetLength()-1;pos++)
			{
				if (strLine.GetAt(pos)=='"')
				{
					delpos[index]=pos;					
					index++;
				}
			}

		// mode = 4 """" delimiters			
		if (index==4) 
			{
			// strId is not useful here. Could help in a comparison with the string table captions values
			strId       =strLine.Mid(delpos[0]+1,delpos[1]-(delpos[0]+1));			
			strCaption  =strLine.Mid(delpos[2]+1,delpos[3]-(delpos[2]+1));
			
			strCaption.Replace("\\n","\n");
		    strCaption.Replace("\\r","\r");				
			
            strArray.Add(strCaption);			
			}
			
		//  mode = 2 "" delimiters
		if (index==2) 
			{
				strCaption=strLine.Mid(delpos[0]+1,delpos[1]-(delpos[0]+1));
				strCaption.Replace("\\n","\n");
		        strCaption.Replace("\\r","\r");				
                strArray.Add(strCaption);
			}

	}
	file.Close();	

}
/////////////////////////////////////////////////////////////////////////////////
// Function : LoadString
// 
// Action :   Read a string in the CStringArray 8 If there is no CStringArray 
//            read it the the string table 
//            
//
// Parameters : Id : index identifier of the string 
//
// Return value : The string requested
/////////////////////////////////////////////////////////////////////////////////

CString CExternStr::LoadString(UINT Id)
{
	CString strTemp;
	
	
	// If no CStringArray use the default CString::LoadString and return
	if (strArray.GetSize()==0)		
	{	  
	  strTemp.LoadString(Id);
	  return strTemp;		
	}

	// Else get the string from the global CStringArray and return
	
    strTemp=strArray[Id-1];
	return strTemp;

	// Error message to help developpers 
	if (Id>(UINT)strArray.GetSize())
	{	
		// todo.. need to get this translated
	    MessageBox(NULL,"Erroneous string replaced with \"???\"","String index error",MB_ICONERROR); 
		return "???";
	}
}













