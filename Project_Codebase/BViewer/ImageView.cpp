// ImageView.cpp : Implementation file for the CImageView class, which
//  implements the image viewing client area for displaying subject study, standard
//  and report images.
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
#include <sys/types.h>
#include <sys/timeb.h>
#include <math.h>
#include "BViewer.h"
#include "Module.h"
#include "ReportStatus.h"
#include "GraphicsAdapter.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "Customization.h"
#include "SelectStudyPage.h"
#include "MainFrm.h"
#include "Client.h"


extern CBViewerApp				ThisBViewerApp;
extern CCustomization			*pBViewerCustomization;
extern CONFIGURATION			BViewerConfiguration;
extern CLIENT_INFO				*p_CurrentClientInfo;


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		ImageViewModuleInfo = { MODULE_IMAGEVIEW, "ImageView Module", InitImageViewModule, CloseImageViewModule };


static ERROR_DICTIONARY_ENTRY	ImageViewErrorCodes[] =
			{
				{ IMAGEVIEW_ERROR_INSUFFICIENT_MEMORY	, "An error occurred allocating a memory block for data storage." },
				{ IMAGEVIEW_ERROR_GL_INVALID_ENUM		, "An OpenGL enum argument is out of range." },
				{ IMAGEVIEW_ERROR_GL_INVALID_VALUE		, "An OpenGL numeric argument is out of range." },
				{ IMAGEVIEW_ERROR_GL_INVALID_OPERATION	, "An OpenGL operation is illegal in its current state." },
				{ IMAGEVIEW_ERROR_GL_STACK_OVERFLOW		, "An OpenGL command was aborted to prevent a stack overflow." },
				{ IMAGEVIEW_ERROR_GL_STACK_UNDERFLOW	, "An OpenGL command was aborted to prevent a stack underflow." },
				{ IMAGEVIEW_ERROR_GL_OUT_OF_MEMORY		, "An OpenGL command was aborted due to insufficient free memory." },
				{ IMAGEVIEW_ERROR_GL_TABLE_TOO_LARGE	, "The specified OpenGL table was too large." },
				{ IMAGEVIEW_ERROR_GL_UNSPECIFIED_ERROR	, "An unspecified OpenGL error occurred." },
				{ 0										, NULL }
			};

static ERROR_DICTIONARY_MODULE		ImageViewStatusErrorDictionary =
										{
										MODULE_IMAGEVIEW,
										ImageViewErrorCodes,
										IMAGEVIEW_ERROR_DICT_LENGTH,
										0
										};

// This function must be called before any other function in this module.
void InitImageViewModule()
{
	LinkModuleToList( &ImageViewModuleInfo );
	RegisterErrorDictionary( &ImageViewStatusErrorDictionary );
}


void CloseImageViewModule()
{
}

static 	DWORD			SystemErrorCode = 0;



// CImageView
CImageView::CImageView()
{
	m_hRC = 0;
	m_hDC = 0;
	m_nPixelFormat = 0;
	m_pAssignedDiagnosticImage = 0;
	m_hDefaultCursor = 0;
	m_Mouse.m_pImageView = (CWnd*)this;
	m_bTheMouseIsOverTheImage = FALSE;
	m_TimeOfLastFastPaint = 0;
	m_TimeOfLastPaint = 0;
	m_DefaultImageSize = IMAGE_VIEW_FIT_TO_SCREEN;
	m_pWndDlgBar = 0;
	m_PageNumber = 1;
	m_ImageDisplayMethod = RENDER_METHOD_NOT_SELECTED;
	m_LoadedImageTextureID = 0;
	m_ScreenImageTextureID = 0;
	m_bEnableMeasure = FALSE;
	m_pActiveMeasurementInterval = 0;
	m_pMeasuredIntervalList = 0;
	m_pImageAnnotationList = 0;
	if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_TEST ||
				BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_STANDARDS )
		m_bEnableAnnotations = FALSE;
	else
		m_bEnableAnnotations = TRUE;
	m_bRenderingCurrentlyBusy = FALSE;
	m_bImageHasBeenRendered = FALSE;
	m_OffScreenFrameBufferID = 0;
	m_ReportFormFrameBufferID = 0;

	m_g30BitColorShaderProgram = NULL;
	m_g30BitScreenShaderProgram = NULL;
	m_g10BitGrayscaleShaderProgram = NULL;
	m_gImageAnnotationShaderProgram = NULL;
	m_gImageMeasurementShaderProgram = NULL;
	m_gLineDrawingShaderProgram = NULL;
	m_gReportTextShaderProgram = NULL;
	m_gReportSignatureShaderProgram = NULL;
	m_gReportFormShaderProgram = NULL;
}


CImageView::~CImageView()
{
	DeallocateMembers();
}


void CImageView::RemoveOffScreenFrameBuffer()
{
	if ( m_OffScreenFrameBufferID != 0 )
		{
		glDeleteFramebuffers( 1, &m_OffScreenFrameBufferID );
		m_OffScreenFrameBufferID = 0;
		}
}


void CImageView::DeallocateMembers()
{
	EraseImageAnnotationInfo();
	// Deallocate the character glyph textures in the GPU.
	if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_PATIENT )
		{
		DeleteImageAnnotationFontGlyphs();
		DeleteImageMeasurementFontGlyphs();
		}
	else if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_REPORT )
		{
		glActiveTexture( TEXTURE_UNIT_REPORT_SIGNATURE );
		glDeleteTextures( 1, (GLuint*)&pBViewerCustomization -> m_ReaderInfo.pSignatureBitmap -> TextureID );
		DeleteReportImage();
		glActiveTexture( TEXTURE_UNIT_DEFAULT );
		}

	glActiveTexture( TEXTURE_UNIT_LOADED_IMAGE );
	glDeleteTextures( 1, &m_LoadedImageTextureID );
	glActiveTexture( TEXTURE_UNIT_SCREEN_IMAGE );
	glDeleteTextures( 1, &m_ScreenImageTextureID );
	glActiveTexture( TEXTURE_UNIT_DEFAULT );
	DeleteImageVertices();
	RemoveOffScreenFrameBuffer();
	// Deselect the current rendering context and delete it.
	if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_PATIENT )
		LogMessage( "Subject study image view closed.", MESSAGE_TYPE_SUPPLEMENTARY );
	else if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_STANDARD )
		LogMessage( "Standard reference image view closed.", MESSAGE_TYPE_SUPPLEMENTARY );
	else if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_REPORT )
		LogMessage( "Report image view closed.", MESSAGE_TYPE_SUPPLEMENTARY );
	wglMakeCurrent( m_hDC, NULL );
	if ( m_hRC != 0 )
		wglDeleteContext( m_hRC );
	::ReleaseDC( m_hWnd, m_hDC );
}


BEGIN_MESSAGE_MAP( CImageView, CWnd )
	//{{AFX_MSG_MAP(CImageView)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


typedef struct GLFormatAttribute
	{
	int				AttributeID;
	int				AttributeValue;
	char			AttributeName[ 40 ];
	} GL_FORMAT_ATTRIBUTE;

static GL_FORMAT_ATTRIBUTE		AttributeTable[] =
	{
		{ WGL_NUMBER_PIXEL_FORMATS_ARB, 0, "WGL_NUMBER_PIXEL_FORMATS_ARB" },
		{ WGL_DRAW_TO_WINDOW_ARB, 0, "WGL_DRAW_TO_WINDOW_ARB" },
		{ WGL_DRAW_TO_BITMAP_ARB, 0, "WGL_DRAW_TO_BITMAP_ARB" },
		{ WGL_ACCELERATION_ARB, 0, "WGL_ACCELERATION_ARB" },
		{ WGL_NEED_PALETTE_ARB, 0, "WGL_NEED_PALETTE_ARB" },
		{ WGL_SUPPORT_GDI_ARB, 0, "WGL_SUPPORT_GDI_ARB" },
		{ WGL_SUPPORT_OPENGL_ARB, 0, "WGL_SUPPORT_OPENGL_ARB" },
		{ WGL_DOUBLE_BUFFER_ARB, 0, "WGL_DOUBLE_BUFFER_ARB" },
		{ WGL_PIXEL_TYPE_ARB, 0, "WGL_PIXEL_TYPE_ARB" },
		{ WGL_COLOR_BITS_ARB, 0, "WGL_COLOR_BITS_ARB" },
		{ WGL_RED_BITS_ARB, 0, "WGL_RED_BITS_ARB" },
		{ WGL_RED_SHIFT_ARB, 0, "WGL_RED_SHIFT_ARB" },
		{ WGL_GREEN_BITS_ARB, 0, "WGL_GREEN_BITS_ARB" },
		{ WGL_GREEN_SHIFT_ARB, 0, "WGL_GREEN_SHIFT_ARB" },
		{ WGL_BLUE_BITS_ARB, 0, "WGL_BLUE_BITS_ARB" },
		{ WGL_BLUE_SHIFT_ARB, 0, "WGL_BLUE_SHIFT_ARB" },
		{ WGL_ALPHA_BITS_ARB, 0, "WGL_ALPHA_BITS_ARB" },
		{ WGL_ALPHA_SHIFT_ARB, 0, "WGL_ALPHA_SHIFT_ARB" },
		{ WGL_DEPTH_BITS_ARB, 0, "WGL_DEPTH_BITS_ARB" },
		{ WGL_STENCIL_BITS_ARB, 0, "WGL_STENCIL_BITS_ARB" },
		{ 0, 0, "" }
	};


// Select the pixel format for a given device context
void CImageView::SetDCPixelFormat( HDC hDC )
{
	BOOL				bNoError = TRUE;
	CGraphicsAdapter	*pGraphicsAdapter;
	int					nPixelFormatNumber;
	char				DisplayIDMsg[ 32 ];
	char				Msg[ 256 ];

	pGraphicsAdapter = (CGraphicsAdapter*)m_pDisplayMonitor -> m_pGraphicsAdapter;
	if ( pGraphicsAdapter != 0 )
		{
		switch ( m_ViewFunction )
			{
			case IMAGE_VIEW_FUNCTION_PATIENT:
				strcpy( DisplayIDMsg, " subject study image display" );
				break;
			case IMAGE_VIEW_FUNCTION_STANDARD:
				strcpy( DisplayIDMsg, " standard image display" );
				break;
			case IMAGE_VIEW_FUNCTION_REPORT:
				strcpy( DisplayIDMsg, " report image display" );
				break;
			}
		sprintf( Msg, "\n\nBegin initializing %s on graphics adapter %s", DisplayIDMsg, pGraphicsAdapter -> m_DisplayAdapterName );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

		m_hRC = pGraphicsAdapter -> CreateWglRenderingContext( hDC );

		if ( pGraphicsAdapter -> m_OpenGLSupportLevel == OPENGL_SUPPORT_330 )
			{
			if ( bNoError )
				{
				// Double-check that the format is really 10bpc
				int			nRedBits;
				int			nAlphaBits;
				int			WglPixelFormatAttributeName = WGL_RED_BITS_ARB;

				nPixelFormatNumber = pGraphicsAdapter -> m_Selected10BitPixelFormatNumber;
				bNoError = pGraphicsAdapter -> m_pFunctionWglGetPixelFormatAttribiv( hDC, nPixelFormatNumber, 0, 1, &WglPixelFormatAttributeName, &nRedBits );
				if (bNoError)
					{
					WglPixelFormatAttributeName = WGL_ALPHA_BITS_ARB;
					bNoError = pGraphicsAdapter -> m_pFunctionWglGetPixelFormatAttribiv( hDC, nPixelFormatNumber, 0, 1, &WglPixelFormatAttributeName, &nAlphaBits );
					}
				if (bNoError)
					{
					sprintf( Msg, "WGL pixel format chosen:  index %d     red bits: %d     alpha bits: %d", nPixelFormatNumber, nRedBits, nAlphaBits );
					LogMessage(Msg, MESSAGE_TYPE_SUPPLEMENTARY);
					}
				}
			}
		}
}


// Select the pixel format for a given device context
void CImageView::SetExportDCPixelFormat( HDC hDC )
{
	PIXELFORMATDESCRIPTOR pfd =
		{
		sizeof( PIXELFORMATDESCRIPTOR ),// Size of this structure
		1,								// Version of this structure	
		PFD_DRAW_TO_BITMAP |			// Draw to bitmap, not to window
		PFD_SUPPORT_OPENGL |			// Support OpenGL calls in window
		PFD_GENERIC_FORMAT |
		PFD_SUPPORT_GDI,
		PFD_TYPE_RGBA,					// RGBA Color mode
		24,								// Want 24 bit color 
		0,0,0,0,0,0,					// Not used to select mode
		0,0,							// Not used to select mode
		0,0,0,0,0,						// Not used to select mode
		0,								// Size of depth buffer
		0,								// Not used to select mode
		0,								// Not used to select mode
		0,	            				// Not used to select mode
		0,								// Not used to select mode
		0,0,0 };						// Not used to select mode

	// Choose a pixel format that best matches that described.
	m_nPixelFormat = ChoosePixelFormat( hDC, &pfd );

	// Set the pixel format for the device context
	SetPixelFormat( hDC, m_nPixelFormat, &pfd );
}


// Each view gets created when its associated image frame is created.  It persists through
// image viewings.  Any initialization associated with a new DiagnosticImage must be done
// later and repeated with each new image.
int CImageView::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	// Store the device context
	m_hDC = ::GetDC( m_hWnd );		

	// Select the pixel format and create an OpenGL rendering context.
	SetDCPixelFormat( m_hDC );

	wglMakeCurrent( m_hDC, m_hRC );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	EstablishImageDisplayMode();

	if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_PATIENT )
		{
		m_AnnotationCharHeight = 42.0f;
		CreateImageAnnotationFontGlyphs(  m_hDC );
		m_MeasurementCharHeight = 84.0f;
		CreateImageMeasurementFontGlyphs(  m_hDC );
		LogMessage( "Subject study image view created.", MESSAGE_TYPE_SUPPLEMENTARY );
		}
	else if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_STANDARD )
		LogMessage( "Standard reference image view created.", MESSAGE_TYPE_SUPPLEMENTARY );
	else if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_REPORT )
		{
		CreateSignatureTexture();
		LogMessage( "Report image view created.", MESSAGE_TYPE_SUPPLEMENTARY );
		}
	InitializeImageVertices();

	return 0;
}


// Set the display capabilities based on the graphics controller features.
void CImageView::EstablishImageDisplayMode()
{
	BOOL				bNoError = TRUE;
	RECT				ClientRect;
	int					ClientWidth;
	int					ClientHeight;
	CGraphicsAdapter	*pGraphicsAdapter;
	char				Msg[ 256 ];
	char				DisplayMethod[ 256 ];
	
	pGraphicsAdapter = 0;
	m_ImageDisplayMethod = RENDER_METHOD_NOT_SELECTED;	// Set default display mode.
	m_PrevImageDisplayMethod = RENDER_METHOD_NOT_SELECTED;
	if ( m_pDisplayMonitor != 0 )
		{
		pGraphicsAdapter = (CGraphicsAdapter*)m_pDisplayMonitor -> m_pGraphicsAdapter;
		if ( pGraphicsAdapter != 0 )
			{
			if ( pGraphicsAdapter -> m_OpenGLSupportLevel == 0 )
				bNoError = pGraphicsAdapter -> CheckOpenGLCapabilities( m_hDC );
			m_ImageDisplayMethod = m_pDisplayMonitor -> m_AssignedRenderingMethod;
			}
		else
			LogMessage( ">>> No graphics adapter set for this display monitor.", MESSAGE_TYPE_SUPPLEMENTARY );

		}
					
	GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;

	if ( pGraphicsAdapter != 0 )
		{
		switch ( m_ImageDisplayMethod )
			{
			case RENDER_METHOD_8BIT_COLOR:
				strcpy( DisplayMethod, "8-bit color mode" );
				break;
			case RENDER_METHOD_16BIT_PACKED_GRAYSCALE:
				strcpy( DisplayMethod, "packed 16-bit grayscale mode" );
				break;
			case RENDER_METHOD_30BIT_COLOR:
				strcpy( DisplayMethod, "30-bit color mode" );
				break;
			default:
				strcpy( DisplayMethod, "No display mode specified." );
				break;
			}
		sprintf( Msg, "  Graphics adapter %s using OpenGL version %s", pGraphicsAdapter -> m_DisplayAdapterName, pGraphicsAdapter -> m_OpenGLVersion );
		switch ( m_ViewFunction )
			{
			case IMAGE_VIEW_FUNCTION_PATIENT:
				strcat( Msg, " on subject study image display." );
				break;
			case IMAGE_VIEW_FUNCTION_STANDARD:
				strcat( Msg, " on standard image display." );
				break;
			case IMAGE_VIEW_FUNCTION_REPORT:
				strcat( Msg, " on report image display." );
				break;
			}
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		sprintf( Msg, "    Display method is %s   ( H: %d  W: %d ).", DisplayMethod, ClientHeight, ClientWidth );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

		bNoError = LoadGPUShaderPrograms();

	wglMakeCurrent( m_hDC, m_hRC );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
		pGraphicsAdapter -> Load10BitGrayscaleShaderLookupTablesAsTextures();

		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );		// Black Background
		glClear( GL_COLOR_BUFFER_BIT );				// Clear out the currently rendered image from the frame buffer.

		if ( pGraphicsAdapter -> m_OpenGLVersionNumber >= 4.3 )
			SetUpDebugContext();
		}

	InitViewport();

}


BOOL CImageView::CheckOpenGLResultAt( char *pSourceFile, int SourceLineNumber )
{
	BOOL			bNoError = TRUE;
	GLenum			GLErrorCode;
	char			MsgBuf[ 64 ];

	GLErrorCode = glGetError();
	if ( GLErrorCode != GL_NO_ERROR )
		{
		sprintf( MsgBuf, "GL Error: %s", gluErrorString( GLErrorCode ) );
		LogMessage( MsgBuf, MESSAGE_TYPE_SUPPLEMENTARY );
		bNoError = FALSE;
		switch ( GLErrorCode )
			{
			case GL_INVALID_ENUM:
				RespondToErrorAt( pSourceFile, SourceLineNumber, MODULE_IMAGEVIEW, IMAGEVIEW_ERROR_GL_INVALID_ENUM );
				break;
			case GL_INVALID_VALUE:
				RespondToErrorAt( pSourceFile, SourceLineNumber, MODULE_IMAGEVIEW, IMAGEVIEW_ERROR_GL_INVALID_VALUE );
				break;
			case GL_INVALID_OPERATION:
				RespondToErrorAt( pSourceFile, SourceLineNumber, MODULE_IMAGEVIEW, IMAGEVIEW_ERROR_GL_INVALID_OPERATION );
				break;
			case GL_STACK_OVERFLOW:
				RespondToErrorAt( pSourceFile, SourceLineNumber, MODULE_IMAGEVIEW, IMAGEVIEW_ERROR_GL_STACK_OVERFLOW );
				break;
			case GL_STACK_UNDERFLOW:
				RespondToErrorAt( pSourceFile, SourceLineNumber, MODULE_IMAGEVIEW, IMAGEVIEW_ERROR_GL_STACK_UNDERFLOW );
				break;
			case GL_OUT_OF_MEMORY:
				RespondToErrorAt( pSourceFile, SourceLineNumber, MODULE_IMAGEVIEW, IMAGEVIEW_ERROR_GL_OUT_OF_MEMORY );
				break;
			case GL_TABLE_TOO_LARGE:
				RespondToErrorAt( pSourceFile, SourceLineNumber, MODULE_IMAGEVIEW, IMAGEVIEW_ERROR_GL_TABLE_TOO_LARGE );
				break;
			default:
				RespondToErrorAt( pSourceFile, SourceLineNumber, MODULE_IMAGEVIEW, IMAGEVIEW_ERROR_GL_UNSPECIFIED_ERROR );
				break;
			}
		}

	return bNoError;
}


static void APIENTRY BViewerDebugCallbackFunction( GLenum Source, GLenum Type, GLuint Id, GLenum Severity, GLsizei Length, const GLchar *Message, const void *UserParam )
{
	char			MsgSource[ 64 ];
	char			MsgType[ 64 ];
	char			MsgSeverity[ 64 ];
	unsigned int	MsgBufferLength;
	char			*pMsgBuf;

	switch ( Source )
		{
		case GL_DEBUG_SOURCE_API:
			strcpy( MsgSource, "Source: OpenGL API" );
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			strcpy( MsgSource, "Source: WGL" );
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			strcpy( MsgSource, "Source: Shader Compiler" );
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			strcpy( MsgSource, "Source: Third Party" );
			break;
		case GL_DEBUG_SOURCE_APPLICATION:
			strcpy( MsgSource, "Source: BViewer Application" );
			break;
		case GL_DEBUG_SOURCE_OTHER:
			strcpy( MsgSource, "Source: Other" );
			break;
		}
	switch ( Type )
		{
		case GL_DEBUG_TYPE_ERROR:
			strcpy( MsgType, "Type: Error" );
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			strcpy( MsgType, "Type: Deprecated" );
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			strcpy( MsgType, "Type: Undefined Results" );
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			strcpy( MsgType, "Type: Non-optimal Performance" );
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			strcpy( MsgType, "Type: Portability" );
			break;
		case GL_DEBUG_TYPE_MARKER:
			strcpy( MsgType, "Type: Marker" );
			break;
		case GL_DEBUG_TYPE_PUSH_GROUP:
			strcpy( MsgType, "Type: Push Group" );
			break;
		case GL_DEBUG_TYPE_POP_GROUP:
			strcpy( MsgType, "Type: Pop Group" );
			break;
		case GL_DEBUG_TYPE_OTHER:
			strcpy( MsgType, "Type: Other" );
			break;
		};
	switch ( Severity )
		{
		case GL_DEBUG_SEVERITY_HIGH:
			strcpy( MsgSeverity, "Severity: High" );
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			strcpy( MsgSeverity, "Severity: Medium" );
			break;
		case GL_DEBUG_SEVERITY_LOW:
			strcpy( MsgSeverity, "Severity: Low" );
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			strcpy( MsgSeverity, "Severity: Notification" );
			break;
		};
	MsgBufferLength = 3 * 64 + Length + 32;
	pMsgBuf = (char*)malloc( MsgBufferLength );
	sprintf( pMsgBuf, "OpenGL Debug Msg:  %s  %s  %s:    %s", MsgSource, MsgType, MsgSeverity, Message );
	LogMessage( pMsgBuf, MESSAGE_TYPE_SUPPLEMENTARY );
	free( pMsgBuf );
}


static int DebugUserParam;

void CImageView::SetUpDebugContext()
{
	DebugUserParam = 0;
	// NOTE:  The following function call is not available before OpenGL version 4.3.
	glDebugMessageCallback( BViewerDebugCallbackFunction, (void*)&DebugUserParam );
//	glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
	glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE );
}


BOOL CImageView::InitViewport()
{
	BOOL			bViewportIsValid;
	RECT			ClientRect;
	int				ClientWidth;
	int				ClientHeight;
	GLsizei			glWindowWidthInPixels;
	GLsizei			glWindowHeightInPixels;

	GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;

	bViewportIsValid = ( ClientWidth > 0 && ClientHeight > 0 );
	if ( bViewportIsValid )
		{
		glWindowWidthInPixels = ClientWidth;
		glWindowHeightInPixels = ClientHeight;
		// Don't allow division by zero.
		if ( glWindowHeightInPixels == 0 )
			glWindowHeightInPixels = 1;

		// Set the viewport to be the entire window.
		glViewport( 0, 0, glWindowWidthInPixels, glWindowHeightInPixels );
		}

	return bViewportIsValid;
}


// Called from CImageFrame::OnButtonShowHistogram().
// Calculate a full histogram accurate for each pixel luminosity, along with a
// viewable histogram of 128 bins, the latter of which reflects the current
// windowing settings.
//
// 12/16/2021 NOTE:  The AdjustedPixelLuminosity was previously set by
//					AdjustedPixelLuminosity = (double)m_pWindowingTableScaled8Bit[ (unsigned int)InputPixelValue ] * (double)MaxGrayscaleValue;
// The m_pWindowingTableScaled8Bit has been removed and the raw pixel value put in its place:  Whether this works needs to be checked.
//		
BOOL CImageView::CreateGrayscaleHistogram()
{
	BOOL					bNoError = TRUE;
	HISTOGRAM_DATA			*pLuminosityHistogram;
	int						HistogramIndex;
	int						ViewableHistogramIndex;
	unsigned char			*pBuffer;
	unsigned char			*pInputReadPoint;
	long					nImagePixelsPerRow;
	long					nImageBytesPerRow;
	long					nImageRows;
	long					nRow;
	int						nBitDepth;
	unsigned long			MaxGrayscaleValue;
	unsigned short			InputPixelValue;
	double					AdjustedPixelLuminosity;
	long					nPixel;

	if ( m_pAssignedDiagnosticImage != 0 )
		{
		pLuminosityHistogram = &m_pAssignedDiagnosticImage -> m_LuminosityHistogram;
		// Create a histogram of the image's pixel grayscale luminosities.
		for ( HistogramIndex = 0; HistogramIndex < pLuminosityHistogram -> nNumberOfBins; HistogramIndex++ )
			pLuminosityHistogram -> pHistogramArray[ HistogramIndex ] = 0;
		for ( ViewableHistogramIndex = 0; ViewableHistogramIndex < 128; ViewableHistogramIndex++ )
			pLuminosityHistogram -> ViewableHistogramArray[ ViewableHistogramIndex ] = 0;
		if ( m_pAssignedDiagnosticImage -> m_pImageData != 0 )
			pBuffer = m_pAssignedDiagnosticImage -> m_pImageData;
		else
			bNoError = FALSE;
		if ( bNoError )
			{
			nImagePixelsPerRow = (long)( m_pAssignedDiagnosticImage -> m_ImageWidthInPixels );
			nImageRows = (long)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
			nBitDepth = m_pAssignedDiagnosticImage -> m_ImageBitDepth;
			if ( nBitDepth <= 8 )
				{
				nImageBytesPerRow = nImagePixelsPerRow;
				MaxGrayscaleValue = 255;
				}
			else
				{
				nImageBytesPerRow = nImagePixelsPerRow * 2;
				MaxGrayscaleValue = m_pAssignedDiagnosticImage -> m_MaxGrayscaleValue;
				}
			pLuminosityHistogram -> AverageBinValue = (double)( nImageRows * nImagePixelsPerRow );
			pLuminosityHistogram -> AverageViewableBinValue = (double)( nImageRows * nImagePixelsPerRow );
			}

		pInputReadPoint = pBuffer;
		if ( bNoError )
			{
			for ( nRow = 0; nRow < nImageRows; nRow++ )
				{
				// Process the row in the image buffer.
				for ( nPixel = 0; nPixel < nImagePixelsPerRow; nPixel++ )
					{
					if ( nBitDepth <= 8 )
						{
						InputPixelValue = (unsigned short)( ( (unsigned char*)pInputReadPoint )[ nPixel ] );
						AdjustedPixelLuminosity = (double)InputPixelValue;
						}
					else
						{
						InputPixelValue = ( (unsigned short*)pInputReadPoint )[ nPixel ];
						AdjustedPixelLuminosity = (double)( InputPixelValue / 256 );
						}
					if ( InputPixelValue > 0 && InputPixelValue < MaxGrayscaleValue )
						{
						pLuminosityHistogram -> pHistogramArray[ (int)InputPixelValue ]++;
						ViewableHistogramIndex = (int)( 128.0 * AdjustedPixelLuminosity / (double)MaxGrayscaleValue );
						if ( ViewableHistogramIndex >= 128 )
							ViewableHistogramIndex = 127;
						if ( ViewableHistogramIndex < 0 )
							ViewableHistogramIndex = 0;
						if ( ViewableHistogramIndex > 1 )
							pLuminosityHistogram -> ViewableHistogramArray[ ViewableHistogramIndex - 2 ]++;
						if ( ViewableHistogramIndex > 0 )
							pLuminosityHistogram -> ViewableHistogramArray[ ViewableHistogramIndex - 1 ]++;
						pLuminosityHistogram -> ViewableHistogramArray[ ViewableHistogramIndex ]++;
						if ( ViewableHistogramIndex < 127 )
							pLuminosityHistogram -> ViewableHistogramArray[ ViewableHistogramIndex + 1 ]++;
						if ( ViewableHistogramIndex < 126 )
							pLuminosityHistogram -> ViewableHistogramArray[ ViewableHistogramIndex + 2 ]++;
						}
					else
						{
						// Don't count the extreme values, since they can be predominantly background around
						// the edges of the actual image.
						pLuminosityHistogram -> AverageBinValue--;
						pLuminosityHistogram -> AverageViewableBinValue--;
						}
					}
				pInputReadPoint += nImageBytesPerRow;
				}
			}
		for ( ViewableHistogramIndex = 0; ViewableHistogramIndex < 128; ViewableHistogramIndex++ )
			pLuminosityHistogram -> ViewableHistogramArray[ ViewableHistogramIndex ] /= 5;
		pLuminosityHistogram -> AverageBinValue /= (double)pLuminosityHistogram -> nNumberOfBins;
		pLuminosityHistogram -> AverageViewableBinValue /= 128.0;
		}

	return bNoError;
}



double CImageView::CalculateGrayscaleHistogramMeanLuminosity()
{
	long					nImageRows;
	long					nImagePixelsPerRow;
	HISTOGRAM_DATA			*pLuminosityHistogram;
	int						HistogramIndex;
	int						nBitDepth;
	double					MeanBinValue;
	double					MeanLuminosity;


	if ( m_pAssignedDiagnosticImage != 0 )
		{
		pLuminosityHistogram = &m_pAssignedDiagnosticImage -> m_LuminosityHistogram;
		nImageRows = (long)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
		nImagePixelsPerRow = (long)( m_pAssignedDiagnosticImage -> m_ImageWidthInPixels );
		nBitDepth = m_pAssignedDiagnosticImage -> m_ImageBitDepth;
		// Average the image's pixel grayscale luminosities.
		MeanBinValue = 0.0;
		for ( HistogramIndex = 0; HistogramIndex < pLuminosityHistogram -> nNumberOfBins; HistogramIndex++ )
			MeanBinValue += (double)HistogramIndex * (double)pLuminosityHistogram -> pHistogramArray[ HistogramIndex ];
		MeanBinValue /= (double)( nImageRows * nImagePixelsPerRow );
		if ( nBitDepth <= 8 )
			MeanLuminosity = MeanBinValue * 255.0 / pLuminosityHistogram -> nNumberOfBins;
		else
		MeanLuminosity = MeanBinValue * (double)m_pAssignedDiagnosticImage -> m_MaxGrayscaleValue / pLuminosityHistogram -> nNumberOfBins;
		}

	return MeanLuminosity;
}


//	The fragment shader that paints the offscreen framebuffer outputs a 4-component color.
//	It only reads the R component of the loaded image texture, but it outputs 4-component
//	color to the screen image texture.
//
typedef struct
	{
	GLenum				OriginalImageExternalColorFormat;
	GLenum				SizedTextureInternalColorFormat;
	GLenum				SizedScreenTextureInternalColorFormat;
	GLenum				OriginalScreenImageColorFormat;
	GLenum				OriginalScreenTextureStorageFormat;
	} TEXTURE_LAYOUT;


static TEXTURE_LAYOUT	TextureSpecsFor10BitGrayscale =
	{
	GL_RED,
	GL_R16,
	GL_RGBA16,
	GL_RGBA,
	GL_FLOAT
	};


static TEXTURE_LAYOUT	TextureSpecsFor30BitColor =
	{
	GL_RGB,
	GL_RGB16,
	GL_RGBA16,
	GL_RGBA,
	GL_FLOAT
	};


static TEXTURE_LAYOUT	TextureSpecsFor24BitColor =
	{
	GL_RED,
	GL_RGB8,
	GL_RGBA8,
	GL_RGBA,
	GL_FLOAT
	};


