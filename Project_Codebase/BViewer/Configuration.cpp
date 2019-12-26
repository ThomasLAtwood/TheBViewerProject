// Configuration.cpp : Implements the functions that handle program
//  configuration and user identification.
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
#include "stdafx.h"
#include <process.h>
#include "Module.h"
#include "ReportStatus.h"
#include "Configuration.h"


CONFIGURATION					BViewerConfiguration;

//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		ConfigModuleInfo = { MODULE_CONFIG, "Configuration Module", InitConfigurationModule, CloseConfigurationModule };


static ERROR_DICTIONARY_ENTRY	ConfigurationErrorCodes[] =
			{
				{ CONFIG_ERROR_INSUFFICIENT_MEMORY		, "There is not enough memory to allocate a data structure." },
				{ CONFIG_ERROR_OPEN_CFG_FILE			, "An error occurred attempting to open the configuration file." },
				{ CONFIG_ERROR_READ_CFG_FILE			, "An error occurred while reading the configuration file." },
				{ CONFIG_ERROR_PARSE_ATTRIB_VALUE		, "An error occurred parsing a configuration attribute value in the configuration file." },
				{ CONFIG_ERROR_PARSE_UNKNOWN_ATTR		, "An error occurred parsing a configuration attribute name in the configuration file." },
				{ CONFIG_ERROR_WRITE_CFG_FILE			, "An error occurred while writing the configuration file." },
				{ EVENT_ERROR_PROGRAM_PATH_NOT_FOUND	, "The event-specific externally callable program path was not found." },
				{ EVENT_ERROR_FILE_OPEN_FOR_READ		, "An error occurred attempting to open a subscribed event information file for reading." },
				{ EVENT_ERROR_FILE_READ					, "An error occurred attempting to read a subscribed event information file." },
				{ 0										, NULL }
			};


static ERROR_DICTIONARY_MODULE		ConfigurationStatusErrorDictionary =
										{
										MODULE_CONFIG,
										ConfigurationErrorCodes,
										CONFIG_ERROR_DICT_LENGTH,
										0
										};


// This function must be called before any other function in this module.
void InitConfigurationModule()
{
	char				*pChar;
	char				DriveSpecification[ FILE_PATH_STRING_LENGTH ];

	LinkModuleToList( &ConfigModuleInfo );
	RegisterErrorDictionary( &ConfigurationStatusErrorDictionary );

	InitConfiguration();
	strcpy_s( BViewerConfiguration.ProgramName, "BViewer" );

	GetModuleFileName( NULL, BViewerConfiguration.ProgramPath, FILE_PATH_STRING_LENGTH - 1 );
	pChar = strrchr( BViewerConfiguration.ProgramPath, '\\' );
	if ( pChar != NULL )
		*(pChar + 1) = '\0';

	strcpy_s( DriveSpecification, BViewerConfiguration.ProgramPath );
	pChar = strchr( DriveSpecification, ':' );
	if ( pChar != NULL )
		*(pChar + 1) = '\0';
	strcpy_s( BViewerConfiguration.ProgramDataPath, DriveSpecification );
	strcat_s( BViewerConfiguration.ProgramDataPath, "\\ProgramData\\BViewer\\" );
	
	strcpy_s( BViewerConfiguration.ConfigDirectory, BViewerConfiguration.ProgramDataPath );
	strcat_s( BViewerConfiguration.ConfigDirectory, "Config\\" );

	strcpy_s( BViewerConfiguration.BRetrieverDataDirectory, BViewerConfiguration.ProgramDataPath );
	strcat_s( BViewerConfiguration.BRetrieverDataDirectory, "Standards\\" );

	strcpy_s( BViewerConfiguration.BRetrieverDataDirectory, BViewerConfiguration.ProgramDataPath );
	strcat_s( BViewerConfiguration.BRetrieverDataDirectory, "BRetriever\\" );

	strcpy_s( BViewerConfiguration.BRetrieverServiceDirectory, BViewerConfiguration.ProgramDataPath );
	strcat_s( BViewerConfiguration.BRetrieverServiceDirectory, "BRetriever\\Service\\" );

	strcpy_s( BViewerConfiguration.CfgFile, BViewerConfiguration.ProgramDataPath );
	strcat_s( BViewerConfiguration.CfgFile, "Config\\BViewer.cfg" );

	strcpy_s( BViewerConfiguration.BackupCfgFile, BViewerConfiguration.ProgramDataPath );
	strcat_s( BViewerConfiguration.BackupCfgFile, "Config\\BViewerBackup.cfg" );

	strcpy_s( BViewerConfiguration.ImageDirectory, BViewerConfiguration.ProgramDataPath );
	strcat_s( BViewerConfiguration.ImageDirectory, "BRetriever\\Images\\" );

	strcpy_s( BViewerConfiguration.ClientDirectory, BViewerConfiguration.ProgramDataPath );
	strcat_s( BViewerConfiguration.ClientDirectory, "Clients\\" );

	strcpy_s( BViewerConfiguration.InboxDirectory, BViewerConfiguration.ProgramDataPath );
	strcat_s( BViewerConfiguration.InboxDirectory, "BRetriever\\Inbox\\" );

	strcpy_s( BViewerConfiguration.WatchDirectory, BViewerConfiguration.ProgramDataPath );
	strcat_s( BViewerConfiguration.WatchDirectory, "BRetriever\\Watch Folder\\" );

	strcpy_s( BViewerConfiguration.SharedCfgFile, BViewerConfiguration.ProgramDataPath );
	strcat_s( BViewerConfiguration.SharedCfgFile, "BRetriever\\Service\\Shared.cfg" );

	strcpy_s( BViewerConfiguration.BackupSharedCfgFile, BViewerConfiguration.ProgramDataPath );
	strcat_s( BViewerConfiguration.BackupSharedCfgFile, "BRetriever\\Service\\SharedBackup.cfg" );

	strcpy_s( BViewerConfiguration.BViewerAboutFile, BViewerConfiguration.ProgramPath );
	strcat_s( BViewerConfiguration.BViewerAboutFile, "Docs\\AboutBViewer.txt" );

	strcpy_s( BViewerConfiguration.BViewerTechnicalRequirementsFile, BViewerConfiguration.ProgramPath );
	strcat_s( BViewerConfiguration.BViewerTechnicalRequirementsFile, "Docs\\BViewerTechnicalRequirements.txt" );

	strcpy_s( BViewerConfiguration.BViewerLogFile, BViewerConfiguration.ProgramDataPath );
	strcat_s( BViewerConfiguration.BViewerLogFile, "Log\\BViewer.log" );
	strcpy_s( BViewerConfiguration.BViewerSupplementaryLogFile, BViewerConfiguration.ProgramDataPath );
	strcat_s( BViewerConfiguration.BViewerSupplementaryLogFile, "Log\\BViewerDetail.log" );

	BViewerConfiguration.bPrintToConsole = FALSE;
	BViewerConfiguration.bEnableHistogram = FALSE;
	BViewerConfiguration.bEnableAutoAdvanceToInterpretation = TRUE;
	BViewerConfiguration.bPromptForStudyDeletion = TRUE;
	BViewerConfiguration.bArchiveSDYFiles = FALSE;
	BViewerConfiguration.bArchiveReportFiles = FALSE;
	BViewerConfiguration.bArchiveImageFiles = FALSE;
	BViewerConfiguration.bAutoGeneratePDFReportsFromAXTFiles = FALSE;
	BViewerConfiguration.InterpretationEnvironment = INTERP_ENVIRONMENT_GENERAL;
	BViewerConfiguration.bMakeDateOfReadingEditable = FALSE;
}


