// Customization.h : Header file defining the structure of the CCustomization class,
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
#pragma once

#include "Configuration.h"

typedef struct
	{
	char				CountryName[ MAX_NAME_LENGTH ];
	int					DateFormat;
							#define		DATE_FORMAT_UNSPECIFIED		0
							#define		DATE_FORMAT_YMD				1
							#define		DATE_FORMAT_DMY				2
							#define		DATE_FORMAT_MDY				3
	} COUNTRY_INFO;


class CCustomization
{
public:
	CCustomization( void );
	virtual ~CCustomization( void );

// NOTE:  Add any new members at the end, so that CBViewerApp::ReadBViewerConfiguration()
// reading an older version of the configuration doesn't corrupt the newer structure.
	unsigned long			m_DisplayAssignments;
								#define ASSIGN_STD_DISPLAY_AUTO			0x00000001
								#define ASSIGN_STD_DISPLAY_PRIMARY		0x00000002
								#define ASSIGN_STD_DISPLAY_MONITOR2		0x00000004
								#define ASSIGN_STD_DISPLAY_MONITOR3		0x00000008
								#define ASSIGN_STUDY_DISPLAY_AUTO		0x00000010
								#define ASSIGN_STUDY_DISPLAY_PRIMARY	0x00000020
								#define ASSIGN_STUDY_DISPLAY_MONITOR2	0x00000040
								#define ASSIGN_STUDY_DISPLAY_MONITOR3	0x00000080

								#define ASSIGN_STD_MASK					0x0000000f
								#define ASSIGN_STUDY_MASK				0x000000f0

	unsigned long			m_StudyInformationDisplayEmphasis;
								#define INFO_EMPHASIS_PATIENT			0x00000001
								#define INFO_EMPHASIS_STUDY				0x00000002
								#define INFO_EMPHASIS_SERIES			0x00000004
								#define INFO_EMPHASIS_IMAGE				0x00000008

								#define INFO_EMPHASIS_MASK				0x0000000f

	unsigned long			m_PrimaryMonitorWidthInMM;
	unsigned long			m_PrimaryMonitorHeightInMM;
	unsigned short			m_PrimaryMonitorGrayScaleBitDepth;
	unsigned long			m_Monitor2WidthInMM;
	unsigned long			m_Monitor2HeightInMM;
	unsigned short			m_Monitor2GrayScaleBitDepth;
	unsigned long			m_Monitor3WidthInMM;
	unsigned long			m_Monitor3HeightInMM;
	unsigned short			m_Monitor3GrayScaleBitDepth;
	
	unsigned long			m_NumberOfRegisteredUsers;
	READER_PERSONAL_INFO	m_ReaderInfo;
	// Reading interpreted .axt files can change the displayed reader information.
	// Saving this info would erase the current user info.  This flag is used to
	// disable reader info saving in cases where it may have been overwritten.
	unsigned long			m_WindowingAlgorithmSelection;
								#define SELECT_LINEAR_WINDOWING		0
								#define SELECT_SIGMOID_WINDOWING	1
	COUNTRY_INFO			m_CountryInfo;
	


// Method prototypes:
//
	void				ReadUserList();
	void				WriteUserList();
	void				ResetToDefaultValues();
	BOOL				CustomizationLooksReasonable();
};


