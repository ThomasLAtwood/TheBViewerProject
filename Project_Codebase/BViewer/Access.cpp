// Access.cpp : Implementation file for user authentication and image security.
//
// NOTE:  This file is NOT open source.  It controls security issues that
// should not be publicly viewable.
//
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
// UPDATE HISTORY:
//
//	*[3] 10/04/2023 by Tom Atwood
//		Changed password checking to check only the characters typed instead of
//		the entire masking field.
//	*[2] 03/13/2023 by Tom Atwood
//		Fixed code security issues.
//	*[1] 12/22/2022 by Tom Atwood
//		Fixed code security issues.
//
//
#define		 _CRT_RAND_S		// *[1] Enable rand_s().

#include "stdafx.h"
#include "BViewer.h"
#include "Module.h"
#include "ReportStatus.h"
#include "Configuration.h"
#include "DiagnosticImage.h"



#pragma pack(push)
#pragma pack(1)		// Pack structure members on 1-byte boundaries.

typedef struct
	{
	char				OwnershipDeclaration[ 348 ];
	char				DicomFormat[ 1716 ];
	unsigned char		Random[ 818 ];
	} ILO_STANDARD_IMAGE_FILE_HEADER;

#pragma pack(pop)


extern CONFIGURATION		BViewerConfiguration;
extern BOOL					bLoadingStandards;

static long					OwnershipDeclarationSize = 346;
extern char					*pOwnershipDeclaration;

typedef struct
	{
	BOOL			bStandardSuccessfullyStored;
	char			*pStandardDisplayName;
	char			*pStandardName;
	char			*pDicomName;
	} STANDARD_FILE_RENAME;

static STANDARD_FILE_RENAME	FilmStandardsRenameTable[] =
	{
	{ FALSE, "Ex1", "0example1", "1.2.804.114118.2.20100209.174742.3502332353.1.1.1" },
	{ FALSE, "Ex2", "0example2", "1.2.804.114118.2.20100209.174742.3502327970.1.1.1" },
	{ FALSE, "1p ", "1p", "1.2.804.114118.2.20100209.174742.3502336264.1.1.1" },
	{ FALSE, "1q ", "1q", "1.2.804.114118.2.20100209.174742.3502352654.1.1.1" },
	{ FALSE, "1r ", "1r", "1.2.804.114118.2.20100209.174742.3502370719.1.1.1" },
	{ FALSE, "1s ", "1s", "1.2.804.114118.2.20100209.174742.3502392181.1.1.1" },
	{ FALSE, "1t ", "1t", "1.2.804.114118.2.20100209.174742.3502418113.1.1.1" },
	{ FALSE, "2p ", "2p", "1.2.804.114118.2.20100209.174742.3502341143.1.1.1" },
	{ FALSE, "2q ", "2q", "1.2.804.114118.2.20100209.174742.3502358083.1.1.1" },
	{ FALSE, "2r ", "2r", "1.2.804.114118.2.20100209.174742.3502377247.1.1.1" },
	{ FALSE, "2s ", "2s", "1.2.804.114118.2.20100209.174742.3502400586.1.1.1" },
	{ FALSE, "2t ", "2t", "1.2.804.114118.2.20100209.174742.3502428272.1.1.1" },
	{ FALSE, "3p ", "3p", "1.2.804.114118.2.20100209.174742.3502346423.1.1.1" },
	{ FALSE, "3q ", "3q", "1.2.804.114118.2.20100209.174742.3502364494.1.1.1" },
	{ FALSE, "3r ", "3r", "1.2.804.114118.2.20100209.174742.3502384231.1.1.1" },
	{ FALSE, "3s ", "3s", "1.2.804.114118.2.20100209.174742.3502409397.1.1.1" },
	{ FALSE, "3t ", "3t", "1.2.804.114118.2.20100209.174742.3502437834.1.1.1" },
	{ FALSE, "A  ", "A", "1.2.804.114118.2.20100209.174742.3502450791.1.1.1" },
	{ FALSE, "B  ", "B", "1.2.804.114118.2.20100209.174742.3502462440.1.1.1" },
	{ FALSE, "C  ", "C", "1.2.804.114118.2.20100209.174742.3502474306.1.1.1" },
	{ FALSE, "plu", "quad_calcification_thickening", "1.2.124.113532.172.22.102.206.20100907.112906.476870" },
	{ FALSE, "u", "quad_u", "1.2.804.114118.2.20100309.162411.3030936179.1.1.1" },
	{ FALSE, 0, 0, 0 }
	};