static	TEXTURE_LAYOUT	TextureLayout;


// The input image data buffer is at m_pAssignedDiagnosticImage -> m_pImageData.
// This image is loaded into texture unit 0 as floating point pixels ready for
// processing by the GPU.
//
void CImageView::InitializeAndLoadTheImageTexture()
{
	GLsizei			ImageWidth;
	GLsizei			ImageHeight;
	float			BorderColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GLenum			OriginalPixelDataType;
	
	// Designate the texture unit to be affected by subsequent texture state operations.
	glActiveTexture( TEXTURE_UNIT_LOADED_IMAGE );

//	glDeleteTextures( 2, m_ImageTextureID );
	glDeleteTextures( 1, &m_LoadedImageTextureID );
	// Generate a texture "name" for the image and save it at m_ImageTextureID.
	// (OpenGL refers to it as a "name", but it's really just an index number.)
	glGenTextures( 1, &m_LoadedImageTextureID );

	glActiveTexture( TEXTURE_UNIT_SCREEN_IMAGE );
	glDeleteTextures( 1, &m_ScreenImageTextureID );
	glGenTextures( 1, &m_ScreenImageTextureID );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
//	glGenTextures( 2, m_ImageTextureID );

	glActiveTexture( TEXTURE_UNIT_LOADED_IMAGE );
	// Bind the image texture name to the 2-dimensional texture target.
	glBindTexture( GL_TEXTURE_2D, m_LoadedImageTextureID );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	// Set the texture wrapping for the S and T coordinates.
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	// Set the texture pixel scaling functions for when pixels are smaller or larger
	// than texture elements.
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	// Set the texture border color.
	glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BorderColor );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	// Load the image data into the texture.  Each pixel is loaded as one or more
	// floating point color components, depending upon whether 10-bit grayscale
	// or 30-bit color is being rendered.
	ImageWidth = (GLsizei)m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
	ImageHeight = (GLsizei)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
	if ( m_pAssignedDiagnosticImage -> m_ImageBitDepth == 8 )
		OriginalPixelDataType = GL_UNSIGNED_BYTE;
	else
		OriginalPixelDataType = GL_UNSIGNED_SHORT;

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );		// Set 1-byte pixel row alignment.

	if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_REPORT || m_ImageDisplayMethod == RENDER_METHOD_8BIT_COLOR )
		TextureLayout = TextureSpecsFor24BitColor;
	else if ( m_ImageDisplayMethod == RENDER_METHOD_30BIT_COLOR )
		TextureLayout = TextureSpecsFor30BitColor;
	else
		TextureLayout = TextureSpecsFor10BitGrayscale;
	TextureLayout.OriginalImageExternalColorFormat = m_pAssignedDiagnosticImage -> m_ImageColorFormat;

	// Load the texture color buffer from the image pixel data, transforming it appropriately.
	glTexImage2D( GL_TEXTURE_2D, 0, TextureLayout.SizedTextureInternalColorFormat, ImageWidth, ImageHeight, 0,
					TextureLayout.OriginalImageExternalColorFormat, OriginalPixelDataType, (GLvoid*)m_pAssignedDiagnosticImage -> m_pImageData );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

}


// Set up the vertices that describe to the GPU where the rectangular image frame is to be positioned
// in the framebuffer (and on the screen).
void CImageView::InitializeImageVertices()
{
	glGenBuffers( 2, m_VertexBufferID );
	CheckOpenGLResultAt( __FILE__, __LINE__ );

	glGenVertexArrays( 2, m_VertexAttributesID );
	CheckOpenGLResultAt( __FILE__, __LINE__ );
}



// Set up the vertices that describe to the GPU where the rectangular image frame is to be positioned
// in the framebuffer (and on the screen).
void CImageView::DeleteImageVertices()
{
	if ( m_VertexBufferID[ IMAGE_VERTEXES ] != 0 )
		glDeleteBuffers( 2, m_VertexBufferID );
	if ( m_VertexAttributesID[ IMAGE_VERTEXES ] != 0 )
		glDeleteVertexArrays( 2, m_VertexAttributesID );
}


// Prepare an off-screen framebuffer object for receiving the image in the form of a texture
// stored in the extended pixel format.  (OpenGL extended pixel formats are not displayable>0
// The image will be rendered to this extended pixel format framebuffer instead of the display
// framebuffer.  Then the extended format texture in this framebuffer texture will be
// rendered to the display.
BOOL CImageView::InitializeOffScreenFrameBuffer()
{
	BOOL			bNoError = TRUE;
	GLsizei			ViewportRect[ 4 ];
	GLenum			FrameBufferCompleteness;

	glGetIntegerv( GL_VIEWPORT, ViewportRect );

	RemoveOffScreenFrameBuffer();
	glGenFramebuffers( 1, &m_OffScreenFrameBufferID );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	// Bind the framebuffer object to the current rendering context as the rendering buffer.
	glBindFramebuffer( GL_FRAMEBUFFER, m_OffScreenFrameBufferID );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	glActiveTexture( TEXTURE_UNIT_SCREEN_IMAGE );

	// Set up the properties for the texture to be rendered into this framebuffer.
	glBindTexture( GL_TEXTURE_2D, m_ScreenImageTextureID );
	CheckOpenGLResultAt( __FILE__, __LINE__ );
	// Set mipmapping to a single level to turn it off.
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 );
	// Turn off mipmapping, or else the texture will not be complete without mipmap specifications.
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	// Allocate storage for the image texture to be rendered here.
	glTexImage2D( GL_TEXTURE_2D, 0, TextureLayout.SizedScreenTextureInternalColorFormat, ViewportRect[ 2 ], ViewportRect[ 3 ], 0,
					TextureLayout.OriginalScreenImageColorFormat, TextureLayout.OriginalScreenTextureStorageFormat, 0 );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ScreenImageTextureID, 0 );

	// Check for framebuffer completeness.
	FrameBufferCompleteness = glCheckFramebufferStatus( GL_DRAW_FRAMEBUFFER );
	if ( FrameBufferCompleteness != GL_FRAMEBUFFER_COMPLETE )
		{
		CheckOpenGLResultAt( __FILE__, __LINE__	);
		LogMessage( "*** Error:  Incomplete Draw Frame Buffer Object", MESSAGE_TYPE_SUPPLEMENTARY );
		bNoError = FALSE;
		}
	// Bind the default framebuffer for on-screen rendering.  The off-screen framebuffer will be bound
	// when rendering to it.
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	// Set up the rectangle for framing the displayed image.
	InitScreenVertexSquareFrame();

	glActiveTexture( TEXTURE_UNIT_DEFAULT );

	return bNoError;
}


// The method used to display an image using the OpenGL interface is (1) to load
// the image into a texture buffer, (2) define the display region as a rectangle,
// and (3) use the texture (the image) to paint the rectangle.
// The input image data buffer is at m_pAssignedDiagnosticImage -> m_pImageData.
//

// This function loads a new image into a texture structure.  This only needs to happen once per image
// view request.

BOOL CImageView::LoadImageAsTexture()
{
	BOOL				bNoError = TRUE;
	BOOL				bViewportIsValid;
	CGraphicsAdapter	*pGraphicsAdapter;
	GLuint				hShaderProgram;
	char				Msg[ 256 ];
	char				SystemErrorMessage[ FULL_FILE_SPEC_STRING_LENGTH ];

	if ( m_pAssignedDiagnosticImage != 0 && m_pAssignedDiagnosticImage -> m_pImageData != 0 )
		{
		GetExclusiveRightToRender();

		bViewportIsValid = InitViewport();
		if ( bViewportIsValid )
			{
			m_bImageHasBeenRendered = FALSE;

			pGraphicsAdapter = (CGraphicsAdapter*)m_pDisplayMonitor -> m_pGraphicsAdapter;
			bNoError = ( pGraphicsAdapter != 0 );
			if ( bNoError )
				{
				bNoError = wglMakeCurrent( m_hDC, m_hRC );
				if ( !bNoError )
					{
					SystemErrorCode = GetLastSystemErrorMessage( SystemErrorMessage, FULL_FILE_SPEC_STRING_LENGTH - 1 );
					if ( SystemErrorCode != 0 )
						{
						sprintf( Msg, "Error:  System message:  %s", SystemErrorMessage );
						LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
						}
					}

				// Empth the OpenGL error message queue.
				do
					bNoError = CheckOpenGLResultAt( __FILE__, __LINE__ );
				while ( !bNoError );

				// Specify which shader program the GPU is to run for displaying this image.
				if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_REPORT )
					hShaderProgram = m_gReportFormShaderProgram;
				else
					{
					switch ( m_ImageDisplayMethod  )
						{
						case RENDER_METHOD_8BIT_COLOR:
						case RENDER_METHOD_30BIT_COLOR:
							hShaderProgram = m_g30BitColorShaderProgram;
							break;
						case RENDER_METHOD_16BIT_PACKED_GRAYSCALE:
							hShaderProgram = m_g10BitGrayscaleShaderProgram;
							break;
						};
					}
				glUseProgram( hShaderProgram );
				CheckOpenGLResultAt( __FILE__, __LINE__ );

				InitializeAndLoadTheImageTexture();

				glActiveTexture( TEXTURE_UNIT_LOADED_IMAGE );			// Re-bind the image texture unit to the current rendering context.
				if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_REPORT )
					glUniform1i( glGetUniformLocation(  hShaderProgram, "ReportFormTexture" ), TEXUNIT_NUMBER_LOADED_IMAGE );
				else if ( m_ImageDisplayMethod == RENDER_METHOD_16BIT_PACKED_GRAYSCALE )
					glUniform1i( glGetUniformLocation(  hShaderProgram, "GrayscaleImageTexture" ), TEXUNIT_NUMBER_LOADED_IMAGE );
				else
					glUniform1i( glGetUniformLocation(  hShaderProgram, "LoadedImageTexture" ), TEXUNIT_NUMBER_LOADED_IMAGE );
				CheckOpenGLResultAt( __FILE__, __LINE__ );

				if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_REPORT || m_ImageDisplayMethod == RENDER_METHOD_8BIT_COLOR )
					RemoveOffScreenFrameBuffer();			// For the report image, flag that the off-screen frame buffer is not being used.
				if ( m_ViewFunction != IMAGE_VIEW_FUNCTION_REPORT )
					{
					if ( m_ImageDisplayMethod == RENDER_METHOD_16BIT_PACKED_GRAYSCALE )
						{
						// Specify the texture unit where the shader will reference the lookup table.
						glUniform1i( glGetUniformLocation(  hShaderProgram, "RGBLookupTable" ), TEXUNIT_NUMBER_GRAYSCALE_LOOKUP );
						CheckOpenGLResultAt( __FILE__, __LINE__ );
						}
					// Set the image dimensions for the GPU shader program.
					glUniform2f( glGetUniformLocation(  hShaderProgram, "ImageSize" ), 
																(float)m_pAssignedDiagnosticImage -> m_ImageWidthInPixels,
																(float)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels );
					CheckOpenGLResultAt( __FILE__, __LINE__ );

					// Init the OpenGl state.
					glShadeModel( GL_SMOOTH );							// Enable Smooth Shading

//					if ( m_ImageDisplayMethod == RENDER_METHOD_30BIT_COLOR || m_ImageDisplayMethod == RENDER_METHOD_8BIT_COLOR )
					if ( m_ImageDisplayMethod == RENDER_METHOD_30BIT_COLOR )
						{
						// Set up the off-screen framebuffer, which is capable of rendering the extended pixel formats.
						bNoError = InitializeOffScreenFrameBuffer();

						glUseProgram( m_g30BitScreenShaderProgram );
						CheckOpenGLResultAt( __FILE__, __LINE__ );
						// For 2-pass 30-bit color rendering, ttach the screen texture to texture unit 1.
						glUniform1i( glGetUniformLocation(  m_g30BitScreenShaderProgram, "Pass2ScreenTexture" ), TEXUNIT_NUMBER_SCREEN_IMAGE );
						CheckOpenGLResultAt( __FILE__, __LINE__ );
						}
					else
						RemoveOffScreenFrameBuffer();			// Flag that the off-screen frame buffer is not being used for 10-bit grayscaale.
					}

				glUseProgram( 0 );
				CheckOpenGLResultAt( __FILE__, __LINE__ );
				}
			}
		else
			AllowOthersToRender();
		CheckOpenGLResultAt( __FILE__, __LINE__ );

		if ( !bNoError )
			LogMessage( " >>>  An error occurred in LoadImageAsTexture().", MESSAGE_TYPE_SUPPLEMENTARY );
		else
			LogMessage( "LoadImageAsTexture() completed.", MESSAGE_TYPE_SUPPLEMENTARY );
		}

	return bViewportIsValid;
}


// This function is called after the image has been loaded as a texture.  Here a rectangle is created
// and scaled to the image.  Any rotation, flip or magnification is performed on the rectangle.  The
// image will later applied as a texture to the transformed rectangle during rendering. 
//
// This function must be called whenever the user changes the image position, magnification or orientation.
// These processing steps are done here, so they won't have to be repeated every time the brightness and
// contrast are changed.  This makes the brightness and contrast changes much faster.
//
// This function performs image scaling, orientation and positioning operations.  It must be called after each left
// mouse button movement, or rotation request, or scaling request.
void CImageView::PrepareImage()
{
	GLsizei					ViewportRect[ 4 ];
	GLfloat					ViewportWidth;
	GLfloat					ViewportHeight;
	GLfloat					ImageWidthInPixels;
	GLfloat					ImageHeightInPixels;
	GLfloat					TranslationX;
	GLfloat					TranslationY;
	GLfloat					ImageScale;
	GLfloat					ImageOriginX;
	GLfloat					ImageOriginY;
	GLfloat					X, Y;
	GLfloat					XMin, XMax;
	GLfloat					YMin, YMax;

	if ( m_pAssignedDiagnosticImage != 0 && m_pAssignedDiagnosticImage -> m_pImageData != 0 )
		{
		CheckOpenGLResultAt( __FILE__, __LINE__ );

		glGetIntegerv( GL_VIEWPORT, ViewportRect );
		ViewportWidth = (GLfloat)ViewportRect[ 2 ] - (GLfloat)ViewportRect[ 0 ];
		ViewportHeight = (GLfloat)ViewportRect[ 3 ] - (GLfloat)ViewportRect[ 1 ];
		ImageWidthInPixels = (float)m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
		ImageHeightInPixels = (float)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;

		ImageScale = (GLfloat)m_pAssignedDiagnosticImage -> m_ScaleFactor;

		ImageOriginX = ( ViewportWidth - ImageWidthInPixels * ImageScale ) / 2.0f;
		ImageOriginY = ( ViewportHeight - ImageHeightInPixels * ImageScale ) / 2.0f;
	
		TranslationX = ( ImageWidthInPixels / 2.0f - (GLfloat)m_pAssignedDiagnosticImage -> m_FocalPoint.x ) * ImageScale;
		TranslationY = - ( ImageHeightInPixels / 2.0f - (GLfloat)m_pAssignedDiagnosticImage -> m_FocalPoint.y ) * ImageScale;

		X = ImageOriginX + TranslationX;
		Y = ImageOriginY + TranslationY;

		// Set up the image frame vertices for the currently scaled and panned image.
		// Adjust cell boundarys to mornalize to -1.0 < x , 1.0, -1.0 < y , 1.0.
		XMin = 2.0f * X / (GLfloat)ViewportWidth - 1.0f;
		YMin = 2.0f * Y / (GLfloat)ViewportHeight - 1.0f;
		XMax = 2.0f * ( X + ImageScale * ImageWidthInPixels ) / (GLfloat)ViewportWidth - 1.0f;
		YMax = 2.0f * ( Y + ImageScale * ImageHeightInPixels ) / (GLfloat)ViewportHeight - 1.0f;

		// Set up the vertex array, which consists of two triangles arranged to make a rectangle on the
		// display, representing the boundaries of the image to be displayed.
		// Also set the texture corners depending upon the rotation setting.
		InitImageVertexRectangle( XMin, XMax, YMin, YMax, (GLfloat)ViewportWidth / (GLfloat)ViewportHeight );

		if ( m_pAssignedDiagnosticImage -> m_bFlipVertically )
			FlipFrameVertically();
		if ( m_pAssignedDiagnosticImage -> m_bFlipHorizontally )
			FlipFrameHorizontally();

		AllowOthersToRender();
//		LogMessage( "PrepareImage() completed.", MESSAGE_TYPE_SUPPLEMENTARY );
		}
}


// This function applies windowing adjustments to the prepared image and renders the result on the display.
// There are two options for rendering:
//
//	10-bit Grayscale:
//		For display panels set to use this rendering method only a single pass rendering is required and
//		the off-screen framebuffer is not used.  A one-dimeensional lookup table is referenced from
//		texture unit 1 to remap the grayscale pixel values into appropriate RGB color values for the
//		display.  The RGB values in the table are used by the display to specify equivalent 10-bit
//		grayscale screen tones.
//
//	30-bit Color:
//		For display panels set to use this rendering method a 2-pass rendering approach is needed.
//		An extended pixel format is specified for rendering 10 bits of color in each pixel component.
//		But OpenGL extended pixel formats are not displayable.  Instead of rendering directly to the
//		screen, the image is rendered (in the extended pixel format) to an intermediate off-screen
//		framebuffer in such a way that it can be read from this framebuffer as a texture.  The second
//		pass then renders this texture, which contains 30-bit color precision, to the screen.
//
//		24-bit Color or 8-bit Grayscale:
//		For display panels that only handle 24-bit color or 8-bit grayscale, the 30-bit color rendering
//		process is used.  The graphics card and associated driver automatically devolve the 30-bit color
//		into conventional 24-bit color rendering.
//
//	This function must be called whenever there is any change to be applied to the image.  It is called
//	most often for changing the windowing (brightness and contrast) settings as the right mouse button
//	is depresed while scrolling the mouse around the image.  For this to be a responsive operation,
//	this function must execute very rapidly.
//
void CImageView::RenderImage()
{
	BOOL				bViewportIsValid;
	CGraphicsAdapter	*pGraphicsAdapter;
	GLuint				hShaderProgram;
	GLfloat				MaxGrayIndex;

	bViewportIsValid = InitViewport();
	CheckOpenGLResultAt( __FILE__, __LINE__ );
	if ( bViewportIsValid )
		{
		pGraphicsAdapter = (CGraphicsAdapter*)m_pDisplayMonitor -> m_pGraphicsAdapter;
		if ( pGraphicsAdapter != 0 )
			{
			if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_REPORT )
				hShaderProgram = m_gReportFormShaderProgram;
			else
				{
				switch ( m_ImageDisplayMethod  )
					{
					case RENDER_METHOD_8BIT_COLOR:
					case RENDER_METHOD_30BIT_COLOR:
						hShaderProgram = m_g30BitColorShaderProgram;
						break;
					case RENDER_METHOD_16BIT_PACKED_GRAYSCALE:
						hShaderProgram = m_g10BitGrayscaleShaderProgram;
						break;
					};
				}
			glUseProgram( hShaderProgram );
			CheckOpenGLResultAt( __FILE__, __LINE__ );

			// This rendering pass references the loaded image texture.
			glActiveTexture( TEXTURE_UNIT_LOADED_IMAGE );

			// Load the values for the variables referenced in the GPU shader program.
			if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_REPORT )
				{
				glBindTexture( GL_TEXTURE_2D, m_LoadedImageTextureID );
				CheckOpenGLResultAt( __FILE__, __LINE__ );
				}
			else if ( m_pAssignedDiagnosticImage != 0 && m_pAssignedDiagnosticImage -> m_pImageData != 0 )
				{
				glUniform1f( glGetUniformLocation( hShaderProgram, "WindowMin" ), (GLfloat)m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowMinPixelAmplitude );
				glUniform1f( glGetUniformLocation( hShaderProgram, "WindowMax" ), (GLfloat)m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowMaxPixelAmplitude );

				if ( pBViewerCustomization -> m_WindowingAlgorithmSelection == SELECT_SIGMOID_WINDOWING )
					glUniform1i( glGetUniformLocation( hShaderProgram, "bWindowingIsSigmoidal" ), 1 );
				else
					glUniform1i( glGetUniformLocation( hShaderProgram, "bWindowingIsSigmoidal" ), 0 );

				if ( m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_bColorsInverted )
					glUniform1i( glGetUniformLocation( hShaderProgram, "bInvertGrayscale" ), 1 );
				else
					glUniform1i( glGetUniformLocation( hShaderProgram, "bInvertGrayscale" ), 0 );
				CheckOpenGLResultAt( __FILE__, __LINE__ );

				MaxGrayIndex = (GLfloat)( 2 << ( m_pAssignedDiagnosticImage -> m_ImageBitDepth - 1 ) );
				glUniform1f( glGetUniformLocation( hShaderProgram, "MaxGrayIndex" ), MaxGrayIndex );

				glUniform1f( glGetUniformLocation( hShaderProgram, "GammaValue" ), (GLfloat)m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_Gamma );
				CheckOpenGLResultAt( __FILE__, __LINE__ );

				glBindTexture( GL_TEXTURE_2D, m_LoadedImageTextureID );
				CheckOpenGLResultAt( __FILE__, __LINE__ );

				if ( m_ImageDisplayMethod == RENDER_METHOD_16BIT_PACKED_GRAYSCALE )
					{
					// For 10-bit grayscale, map the 1-dimensional RGB lookup table into texture unit 1.
					glActiveTexture( TEXTURE_UNIT_GRAYSCALE_LOOKUP );
					glBindTexture( GL_TEXTURE_1D, pGraphicsAdapter -> m_glLUT12BitTextureId );
					glActiveTexture( TEXTURE_UNIT_LOADED_IMAGE );			// Re-bind the image texture unitto the current rendering context.
					CheckOpenGLResultAt( __FILE__, __LINE__ );
					}
				}
			else
				{
				glBindTexture( GL_TEXTURE_2D, 0 );			// If we don't know what's happening, bail out.
				CheckOpenGLResultAt( __FILE__, __LINE__ );
				}

			// Bind the vertex array and associated attributes to the current rendering context.
			glBindBuffer( GL_ARRAY_BUFFER, m_VertexBufferID[ IMAGE_VERTEXES ] );
			CheckOpenGLResultAt( __FILE__, __LINE__ );

			glBufferData( GL_ARRAY_BUFFER, sizeof( m_VertexRectangle ), &m_VertexRectangle, GL_STATIC_DRAW );
			CheckOpenGLResultAt( __FILE__, __LINE__ );

			// Bind the Vertex Array Object first, then bind and set the vertex buffer.  Then configure vertex attributes(s).
			glBindVertexArray( m_VertexAttributesID[ IMAGE_VERTEXES ] );
			CheckOpenGLResultAt( __FILE__, __LINE__ );

			// Set up and enable the vertex attribute array at location 0 in the vertex shader.  This array specifies the vertex positions.
			glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0 );
			CheckOpenGLResultAt( __FILE__, __LINE__ );

			// Set up and enable the vertex attribute array at location 1 in the vertex shader.  This array specifies the texture vertex positions.
			glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (GLvoid*)( 12 * sizeof(float) ) );
			CheckOpenGLResultAt( __FILE__, __LINE__ );

			glEnableVertexAttribArray( 0 );
			CheckOpenGLResultAt( __FILE__, __LINE__ );

			glEnableVertexAttribArray( 1 );		// ( location = 1 )
			CheckOpenGLResultAt( __FILE__, __LINE__ );

			glBindVertexArray( m_VertexAttributesID[ IMAGE_VERTEXES ] );
			CheckOpenGLResultAt( __FILE__, __LINE__ );

			// If the off-screen frame buffer has been initialized, set up to render to it.  Otherwise, the
			// default display buffer (for on-screen rendering) is still bound.
			if ( m_OffScreenFrameBufferID != 0 )
				{
				glBindFramebuffer( GL_FRAMEBUFFER, m_OffScreenFrameBufferID );
				CheckOpenGLResultAt( __FILE__, __LINE__	);

				glDrawBuffer( GL_COLOR_ATTACHMENT0 );			// Specify which framebuffer attachment is to be rendered to.
				CheckOpenGLResultAt( __FILE__, __LINE__	);
				}
			// Apply the texture image to the appropriately transformed rectangle.
			// This transformed image is now in the frame buffer.
			glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );				// Black Background
			glClear( GL_COLOR_BUFFER_BIT );						// Clear out the currently rendered image from the frame buffer.

			// Set up and render the currently bound 2-dimensional texture.
//			glEnable( GL_TEXTURE_2D );
//			if ( m_ImageDisplayMethod == RENDER_METHOD_16BIT_PACKED_GRAYSCALE )
//				glEnable( GL_TEXTURE_1D );
			CheckOpenGLResultAt( __FILE__, __LINE__ );

			// Render the image into the framebuffer.  If the off-screen framebuffer is not being used
			// this will render directly to the display.  
			glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
			CheckOpenGLResultAt( __FILE__, __LINE__ );

//			glDisable( GL_TEXTURE_2D );
//			if ( m_ImageDisplayMethod == RENDER_METHOD_16BIT_PACKED_GRAYSCALE )
//				glDisable( GL_TEXTURE_1D );


			// If the image was rendered into the off-screen texture with enhanced pixel bit-depth,
			// switch over to the default frame buffer and blit the image to the display.
			if ( m_OffScreenFrameBufferID != 0 )
				{
				// Switch over to use the shader program for the default (screen) frame buffer.
				glUseProgram( m_g30BitScreenShaderProgram );
				CheckOpenGLResultAt( __FILE__, __LINE__ );
	
				glActiveTexture( TEXTURE_UNIT_SCREEN_IMAGE );
//				glEnable( GL_TEXTURE_2D );

				glUniform1i( glGetUniformLocation(  m_g30BitScreenShaderProgram, "Pass2ScreenTexture" ), TEXUNIT_NUMBER_SCREEN_IMAGE );

				glBindFramebuffer( GL_FRAMEBUFFER, 0 );			// Bind the default (screen) framebuffer to the current rendering context.
				CheckOpenGLResultAt( __FILE__, __LINE__	);

				// Reference the full-screen vertex array.
				glBindBuffer( GL_ARRAY_BUFFER, m_VertexBufferID[ SCREEN_VERTEXES ] );
				CheckOpenGLResultAt( __FILE__, __LINE__ );

				glBufferData( GL_ARRAY_BUFFER, sizeof( m_ScreenVertexRectangle ), &m_ScreenVertexRectangle, GL_STATIC_DRAW );
				CheckOpenGLResultAt( __FILE__, __LINE__ );

				// Enable the vertex attribute array at location 0 in the vertex shader.  This array specifies the vertex positions.

				// Set up and enable the vertex attribute array at location 0 in the vertex shader.  This array specifies the vertex positions.
				glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0 );
				CheckOpenGLResultAt( __FILE__, __LINE__ );

				glEnableVertexAttribArray( 0 );
				CheckOpenGLResultAt( __FILE__, __LINE__ );

				// Set up and enable the vertex attribute array at location 1 in the vertex shader.  This array specifies the texture positions.
				glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (GLvoid*)( 12 * sizeof(float) ) );
				CheckOpenGLResultAt( __FILE__, __LINE__ );
				glEnableVertexAttribArray( 1 );		// ( location = 1 )
				CheckOpenGLResultAt( __FILE__, __LINE__ );

				glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );				// Black Background
				glClear( GL_COLOR_BUFFER_BIT );						// Clear out the currently rendered image from the frame buffer.
				CheckOpenGLResultAt( __FILE__, __LINE__ );

				glDrawBuffer( GL_BACK );
				CheckOpenGLResultAt( __FILE__, __LINE__ );

				// Bind the contents of the off-screen framebuffer as an extended bit-depth texture.  This texture was previously
				// mapped to the GL_COLOR_ATTACHMENT0 of the off-screen framebuffer during framebuffer initialization.
				glBindTexture( GL_TEXTURE_2D, m_ScreenImageTextureID );
				CheckOpenGLResultAt( __FILE__, __LINE__ );

				// Render the image into the default framebuffer to display it.  This applies the currently bound texture
				// to the screen vertices.
				glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
				CheckOpenGLResultAt( __FILE__, __LINE__ );
				}
			glUseProgram( 0 );
			CheckOpenGLResultAt( __FILE__, __LINE__ );
			}
//		glDisable( GL_TEXTURE_2D );
		glActiveTexture( TEXTURE_UNIT_DEFAULT );

		CheckOpenGLResultAt( __FILE__, __LINE__ );

		// Apply any required overlays to the image in the framebuffer.  For saving or printing reports, a
		// separate off-screen framebuffer is set up to build a full-scale image of the report.  Rendering
		// to that framebuffer is handled separately in the save and print functions.

		if ( m_pAssignedDiagnosticImage != 0 && m_pAssignedDiagnosticImage -> m_bEnableOverlays )
			{
			if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_PATIENT )
				{
				if ( m_pImageAnnotationList != 0 )
					{
					RenderImageAnnotations( m_hDC );
					CheckOpenGLResultAt( __FILE__, __LINE__ );
					}
				if ( m_pMeasuredIntervalList != 0 && m_PixelsPerMillimeter > 0.0 )
					{
					RenderImageMeasurements();
					CheckOpenGLResultAt( __FILE__, __LINE__ );
					}
				}
			else if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_REPORT )
				{
				RenderReport( m_hDC, IMAGE_DESTINATION_WINDOW );
				CheckOpenGLResultAt( __FILE__, __LINE__ );
				}
			}

		CheckOpenGLResultAt( __FILE__, __LINE__ );
		}

	Invalidate( FALSE );			// Tell windows to repaint the display.
	m_bImageHasBeenRendered = TRUE;

	AllowOthersToRender();
//		LogMessage( "RenderImage() completed.", MESSAGE_TYPE_SUPPLEMENTARY );
}



// Note:  If all the views run at the same priority, this should not need mutex protection.
void CImageView::GetExclusiveRightToRender()
{
	CMainFrame		*pMainFrame;
	char			SystemErrorMessage[ FULL_FILE_SPEC_STRING_LENGTH ];
	char			Msg[ FULL_FILE_SPEC_STRING_LENGTH ];

	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if (pMainFrame != 0 )
		{
		while ( !pMainFrame -> m_bOKToRenderImage )
			Sleep( 2 );		// Release the processor for 2 milliseconds to allow other view to finish rendering.
		pMainFrame -> m_bOKToRenderImage = FALSE;		// Prevent other views from rendering.
		if ( wglMakeCurrent( m_hDC, m_hRC ) == FALSE )
			{
			SystemErrorCode = GetLastSystemErrorMessage( SystemErrorMessage, FULL_FILE_SPEC_STRING_LENGTH - 1 );
			if ( SystemErrorCode != 0 )
				{
				sprintf( Msg, "Error setting current rendering context.  System message:  %s", SystemErrorMessage );
				LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
				}
			CheckOpenGLResultAt( __FILE__, __LINE__ );
			}
		}
}


void CImageView::AllowOthersToRender()
{
	CMainFrame		*pMainFrame;

	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if (pMainFrame != 0 )
		pMainFrame -> m_bOKToRenderImage = TRUE;			// Re-enable other views to render.
}


void CImageView::OnPaint()
{
	if ( !m_bRenderingCurrentlyBusy )
		{
		m_bRenderingCurrentlyBusy = TRUE;

		GetExclusiveRightToRender();

		RenderImage();

		// Call function to swap the buffers.
		SwapBuffers( m_hDC );
		ValidateRect( NULL );	// Validate the entire client area, so Windows won't keep trying to re-paint it.
		m_bRenderingCurrentlyBusy = FALSE;
		AllowOthersToRender();
		}
}


