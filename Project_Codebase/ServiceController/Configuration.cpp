// Configuration.cpp : Implements the functions that handle program
//  configuration.
//
//	Written by Thomas L. Atwood
//	P.O. Box 1089
//	West Fork, Arkansas 72774
//	(479)445-4690
//	Tom_Atwood@Earthlink.net
//
//	Copyright © 2019 University of Illinois
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

// Disable function "deprecation" warnings.
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif


#include "stdafx.h"
#include <process.h>
#include "Module.h"
#include "ReportStatus.h"
#include "Configuration.h"


SERVICE_DESCRIPTOR				Configuration;

extern SERVICE_DESCRIPTOR		ServiceDescriptor;



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
				{ CONFIG_ERROR_PARSE_ALLOCATION			, "An error occurred allocating memory for reading data from the configuration file." },
				{ CONFIG_ERROR_PARSE_ATTRIB_VALUE		, "An error occurred parsing a configuration attribute value in the configuration file." },
				{ CONFIG_ERROR_PARSE_UNKNOWN_ATTR		, "An error occurred parsing a configuration attribute name in the configuration file." },
				{ CONFIG_ERROR_PARSE_NULL_OPERATION		, "The configuration led to a null operation structure in the configuration file." },
				{ CONFIG_ERROR_PARSE_TIME				, "An error occurred parsing a date/time specification in the configuration file." },
				{ CONFIG_ERROR_PARSE_FORMAT_TYPE		, "An error occurred parsing an invalid format type in the configuration file." },
				{ CONFIG_ERROR_PARSE_ENDPOINT_TYPE		, "An error occurred parsing an endpoint type in the configuration file." },
				{ CONFIG_ERROR_PARSE_ENDPOINT_NOT_FOUND	, "An operation requested an end point that was not in the end point list in the configuration file." },
				{ CONFIG_ERROR_WRITE_CFG_FILE			, "An error occurred while writing the configuration file." },
				{ CONFIG_ERROR_PARSE_ENDPOINT_USAGE		, "An error occurred parsing an endpoint usage in the configuration file." },
				{ CONFIG_ERROR_PARSE_ENDPOINT_PRODUCT	, "An error occurred parsing an endpoint product selection in the configuration file." },
				{ CONFIG_ERROR_PRODUCT_ROUTING_GRAMMAR	, "Unable to successfully parse the product routing grammar specification." },
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
	char				DriveSpecification[ FULL_FILE_SPEC_STRING_LENGTH ];

	LinkModuleToList( &ConfigModuleInfo );
	RegisterErrorDictionary( &ConfigurationStatusErrorDictionary );

	strcpy_s( Configuration.ConfigurationName, "" );
	strcpy_s( Configuration.ProgramDataPath, "" );

	strcpy( Configuration.ShortServiceName, "BRetriever" );
	strcpy( Configuration.DisplayedServiceName, "BViewer Image Retriever Service" );


	strcpy_s( Configuration.ProgramName, "ServiceController" );

	GetModuleFileNameA( NULL, Configuration.ProgramPath, FULL_FILE_SPEC_STRING_LENGTH - 1 );
	pChar = strrchr( Configuration.ProgramPath, '\\' );
	if ( pChar != NULL )
		*(pChar + 1) = '\0';

	strcpy_s( DriveSpecification, Configuration.ProgramPath );
	pChar = strchr( DriveSpecification, ':' );
	if ( pChar != NULL )
		*(pChar + 1) = '\0';

	strcpy( Configuration.ServicePathSpecification, Configuration.ProgramPath );
	strcpy( Configuration.ServiceExeFileSpecification, Configuration.ServicePathSpecification );
	SetCurrentDirectory( Configuration.ServiceExeFileSpecification );
	strcat( Configuration.ServiceExeFileSpecification, Configuration.ShortServiceName );
	strcat( Configuration.ServiceExeFileSpecification, ".exe" );

	strcpy_s( Configuration.ProgramDataPath, DriveSpecification );
	strcat_s( Configuration.ProgramDataPath, "\\ProgramData\\BViewer\\BRetriever\\" );
	
	strcpy_s( Configuration.CfgFile, Configuration.ProgramDataPath );
	strcat_s( Configuration.CfgFile, "Config\\ServiceController.cfg" );

	strcpy_s( Configuration.LogDirectory, Configuration.ProgramDataPath );
	strcat_s( Configuration.LogDirectory, "Log\\" );
	strcpy_s( Configuration.LogFileSpecification, Configuration.LogDirectory );
	strcat_s( Configuration.LogFileSpecification, "ServiceController.log" );
	strcpy_s( Configuration.SupplementaryLogFileSpecification, Configuration.LogDirectory );
	strcat_s( Configuration.SupplementaryLogFileSpecification, "ServiceControllerDetail.log" );
	Configuration.LoggingDetail = CONFIG_LOGGING_NORMAL;
	Configuration.bPrintToConsole = FALSE;

	memcpy( &ServiceDescriptor, &Configuration, sizeof(SERVICE_DESCRIPTOR) );
}