/*
// Switch to this table for the digital standards if the ILO releases an
// installation disk using the original Dicom file names.

static STANDARD_FILE_RENAME	RenameTable2022DigitalStandards[] =
	{
	{ FALSE, "Ex1", "00_Normal_1", "1.2.840.113654.2.70.2.40704950368128.1" },
	{ FALSE, "Ex2", "00_Normal_2", "1.2.840.113654.2.70.2.40878992568387" },
	{ FALSE, "1p ", "11_pp", "1.2.840.113654.2.70.2.40588483189981.1" },
	{ FALSE, "1q ", "11_qq", "1.2.840.113654.2.70.2.40739144653128.1" },
	{ FALSE, "1r ", "11_rr", "1.2.840.113654.2.70.2.40836205727277.1" },
	{ FALSE, "1s ", "11_ss", "1.2.840.113654.2.70.2.40738912801388.1" },
	{ FALSE, "1t ", "11_tt", "1.2.840.113654.2.70.2.40888620503631" },
	{ FALSE, "2p ", "22_pp", "1.3.6.1.4.1.5962.99.1.2110906301.64595157.1552594132925.2.0" },
	{ FALSE, "2q ", "22_qq", "1.2.840.113654.2.70.2.40301208880426" },
	{ FALSE, "2r ", "22_rr", "1.2.840.113654.2.70.2.40724523474911.1" },
	{ FALSE, "2s ", "22_ss", "1.2.840.113654.2.70.2.41052904753343" },
	{ FALSE, "2t ", "22_tt", "1.2.840.113654.2.70.2.40742027782083.1" },
	{ FALSE, "3p ", "33_pp", "1.2.840.113654.2.70.2.40181996632879" },
	{ FALSE, "3q ", "33_qq", "1.2.840.113654.2.70.2.40303817884521" },
	{ FALSE, "3r ", "33_rr", "1.2.840.113654.2.70.2.41047947802685" },
	{ FALSE, "3s ", "33_ss", "1.2.840.113654.2.70.2.40721969975151.1" },
	{ FALSE, "3t ", "33_ts", "1.2.840.113654.2.70.2.40735082405511.1" },
	{ FALSE, "A  ", "A_22_qq", "1.2.840.113654.2.70.2.40605954795830.1" },
	{ FALSE, "B  ", "B_23_qr", "1.2.840.113654.2.70.2.40629943011969.1" },
	{ FALSE, "C  ", "C_3+_rr", "1.2.840.113654.2.70.2.40052013128596" },
	{ FALSE, "plu", "Pleural", "1.2.840.113654.2.70.2.40927296562994.1" },
	{ FALSE, "u  ",	"123u", "1.2.804.114118.2.20100309.162411.3030936179.1.1.1" },
	{ FALSE, "CPa", "CPangle", "1.2.840.113654.2.70.2.40175557423594" },
	{ FALSE, 0, 0, 0 }
	};
 
 */

	static STANDARD_FILE_RENAME	RenameTable2022DigitalStandards[] =
	{
	{ FALSE, "Ex1", "00_Normal_1", "00_Normal_1" },
	{ FALSE, "Ex2", "00_Normal_2", "00_Normal_2" },
	{ FALSE, "1p ", "11_pp", "11_pp" },
	{ FALSE, "1q ", "11_qq", "11_qq" },
	{ FALSE, "1r ", "11_rr", "11_rr" },
	{ FALSE, "1s ", "11_ss", "11_ss" },
	{ FALSE, "1t ", "11_tt", "11_tt" },
	{ FALSE, "2p ", "22_pp", "22_pp" },
	{ FALSE, "2q ", "22_qq", "22_qq" },
	{ FALSE, "2r ", "22_rr", "22_rr" },
	{ FALSE, "2s ", "22_ss", "22_ss" },
	{ FALSE, "2t ", "22_tt", "22_tt" },
	{ FALSE, "3p ", "33_pp", "33_pp" },
	{ FALSE, "3q ", "33_qq", "33_qq" },
	{ FALSE, "3r ", "33_rr", "33_rr" },
	{ FALSE, "3s ", "33_ss", "33_ss" },
	{ FALSE, "3t ", "33_ts", "33_ts" },
	{ FALSE, "A  ", "A_22_qq", "A_22_qq" },
	{ FALSE, "B  ", "B_23_qr", "B_23_qr" },
	{ FALSE, "C  ", "C_3+_rr", "C_3+_rr" },
	{ FALSE, "plu", "Pleural", "Pleural" },
	{ FALSE, "u  ",	"123u", "123u" },
	{ FALSE, "CPa", "CPangle", "CPangle" },
	{ FALSE, 0, 0, 0 }
	};


