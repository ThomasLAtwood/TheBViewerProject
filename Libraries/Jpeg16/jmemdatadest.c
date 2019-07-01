/*
* jmemdatadest.c
 *
 * Created 2011 by Tom Atwood from jdatadst.c, which is:
 *
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains compression data destination routines for the case of
 * emitting JPEG data to a memory buffer just large enough to contain all the
 * compressed image data.
 */

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */
#include "jinclude16.h"
#include "jpeglib16.h"
#include "jerror16.h"
#include <malloc.h>


/* Expanded data destination object for output to a memory buffer. */

#define OUTPUT_BUF_SIZE  0x10000


typedef struct _LinkedTemporaryOutputBuffer
	{
	struct _LinkedTemporaryOutputBuffer		*pNextBuffer;
	size_t									StoredDataSizeInBytes;
	JOCTET									Buffer[ OUTPUT_BUF_SIZE ];
	} TEMPORARY_OUTPUT_BUFFER;


/* Expanded data destination object for memory output */
typedef struct
	{
	struct jpeg_destination_mgr	pub;	/* public fields */
	
	char						**ppDestinationBuffer;
	size_t						nTotalDataCount;
	TEMPORARY_OUTPUT_BUFFER		*pTemporaryOutputBufferList;
	TEMPORARY_OUTPUT_BUFFER		*pLatestTemporaryOutputBuffer;
	} DESTINATION_MANAGER;


static TEMPORARY_OUTPUT_BUFFER *CreateOutputBuffer()
{
	TEMPORARY_OUTPUT_BUFFER			*pNewBuffer;
	
	pNewBuffer = (TEMPORARY_OUTPUT_BUFFER*)malloc( sizeof( TEMPORARY_OUTPUT_BUFFER ) );
	if ( pNewBuffer != 0 )
		{
		pNewBuffer -> pNextBuffer = 0;
		pNewBuffer -> StoredDataSizeInBytes = 0;
		}
		
	return pNewBuffer;
}

/*
 * Initialize destination --- called by jpeg_start_compress
 * before any data is actually written.
 */

METHODDEF(void)
init_destination( j_compress_ptr cinfo )
{
	DESTINATION_MANAGER			*pDestinationManager = (DESTINATION_MANAGER*)cinfo -> dest;
	TEMPORARY_OUTPUT_BUFFER		*pFirstBuffer;

	/* Allocate the first output buffer --- it will be released when done with image */
	pFirstBuffer = CreateOutputBuffer();
	if ( pFirstBuffer != 0 )
		{
		pDestinationManager -> pTemporaryOutputBufferList = pFirstBuffer;
		pDestinationManager -> pLatestTemporaryOutputBuffer = pFirstBuffer;
		pDestinationManager -> nTotalDataCount = 0;
		
		pDestinationManager -> pub.next_output_byte = pFirstBuffer -> Buffer;
		pDestinationManager -> pub.free_in_buffer = OUTPUT_BUF_SIZE;
		}
	else
		ERREXIT( cinfo,  JERR_OUT_OF_MEMORY );
}


/*
 * Empty the output buffer --- called whenever buffer fills up.
 *
 * In typical applications, this should write the entire output buffer
 * (ignoring the current state of next_output_byte & free_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been dumped.
 *
 * In applications that need to be able to suspend compression due to output
 * overrun, a FALSE return indicates that the buffer cannot be emptied now.
 * In this situation, the compressor will return to its caller (possibly with
 * an indication that it has not accepted all the supplied scanlines).  The
 * application should resume compression after it has made more room in the
 * output buffer.  Note that there are substantial restrictions on the use of
 * suspension --- see the documentation.
 *
 * When suspending, the compressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_output_byte & free_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point will be regenerated after resumption, so do not
 * write it out when emptying the buffer externally.
 */