// This method speeds up the repainting of images.
void CImageView::RepaintFast()
{
	struct __timeb64	CurrentSystemTime;
	double				TimeInSeconds;				// To the nearest millisecond.
	double				TimeDifferenceInSeconds;

	if ( !m_bRenderingCurrentlyBusy )
		{
		_ftime64( &CurrentSystemTime );
		TimeInSeconds = CurrentSystemTime.time + CurrentSystemTime.millitm / 1000.0;
		TimeDifferenceInSeconds = TimeInSeconds - m_TimeOfLastFastPaint;
		if ( TimeDifferenceInSeconds > 0.005 )
			{
			Invalidate( TRUE );		// Signal to repaint the entire client area, erasing the background.
			UpdateWindow();			// Bypass the windows message queue by sending a WM_PAINT message directly.
			m_TimeOfLastFastPaint = TimeInSeconds;
			}
		}
}


typedef struct
	{
	GLfloat				BoxX;
	GLfloat				BoxY;
	int					ResourceSymbol;
	} REPORT_BOX_LOCATION;



REPORT_BOX_LOCATION		ReportNIOSHOnlyPage1BoxArray[] =
					{
						{	462.0f,	629.0f,	IDC_BUTTON_A_READER					},
						{	498.0f,	629.0f,	IDC_BUTTON_B_READER					},
						{	530.0f,	629.0f,	IDC_BUTTON_FACILITY_READING			},

						{	373.0f,	290.0f,	IDC_BUTTON_ANGLE_OBLIT_0			},

						{	325.0f,	183.0f,	IDC_BUTTON_OTHER_YES				},
						{	471.0f,	183.0f,	IDC_BUTTON_OTHER_NO					},

						{ 0.0,	0.0,	0										}
					};

REPORT_BOX_LOCATION		ReportGeneralPurposeOnlyPage1BoxArray[] =
					{
						{	518.0f,	290.0f,	IDC_BUTTON_ANGLE_OBLIT_0			},

						{	404.0f,	182.0f,	IDC_BUTTON_OTHER_YES				},
						{	519.0f,	182.0f,	IDC_BUTTON_OTHER_NO					},

						{	322.0f,	106.0f,	IDC_BUTTON_SEE_PHYSICIAN_YES		},
						{	375.0f,	106.0f,	IDC_BUTTON_SEE_PHYSICIAN_NO			},

						{	68.0f,	153.0f,	IDC_BUTTON_SYMBOL_AA				},
						{	84.0f,	153.0f,	IDC_BUTTON_SYMBOL_AT				},
						{	103.0f,	153.0f,	IDC_BUTTON_SYMBOL_AX				},
						{	120.0f,	153.0f,	IDC_BUTTON_SYMBOL_BU				},
						{	138.0f,	153.0f,	IDC_BUTTON_SYMBOL_CA				},
						{	155.0f,	153.0f,	IDC_BUTTON_SYMBOL_CG				},
						{	172.0f,	153.0f,	IDC_BUTTON_SYMBOL_CN				},
						{	189.0f,	153.0f,	IDC_BUTTON_SYMBOL_CO				},
						{	206.0f,	153.0f,	IDC_BUTTON_SYMBOL_CP				},
						{	223.0f,	153.0f,	IDC_BUTTON_SYMBOL_CV				},
						{	241.0f,	153.0f,	IDC_BUTTON_SYMBOL_DI				},
						{	258.0f,	153.0f,	IDC_BUTTON_SYMBOL_EF				},
						{	275.0f,	153.0f,	IDC_BUTTON_SYMBOL_EM				},
						{	292.0f,	153.0f,	IDC_BUTTON_SYMBOL_ES				},
						{	309.0f,	153.0f,	IDC_BUTTON_SYMBOL_FR				},
						{	326.0f,	153.0f,	IDC_BUTTON_SYMBOL_HI				},
						{	343.0f,	153.0f,	IDC_BUTTON_SYMBOL_HO				},
						{	361.0f,	153.0f,	IDC_BUTTON_SYMBOL_ID				},
						{	378.0f,	153.0f,	IDC_BUTTON_SYMBOL_IH				},
						{	395.0f,	153.0f,	IDC_BUTTON_SYMBOL_KL				},
						{	412.0f,	153.0f,	IDC_BUTTON_SYMBOL_ME				},
						{	429.0f,	153.0f,	IDC_BUTTON_SYMBOL_PA				},
						{	446.0f,	153.0f,	IDC_BUTTON_SYMBOL_PB				},
						{	463.0f,	153.0f,	IDC_BUTTON_SYMBOL_PI				},
						{	481.0f,	153.0f,	IDC_BUTTON_SYMBOL_PX				},
						{	498.0f,	153.0f,	IDC_BUTTON_SYMBOL_RA				},
						{	515.0f,	153.0f,	IDC_BUTTON_SYMBOL_RP				},
						{	530.0f,	153.0f,	IDC_BUTTON_SYMBOL_TB				},
						{	68.0f,	133.0f,	IDC_BUTTON_SYMBOL_OD				},

						{ 0.0,	0.0,	0										}
					};


REPORT_BOX_LOCATION		ReportPage1BoxArray[] =
					{
						{	56.1f,	562.0f,	IDC_BUTTON_IMAGE_GRADE_1			},
						{	75.4f,	562.0f,	IDC_BUTTON_IMAGE_GRADE_2			},
						{	94.1f,	562.0f,	IDC_BUTTON_IMAGE_GRADE_3			},
						{	112.2f,	562.0f,	IDC_BUTTON_IMAGE_GRADE_UR			},
						{	143.0f,	579.6f,	IDC_BUTTON_IMAGE_OVEREXPOSED		},
						{	143.0f, 559.0f,	IDC_BUTTON_IMAGE_UNDEREXPOSED		},
						{	143.0f,	538.6f,	IDC_BUTTON_IMAGE_ARTIFACTS			},
						{	242.0f,	579.6f,	IDC_BUTTON_IMAGE_IMPROPER_POSITION	},
						{	242.0f,	559.0f,	IDC_BUTTON_IMAGE_POOR_CONTRAST		},
						{	242.0f,	538.6f,	IDC_BUTTON_IMAGE_POOR_PROCESSING	},
						{	328.0f,	579.6f,	IDC_BUTTON_IMAGE_UNDERINFLATION		},
						{	328.0f,	559.0f,	IDC_BUTTON_IMAGE_MOTTLE				},
						{	328.0f,	538.6f,	IDC_BUTTON_IMAGE_EXCESSIVE_EDGE		},
						{	425.0f,	579.6f,	IDC_BUTTON_IMAGE_OTHER				},

						{	408.0f,	513.0f,	IDC_BUTTON_PARENCHYMAL_YES			},
						{	516.0f,	513.0f,	IDC_BUTTON_PARENCHYMAL_NO			},
						{	74.0f,	463.0f,	IDC_BUTTON_PRIMARY_P				},
						{	91.0f,	463.0f,	IDC_BUTTON_PRIMARY_S				},
						{	74.0f,	447.0f,	IDC_BUTTON_PRIMARY_Q				},
						{	91.0f,	447.0f,	IDC_BUTTON_PRIMARY_T				},
						{	74.0f,	431.0f,	IDC_BUTTON_PRIMARY_R				},
						{	91.0f,	431.0f,	IDC_BUTTON_PRIMARY_U				},

						{	124.0f,	463.0f,	IDC_BUTTON_SECONDARY_P				},
						{	141.0f,	463.0f,	IDC_BUTTON_SECONDARY_S				},
						{	124.0f,	447.0f,	IDC_BUTTON_SECONDARY_Q				},
						{	141.0f,	447.0f,	IDC_BUTTON_SECONDARY_T				},
						{	124.0f,	431.0f,	IDC_BUTTON_SECONDARY_R				},
						{	141.0f,	431.0f,	IDC_BUTTON_SECONDARY_U				},

						{	228.0f,	463.7f,	IDC_BUTTON_ZONE_UPPER_RIGHT			},
						{	245.0f,	463.7f,	IDC_BUTTON_ZONE_UPPER_LEFT			},
						{	228.0f,	447.0f,	IDC_BUTTON_ZONE_MIDDLE_RIGHT		},
						{	245.0f,	447.0f,	IDC_BUTTON_ZONE_MIDDLE_LEFT			},
						{	228.0f,	430.5f,	IDC_BUTTON_ZONE_LOWER_RIGHT			},
						{	245.0f,	430.5f,	IDC_BUTTON_ZONE_LOWER_LEFT			},

						{	295.0f,	481.5f,	IDC_BUTTON_PROFUSION_0MINUS			},
						{	311.5f,	481.5f,	IDC_BUTTON_PROFUSION_00				},
						{	327.5f,	481.5f,	IDC_BUTTON_PROFUSION_01				},
						{	295.0f,	464.5f,	IDC_BUTTON_PROFUSION_10				},
						{	311.5f,	464.5f,	IDC_BUTTON_PROFUSION_11				},
						{	327.5f,	464.5f,	IDC_BUTTON_PROFUSION_12				},
						{	295.0f,	449.0f,	IDC_BUTTON_PROFUSION_21				},
						{	311.5f,	449.0f,	IDC_BUTTON_PROFUSION_22				},
						{	327.5f,	449.0f,	IDC_BUTTON_PROFUSION_23				},
						{	295.0f,	432.0f,	IDC_BUTTON_PROFUSION_32				},
						{	311.5f,	432.0f,	IDC_BUTTON_PROFUSION_33				},
						{	327.5f,	432.0f,	IDC_BUTTON_PROFUSION_3PLUS			},

						{	426.0f,	458.0f,	IDC_BUTTON_LARGE_OPACITY_0			},
						{	446.0f,	458.0f,	IDC_BUTTON_LARGE_OPACITY_A			},
						{	465.0f,	458.0f,	IDC_BUTTON_LARGE_OPACITY_B			},
						{	484.0f,	458.0f,	IDC_BUTTON_LARGE_OPACITY_C			},
						
						{	404.0f,	404.6f,	IDC_BUTTON_PLEURAL_YES				},
						{	518.0f,	404.6f,	IDC_BUTTON_PLEURAL_NO				},

						{	111.0f,	361.0f,	IDC_BUTTON_PLEURAL_SITE_PROFILE_0	},
						{	128.0f,	361.0f,	IDC_BUTTON_PLEURAL_SITE_PROFILE_R	},
						{	146.0f,	361.0f,	IDC_BUTTON_PLEURAL_SITE_PROFILE_L	},
						{	111.0f,	344.0f,	IDC_BUTTON_PLEURAL_SITE_FACE_ON_0	},
						{	128.0f,	344.0f,	IDC_BUTTON_PLEURAL_SITE__FACE_ON_R	},
						{	146.0f,	344.0f,	IDC_BUTTON_PLEURAL_SITE__FACE_ON_L	},
						{	111.0f,	328.0f,	IDC_BUTTON_PLEURAL_SITE_DIAPHRAGM_0	},
						{	128.0f,	328.0f,	IDC_BUTTON_PLEURAL_SITE_DIAPHRAGM_R	},
						{	146.0f,	328.0f,	IDC_BUTTON_PLEURAL_SITE_DIAPHRAGM_L	},
						{	111.0f,	310.0f,	IDC_BUTTON_PLEURAL_SITE_OTHER_0		},
						{	128.0f,	310.0f,	IDC_BUTTON_PLEURAL_SITE_OTHER_R		},
						{	146.0f,	310.0f,	IDC_BUTTON_PLEURAL_SITE_OTHER_L		},

						{	177.0f,	361.0f,	IDC_BUTTON_CALCIFICATION_PROFILE_0	},
						{	193.0f,	361.0f,	IDC_BUTTON_CALCIFICATION_PROFILE_R	},
						{	210.0f,	361.0f,	IDC_BUTTON_CALCIFICATION_PROFILE_L	},
						{	177.0f,	344.0f,	IDC_BUTTON_CALCIFICATION_FACE_ON_0	},
						{	193.0f,	344.0f,	IDC_BUTTON_CALCIFICATION__FACE_ON_R	},
						{	210.0f,	344.0f,	IDC_BUTTON_CALCIFICATION__FACE_ON_L	},
						{	177.0f,	328.0f,	IDC_BUTTON_CALCIFICATION_DIAPHRAGM_0},
						{	193.0f,	328.0f,	IDC_BUTTON_CALCIFICATION_DIAPHRAGM_R},
						{	210.0f,	328.0f,	IDC_BUTTON_CALCIFICATION_DIAPHRAGM_L},
						{	177.0f,	310.0f,	IDC_BUTTON_CALCIFICATION_OTHER_0	},
						{	193.0f,	310.0f,	IDC_BUTTON_CALCIFICATION_OTHER_R	},
						{	210.0f,	310.0f,	IDC_BUTTON_CALCIFICATION_OTHER_L	},

						{	260.0f,	327.0f,	IDC_BUTTON_PLEURAL_EXTENT_NO_RIGHT	},
						{	277.0f,	327.0f,	IDC_BUTTON_PLEURAL_EXTENT_RIGHT		},
						{	260.0f,	310.5f,	IDC_BUTTON_PLEURAL_EXTENT_RSIZE1	},
						{	277.0f,	310.5f,	IDC_BUTTON_PLEURAL_EXTENT_RSIZE2	},
						{	294.0f,	310.5f,	IDC_BUTTON_PLEURAL_EXTENT_RSIZE3	},
						{	322.0f,	327.0f,	IDC_BUTTON_PLEURAL_EXTENT_NO_LEFT	},
						{	340.0f,	327.0f,	IDC_BUTTON_PLEURAL_EXTENT_LEFT		},
						{	322.0f,	310.5f,	IDC_BUTTON_PLEURAL_EXTENT_LSIZE1	},
						{	340.0f,	310.5f,	IDC_BUTTON_PLEURAL_EXTENT_LSIZE2	},
						{	357.0f,	310.5f,	IDC_BUTTON_PLEURAL_EXTENT_LSIZE3	},

						{	410.0f,	328.0f,	IDC_BUTTON_PLEURAL_WIDTH_NO_RIGHT	},
						{	426.0f,	328.0f,	IDC_BUTTON_PLEURAL_WIDTH_RIGHT		},
						{	410.0f,	311.5f,	IDC_BUTTON_PLEURAL_WIDTH_RSIZE1		},
						{	426.0f,	311.5f,	IDC_BUTTON_PLEURAL_WIDTH_RSIZE2		},
						{	443.0f,	311.5f,	IDC_BUTTON_PLEURAL_WIDTH_RSIZE3		},
						{	473.0f,	328.0f,	IDC_BUTTON_PLEURAL_WIDTH_NO_LEFT	},
						{	490.0f,	328.0f,	IDC_BUTTON_PLEURAL_WIDTH_LEFT		},
						{	473.0f,	311.5f,	IDC_BUTTON_PLEURAL_WIDTH_LSIZE1		},
						{	490.0f,	311.5f,	IDC_BUTTON_PLEURAL_WIDTH_LSIZE2		},
						{	507.0f,	311.5f,	IDC_BUTTON_PLEURAL_WIDTH_LSIZE3		},

						{	258.0f,	290.0f,	IDC_BUTTON_ANGLE_OBLIT_R			},
						{	275.0f,	290.0f,	IDC_BUTTON_ANGLE_OBLIT_L			},

						{	112.0f,	228.0f,	IDC_BUTTON_PLEURAL_THICK_SITE_PROFILE_0	},
						{	128.0f,	228.0f,	IDC_BUTTON_PLEURAL_THICK_SITE_PROFILE_R	},
						{	145.0f,	228.0f,	IDC_BUTTON_PLEURAL_THICK_SITE_PROFILE_L	},
						{	112.0f,	212.0f,	IDC_BUTTON_PLEURAL_THICK_SITE_FACE_ON_0	},
						{	128.0f,	212.0f,	IDC_BUTTON_PLEURAL_THICK_SITE_FACE_ON_R	},
						{	145.0f,	212.0f,	IDC_BUTTON_PLEURAL_THICK_SITE_FACE_ON_L	},

						{	208.0f,	228.0f,	IDC_BUTTON_THICK_CALCIFICATION_PROFILE_0},
						{	225.0f,	228.0f,	IDC_BUTTON_THICK_CALCIFICATION_PROFILE_R},
						{	242.0f,	228.0f,	IDC_BUTTON_THICK_CALCIFICATION_PROFILE_L},
						{	208.0f,	212.0f,	IDC_BUTTON_THICK_CALCIFICATION_FACE_ON_0},
						{	225.0f,	212.0f,	IDC_BUTTON_THICK_CALCIFICATION_FACE_ON_R},
						{	242.0f,	212.0f,	IDC_BUTTON_THICK_CALCIFICATION_FACE_ON_L},

						{	298.0f,	226.0f,	IDC_BUTTON_PLEURAL_THICK_EXTENT_NO_RIGHT},
						{	315.0f,	226.0f,	IDC_BUTTON_PLEURAL_THICK_EXTENT_RIGHT	},
						{	298.0f,	210.0f,	IDC_BUTTON_PLEURAL_THICK_EXTENT_RSIZE1	},
						{	315.0f,	210.0f,	IDC_BUTTON_PLEURAL_THICK_EXTENT_RSIZE2	},
						{	332.0f,	210.0f,	IDC_BUTTON_PLEURAL_THICK_EXTENT_RSIZE3	},
						{	362.0f,	226.0f,	IDC_BUTTON_PLEURAL_THICK_EXTENT_NO_LEFT	},
						{	378.0f,	226.0f,	IDC_BUTTON_PLEURAL_THICK_EXTENT_LEFT	},
						{	361.0f,	210.0f,	IDC_BUTTON_PLEURAL_THICK_EXTENT_LSIZE1	},
						{	378.0f,	210.0f,	IDC_BUTTON_PLEURAL_THICK_EXTENT_LSIZE2	},
						{	395.0f,	210.0f,	IDC_BUTTON_PLEURAL_THICK_EXTENT_LSIZE3	},

						{	437.0f,	227.0f,	IDC_BUTTON_PLEURAL_THICK_WIDTH_NO_RIGHT	},
						{	454.0f,	227.0f,	IDC_BUTTON_PLEURAL_THICK_WIDTH_RIGHT	},
						{	437.0f,	211.0f,	IDC_BUTTON_PLEURAL_THICK_WIDTH_RSIZE1	},
						{	454.0f,	211.0f,	IDC_BUTTON_PLEURAL_THICK_WIDTH_RSIZE2	},
						{	471.0f,	211.0f,	IDC_BUTTON_PLEURAL_THICK_WIDTH_RSIZE3	},
						{	501.0f,	227.0f,	IDC_BUTTON_PLEURAL_THICK_WIDTH_NO_LEFT	},
						{	518.0f,	227.0f,	IDC_BUTTON_PLEURAL_THICK_WIDTH_LEFT		},
						{	501.0f,	211.0f,	IDC_BUTTON_PLEURAL_THICK_WIDTH_LSIZE1	},
						{	518.0f,	211.0f,	IDC_BUTTON_PLEURAL_THICK_WIDTH_LSIZE2	},
						{	534.0f,	211.0f,	IDC_BUTTON_PLEURAL_THICK_WIDTH_LSIZE3	},

						{ 0.0,	0.0,	0										}
					};


REPORT_BOX_LOCATION		ReportNIOSHOnlyPage2BoxArray[] =
					{
						{	60.0f,	692.0f,	IDC_BUTTON_SYMBOL_AA				},
						{	78.0f,	692.0f,	IDC_BUTTON_SYMBOL_AT				},
						{	95.0f,	692.0f,	IDC_BUTTON_SYMBOL_AX				},
						{	112.0f,	692.0f,	IDC_BUTTON_SYMBOL_BU				},
						{	130.0f,	692.0f,	IDC_BUTTON_SYMBOL_CA				},
						{	147.0f,	692.0f,	IDC_BUTTON_SYMBOL_CG				},
						{	164.0f,	692.0f,	IDC_BUTTON_SYMBOL_CN				},
						{	181.0f,	692.0f,	IDC_BUTTON_SYMBOL_CO				},
						{	198.0f,	692.0f,	IDC_BUTTON_SYMBOL_CP				},
						{	215.0f,	692.0f,	IDC_BUTTON_SYMBOL_CV				},
						{	233.0f,	692.0f,	IDC_BUTTON_SYMBOL_DI				},
						{	251.0f,	692.0f,	IDC_BUTTON_SYMBOL_EF				},
						{	267.0f,	692.0f,	IDC_BUTTON_SYMBOL_EM				},
						{	284.0f,	692.0f,	IDC_BUTTON_SYMBOL_ES				},
						{	301.0f,	692.0f,	IDC_BUTTON_SYMBOL_FR				},
						{	318.0f,	692.0f,	IDC_BUTTON_SYMBOL_HI				},
						{	335.0f,	692.0f,	IDC_BUTTON_SYMBOL_HO				},
						{	353.0f,	692.0f,	IDC_BUTTON_SYMBOL_ID				},
						{	371.0f,	692.0f,	IDC_BUTTON_SYMBOL_IH				},
						{	387.0f,	692.0f,	IDC_BUTTON_SYMBOL_KL				},
						{	404.0f,	692.0f,	IDC_BUTTON_SYMBOL_ME				},
						{	421.0f,	692.0f,	IDC_BUTTON_SYMBOL_PA				},
						{	438.0f,	692.0f,	IDC_BUTTON_SYMBOL_PB				},
						{	455.0f,	692.0f,	IDC_BUTTON_SYMBOL_PI				},
						{	473.0f,	692.0f,	IDC_BUTTON_SYMBOL_PX				},
						{	490.0f,	692.0f,	IDC_BUTTON_SYMBOL_RA				},
						{	507.0f,	692.0f,	IDC_BUTTON_SYMBOL_RP				},
						{	522.0f,	692.0f,	IDC_BUTTON_SYMBOL_TB				},

						{	56.0f,	450.0f,	IDC_BUTTON_EVENTRATION				},
						{	56.0f,	436.0f,	IDC_BUTTON_HIATAL_HERNIA			},
						{	56.0f,	408.0f,	IDC_BUTTON_BRONCHOVASCULAR_MARKINGS	},
						{	56.0f,	395.0f,	IDC_BUTTON_HYPERINFLATION			},
						{	56.0f,	366.0f,	IDC_BUTTON_BONY_CHEST_CAGE			},
						{	56.0f,	353.0f,	IDC_BUTTON_FRACTURE_HEALED			},
						{	56.0f,	340.0f,	IDC_BUTTON_FRACTURE_NONHEALED		},
						{	56.0f,	328.0f,	IDC_BUTTON_SCOLIOSIS				},
						{	56.0f,	313.0f,	IDC_BUTTON_VERTEBRAL_COLUMN			},

						{	367.0f,	445.0f,	IDC_BUTTON_AZYGOS_LOBE				},
						{	367.0f,	432.0f,	IDC_BUTTON_LUNG_DENSITY				},
						{	367.0f,	420.0f,	IDC_BUTTON_INFILTRATE				},
						{	367.0f,	406.0f,	IDC_BUTTON_NODULAR_LESION			},
						{	367.0f,	381.0f,	IDC_BUTTON_FOREIGN_BODY				},
						{	367.0f,	369.0f,	IDC_BUTTON_POSTSURGICAL				},
						{	367.0f,	356.0f,	IDC_BUTTON_CYST						},
						{	367.0f,	330.0f,	IDC_BUTTON_AORTA_ANOMALY			},
						{	367.0f,	317.0f,	IDC_BUTTON_VASCULAR_ABNORMALITY		},

						{	332.0f,	257.0f,	IDC_BUTTON_SEE_PHYSICIAN_YES		},
						{	379.0f,	257.0f,	IDC_BUTTON_SEE_PHYSICIAN_NO			},

						{ 0.0,	0.0,	0										}
					};

					

REPORT_BOX_LOCATION		ReportGeneralPurposeOnlyPage2BoxArray[] =
					{
						{	305.0f,	110.0f,	IDC_BUTTON_A_READER					},
						{	330.0f,	110.0f,	IDC_BUTTON_B_READER					},
						{	381.0f,	110.0f,	IDC_BUTTON_OTHER_READ				},
						{	58.0f,	471.0f,	IDC_BUTTON_EVENTRATION				},
						{	58.0f,	457.0f,	IDC_BUTTON_HIATAL_HERNIA			},
						{	58.0f,	429.0f,	IDC_BUTTON_BRONCHOVASCULAR_MARKINGS	},
						{	58.0f,	416.0f,	IDC_BUTTON_HYPERINFLATION			},
						{	58.0f,	387.0f,	IDC_BUTTON_BONY_CHEST_CAGE			},
						{	58.0f,	374.0f,	IDC_BUTTON_FRACTURE_HEALED			},
						{	58.0f,	361.0f,	IDC_BUTTON_FRACTURE_NONHEALED		},
						{	58.0f,	349.0f,	IDC_BUTTON_SCOLIOSIS				},
						{	58.0f,	334.0f,	IDC_BUTTON_VERTEBRAL_COLUMN			},

						{	369.0f,	466.0f,	IDC_BUTTON_AZYGOS_LOBE				},
						{	369.0f,	453.0f,	IDC_BUTTON_LUNG_DENSITY				},
						{	369.0f,	441.0f,	IDC_BUTTON_INFILTRATE				},
						{	369.0f,	427.0f,	IDC_BUTTON_NODULAR_LESION			},
						{	369.0f,	402.0f,	IDC_BUTTON_FOREIGN_BODY				},
						{	369.0f,	390.0f,	IDC_BUTTON_POSTSURGICAL				},
						{	369.0f,	377.0f,	IDC_BUTTON_CYST						},
						{	369.0f,	351.0f,	IDC_BUTTON_AORTA_ANOMALY			},
						{	369.0f,	338.0f,	IDC_BUTTON_VASCULAR_ABNORMALITY		},
						{ 0.0,	0.0,	0										}
					};


typedef struct
	{
	GLfloat				X;
	GLfloat				Y;
	GLfloat				CharWidth;
	unsigned short		CharCount;
	unsigned short		nFirstCharacter;
	int					ResourceSymbol;
	int					SubfieldID;
	} REPORT_CHAR_FIELD;


REPORT_CHAR_FIELD		ReportPage1GeneralPurposeFieldArray[] =
					{
						{	0.0,	0.0,	0,		0,	0,	0,									0	}
					};

REPORT_CHAR_FIELD		ReportPage2GeneralPurposeFieldArray[] =
					{
						{	0.0,	0.0,	0,		0,	0,	0,									0	}
					};

REPORT_CHAR_FIELD		NIOSHReportPage1FieldArray[] =
					{
						{	36.1f,	728.0f,	14.0f,	2,	0,	IDC_EDIT_DATE_OF_RADIOGRAPH,		1	},
						{	80.0f,	728.0f,	14.0f,	2,	3,	IDC_EDIT_DATE_OF_RADIOGRAPH,		2	},
						{	126.0f,	728.0f,	14.0f,	4,	6,	IDC_EDIT_DATE_OF_RADIOGRAPH,		3	},

						{	55.0f,	144.0f,	16.0f,	9,	0,	IDC_EDIT_READER_ID,					1	},
						{	305.0f,	145.0f,	21.0f,	3,	0,	IDC_EDIT_READER_INITIALS,			1	},

						{	411.0f,	146.0f,	14.5f,	2,	0,	IDC_EDIT_DATE_OF_READING,			1	},
						{	455.6f,	146.0f,	14.5f,	2,	3,	IDC_EDIT_DATE_OF_READING,			2	},
						{	501.2f,	146.0f,	14.5f,	4,	6,	IDC_EDIT_DATE_OF_READING,			3	},

						{	417.0f,	69.0f,	14.0f,	2,	0,	IDC_EDIT_READER_STATE,				1	},
						{	457.7f,	69.0f,	19.0f,	5,	0,	IDC_EDIT_READER_ZIPCODE,			1	},

						{	0.0,	0.0,	0,		0,	0,	0,									0	}
					};


REPORT_CHAR_FIELD		NIOSHReportPage2FieldArray[] =
					{
						{	0.0,	0.0,	0,		0,	0,	0,									0	}
					};



typedef struct
	{
	GLfloat				X;
	GLfloat				Y;
	long				DataStructureOffset;
	} CLIENT_TEXT_FIELD;


CLIENT_TEXT_FIELD	ReportPage1GeneralPurposeClientTextFieldArray[] =
					{
						{	250.0f,	750.0f,		offsetof( CLIENT_INFO, Name )				},
						{	420.0f,	730.0f,		offsetof( CLIENT_INFO, StreetAddress )		},
						{	420.0f,	720.0f,		offsetof( CLIENT_INFO, City )				},
						{	420.0f,	720.0f,		offsetof( CLIENT_INFO, State )				},
						{	420.0f,	720.0f,		offsetof( CLIENT_INFO, ZipCode )			},
						{	420.0f,	710.0f,		offsetof( CLIENT_INFO, Phone )				},
						{	420.0f,	700.0f,		offsetof( CLIENT_INFO, OtherContactInfo )	},
						{	0.0,	0.0,		0											}
					};



typedef struct
	{
	GLfloat				X;
	GLfloat				Y;
	GLfloat				X_EOL;
	int					ResourceSymbol;
	} REPORT_TEXT_FIELD;


REPORT_TEXT_FIELD	ReportPage1GeneralPurposeTextFieldArray[] =
					{
						{	68.0f,	737.0f,	210.0,		IDC_EDIT_REPORT_PATIENT_NAME	},
						{	68.0f,	713.0f,	162.0,		IDC_EDIT_REPORT_DOB				},
						{	68.0f,	689.0f,	162.0,		IDC_EDIT_REPORT_PATIENT_ID		},
						{	81.0f,	667.0f,	162.0,		IDC_EDIT_DATE_OF_RADIOGRAPH		},
						{	0.0,	0.0,	0,			0								}
					};


REPORT_TEXT_FIELD	ReportPage2GeneralPurposeTextFieldArray[] =
					{
						{	43.4f,	166.0f,	210.0,		IDC_EDIT_REPORT_PATIENT_NAME	},
						{	225.3f,	166.0f,	162.0,		IDC_EDIT_REPORT_DOB				},
						{	355.3f,	166.0f,	162.0,		IDC_EDIT_REPORT_PATIENT_ID		},
						{	79.0f,	96.0f,	267.0,		IDC_EDIT_ORDERING_PHYSICIAN_NAME},
						{	73.0f,	122.0f,	267.0,		IDC_EDIT_ORDERING_FACILITY		},
						{	81.0f,	68.0f,	162.0,		IDC_EDIT_DATE_OF_READING		},
						{	85.0f,	45.0f,	267.0,		IDC_EDIT_CLASSIFICATION_PURPOSE	},
						{	400.0f,	113.0f,	481.0,		IDC_EDIT_TYPE_OF_READING_OTHER	},
						{	343.0f,	81.0f,	572.0,		IDC_EDIT_READER_SIGNATURE_NAME	},
						{	0.0,	0.0,	0,			0								}
					};


REPORT_TEXT_FIELD	ReportPage1NIOSHTextFieldArray[] =
					{
						{	39.0f,	632.0f,	297,		IDC_EDIT_REPORT_PATIENT_NAME	},
						{	39.0f,	695.0f,	297,		IDC_EDIT_REPORT_PATIENT_ID		},
						{	320.0f,	111.0f,	562.0,		IDC_EDIT_READER_SIGNATURE_NAME	},
						{	56.7f,	73.0f,	235.0f,		IDC_EDIT_READER_STREET_ADDRESS	},
						{	250.0f,	73.0f,	404.0f,		IDC_EDIT_READER_CITY			},
						{	0.0,	0.0,	0,			0								}
					};


REPORT_TEXT_FIELD	ReportPage2NIOSHTextFieldArray[] =
					{
						{	35.0f,	745.0f,	312,		IDC_EDIT_REPORT_PATIENT_NAME,	},
						{	0.0,	0.0,	0,			0								}
					};



typedef struct
	{
	GLfloat				X;
	GLfloat				Y;
	unsigned short		nCharactersPerLine;
	GLfloat				LineSpacing;
	unsigned short		LineCount;
	int					ResourceSymbol;
	} REPORT_COMMENT_FIELD;