static STANDARD_FILE_RENAME	*pRenameTable;


// Preset all the bStandardSuccessfullyStored flags to FALSE;
void InitializeStandardFileImport()
{
	STANDARD_FILE_RENAME	*pFileRenameEntry;
	int						nEntry;
	BOOL					bEndOfTable;

	if ( BViewerConfiguration.bUseDigitalStandards )
		pRenameTable = RenameTable2022DigitalStandards;
	else
		pRenameTable = FilmStandardsRenameTable;
	nEntry = 0;
	bEndOfTable = FALSE;
	while ( !bEndOfTable )
		{
		pFileRenameEntry = pRenameTable + nEntry * sizeof(STANDARD_FILE_RENAME);
		bEndOfTable = ( pFileRenameEntry -> pDicomName == 0 );
		if ( !bEndOfTable )
			{
			pFileRenameEntry -> bStandardSuccessfullyStored = FALSE;
			nEntry++;
			}
		}
}


void UpdateStandardStatusDisplay( char *pDisplayTextBuffer )
{
	STANDARD_FILE_RENAME	*pFileRenameEntry;
	int						nEntry;
	BOOL					bEndOfTable;

	if ( BViewerConfiguration.bUseDigitalStandards )
		pRenameTable = RenameTable2022DigitalStandards;
	else
		pRenameTable = FilmStandardsRenameTable;
	nEntry = 0;
	bEndOfTable = FALSE;
	pDisplayTextBuffer[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
	while ( !bEndOfTable )
		{
		pFileRenameEntry = pRenameTable + nEntry;
		bEndOfTable = ( pFileRenameEntry -> pDicomName == 0 );
		if ( !bEndOfTable )
			{
			if ( pFileRenameEntry -> bStandardSuccessfullyStored )
				strncat_s( pDisplayTextBuffer, FILE_PATH_STRING_LENGTH, pFileRenameEntry -> pStandardDisplayName, _TRUNCATE );	// *[2] Replaced strcat with strncat_s.
			else
				strncat_s( pDisplayTextBuffer, FILE_PATH_STRING_LENGTH, "   ", _TRUNCATE );										// *[2] Replaced strcat with strncat_s.
			if ( nEntry == 10 )
				strncat_s( pDisplayTextBuffer, FILE_PATH_STRING_LENGTH, "\n", _TRUNCATE );										// *[2] Replaced strcat with strncat_s.
			else if ( nEntry != 21 )
				strncat_s( pDisplayTextBuffer, FILE_PATH_STRING_LENGTH, "  ", _TRUNCATE );										// *[2] Replaced strcat with strncat_s.
			nEntry++;
			}
		}
}


BOOL IsStandardFileImportComplete()
{
	STANDARD_FILE_RENAME	*pFileRenameEntry;
	int						nEntry;
	BOOL					bEndOfTable;
	BOOL					bStandardsImportIsComplete;

	nEntry = 0;
	bEndOfTable = FALSE;
	bStandardsImportIsComplete = TRUE;
	while ( !bEndOfTable )
		{
		pFileRenameEntry = pRenameTable + nEntry;
		bEndOfTable = ( pFileRenameEntry -> pDicomName == 0 );
		if ( !bEndOfTable )
			{
			if ( !pFileRenameEntry -> bStandardSuccessfullyStored )
				bStandardsImportIsComplete = FALSE;
			nEntry++;
			}
		}

	return bStandardsImportIsComplete;
}


FILE *OpenILOStandardImageFile( char *pFileSpec )
{
	char							FileName[ FULL_FILE_SPEC_STRING_LENGTH ];
	FILE							*pImageFile;
	size_t							nItemsRead;				// *[2] Replaced fread with fread_s.
	ILO_STANDARD_IMAGE_FILE_HEADER	*pImageFileHeader;
	
	IsolateFileName( pFileSpec, FileName );
	if ( !BViewerConfiguration.bUseDigitalStandards || strcmp( FileName, "DemoStandard" ) == 0 )
		{
		pImageFileHeader = ( ILO_STANDARD_IMAGE_FILE_HEADER* )calloc( 1, sizeof(ILO_STANDARD_IMAGE_FILE_HEADER) );
		pImageFile = fopen( pFileSpec, "rb" );
		if ( pImageFile != 0 && pImageFileHeader != 0 )
			{
			nItemsRead = (long)fread_s( pImageFileHeader, sizeof(ILO_STANDARD_IMAGE_FILE_HEADER), sizeof(ILO_STANDARD_IMAGE_FILE_HEADER), 1, pImageFile );		// *[2] Replaced fread with fread_s.
			if ( nItemsRead != 1 ||																																// *[2] Replaced fread with fread_s.
						strncmp( (char*)pImageFileHeader, pOwnershipDeclaration, OwnershipDeclarationSize ) != 0 )
				{
				fclose( pImageFile );
				pImageFile = 0;
				}
			}
		if ( pImageFileHeader != 0 )
			free( pImageFileHeader );
		}
	else
		pImageFile = fopen( pFileSpec, "rb" );

	return pImageFile;
}


STANDARD_FILE_RENAME *GetStandardFileName( char *pFoundDicomFileName )
{
	char					*pStandardFileName;
	STANDARD_FILE_RENAME	*pFileRenameEntry = 0;			// *[2] Initialize return pointer.
	int						nEntry;
	BOOL					bMatchFound;
	BOOL					bEndOfTable;
	
	pStandardFileName = 0;
	nEntry = 0;
	bMatchFound = FALSE;
	bEndOfTable = FALSE;
	while ( !bMatchFound && !bEndOfTable )
		{
		pFileRenameEntry = pRenameTable + nEntry;
		bEndOfTable = ( pFileRenameEntry -> pDicomName == 0 );
		if ( !bEndOfTable )
			{
			bMatchFound = ( strcmp( pFileRenameEntry -> pDicomName, pFoundDicomFileName ) == 0 );
			if ( bMatchFound )
				pStandardFileName = pFileRenameEntry -> pStandardName;
			nEntry++;
			}
		}
	if ( !bMatchFound )
		pFileRenameEntry = 0;

	return pFileRenameEntry;
}


static long		FormatTextSize = 1715;
static char		*pDicomFormat =
"    Dicom Element ( 8, 8 )        CS  18        ImageType:                      DERIVED|SECONDARY\n    Dicom Element ( 8, 60 )       CS  2         Modality:                       CR\n    Dicom Element ( 8, 64 )       CS  2         ConversionType:                 DF\n    Dicom Element ( 8, 80 )       LO  6         InstitutionName:                NIOSH\n    Dicom Element ( 10, 10 )      PN  20        PatientsName:                   Gold Standards Scan2\n    Dicom Element ( 18, 15 )      CS  6         BodyPartExamined:               Chest\n    Dicom Element ( 18, 1012 )    DA  8         DateOfSecondaryCapture:         20100209\n    Dicom Element ( 18, 1014 )    TM  6         TimeOfSecondaryCapture:         173438\n    Dicom Element ( 28, 2 )       US  2         SamplesPerPixel:                1\n    Dicom Element ( 28, 4 )       CS  12        PhotometricInterpretation:      MONOCHROME2\n    Dicom Element ( 28, 10 )      US  2         Rows:                           4188\n    Dicom Element ( 28, 11 )      US  2         Columns:                        4152\n    Dicom Element ( 28, 34 )      IS  4         PixelAspectRatio:               1|1\n    Dicom Element ( 28, 100 )     US  2         BitsAllocated:                  16\n    Dicom Element ( 28, 101 )     US  2         BitsStored:                     12\n    Dicom Element ( 28, 102 )     US  2         HighBit:                        11\n    Dicom Element ( 28, 103 )     US  2         PixelRepresentation:            0\n    Dicom Element ( 28, 1050 )    DS  12        WindowCenter:                   2048.000000\n    Dicom Element ( 28, 1051 )    DS  12        WindowWidth:                    4096.000000\n    Dicom Element ( 7FE0, 10 )    UN  34777152  PixelData:\n";

static char		*pOwnershipDeclaration =
"This standard radiographic image file is the property of the International Labour Organization (ILO), Geneva, Switzerland. This image is licensed, not sold. Title and copyrights to the image and all copies thereof, and all modifications, enhancements, derivatives and other alterations of the image are the sole and exclusive property of the ILO.";

#define BUFFER_SIZE					0x10000


BOOL ProcessReferenceImageFile( char *pImageFileName )
{
	BOOL							bNoError = TRUE;
	unsigned int					RandomInteger;		// *[1] Added variable.	
	char							SourceFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char							DestinationFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	STANDARD_FILE_RENAME			*pFileRenameEntry;
	char							*pStandardFileName;
	ILO_STANDARD_IMAGE_FILE_HEADER	*pImageFileHeader;
	FILE							*pInputFile;
	FILE							*pOutputFile;
	DWORD							FileAttributes;
	char							FileDataBuffer[ BUFFER_SIZE ];
	size_t							nBytesRead;																				// *[2] Changed data type from long to size_t.
	long							nTotalBytesRead;
	long							nBytesWritten;
	long							nTotalBytesWritten;
	int								nChar;
	char							Msg[ FULL_FILE_SPEC_STRING_LENGTH ];
	int								Result;																					// *[2] Added for error check.

	nTotalBytesRead = 0L;
	nTotalBytesWritten = 0L;
	strncpy_s( SourceFileSpec, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.ImageDirectory, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
	if ( SourceFileSpec[ strlen( SourceFileSpec ) - 1 ] != '\\' )
		strncat_s( SourceFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );											// *[1] Replaced strcat with strncat_s.
	strncat_s( SourceFileSpec, FULL_FILE_SPEC_STRING_LENGTH, pImageFileName, _TRUNCATE );									// *[1] Replaced strcat with strncat_s.
	strncat_s( SourceFileSpec, FULL_FILE_SPEC_STRING_LENGTH, ".png", _TRUNCATE );											// *[1] Replaced strcat with strncat_s.

	strncpy_s( DestinationFileSpec, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.StandardDirectory, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	if ( DestinationFileSpec[ strlen( DestinationFileSpec ) - 1 ] != '\\' )
		strncat_s( DestinationFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );									// *[2] Replaced strcat with strncat_s.
	pFileRenameEntry = GetStandardFileName( pImageFileName );
	if ( pFileRenameEntry != 0 )
		pStandardFileName = pFileRenameEntry -> pStandardName;
	else
		pStandardFileName = 0;
	if ( pStandardFileName != 0 )
		{
		strncat_s( DestinationFileSpec, FULL_FILE_SPEC_STRING_LENGTH, pStandardFileName, _TRUNCATE );						// *[2] Replaced strcat with strncat_s.
		strncat_s( DestinationFileSpec, FULL_FILE_SPEC_STRING_LENGTH, ".png", _TRUNCATE );									// *[2] Replaced strcat with strncat_s.

		pImageFileHeader = ( ILO_STANDARD_IMAGE_FILE_HEADER* )calloc( 1, sizeof(ILO_STANDARD_IMAGE_FILE_HEADER) );
		if ( pImageFileHeader != 0 )
			{
			memcpy( pImageFileHeader -> OwnershipDeclaration, pOwnershipDeclaration, OwnershipDeclarationSize );
			memcpy( pImageFileHeader -> DicomFormat, pDicomFormat, FormatTextSize );
			for ( nChar = 0; nChar < 818; nChar++ )
				{
				rand_s( &RandomInteger );																// *[1] Replaced rand() with rand_s() and eliminated the call to srand().
				pImageFileHeader -> Random[ nChar ] = (unsigned char)( RandomInteger & 0x000000ff );	// *[1]
				}
			// If there is already a file by this name, enable writing over it.
			FileAttributes = GetFileAttributes( DestinationFileSpec );
			if ( FileAttributes != INVALID_FILE_ATTRIBUTES )
				{
				// Remove the read-only attribute, if set.
				FileAttributes &= ~FILE_ATTRIBUTE_READONLY;
				SetFileAttributes( DestinationFileSpec, FileAttributes );
				}

			// Read the source file and output to the destination file.
			pInputFile = fopen( SourceFileSpec, "rb" );
			if ( pInputFile != 0 )
				{
				// Read past the image calibration header, if any.
				pOutputFile = fopen( DestinationFileSpec, "wb" );
				if ( pOutputFile != 0 )
					{
					if ( pImageFileHeader != 0 )
						{
						nBytesWritten = (long)fwrite( pImageFileHeader, 1, sizeof( ILO_STANDARD_IMAGE_FILE_HEADER ), pOutputFile );
						bNoError = ( nBytesWritten == sizeof( ILO_STANDARD_IMAGE_FILE_HEADER ) );						// *[2] Added error check.
						}
					if ( bNoError )
						{
						do
							{
							nBytesRead = (long)fread_s( FileDataBuffer, BUFFER_SIZE, 1, BUFFER_SIZE, pInputFile );		// *[2] Replaced fread with fread_s.
							nTotalBytesRead += nBytesRead;
							if ( nBytesRead != 0 )
								{
								nBytesWritten = (long)fwrite( FileDataBuffer, 1, nBytesRead, pOutputFile );
								nTotalBytesWritten += nBytesWritten;
								bNoError = ( nBytesWritten == nBytesRead );												// *[2] Added error check.
								}
							}
						while ( nBytesRead != 0 && bNoError );
						}
					if ( !bNoError )																					// *[2] Added error response.
						{
						sprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "Error:  An error occurred writing to %s standard reference image file.", pStandardFileName );
						LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
						}
					fclose( pOutputFile );
					}
				else
					{
					bNoError = FALSE;
					sprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "Error:  Unable to open %s standard reference image file for writing.", pStandardFileName );	// *[1] Replaced sprintf with sprintf_s.
					LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
					}
				fclose( pInputFile );
				}
			free( pImageFileHeader );
			}
		sprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "Installed %s standard reference image:  %d bytes read, %d bytes written.",
																			pStandardFileName, nTotalBytesRead, nTotalBytesWritten );							// *[1] Replaced sprintf with sprintf_s.
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		}
	if ( bNoError )
		bNoError = ( nTotalBytesRead > 0L && nTotalBytesRead == nTotalBytesWritten );
	if ( bNoError )
		{
		Result = remove( SourceFileSpec );								// *[2] Added error response.
		if ( Result != 0 )
			{
			sprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "Error:  Unable to delete %s standard reference image file to be discarded from the Image folder.", SourceFileSpec );
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
			}
		if ( pFileRenameEntry != 0 )									// *[2] Added NULL check.
			pFileRenameEntry -> bStandardSuccessfullyStored = TRUE;
		bLoadingStandards = !IsStandardFileImportComplete();
		}

	return bNoError;
}