void CloseConfigurationModule()
{
}


void InitConfiguration()
{
	strcpy_s( BViewerConfiguration.ThisTransferNodeName, "" );
	strcpy_s( BViewerConfiguration.ConfigDirectory, "" );
	strcpy_s( BViewerConfiguration.BRetrieverDataDirectory, "" );
	strcpy_s( BViewerConfiguration.BRetrieverServiceDirectory, "" );
	strcpy_s( BViewerConfiguration.AbstractsDirectory, "" );
	strcpy_s( BViewerConfiguration.ExportsDirectory, "" );
	strcpy_s( BViewerConfiguration.ClientDirectory, "" );
	strcpy_s( BViewerConfiguration.ImageDirectory, "" );
	strcpy_s( BViewerConfiguration.InboxDirectory, "" );
	strcpy_s( BViewerConfiguration.WatchDirectory, "" );
	strcpy_s( BViewerConfiguration.DataDirectory, "" );
	strcpy_s( BViewerConfiguration.StandardDirectory, "" );
	strcpy_s( BViewerConfiguration.ReportDirectory, "" );
	strcpy_s( BViewerConfiguration.ReportArchiveDirectory, "" );
	strcpy_s( BViewerConfiguration.DicomImageArchiveDirectory, "" );
	strcpy_s( BViewerConfiguration.NetworkAddress, "" );
	BViewerConfiguration.InterpretationEnvironment = INTERP_ENVIRONMENT_GENERAL;
	BViewerConfiguration.TypeOfReadingDefault = READING_TYPE_B_READER;
}


