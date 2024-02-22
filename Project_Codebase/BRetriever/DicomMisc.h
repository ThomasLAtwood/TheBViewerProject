// DicomMisc.h
//
#pragma once

#define DICOMMISC_ERROR_INSUFFICIENT_MEMORY			1

#define DICOMMISC_ERROR_DICT_LENGTH					1




void			InitDicomMiscModule();

void			COPY_LONG_BIG( unsigned long LongValue, unsigned char *pBuffer );
void			COPY_SHORT_BIG( unsigned short ShortValue, unsigned char *pBuffer );
void			EXTRACT_LONG_BIG( unsigned char *pBuffer, unsigned long *pLongValue );
void			EXTRACT_SHORT_BIG( unsigned char *pBuffer, unsigned short *pShortValue );
void			TrimTrailingSpaces( char *s );