METHODDEF(boolean)
empty_output_buffer( j_compress_ptr cinfo )
{
	DESTINATION_MANAGER			*pDestinationManager = (DESTINATION_MANAGER*)cinfo -> dest;
	TEMPORARY_OUTPUT_BUFFER		*pNewBuffer;
	
	pNewBuffer = CreateOutputBuffer();
	pDestinationManager -> pLatestTemporaryOutputBuffer -> StoredDataSizeInBytes = OUTPUT_BUF_SIZE * SIZEOF( JOCTET );
	pDestinationManager -> nTotalDataCount += OUTPUT_BUF_SIZE;
	pDestinationManager -> pLatestTemporaryOutputBuffer -> pNextBuffer = pNewBuffer;
	pDestinationManager -> pLatestTemporaryOutputBuffer = pNewBuffer;
	
	pDestinationManager -> pub.next_output_byte = pNewBuffer -> Buffer;
	pDestinationManager -> pub.free_in_buffer = OUTPUT_BUF_SIZE;
	
	return TRUE;
}


/*
 * Terminate destination --- called by jpeg_finish_compress
 * after all data has been written.  Usually needs to flush buffer.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF(void)
term_destination( j_compress_ptr cinfo )
{
	DESTINATION_MANAGER			*pDestinationManager = (DESTINATION_MANAGER*)cinfo -> dest;
								// Data remaining in final buffer:
	size_t						datacount = OUTPUT_BUF_SIZE - pDestinationManager -> pub.free_in_buffer;
	TEMPORARY_OUTPUT_BUFFER		*pTempBuffer;
	TEMPORARY_OUTPUT_BUFFER		*pPrevTempBuffer;
	char						*pOutputCursor;
	
	if (datacount > 0)
		{
		pDestinationManager -> pLatestTemporaryOutputBuffer -> StoredDataSizeInBytes = datacount * SIZEOF( JOCTET );
		pDestinationManager -> nTotalDataCount += datacount;
		}
	// Copy the data from the temporary buffers to an output buffer large enough to hold
	// the entire compressed image.
	*pDestinationManager -> ppDestinationBuffer = (char*)malloc( pDestinationManager -> nTotalDataCount * SIZEOF( JOCTET ) );
	if ( *pDestinationManager -> ppDestinationBuffer != 0 )
		{
		pTempBuffer = pDestinationManager -> pTemporaryOutputBufferList;
		pOutputCursor = *pDestinationManager -> ppDestinationBuffer;
		while ( pTempBuffer != 0 )
			{
			memcpy( pOutputCursor, (char*)pTempBuffer -> Buffer, pTempBuffer -> StoredDataSizeInBytes );
			pOutputCursor += pTempBuffer -> StoredDataSizeInBytes;
			pDestinationManager -> pub.next_output_byte = pOutputCursor;
			
			pPrevTempBuffer = pTempBuffer;
			pTempBuffer = pTempBuffer -> pNextBuffer;
			free( pPrevTempBuffer );
			}
		}
	else
		ERREXIT( cinfo, JERR_OUT_OF_MEMORY );
}


/*
 * Prepare for output to a memory buffer.  The buffer is allocated by this
 * destination manager after it knows the final size of the output.
 * The caller is responsible for deallocating the single output image buffer.
 *
 * The destination manager creates a linked list of temporary, fixed-size
 * buffers as the image is being compressed.  On completion, the content of
 * these buffers is sequentially copied to the destination buffer and they
 * are deallocated. 
 */

GLOBAL(void)
jpeg_memory_dest( j_compress_ptr cinfo, char **ppDestinationBuffer )
{
	DESTINATION_MANAGER			*pDestinationManager;
	
	/* The destination object is made permanent so that multiple JPEG images
	* can be written to the same buffer without re-executing jpeg_memory_dest.
	* This makes it dangerous to use this manager and a different destination
	* manager serially with the same JPEG object, because their private object
	* sizes may be different.  Caveat programmer.
	*/
	if ( cinfo -> dest == NULL )
		/* first time for this JPEG object? */
		cinfo -> dest = (struct jpeg_destination_mgr*)( *cinfo -> mem -> alloc_small )(
									(j_common_ptr)cinfo, JPOOL_PERMANENT, SIZEOF(DESTINATION_MANAGER) );	
	pDestinationManager = (DESTINATION_MANAGER*)cinfo -> dest;
	pDestinationManager -> pub.init_destination = init_destination;
	pDestinationManager -> pub.empty_output_buffer = empty_output_buffer;
	pDestinationManager -> pub.term_destination = term_destination;
	pDestinationManager -> ppDestinationBuffer = ppDestinationBuffer;
}