REPORT_COMMENT_FIELD		ReportPage1GeneralPurposeCommentArray[] =
					{
						{	425.0f,	566.0f,	26,		12.3f,	3,	IDC_EDIT_IMAGE_QUALITY_OTHER	},
						{	0.0,	0.0,	0,		0.0,	0,	0								}
					};


REPORT_COMMENT_FIELD	ReportPage2GeneralPurposeCommentArray[] =
					{
						{	74.0f,	300.1f,	105,	15.0f,	7,	IDC_EDIT_OTHER_COMMENTS			},
						{	0.0,	0.0,	0,		0.0,	0,	0								}
					};


REPORT_COMMENT_FIELD		ReportPage1NIOSHCommentArray[] =
					{
						{	425.0f,	566.0f,	26,		12.3f,	3,	IDC_EDIT_IMAGE_QUALITY_OTHER	},
						{	0.0,	0.0,	0,		0.0,	0,	0								}
					};


REPORT_COMMENT_FIELD	ReportPage2NIOSHCommentArray[] =
					{
						{	79.0f,	229.0f,	105,	15.0f,	7,	IDC_EDIT_OTHER_COMMENTS			},
						{	0.0,	0.0,	0,		0.0,	0,	0								}
					};


BOOL CImageView::CreateReportFontGlyphs( HDC hDC, int FontHeight, int FontWidth, int FontWeight, BOOL bItalic, char FontPitch, char* pFontName )
{
	BOOL					bOK;
	GLfloat					ViewportRect[ 4 ];
	GLfloat					ViewportWidth;
	GLfloat					ViewportHeight;
//	GLuint					hShaderProgram;
	CFont					TextFont;
	HGDIOBJ					hSavedFontHandle;
	int						nChar;
	DWORD					GlyphBitmapBufferSize;
	GLYPH_BITMAP_INFO		*pGlyphBitmap;
	GLYPHMETRICS			*pGlyphMetrics;
	MAT2					IdentityMatrix = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
	unsigned int			TextureID;
	GLsizei					GlyphBitmapHeight;
	GLsizei					GlyphBitmapWidth;
	GLfloat					Color[ 3 ] = { 0.0f, 1.0f, 0.0f };		// Paint the annotation characters green.

	CheckOpenGLResultAt( __FILE__, __LINE__	);

	glGetFloatv( GL_VIEWPORT, ViewportRect );
	ViewportWidth = ViewportRect[ 2 ] - ViewportRect[ 0 ];
	ViewportHeight = ViewportRect[ 3 ] - ViewportRect[ 1 ];

	// Create display lists for font character glyphs 0 through 128.
	bOK = ( TextFont.CreateFont(
			FontHeight,						// nHeight in device units.
			FontWidth,						// nWidth - use available aspect ratio
			0,								// nEscapement - make character lines horizontal
			0,								// nOrientation - individual chars are horizontal
			FontWeight,						// nWeight - character stroke thickness
			bItalic,						// bItalic - not italic
			FALSE,							// bUnderline - not underlined
			FALSE,							// cStrikeOut - not a strikeout font
			ANSI_CHARSET,					// nCharSet - normal ansi characters
			OUT_TT_ONLY_PRECIS,				// nOutPrecision - choose font type using default search
			CLIP_DEFAULT_PRECIS,			// nClipPrecision - use default clipping
			PROOF_QUALITY,					// nQuality - best possible appearance
			FontPitch,						// nPitchAndFamily - fixed or variable pitch
			pFontName						// lpszFacename
			) != 0 );

	if ( bOK )
		{
		hSavedFontHandle = ::SelectObject( hDC, (HGDIOBJ)(HFONT)( TextFont.GetSafeHandle() ) );
		if ( hSavedFontHandle != 0 )
			{
			// Generate a sequence of character glyph bitmaps for this font.
			// The Windows glyph bitmaps are DWORD alligned.
			glActiveTexture( TEXTURE_UNIT_REPORT_TEXT );		// Use texture unit 5 for report glyph textures.
			glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

			for ( nChar = 0; nChar < 128; nChar++ )
				{
				// Point to the bitmap array slot for this font character.
				pGlyphBitmap = &m_ReportFontGlyphBitmapArray[ nChar ];
				pGlyphMetrics = &pGlyphBitmap -> GlyphMetrics;
				// Get the buffer size required for this character's bitmap by passing a NULL buffer size and pointer.
				GlyphBitmapBufferSize = GetGlyphOutlineA( hDC, nChar, GGO_GRAY8_BITMAP, pGlyphMetrics, 0, NULL, &IdentityMatrix );
				bOK = ( GlyphBitmapBufferSize != GDI_ERROR );
				if ( bOK )
					{
					pGlyphBitmap -> pBitmapBuffer = (char*)malloc( GlyphBitmapBufferSize );
					pGlyphBitmap -> BufferSizeInBytes = GlyphBitmapBufferSize;
					bOK = (pGlyphBitmap -> pBitmapBuffer != 0 );
					if ( bOK )
						{
						// Retrieve the character bitmap into the buffer at pGlyphBitmapBuffer.  The grayscale value for each pixel ranges from
						// 0 to 63.  The fragment shader will multiply by 4 to bring each pixel to full scale 0 to 255.
						bOK = ( GetGlyphOutlineA( hDC, nChar, GGO_GRAY8_BITMAP, pGlyphMetrics, GlyphBitmapBufferSize,
																			pGlyphBitmap -> pBitmapBuffer, &IdentityMatrix ) != GDI_ERROR );
						}
					if ( bOK )
						{
						// Create a texture containing the current font character.
						GlyphBitmapWidth = pGlyphMetrics -> gmBlackBoxX;
						GlyphBitmapHeight = pGlyphMetrics -> gmBlackBoxY;
						glGenTextures( 1, &TextureID );
						glBindTexture( GL_TEXTURE_2D, TextureID );

						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

						glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, GlyphBitmapWidth, GlyphBitmapHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pGlyphBitmap -> pBitmapBuffer );
						bOK = CheckOpenGLResultAt( __FILE__, __LINE__	);

						// Save the texture ID associated with this character glyph.  It will need to be deleted on program exit.
						pGlyphBitmap -> TextureID = TextureID;

						// Free the bitmap buffer.  The character bitmap is now stored as a texture in the GPU memory.
						free( pGlyphBitmap -> pBitmapBuffer );
						pGlyphBitmap -> pBitmapBuffer = 0;
						pGlyphBitmap -> BufferSizeInBytes = 0;

						CheckOpenGLResultAt( __FILE__, __LINE__	);
						}
					}
				}
			::SelectObject( hDC, hSavedFontHandle );
			TextFont.DeleteObject();
			}
		}

	glActiveTexture( TEXTURE_UNIT_DEFAULT );

	return bOK;
}


void CImageView::DeleteReportFontGlyphs()
{
	int						nChar;
	GLYPH_BITMAP_INFO		*pGlyphBitmap;

	glActiveTexture( TEXTURE_UNIT_REPORT_TEXT );
	for ( nChar = 0; nChar < 128; nChar++ )
		{
		pGlyphBitmap = &m_ReportFontGlyphBitmapArray[ nChar ];
		if ( pGlyphBitmap -> TextureID != 0 )
			{
			glDeleteTextures( 1, (GLuint*)&pGlyphBitmap -> TextureID );
			pGlyphBitmap -> TextureID = 0;
			}
		}
	glActiveTexture( TEXTURE_UNIT_DEFAULT );
}


void CImageView::CreateSignatureTexture()
{
	unsigned int			TextureID;
	GLsizei					SignatureBitmapHeight;
	GLsizei					SignatureBitmapWidth;
	SIGNATURE_BITMAP		*pSignatureBitmap;

	pSignatureBitmap = pBViewerCustomization -> m_ReaderInfo.pSignatureBitmap;
	if ( pSignatureBitmap != 0 )
		{
		glActiveTexture( TEXTURE_UNIT_REPORT_SIGNATURE );
		SignatureBitmapWidth = pSignatureBitmap -> WidthInPixels;
		SignatureBitmapHeight = pSignatureBitmap -> HeightInPixels;

		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		glGenTextures( 1, &TextureID );
		glBindTexture( GL_TEXTURE_2D, TextureID );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, SignatureBitmapWidth, SignatureBitmapHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pSignatureBitmap -> pImageData );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		pSignatureBitmap -> TextureID = TextureID;
		CheckOpenGLResultAt( __FILE__, __LINE__	);
		glActiveTexture( TEXTURE_UNIT_DEFAULT );
		}
}


void CImageView::RenderSignatureTexture( GLuint hShaderProgram, SIGNATURE_BITMAP *pSignatureBitmap,
											unsigned int VertexBufferID, unsigned int VertexAttributesID, float x, float y, float ScaledBitmapWidth, float ScaledBitmapHeight )
{
	GLfloat					ViewportRect[ 4 ];
	GLfloat					XMin, XMax;
	GLfloat					YMin, YMax;
	GLfloat					ViewportWidth;
	GLfloat					ViewportHeight;

	glGetFloatv( GL_VIEWPORT, ViewportRect );
	ViewportWidth = ViewportRect[ 2 ] - ViewportRect[ 0 ];
	ViewportHeight = ViewportRect[ 3 ] - ViewportRect[ 1 ];
	glUseProgram( hShaderProgram );
	CheckOpenGLResultAt( __FILE__, __LINE__ );
	glBindVertexArray( VertexAttributesID );

	// Set up the vertex buffer for the signature bitmap.
	// Adjust cell boundarys to mornalize to -1.0 < x , 1.0, -1.0 < y , 1.0.

	XMin = 2.0f * x / (GLfloat)ViewportWidth - 1.0f;
	YMin = 2.0f * y / (GLfloat)ViewportHeight - 1.0f;
	XMax = 2.0f * ( x + ScaledBitmapWidth / 2.0f ) / (GLfloat)ViewportWidth - 1.0f;
	YMax = 2.0f * ( y + ScaledBitmapHeight / 2.0f ) / (GLfloat)ViewportHeight - 1.0f;

	InitCharacterGlyphVertexRectangle( XMin, XMax, YMin, YMax );		// Invert the Y's.

	// Bind the texture for the signature bitmap.  Indicate to the shader that we're using texture unit TEXTURE_UNIT_REPORT_SIGNATURE.
	glUniform1i( glGetUniformLocation(  hShaderProgram, "ReportSignatureTexture" ), TEXUNIT_NUMBER_REPORT_SIGNATURE );

	glBindTexture( GL_TEXTURE_2D, pSignatureBitmap -> TextureID );
	// Bind the externally declared vertex buffer.
	glBindBuffer( GL_ARRAY_BUFFER, VertexBufferID );
	// Associate the current vertex array just specified with the OpenGL array buffer.
	glBufferData( GL_ARRAY_BUFFER, sizeof( m_CharacterGlyphVertexRectangle ), &m_CharacterGlyphVertexRectangle, GL_STREAM_DRAW );
	CheckOpenGLResultAt( __FILE__, __LINE__ );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	// Render the bitmap.
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
	CheckOpenGLResultAt( __FILE__, __LINE__ );
}


void CImageView::CreateReportTextVertices( GLuint hShaderProgram )
{
	GLfloat					ViewportRect[ 4 ];
	GLfloat					ViewportWidth;
	GLfloat					ViewportHeight;
	CFont					TextFont;

	if ( m_pAssignedDiagnosticImage != 0 )
		{
		glGetFloatv( GL_VIEWPORT, ViewportRect );
		ViewportWidth = ViewportRect[ 2 ] - ViewportRect[ 0 ];
		ViewportHeight = ViewportRect[ 3 ] - ViewportRect[ 1 ];

		glUseProgram( hShaderProgram );
			
		glGenBuffers( 1, &m_ReportVertexBufferID );
		glBindBuffer( GL_ARRAY_BUFFER, m_ReportVertexBufferID );

		glGenVertexArrays( 1, &m_ReportVertexAttributesID );
		// Bind the Vertex Array Object first, then bind and set the vertex buffer.  Then configure vertex attributes(s).
		glBindVertexArray( m_ReportVertexAttributesID );

		// Set up and enable the vertex attribute array at location 0 in the vertex shader.  This array specifies the vertex positions.
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0 );
		glEnableVertexAttribArray( 0 );		// ( location = 0 )

		// Set up and enable the vertex attribute array at location 1 in the vertex shader.  This array specifies the texture vertex positions.
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (GLvoid*)( 12 * sizeof(float) ) );
		glEnableVertexAttribArray( 1 );		// ( location = 1 )
		CheckOpenGLResultAt( __FILE__, __LINE__	);

		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glBindVertexArray( 0 );

		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		CheckOpenGLResultAt( __FILE__, __LINE__	);
		}
}


void CImageView::DeleteReportTextVertices( GLuint hShaderProgram )
{
	glUseProgram( hShaderProgram );
	glDeleteVertexArrays( 1, &m_ReportVertexAttributesID );
	glDeleteBuffers( 1, &m_ReportVertexBufferID );
	glDisable( GL_BLEND );
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture( TEXTURE_UNIT_DEFAULT );
	glUseProgram( 0 );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
}


void CImageView::RenderReportCheckmark()
{
	GLfloat					ViewportRect[ 4 ];
	GLfloat					ViewportWidth;
	GLfloat					ViewportHeight;
	GLuint					hShaderProgram;
	CFont					TextFont;
	unsigned int			VertexBufferID;
	unsigned int			VertexAttributesID;
	GLfloat					Color[ 3 ] = { 0.0f, 0.0f, 0.5f };

	CheckOpenGLResultAt( __FILE__, __LINE__	);
	glGetFloatv( GL_VIEWPORT, ViewportRect );

	ViewportWidth = ViewportRect[ 2 ] - ViewportRect[ 0 ];
	ViewportHeight = ViewportRect[ 3 ] - ViewportRect[ 1 ];

	hShaderProgram = m_gLineDrawingShaderProgram;
	glUseProgram( hShaderProgram );
			
	glGenBuffers( 1, &VertexBufferID );
	glBindBuffer( GL_ARRAY_BUFFER, VertexBufferID );

	glGenVertexArrays( 1, &VertexAttributesID );
	// Bind the Vertex Array Object first, then bind and set the vertex buffer.  Then configure vertex attributes(s).
	glBindVertexArray( VertexAttributesID );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	// Set up and enable the vertex attribute array at location 0 in the vertex shader.  This array specifies the vertex positions.
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0 );
	glEnableVertexAttribArray( 0 );		// ( location = 0 )
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	glUniform3f( glGetUniformLocation( hShaderProgram, "DrawingColor"), Color[ 0 ], Color[ 1 ], Color[ 2 ] );
	CheckOpenGLResultAt( __FILE__, __LINE__ );

	// Associate the current vertex array just specified with the OpenGL array buffer.
	glBufferData( GL_ARRAY_BUFFER, sizeof( m_XMarkVertexArray ), &m_XMarkVertexArray, GL_STREAM_DRAW );
	CheckOpenGLResultAt( __FILE__, __LINE__ );

	// Render the four vertices for the line drawings that create the forward slash of the "X" mark.
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

	// Render the four vertices for the line drawings that create the backward slash of the "X" mark.
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)( 8 * sizeof(float) ) );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

	glDeleteVertexArrays( 1, &VertexAttributesID );
	glDeleteBuffers( 1, &VertexBufferID );

	glUseProgram( 0 );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
}


void CImageView::RenderReport( HDC hDC, unsigned long ImageDestination )
{
	GLfloat					ViewportRect[ 4 ];
	GLfloat					ViewportWidth;
	GLfloat					ViewportHeight;
	GLuint					hShaderProgram;
	GLfloat					Color[ 3 ];
	GLfloat					x, y;
	float					MarkDx;
	float					MarkDy;
	GLfloat					TranslationX;
	GLfloat					TranslationY;
	GLfloat					BaseScale;
	GLfloat					BaseScale2;
	GLfloat					ImageOriginX;
	GLfloat					ImageOriginY;
	GLfloat					ImageWidthInPixels;
	GLfloat					ImageHeightInPixels;
	GLfloat					CheckX;
	GLfloat					CheckY;
	GLfloat					XLineWidth;
	GLfloat					CharPosX;
	GLfloat					CharPosY;
	int						nBox;
	REPORT_BOX_LOCATION		*pBoxLocationInfo;
	int						nField;
	REPORT_CHAR_FIELD		*pFieldLocationInfo;
	REPORT_TEXT_FIELD		*pTextFieldLocationInfo;
	CLIENT_TEXT_FIELD		*pClientFieldLocationInfo;
	char					*pDataStructure;
	char					*pDataFieldValue;
	size_t					TextLength;
	char					TextField[ 256 ];
	char					TextLine[ 256 ];
	char					TextChar[ 2 ];
	char					*pTextField;
	char					*pStartOfLine;
	char					*pChar;
	int						nChar;
	int						nSkippedChars;
	int						nLine;
	int						nCharactersRemaining;
	int						nCharactersToDisplay;
	int						nLastCharacterInLine;
	CStudy					*pCurrentStudy;
	REPORT_COMMENT_FIELD	*pCommentFieldLocationInfo;
	CFont					TextFont;
	CFont					BoldFont;
	CFont					SmallFont;
	CFont					SmallItallicFont;
	char					SelectedFont;
								#define		NO_FONT_SELECTED			0
								#define		SELECT_BOLD_FONT			1
								#define		SELECT_SMALL_FONT			2
								#define		SELECT_SMALL_ITALIC_FONT	3
	char					PrevSelectedFont;
	bool					bFontOk;
	int						FontHeight;
	BOOL					bConfigurablePart;
	SIGNATURE_BITMAP		*pSignatureBitmap;
	float					ScaledBitmapWidth, ScaledBitmapHeight;

	CheckOpenGLResultAt( __FILE__, __LINE__ );

	glGetFloatv( GL_VIEWPORT, ViewportRect );
	ViewportWidth = ViewportRect[ 2 ] - ViewportRect[ 0 ];
	ViewportHeight = ViewportRect[ 3 ] - ViewportRect[ 1 ];
	BaseScale = (GLfloat)m_pAssignedDiagnosticImage -> m_ScaleFactor;
	ImageWidthInPixels = (GLfloat)m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
	ImageHeightInPixels = (GLfloat)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;

	if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_REPORT && m_pAssignedDiagnosticImage != 0 )
		{
		pCurrentStudy = ThisBViewerApp.m_pCurrentStudy;
		if ( pCurrentStudy != 0  )
			{
			// Set drawing color.
//			glColor3f( 0.0f, 0.0f, 0.5f );
			Color[ 0 ] = 0.0f;
			Color[ 1 ] = 0.0f;
			Color[ 2 ] = 0.5f;
			if ( ImageDestination == IMAGE_DESTINATION_WINDOW )
				{
				// Set scaling.
				glGetFloatv( GL_VIEWPORT, ViewportRect );

				ImageOriginX = ( ViewportWidth - ImageWidthInPixels * BaseScale ) / 2.0f;
				ImageOriginY = ( ViewportHeight - ImageHeightInPixels * BaseScale ) / 2.0f;
	
				TranslationX = ( ImageWidthInPixels / 2.0f - (GLfloat)m_pAssignedDiagnosticImage -> m_FocalPoint.x ) * BaseScale;
				TranslationY = - ( ImageHeightInPixels / 2.0f - (GLfloat)m_pAssignedDiagnosticImage -> m_FocalPoint.y ) * BaseScale;

				BaseScale2 = 2.78f * BaseScale;			// Adjust for the report form scale used originally for specifying character and line overlay positions.
				FontHeight = (int)( 1.0f * 28.0f * (GLfloat)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels * m_pAssignedDiagnosticImage -> m_ScaleFactor / 1000.0f );
				}
			else if ( ImageDestination == IMAGE_DESTINATION_FILE || ImageDestination == IMAGE_DESTINATION_PRINTER )
				{
				// Saving and printing render the full-scale image.
				BaseScale2 = 2.78f;

				ImageOriginX = 0.0f;
				ImageOriginY = 0.0f;
	
				TranslationX = 0.0f;
				TranslationY = 0.0f;

				FontHeight = (int)( 1.0f * 28.0f * (GLfloat)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels / 1000.0f );
				}

			XLineWidth = 8 * BaseScale2 / ViewportWidth;
	CheckOpenGLResultAt( __FILE__, __LINE__ );

			MarkDx = 20.14f * BaseScale2 / ViewportWidth;
			MarkDy = 20.14f * BaseScale2 / ViewportHeight;

			if ( m_PageNumber == 2 )
				{
				MarkDx = 14.03f * BaseScale2 / ViewportWidth;
				MarkDy = 14.03f * BaseScale2 / ViewportHeight;
				}
			// Render checkmarks in the appropriate report boxes.
			nBox = 0;
			bConfigurablePart = TRUE;
	CheckOpenGLResultAt( __FILE__, __LINE__ );

			// For page 1 of the report form, first do the part dependent on the interpretation environment
			// (in other words, dependent on the type of report form being used), then do the interpretation part.
			do
				{
				if ( m_PageNumber == 1 )
					{
					if ( bConfigurablePart )
						{
						if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
							pBoxLocationInfo = &ReportGeneralPurposeOnlyPage1BoxArray[ nBox ];
						else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
							pBoxLocationInfo = &ReportNIOSHOnlyPage1BoxArray[ nBox ];
						}
					else
						pBoxLocationInfo = &ReportPage1BoxArray[ nBox ];
					}
				else if ( m_PageNumber == 2 )
					{
					if ( bConfigurablePart )
						{
						if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
							pBoxLocationInfo = &ReportGeneralPurposeOnlyPage2BoxArray[ nBox ];
						else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
							pBoxLocationInfo = &ReportNIOSHOnlyPage2BoxArray[ nBox ];
						}
					}

				nBox++;
				if ( pBoxLocationInfo -> ResourceSymbol == 0 && bConfigurablePart )
					{
					bConfigurablePart = FALSE;
					nBox = 0;
					}
				else if ( pCurrentStudy -> StudyButtonWasOn( pBoxLocationInfo -> ResourceSymbol ) )
					{
					// Render a check mark in this report box.
					CheckX = BaseScale2 * pBoxLocationInfo -> BoxX + ImageOriginX + TranslationX;
					CheckY = BaseScale2 * pBoxLocationInfo -> BoxY + ImageOriginY + TranslationY;
	
					x = 2.0f * CheckX / (GLfloat)ViewportWidth - 1.0f;
					y = 2.0f * CheckY / (GLfloat)ViewportHeight - 1.0f;

					// Render each line (forward slash and backward slash) of the "X" as a quad, since
					// there is no OpenGL line width function (although one may be present for some graphics cards.

					m_XMarkVertexArray.FwdSlashXbl = x;
					m_XMarkVertexArray.FwdSlashYbl = y;
					m_XMarkVertexArray.FwdSlashXbr = x + XLineWidth;
					m_XMarkVertexArray.FwdSlashYbr = y;

					m_XMarkVertexArray.FwdSlashXtl = x + MarkDx;
					m_XMarkVertexArray.FwdSlashYtl = y + MarkDy;
					m_XMarkVertexArray.FwdSlashXtr = x + MarkDx + XLineWidth;
					m_XMarkVertexArray.FwdSlashYtr = y + MarkDy;

					m_XMarkVertexArray.BkwdSlashXbl = x + MarkDx;
					m_XMarkVertexArray.BkwdSlashYbl = y;
					m_XMarkVertexArray.BkwdSlashXbr = x + MarkDx + XLineWidth;
					m_XMarkVertexArray.BkwdSlashYbr = y;

					m_XMarkVertexArray.BkwdSlashXtl = x;
					m_XMarkVertexArray.BkwdSlashYtl = y + MarkDy;
					m_XMarkVertexArray.BkwdSlashXtr = x + XLineWidth;
					m_XMarkVertexArray.BkwdSlashYtr = y + MarkDy;

					RenderReportCheckmark();
	CheckOpenGLResultAt( __FILE__, __LINE__ );
					}
				}
			while ( pBoxLocationInfo -> ResourceSymbol != 0 || nBox == 0 );

			// Now that the checkmarks are rendered, move on to render the text fields.

			CheckOpenGLResultAt( __FILE__, __LINE__ );
			// Render the date string characters (on the NIOSH report form).
			hShaderProgram = m_gReportTextShaderProgram;

			bFontOk = CreateReportFontGlyphs( hDC, -FontHeight, 0, FW_SEMIBOLD, FALSE, FIXED_PITCH, "Dontcare" );
			CheckOpenGLResultAt( __FILE__, __LINE__	);

			if ( bFontOk )
				{
				CreateReportTextVertices( hShaderProgram );
				nField = -1;
				do
					{
					nField++;
					if ( m_PageNumber == 1 )
						{
						if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
							pFieldLocationInfo = &ReportPage1GeneralPurposeFieldArray[ nField ];
						else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
							pFieldLocationInfo = &NIOSHReportPage1FieldArray[ nField ];
						}
					else if ( m_PageNumber == 2 )
						{
						if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
							pFieldLocationInfo = &ReportPage2GeneralPurposeFieldArray[ nField ];
						else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
							pFieldLocationInfo = &NIOSHReportPage2FieldArray[ nField ];
						}
					pCurrentStudy -> GetStudyEditField( pFieldLocationInfo -> ResourceSymbol, TextField );
					TextLength = pFieldLocationInfo -> CharCount;
					if ( TextLength > 0 )
						{
						if ( strlen( TextField ) - pFieldLocationInfo -> nFirstCharacter + 1  < TextLength )
							TextLength = strlen( TextField ) - pFieldLocationInfo -> nFirstCharacter + 1;

						for ( nChar = 0; nChar < (int)TextLength; nChar++ )
							{
							CharPosX = BaseScale2 * ( pFieldLocationInfo -> X +  ( nChar * pFieldLocationInfo -> CharWidth ) ) + ImageOriginX + TranslationX;
							CharPosY = BaseScale2 *  pFieldLocationInfo -> Y + ImageOriginY + TranslationY;
							TextChar[ 0 ] = TextField[ pFieldLocationInfo -> nFirstCharacter + nChar ];
							TextChar[ 1 ] = '\0';
							// Render each character as an individual string to properly fit it into the rectangular box on the report form.
							RenderReportTextString( hShaderProgram, m_ReportFontGlyphBitmapArray, TextChar,
														m_ReportVertexBufferID, m_ReportVertexAttributesID, CharPosX, CharPosY, Color );
							}
						}
					CheckOpenGLResultAt( __FILE__, __LINE__	);
					}
				while ( pFieldLocationInfo -> ResourceSymbol != 0 );

				DeleteReportTextVertices( hShaderProgram );
				DeleteReportFontGlyphs();
				}
			CheckOpenGLResultAt( __FILE__, __LINE__ );

			// Similarly, render the comment text fields.

			// Create display lists for comment font character glyphs 0 through 128.
			bFontOk = CreateReportFontGlyphs( hDC, -FontHeight / 2, 0, FW_MEDIUM, FALSE, DEFAULT_PITCH, "Roman" );
			CheckOpenGLResultAt( __FILE__, __LINE__	);

			if ( bFontOk )
				{
				CreateReportTextVertices( hShaderProgram );
				nField = -1;
				do
					{
					nField++;
					if ( m_PageNumber == 1 )
						{
						if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
							pCommentFieldLocationInfo = &ReportPage1GeneralPurposeCommentArray[ nField ];
						else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
							pCommentFieldLocationInfo = &ReportPage1NIOSHCommentArray[ nField ];
						}
					else if ( m_PageNumber == 2 )
						{
						if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
							pCommentFieldLocationInfo = &ReportPage2GeneralPurposeCommentArray[ nField ];
						else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
							pCommentFieldLocationInfo = &ReportPage2NIOSHCommentArray[ nField ];
						}
					pTextField = pCurrentStudy -> GetStudyCommentField( pCommentFieldLocationInfo -> ResourceSymbol );

					if ( pTextField != 0 )
						{				
						TextLength = strlen( pTextField );
						nCharactersRemaining = (int)TextLength;
						nSkippedChars = 0;		// Used to count end-of-line characters, which need to be skipped.
						for ( nLine = 0; nLine < pCommentFieldLocationInfo -> LineCount && nCharactersRemaining > 0; nLine++ )
							{
							nCharactersToDisplay = pCommentFieldLocationInfo -> nCharactersPerLine;

							pStartOfLine = &pTextField[ TextLength - nCharactersRemaining ];
							pChar = strchr( pStartOfLine, 0x0d );	// Look for embedded end of line.

							if ( pChar != 0 && (LONG_PTR)( pChar - pStartOfLine ) <= nCharactersToDisplay )
								{
								nCharactersToDisplay = (int)( (LONG_PTR)( pChar - pStartOfLine ) );
								nSkippedChars = 2;
								}
							else if ( nCharactersRemaining > nCharactersToDisplay )
								{
								nSkippedChars = 0;
								nLastCharacterInLine = (int)TextLength - nCharactersRemaining + nCharactersToDisplay - 1;
								nChar = pCommentFieldLocationInfo -> nCharactersPerLine / 4;
								while ( nChar > 0 && pTextField[ nLastCharacterInLine ] != ' ' )
									{
									nChar--;
									nLastCharacterInLine--;
									nCharactersToDisplay--;
									}
								}
							else
								nSkippedChars = 0;
							if ( nCharactersRemaining < nCharactersToDisplay )
								nCharactersToDisplay = nCharactersRemaining;
							strcpy( TextLine, "" );
							strncat( TextLine, pStartOfLine, nCharactersToDisplay );

							CharPosX = BaseScale2 * ( pCommentFieldLocationInfo -> X ) + ImageOriginX + TranslationX;
							CharPosY = BaseScale2 * ( pCommentFieldLocationInfo -> Y - ( nLine * pCommentFieldLocationInfo -> LineSpacing ) ) + ImageOriginY + TranslationY;

							RenderReportTextString( hShaderProgram, m_ReportFontGlyphBitmapArray, TextLine,
																					m_ReportVertexBufferID, m_ReportVertexAttributesID, CharPosX, CharPosY, Color );
							CheckOpenGLResultAt( __FILE__, __LINE__	);
							nCharactersRemaining -= nCharactersToDisplay + nSkippedChars;
							}
						}
					}
				while ( pCommentFieldLocationInfo -> ResourceSymbol != 0 );
				}

			// Similarly, render the required text fields.
			if ( bFontOk )
				{
				nField = -1;
				do
					{
					nField++;
					if ( m_PageNumber == 1 )
						{
						if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
							pTextFieldLocationInfo = &ReportPage1GeneralPurposeTextFieldArray[ nField ];
						else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
							pTextFieldLocationInfo = &ReportPage1NIOSHTextFieldArray[ nField ];
						}
					else if ( m_PageNumber == 2 )
						{
						if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
							pTextFieldLocationInfo = &ReportPage2GeneralPurposeTextFieldArray[ nField ];
						else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
							pTextFieldLocationInfo = &ReportPage2NIOSHTextFieldArray[ nField ];
						}
					if ( pTextFieldLocationInfo -> ResourceSymbol != 0 )
						{
						pCurrentStudy -> GetStudyEditField( pTextFieldLocationInfo -> ResourceSymbol, TextField );
						TextLength = strlen( TextField );

						CharPosX = BaseScale2 * (  pTextFieldLocationInfo -> X ) + ImageOriginX + TranslationX;
						CharPosY = BaseScale2 * (  pTextFieldLocationInfo -> Y ) + ImageOriginY + TranslationY;

						RenderReportTextString( hShaderProgram, m_ReportFontGlyphBitmapArray, TextField,
																				m_ReportVertexBufferID, m_ReportVertexAttributesID, CharPosX, CharPosY, Color );
						CheckOpenGLResultAt( __FILE__, __LINE__	);
						}
					}
				while ( pTextFieldLocationInfo -> ResourceSymbol != 0 );
				DeleteReportTextVertices( hShaderProgram );
				DeleteReportFontGlyphs();
				CheckOpenGLResultAt( __FILE__, __LINE__	);

				if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
					{
					// Render the client heading information.
					if ( m_PageNumber == 1 && p_CurrentClientInfo != 0 && _stricmp( p_CurrentClientInfo -> Name, "None" ) != 0 )
						{

						pDataStructure = (char*)p_CurrentClientInfo;
						nField = -1;
						// Set drawing color.
//						glColor3f( 0.0f, 0.0f, 0.0f );
						PrevSelectedFont = NO_FONT_SELECTED;
						do
							{
							nField++;
							pClientFieldLocationInfo = &ReportPage1GeneralPurposeClientTextFieldArray[ nField ];
							if ( pClientFieldLocationInfo -> X != 0.0 )
								{
								pDataFieldValue = (char*)( pDataStructure + pClientFieldLocationInfo -> DataStructureOffset );
								switch ( nField )
									{
									case 0:					// Client name field.
										strcpy( TextField, "" );
										strncat( TextField, pDataFieldValue, 256 - 1 );
										SelectedFont = SELECT_BOLD_FONT;
										break;
									case 1:					// Street address field.
										strcpy( TextField, "" );
										strncat( TextField, pDataFieldValue, 256 - 1 );
										if ( bFontOk )		// Switch to the small font for the client address fields.
											{
											::SelectObject( hDC, (HGDIOBJ)(HFONT)( SmallFont.GetSafeHandle() ) );
											}
										SelectedFont = SELECT_SMALL_FONT;
										break;
									case 3:					// State field.
										strncat( TextField, ", ", 256 - strlen( TextField ) - 1 );
										strncat( TextField, pDataFieldValue, 256 - strlen( TextField ) - 1 );
										SelectedFont = SELECT_SMALL_FONT;
										break;
									case 4:					// Zip code field.
										strncat( TextField, "     ", 256 - strlen( TextField ) - 1 );
										strncat( TextField, pDataFieldValue, 256 - strlen( TextField ) - 1 );
										SelectedFont = SELECT_SMALL_FONT;
										break;
									case 6:					// Other contact info field.
										strcpy( TextField, "" );
										strncat( TextField, pDataFieldValue, 256 - 1 );
										if ( bFontOk )		// Switch to the small font for the client address fields.
											{
											::SelectObject( hDC, (HGDIOBJ)(HFONT)( SmallItallicFont.GetSafeHandle() ) );
											}
										SelectedFont = SELECT_SMALL_ITALIC_FONT;
										break;
									default:
										strcpy( TextField, "" );
										strncat( TextField, pDataFieldValue, 256 - 1 );
										SelectedFont = SELECT_SMALL_FONT;
										break;
									}
								TextLength = strlen( TextField );
								if ( SelectedFont != PrevSelectedFont )
									{
									CheckOpenGLResultAt( __FILE__, __LINE__	);
									if ( PrevSelectedFont != NO_FONT_SELECTED )
										DeleteReportFontGlyphs();
									CheckOpenGLResultAt( __FILE__, __LINE__	);
									switch ( SelectedFont )
										{
										case SELECT_BOLD_FONT:
											bFontOk = CreateReportFontGlyphs( hDC, -2 * FontHeight / 3, 0, FW_BOLD, FALSE, DEFAULT_PITCH, "Roman" );
											break;
										case SELECT_SMALL_FONT:
											bFontOk = CreateReportFontGlyphs( hDC, FontHeight / 2, 2 * FontHeight / 10, FW_MEDIUM, FALSE, DEFAULT_PITCH, "Roman" );
											break;
										case SELECT_SMALL_ITALIC_FONT:
											bFontOk = CreateReportFontGlyphs( hDC, FontHeight / 2, 15 * FontHeight / 80, FW_MEDIUM, TRUE, DEFAULT_PITCH, "Roman" );
											break;
										}
									CheckOpenGLResultAt( __FILE__, __LINE__	);
									}

								if ( bFontOk )
									{
									CreateReportTextVertices( hShaderProgram );

									CharPosX = BaseScale2 * (   pClientFieldLocationInfo -> X ) + ImageOriginX + TranslationX;
									CharPosY = BaseScale2 * (   pClientFieldLocationInfo -> Y ) + ImageOriginY + TranslationY;

									RenderReportTextString( hShaderProgram, m_ReportFontGlyphBitmapArray, TextField,
																							m_ReportVertexBufferID, m_ReportVertexAttributesID, CharPosX, CharPosY, Color );
									DeleteReportTextVertices( hShaderProgram );
									}
								CheckOpenGLResultAt( __FILE__, __LINE__	);
								PrevSelectedFont = SelectedFont;
								}
							}
						while ( pClientFieldLocationInfo -> X != 0.0 );
						// Restore drawing color.
						CheckOpenGLResultAt( __FILE__, __LINE__	);
						CheckOpenGLResultAt( __FILE__, __LINE__	);
						DeleteReportFontGlyphs();
						CheckOpenGLResultAt( __FILE__, __LINE__	);
						}
					}

				CheckOpenGLResultAt( __FILE__, __LINE__ );
				// Render the reader's digital signature.
				pSignatureBitmap = pBViewerCustomization -> m_ReaderInfo.pSignatureBitmap;
				if ( pSignatureBitmap != 0 && ( 
							( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL && m_PageNumber == 2 ) ||
							  BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_GENERAL && m_PageNumber == 1 ) )
					{
					hShaderProgram = m_gReportSignatureShaderProgram;
					glActiveTexture( TEXTURE_UNIT_REPORT_SIGNATURE );

					CreateReportTextVertices( hShaderProgram );

					if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
						{
						CharPosX = BaseScale2 * 330.0f + ImageOriginX + TranslationX;
						CharPosY = BaseScale2 * 39.0f + ImageOriginY + TranslationY;

						}
					else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
						{
						CharPosX = BaseScale2 * 53.0f + ImageOriginX + TranslationX;
						CharPosY = BaseScale2 * 103.0f + ImageOriginY + TranslationY;

						}
					if ( pSignatureBitmap -> pImageData != 0 && pSignatureBitmap -> BitsPerPixel == 24 )
						{
						ScaledBitmapWidth = 0.306f * BaseScale2 * pSignatureBitmap -> WidthInPixels;
						ScaledBitmapHeight = 0.306f *BaseScale2 * pSignatureBitmap -> HeightInPixels;

						RenderSignatureTexture( hShaderProgram, pSignatureBitmap, m_ReportVertexBufferID, m_ReportVertexAttributesID,
																			CharPosX, CharPosY, ScaledBitmapWidth, ScaledBitmapHeight );
						}
					CheckOpenGLResultAt( __FILE__, __LINE__	);

					DeleteReportTextVertices( hShaderProgram );
					glActiveTexture( TEXTURE_UNIT_DEFAULT );
					}
				TextFont.DeleteObject();
				}
			}
		}
	CheckOpenGLResultAt( __FILE__, __LINE__ );
}