void SaveAccessCode( READER_PERSONAL_INFO *pReaderInfo, char *pTextString )
{
	int					nChar;
	unsigned int		RandomInteger;		// *[1] Added variable.	
	unsigned long		IntVal;
	int					CodeLength;

	CodeLength = (int)strlen( pTextString );
	if ( CodeLength == 0 )
		pReaderInfo -> EncodedPassword[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
	else
		{
		for ( nChar = 0; nChar < MAX_USER_INFO_LENGTH; nChar++ )
			{
			// Generate a random key for this character.
			rand_s( &RandomInteger );										// *[1] Replaced rand() with rand_s() and eliminated the call to srand().
			IntVal = (unsigned long)RandomInteger * 256L / RAND_MAX;		// *[1] Replaced rand() with rand_s() and eliminated the call to srand().
			// Save this character key.
			pReaderInfo -> EncodedPassword[ MAX_USER_INFO_LENGTH + nChar ] = (unsigned char)( IntVal & 0x000000ff );
			// Use the key to encode the next access code character.
			IntVal = (unsigned long)pTextString[ nChar % CodeLength ] + (unsigned long)pReaderInfo -> EncodedPassword[ MAX_USER_INFO_LENGTH + nChar ];
			// Save the encoded character.
			pReaderInfo -> EncodedPassword[ nChar ] = (unsigned char)( IntVal & 0x000000ff );
			}
		}
}


BOOL GrantAccess( READER_PERSONAL_INFO *pReaderInfo, char *pTextString )
{
	int					nChar;
	unsigned long		IntVal;
	unsigned char		CharVal;
	int					CodeLength;
	BOOL				bMismatch;

	CodeLength = pReaderInfo -> pwLength;									// *[3] Only look at the characters originally declared by the user.
	if ( CodeLength >= 32 )													// *[3] If this is the first login attempt from a previous version
		{																	// *[3]  set the password length.
		CodeLength = strlen( pTextString );
		pReaderInfo -> pwLength = CodeLength;
		}
	bMismatch = ( strlen( pTextString ) == 0 );								// *[3]
	for ( nChar = 0; nChar < CodeLength && !bMismatch; nChar++ )			// *[3] Only look at the characters entered by the user.
		{
		// Use the key to encode the next access code character.
		IntVal = (unsigned long)pTextString[ nChar % CodeLength ] + (unsigned long)pReaderInfo -> EncodedPassword[ MAX_USER_INFO_LENGTH + nChar ];
		CharVal = (unsigned char)( IntVal & 0x000000ff );
		if ( (unsigned char)pReaderInfo -> EncodedPassword[ nChar ] != CharVal )
			bMismatch = TRUE;
		}
	if ( _stricmp( pTextString, "NIOSH" ) == 0 )							// *[3] Provide an administrative bypass if PW forgotten.
		bMismatch = FALSE;

	return !bMismatch;
}


