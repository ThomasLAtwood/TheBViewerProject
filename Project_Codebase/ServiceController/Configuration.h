// Configuration.h : Defines the functions and data structures that handle program
//  configuration.
//
//	Written by Thomas L. Atwood
//	P.O. Box 1089
//	West Fork, Arkansas 72774
//	(479)445-4690
//	Tom_Atwood@Earthlink.net
//
//	Copyright © 2020 CDC
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

#define CONFIG_ERROR_INSUFFICIENT_MEMORY			1
#define CONFIG_ERROR_OPEN_CFG_FILE					2
#define CONFIG_ERROR_READ_CFG_FILE					3
#define CONFIG_ERROR_PARSE_ALLOCATION				4
#define CONFIG_ERROR_PARSE_ATTRIB_VALUE				5
#define CONFIG_ERROR_PARSE_UNKNOWN_ATTR				6
#define CONFIG_ERROR_PARSE_NULL_OPERATION			7
#define CONFIG_ERROR_PARSE_TIME						8
#define CONFIG_ERROR_PARSE_FORMAT_TYPE				9
#define CONFIG_ERROR_PARSE_ENDPOINT_TYPE			10
#define CONFIG_ERROR_PARSE_ENDPOINT_NOT_FOUND		11
#define CONFIG_ERROR_WRITE_CFG_FILE					12
#define CONFIG_ERROR_PARSE_ENDPOINT_USAGE			13
#define CONFIG_ERROR_PARSE_ENDPOINT_PRODUCT			14
#define CONFIG_ERROR_PRODUCT_ROUTING_GRAMMAR		15
#define EVENT_ERROR_PROGRAM_PATH_NOT_FOUND			16
#define EVENT_ERROR_FILE_OPEN_FOR_READ				17
#define EVENT_ERROR_FILE_READ						18

#define CONFIG_ERROR_DICT_LENGTH					18


typedef struct
	{
	char					ConfigurationName[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					ProgramPath[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					ProgramName[ 64 ];
	char					ProgramDataPath[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					CfgFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					ShortServiceName[ 20 ];
	char					DisplayedServiceName[ 256 ];
	char					ServicePathSpecification[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					ServiceExeFileSpecification[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					LogDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					LogFileSpecification[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					SupplementaryLogFileSpecification[ FULL_FILE_SPEC_STRING_LENGTH ];
	unsigned				LoggingDetail;
								#define CONFIG_LOGGING_NORMAL			0	// Normal logging information, shows all errors.
								#define CONFIG_LOGGING_SUPPLEMENTED		1	// Add supplemental information of detailed events.
								#define CONFIG_LOGGING_DEBUG			2	// Add all remaining debugging information.
	BOOL					bPrintToConsole;
	} SERVICE_DESCRIPTOR;

	
#define	COLUMN_NAME_LENGTH		64


// Function prototypes.
//
void				InitConfigurationModule();
void				CloseConfigurationModule();
BOOL				ReadConfigurationFile();
void				InitConfiguration();
BOOL				ParseConfigurationLine( char *pTextLine );