// This function creates a full-scale texture to store the report form image for printing or saving to a file.
unsigned int CImageView::CreateReportFormTexture()
{
	unsigned int			TextureID;
	GLsizei					ReportFormBitmapHeight;
	GLsizei					ReportFormBitmapWidth;

	ReportFormBitmapWidth = m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
	ReportFormBitmapHeight = m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;

	glGenTextures( 1, &TextureID );
	glBindTexture( GL_TEXTURE_2D, TextureID );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, ReportFormBitmapWidth, ReportFormBitmapHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pAssignedDiagnosticImage -> m_pImageData );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	return TextureID;
}


// Prepare a framebuffer object for receiving the report form image in the form of a texture.
// This framebuffer will be used to render the unscaled report overlays for printing or saving to a file.
BOOL CImageView::InitializReportFormFrameBuffer()
{
	BOOL			bNoError = TRUE;
	GLenum			FrameBufferCompleteness;
	GLsizei			ViewportRect[ 4 ];
	GLsizei			ViewportWidth;
	GLsizei			ViewportHeight;

	glGetIntegerv( GL_VIEWPORT, ViewportRect );
	ViewportWidth = ViewportRect[ 2 ] - ViewportRect[ 0 ];
	ViewportHeight = ViewportRect[ 3 ] - ViewportRect[ 1 ];

	DeleteReportImage();			// Deallocate the framebuffer and renderbuffer from the previous saved or printed report image.
	glGenFramebuffers( 1, &m_ReportFormFrameBufferID );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	// Bind the FBO to the current rendering context as the rendering buffer.
	glBindFramebuffer( GL_FRAMEBUFFER, m_ReportFormFrameBufferID );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	// Create a RenderBuffer to receive the full-sized report form image.
	glGenRenderbuffers( 1, &m_ReportFormRenderBufferID );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	glBindRenderbuffer( GL_RENDERBUFFER, m_ReportFormRenderBufferID );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	glRenderbufferStorage( GL_RENDERBUFFER, GL_RGB, ViewportWidth, ViewportHeight );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_ReportFormRenderBufferID );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	FrameBufferCompleteness = glCheckFramebufferStatus( GL_DRAW_FRAMEBUFFER );
	if ( FrameBufferCompleteness != GL_FRAMEBUFFER_COMPLETE )
		{
		CheckOpenGLResultAt( __FILE__, __LINE__	);
		LogMessage( "*** Error:  Incomplete Draw Frame Buffer Object", MESSAGE_TYPE_SUPPLEMENTARY );
		bNoError = FALSE;
		}
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	return bNoError;
}


// This function renders a full-scale report image for printing or saving to a file.
void CImageView::RenderReportFormTexture( GLuint hShaderProgram, unsigned int TextureID,
											unsigned int VertexBufferID, unsigned int VertexAttributesID, float x, float y, float BitmapWidth, float BitmapHeight )
{
	GLfloat					ViewportRect[ 4 ];
	GLfloat					XMin, XMax;
	GLfloat					YMin, YMax;
	GLfloat					ViewportWidth;
	GLfloat					ViewportHeight;

	glGetFloatv( GL_VIEWPORT, ViewportRect );
	ViewportWidth = ViewportRect[ 2 ] - ViewportRect[ 0 ];
	ViewportHeight = ViewportRect[ 3 ] - ViewportRect[ 1 ];
	glUseProgram( hShaderProgram );
	CheckOpenGLResultAt( __FILE__, __LINE__ );
	glBindVertexArray( VertexAttributesID );

	// Set up the vertex buffer for the current character.
	// Adjust cell boundarys to mornalize to -1.0 < x , 1.0, -1.0 < y , 1.0.
	XMin = 2.0f * x / (GLfloat)ViewportWidth - 1.0f;
	YMin = 2.0f * y / (GLfloat)ViewportHeight - 1.0f;
	XMax = 2.0f * ( x + BitmapWidth / 2.0f ) / (GLfloat)ViewportWidth;
	YMax = 2.0f * ( y + BitmapHeight / 2.0f ) / (GLfloat)ViewportHeight;

	// Reuse (borrow) the character glyph vertex array.
	InitCharacterGlyphVertexRectangle( XMin, XMax, YMin, YMax );		// Invert the Y's.

	glUniform1i( glGetUniformLocation(  hShaderProgram, "ReportFormTexture" ), TEXUNIT_NUMBER_REPORT_IMAGE );

	glBindTexture( GL_TEXTURE_2D, TextureID );
	// Bind the externally declared vertex buffer.
	glBindBuffer( GL_ARRAY_BUFFER, VertexBufferID );
	// Associate the current vertex array just specified with the OpenGL array buffer.
	glBufferData( GL_ARRAY_BUFFER, sizeof( m_CharacterGlyphVertexRectangle ), &m_CharacterGlyphVertexRectangle, GL_STREAM_DRAW );
	CheckOpenGLResultAt( __FILE__, __LINE__ );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	glDrawBuffer( GL_COLOR_ATTACHMENT0 );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );				// Black Background
	glClear( GL_COLOR_BUFFER_BIT );						// Clear out the currently rendered image from the frame buffer.
//	glEnable( GL_TEXTURE_2D );

	// Render the blank report form.
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
	CheckOpenGLResultAt( __FILE__, __LINE__ );

//	glDisable( GL_TEXTURE_2D );
}


// This function creates and renders a full-scale report image for printing or saving to a file.
void CImageView::CreateReportImage( unsigned long ImageDestination, BOOL bUseCurrentStudy )
{
	HDC					hSavedDC;
	HGLRC				hSavedGLRC;
	float				ReportFormWidthInPixels;
	float				ReportFormHeighthInPixels;
	GLuint				hShaderProgram;
	unsigned int		ReportFormTextureID;
	GLenum				OutputColorFormat;

	hSavedDC = wglGetCurrentDC();
	hSavedGLRC = wglGetCurrentContext();
	wglMakeCurrent( m_hDC, m_hRC );
	ReportFormWidthInPixels = (float)m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
	ReportFormHeighthInPixels = (float)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
	glViewport( 0, 0, m_pAssignedDiagnosticImage -> m_ImageWidthInPixels,
						m_pAssignedDiagnosticImage -> m_ImageHeightInPixels );
	hShaderProgram = m_gReportFormShaderProgram;
	glActiveTexture( TEXTURE_UNIT_REPORT_IMAGE );

	ReportFormTextureID = CreateReportFormTexture();
	if ( ReportFormTextureID != 0 )
		{
		InitializReportFormFrameBuffer();
		CreateReportTextVertices( hShaderProgram );

		// If the off-screen frame buffer has been initialized, set up to render to it.  Otherwise, the
		// default display buffer (for on-screen rendering) is still bound.
		if ( m_ReportFormFrameBufferID != 0 )
			{
			glBindFramebuffer( GL_FRAMEBUFFER, m_ReportFormFrameBufferID );
			CheckOpenGLResultAt( __FILE__, __LINE__	);

			RenderReportFormTexture( hShaderProgram, ReportFormTextureID, m_ReportVertexBufferID, m_ReportVertexAttributesID,
																	0, 0, ReportFormWidthInPixels, ReportFormHeighthInPixels );
			if ( bUseCurrentStudy )
				RenderReport( m_hDC, IMAGE_DESTINATION_FILE );

			// Allocate an output buffer associated with the current study and load it from the temporary framebuffer.
			m_pAssignedDiagnosticImage -> m_pOutputImageData = (unsigned char*)malloc( (int)( ReportFormWidthInPixels * ReportFormHeighthInPixels * 3.0f ) );
			if ( ImageDestination == IMAGE_DESTINATION_PRINTER )
				OutputColorFormat = GL_BGR;
			else
				OutputColorFormat = GL_RGB;
			glReadPixels( 0, 0, (GLsizei)ReportFormWidthInPixels, (GLsizei)ReportFormHeighthInPixels, OutputColorFormat, GL_UNSIGNED_BYTE, m_pAssignedDiagnosticImage -> m_pOutputImageData );
			m_pAssignedDiagnosticImage -> m_OutputImageHeightInPixels = m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
			m_pAssignedDiagnosticImage -> m_OutputImageWidthInPixels = m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
			}

		DeleteReportTextVertices( hShaderProgram );
		glDeleteTextures( 1, (GLuint*)&ReportFormTextureID );
		glActiveTexture( TEXTURE_UNIT_DEFAULT );
		DeleteReportImage();			// Delete the image structures in the GPU.
		}
	wglMakeCurrent( hSavedDC, hSavedGLRC );
}


// This function deletes the report image after printing or saving to a file.
void CImageView::DeleteReportImage()
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );			// Revert to the display screen framebuffer.
	if ( m_ReportFormFrameBufferID != 0 )
		{
		glDeleteRenderbuffers( 1, &m_ReportFormRenderBufferID );
		glDeleteFramebuffers( 1, &m_ReportFormFrameBufferID );
		m_ReportFormFrameBufferID = 0;
		}
}


