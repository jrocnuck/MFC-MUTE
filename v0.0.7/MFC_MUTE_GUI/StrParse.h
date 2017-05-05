//*******************************************************************            
//  FILE:       StrParse.H              
//  AUTHOR:     J. Eric Nuckols
//  COMPONENT:  CStringParser
//  DATE:       11.08.2002
//  COMMENTS:   This is a generic class for parsing ASCII strings
//				that have delimited fields... such as comma or tab delimited.
//				It is up to the user to determine the usage.
//	
//				This uses the STL string template/class and should compile
//				on just about any operating system that can handle C++
//				and STL.
//
// Copyright (C) 2000 - Present JROC (aka J. Eric Nuckols)
// All rights reserved - not to be sold.
// Please, if you are going to use this, keep the header and copyright info
// intact as is.
///////////////////////////////////////////////////////////////////////////////
//*******************************************************************



// Includes
#include <string>
#include <vector>
using namespace std;

#pragma warning( disable:4786 )

#ifndef __STRINGPARSER_H__
#define __STRINGPARSER_H__

class CStringParser
{
public:
/////////////////
// Constructors 
/////////////////
	CStringParser( BOOL skip_spaces_flag = FALSE )
	{
		m_nDelim = ','; // default delimiter is a comma
		skip_spaces = skip_spaces_flag;
	}
	
	CStringParser( string& strInput , const char delim, BOOL skip_spaces_flag = FALSE )
	{
		skip_spaces = skip_spaces_flag;
		m_nDelim = delim;
		m_strInput = strInput.c_str();
		vFindSepLocations();
		vFillFields();
	}
	

	CStringParser( const char * strInput , const char delim, BOOL skip_spaces_flag = FALSE )
	{
		m_nDelim = delim;
		skip_spaces = skip_spaces_flag;
		if ( strInput != NULL )
		{
			m_strInput = strInput;
			vFindSepLocations();
			vFillFields();
		}
	}

	CStringParser( string& strInput, BOOL skip_spaces_flag = FALSE)
	{
		m_nDelim = ','; // default delimiter is a comma
		skip_spaces = skip_spaces_flag;
		m_strInput = strInput.c_str();
		vFindSepLocations();
		vFillFields();
	}
	
	CStringParser( const char * strInput, BOOL skip_spaces_flag = FALSE)
	{
		m_nDelim = ','; // default delimiter is a comma
		skip_spaces = skip_spaces_flag;
		if ( strInput != NULL )
		{
			m_strInput = strInput;
			vFindSepLocations();
			vFillFields();
		}
	}

	CStringParser( CStringParser& copy )
	{
		int i;
		m_nDelim = copy.m_nDelim;
		m_strInput = copy.m_strInput;
		skip_spaces = copy.skip_spaces;
		m_strOutput.resize( copy.m_strOutput.size() );
		for ( i = 0; i < (int)m_strOutput.size(); i++ )
			m_strOutput[i] = copy.m_strOutput[i];
		m_nSepLocations.resize( copy.m_nSepLocations.size() );
		for( i = 0 ; i < (int)copy.m_nSepLocations.size() ; i++ )
			m_nSepLocations[ i ] = copy.m_nSepLocations[ i ];
	}

//////////////
// Destructor
//////////////
	~CStringParser()
	{
		m_strOutput.erase( m_strOutput.begin(), m_strOutput.end() );
	}
///////////////////
// Overloaded operators
///////////////////
	CStringParser &CStringParser ::operator=( const CStringParser  &copy )
	{
		int i;
		m_nDelim = copy.m_nDelim;
		skip_spaces = copy.skip_spaces;
		m_strInput = copy.m_strInput;
		m_strOutput.resize( copy.m_strOutput.size() );
		for ( i = 0; i < (int)m_strOutput.size(); i++ )
			m_strOutput[i] = copy.m_strOutput[i];		
		m_nSepLocations.resize( copy.m_nSepLocations.size() );
		for( i = 0 ; i < (int)copy.m_nSepLocations.size() ; i++ )
			m_nSepLocations[ i ] = copy.m_nSepLocations[ i ];
		return *this;
	}
////////////////////
// Member Functions
///////////////////
	inline void vGo( const char *strInput , const char delim = ',', BOOL skip_spaces_flag = FALSE )
	{
		m_nDelim = delim;
		skip_spaces = skip_spaces_flag;
		if ( strInput != NULL && strlen(strInput) > 0)
		{
			m_strInput = strInput;
			vFindSepLocations();
			vFillFields();
		}
	}
	inline void vSetDelim( const char delim ){ m_nDelim = delim; }
	inline int nGetNumFields( void ) { return m_strOutput.size(); }
	inline void vRefresh( void )
	{
		vFindSepLocations();
		vFillFields();
	}


	inline void vReturnFields( vector< string >& vec_strFields )
	{
		vRefresh();
		vec_strFields.resize( m_strOutput.size() );
		for( int i = 0 ; i < (int)m_strOutput.size() ; i++ )
		{
			vec_strFields[ i ] = m_strOutput[ i ];
		}
		
	}

	inline void vFindSepLocations( void )
	{
		m_nSepLocations.clear();
		for( int zz = 0 ; zz < (int) m_strInput.size() ; zz++ )
		{
			if( m_nDelim == m_strInput[ zz ] )
				m_nSepLocations.push_back( zz );
		}
		if ( m_nSepLocations.size() == 0 )
			m_nSepLocations.push_back( m_strInput.size() );
		m_nNumFields = m_nSepLocations.size();		
		if( m_nSepLocations[ m_nSepLocations.size() - 1 ] == m_strInput.size() - 1 )
		{
			m_nNumFields++; // there is another field after the last delimiter
			m_nSepLocations.push_back( m_strInput.size() );
		}
		if( ( (int)m_strInput.size() - 1 ) > m_nSepLocations[ m_nSepLocations.size() - 1 ] )
		{
			m_nNumFields++;
			m_nSepLocations.push_back( m_strInput.size() );
		}	
	}

private:
	inline void vFillFields( void )
	{
		int i,j;
		int start,stop;
		string str;
		
		i = 0;
		start = 0;
		m_strOutput.resize( m_nNumFields );
		do
		{
			str.erase( str.begin() , str.end() );
			stop = m_nSepLocations[ i ];
			for( j = start; j < stop ; j++ )
			{
				if ( ( !skip_spaces ) || ( ' ' != m_strInput[ j ] ) )
					str += m_strInput[ j ];
			}
			m_strOutput[i] = str.c_str();
			start = stop + 1;
			i++;
			
		}
		while( i < m_nNumFields );
	}

///////////////////////////
// Public Member variables
///////////////////////////
public:
	string m_strInput;
	BOOL skip_spaces;
	vector< string > m_strOutput;
		
////////////////////////////
// Private Member variables
////////////////////////////
private:
	char m_nDelim;
	vector< int > m_nSepLocations;
	int m_nNumFields;
	
	
};


#endif  //#ifndef __STRINGPARSER_H__


