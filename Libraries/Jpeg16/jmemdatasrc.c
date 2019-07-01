/*
 * jmemdatasrc.c
 *
 * Rewritten by Tom Atwood from jdatasrc.c, which is
  * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains decompression data source routines for the case of
 * reading JPEG data from a single memory buffer.  It defines a different
 * source manager for this purpose.  Only one image is contained in the
 * buffer.
 */

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */
#include "jinclude16.h"
#include "jpeglib16.h"
#include "jerror16.h"


/* Expanded data source object for input from a memory buffer. */

typedef struct
	{
	struct jpeg_source_mgr		pub;	/* public fields */
	
	JOCTET						*pSourceBuffer;
	unsigned long				nBytesInSourceBuffer;
	JOCTET						SubstituteBufferForTermination[ 2 ];

	boolean						start_of_file;	/* have we gotten any data yet? */
	} SOURCE_MANAGER;


#define INPUT_BUF_SIZE  4096	/* choose an efficiently fread'able size */


/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */
METHODDEF(void)
init_source( j_decompress_ptr cinfo )
{
  SOURCE_MANAGER		*pSourceManager = (SOURCE_MANAGER*)cinfo -> src;

  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  pSourceManager -> start_of_file = TRUE;
}


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */
METHODDEF(boolean)
fill_input_buffer( j_decompress_ptr cinfo )
{
	SOURCE_MANAGER			*pSourceManager = (SOURCE_MANAGER*)cinfo -> src;
	size_t					nbytes;
	
	if ( pSourceManager -> start_of_file )
		nbytes = pSourceManager -> nBytesInSourceBuffer;
	else
		nbytes = 0;
	
	if ( nbytes == 0 )
		{
		if ( pSourceManager -> start_of_file )	/* Treat empty input file as fatal error */
			ERREXIT( cinfo, JERR_INPUT_EMPTY );
		WARNMS( cinfo, JWRN_JPEG_EOF );
		// Insert a fake EOI marker.
		pSourceManager -> pSourceBuffer = pSourceManager -> SubstituteBufferForTermination;
		pSourceManager -> pSourceBuffer[0] = (JOCTET)0xFF;
		pSourceManager -> pSourceBuffer[1] = (JOCTET)JPEG_EOI;
		nbytes = 2;
		}
	
	pSourceManager -> pub.next_input_byte = pSourceManager -> pSourceBuffer;
	pSourceManager -> pub.bytes_in_buffer = nbytes;
	pSourceManager -> start_of_file = FALSE;
	
	return TRUE;
}


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */
METHODDEF(void)
skip_input_data( j_decompress_ptr cinfo, long num_bytes )
{
	SOURCE_MANAGER			*pSourceManager = (SOURCE_MANAGER*)cinfo -> src;

	if ( num_bytes > 0 )
		{
		while ( num_bytes > (long)pSourceManager -> pub.bytes_in_buffer )
			{
			num_bytes -= (long) pSourceManager -> pub.bytes_in_buffer;
			fill_input_buffer( cinfo );
			/* note we assume that fill_input_buffer will never return FALSE,
			* so suspension need not be handled.
			*/
			}
		pSourceManager -> pub.next_input_byte += (size_t)num_bytes;
		pSourceManager -> pub.bytes_in_buffer -= (size_t)num_bytes;
		}
}


/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */
METHODDEF(void)
term_source( j_decompress_ptr cinfo )
{
	// No work necessary here.
}


/*
 * Prepare for input from a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */
GLOBAL(void)
jpeg_memory_src( j_decompress_ptr cinfo, char *pSourceBuffer, unsigned long nBytesInSourceBuffer )
{
	SOURCE_MANAGER			*pSourceManager;
	
	/* The source object and input buffer are made permanent so that a series
	* of JPEG images can be read from the same file by calling jpeg_stdio_src
	* only before the first one.  (If we discarded the buffer at the end of
	* one image, we'd likely lose the start of the next one.)
	* This makes it unsafe to use this manager and a different source
	* manager serially with the same JPEG object.  Caveat programmer.
	*/
	if ( cinfo -> src == NULL )
		cinfo -> src = (struct jpeg_source_mgr*)( *cinfo -> mem -> alloc_small )( (j_common_ptr)cinfo, JPOOL_PERMANENT, SIZEOF(SOURCE_MANAGER) );

	pSourceManager = (SOURCE_MANAGER*)cinfo -> src;
	pSourceManager -> pub.init_source = init_source;
	pSourceManager -> pub.fill_input_buffer = fill_input_buffer;
	pSourceManager -> pub.skip_input_data = skip_input_data;
	pSourceManager -> pub.resync_to_restart = jpeg_resync_to_restart;	// Use default method.
	pSourceManager -> pub.term_source = term_source;
	pSourceManager -> pSourceBuffer = pSourceBuffer;
	pSourceManager -> pub.bytes_in_buffer = nBytesInSourceBuffer;	// Don't force fill_input_buffer on first read.
	pSourceManager -> pub.next_input_byte = pSourceBuffer;
}