void CImageView::SaveReport()
{
	BOOL				bNoError = TRUE;
	float				ReportFormWidthInPixels;
	float				ReportFormHeighthInPixels;
	char				FileSpecForWriting[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				FileSpecForWritingPDFReport[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				ArchivedReportFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				StudyFileName[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				StudyFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				ArchivedStudyFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				ImageFileName[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				ImageFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				ArchivedImageFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				*pFileName;
	char				*pPDFFileName;
	char				Msg[ 512 ];
	CStudy				*pCurrentStudy;
	char				DateTimeString[ 32 ];
	char				DateOfRadiographString[ 32 ];
	char				*pChar;

	if ( m_pAssignedDiagnosticImage != 0 )
		{
		ReportFormWidthInPixels = (float)m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
		ReportFormHeighthInPixels = (float)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
		glViewport( 0, 0, m_pAssignedDiagnosticImage -> m_ImageWidthInPixels,
							m_pAssignedDiagnosticImage -> m_ImageHeightInPixels );

		// Create the report image in the GPU and copy it to the m_pAssignedDiagnosticImage output image buffer.
		CreateReportImage( IMAGE_DESTINATION_FILE, TRUE );

		pCurrentStudy = ThisBViewerApp.m_pCurrentStudy;
		strcpy( FileSpecForWriting, "" );
		strncat( FileSpecForWriting, BViewerConfiguration.ReportDirectory, FILE_PATH_STRING_LENGTH );
		if ( FileSpecForWriting[ strlen( FileSpecForWriting ) - 1 ] != '\\' )
			strcat( FileSpecForWriting, "\\" );
		pFileName = FileSpecForWriting + strlen( FileSpecForWriting );
		strcpy( FileSpecForWritingPDFReport, FileSpecForWriting );
		pPDFFileName = FileSpecForWritingPDFReport + strlen( FileSpecForWritingPDFReport );
		if ( pCurrentStudy != 0 )
			{
			strcat( FileSpecForWriting, pCurrentStudy -> m_PatientLastName );
			strcat( FileSpecForWriting, "-" );
			strcat( FileSpecForWriting, pCurrentStudy -> m_PatientFirstName );
			strcat( FileSpecForWriting, "_" );

			strcat( FileSpecForWritingPDFReport, pCurrentStudy -> m_PatientLastName );
			strcat( FileSpecForWritingPDFReport, "_" );
			strcat( FileSpecForWritingPDFReport, pCurrentStudy -> m_PatientFirstName );
			strcat( FileSpecForWritingPDFReport, "_" );

			if ( m_PageNumber == 1 )
				{
				GetDateAndTimeForFileName( DateTimeString );
				strcpy( m_ReportDateTimeString, DateTimeString );
				}
			else
				{
				strcpy( DateTimeString, "" );
				strncat( DateTimeString, m_ReportDateTimeString, 16 );
				}
			strcat( FileSpecForWriting, DateTimeString );
			strcat( FileSpecForWriting, "_" );
			SubstituteCharacterInText( pFileName, ' ', '_' );

			pCurrentStudy -> GetDateOfRadiographMMDDYY( DateOfRadiographString );
			strcat( FileSpecForWritingPDFReport, DateOfRadiographString );
			PruneEmbeddedWhiteSpace( FileSpecForWritingPDFReport );
			strcat( FileSpecForWritingPDFReport, ".pdf" );
			}
		strcat( FileSpecForWriting, "ReportPage" );
		if ( m_PageNumber == 1 )
			strcat( FileSpecForWriting, "1" );
		else if ( m_PageNumber == 2 )
			strcat( FileSpecForWriting, "2" );
		strcat( FileSpecForWriting, ".png" );
		if ( m_PageNumber == 1 )
			strcpy( pCurrentStudy -> m_ReportPage1FilePath, FileSpecForWriting );
		else if ( m_PageNumber == 2 )
			strcpy( pCurrentStudy -> m_ReportPage2FilePath, FileSpecForWriting );
		if ( m_PageNumber == 1 && pCurrentStudy -> m_pEventParameters != 0 )
			{
			strcpy( pCurrentStudy -> m_pEventParameters -> ReportPNGFilePath, FileSpecForWriting );
			pChar = strstr( pCurrentStudy -> m_pEventParameters -> ReportPNGFilePath, "__ReportPage1" );
			strcpy( pChar, "__ReportPage?.png" );

			strcpy( pCurrentStudy -> m_pEventParameters -> ReportPDFFilePath, FileSpecForWritingPDFReport );
			}
				
		// Write the report image to a file.
		bNoError = m_pAssignedDiagnosticImage -> WritePNGImageFile( FileSpecForWriting );
		}

	if ( BViewerConfiguration.bArchiveReportFiles )
		{
		strcpy( ArchivedReportFileSpec, BViewerConfiguration.ReportArchiveDirectory );
		LocateOrCreateDirectory( ArchivedReportFileSpec );	// Ensure directory exists.
		if ( ArchivedReportFileSpec[ strlen( ArchivedReportFileSpec ) - 1 ] != '\\' )
			strcat( ArchivedReportFileSpec, "\\" );
		strncat( ArchivedReportFileSpec, pFileName,
				FULL_FILE_SPEC_STRING_LENGTH - 1 - strlen( ArchivedReportFileSpec ) );

		sprintf( Msg, "    Copying report file:  %s to the archive folder", ArchivedReportFileSpec );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		bNoError = CopyFile( FileSpecForWriting, ArchivedReportFileSpec, FALSE );
		if ( !bNoError )
			{
			SystemErrorCode = GetLastError();
			sprintf( Msg, "   >>> Copy to report archive system error code %d", SystemErrorCode );
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
			}
		}
	pCurrentStudy -> GetStudyFileName( StudyFileName );
	strcpy( StudyFileSpec, "" );
	strncat( StudyFileSpec, BViewerConfiguration.DataDirectory, FULL_FILE_SPEC_STRING_LENGTH );
	if ( StudyFileSpec[ strlen( StudyFileSpec ) - 1 ] != '\\' )
		strcat( StudyFileSpec, "\\" );
	strncat( StudyFileSpec, StudyFileName, FULL_FILE_SPEC_STRING_LENGTH - strlen( StudyFileSpec ) - 1 );
	if ( pCurrentStudy -> m_pEventParameters != 0 )
		strcpy( pCurrentStudy -> m_pEventParameters -> SDYFilePath, StudyFileSpec );

	if ( BViewerConfiguration.bArchiveSDYFiles && m_PageNumber == 2 && pCurrentStudy != 0 )
		{
		strcpy( ArchivedStudyFileSpec, "" );
		strncat( ArchivedStudyFileSpec, BViewerConfiguration.DataArchiveDirectory, FULL_FILE_SPEC_STRING_LENGTH );
		LocateOrCreateDirectory( ArchivedStudyFileSpec );	// Ensure directory exists.
		if ( ArchivedStudyFileSpec[ strlen( ArchivedStudyFileSpec ) - 1 ] != '\\' )
			strcat( ArchivedStudyFileSpec, "\\" );

		StudyFileName[ strlen( StudyFileName ) - 4 ] = '\0';
		strcat( StudyFileName, "_" );
		strcat( StudyFileName, m_ReportDateTimeString );
		strcat( StudyFileName, ".sdy" );
		strncat( ArchivedStudyFileSpec, StudyFileName, FULL_FILE_SPEC_STRING_LENGTH - strlen( ArchivedStudyFileSpec ) - 1 );
	
		sprintf( Msg, "    Copying study data file:  %s to the archive folder", ArchivedStudyFileSpec );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		bNoError = CopyFile( StudyFileSpec, ArchivedStudyFileSpec, FALSE );
		if ( !bNoError )
			{
			SystemErrorCode = GetLastError();
			sprintf( Msg, "   >>> Copy to SDY file archive system error code %d", SystemErrorCode );
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
			}
		}
	if ( BViewerConfiguration.bArchiveImageFiles && m_PageNumber == 2 && pCurrentStudy != 0 )
		{
		strcpy( ImageFileName, "" );
		if ( pCurrentStudy ->m_pDiagnosticStudyList != 0 )
			if ( pCurrentStudy ->m_pDiagnosticStudyList -> pDiagnosticSeriesList != 0 )
				if ( pCurrentStudy ->m_pDiagnosticStudyList -> pDiagnosticSeriesList -> pDiagnosticImageList != 0 )
					{
					strncat( ImageFileName,  pCurrentStudy ->m_pDiagnosticStudyList -> pDiagnosticSeriesList -> pDiagnosticImageList -> SOPInstanceUID, FULL_FILE_SPEC_STRING_LENGTH - 1 );
					strncat( ImageFileName, ".png", FULL_FILE_SPEC_STRING_LENGTH - strlen( ImageFileName ) - 1 );

					strcpy( ImageFileSpec, "" );
					strncat( ImageFileSpec, BViewerConfiguration.ImageDirectory, FULL_FILE_SPEC_STRING_LENGTH );
					if ( ImageFileSpec[ strlen( ImageFileSpec ) - 1 ] != '\\' )
						strcat( ImageFileSpec, "\\" );
					strncat( ImageFileSpec, ImageFileName, FULL_FILE_SPEC_STRING_LENGTH - strlen( ImageFileSpec ) - 1 );

					strcpy( ArchivedImageFileSpec, "" );
					strncat( ArchivedImageFileSpec, BViewerConfiguration.ImageArchiveDirectory, FULL_FILE_SPEC_STRING_LENGTH );
					LocateOrCreateDirectory( ArchivedImageFileSpec );	// Ensure directory exists.
					if ( ArchivedImageFileSpec[ strlen( ArchivedImageFileSpec ) - 1 ] != '\\' )
						strcat( ArchivedImageFileSpec, "\\" );
					strncat( ArchivedImageFileSpec, ImageFileName, FULL_FILE_SPEC_STRING_LENGTH - strlen( ArchivedImageFileSpec ) - 1 );
	
					sprintf( Msg, "    Copying study image file:  %s to the archive folder", ArchivedImageFileSpec );
					LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
					bNoError = CopyFile( ImageFileSpec, ArchivedImageFileSpec, FALSE );
					if ( !bNoError )
						{
						SystemErrorCode = GetLastError();
						sprintf( Msg, "   >>> Copy to image file archive system error code %d", SystemErrorCode );
						LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
						}
					}
		}
	DeleteReportImage();
}


BOOL CImageView::OpenReportForPrinting( BOOL bShowPrintDialog )
{
	BOOL				bNoError = TRUE;
	DOCINFO				PrinterDocInfo;
	int					nResponseCode;

	CPrintDialog		PrintDialog( FALSE,
								PD_ALLPAGES |
								PD_USEDEVMODECOPIES |
								PD_NOPAGENUMS |
								PD_HIDEPRINTTOFILE |
								PD_NOSELECTION |
								PD_RETURNDC
								);

	if ( bShowPrintDialog )
		{
		bNoError = ( PrintDialog.DoModal() == IDOK );
		if ( bNoError )
			{
			memcpy( (void*)&m_UserPrintInfoInput, &PrintDialog.m_pd, sizeof(PRINTDLG) );
			// Assign the device context of the selected printer.
			bNoError = m_PrinterDC.Attach( m_UserPrintInfoInput.hDC );
			}
		}
	if ( bNoError )
		{
		// Begin the print job.
		::ZeroMemory( &PrinterDocInfo, sizeof(PrinterDocInfo) );
		PrinterDocInfo.lpszDocName = "BViewer Report";
		nResponseCode = m_PrinterDC.StartDoc( &PrinterDocInfo );
		bNoError = ( nResponseCode > 0 );
		if ( bNoError )
			m_hCompatibleDC = CreateCompatibleDC( NULL );
		}
	else
		bNoError = FALSE;

	return bNoError;
}


void CImageView::PrintReportPage( BOOL bUseCurrentStudy )
{
	CWaitCursor			DisplaysHourglass;
	BOOL				bNoError = TRUE;
	int					nResponseCode;
	unsigned long		nImageWidth;
	unsigned long		nImageHeight;
	unsigned long		ImageSizeInBytes;
	int					nRastersCopiedToPrinter;

	if ( m_pAssignedDiagnosticImage != 0 )
		{
		nImageWidth = m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
		nImageHeight = m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
		
		if ( m_hCompatibleDC != 0 && m_pAssignedDiagnosticImage != 0 )
			{
			nImageWidth = m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
			nImageHeight = m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
		
			memset( &m_PrintableBitmapInfo, 0, sizeof( BITMAPINFO ) );
			m_PrintableBitmapInfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
			m_PrintableBitmapInfo.bmiHeader.biWidth = nImageWidth;
			m_PrintableBitmapInfo.bmiHeader.biHeight = nImageHeight;
			m_PrintableBitmapInfo.bmiHeader.biPlanes = 1;
			m_PrintableBitmapInfo.bmiHeader.biBitCount = 24;
			m_PrintableBitmapInfo.bmiHeader.biCompression = BI_RGB;

			m_hPrintableBitmap = ::CreateDIBSection( m_hCompatibleDC, &m_PrintableBitmapInfo, DIB_RGB_COLORS, (void**)&m_pDIBImageData, NULL, 0 );
			if ( m_hPrintableBitmap != 0 )
				{
				::SelectObject( m_hCompatibleDC, m_hPrintableBitmap );
				SetExportDCPixelFormat( m_hCompatibleDC );
				m_hGLPrintingRC = wglCreateContext( m_hCompatibleDC );
				if ( m_hGLPrintingRC == 0 )
					{
					SystemErrorCode = GetLastError();
					bNoError = FALSE;
					}
				if ( bNoError )
					{
					m_hSavedDC = wglGetCurrentDC();
					m_hSavedGLRC = wglGetCurrentContext();
					if ( wglMakeCurrent( m_hCompatibleDC, m_hGLPrintingRC ) == FALSE )
						{
						SystemErrorCode = GetLastError();
						bNoError = FALSE;
						}
					}
				}
			}
		else
			bNoError = FALSE;


		if ( m_hPrintableBitmap != 0 )
			{

			::SelectObject( m_hCompatibleDC, m_hPrintableBitmap );
			if ( wglMakeCurrent( m_hCompatibleDC, m_hGLPrintingRC ) == FALSE )
				{
				SystemErrorCode = GetLastError();
				bNoError = FALSE;
				}
			// At this point, the bitmap has been created on the device context and provides
			// the memory for rendering the image.
			glViewport( 0, 0, nImageWidth, nImageHeight );

			// Create the report image in the GPU and copy it to the m_pAssignedDiagnosticImage output image buffer.
			CreateReportImage( IMAGE_DESTINATION_PRINTER, bUseCurrentStudy );

			ImageSizeInBytes = m_pAssignedDiagnosticImage -> m_OutputImageWidthInPixels * m_pAssignedDiagnosticImage -> m_OutputImageHeightInPixels * 3;
			memcpy( (unsigned char*)m_pDIBImageData, m_pAssignedDiagnosticImage -> m_pOutputImageData, ImageSizeInBytes );
			// At this point the current page bitmap is available for printing.
			// Prepare the printer driver to receive data.
			nResponseCode = m_PrinterDC.StartPage();
			bNoError = ( nResponseCode > 0 );
			if ( bNoError )
				{
				CSize			PrinterImageSize( m_PrinterDC.GetDeviceCaps( HORZRES ), m_PrinterDC.GetDeviceCaps( VERTRES ) );

				// Use StretchDIBits to scale the bitmap and maintain 
				// its original proportions (that is, if the bitmap was square 
				// when it appeared in the application's client area, it should 
				// also appear square on the page). 
				nRastersCopiedToPrinter = StretchDIBits( m_UserPrintInfoInput.hDC,
															0, 0, PrinterImageSize.cx, PrinterImageSize.cy,
															0, 0, m_PrintableBitmapInfo.bmiHeader.biWidth, m_PrintableBitmapInfo.bmiHeader.biHeight,
															m_pDIBImageData, &m_PrintableBitmapInfo, DIB_RGB_COLORS, SRCCOPY );
				}
			m_PrinterDC.EndPage();
			}
		}
}


void CImageView::CloseReportForPrinting()
{
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( m_hGLPrintingRC );
	wglMakeCurrent( m_hSavedDC, m_hSavedGLRC );
	m_PrinterDC.EndDoc();
	::DeleteObject( m_hPrintableBitmap );
	::DeleteDC( m_hCompatibleDC );
	free( m_pAssignedDiagnosticImage -> m_pOutputImageData );
	m_pAssignedDiagnosticImage -> m_pOutputImageData = 0;
}


void CImageView::OnSize( UINT nType, int cx, int cy )
{
	CWnd::OnSize( nType, cx, cy );
	GetExclusiveRightToRender();
	InitViewport();
	AllowOthersToRender();
}


LIST_FORMAT		ImageAnnotationListFormat =
					{ MAX_ANNOTATION_COUNT, {
						{ " Image File",		ABSTRACT_LEVEL_IMAGE,	350,	offsetof( DIAGNOSTIC_IMAGE, SOPInstanceUID )			},
						{ " Name",				ABSTRACT_LEVEL_PATIENT,	120,	offsetof( CStudy, m_PatientLastName )					},
						{ " ",					ABSTRACT_LEVEL_PATIENT, 80,		offsetof( CStudy, m_PatientFirstName )					},
						{ " ID",				ABSTRACT_LEVEL_PATIENT, 100,	offsetof( CStudy, m_PatientID )							},
						{ " Birth Date",		ABSTRACT_LEVEL_PATIENT, 80,		offsetof( CStudy, m_PatientsBirthDate )					},
						{ " Sex",				ABSTRACT_LEVEL_PATIENT, 40,		offsetof( CStudy, m_PatientsSex )						},
						{ " Ref. Physician",	ABSTRACT_LEVEL_STUDY,	120,	offsetof( DIAGNOSTIC_STUDY, ReferringPhysiciansName )	},
						{ " Modality",			ABSTRACT_LEVEL_SERIES,	50,		offsetof( DIAGNOSTIC_SERIES, Modality )					},
						{ " Study Date",		ABSTRACT_LEVEL_PATIENT,	60,		offsetof( CStudy, m_DateOfRadiograph )					},
						{ " Image Time",		ABSTRACT_LEVEL_IMAGE,	50,		offsetof( DIAGNOSTIC_IMAGE, InstanceCreationTime )		}
					 } };


void CImageView::EraseImageAnnotationInfo()
{
	IMAGE_ANNOTATION			*pImageAnnotationItem;
	IMAGE_ANNOTATION			*pPrevImageAnnotationItem;

	pImageAnnotationItem = m_pImageAnnotationList;
	while( pImageAnnotationItem != 0 )
		{
		pPrevImageAnnotationItem = pImageAnnotationItem;
		pImageAnnotationItem = pImageAnnotationItem -> pNextAnnotation;
		free( pPrevImageAnnotationItem );
		}
	m_pImageAnnotationList = 0;
}


void CImageView::LoadImageAnnotationInfo()
{
	unsigned long			nAnnotation;
	LIST_COLUMN_FORMAT		*pAnnotationFormat;
	CStudy					*pStudy;
	char					*pDataStructure;
	DIAGNOSTIC_STUDY		*pDiagnosticStudy;
	DIAGNOSTIC_SERIES		*pDiagnosticSeries;
	DIAGNOSTIC_IMAGE		*pDiagnosticImage;
	IMAGE_ANNOTATION		*pImageAnnotationItem;
	IMAGE_ANNOTATION		*pPrevImageAnnotationItem;
	char					*pAnnotationFieldValue;
	SYSTEMTIME				*pDate;
	
	pStudy = ThisBViewerApp.m_pCurrentStudy;
	if ( pStudy != 0 )
		{
		EraseImageAnnotationInfo();
		pPrevImageAnnotationItem = 0;
		pDiagnosticStudy = pStudy -> m_pCurrentStudyInfo;
		pDiagnosticSeries = pStudy -> m_pCurrentSeriesInfo;
		pDiagnosticImage = pStudy -> m_pCurrentImageInfo;
		for ( nAnnotation = 0; nAnnotation < ImageAnnotationListFormat.nColumns; nAnnotation++ )
			{
			pAnnotationFormat = &ImageAnnotationListFormat.ColumnFormatArray[ nAnnotation ];
			switch ( pAnnotationFormat -> DatabaseHierarchyLevel )
				{
				case ABSTRACT_LEVEL_PATIENT:
					pDataStructure = (char*)pStudy;
					break;
				case ABSTRACT_LEVEL_STUDY:
					pDataStructure = (char*)pDiagnosticStudy;
					break;
				case ABSTRACT_LEVEL_SERIES:
					pDataStructure = (char*)pDiagnosticSeries;
					break;
				case ABSTRACT_LEVEL_IMAGE:
					pDataStructure = (char*)pDiagnosticImage;
					break;
				}
			pAnnotationFieldValue = (char*)( pDataStructure + pAnnotationFormat -> DataStructureOffset );
			if ( strlen( pAnnotationFieldValue ) > 0 )
				{
				pImageAnnotationItem = (IMAGE_ANNOTATION*)malloc( sizeof(IMAGE_ANNOTATION) );
				if ( pImageAnnotationItem != 0 )
					{
					pImageAnnotationItem -> pNextAnnotation = 0;
					if ( pPrevImageAnnotationItem != 0 )
						pPrevImageAnnotationItem -> pNextAnnotation = pImageAnnotationItem;
					else
						m_pImageAnnotationList = pImageAnnotationItem;
					strcpy( pImageAnnotationItem -> TextField, pAnnotationFormat -> pColumnTitle );
					if ( strlen( pImageAnnotationItem -> TextField ) > 1 )
						strcat( pImageAnnotationItem -> TextField, ":  " );
					// Insert spaces after title.
					strncat( pImageAnnotationItem -> TextField, "                    ",
										18 - strlen( pImageAnnotationItem -> TextField ) );
					if ( strcmp( pAnnotationFormat -> pColumnTitle, " Birth Date" ) == 0 ||
								strcmp( pAnnotationFormat -> pColumnTitle, " Study Date" ) == 0 )
						{
						if ( ( (EDITED_DATE*)pAnnotationFieldValue ) -> bDateHasBeenEdited )
							{
							pDate = &( (EDITED_DATE*)pAnnotationFieldValue ) -> Date;
							sprintf( &pImageAnnotationItem -> TextField[ strlen( pImageAnnotationItem -> TextField ) ],
														"%2u/%2u/%4u", pDate -> wMonth, pDate -> wDay, pDate -> wYear );
							}
						}
					else
						strncat( pImageAnnotationItem -> TextField, pAnnotationFieldValue,
											MAX_ANNOTATION_CHARS - 1 - strlen( pImageAnnotationItem -> TextField ) );
					pPrevImageAnnotationItem = pImageAnnotationItem;
					}
				}
			}
		}
}


void CImageView::CreateImageAnnotationFontGlyphs( HDC hDC )
{
	BOOL					bOK;
	GLfloat					ViewportRect[ 4 ];
	GLfloat					ViewportWidth;
	GLfloat					ViewportHeight;
	CFont					TextFont;
	HGDIOBJ					hSavedFontHandle;
	int						nChar;
	DWORD					GlyphBitmapBufferSize;
	GLYPH_BITMAP_INFO		*pGlyphBitmap;
	GLYPHMETRICS			*pGlyphMetrics;
	MAT2					IdentityMatrix = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
	unsigned int			TextureID;
	GLsizei					GlyphBitmapHeight;
	GLsizei					GlyphBitmapWidth;
	GLfloat					Color[ 3 ] = { 0.0f, 1.0f, 0.0f };		// Paint the annotation characters green.

	glGetFloatv( GL_VIEWPORT, ViewportRect );
	ViewportWidth = ViewportRect[ 2 ] - ViewportRect[ 0 ];
	ViewportHeight = ViewportRect[ 3 ] - ViewportRect[ 1 ];

	// Create display lists for font character glyphs 0 through 128.
	bOK = ( TextFont.CreateFont(
			-(int)m_AnnotationCharHeight,	// nHeight in device units.
			0,								// nWidth - use available aspect ratio
			0,								// nEscapement - make character lines horizontal
			0,								// nOrientation - individual chars are horizontal
			FW_SEMIBOLD,					// nWeight - character stroke thickness
			FALSE,							// bItalic - not italic
			FALSE,							// bUnderline - not underlined
			FALSE,							// cStrikeOut - not a strikeout font
			ANSI_CHARSET,					// nCharSet - normal ansi characters
			OUT_TT_ONLY_PRECIS,				// nOutPrecision - choose font type using default search
			CLIP_DEFAULT_PRECIS,			// nClipPrecision - use default clipping
			PROOF_QUALITY,					// nQuality - best possible appearance
			FIXED_PITCH,					// nPitchAndFamily - fixed or variable pitch
			"Dontcare"						// lpszFacename
			) != 0 );

	if ( bOK )
		{
		hSavedFontHandle = ::SelectObject( hDC, (HGDIOBJ)(HFONT)( TextFont.GetSafeHandle() ) );
		if ( hSavedFontHandle != 0 )
			{
			// Generate a sequence of character glyph bitmaps for this font.
			// The Windows glyph bitmaps are DWORD alligned.
			glActiveTexture( TEXTURE_UNIT_IMAGE_ANNOTATIONS );		// Use texture unit 4 for glyph textures.
			glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

			for ( nChar = 0; nChar < 128 && bOK; nChar++ )
				{
				// Point to the bitmap array slot for this font character.
				pGlyphBitmap = &m_AnnotationFontGlyphBitmapArray[ nChar ];
				pGlyphMetrics = &pGlyphBitmap -> GlyphMetrics;
				// Get the buffer size required for this character's bitmap by passing a NULL buffer size and pointer.
				GlyphBitmapBufferSize = GetGlyphOutlineA( hDC, nChar, GGO_GRAY8_BITMAP, pGlyphMetrics, 0, NULL, &IdentityMatrix );
				bOK = ( GlyphBitmapBufferSize != GDI_ERROR );
				if ( bOK )
					{
					pGlyphBitmap -> pBitmapBuffer = (char*)malloc( GlyphBitmapBufferSize );
					pGlyphBitmap -> BufferSizeInBytes = GlyphBitmapBufferSize;
					bOK = (pGlyphBitmap -> pBitmapBuffer != 0 );
					if ( bOK )
						{
						// Retrieve the character bitmap into the buffer at pGlyphBitmapBuffer.  The grayscale value for each pixel ranges from
						// 0 to 63.  The fragment shader will multiply by 4 to bring each pixel to full scale 0 to 255.
						bOK = ( GetGlyphOutlineA( hDC, nChar, GGO_GRAY8_BITMAP, pGlyphMetrics, GlyphBitmapBufferSize,
																			pGlyphBitmap -> pBitmapBuffer, &IdentityMatrix ) != GDI_ERROR );
						}
					if ( bOK )
						{
						// Create a texture containing the current font character.
						GlyphBitmapWidth = pGlyphMetrics -> gmBlackBoxX;
						GlyphBitmapHeight = pGlyphMetrics -> gmBlackBoxY;
						glGenTextures( 1, &TextureID );
						glBindTexture( GL_TEXTURE_2D, TextureID );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
						glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, GlyphBitmapWidth, GlyphBitmapHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pGlyphBitmap -> pBitmapBuffer );
						// Save the texture ID associated with this character glyph.  It will need to be deleted on program exit.
						pGlyphBitmap -> TextureID = TextureID;

						// Free the bitmap buffer.  The character bitmap is now stored as a texture in the GPU memory.
						free( pGlyphBitmap -> pBitmapBuffer );
						pGlyphBitmap -> pBitmapBuffer = 0;
						pGlyphBitmap -> BufferSizeInBytes = 0;

						bOK = CheckOpenGLResultAt( __FILE__, __LINE__	);
						}
					}
				}
			::SelectObject( hDC, hSavedFontHandle );
			TextFont.DeleteObject();
			}
		}

	glActiveTexture( TEXTURE_UNIT_DEFAULT );
	if ( !bOK )
		LogMessage( ">>> Error creating image annotation text character glyph bitmaps.", MESSAGE_TYPE_SUPPLEMENTARY );
}


void CImageView::DeleteImageAnnotationFontGlyphs()
{
	int						nChar;
	GLYPH_BITMAP_INFO		*pGlyphBitmap;

	glActiveTexture( TEXTURE_UNIT_IMAGE_ANNOTATIONS );		// Use texture unit 4 for glyph textures.
	for ( nChar = 0; nChar < 128; nChar++ )
		{
		pGlyphBitmap = &m_AnnotationFontGlyphBitmapArray[ nChar ];
		if ( pGlyphBitmap -> TextureID != 0 )
			{
			glDeleteTextures( 1, (GLuint*)&pGlyphBitmap -> TextureID );
			pGlyphBitmap -> TextureID = 0;
			}
		}
	glActiveTexture( TEXTURE_UNIT_DEFAULT );
}


void CImageView::RenderImageAnnotations(  HDC hDC )
{
	GLfloat					ViewportRect[ 4 ];
	GLfloat					ViewportWidth;
	GLfloat					ViewportHeight;
	GLuint					hShaderProgram;
	CFont					TextFont;
	unsigned int			VertexBufferID;
	unsigned int			VertexAttributesID;
	IMAGE_ANNOTATION		*pImageAnnotationInfo;
	GLfloat					x, y;
	GLfloat					Color[ 3 ] = { 0.0f, 1.0f, 0.0f };		// Paint the annotation characters green.

	if ( m_pAssignedDiagnosticImage != 0 && m_bEnableAnnotations )
		{
		glGetFloatv( GL_VIEWPORT, ViewportRect );
		ViewportWidth = ViewportRect[ 2 ] - ViewportRect[ 0 ];
		ViewportHeight = ViewportRect[ 3 ] - ViewportRect[ 1 ];

		hShaderProgram = m_gImageAnnotationShaderProgram;
		glUseProgram( hShaderProgram );
			
		glActiveTexture( TEXTURE_UNIT_IMAGE_ANNOTATIONS );

		// Display study image text annotations if enabled.
		glGenBuffers( 1, &VertexBufferID );
		glBindBuffer( GL_ARRAY_BUFFER, VertexBufferID );

		glGenVertexArrays( 1, &VertexAttributesID );
		// Bind the Vertex Array Object first, then bind and set the vertex buffer.  Then configure vertex attributes(s).
		glBindVertexArray( VertexAttributesID );

		// Set up and enable the vertex attribute array at location 0 in the vertex shader.  This array specifies the vertex positions.
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0 );
		glEnableVertexAttribArray( 0 );		// ( location = 0 )

		// Set up and enable the vertex attribute array at location 1 in the vertex shader.  This array specifies the texture vertex positions.
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (GLvoid*)( 12 * sizeof(float) ) );
		glEnableVertexAttribArray( 1 );		// ( location = 1 )
		CheckOpenGLResultAt( __FILE__, __LINE__	);

		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		y = ( ViewportHeight ) - 20.0f;
		pImageAnnotationInfo = m_pImageAnnotationList;
		while ( pImageAnnotationInfo != 0 )
			{
			x = ( - ViewportWidth / 2 ) + 25.0f;
			y -= m_AnnotationCharHeight;

			RenderTextString( hShaderProgram, TEXTURE_UNIT_IMAGE_ANNOTATIONS, m_AnnotationFontGlyphBitmapArray, pImageAnnotationInfo -> TextField,
														VertexBufferID, VertexAttributesID, x, y, Color );

			pImageAnnotationInfo = pImageAnnotationInfo -> pNextAnnotation;
			}
		glDeleteVertexArrays( 1, &VertexAttributesID );
		glDeleteBuffers( 1, &VertexBufferID );
		glDisable( GL_BLEND );
		glBindTexture( GL_TEXTURE_2D, 0 );
		glActiveTexture( TEXTURE_UNIT_DEFAULT );
		glUseProgram( 0 );
		CheckOpenGLResultAt( __FILE__, __LINE__	);
		}
}


void CImageView::RenderReportTextString( GLuint hShaderProgram, GLYPH_BITMAP_INFO *pGlyphBitmapArray, char *pTextString, unsigned int VertexBufferID,
											unsigned int VertexAttributesID, float x, float y, GLfloat Color[ 3 ] )
{
	unsigned int			nChar;
	char					Char;
	GLYPH_BITMAP_INFO		*pGlyphBitmap;
	GLYPHMETRICS			*pGlyphMetrics;
	GLfloat					ViewportRect[ 4 ];
	GLfloat					XPos, YPos;
	GLfloat					CellWidth, CellHeight;
	GLfloat					XMin, XMax;
	GLfloat					YMin, YMax;
	GLfloat					ViewportWidth;
	GLfloat					ViewportHeight;

	glGetFloatv( GL_VIEWPORT, ViewportRect );
	ViewportWidth = ViewportRect[ 2 ] - ViewportRect[ 0 ];
	ViewportHeight = ViewportRect[ 3 ] - ViewportRect[ 1 ];
	glUseProgram( hShaderProgram );
	glUniform3f( glGetUniformLocation( hShaderProgram, "TextColor"), Color[ 0 ], Color[ 1 ], Color[ 2 ] );
	CheckOpenGLResultAt( __FILE__, __LINE__ );
	glBindVertexArray( VertexAttributesID );
	glActiveTexture( TEXTURE_UNIT_REPORT_TEXT );

    // Iterate through the characters in pTextString.
	for ( nChar = 0; nChar < strlen( pTextString ); nChar++ )
		{
		Char = pTextString[ nChar ];
		if ( Char >= 0 && Char < 128 )
			{
			pGlyphBitmap = &pGlyphBitmapArray[ Char ];
			pGlyphMetrics = &pGlyphBitmap -> GlyphMetrics;

			// Offset the glyph bitmap to position it correctly inside the current character cell.
			XPos = x + pGlyphMetrics -> gmptGlyphOrigin.x;
			YPos = y + ( (GLfloat)pGlyphMetrics -> gmptGlyphOrigin.y - (GLfloat)pGlyphMetrics -> gmBlackBoxY );

 			CellWidth = (GLfloat)pGlyphMetrics -> gmBlackBoxX;
			CellHeight = (GLfloat)pGlyphMetrics -> gmBlackBoxY;

			// Set up the vertex buffer for the current character.
			// Adjust cell boundarys to mornalize to -1.0 < x , 1.0, -1.0 < y , 1.0.  The geometric
			// transformation matrix expects this.
			XMin = 2.0f * XPos / (GLfloat)ViewportWidth - 1.0f;
			YMin = 2.0f * YPos / (GLfloat)ViewportHeight - 1.0f;
			XMax = 2.0f * ( XPos +  + CellWidth ) / (GLfloat)ViewportWidth - 1.0f;
			YMax = 2.0f * ( YPos + CellHeight ) / (GLfloat)ViewportHeight - 1.0f;

			InitCharacterGlyphVertexRectangle( XMin, XMax, YMax, YMin );		// Invert the Y's.
			// Bind the texture for the current character.  Indicate to the shader that we're using texture unit 4.
			glUniform1i( glGetUniformLocation(  hShaderProgram, "ReportGlyphTexture" ), TEXUNIT_NUMBER_REPORT_TEXT );
			glBindTexture( GL_TEXTURE_2D, pGlyphBitmap -> TextureID );
			// Bind the externally declared vertex buffer.
			glBindBuffer( GL_ARRAY_BUFFER, VertexBufferID );
			// Associate the current vertex array just specified with the OpenGL array buffer.
			glBufferData( GL_ARRAY_BUFFER, sizeof( m_CharacterGlyphVertexRectangle ), &m_CharacterGlyphVertexRectangle, GL_STREAM_DRAW );
			CheckOpenGLResultAt( __FILE__, __LINE__ );
			glBindBuffer( GL_ARRAY_BUFFER, 0 );
			// Render the character.
			glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
			}

		CellWidth = pGlyphMetrics -> gmCellIncX;
		x += CellWidth;
		}
	glBindTexture( GL_TEXTURE_2D, 0 );
	glActiveTexture( TEXTURE_UNIT_DEFAULT );
	CheckOpenGLResultAt( __FILE__, __LINE__ );
}


void CImageView::RenderTextString( GLuint hShaderProgram, GLuint TextureUnit, GLYPH_BITMAP_INFO *pGlyphBitmapArray, char *pTextString, unsigned int VertexBufferID,
											unsigned int VertexAttributesID, float x, float y, GLfloat Color[ 3 ] )
{
	unsigned int			nChar;
	char					Char;
	GLYPH_BITMAP_INFO		*pGlyphBitmap;
	GLYPHMETRICS			*pGlyphMetrics;
	GLfloat					ViewportRect[ 4 ];
	GLfloat					XPos, YPos;
	GLfloat					CellWidth, CellHeight;
	GLfloat					XMin, XMax;
	GLfloat					YMin, YMax;
	GLfloat					ViewportWidth;
	GLfloat					ViewportHeight;

	glGetFloatv( GL_VIEWPORT, ViewportRect );
	ViewportWidth = ViewportRect[ 2 ] - ViewportRect[ 0 ];
	ViewportHeight = ViewportRect[ 3 ] - ViewportRect[ 1 ];
	glUseProgram( hShaderProgram );
	glUniform3f( glGetUniformLocation( hShaderProgram, "TextColor"), Color[ 0 ], Color[ 1 ], Color[ 2 ] );
	CheckOpenGLResultAt( __FILE__, __LINE__ );
	glBindVertexArray( VertexAttributesID );

    // Iterate through the characters in pTextString.
	for ( nChar = 0; nChar < strlen( pTextString ); nChar++ )
		{
		Char = pTextString[ nChar ];
		if ( Char >= 0 && Char < 128 )
			{
			pGlyphBitmap = &pGlyphBitmapArray[ Char ];
			pGlyphMetrics = &pGlyphBitmap -> GlyphMetrics;

			// Offset the glyph bitmap to position it correctly inside the current character cell.
			XPos = x + pGlyphMetrics -> gmptGlyphOrigin.x;
			YPos = y + ( (GLfloat)pGlyphMetrics -> gmptGlyphOrigin.y - (GLfloat)pGlyphMetrics -> gmBlackBoxY );

 			CellWidth = (GLfloat)pGlyphMetrics -> gmBlackBoxX;
			CellHeight = (GLfloat)pGlyphMetrics -> gmBlackBoxY;

			// Set up the vertex buffer for the current character.
			// Adjust cell boundarys to mornalize to -1.0 < x , 1.0, -1.0 < y , 1.0.  The geometric
			// transformation matrix expects this.
			XMin = (GLfloat)( ( XPos - ViewportWidth / 2.0 ) / ViewportWidth );
			XMax = (GLfloat)( ( XPos + CellWidth - ViewportWidth / 2.0 ) / ViewportWidth );
			YMin = (GLfloat)( ( YPos ) / ViewportHeight );
			YMax = (GLfloat)( ( YPos + CellHeight ) / ViewportHeight );

			InitCharacterGlyphVertexRectangle( XMin, XMax, YMax, YMin );		// Invert the Y's.
			// Bind the texture for the current character.  Indicate to the shader that we're using texture unit 4.
			if ( TextureUnit == TEXTURE_UNIT_IMAGE_ANNOTATIONS )
				glUniform1i( glGetUniformLocation(  hShaderProgram, "AnnotationGlyphTexture" ), TEXUNIT_NUMBER_IMAGE_ANNOTATIONS );
			else if ( TextureUnit == TEXTURE_UNIT_IMAGE_MEASUREMENTS )
				glUniform1i( glGetUniformLocation(  hShaderProgram, "MeasurementGlyphTexture" ), TEXUNIT_NUMBER_IMAGE_MEASUREMENTS );
			CheckOpenGLResultAt( __FILE__, __LINE__ );
			glBindTexture( GL_TEXTURE_2D, pGlyphBitmap -> TextureID );
			// Bind the externally declared vertex buffer.
			glBindBuffer( GL_ARRAY_BUFFER, VertexBufferID );
			// Associate the current vertex array just specified with the OpenGL array buffer.
			glBufferData( GL_ARRAY_BUFFER, sizeof( m_CharacterGlyphVertexRectangle ), &m_CharacterGlyphVertexRectangle, GL_STREAM_DRAW );
			CheckOpenGLResultAt( __FILE__, __LINE__ );
			// Render the character.
			glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
			}

		CellWidth = pGlyphMetrics -> gmCellIncX;
		x += CellWidth;
		}
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
}


void CImageView::CreateImageMeasurementFontGlyphs( HDC hDC )
{
	BOOL					bOK;
	GLfloat					ViewportRect[ 4 ];
	GLfloat					ViewportWidth;
	GLfloat					ViewportHeight;
//	GLuint					hShaderProgram;
	CFont					TextFont;
	HGDIOBJ					hSavedFontHandle;
	bool					bFontOk = FALSE;
	int						nChar;
	DWORD					GlyphBitmapBufferSize;
	GLYPH_BITMAP_INFO		*pGlyphBitmap;
	GLYPHMETRICS			*pGlyphMetrics;
	MAT2					IdentityMatrix = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
	unsigned int			TextureID;
	GLsizei					GlyphBitmapHeight;
	GLsizei					GlyphBitmapWidth;
	GLfloat					Color[ 3 ] = { 0.0f, 1.0f, 0.0f };		// Paint the annotation characters green.

	glGetFloatv( GL_VIEWPORT, ViewportRect );
	ViewportWidth = ViewportRect[ 2 ] - ViewportRect[ 0 ];
	ViewportHeight = ViewportRect[ 3 ] - ViewportRect[ 1 ];

	glActiveTexture( TEXTURE_UNIT_IMAGE_MEASUREMENTS );

	// Create display lists for font character glyphs 0 through 128.
	bFontOk = ( TextFont.CreateFont(
			-(int)m_MeasurementCharHeight,	// nHeight in device units.
			0,								// nWidth - use available aspect ratio
			0,								// nEscapement - make character lines horizontal
			0,								// nOrientation - individual chars are horizontal
			FW_SEMIBOLD,					// nWeight - character stroke thickness
			FALSE,							// bItalic - not italic
			FALSE,							// bUnderline - not underlined
			FALSE,							// cStrikeOut - not a strikeout font
			ANSI_CHARSET,					// nCharSet - normal ansi characters
			OUT_TT_ONLY_PRECIS,				// nOutPrecision - choose font type using default search
			CLIP_DEFAULT_PRECIS,			// nClipPrecision - use default clipping
			PROOF_QUALITY,					// nQuality - best possible appearance
			FIXED_PITCH,					// nPitchAndFamily - fixed or variable pitch
			"Dontcare"						// lpszFacename
			) != 0 );

	if ( bFontOk )
		{
		hSavedFontHandle = ::SelectObject( hDC, (HGDIOBJ)(HFONT)( TextFont.GetSafeHandle() ) );
		if ( hSavedFontHandle != 0 )
			{
			// Generate a sequence of character glyph bitmaps for this font.
			// The Windows glyph bitmaps are DWORD alligned.
			glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

			for ( nChar = 0; nChar < 128; nChar++ )
				{
				// Point to the bitmap array slot for this font character.
				pGlyphBitmap = &m_MeasurementFontGlyphBitmapArray[ nChar ];
				pGlyphMetrics = &pGlyphBitmap -> GlyphMetrics;
				// Get the buffer size required for this character's bitmap by passing a NULL buffer size and pointer.
				GlyphBitmapBufferSize = GetGlyphOutlineA( hDC, nChar, GGO_GRAY8_BITMAP, pGlyphMetrics, 0, NULL, &IdentityMatrix );
				bOK = ( GlyphBitmapBufferSize != GDI_ERROR );
				if ( bOK )
					{
					pGlyphBitmap -> pBitmapBuffer = (char*)malloc( GlyphBitmapBufferSize );
					pGlyphBitmap -> BufferSizeInBytes = GlyphBitmapBufferSize;
					bOK = (pGlyphBitmap -> pBitmapBuffer != 0 );
					if ( bOK )
						{
						// Retrieve the character bitmap into the buffer at pGlyphBitmapBuffer.  The grayscale value for each pixel ranges from
						// 0 to 63.  The fragment shader will multiply by 4 to bring each pixel to full scale 0 to 255.
						bOK = ( GetGlyphOutlineA( hDC, nChar, GGO_GRAY8_BITMAP, pGlyphMetrics, GlyphBitmapBufferSize,
																			pGlyphBitmap -> pBitmapBuffer, &IdentityMatrix ) != GDI_ERROR );
						}
					if ( bOK )
						{
						// Create a texture containing the current font character.
						GlyphBitmapWidth = pGlyphMetrics -> gmBlackBoxX;
						GlyphBitmapHeight = pGlyphMetrics -> gmBlackBoxY;
						glGenTextures( 1, &TextureID );
						glBindTexture( GL_TEXTURE_2D, TextureID );
						glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, GlyphBitmapWidth, GlyphBitmapHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pGlyphBitmap -> pBitmapBuffer );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
						// Save the texture ID associated with this character glyph.  It will need to be deleted on program exit.
						pGlyphBitmap -> TextureID = TextureID;

						// Free the bitmap buffer.  The character bitmap is now stored as a texture in the GPU memory.
						free( pGlyphBitmap -> pBitmapBuffer );
						pGlyphBitmap -> pBitmapBuffer = 0;
						pGlyphBitmap -> BufferSizeInBytes = 0;

						CheckOpenGLResultAt( __FILE__, __LINE__	);
						}
					}
				}
			::SelectObject( hDC, hSavedFontHandle );
			TextFont.DeleteObject();
			}
		}

	glActiveTexture( TEXTURE_UNIT_DEFAULT );
}


void CImageView::DeleteImageMeasurementFontGlyphs()
{
	int						nChar;
	GLYPH_BITMAP_INFO		*pGlyphBitmap;

	glActiveTexture( TEXTURE_UNIT_IMAGE_MEASUREMENTS );
	for ( nChar = 0; nChar < 128; nChar++ )
		{
		pGlyphBitmap = &m_MeasurementFontGlyphBitmapArray[ nChar ];
		if ( pGlyphBitmap -> TextureID != 0 )
			{
			glDeleteTextures( 1, (GLuint*)&pGlyphBitmap -> TextureID );
			pGlyphBitmap -> TextureID = 0;
			}
		}
	glActiveTexture( TEXTURE_UNIT_DEFAULT );
}



