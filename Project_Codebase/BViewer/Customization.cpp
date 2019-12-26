// Customization.cpp : Implementation file for the CCustomization class,
//	which stores the data input from the CCustomizePage property page.
//
//	Written by Thomas L. Atwood
//	P.O. Box 1089
//	West Fork, Arkansas 72774
//	(479)445-4690
//	TomAtwood@Earthlink.net
//
//	Copyright © 2010 CDC
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//	
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.
//
#include "StdAfx.h"
#include "Module.h"
#include "Customization.h"
#include "BViewer.h"

extern CBViewerApp				ThisBViewerApp;
extern CONFIGURATION			BViewerConfiguration;
extern LIST_HEAD				RegisteredUserList;
extern BOOL						bOKToSaveReaderInfo;


CCustomization::CCustomization( void )
{
	ResetToDefaultValues();
}


CCustomization::~CCustomization( void )
{
}


void CCustomization::ResetToDefaultValues()
{
	m_DisplayAssignments = ASSIGN_STD_DISPLAY_AUTO | ASSIGN_STUDY_DISPLAY_AUTO;
	m_StudyInformationDisplayEmphasis = INFO_EMPHASIS_STUDY;
	m_WindowingAlgorithmSelection = SELECT_LINEAR_WINDOWING;
	m_PrimaryMonitorWidthInMM = 0;
	m_PrimaryMonitorHeightInMM = 0;
	m_PrimaryMonitorGrayScaleBitDepth = 8;
	m_Monitor2WidthInMM = 335;
	m_Monitor2HeightInMM = 424;
	m_Monitor2GrayScaleBitDepth = 8;
	m_Monitor3WidthInMM = 335;
	m_Monitor3HeightInMM = 424;
	m_Monitor3GrayScaleBitDepth = 8;
	m_NumberOfRegisteredUsers = 0;
	memset( &m_ReaderInfo, '\0', sizeof( READER_PERSONAL_INFO ) );
	bOKToSaveReaderInfo = TRUE;
	memset( &m_CountryInfo, '\0', sizeof(COUNTRY_INFO) );
}


BOOL CCustomization::CustomizationLooksReasonable()
{
	BOOL			bEmphasisIsOK;
	BOOL			bDisplayAssignmentIsOK;
	BOOL			bCustomizationIsOK;

	if ( m_WindowingAlgorithmSelection != SELECT_LINEAR_WINDOWING && m_WindowingAlgorithmSelection != SELECT_SIGMOID_WINDOWING )
		 m_WindowingAlgorithmSelection = SELECT_LINEAR_WINDOWING;
	
	bEmphasisIsOK = ( ( m_StudyInformationDisplayEmphasis & ~INFO_EMPHASIS_MASK ) == 0 ) &&					// No stray bits set.
						( ( m_StudyInformationDisplayEmphasis & INFO_EMPHASIS_MASK ) != 0 );				// At least one mask bit set.
	bDisplayAssignmentIsOK = ( ( m_DisplayAssignments & ~( ASSIGN_STD_MASK | ASSIGN_STUDY_MASK ) ) == 0 ) &&	// No stray bits set.
						( ( m_DisplayAssignments & ASSIGN_STD_MASK ) != 0 ) &&								// A standard monitor specified.
						( ( m_DisplayAssignments & ASSIGN_STUDY_MASK ) != 0 );								// A subject study monitor specified.
	bCustomizationIsOK = bEmphasisIsOK && bDisplayAssignmentIsOK;
	
	return bCustomizationIsOK;
}


void CCustomization::ReadUserList()
{
	READER_PERSONAL_INFO	ReaderInfo;
	READER_PERSONAL_INFO	*pNewReaderInfo;
	char					ReaderInfoFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	FILE					*pReaderInfoFile;
	size_t					nBytesToRead;
	size_t					nBytesRead;
	BOOL					bFileHasBeenCompletelyRead;

	m_NumberOfRegisteredUsers = 0;
	strcpy( ReaderInfoFileSpec, "" );
	strncat( ReaderInfoFileSpec, BViewerConfiguration.ConfigDirectory, FULL_FILE_SPEC_STRING_LENGTH - 1 );
	LocateOrCreateDirectory( ReaderInfoFileSpec );	// Ensure directory exists.
	if ( ReaderInfoFileSpec[ strlen( ReaderInfoFileSpec ) - 1 ] != '\\' )
		strcat( ReaderInfoFileSpec, "\\" );
	strncat( ReaderInfoFileSpec, "CriticalData1.sav",
				FULL_FILE_SPEC_STRING_LENGTH - 1 - strlen( ReaderInfoFileSpec ) );
	pReaderInfoFile = fopen( ReaderInfoFileSpec, "rb" );
	if ( pReaderInfoFile != 0 )
		{
		ThisBViewerApp.EraseUserList();
		bFileHasBeenCompletelyRead = FALSE;
		while( !bFileHasBeenCompletelyRead )
			{
			nBytesToRead = sizeof( READER_PERSONAL_INFO );
			nBytesRead = fread( &ReaderInfo, 1, nBytesToRead, pReaderInfoFile );
			bFileHasBeenCompletelyRead = ( nBytesRead < nBytesToRead );
			if ( !bFileHasBeenCompletelyRead )
				{
				pNewReaderInfo = (READER_PERSONAL_INFO*)malloc( sizeof(READER_PERSONAL_INFO) );
				if ( pNewReaderInfo != 0 )
					{
					memcpy( pNewReaderInfo, &ReaderInfo, sizeof(READER_PERSONAL_INFO) );
					AppendToList( &RegisteredUserList, (void*)pNewReaderInfo );
					m_NumberOfRegisteredUsers++;
					}
				}
			}
		fclose( pReaderInfoFile );
		}
}


void CCustomization::WriteUserList()
{
	LIST_ELEMENT			*pUserListElement;
	READER_PERSONAL_INFO	*pReaderInfo;
	char					ReaderInfoFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	FILE					*pReaderInfoFile;
	size_t					nBytesWritten;

	strcpy( ReaderInfoFileSpec, "" );
	strncat( ReaderInfoFileSpec, BViewerConfiguration.ConfigDirectory, FULL_FILE_SPEC_STRING_LENGTH - 1 );
	LocateOrCreateDirectory( ReaderInfoFileSpec );	// Ensure directory exists.
	if ( ReaderInfoFileSpec[ strlen( ReaderInfoFileSpec ) - 1 ] != '\\' )
		strcat( ReaderInfoFileSpec, "\\" );
	strncat( ReaderInfoFileSpec, "CriticalData1.sav",
				FULL_FILE_SPEC_STRING_LENGTH - 1 - strlen( ReaderInfoFileSpec ) );
	pReaderInfoFile = fopen( ReaderInfoFileSpec, "wb" );
	if ( pReaderInfoFile != 0 )
		{
		pUserListElement = RegisteredUserList;
		while ( pUserListElement != 0 )
			{
			pReaderInfo = (READER_PERSONAL_INFO*)pUserListElement -> pItem;
			nBytesWritten = fwrite( (void*)pReaderInfo, 1, sizeof( READER_PERSONAL_INFO ), pReaderInfoFile );
			pUserListElement = pUserListElement -> pNextListElement;
			}
		fclose( pReaderInfoFile );
		}
}