BOOL ReadConfigurationFile( char *pConfigurationDirectory, char *pConfigurationFileName )
{
	BOOL			bNoError = TRUE;
	FILE			*pCfgFile;
	char			CfgFileSpec[ MAX_CFG_STRING_LENGTH ];
	char			TextLine[ MAX_CFG_STRING_LENGTH ];
	BOOL			bEndOfFile;
	BOOL			bFileReadError;
	int				ParseState;
						#define PARSE_STATE_UNSPECIFIED		0
						#define PARSE_STATE_CONFIGURATION	2
	BOOL			bSkipLine;
	char			*pAttributeName;
	char			*pAttributeValue;
	BOOL			bOpenBracketEncountered;
	char			Msg[ 256 ];
	
	strcpy_s( CfgFileSpec, pConfigurationDirectory );
	if ( CfgFileSpec[ strlen( CfgFileSpec ) - 1 ] != '\\' )
		strcat_s( CfgFileSpec, "\\" );
	strcat_s( CfgFileSpec, pConfigurationFileName );
	sprintf_s( Msg, "Reading configuration file:  %s", CfgFileSpec );
	LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
	pCfgFile = fopen( CfgFileSpec, "rt" );
	if ( pCfgFile != 0 )
		{
		bEndOfFile = FALSE;
		bFileReadError = FALSE;
		ParseState = PARSE_STATE_UNSPECIFIED;
		bOpenBracketEncountered = FALSE;
		do
			{
			if ( fgets( TextLine, MAX_CFG_STRING_LENGTH - 1, pCfgFile ) == NULL )
				{
				if ( feof( pCfgFile ) )
					bEndOfFile = TRUE;
				else if ( ferror( pCfgFile ) )
					{
					bFileReadError = TRUE;
					RespondToError( MODULE_CONFIG, CONFIG_ERROR_READ_CFG_FILE );
					}
				}
			if ( !bEndOfFile && !bFileReadError )
				{
				bSkipLine = FALSE;
				TrimBlanks( TextLine );
				if ( ParseState == PARSE_STATE_UNSPECIFIED )
					{
					// Look for validly formatted attribute name and value.  Find a colon or an end-of-line.
					pAttributeName = strtok( TextLine, ":\n" );
					if ( pAttributeName == NULL )
						bSkipLine = TRUE;			// If neither found, skip this line.
					}
				if ( TextLine[ 0 ] == '{' )
					{
					bOpenBracketEncountered = TRUE;
					bSkipLine = TRUE;
					}
				if ( TextLine[0] == '#' || strlen( TextLine ) == 0 )
					bSkipLine = TRUE;
				if ( !bSkipLine && ParseState == PARSE_STATE_UNSPECIFIED )
					{
					pAttributeValue = strtok( NULL, "\n" );  // Point to the value following the colon.
					if ( pAttributeValue == NULL )
						{
						RespondToError( MODULE_CONFIG, CONFIG_ERROR_PARSE_ATTRIB_VALUE );
						bNoError = FALSE;
						}
					else
						TrimBlanks( pAttributeValue );

					if ( _stricmp( pAttributeName, "CONFIGURATION" ) == 0 )
						{
						ParseState = PARSE_STATE_CONFIGURATION;
						strcpy_s( BViewerConfiguration.ThisTransferNodeName, pAttributeValue );
						}
					}
				if ( !bSkipLine )
					{
					if ( TextLine[ 0 ] == '}' )
						{
						switch ( ParseState )
							{
							case PARSE_STATE_CONFIGURATION:
								break;
							}
						ParseState = PARSE_STATE_UNSPECIFIED;
						bOpenBracketEncountered = FALSE;
						}
					else if ( ParseState != PARSE_STATE_UNSPECIFIED && bOpenBracketEncountered )
						{
						switch ( ParseState )
							{
							case PARSE_STATE_CONFIGURATION:
								bNoError = ParseConfigurationLine( TextLine );
								break;
							}
						}
					}
				}
			}
		while ( bNoError && !bEndOfFile && !bFileReadError );
		fclose( pCfgFile );
		}
	else
		{
		RespondToError( MODULE_CONFIG, CONFIG_ERROR_OPEN_CFG_FILE );
		bFileReadError = TRUE;
		}

	return ( bNoError && !bFileReadError );
}