void CImageView::RenderImageMeasurementLines()
{
	GLfloat						ViewportRect[ 4 ];
	GLfloat						ViewportWidth;
	GLfloat						ViewportHeight;
	GLfloat						ImageWidthInPixels;
	GLfloat						ImageHeightInPixels;
	GLfloat						ImageOriginX;
	GLfloat						ImageOriginY;
	GLfloat						TranslationX;
	GLfloat						TranslationY;
	GLfloat						ScaleFactor;
	GLfloat						PositionX;			// Viewport position (pixels).
	GLfloat						PositionY;
	float						x;					// GPU coordinate.
	float						y;
	float						MarkDx;
	float						MarkDy;
	MEASUREMENT_LINE_VERTICES	*pVertexArray;
	GLuint						hShaderProgram;
	CFont						TextFont;
	unsigned int				VertexBufferID;
	unsigned int				VertexAttributesID;
	GLfloat						Color[ 3 ] = { 0.5f, 0.0f, 0.0f };		// Paint the annotation characters dark red.
	MEASURED_INTERVAL			*pMeasuredInterval;


	glGetFloatv( GL_VIEWPORT, ViewportRect );
	ViewportWidth = ViewportRect[ 2 ] - ViewportRect[ 0 ];
	ViewportHeight = ViewportRect[ 3 ] - ViewportRect[ 1 ];
	ImageWidthInPixels = (GLfloat)m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
	ImageHeightInPixels = (GLfloat)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
	ScaleFactor = (GLfloat)m_pAssignedDiagnosticImage -> m_ScaleFactor;

	ImageOriginX = ( ViewportWidth - ImageWidthInPixels * ScaleFactor ) / 2.0f;
	ImageOriginY = ( ViewportHeight - ImageHeightInPixels * ScaleFactor ) / 2.0f;
	
	TranslationX = ( ImageWidthInPixels / 2.0f - (GLfloat)m_pAssignedDiagnosticImage -> m_FocalPoint.x ) * ScaleFactor;
	TranslationY = ( ImageHeightInPixels / 2.0f - (GLfloat)m_pAssignedDiagnosticImage -> m_FocalPoint.y ) * ScaleFactor;
	
	hShaderProgram = m_gLineDrawingShaderProgram;
	glUseProgram( hShaderProgram );
			
	// Display measurement lines.
	glGenBuffers( 1, &VertexBufferID );
	glBindBuffer( GL_ARRAY_BUFFER, VertexBufferID );

	glGenVertexArrays( 1, &VertexAttributesID );
	// Bind the Vertex Array Object first, then bind and set the vertex buffer.  Then configure vertex attributes(s).
	glBindVertexArray( VertexAttributesID );

	// Set up and enable the vertex attribute array at location 0 in the vertex shader.  This array specifies the vertex positions.
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
	glEnableVertexAttribArray( 0 );		// ( location = 0 )
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	glUniform3f( glGetUniformLocation( hShaderProgram, "DrawingColor"), Color[ 0 ], Color[ 1 ], Color[ 2 ] );
	CheckOpenGLResultAt( __FILE__, __LINE__ );

	pMeasuredInterval = m_pMeasuredIntervalList;
	while ( pMeasuredInterval != 0 )
		{
		PositionX = ScaleFactor * pMeasuredInterval -> ScaledStartingPointX + ImageOriginX + TranslationX;
		PositionY = ViewportHeight - ( ScaleFactor * pMeasuredInterval -> ScaledStartingPointY + ImageOriginY + TranslationY );
	
		x = 2.0f * PositionX / (GLfloat)ViewportWidth - 1.0f;
		y = 2.0f * PositionY / (GLfloat)ViewportHeight - 1.0f;

		MarkDx = 40.0f / ViewportRect[ 2 ];
		MarkDy = 40.0f / ViewportRect[ 3 ];
		pVertexArray = &pMeasuredInterval -> MeasurementLineVertexArray;

		// Set up the vertex coordinates for drawing the origin "plus" marker.
		pVertexArray -> OriginMarkHorizontalBeginX = x - MarkDx;
		pVertexArray -> OriginMarkHorizontalBeginY =  y;
		pVertexArray -> OriginMarkHorizontalEndX = x + MarkDx;
		pVertexArray -> OriginMarkHorizontalEndY =  y;
		pVertexArray -> OriginMarkVerticalBeginX = x;
		pVertexArray -> OriginMarkVerticalBeginY = y - MarkDy;
		pVertexArray -> OriginMarkVerticalEndX = x;
		pVertexArray -> OriginMarkVerticalEndY = y + MarkDy;

		// Establish the vertex for the beginning of the line.
		pVertexArray -> LineBeginX = x;
		pVertexArray -> LineBeginY = y;

		PositionX = ScaleFactor * pMeasuredInterval -> ScaledEndingPointX + ImageOriginX + TranslationX;
		PositionY = ViewportHeight - ( ScaleFactor * pMeasuredInterval -> ScaledEndingPointY + ImageOriginY + TranslationY );
	
		x = 2.0f * PositionX / (GLfloat)ViewportWidth - 1.0f;
		y = 2.0f * PositionY / (GLfloat)ViewportHeight - 1.0f;

		MarkDx = 40.0f / ViewportRect[ 2 ];
		MarkDy = 40.0f / ViewportRect[ 3 ];
		pVertexArray = &pMeasuredInterval -> MeasurementLineVertexArray;

		// Set up the vertex coordinates for drawing the destination "plus" marker.
		pVertexArray -> DestinationMarkHorizontalBeginX = x - MarkDx;
		pVertexArray -> DestinationMarkHorizontalBeginY = y;
		pVertexArray -> DestinationMarkHorizontalEndX = x + MarkDx;
		pVertexArray -> DestinationMarkHorizontalEndY = y;
		pVertexArray -> DestinationMarkVerticalBeginX = x;
		pVertexArray -> DestinationMarkVerticalBeginY = y - MarkDy;
		pVertexArray -> DestinationMarkVerticalEndX = x;
		pVertexArray -> DestinationMarkVerticalEndY = y + MarkDy;

		// Establish the vertex for the beginning of the line.
		pVertexArray -> LineEndX = x;
		pVertexArray -> LineEndY = y;

		// Associate the current vertex array just specified with the OpenGL array buffer.
		glBufferData( GL_ARRAY_BUFFER, sizeof( pMeasuredInterval -> MeasurementLineVertexArray ), &pMeasuredInterval -> MeasurementLineVertexArray, GL_STREAM_DRAW );
		CheckOpenGLResultAt( __FILE__, __LINE__ );
		// Render the ten vertices for the line drawings to be associated with this measurement.
		glDrawArrays( GL_LINES, 0, 10 );

		pMeasuredInterval = pMeasuredInterval -> pNextInterval;
		}

	glDeleteVertexArrays( 1, &VertexAttributesID );
	glDeleteBuffers( 1, &VertexBufferID );
	glUseProgram( 0 );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
}


void CImageView::RenderImageMeasurements()
{
	GLfloat					ViewportRect[ 4 ];
	GLfloat					ViewportWidth;
	GLfloat					ViewportHeight;
	GLuint					hShaderProgram;
	CFont					TextFont;
	unsigned int			VertexBufferID;
	unsigned int			VertexAttributesID;
	GLfloat					x, y;
	GLfloat					Color[ 3 ] = { 0.5f, 0.0f, 0.0f };		// Paint the annotation characters dark red.
	double					ScaleFactor;
	double					TranslationX;
	double					TranslationY;
	MEASURED_INTERVAL		*pMeasuredInterval;
	double					MeasuredLength;
	char					TextField[ 32 ];
	size_t					TextLength;

	if ( m_pAssignedDiagnosticImage != 0 )
		{
		glGetFloatv( GL_VIEWPORT, ViewportRect );
		ViewportWidth = ViewportRect[ 2 ] - ViewportRect[ 0 ];
		ViewportHeight = ViewportRect[ 3 ] - ViewportRect[ 1 ];

		hShaderProgram = m_gImageMeasurementShaderProgram;
		glUseProgram( hShaderProgram );
			
		// Display study image text annotations if enabled.
		glGenBuffers( 1, &VertexBufferID );
		glBindBuffer( GL_ARRAY_BUFFER, VertexBufferID );

		glGenVertexArrays( 1, &VertexAttributesID );
		// Bind the Vertex Array Object first, then bind and set the vertex buffer.  Then configure vertex attributes(s).
		glBindVertexArray( VertexAttributesID );

		// Set up and enable the vertex attribute array at location 0 in the vertex shader.  This array specifies the vertex positions.
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0 );
		glEnableVertexAttribArray( 0 );		// ( location = 0 )

		// Set up and enable the vertex attribute array at location 1 in the vertex shader.  This array specifies the texture vertex positions.
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (GLvoid*)( 12 * sizeof(float) ) );
		glEnableVertexAttribArray( 1 );		// ( location = 1 )
		CheckOpenGLResultAt( __FILE__, __LINE__	);

		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glBindVertexArray( 0 );

		glActiveTexture( TEXTURE_UNIT_IMAGE_MEASUREMENTS );

		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		ScaleFactor = m_pAssignedDiagnosticImage -> m_ScaleFactor;

		// The center of the viewport rectangle is the starting focal point.  The translation vector is the net movement
		// in screen coordinates of the center of the image.
		TranslationX = (double)ViewportRect[ 2 ] / 2.0 - (double)m_pAssignedDiagnosticImage -> m_FocalPoint.x * ScaleFactor;
		TranslationY = (double)ViewportRect[ 3 ] / 2.0 - (double)m_pAssignedDiagnosticImage -> m_FocalPoint.y * ScaleFactor;

		pMeasuredInterval = m_pMeasuredIntervalList;
		while ( pMeasuredInterval != 0 )
			{
			x = (GLfloat)( (double)pMeasuredInterval -> ScaledStartingPointX * ScaleFactor + TranslationX );
			y = (GLfloat)( (double)pMeasuredInterval -> ScaledStartingPointY * ScaleFactor + TranslationY );
					
			MeasuredLength = 0.0;
			sprintf( TextField, "%d mm", (int)MeasuredLength );
			TextLength = strlen( TextField );
			x = 2.0f * x - ViewportWidth / 2.0f;
			y = -2.0f * y + 30 + ViewportHeight;
			RenderTextString( hShaderProgram, TEXTURE_UNIT_IMAGE_MEASUREMENTS, m_MeasurementFontGlyphBitmapArray, TextField, VertexBufferID, VertexAttributesID, x, y, Color );

			x = (GLfloat)( (double)pMeasuredInterval -> ScaledEndingPointX * ScaleFactor + TranslationX );
			y = (GLfloat)( (double)pMeasuredInterval -> ScaledEndingPointY * ScaleFactor + TranslationY );

			MeasuredLength = pMeasuredInterval -> Distance / m_PixelsPerMillimeter;
			sprintf( TextField, "%d mm", (int)MeasuredLength );
			TextLength = strlen( TextField );
			x = 2.0f * x - ViewportWidth / 2.0f;
			y = -2.0f * y + 30 + ViewportHeight;
			RenderTextString( hShaderProgram, TEXTURE_UNIT_IMAGE_MEASUREMENTS, m_MeasurementFontGlyphBitmapArray, TextField, VertexBufferID, VertexAttributesID, x, y, Color );

			pMeasuredInterval = pMeasuredInterval -> pNextInterval;
			}

		glDeleteVertexArrays( 1, &VertexAttributesID );
		glDeleteBuffers( 1, &VertexBufferID );
		glDisable( GL_BLEND );
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture( TEXTURE_UNIT_DEFAULT );
		RenderImageMeasurementLines();
		glUseProgram( 0 );
		CheckOpenGLResultAt( __FILE__, __LINE__	);
		}
}


void CImageView::SetDiagnosticImage( CDiagnosticImage *pDiagnosticImage, CStudy *pStudy )
{
	BOOL			bNoError = TRUE;
	char			SystemErrorMessage[ FULL_FILE_SPEC_STRING_LENGTH ];
	char			Msg[ FULL_FILE_SPEC_STRING_LENGTH ];

	if ( wglMakeCurrent( m_hDC, m_hRC ) == FALSE )
		{
		SystemErrorCode = GetLastSystemErrorMessage( SystemErrorMessage, FULL_FILE_SPEC_STRING_LENGTH - 1 );
		if ( SystemErrorCode != 0 )
			{
			sprintf( Msg, "Error setting current rendering context.  System message:  %s", SystemErrorMessage );
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
			}
		}
	do
		bNoError = CheckOpenGLResultAt( __FILE__, __LINE__ );
	while ( !bNoError );

	EraseImageAnnotationInfo();
	if ( pDiagnosticImage != 0 )
		{
		m_pAssignedDiagnosticImage = pDiagnosticImage;
		m_Mouse.m_pTargetImage = pDiagnosticImage;
		LogMessage( "Loading image into view.", MESSAGE_TYPE_SUPPLEMENTARY );
		ResetDiagnosticImage( FALSE );
		}
	CheckOpenGLResultAt( __FILE__, __LINE__ );
}


void CImageView::LoadCurrentImageSettingsIntoEditBoxes()
{
	TomEdit					*pCtrlWindowCenter;
	TomEdit					*pCtrlWindowWidth;
	TomEdit					*pCtrlGamma;
	char					NumberConvertedToText[ _CVTBUFSIZE ];
	char					NumberFormat[ 32 ];
	double					AdjustedMaxPixelValue;
	IMAGE_CALIBRATION_INFO	*pImageCalibrationInfo;
	double					OriginalMaxPixelValue;
	int						nHighBit;

	if ( m_pAssignedDiagnosticImage != 0 )
		{
		// Get the scale factors for converting from adjusted to original pixel value scale.
		AdjustedMaxPixelValue = m_pAssignedDiagnosticImage -> m_MaxGrayscaleValue;
		pImageCalibrationInfo = m_pAssignedDiagnosticImage -> m_pImageCalibrationInfo;
		nHighBit = m_pAssignedDiagnosticImage -> m_nHighBit;
		if ( pImageCalibrationInfo != 0 )
			OriginalMaxPixelValue = pow( (double)2, nHighBit + 1 ) - 1;
		else
			OriginalMaxPixelValue = AdjustedMaxPixelValue;

		pCtrlWindowCenter = (TomEdit*)m_pWndDlgBar -> GetDlgItem( IDC_EDIT_WINDOW_CENTER );
		if ( pCtrlWindowCenter != 0 )
			{
			// Adjust the window center from the 16-bit max pixel value to the original max pixel value.
			sprintf( NumberConvertedToText, "%d", (int)( ( OriginalMaxPixelValue *
							( m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowCenter - 0.5) / AdjustedMaxPixelValue ) + 0.5 ) );
			TrimBlanks( NumberConvertedToText );
			pCtrlWindowCenter -> SetWindowText( NumberConvertedToText );
			}
		pCtrlWindowWidth = (TomEdit*)m_pWndDlgBar -> GetDlgItem( IDC_EDIT_WINDOW_WIDTH );
		if ( pCtrlWindowWidth != 0 )
			{
			// Adjust the window width from the 16-bit max pixel value to the original max pixel value.
			sprintf( NumberConvertedToText, "%d", (int)( ( OriginalMaxPixelValue *
							( m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowWidth - 1.0 ) / AdjustedMaxPixelValue ) + 1.0 ) );
			TrimBlanks( NumberConvertedToText );
			pCtrlWindowWidth -> SetWindowText( NumberConvertedToText );
			}
		if ( m_pAssignedDiagnosticImage -> m_bEnableGammaCorrection )
			{
			pCtrlGamma = (TomEdit*)m_pWndDlgBar -> GetDlgItem( IDC_EDIT_GAMMA );
			if ( pCtrlGamma != 0 )
				{
				_gcvt( m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_Gamma, 3, NumberConvertedToText );
				if ( pCtrlGamma -> m_DecimalDigitsDisplayed == 0 )
					strcpy( NumberFormat, "%8.0f" );
				else if ( pCtrlGamma -> m_DecimalDigitsDisplayed == 1 )
					strcpy( NumberFormat, "%7.1f" );
				else if ( pCtrlGamma -> m_DecimalDigitsDisplayed == 2 )
					strcpy( NumberFormat, "%6.2f" );
				else if ( pCtrlGamma -> m_DecimalDigitsDisplayed == 3 )
					strcpy( NumberFormat, "%5.3f" );
				else
					strcpy( NumberFormat, "%16.8f" );
				sprintf( NumberConvertedToText, NumberFormat, m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_Gamma );
				TrimBlanks( NumberConvertedToText );
				pCtrlGamma -> SetWindowText( NumberConvertedToText );
				}
			}
		m_pWndDlgBar -> Invalidate();
		m_pWndDlgBar -> UpdateWindow();
		}
}


void CImageView::ResetDiagnosticImage( BOOL bRescaleOnly )
{
	RECT				ClientRect;
	int					ClientWidth;
	int					ClientHeight;
	double				DisplayedPixelsPerMM;

	GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;

	DisplayedPixelsPerMM = (double)( m_pDisplayMonitor -> DesktopCoverageRectangle.right -
										m_pDisplayMonitor -> DesktopCoverageRectangle.left ) /
									(double)m_pDisplayMonitor -> m_MonitorWidthInMM;
	if ( m_pAssignedDiagnosticImage != 0 )
		{
		// This call reloads the original and current IMAGE_GRAYSCALE_SETTING structures with the calibration values if !bRescaleOnly.
		m_pAssignedDiagnosticImage -> ResetImage( ClientWidth, ClientHeight, ( m_DefaultImageSize == IMAGE_VIEW_FULL_SIZE ),
																						DisplayedPixelsPerMM, bRescaleOnly );
		if ( ThisBViewerApp.m_pCurrentStudy != 0 && m_ViewFunction == IMAGE_VIEW_FUNCTION_PATIENT )
			{
			LoadImageAnnotationInfo();
			if ( !bRescaleOnly )
				{
				LoadCurrentImageSettingsIntoEditBoxes();
				}
			}
		if ( LoadImageAsTexture() )
			{
			PrepareImage();
			RepaintFast();
			}
		}
}


void CImageView::UpdateImageGrayscaleDisplay( IMAGE_GRAYSCALE_SETTING *pNewGrayscaleSetting )
{
	if ( m_pAssignedDiagnosticImage != 0 && pNewGrayscaleSetting != 0 )
		{
		memcpy( (char*)&m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting, (char*)pNewGrayscaleSetting, sizeof(IMAGE_GRAYSCALE_SETTING) );
		RepaintFast();
		}
}


void CImageView::ClearDiagnosticImage()
{
	EraseImageAnnotationInfo();
	if ( m_pAssignedDiagnosticImage != 0 )
		{
		m_pAssignedDiagnosticImage = 0;
		m_Mouse.m_pTargetImage = 0;
		RepaintFast();
		if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_PATIENT )
			LogMessage( "Cleared subject study image.", MESSAGE_TYPE_NORMAL_LOG );
		else if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_STANDARD )
			LogMessage( "Cleared standard image.", MESSAGE_TYPE_NORMAL_LOG );
		else if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_REPORT )
			LogMessage( "Cleared report image.", MESSAGE_TYPE_NORMAL_LOG );
		}
}


void CImageView::OnLButtonDown( UINT nFlags, CPoint point )
{
	HCURSOR			hNewCursor;
	CWnd			*pParentWindow;
	
	SetCapture();		// Continue handling mouse input until the LButtonUp event, even if the
						// mouse moves outside this window.
	m_Mouse.OnLButtonDown( nFlags, point );
	hNewCursor = LoadCursor( NULL, IDC_SIZEALL );
	m_hDefaultCursor = SetCursor( hNewCursor );

// ZZZ:  This is temporary functionality for measuring field insertion positions on the report form.
//		 Useful for adding or relocating fields.  It displays the current scaled position in the image name field.
/*
	if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_REPORT )
		{
		char			Msg[ 256 ];
		double			XPos;
		double			YPos;

		// NOTE:  point.x and point.y are pixel numbers relative to the edge of the window's
		// client area.  The pixel offsets are calculated to make the left edge and the
		// bottom of the IMAGE equal zero.  The scale factors depend upon the window size
		// and the image magnification, which MUST be held constant throughout a series
		// of measurements if these values are not to change.  The XPos and YPos values
		// then represent image coordinates for the data field placements.
		XPos = ( 0.8592 * ( (double)point.x - 60.0 ) );			// Scale factors are approximate for text fields.
		YPos = ( 0.8313 * ( (double)( m_WindowSizeInPixels.bottom - point.y ) - 87.0 ) );

		XPos = ( 0.8500 * ( (double)point.x - 60.0 ) );			// Scale factors are approximate for text fields.
		YPos = ( 0.8400 * ( (double)( m_WindowSizeInPixels.bottom - point.y ) - 87.0 ) );

		sprintf( Msg, "%5.1f   %5.1f", XPos, YPos );
		m_pWndDlgBar -> m_EditImageName.SetWindowText( Msg );
		m_pWndDlgBar -> m_EditImageName.Invalidate( TRUE );
		}
*/
	CWnd::OnLButtonDown(nFlags, point);

	// If the ImageFrame window doesn't have the focus, use the click on the
	// image as and indication that it should.
	pParentWindow = GetParent();
	if ( pParentWindow != NULL )
		{
		if ( pParentWindow != GetFocus() )
			pParentWindow -> SetFocus();
		}
}


void CImageView::OnLButtonUp( UINT nFlags, CPoint point )
{
	m_Mouse.OnLButtonUp( nFlags, point );
	SetCursor( m_hDefaultCursor );
	ReleaseCapture();
	CWnd::OnLButtonUp(nFlags, point);
}


void CImageView::OnRButtonDown( UINT nFlags, CPoint point )
{
	TRACKMOUSEEVENT			TrackingInfo;
	HCURSOR					hNewCursor;
	MEASURED_INTERVAL		*pMeasuredInterval;
	double					ScaleFactor;
	double					TranslationX;
	double					TranslationY;
	GLint					ViewportRect[ 4 ];
	long					x, y;
	double					VerticalOrientation;
	double					HorizontalOrientation;

	if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_PATIENT && m_pAssignedDiagnosticImage != 0 )
		{
		// Send notification if the mouse leaves the client window.
		TrackingInfo.cbSize = sizeof( TRACKMOUSEEVENT );
		TrackingInfo.dwFlags = TME_LEAVE;
		TrackingInfo.hwndTrack = this -> GetSafeHwnd();
		TrackMouseEvent( &TrackingInfo );
		
		m_Mouse.OnRButtonDown( nFlags, point );
		if ( m_bEnableMeasure )
			{
			hNewCursor = LoadCursor( NULL, IDC_CROSS );
			m_hDefaultCursor = SetCursor( hNewCursor );

			m_pActiveMeasurementInterval = (MEASURED_INTERVAL*)malloc( sizeof(MEASURED_INTERVAL) );
			if ( m_pActiveMeasurementInterval != 0 )
				{
				glGetIntegerv( GL_VIEWPORT, ViewportRect );

				x = point.x;
				y = point.y;
				if ( m_pAssignedDiagnosticImage -> m_bFlipHorizontally )
					x = ViewportRect[ 2 ] - x;
				if ( m_pAssignedDiagnosticImage -> m_bFlipVertically )
					y = ViewportRect[ 3 ] - y;

				HorizontalOrientation = 1.0;
				if ( m_pAssignedDiagnosticImage -> m_bFlipHorizontally )
					HorizontalOrientation = -1.0;
				VerticalOrientation = 1.0;
				if ( m_pAssignedDiagnosticImage -> m_bFlipVertically )
					VerticalOrientation = -1.0;

				m_pActiveMeasurementInterval -> ScreenStartingPoint.x = x;
				m_pActiveMeasurementInterval -> ScreenStartingPoint.y = y;
				m_pActiveMeasurementInterval -> ScreenEndingPoint.x = x;
				m_pActiveMeasurementInterval -> ScreenEndingPoint.y = y;
				m_pActiveMeasurementInterval -> Distance = 0.0;

				// Translation in screen coordinates.  The focal point is in image coordinates.
				ScaleFactor = m_pAssignedDiagnosticImage -> m_ScaleFactor;
				TranslationX = (double)ViewportRect[ 2 ] / 2.0 -
							(double)m_pAssignedDiagnosticImage -> m_FocalPoint.x * m_pAssignedDiagnosticImage -> m_ScaleFactor;
				TranslationY = (double)ViewportRect[ 3 ] / 2.0 -
							(double)m_pAssignedDiagnosticImage -> m_FocalPoint.y * m_pAssignedDiagnosticImage -> m_ScaleFactor;
				m_pActiveMeasurementInterval -> ScaledStartingPointX = (GLfloat)
							( ( (double)m_pActiveMeasurementInterval -> ScreenStartingPoint.x - HorizontalOrientation * TranslationX ) / ScaleFactor );
				m_pActiveMeasurementInterval -> ScaledStartingPointY = (GLfloat)
							( ( (double)m_pActiveMeasurementInterval -> ScreenStartingPoint.y - VerticalOrientation * TranslationY ) / ScaleFactor );
				m_pActiveMeasurementInterval -> ScaledEndingPointX = m_pActiveMeasurementInterval -> ScaledStartingPointX;
				m_pActiveMeasurementInterval -> ScaledEndingPointY = m_pActiveMeasurementInterval -> ScaledStartingPointY;
				m_pActiveMeasurementInterval -> Distance = 0.0;
				m_pActiveMeasurementInterval -> pNextInterval = 0;
				// Link the new interval to the list.
				if ( m_pMeasuredIntervalList == 0 )
					m_pMeasuredIntervalList = m_pActiveMeasurementInterval;
				else
					{
					pMeasuredInterval = m_pMeasuredIntervalList;
					while ( pMeasuredInterval != 0 && pMeasuredInterval -> pNextInterval != 0 )
						pMeasuredInterval = pMeasuredInterval -> pNextInterval;
					pMeasuredInterval -> pNextInterval = m_pActiveMeasurementInterval;
					}
				}
			RepaintFast();
			}
		else
			LoadCurrentImageSettingsIntoEditBoxes();
		}
	CWnd::OnRButtonDown(nFlags, point);
}


void CImageView::OnRButtonUp( UINT nFlags, CPoint point )
{
	double					TranslationX;
	double					TranslationY;
	double					ScaleFactor;
	GLint					ViewportRect[ 4 ];
	long					x, y;
	double					VerticalOrientation;
	double					HorizontalOrientation;

	if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_PATIENT )
		m_Mouse.OnRButtonUp( nFlags, point );

	if ( m_bEnableMeasure && m_pAssignedDiagnosticImage != 0 )
		{
		SetCursor( m_hDefaultCursor );
		if ( m_pActiveMeasurementInterval != 0 )
			{
			glGetIntegerv( GL_VIEWPORT, ViewportRect );

			x = point.x;
			y = point.y;
			if ( m_pAssignedDiagnosticImage -> m_bFlipHorizontally )
				x = ViewportRect[ 2 ] - x;
			if ( m_pAssignedDiagnosticImage -> m_bFlipVertically )
				y = ViewportRect[ 3 ] - y;

			HorizontalOrientation = 1.0;
			if ( m_pAssignedDiagnosticImage -> m_bFlipHorizontally )
				HorizontalOrientation = -1.0;
			VerticalOrientation = 1.0;
			if ( m_pAssignedDiagnosticImage -> m_bFlipVertically )
				VerticalOrientation = -1.0;

			m_pActiveMeasurementInterval -> ScreenEndingPoint.x = x;
			m_pActiveMeasurementInterval -> ScreenEndingPoint.y = y;

			// The translation is in screen coordinates.  The focal point is in image coordinates.
			ScaleFactor = m_pAssignedDiagnosticImage -> m_ScaleFactor;
			TranslationX = (double)ViewportRect[ 2 ] / 2.0 -
						(double)m_pAssignedDiagnosticImage -> m_FocalPoint.x * m_pAssignedDiagnosticImage -> m_ScaleFactor;
			TranslationY = (double)ViewportRect[ 3 ] / 2.0 -
						(double)m_pAssignedDiagnosticImage -> m_FocalPoint.y * m_pAssignedDiagnosticImage -> m_ScaleFactor;

			m_pActiveMeasurementInterval -> ScaledEndingPointX = (GLfloat)
						( ( (double)m_pActiveMeasurementInterval -> ScreenEndingPoint.x - HorizontalOrientation * TranslationX ) / ScaleFactor );
			m_pActiveMeasurementInterval -> ScaledEndingPointY = (GLfloat)
						( ( (double)m_pActiveMeasurementInterval -> ScreenEndingPoint.y - VerticalOrientation * TranslationY ) / ScaleFactor );
			m_pActiveMeasurementInterval -> Distance =
						sqrt( pow( (double)m_pActiveMeasurementInterval -> ScaledEndingPointX -
									(double)m_pActiveMeasurementInterval -> ScaledStartingPointX, 2.0 ) +
								pow( (double)m_pActiveMeasurementInterval -> ScaledEndingPointY -
									(double)m_pActiveMeasurementInterval -> ScaledStartingPointY, 2.0 )	);
			}
		m_pActiveMeasurementInterval = 0;
		RepaintFast();
		}

	CWnd::OnRButtonUp(nFlags, point);
}


BOOL CImageView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	m_Mouse.OnMouseWheel( nFlags, zDelta, pt );
	if ( m_pAssignedDiagnosticImage != 0 )
		PrepareImage();
	RepaintFast();

	return TRUE;
}


void CImageView::OnMouseMove( UINT nFlags, CPoint point )
{
	CRect					ClientRect;
	double					ScaleFactor;
	double					TranslationX;
	double					TranslationY;
	GLint					ViewportRect[ 4 ];
	long					x, y;
	double					VerticalOrientation;
	double					HorizontalOrientation;

	// Verify that the mouse is over the image.
	GetClientRect( &ClientRect );
	if ( ClientRect.PtInRect( point ) )	
		{
		if ( !m_bTheMouseIsOverTheImage )	
			m_bTheMouseIsOverTheImage = TRUE;		// The mouse just reentered the image.
		} 
	else
		{
		// If the mouse moves off the image, reset the mouse button setting,
		// since it can't easily be tracked when the mouse is over another window.
		m_Mouse.m_ButtonDown = NO_BUTTON;
		m_bTheMouseIsOverTheImage = FALSE;
		}
	// If the mouse is over the image, process the movement.
	if ( m_bTheMouseIsOverTheImage )
		{
		if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_PATIENT && m_Mouse.m_ButtonDown == RIGHT_BUTTON )
			{
			if ( m_bEnableMeasure )
				{
				if ( m_pActiveMeasurementInterval != 0 )
					{
					glGetIntegerv( GL_VIEWPORT, ViewportRect );

					// Translation in screen coordinates.  The focal point is in image coordinates.
					ScaleFactor = m_pAssignedDiagnosticImage -> m_ScaleFactor;
					TranslationX = (double)ViewportRect[ 2 ] / 2.0 -
								(double)m_pAssignedDiagnosticImage -> m_FocalPoint.x * m_pAssignedDiagnosticImage -> m_ScaleFactor;
					TranslationY = (double)ViewportRect[ 3 ] / 2.0 -
								(double)m_pAssignedDiagnosticImage -> m_FocalPoint.y * m_pAssignedDiagnosticImage -> m_ScaleFactor;

					HorizontalOrientation = 1.0;
					if ( m_pAssignedDiagnosticImage -> m_bFlipHorizontally )
						HorizontalOrientation = -1.0;
					VerticalOrientation = 1.0;
					if ( m_pAssignedDiagnosticImage -> m_bFlipVertically )
						VerticalOrientation = -1.0;

					x = point.x;
					y = point.y;
					if ( m_pAssignedDiagnosticImage -> m_bFlipHorizontally )
						x = ViewportRect[ 2 ] - x;
					if ( m_pAssignedDiagnosticImage -> m_bFlipVertically )
						y = ViewportRect[ 3 ] - y;

					m_pActiveMeasurementInterval -> ScreenEndingPoint.x = x;
					m_pActiveMeasurementInterval -> ScreenEndingPoint.y = y;

					m_pActiveMeasurementInterval -> ScaledEndingPointX = (GLfloat)( ( (double)x - HorizontalOrientation * TranslationX ) / ScaleFactor );
					m_pActiveMeasurementInterval -> ScaledEndingPointY = (GLfloat)( ( (double)y - VerticalOrientation * TranslationY ) / ScaleFactor );
					m_pActiveMeasurementInterval -> Distance =
								sqrt( pow( (double)m_pActiveMeasurementInterval -> ScaledEndingPointX -
											(double)m_pActiveMeasurementInterval -> ScaledStartingPointX, 2.0 ) +
										pow( (double)m_pActiveMeasurementInterval -> ScaledEndingPointY -
											(double)m_pActiveMeasurementInterval -> ScaledStartingPointY, 2.0 )	);
					}
				RepaintFast();
				}
			else
				LoadCurrentImageSettingsIntoEditBoxes();
			}
		m_Mouse.OnMouseMove( nFlags, point );
		}
	CWnd::OnMouseMove( nFlags, point );
}


void CImageView::SetImageGrayscalePreference( double MouseHorizontalDisplacement, double MouseVerticalDisplacement )
{
	if ( m_pAssignedDiagnosticImage != 0 )
		{
		m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_RelativeMouseHorizontalPosition = MouseHorizontalDisplacement;
		m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_RelativeMouseVerticalPosition = MouseVerticalDisplacement;
		m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowWidth = ( 1.0 + MouseHorizontalDisplacement / 100.0 ) * m_pAssignedDiagnosticImage -> m_OriginalGrayscaleSetting.m_WindowWidth;
		if ( _stricmp( m_pAssignedDiagnosticImage -> m_pImageCalibrationInfo -> Modality, "CT" ) == 0 )
			m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowCenter = ( 1.0 + MouseVerticalDisplacement / 50.0 ) * m_pAssignedDiagnosticImage -> m_OriginalGrayscaleSetting.m_WindowCenter;
		else	
			m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowCenter = ( 1.0 + MouseVerticalDisplacement / 100.0 ) * m_pAssignedDiagnosticImage -> m_OriginalGrayscaleSetting.m_WindowCenter;

		m_pAssignedDiagnosticImage -> LoadStudyWindowCenterAndWidth();
		}
	m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowMinPixelAmplitude =
				m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowCenter - ( m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowWidth - 1.0 ) / 2.0;
	m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowMaxPixelAmplitude =
				m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowCenter - 0.5 + ( m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowWidth - 1.0 ) / 2.0;
}


