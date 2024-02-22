// DicomMisc.cpp
//

#include "Module.h"
#include "ReportStatus.h"
#include "DicomMisc.h"


static MODULE_INFO		DicomMiscModuleInfo = { MODULE_DICOMMISC, "Dicom Misc Module", InitDicomMiscModule, 0 };


static ERROR_DICTIONARY_ENTRY	DicomMiscErrorCodes[] =
			{
				{ DICOMMISC_ERROR_INSUFFICIENT_MEMORY		, "There is not enough memory to allocate a data structure." },
				{ 0											, NULL }
			};


static ERROR_DICTIONARY_MODULE		DicomMiscStatusErrorDictionary =
										{
										MODULE_DICOMMISC,
										DicomMiscErrorCodes,
										DICOMMISC_ERROR_DICT_LENGTH,
										0
										};

// This function must be called before any other function in this module.
void InitDicomMiscModule()
{
	LinkModuleToList( &DicomMiscModuleInfo );
	RegisterErrorDictionary( &DicomMiscStatusErrorDictionary );
}



void COPY_LONG_BIG( unsigned long LongValue, unsigned char *pBuffer )
{
	pBuffer[0] = (unsigned char)( LongValue >> 24 );
	pBuffer[1] = (unsigned char)( LongValue >> 16 );
	pBuffer[2] = (unsigned char)( LongValue >> 8 );
	pBuffer[3] = (unsigned char)( LongValue );
}

void COPY_SHORT_BIG( unsigned short ShortValue, unsigned char *pBuffer )
{
	pBuffer[0] = (unsigned char)( ShortValue >> 8 );
	pBuffer[1] = (unsigned char)( ShortValue );
}


void EXTRACT_LONG_BIG( unsigned char *pBuffer, unsigned long *pLongValue )
{			\
	*pLongValue = (unsigned long)pBuffer[3] | ( ( (unsigned long)pBuffer[2] ) << 8 ) |
				( ( (unsigned long)pBuffer[1] ) << 16 ) | ( ( (unsigned long)pBuffer[0] ) << 24 );
}


void EXTRACT_SHORT_BIG( unsigned char *pBuffer, unsigned short *pShortValue )
{
	*pShortValue = (unsigned short)pBuffer[1] | ( ( (unsigned short)pBuffer[0] ) << 8 );
}


// trim trailing spaces
//
//      s       The character string from which the trailing spaces are to be removed.
//
void TrimTrailingSpaces( char *s )
{
	char			*p;

	p = s;
	while ( *p != '\0' )
		p++;
	if (p == s)
		return;

	p--;
	while ( p >= s && *p == ' ' )
		*p-- = '\0';
}