void CloseConfigurationModule()
{
}

// Note:  InitConfigurationModule() must be called to set up initial values for Configuration structure.
BOOL ReadConfigurationFile()
{
	BOOL			bNoError = TRUE;
	FILE			*pCfgFile;
	char			CfgFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char			TextLine[ FULL_FILE_SPEC_STRING_LENGTH ];
	BOOL			bEndOfFile;
	BOOL			bFileReadError;
	int				ParseState;
						#define PARSE_STATE_UNSPECIFIED		0
						#define PARSE_STATE_CONFIGURATION	1
	BOOL			bSkipLine;
	char			*pAttributeName;
	char			*pAttributeValue;
	BOOL			bOpenBracketEncountered;
	char			Msg[ 256 ];
	
	strcpy_s( CfgFileSpec, Configuration.CfgFile );
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
			if ( fgets( TextLine, FULL_FILE_SPEC_STRING_LENGTH - 1, pCfgFile ) == NULL )
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
						strcpy_s( Configuration.ConfigurationName, pAttributeValue );
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
	char			TextLine[ FULL_FILE_SPEC_STRING_LENGTH ];
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
			if ( _stricmp( pAttributeName, "SHORT SERVICE NAME" ) == 0 )
				{
				strcpy_s( Configuration.ShortServiceName, "" );
				strncat( Configuration.ShortServiceName, pAttributeValue, FULL_FILE_SPEC_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "DISPLAYED SERVICE NAME" ) == 0 )
				{
				strcpy_s( Configuration.DisplayedServiceName, "" );
				strncat( Configuration.DisplayedServiceName, pAttributeValue, FULL_FILE_SPEC_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "PROGRAM DATA FOLDER" ) == 0 )
				{
				strcpy_s( Configuration.ProgramDataPath, "" );
				strncat( Configuration.ProgramDataPath, pAttributeValue, FULL_FILE_SPEC_STRING_LENGTH - 1 );
				bNoError = LocateOrCreateDirectory( Configuration.ProgramDataPath );;
				}
			else if ( _stricmp( pAttributeName, "SERVICE FOLDER" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( Configuration.ServicePathSpecification, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( Configuration.ServicePathSpecification, Configuration.ProgramPath );
				strncat( Configuration.ServicePathSpecification, pAttributeValue, FULL_FILE_SPEC_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "SERVICE EXECUTABLE FILE SPEC" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( Configuration.ServiceExeFileSpecification, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( Configuration.ServiceExeFileSpecification, Configuration.ProgramPath );
				strncat( Configuration.ServiceExeFileSpecification, pAttributeValue, FULL_FILE_SPEC_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "PROGRAM LOG FOLDER" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( Configuration.LogDirectory, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( Configuration.LogDirectory, Configuration.ProgramDataPath );
				strncat( Configuration.LogDirectory, pAttributeValue, FULL_FILE_SPEC_STRING_LENGTH - 1 );
				bNoError = LocateOrCreateDirectory( Configuration.LogDirectory );;
				}
			else if ( _stricmp( pAttributeName, "SUMMARY LOG FILE NAME" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( Configuration.LogFileSpecification, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( Configuration.LogFileSpecification, Configuration.ProgramDataPath );
				strncat( Configuration.LogFileSpecification, pAttributeValue, FULL_FILE_SPEC_STRING_LENGTH - 1 );
				}
			else if ( _stricmp( pAttributeName, "SUPPLEMENTARY LOG FILE NAME" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )
					// If an absolute directory specification is given, load it as is.
					strcpy_s( Configuration.SupplementaryLogFileSpecification, "" );
				else
					// If a relative path is specified, base it on the (executable) program directory.
					strcpy_s( Configuration.SupplementaryLogFileSpecification, Configuration.ProgramDataPath );
				strncat( Configuration.SupplementaryLogFileSpecification, pAttributeValue, FULL_FILE_SPEC_STRING_LENGTH - 1 );
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
		strncat( TextLine, pTextLine, FULL_FILE_SPEC_STRING_LENGTH - 20 );
		LogMessage( TextLine, MESSAGE_TYPE_ERROR );
		}
	// Don't terminate the application just becuase a configuration line was bad.  Just use the default value.
	bNoError = TRUE;

	return bNoError;
}