void CImageView::OnDestroy()
{
	CWnd::OnDestroy();
	DeallocateMembers();
}



// Set up the vertex array, which consists of two triangles arranged to make a rectangle on the
// display, representing the boundaries of the image to be displayed.
// Also set the texture corners depending upon the rotation setting.
void CImageView::InitImageVertexRectangle( float XMin, float XMax, float YMin, float YMax, float ViewportAspectRatio )
{
	float		FrameWidth;
	float		FrameHeight;
	float		Overhang;

	FrameWidth = XMax - XMin;
	FrameHeight = YMax - YMin;
	Overhang = ( FrameHeight - FrameWidth * ViewportAspectRatio ) / 2.0f;

	m_VertexRectangle.Zbl = 0.0;
	m_VertexRectangle.Zbr = 0.0;
	m_VertexRectangle.Ztl = 0.0;
	m_VertexRectangle.Ztr = 0.0;
	if ( m_pAssignedDiagnosticImage -> m_RotationQuadrant == 0 || m_pAssignedDiagnosticImage -> m_RotationQuadrant == 2 )
		{
		m_VertexRectangle.Xbl = XMin;
		m_VertexRectangle.Ybl = YMin;

		m_VertexRectangle.Xbr = XMax;
		m_VertexRectangle.Ybr = YMin;

		m_VertexRectangle.Xtl = XMin;
		m_VertexRectangle.Ytl = YMax;

		m_VertexRectangle.Xtr = XMax;
		m_VertexRectangle.Ytr = YMax;
		}
	else if ( m_pAssignedDiagnosticImage -> m_RotationQuadrant == 1 || m_pAssignedDiagnosticImage -> m_RotationQuadrant == 3 )
		{
		m_VertexRectangle.Xbl = XMin - Overhang;
		m_VertexRectangle.Ybl = YMin + Overhang;

		m_VertexRectangle.Xbr = XMax + Overhang;
		m_VertexRectangle.Ybr = YMin + Overhang;

		m_VertexRectangle.Xtl = XMin - Overhang;
		m_VertexRectangle.Ytl = YMax - Overhang;

		m_VertexRectangle.Xtr = XMax + Overhang;
		m_VertexRectangle.Ytr = YMax - Overhang;
		}

	if ( m_pAssignedDiagnosticImage -> m_RotationQuadrant == 0 )
		{
		m_VertexRectangle.TXtl = 0.0;
		m_VertexRectangle.TYtl = 1.0;
		m_VertexRectangle.TXbl = 0.0;
		m_VertexRectangle.TYbl = 0.0;
		m_VertexRectangle.TXbr = 1.0;
		m_VertexRectangle.TYbr = 0.0;
		m_VertexRectangle.TXtr = 1.0;
		m_VertexRectangle.TYtr = 1.0;
		}
	else if ( m_pAssignedDiagnosticImage -> m_RotationQuadrant == 1 )
		{
		m_VertexRectangle.TXtl = 0.0;
		m_VertexRectangle.TYtl = 0.0;
		m_VertexRectangle.TXbl = 1.0;
		m_VertexRectangle.TYbl = 0.0;
		m_VertexRectangle.TXbr = 1.0;
		m_VertexRectangle.TYbr = 1.0;
		m_VertexRectangle.TXtr = 0.0;
		m_VertexRectangle.TYtr = 1.0;
		}
	else if ( m_pAssignedDiagnosticImage -> m_RotationQuadrant == 2 )
		{
		m_VertexRectangle.TXtl = 1.0;
		m_VertexRectangle.TYtl = 0.0;
		m_VertexRectangle.TXbl = 1.0;
		m_VertexRectangle.TYbl = 1.0;
		m_VertexRectangle.TXbr = 0.0;
		m_VertexRectangle.TYbr = 1.0;
		m_VertexRectangle.TXtr = 0.0;
		m_VertexRectangle.TYtr = 0.0;
		}
	else if ( m_pAssignedDiagnosticImage -> m_RotationQuadrant == 3 )
		{
		m_VertexRectangle.TXtl = 1.0;
		m_VertexRectangle.TYtl = 1.0;
		m_VertexRectangle.TXbl = 0.0;
		m_VertexRectangle.TYbl = 1.0;
		m_VertexRectangle.TXbr = 0.0;
		m_VertexRectangle.TYbr = 0.0;
		m_VertexRectangle.TXtr = 1.0;
		m_VertexRectangle.TYtr = 0.0;
		}
}


// Set up the rectangle for framing the displayed image.
void CImageView::InitScreenVertexSquareFrame()
{
	m_ScreenVertexRectangle.Xbl = -1;
	m_ScreenVertexRectangle.Ybl = -1;
	m_ScreenVertexRectangle.Zbl = 0.0;

	m_ScreenVertexRectangle.Xbr = 1;
	m_ScreenVertexRectangle.Ybr = -1;
	m_ScreenVertexRectangle.Zbr = 0.0;

	m_ScreenVertexRectangle.Xtl = -1;
	m_ScreenVertexRectangle.Ytl = 1;
	m_ScreenVertexRectangle.Ztl = 0.0;

	m_ScreenVertexRectangle.Xtr = 1;
	m_ScreenVertexRectangle.Ytr = 1;
	m_ScreenVertexRectangle.Ztr = 0.0;

	m_ScreenVertexRectangle.TXtl = 0.0;
	m_ScreenVertexRectangle.TYtl = 1.0;
	m_ScreenVertexRectangle.TXbl = 0.0;
	m_ScreenVertexRectangle.TYbl = 0.0;
	m_ScreenVertexRectangle.TXbr = 1.0;
	m_ScreenVertexRectangle.TYbr = 0.0;
	m_ScreenVertexRectangle.TXtr = 1.0;
	m_ScreenVertexRectangle.TYtr = 1.0;
}



void CImageView::InitCharacterGlyphVertexRectangle( float XMin, float XMax, float YMin, float YMax )
{
	m_CharacterGlyphVertexRectangle.Xbl = XMin;
	m_CharacterGlyphVertexRectangle.Ybl = YMin;
	m_CharacterGlyphVertexRectangle.Zbl = 0.0;

	m_CharacterGlyphVertexRectangle.Xbr = XMax;
	m_CharacterGlyphVertexRectangle.Ybr = YMin;
	m_CharacterGlyphVertexRectangle.Zbr = 0.0;

	m_CharacterGlyphVertexRectangle.Xtl = XMin;
	m_CharacterGlyphVertexRectangle.Ytl = YMax;
	m_CharacterGlyphVertexRectangle.Ztl = 0.0;

	m_CharacterGlyphVertexRectangle.Xtr = XMax;
	m_CharacterGlyphVertexRectangle.Ytr = YMax;
	m_CharacterGlyphVertexRectangle.Ztr = 0.0;

	m_CharacterGlyphVertexRectangle.TXtl = 0.0;
	m_CharacterGlyphVertexRectangle.TYtl = 1.0;
	m_CharacterGlyphVertexRectangle.TXbl = 0.0;
	m_CharacterGlyphVertexRectangle.TYbl = 0.0;
	m_CharacterGlyphVertexRectangle.TXbr = 1.0;
	m_CharacterGlyphVertexRectangle.TYbr = 0.0;
	m_CharacterGlyphVertexRectangle.TXtr = 1.0;
	m_CharacterGlyphVertexRectangle.TYtr = 1.0;
}


void CImageView::FlipFrameHorizontally()
{
	GLfloat			Temp;

	Temp = m_VertexRectangle.TXtl;
	m_VertexRectangle.TXtl = m_VertexRectangle.TXtr;
	m_VertexRectangle.TXtr = Temp;
	Temp = 	m_VertexRectangle.TXbl;
	m_VertexRectangle.TXbl = m_VertexRectangle.TXbr;
	m_VertexRectangle.TXbr = Temp;
}


void CImageView::FlipFrameVertically()
{
	GLfloat			Temp;

	Temp = m_VertexRectangle.TYtl;
	m_VertexRectangle.TYtl = m_VertexRectangle.TYbl;
	m_VertexRectangle.TYbl = Temp;
	Temp = 	m_VertexRectangle.TYbr;
	m_VertexRectangle.TYbr = m_VertexRectangle.TYtr;
	m_VertexRectangle.TYtr = Temp;
}


// 
// This vertex shader positions the rectangular edges of the displayed window into gl_Position.
// The image rectangle, which can be smaller or larger than the displayed window, is represented
// by the texture coordinates output as TexCoord.  The geometrical transformation matrix
// adjusts the texture coordinates for changes in the image position, scaling and rotation.
//
const GLchar		VertexShaderWithTextureSourceCode[] =
"#version	330 core									\n"
"														\n"
"														\n"
"layout (location = 0) in vec3 inVertexCoordinates;		\n"
"layout (location = 1) in vec2 inTextureCoordinates;	\n"
"														\n"
"out	vec2	TexCoord;								\n"
"														\n"
"														\n"
"void main( void )										\n"
"{														\n"
"														\n"
"	gl_Position = vec4( inVertexCoordinates, 1.0f );	\n"
"														\n"
"	TexCoord = inTextureCoordinates;					\n"
"														\n"
"}														\0";


// 
// This fragment shader determines the color to be rendered at a given point on the display.
// It performs the grayscale windowing adjustsments, grayscale inversion if requested, and
// adjustments for the display gamma setting.
//
const GLchar		FragmentShaderFor30BitColorSourceCode[] =
"#version	330 core									\n"
"														\n"
"in		vec2	TexCoord;								\n"	// Get the x and y coordinate of the current texel (in the range [0, 1]).
"														\n"
"out	vec4	FragColor;								\n"
"														\n"
// The following global parameters are set by the application.
"uniform sampler2D LoadeddImageTexture;					\n" // Reference to the 2D texture grayscale image:
"														\n"
"uniform vec2		ImageSize;							\n" // The size (in texels) of the image texture.
"uniform float		WindowMin = 0.0;					\n" // Needs to be set from Window width and level.
"uniform float		WindowMax = 0.0;					\n" // Needs to be set from Window width and level.
"uniform float		GammaValue = 1.0;					\n"
"uniform int		bWindowingIsSigmoidal = 0;			\n"
"uniform int		bInvertGrayscale = 0;				\n"
"uniform float		MaxGrayIndex = 4096.0;				\n" // Needs to be set if not 12-bit grayscale.
"														\n"
"void main( void )										\n"
"{														\n"
"	float		fRawGrayIndex;							\n"
"	float		fUpScaledGrayIndex;						\n"
"	float		fGrayIndex;								\n"
"	float		fCorrectedGrayIndex;					\n"
"	float		normalizer = 1.0 / MaxGrayIndex;		\n" // The index into the lookup table is normalized into the range [0, 1].
"	vec4		fRawGrayColor;							\n"

// Read the 16-bit UINT value from the alpha component.
"	fRawGrayColor = texture( LoadeddImageTexture, TexCoord );	\n"
"	fRawGrayIndex = fRawGrayColor.r;					\n"
"	fUpScaledGrayIndex = fRawGrayIndex * MaxGrayIndex;	\n"
// Apply VOI windowing, if specified.
"	if ( WindowMin != 0.0 && WindowMax > 1.0 )			\n"
"		{												\n"
"		if ( bWindowingIsSigmoidal != 0 )               \n"
"			{											\n"
"			fUpScaledGrayIndex = MaxGrayIndex / ( 1.0 + exp( -4.0 * ( fUpScaledGrayIndex - ( WindowMax - WindowMin ) / 2.0 ) /( WindowMax - WindowMin ) ) );  \n"
"			}											\n"
"		else											\n"
"			{											\n"
"			if ( fUpScaledGrayIndex <= WindowMin )		\n"
"				fUpScaledGrayIndex = 0.0;				\n"
"			else if ( fUpScaledGrayIndex > WindowMax )	\n"
"				fUpScaledGrayIndex = MaxGrayIndex;		\n"
"			else										\n"
"				fUpScaledGrayIndex = ( MaxGrayIndex / ( WindowMax - WindowMin ) ) * ( fUpScaledGrayIndex - WindowMin );	\n"
"			}											\n"
"		}												\n"
"	fGrayIndex = fUpScaledGrayIndex * normalizer;		\n"
"	fCorrectedGrayIndex = pow( fGrayIndex, GammaValue );	\n"
"	if ( bInvertGrayscale != 0 )							\n"
"		fCorrectedGrayIndex = 1.0 - fCorrectedGrayIndex;	\n"
"	vec4		Gray = vec4( fCorrectedGrayIndex, fCorrectedGrayIndex, fCorrectedGrayIndex, 1.0f );		\n"
"	FragColor = Gray;									\n"  // Paint the pixel color into the frame buffer.
"}";



// 
// This fragment shader determines the color to be rendered at a given point on the display.
// This shader is almost identical to the 30-bit color shader above, except that it processes
// the output grayscale value through a lookup table to pack it into RGB color pixels.
// This shader performs the grayscale windowing adjustsments, grayscale inversion if requested,
// and adjustments for the display gamma setting.
//
const GLchar		FragmentShaderFor10BitGrayscaleSourceCode[] =
"#version	330 core									\n"
"														\n"
"in		vec2	TexCoord;								\n"	// Get the x and y coordinate of the current texel (in the range [0, 1]).
"														\n"
"out	vec4	FragColor;								\n"
"														\n"
"uniform sampler2D GrayscaleImageTexture;				\n" // Reference to the 2D texture grayscale image:
														//		Set by app to texture unit 0, containing unsigned integer samples.
"uniform sampler1D	RGBLookupTable;						\n" // Reference to the 1D texture lookup table.
														//		Set by app to texture unit 1, normalized float sampler.

// The following global parameters are set by the application.
"uniform vec2		ImageSize;							\n" // The size (in texels) of the image texture.
"uniform float		WindowMin = 0.0;					\n" // Needs to be set from Window width and level.
"uniform float		WindowMax = 0.0;					\n" // Needs to be set from Window width and level.
"uniform float		GammaValue = 1.0;					\n"
"uniform int		bWindowingIsSigmoidal = 0;			\n"
"uniform int		bInvertGrayscale = 0;				\n"
"uniform float		MaxGrayIndex = 4096.0;				\n" // Needs to be set if not 12-bit grayscale.
"														\n"
"void main( void )										\n"
"{														\n"
"	float		fRawGrayIndex;							\n"
"	float		fUpScaledGrayIndex;						\n"
"	float		fGrayIndex;								\n"
"	float		fCorrectedGrayIndex;					\n"
"	float		normalizer = 1.0 / MaxGrayIndex;		\n" // The index into the lookup table is normalized into the range [0, 1].
"	vec4		fRawGrayColor;							\n"

// Read the 16-bit UINT value from the alpha component.
"	fRawGrayColor = texture( GrayscaleImageTexture, TexCoord );	\n"
"	fRawGrayIndex = fRawGrayColor.r;					\n"
"	fUpScaledGrayIndex = fRawGrayIndex * MaxGrayIndex;	\n"
// Apply VOI windowing, if specified.
"	if ( WindowMin != 0.0 && WindowMax > 1.0 )			\n"
"		{												\n"
"		if ( bWindowingIsSigmoidal != 0 )               \n"
"			{											\n"
"			fUpScaledGrayIndex = MaxGrayIndex / ( 1.0 + exp( -4.0 * ( fUpScaledGrayIndex - ( WindowMax - WindowMin ) / 2.0 ) /( WindowMax - WindowMin ) ) );  \n"
"			}											\n"
"		else											\n"
"			{											\n"
"			if ( fUpScaledGrayIndex <= WindowMin )		\n"
"				fUpScaledGrayIndex = 0.0;				\n"
"			else if ( fUpScaledGrayIndex > WindowMax )	\n"
"				fUpScaledGrayIndex = MaxGrayIndex;		\n"
"			else										\n"
"				fUpScaledGrayIndex = ( MaxGrayIndex / ( WindowMax - WindowMin ) ) * ( fUpScaledGrayIndex - WindowMin );	\n"
"			}											\n"
"		}												\n"
"	fGrayIndex = fUpScaledGrayIndex * normalizer;		\n"
"	fCorrectedGrayIndex = pow( fGrayIndex, GammaValue );\n"
"	if ( fCorrectedGrayIndex < 0.0f )					\n"
"		fCorrectedGrayIndex = 0.0f;						\n"
"	if ( fCorrectedGrayIndex > 0.999f )				\n"
"		fCorrectedGrayIndex = 0.999f;					\n"
"	if ( bInvertGrayscale != 0 )						\n"
"		fCorrectedGrayIndex = 1.0 - fCorrectedGrayIndex;\n"
// Get the corresponding rgba value from the lookup table, using an index in the range [0, 1].
"	vec4		Gray = vec4( texture( RGBLookupTable, fCorrectedGrayIndex ) );	\n"
"	FragColor = Gray.rgba;								\n"  // Paint the pixel color onto the display.
"}";



// 
// This fragment shader samples the image texture copied from the intermediate frame buffer
// and outputs the grayscale value as the FragColor vector.
//
const GLchar		ScreenFragmentShaderSourceCode[] =
"#version	330 core									\n"
"														\n"
"														\n"
"in	vec2	TexCoord;									\n"
"														\n"
"out	vec4	FragColor;								\n"
"														\n"
"uniform sampler2D Pass2ScreenTexture;					\n"
"														\n"
"void main( void )										\n"
"{														\n"
"	FragColor = texture( Pass2ScreenTexture, TexCoord );\n"  // Paint the pixel color onto the display.
"}														\0";


// 
// This fragment shader samples the annotation character glyph texture to be rendered and outputs the grayscale
// value as the FragColor vector.  The color to be painted is passed in as the 3-vector TextColor.
//
const GLchar		ImageAnnotationFragmentShaderSourceCode[] =
"#version	330 core									\n"
"														\n"
"														\n"
"in	vec2	TexCoord;									\n"
"														\n"
"out	vec4	FragColor;								\n"
"														\n"
"uniform sampler2D AnnotationGlyphTexture;				\n"
"uniform vec3 TextColor;								\n"
"														\n"
"void main( void )										\n"
"{														\n"
"														\n"
"	vec4 TextureColor = vec4( 1.0, 1.0, 1.0, 4.0 * texture( AnnotationGlyphTexture, TexCoord ).r );	\n"
"														\n"
"	FragColor = vec4( TextColor, 1.0 ) * TextureColor;	\n"
"}														\0";


// 
// This fragment shader samples the image measurement character glyph texture to be rendered and outputs the grayscale
// value as the FragColor vector.  The color to be painted is passed in as the 3-vector TextColor.
//
const GLchar		ImageMeasurementFragmentShaderSourceCode[] =
"#version	330 core									\n"
"														\n"
"														\n"
"in	vec2	TexCoord;									\n"
"														\n"
"out	vec4	FragColor;								\n"
"														\n"
"uniform sampler2D	MeasurementGlyphTexture;			\n"
"uniform vec3 TextColor;								\n"
"														\n"
"void main( void )										\n"
"{														\n"
"														\n"
"	vec4 TextureColor = vec4( 1.0, 1.0, 1.0, 4.0 * texture( MeasurementGlyphTexture, TexCoord ).r );	\n"
"														\n"
"	FragColor = vec4( TextColor, 1.0 ) * TextureColor;	\n"
"}														\0";


const GLchar		VertexShaderWithoutTextureSourceCode[] =
"#version	330 core									\n"
"														\n"
"														\n"
"layout (location = 0) in vec2 inVertexCoordinates;		\n"
"														\n"
"void main( void )										\n"
"{														\n"
"																						\n"
"	gl_Position = vec4( inVertexCoordinates, 0.0f, 1.0f );								\n"
"																						\n"
"}														\0";


// 
// This fragment shader draws the line by outputing the grayscale color value as the FragColor vector.
//
const GLchar		LineDrawingFragmentShaderSourceCode[] =
"#version	330 core									\n"
"														\n"
"														\n"
"out	vec4	FragColor;								\n"
"														\n"
"uniform vec3 DrawingColor;								\n"
"														\n"
"void main( void )										\n"
"{														\n"
"														\n"
"	FragColor = vec4( DrawingColor, 1.0 );				\n"
"														\n"
"}														\0";


// 
// This fragment shader samples the report character glyph texture to be rendered and outputs the grayscale
// value as the FragColor vector.  The color to be painted is passed in as the 3-vector TextColor.
//
const GLchar		ImageReportTextFragmentShaderSourceCode[] =
"#version	330 core									\n"
"														\n"
"														\n"
"in	vec2	TexCoord;									\n"
"														\n"
"out	vec4	FragColor;								\n"
"														\n"
"uniform sampler2D ReportGlyphTexture;					\n"
"uniform vec3 TextColor;								\n"
"														\n"
"void main( void )										\n"
"{														\n"
"														\n"
"	vec4 TextureColor = vec4( 1.0, 1.0, 1.0, 4.0 * texture( ReportGlyphTexture, TexCoord ).r );	\n"
"														\n"
"	FragColor = vec4( TextColor, 1.0 ) * TextureColor;	\n"
"}														\0";


// 
// This fragment shader samples the report signature bitmap texture and outputs it to the current
// framebuffer.
//
const GLchar		ReportSignatureFragmentShaderSourceCode[] =
"#version	330 core									\n"
"														\n"
"														\n"
"in	vec2	TexCoord;									\n"
"														\n"
"out	vec4	FragColor;								\n"
"														\n"
"uniform sampler2D ReportSignatureTexture;				\n"
"														\n"
"void main( void )										\n"
"{														\n"
"	FragColor = texture( ReportSignatureTexture, TexCoord );\n"  // Paint the pixel color onto the display.
"}														\0";


// 
// This fragment shader samples the report signature bitmap texture and outputs it to the current
// framebuffer.
//
const GLchar		ReportFormFragmentShaderSourceCode[] =
"#version	330 core									\n"
"														\n"
"														\n"
"in	vec2	TexCoord;									\n"
"														\n"
"out	vec4	FragColor;								\n"
"														\n"
"uniform sampler2D ReportFormTexture;					\n"
"														\n"
"void main( void )										\n"
"{														\n"
"	FragColor = texture( ReportFormTexture, TexCoord );	\n"  // Paint the pixel color onto the display.
"}														\0";





BOOL CImageView::LoadGPUShaderPrograms()
{
	BOOL				bNoError = TRUE;

	// Create a shader program for rendering the extended pixel format to the off-screen framebuffer.
	bNoError = PrepareGPUShaderProgram( (char*)VertexShaderWithTextureSourceCode, (char*)FragmentShaderFor30BitColorSourceCode, &m_g30BitColorShaderProgram );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	// Create a shader program for rendering to the display the extended pixel format texture in the off-screen framebuffer.
	bNoError = PrepareGPUShaderProgram( (char*)VertexShaderWithTextureSourceCode, (char*)ScreenFragmentShaderSourceCode, &m_g30BitScreenShaderProgram );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	// Create a shader program for rendering to the 10-bit grayscale display.
	bNoError = PrepareGPUShaderProgram( (char*)VertexShaderWithTextureSourceCode, (char*)FragmentShaderFor10BitGrayscaleSourceCode, &m_g10BitGrayscaleShaderProgram );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	// Create a shader program for rendering the image annotations.
	bNoError = PrepareGPUShaderProgram( (char*)VertexShaderWithTextureSourceCode, (char*)ImageAnnotationFragmentShaderSourceCode, &m_gImageAnnotationShaderProgram );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	// Create a shader program for rendering the image measurement text glyphs.
	bNoError = PrepareGPUShaderProgram( (char*)VertexShaderWithTextureSourceCode, (char*)ImageMeasurementFragmentShaderSourceCode, &m_gImageMeasurementShaderProgram );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	// Create a shader program for rendering the image measurement lines.
	bNoError = PrepareGPUShaderProgram( (char*)VertexShaderWithoutTextureSourceCode, (char*)LineDrawingFragmentShaderSourceCode, &m_gLineDrawingShaderProgram );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	// Create a shader program for rendering the report text characters.
	bNoError = PrepareGPUShaderProgram( (char*)VertexShaderWithTextureSourceCode, (char*)ImageReportTextFragmentShaderSourceCode, &m_gReportTextShaderProgram );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	// Create a shader program for rendering the report signature bitmap.
	bNoError = PrepareGPUShaderProgram( (char*)VertexShaderWithTextureSourceCode, (char*)ReportSignatureFragmentShaderSourceCode, &m_gReportSignatureShaderProgram );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	// Create a shader program for rendering the report form (empty or with the information filled in) as a texture.
	bNoError = PrepareGPUShaderProgram( (char*)VertexShaderWithTextureSourceCode, (char*)ReportFormFragmentShaderSourceCode, &m_gReportFormShaderProgram );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	return bNoError;
}


// Compile the two shader's sourde code.  Associate the shaders with the designated GPU program.  Link the shaders into the program.
BOOL CImageView::PrepareGPUShaderProgram( char *pVertexShaderSourceCode, char *pFragmentShaderSourceCode, GLuint *pShaderProgram )
{
	BOOL				bNoError = TRUE;
	GLuint				hVertexShader;
	GLuint				hFragmentShader;
	const char			*pShaderSourceCode;
	GLint				bSuccessfulCompilation;
	GLint				bSuccessfulLink;
	GLint				bSuccessfulProgramValidation;
	int					StringLength;
	char				*pLogText;
	char				Msg[ 256 ];
	
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	// Create the program object that will contain the shader.
	*pShaderProgram = glCreateProgram();
	bNoError = ( *pShaderProgram != 0 );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	if ( bNoError )
		{
		// Create a vertex shader object.
		hVertexShader = glCreateShader( GL_VERTEX_SHADER );
		CheckOpenGLResultAt( __FILE__, __LINE__ );
		bNoError = ( hVertexShader != 0 );
		if ( bNoError )
			{
			// Specify the shader source code.
			pShaderSourceCode = (char*)pVertexShaderSourceCode;
			glShaderSource( hVertexShader, 1, &pShaderSourceCode, NULL );
			CheckOpenGLResultAt( __FILE__, __LINE__	);
			// Compile the shader source code.
			glCompileShader( hVertexShader );
			CheckOpenGLResultAt( __FILE__, __LINE__	);
			bSuccessfulCompilation = 0;
			glGetShaderiv( hVertexShader, GL_COMPILE_STATUS, &bSuccessfulCompilation );
			if ( bSuccessfulCompilation )
				{
				// Attach the successfully compiled shader to the program object.
				CheckOpenGLResultAt( __FILE__, __LINE__ );
				glAttachShader( *pShaderProgram, hVertexShader );
				CheckOpenGLResultAt( __FILE__, __LINE__ );
				}
			else
				{
				bNoError = FALSE;
				StringLength = 0;
				glGetShaderiv( hVertexShader, GL_INFO_LOG_LENGTH, &StringLength );
				if ( StringLength > 0 )
					{
					pLogText = (char*)malloc( StringLength );
					if ( pLogText != 0 )
						{
						glGetShaderInfoLog( hVertexShader, StringLength, &StringLength, pLogText );
						LogMessage( "Shader compilation results log:", MESSAGE_TYPE_SUPPLEMENTARY );
						LogMessage( pLogText, MESSAGE_TYPE_SUPPLEMENTARY );
						free( pLogText );
						}
					}
				}
			}
		}
	if ( bNoError )
		{
		// Create a fragment shader object.
		hFragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		CheckOpenGLResultAt( __FILE__, __LINE__ );
		bNoError = ( hFragmentShader != 0 );
		if ( bNoError )
			{
			// Specify the shader source code.
			pShaderSourceCode = (char*)pFragmentShaderSourceCode;
			glShaderSource( hFragmentShader, 1, &pShaderSourceCode, NULL );
			CheckOpenGLResultAt( __FILE__, __LINE__	);
			// Compile the shader source code.
			glCompileShader( hFragmentShader );
			CheckOpenGLResultAt( __FILE__, __LINE__	);
			bSuccessfulCompilation = 0;
			glGetShaderiv( hFragmentShader, GL_COMPILE_STATUS, &bSuccessfulCompilation );
			if ( bSuccessfulCompilation )
				{
				// Attach the successfully compiled shader to the program object.
				CheckOpenGLResultAt( __FILE__, __LINE__ );
				glAttachShader( *pShaderProgram, hFragmentShader );
				CheckOpenGLResultAt( __FILE__, __LINE__ );
				}
			else
				{
				bNoError = FALSE;
				StringLength = 0;
				glGetShaderiv( hFragmentShader, GL_INFO_LOG_LENGTH, &StringLength );
				if ( StringLength > 0 )
					{
					pLogText = (char*)malloc( StringLength );
					if ( pLogText != 0 )
						{
						glGetShaderInfoLog( hFragmentShader, StringLength, &StringLength, pLogText );
						LogMessage( "Shader compilation results log:", MESSAGE_TYPE_SUPPLEMENTARY );
						LogMessage( pLogText, MESSAGE_TYPE_SUPPLEMENTARY );
						free( pLogText );
						}
					}
				}
			}
		}
	if ( bNoError )
		{
		// Link the shader program.
		glLinkProgram( *pShaderProgram );
		CheckOpenGLResultAt( __FILE__, __LINE__	);
		// Respond to any shader program linking errors.
		bSuccessfulLink = 0;
		glGetProgramiv( *pShaderProgram, GL_LINK_STATUS, &bSuccessfulLink );
		CheckOpenGLResultAt( __FILE__, __LINE__	);
		if ( !bSuccessfulLink )
			{
			bNoError = FALSE;
			StringLength = 0;
			glGetProgramiv( *pShaderProgram, GL_INFO_LOG_LENGTH, &StringLength );
			if ( StringLength > 0 )
				{
				pLogText = (char*)malloc( StringLength );
				if ( pLogText != 0 )
					{
					glGetProgramInfoLog( *pShaderProgram, StringLength, &StringLength, pLogText );
					LogMessage( "Shader linking results log:", MESSAGE_TYPE_SUPPLEMENTARY );
					LogMessage( pLogText, MESSAGE_TYPE_SUPPLEMENTARY );
					free( pLogText );
					}
				}
			}
		}
	sprintf( Msg, "Shader Load Completion at:  %s( %d )", __FILE__, __LINE__ );
	LogMessageAt( __FILE__, __LINE__, Msg, MESSAGE_TYPE_SUPPLEMENTARY );

	glDeleteShader( hVertexShader );			// The source code is no longer needed;  The GPU has the compiled version.
	glDeleteShader( hFragmentShader );
	CheckOpenGLResultAt( __FILE__, __LINE__	);

	bNoError = glIsProgram( *pShaderProgram );
	CheckOpenGLResultAt( __FILE__, __LINE__ );
	glUseProgram( *pShaderProgram );
	CheckOpenGLResultAt( __FILE__, __LINE__ );

	glValidateProgram(  *pShaderProgram );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	glGetProgramiv(  *pShaderProgram, GL_VALIDATE_STATUS, &bSuccessfulProgramValidation );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	if ( !bSuccessfulProgramValidation )
		{
		bNoError = FALSE;
		StringLength = 0;
		glGetProgramiv(  *pShaderProgram, GL_INFO_LOG_LENGTH, &StringLength );
		if ( StringLength > 0 )
			{
			pLogText = (char*)malloc( StringLength );
			if ( pLogText != 0 )
				{
				glGetProgramInfoLog(  *pShaderProgram, StringLength, &StringLength, pLogText );
				LogMessage( "Shader program validation results log:", MESSAGE_TYPE_SUPPLEMENTARY );
				LogMessage( pLogText, MESSAGE_TYPE_SUPPLEMENTARY );
				free( pLogText );
				}
			}
		}
	glUseProgram( 0 );
	CheckOpenGLResultAt( __FILE__, __LINE__ );

	return bNoError;
}