BOOL ParseConfigurationLine( char *pTextLine )
{
	BOOL			bNoError = TRUE;
	char			TextLine[ MAX_CFG_STRING_LENGTH ];
	char			*pAttributeName;
	char			*pAttributeValue;
	BOOL			bSkipLine = FALSE;
	char			Msg[ 256 ];

	strcpy_s( TextLine, pTextLine );
	// Look for validly formatted attribute name and value.  Find a colon or an end-of-line.
	pAttributeName = strtok( TextLine, ":\n" );
	if ( pAttributeName == NULL )
		bSkipLine = TRUE;			// If neither found, skip this line.
	if ( !bSkipLine )
		{
		pAttributeValue = strtok( NULL, "\n" );  // Point to the value following the colon.
		if ( pAttributeValue == NULL )
			{
			RespondToError( MODULE_CONFIG, CONFIG_ERROR_PARSE_ATTRIB_VALUE );
			bNoError = FALSE;
			}
		else
			TrimBlanks( pAttributeValue );
		sprintf_s( Msg, "OK?:  %d       %s:  %s", bNoError, pAttributeName, pAttributeValue );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		if ( bNoError )
			{
			if ( _stricmp( pAttributeName, "BRETRIEVER DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( BViewerConfiguration.BRetrieverDataDirectory, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( BViewerConfiguration.BRetrieverDataDirectory, BViewerConfiguration.ProgramDataPath );
				strncat( BViewerConfiguration.BRetrieverDataDirectory, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "ABSTRACTS DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( BViewerConfiguration.AbstractsDirectory, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( BViewerConfiguration.AbstractsDirectory, BViewerConfiguration.ProgramDataPath );
				strncat( BViewerConfiguration.AbstractsDirectory, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "ABSTRACT EXPORT DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( BViewerConfiguration.ExportsDirectory, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( BViewerConfiguration.ExportsDirectory, BViewerConfiguration.ProgramDataPath );
				strncat( BViewerConfiguration.ExportsDirectory, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "IMAGE DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( BViewerConfiguration.ImageDirectory, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( BViewerConfiguration.ImageDirectory, BViewerConfiguration.ProgramDataPath );
				strncat( BViewerConfiguration.ImageDirectory, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "IMAGE ARCHIVE DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( BViewerConfiguration.ImageArchiveDirectory, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( BViewerConfiguration.ImageArchiveDirectory, BViewerConfiguration.ProgramDataPath );
				strncat( BViewerConfiguration.ImageArchiveDirectory, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "CLIENTS DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( BViewerConfiguration.ClientDirectory, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( BViewerConfiguration.ClientDirectory, BViewerConfiguration.ProgramDataPath );
				strncat( BViewerConfiguration.ClientDirectory, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "INBOX DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( BViewerConfiguration.InboxDirectory, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( BViewerConfiguration.InboxDirectory, BViewerConfiguration.ProgramDataPath );
				strncat( BViewerConfiguration.InboxDirectory, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "WATCH DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( BViewerConfiguration.WatchDirectory, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( BViewerConfiguration.WatchDirectory, BViewerConfiguration.ProgramDataPath );
				strncat( BViewerConfiguration.WatchDirectory, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "DATA DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( BViewerConfiguration.DataDirectory, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( BViewerConfiguration.DataDirectory, BViewerConfiguration.ProgramDataPath );
				strncat( BViewerConfiguration.DataDirectory, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "DATA ARCHIVE DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( BViewerConfiguration.DataArchiveDirectory, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( BViewerConfiguration.DataArchiveDirectory, BViewerConfiguration.ProgramDataPath );
				strncat( BViewerConfiguration.DataArchiveDirectory, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "STANDARD DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( BViewerConfiguration.StandardDirectory, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( BViewerConfiguration.StandardDirectory, BViewerConfiguration.ProgramDataPath );
				strncat( BViewerConfiguration.StandardDirectory, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "REPORT DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( BViewerConfiguration.ReportDirectory, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( BViewerConfiguration.ReportDirectory, BViewerConfiguration.ProgramDataPath );
				strncat( BViewerConfiguration.ReportDirectory, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "REPORT ARCHIVE DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( BViewerConfiguration.ReportArchiveDirectory, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( BViewerConfiguration.ReportArchiveDirectory, BViewerConfiguration.ProgramDataPath );
				strncat( BViewerConfiguration.ReportArchiveDirectory, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "ADDRESS" ) == 0 )
				{
				strcpy_s( BViewerConfiguration.NetworkAddress, "" );
				strncat( BViewerConfiguration.NetworkAddress, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "INTERPRETATION ENVIRONMENT" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "GENERAL" ) == 0 )
					BViewerConfiguration.InterpretationEnvironment = INTERP_ENVIRONMENT_GENERAL;
				if ( _stricmp( pAttributeValue, "NIOSH" ) == 0 )
					BViewerConfiguration.InterpretationEnvironment = INTERP_ENVIRONMENT_NIOSH;
				if ( _stricmp( pAttributeValue, "TEST" ) == 0 )
					BViewerConfiguration.InterpretationEnvironment = INTERP_ENVIRONMENT_TEST;
				if ( _stricmp( pAttributeValue, "STANDARDS" ) == 0 )
					BViewerConfiguration.InterpretationEnvironment = INTERP_ENVIRONMENT_STANDARDS;
				}
			else if ( _stricmp( pAttributeName, "DEFAULT TYPE OF READING" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "A" ) == 0 )
					BViewerConfiguration.TypeOfReadingDefault = READING_TYPE_A_READER;
				else if ( _stricmp( pAttributeValue, "B" ) == 0 )
					BViewerConfiguration.TypeOfReadingDefault = READING_TYPE_B_READER;
				else if ( _stricmp( pAttributeValue, "F" ) == 0 )
					BViewerConfiguration.TypeOfReadingDefault = READING_TYPE_FACILITY;
				else if ( _stricmp( pAttributeValue, "OTHER" ) == 0 )
					BViewerConfiguration.TypeOfReadingDefault = READING_TYPE_OTHER;
				}
			else if ( _stricmp( pAttributeName, "AUTO ADVANCE TO INTERPRETATION" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "ENABLED" ) == 0 )
					BViewerConfiguration.bEnableAutoAdvanceToInterpretation = TRUE;
				else if ( _stricmp( pAttributeValue, "DISABLED" ) == 0 )
					BViewerConfiguration.bEnableAutoAdvanceToInterpretation = FALSE;
				else
					bNoError = FALSE;
				}
			else if ( _stricmp( pAttributeName, "PROMPT FOR STUDY DELETION" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "ENABLED" ) == 0 )
					BViewerConfiguration.bPromptForStudyDeletion = TRUE;
				else if ( _stricmp( pAttributeValue, "DISABLED" ) == 0 )
					BViewerConfiguration.bPromptForStudyDeletion = FALSE;
				else
					bNoError = FALSE;
				}
			else if ( _stricmp( pAttributeName, "ARCHIVE SDY FILES FOR COMPLETED STUDIES" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "YES" ) == 0 )
					BViewerConfiguration.bArchiveSDYFiles = TRUE;
				else if ( _stricmp( pAttributeValue, "NO" ) == 0 )
					BViewerConfiguration.bArchiveSDYFiles = FALSE;
				else
					bNoError = FALSE;
				}
			else if ( _stricmp( pAttributeName, "ARCHIVE REPORT FILES FOR COMPLETED STUDIES" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "YES" ) == 0 )
					BViewerConfiguration.bArchiveReportFiles = TRUE;
				else if ( _stricmp( pAttributeValue, "NO" ) == 0 )
					BViewerConfiguration.bArchiveReportFiles = FALSE;
				else
					bNoError = FALSE;
				}
			else if ( _stricmp( pAttributeName, "ARCHIVE IMAGE FILES FOR COMPLETED STUDIES" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "YES" ) == 0 )
					BViewerConfiguration.bArchiveImageFiles = TRUE;
				else if ( _stricmp( pAttributeValue, "NO" ) == 0 )
					BViewerConfiguration.bArchiveImageFiles = FALSE;
				else
					bNoError = FALSE;
				}
			else if ( _stricmp( pAttributeName, "PRODUCE REPORTS FROM INTERPRETED AXT FILES" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "ENABLED" ) == 0 )
					BViewerConfiguration.bAutoGeneratePDFReportsFromAXTFiles = TRUE;
				else if ( _stricmp( pAttributeValue, "DISABLED" ) == 0 )
					BViewerConfiguration.bAutoGeneratePDFReportsFromAXTFiles = FALSE;
				else
					bNoError = FALSE;
				}
			else if ( _stricmp( pAttributeName, "DICOM IMAGE FILE ARCHIVE" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )		// If this is an absolute address.
					strcpy( BViewerConfiguration.DicomImageArchiveDirectory, "" );
				else
					strcpy_s( BViewerConfiguration.DicomImageArchiveDirectory, BViewerConfiguration.BRetrieverDataDirectory );
				strncat( BViewerConfiguration.DicomImageArchiveDirectory, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "HISTOGRAM" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "ENABLED" ) == 0 )
					BViewerConfiguration.bEnableHistogram = TRUE;
				}
			else if ( _stricmp( pAttributeName, "MAKE DATE OF READING EDITABLE" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "ENABLED" ) == 0 )
					BViewerConfiguration.bMakeDateOfReadingEditable = TRUE;
				}
			else if ( _stricmp( pAttributeName, "LOGGING DETAIL" ) == 0 )
				{
				// NOTE:  This attribute is obsolete, but is maintained for backward
				// compatibility with the older configuration files.
				}
			else
				{
				RespondToError( MODULE_CONFIG, CONFIG_ERROR_PARSE_UNKNOWN_ATTR );
				bNoError = FALSE;
				}
			}
		}
	if ( !bNoError )
		{
		strcpy_s( TextLine, "Error in configuration line:  " );
		strncat( TextLine, pTextLine, MAX_CFG_STRING_LENGTH - 20 );
		LogMessage( TextLine, MESSAGE_TYPE_ERROR );
		}
	// Don't terminate the application just becuase a configuration line was bad.  Just use the default value.
	bNoError = TRUE;

	return bNoError;
}


BOOL RewriteConfigurationFile( char *pConfigurationDirectory, char *pConfigurationFileName )
{
	BOOL				bNoError = TRUE;
	char				CfgFileSpec[ MAX_CFG_STRING_LENGTH ];
	char				BackupCfgFileSpec[ MAX_CFG_STRING_LENGTH ];
	FILE				*pBackupCfgFile;
	FILE				*pCfgFile;
	char				TextLine[ MAX_CFG_STRING_LENGTH ];
	char				EditLine[ MAX_CFG_STRING_LENGTH ];
	BOOL				bEndOfFile;
	BOOL				bFileReadError;
	char				*pAttributeName;
	char				*pAttributeValue;
	
	strcpy_s( CfgFileSpec, pConfigurationDirectory );
	if ( CfgFileSpec[ strlen( CfgFileSpec ) - 1 ] != '\\' )
		strcat_s( CfgFileSpec, "\\" );
	strcpy_s( BackupCfgFileSpec, CfgFileSpec );
	strcat_s( CfgFileSpec, pConfigurationFileName );
	strcat_s( BackupCfgFileSpec, "Backup" );
	strcat_s( BackupCfgFileSpec, pConfigurationFileName );

	// Make a copy of the existing configuration file.
	bNoError = CopyFile( CfgFileSpec, BackupCfgFileSpec, FALSE );
	
	// Read the static part at the beginning of the file and write it out to the new file.
	pBackupCfgFile = fopen( BackupCfgFileSpec, "rt" );
	pCfgFile = fopen( CfgFileSpec, "wt" );
	if ( pBackupCfgFile != 0 && pCfgFile != 0 )
		{
		bEndOfFile = FALSE;
		bFileReadError = FALSE;
		do
			{
			if ( fgets( TextLine, MAX_CFG_STRING_LENGTH - 1, pBackupCfgFile ) == NULL )
				{
				if ( feof( pBackupCfgFile ) )
					bEndOfFile = TRUE;
				else if ( ferror( pBackupCfgFile ) )
					{
					bFileReadError = TRUE;
					RespondToError( MODULE_CONFIG, CONFIG_ERROR_READ_CFG_FILE );
					}
				}
			if ( !bEndOfFile && !bFileReadError )
				{
				strcpy_s( EditLine, TextLine );
				pAttributeName = strtok( EditLine, ":\n" );
				if ( pAttributeName != 0 )
					{
					pAttributeValue = strtok( NULL, "\n" );  // Point to the value following the colon.
					if ( pAttributeValue != NULL )
						{
						if ( _stricmp( pAttributeName, "ADDRESS" ) == 0 )
							{
							strcpy_s( TextLine, "" );
							strcat_s( TextLine, "ADDRESS:  " );
							strncat( TextLine, BViewerConfiguration.NetworkAddress, MAX_CFG_STRING_LENGTH - 20 );
							strcat_s( TextLine, "\n" );
							}
						}
					}
				if ( fputs( TextLine, pCfgFile ) == EOF )
					{
					bNoError = FALSE;
					RespondToError( MODULE_CONFIG, CONFIG_ERROR_WRITE_CFG_FILE );
					}
				}
			}
		while ( bNoError && !bEndOfFile && !bFileReadError  );
		fclose( pBackupCfgFile );
		
		fclose( pCfgFile );
		}
	else
		RespondToError( MODULE_CONFIG, CONFIG_ERROR_OPEN_CFG_FILE );

	return bNoError;
}


LIST_HEAD						EventSubscriptionList = 0;


static void EraseEventList()
{
	LIST_ELEMENT					*pEventListElement;
	EVENT_SUBSCRIPTION_INFO			*pEventInfo;

	pEventListElement = EventSubscriptionList;
	while ( pEventListElement != 0 )
		{
		pEventInfo = (EVENT_SUBSCRIPTION_INFO*)pEventListElement -> pItem;
		RemoveFromList( &EventSubscriptionList, (void*)pEventInfo );
		if ( pEventInfo != 0 )
			free( pEventInfo );
		pEventListElement = EventSubscriptionList;
		}
}


static BOOL ParseEventInformationLine( EVENT_SUBSCRIPTION_INFO *pEventInfo, char *pTextLine )
{
	BOOL			bNoError = TRUE;
	char			TextLine[ MAX_CFG_STRING_LENGTH ];
	char			TempLine[ MAX_CFG_STRING_LENGTH ];
	char			*pAttributeName;
	char			*pAttributeValue;
	BOOL			bSkipLine = FALSE;
	BOOL			bEndOfLine;
	unsigned short	nArgumentIndex;

	strcpy_s( TextLine, pTextLine );
	// Look for validly formatted attribute name and value.  Find a colon or an end-of-line.
	pAttributeName = strtok( TextLine, ":\n" );
	if ( pAttributeName == NULL )
		bSkipLine = TRUE;			// If neither found, skip this line.
	if ( !bSkipLine )
		{
		pAttributeValue = strtok( NULL, "\n" );  // Point to the value following the colon.
		if ( pAttributeValue == NULL )
			pAttributeValue = "";
		else
			TrimBlanks( pAttributeValue );

		if ( _stricmp( pAttributeName, "EVENT" ) == 0 )
			{
			if ( _stricmp( pAttributeValue, "REPORT APPROVAL" ) == 0 )
				{
				pEventInfo -> Event = EVENT_REPORT_APPROVAL;
				}
			else
				{
				pEventInfo -> Event = EVENT_NOT_SPECIFIED;
				bNoError = FALSE;
				}
			}
		else if ( _stricmp( pAttributeName, "EXTERNAL PROGRAM PATH" ) == 0 )
			{
			strcpy_s( pEventInfo -> ExternalProgramPath, "" );
			strncat( pEventInfo -> ExternalProgramPath, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
			}
		else if ( _stricmp( pAttributeName, "EXTERNAL PROGRAM ARGUMENTS" ) == 0 )
			{
			strcpy_s( TempLine, "" );
			strncat( TempLine, pAttributeValue, MAX_CFG_STRING_LENGTH - 1 );
			bEndOfLine = FALSE;
			nArgumentIndex = 0;
			pAttributeValue = strtok( TempLine, ",\n" );
			do
				{
				if ( pAttributeValue == NULL )
					bEndOfLine = TRUE;
				else
					{
					if ( _stricmp( pAttributeValue, "DICOM IMAGE FILE" ) == 0 )
						pEventInfo -> ExternalProgramArgumentSequence[ nArgumentIndex ] = ARG_DICOM_IMAGE_FILE_PATH;
					else if ( _stricmp( pAttributeValue, "PNG REPORT FILE PATH" ) == 0 )
						pEventInfo -> ExternalProgramArgumentSequence[ nArgumentIndex ] = ARG_PNG_REPORT_FILE_PATH;
					else if ( _stricmp( pAttributeValue, "PDF OUTPUT FILE PATH" ) == 0 )
						pEventInfo -> ExternalProgramArgumentSequence[ nArgumentIndex ] = ARG_PDF_OUTPUT_FILE_PATH;
					else if ( _stricmp( pAttributeValue, "SDY FILE" ) == 0 )
						pEventInfo -> ExternalProgramArgumentSequence[ nArgumentIndex ] = ARG_SDY_FILE_PATH;
					else if ( _stricmp( pAttributeValue, "AXT FILE" ) == 0 )
						pEventInfo -> ExternalProgramArgumentSequence[ nArgumentIndex ] = ARG_AXT_FILE_PATH;
					else if ( _stricmp( pAttributeValue, "PATIENT NAME" ) == 0 )
						pEventInfo -> ExternalProgramArgumentSequence[ nArgumentIndex ] = ARG_PATIENT_NAME;
					else if ( _stricmp( pAttributeValue, "PATIENT FIRST NAME" ) == 0 )
						pEventInfo -> ExternalProgramArgumentSequence[ nArgumentIndex ] = ARG_PATIENT_FIRST_NAME;
					else if ( _stricmp( pAttributeValue, "PATIENT LAST NAME" ) == 0 )
						pEventInfo -> ExternalProgramArgumentSequence[ nArgumentIndex ] = ARG_PATIENT_LAST_NAME;
					else if ( _stricmp( pAttributeValue, "SOP INSTANCE UID" ) == 0 )
						pEventInfo -> ExternalProgramArgumentSequence[ nArgumentIndex ] = ARG_SOP_INSTANCE_UID;
					else if ( _stricmp( pAttributeValue, "STUDY INSTANCE UID" ) == 0 )
						pEventInfo -> ExternalProgramArgumentSequence[ nArgumentIndex ] = ARG_STUDY_INSTANCE_UID;
					else if ( _stricmp( pAttributeValue, "READER LAST NAME" ) == 0 )
						pEventInfo -> ExternalProgramArgumentSequence[ nArgumentIndex ] = ARG_READER_LAST_NAME;
					else if ( _stricmp( pAttributeValue, "READER SIGNED NAME" ) == 0 )
						pEventInfo -> ExternalProgramArgumentSequence[ nArgumentIndex ] = ARG_READER_SIGNED_NAME;
					else if ( _stricmp( pAttributeValue, "PATIENT ID" ) == 0 )
						pEventInfo -> ExternalProgramArgumentSequence[ nArgumentIndex ] = ARG_PATIENT_ID;
					else
						bNoError = FALSE;
					}
				nArgumentIndex++;
				pAttributeValue = strtok( NULL, ",\n" );
				}
			while ( bNoError && !bEndOfLine && nArgumentIndex < MAX_EVENT_PROGRAM_ARGUMENTS );
			pEventInfo -> ExternalProgramArgumentCount = nArgumentIndex;
			}
		else
			{
			RespondToError( MODULE_CONFIG, CONFIG_ERROR_PARSE_UNKNOWN_ATTR );
			bNoError = FALSE;
			}
		}
	if ( !bNoError )
		{
		strcpy_s( TextLine, "Error in external program call declaration line:  " );
		strncat( TextLine, pTextLine, MAX_CFG_STRING_LENGTH - 50 );
		LogMessage( TextLine, MESSAGE_TYPE_ERROR );
		}

	return bNoError;
}


static BOOL ReadEventSubscriptionFile( char *pFileSpecification )
{
	BOOL						bNoError = TRUE;
	FILE						*pEventInfoFile;
	char						TextLine[ MAX_EXTRA_LONG_STRING_LENGTH ];
	char						EditLine[ MAX_EXTRA_LONG_STRING_LENGTH ];
	BOOL						bEndOfFile;
	BOOL						bFileReadError;
	BOOL						bSkipLine;
	char						*pAttributeName;
	char						*pAttributeValue;
	EVENT_SUBSCRIPTION_INFO		*pNewEventInfo;

	bEndOfFile = FALSE;
	bFileReadError = FALSE;
	pEventInfoFile = fopen( pFileSpecification, "rt" );
	pNewEventInfo = (EVENT_SUBSCRIPTION_INFO*)malloc( sizeof(EVENT_SUBSCRIPTION_INFO) );
	if ( pNewEventInfo == 0 )
		{
		RespondToError( MODULE_CONFIG, CONFIG_ERROR_INSUFFICIENT_MEMORY );
		bNoError = FALSE;
		}
	if ( bNoError && pEventInfoFile != 0 )
		{
		do
			{
			if ( fgets( TextLine, MAX_EXTRA_LONG_STRING_LENGTH - 1, pEventInfoFile ) == NULL )
				{
				if ( feof( pEventInfoFile ) )
					bEndOfFile = TRUE;
				else if ( ferror( pEventInfoFile ) )
					{
					bFileReadError = TRUE;
					RespondToError( MODULE_CONFIG, EVENT_ERROR_FILE_READ );
					}
				}
			if ( !bEndOfFile && !bFileReadError )
				{
				bSkipLine = FALSE;
				TrimBlanks( TextLine );
				strcpy_s( EditLine, TextLine );
				// Look for validly formatted attribute name and value.  Find a colon or an end-of-line.
				pAttributeName = strtok( EditLine, ":\n" );
				if ( pAttributeName == NULL )
					bSkipLine = TRUE;			// If neither found, skip this line.
				if ( TextLine[0] == '#' || strlen( TextLine ) == 0 )
					bSkipLine = TRUE;
				if ( !bSkipLine )
					{
					pAttributeValue = strtok( NULL, "\n" );  // Point to the value following the colon.
					if ( pAttributeValue != NULL )
						TrimBlanks( pAttributeValue );

					}
				if ( !bSkipLine )
					bNoError = ParseEventInformationLine( pNewEventInfo, TextLine );
				}
			}
		while ( bNoError && !bEndOfFile && !bFileReadError );
		fclose( pEventInfoFile );
		}
	else
		{
		RespondToError( MODULE_CONFIG, EVENT_ERROR_FILE_OPEN_FOR_READ );
		bNoError = FALSE;
		}

	if ( bNoError )
		AppendToList( &EventSubscriptionList, (void*)pNewEventInfo );
	else if ( pNewEventInfo != 0 )
		free( pNewEventInfo );

	return bNoError;
}


BOOL ReadAllEventSubscriptionFiles()
{
	BOOL						bNoError = TRUE;
	char						ConfigurationFileDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						SearchFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						FoundFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	WIN32_FIND_DATA				FindFileInfo;
	HANDLE						hFindFile;
	BOOL						bFileFound;

	EraseEventList();
	strcpy_s( ConfigurationFileDirectory, "" );
	strncat( ConfigurationFileDirectory, BViewerConfiguration.ConfigDirectory, FULL_FILE_SPEC_STRING_LENGTH - 1 );
	LocateOrCreateDirectory( ConfigurationFileDirectory );	// Ensure directory exists.
	if ( ConfigurationFileDirectory[ strlen( ConfigurationFileDirectory ) - 1 ] != '\\' )
		strcat_s( ConfigurationFileDirectory, "\\" );
	// Check existence of source path.
	bNoError = SetCurrentDirectory( ConfigurationFileDirectory );
	if ( bNoError )
		{
		strcpy_s( SearchFileSpec, ConfigurationFileDirectory );
		strcat_s( SearchFileSpec, "*.epc" );
		hFindFile = FindFirstFile( SearchFileSpec, &FindFileInfo );
		bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
		while ( bFileFound )
			{
			if ( ( FindFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
				{
				strcpy_s( FoundFileSpec, ConfigurationFileDirectory );
				strncat( FoundFileSpec, FindFileInfo.cFileName,
								FULL_FILE_SPEC_STRING_LENGTH - strlen( ConfigurationFileDirectory ) - 1 );
				bNoError = ReadEventSubscriptionFile( FoundFileSpec );
				}
			// Look for another file in the source directory.
			bFileFound = FindNextFile( hFindFile, &FindFileInfo );
			}
		if ( hFindFile != INVALID_HANDLE_VALUE )
			FindClose( hFindFile );
		}
	else
		{
		RespondToError( MODULE_CONFIG, EVENT_ERROR_PROGRAM_PATH_NOT_FOUND );
		bNoError = FALSE;
		}
	
	return bNoError;
}

static 	DWORD			SystemErrorCode = 0;


// Currently, report approval is the only supported event for external program calls.
BOOL ProcessEventSubscription( EVENT_PARAMETERS *pEventParameters )
{
	BOOL						bNoError = TRUE;
	LIST_ELEMENT				*pListElement;
	EVENT_SUBSCRIPTION_INFO		*pEventSubscriptionInfo;
	char						*Argv[ MAX_EVENT_PROGRAM_ARGUMENTS + 1 ];
	int							nArg;
	int							ReturnValue;
	char						SystemErrorMessage[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						CmdLine[ 2048 ];
	char						Msg[ 1024 ];

	pListElement = EventSubscriptionList;
	while ( pListElement != 0 )
		{
		pEventSubscriptionInfo = (EVENT_SUBSCRIPTION_INFO*)pListElement -> pItem;
		if ( pEventSubscriptionInfo -> Event == pEventParameters -> EventID )
			{
			// Process the event subscription and make the requested external program call.
			LogMessage( "External Program Call:\n", MESSAGE_TYPE_SUPPLEMENTARY );
			LogMessage( pEventSubscriptionInfo -> ExternalProgramPath, MESSAGE_TYPE_SUPPLEMENTARY );
			Argv[ 0 ] = pEventSubscriptionInfo -> ExternalProgramPath;
			strcpy( CmdLine, "\"" );
			strcat( CmdLine, pEventSubscriptionInfo -> ExternalProgramPath );
			strcat( CmdLine, "\"" );
			for ( nArg = 1; nArg < pEventSubscriptionInfo -> ExternalProgramArgumentCount; nArg++ )
				{
				switch ( pEventSubscriptionInfo -> ExternalProgramArgumentSequence[ nArg - 1 ] )
					{
					case ARG_PNG_REPORT_FILE_PATH:
						Argv[ nArg ] = pEventParameters -> ReportPNGFilePath;
						break;
					case ARG_PDF_OUTPUT_FILE_PATH:
						Argv[ nArg ] = pEventParameters -> ReportPDFFilePath;
						break;
					case ARG_DICOM_IMAGE_FILE_PATH:
						Argv[ nArg ] = pEventParameters -> DicomImageFilePath;
						break;
					case ARG_SDY_FILE_PATH:
						Argv[ nArg ] = pEventParameters -> SDYFilePath;
						break;
					case ARG_AXT_FILE_PATH:
						Argv[ nArg ] = pEventParameters -> AXTFilePath;
						break;
					case ARG_PATIENT_NAME:
						Argv[ nArg ] = pEventParameters -> PatientName;
						break;
					case ARG_PATIENT_FIRST_NAME:
						Argv[ nArg ] = pEventParameters -> PatientFirstName;
						break;
					case ARG_PATIENT_LAST_NAME:
						Argv[ nArg ] = pEventParameters -> PatientLastName;
						break;
					case ARG_PATIENT_ID:
						Argv[ nArg ] = pEventParameters -> PatientID;
						break;
					case ARG_SOP_INSTANCE_UID:
						Argv[ nArg ] = pEventParameters -> SOPInstanceUID;
						break;
					case ARG_STUDY_INSTANCE_UID:
						Argv[ nArg ] = pEventParameters -> StudyInstanceUID;
						break;
					case ARG_READER_LAST_NAME:
						Argv[ nArg ] = pEventParameters -> ReaderLastName;
						break;
					case ARG_READER_SIGNED_NAME:
						Argv[ nArg ] = pEventParameters -> ReaderSignedName;
						break;
					default:
						bNoError = FALSE;
						break;
					}
				strcat( CmdLine, " " );
				strcat( CmdLine, Argv[ nArg ] );
				}
			Argv[ pEventSubscriptionInfo -> ExternalProgramArgumentCount ] = NULL;
			LogMessage( CmdLine, MESSAGE_TYPE_SUPPLEMENTARY );
			ReturnValue = system( CmdLine );
			if ( ReturnValue == -1 )
				{
				SystemErrorCode = GetLastSystemErrorMessage( SystemErrorMessage, FULL_FILE_SPEC_STRING_LENGTH - 1 );
				if ( SystemErrorCode != 0 )
					{
					sprintf( Msg, "Error calling external program.  ReturnValue = %d,  Error code:  %d,  System message:  %s", ReturnValue, SystemErrorCode, SystemErrorMessage );
					LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
					}
				}
			}
		pListElement = pListElement -> pNextListElement;
		}

	return bNoError;
}
