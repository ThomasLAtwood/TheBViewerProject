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
	m_pSavedDisplay = 0;
	m_SavedDisplayWidth = 0;
	m_SavedDisplayHeight = 0;
	m_hDefaultCursor = 0;
	m_Mouse.m_pImageView = (CWnd*)this;
	m_bTheMouseIsOverTheImage = FALSE;
	m_TimeOfLastFastPaint = 0;
	m_TimeOfLastPaint = 0;
	m_DefaultImageSize = IMAGE_VIEW_FIT_TO_SCREEN;
	m_pWndDlgBar = 0;
	m_pWindowingTableScaled8Bit = 0;
	m_pInversionTableScaled8Bit = 0;
	m_bScaleToTextureBuffer = FALSE;
	m_PageNumber = 1;
	m_ImageDisplayMethod = IMAGE_DISPLAY_UNSPECIFIED;
	m_glImageTextureId = 0;
	InitSquareFrame();
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
}


CImageView::~CImageView()
{
	DeallocateMembers();
}


void CImageView::DeallocateMembers()
{
	EraseImageAnnotationInfo();
	glDeleteTextures( 1, &m_glImageTextureId );
	// Deselect the current rendering context and delete it.
	wglMakeCurrent( m_hDC, NULL );

	if ( m_pWindowingTableScaled8Bit != 0 )
		{
		free( m_pWindowingTableScaled8Bit );
		m_pWindowingTableScaled8Bit = 0;
		}
	if ( m_pInversionTableScaled8Bit != 0 )
		{
		free( m_pInversionTableScaled8Bit );
		m_pInversionTableScaled8Bit = 0;
		}
	if ( m_pSavedDisplay != 0 )
		{
		free( m_pSavedDisplay );
		m_pSavedDisplay = 0;
		}
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




// Select the pixel format for a given device context
void CImageView::SetDCPixelFormat( HDC hDC )
{
	PIXELFORMATDESCRIPTOR pfd =
		{
		sizeof( PIXELFORMATDESCRIPTOR ),// Size of this structure
		1,								// Version of this structure	
		PFD_DRAW_TO_WINDOW |			// Draw to Window (not to bitmap)
		PFD_SUPPORT_OPENGL |			// Support OpenGL calls in window
		PFD_DOUBLEBUFFER |				// Double buffered mode
		PFD_DEPTH_DONTCARE |			// A depth buffer is not needed.
		PFD_SWAP_EXCHANGE,				// When swapping, exchange the buffer pointers.
		PFD_TYPE_RGBA,					// RGBA Color mode
		32,								// Want 32 bit color 
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

	// Select the pixel format
	SetDCPixelFormat( m_hDC );		

	EstablishImageDisplayMode();

	LogMessage( "Image view created.", MESSAGE_TYPE_SUPPLEMENTARY );

	return 0;
}


void CImageView::EstablishImageDisplayMode()
{
	RECT				ClientRect;
	int					ClientWidth;
	int					ClientHeight;
	CGraphicsAdapter	*pGraphicsAdapter;
	char				Msg[ 256 ];
	char				DisplayMethod[ 256 ];
	int					nGrayValue;
	double				CorrectedGrayValue;
	
	pGraphicsAdapter = 0;
	m_ImageDisplayMethod = IMAGE_DISPLAY_SLOW;	// Set default display mode.
	m_PrevImageDisplayMethod = IMAGE_DISPLAY_UNSPECIFIED;
	if ( m_pDisplayMonitor != 0 )
		{
		pGraphicsAdapter = (CGraphicsAdapter*)m_pDisplayMonitor -> m_pGraphicsAdapter;
		if ( pGraphicsAdapter != 0 )
			{
			// The following call makes the rendering context current.  The rendering context
			// is created/assigned by the graphics adapter object.
			m_hRC = pGraphicsAdapter -> CheckOpenGLCapabilities( m_hDC );
			if ( pGraphicsAdapter -> m_OpenGLSupportLevel & OPENGL_SUPPORT_PRIMITIVE )
				{
				m_ImageDisplayMethod = IMAGE_DISPLAY_SLOW;
				strcpy( DisplayMethod, "8-bit operating system native OpenGL" );
				}
			if ( pGraphicsAdapter -> m_OpenGLSupportLevel & OPENGL_SUPPORT_TEXTURES )
				{
				m_ImageDisplayMethod = IMAGE_DISPLAY_USING_8BIT_TEXTURE;
				strcpy( DisplayMethod, "8-bit accelerated OpenGL using texture" );
				}
			if ( pGraphicsAdapter -> m_OpenGLSupportLevel & OPENGL_SUPPORT_PIXEL_PACK )
				{
				// Support is available for using the shader for 8-bit, 10-bit or 12-bit grayscale images.
				// Both the graphics adapter capabilities and the display monitor capabilities
				// must be examined, in order to determine the actual display mode.
				if ( m_pDisplayMonitor -> m_GrayScaleBitDepth <= 8 )
					// As a minimum, at least 8-bit "pixel packing" is supported, although it
					// is not very meaningful.
					{
					m_ImageDisplayMethod = IMAGE_DISPLAY_USING_PACKED_8BIT;
					strcpy( DisplayMethod, "8-bit packed pixel mode" );
					}
				else if ( m_pDisplayMonitor -> m_GrayScaleBitDepth == 10 )
					{
					m_ImageDisplayMethod = IMAGE_DISPLAY_USING_PACKED_10BIT;
					strcpy( DisplayMethod, "10-bit packed pixel mode" );
					}
				else if ( m_pDisplayMonitor -> m_GrayScaleBitDepth == 12 )
					{
					m_ImageDisplayMethod = IMAGE_DISPLAY_USING_PACKED_12BIT;
					strcpy( DisplayMethod, "12-bit packed pixel mode" );
					}
				else if ( m_pDisplayMonitor -> m_GrayScaleBitDepth == 16 )
					{
					m_ImageDisplayMethod = IMAGE_DISPLAY_USING_PACKED_16BIT;
					strcpy( DisplayMethod, "16-bit packed pixel mode" );
					}
				}
			}
		else
			LogMessage( ">>> No graphics adapter set for this display monitor.", MESSAGE_TYPE_SUPPLEMENTARY );

		}
	GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;

	if ( pGraphicsAdapter != 0 )
		{
		sprintf( Msg, "  Using OpenGL version %s", pGraphicsAdapter -> m_OpenGLVersion );
		switch ( m_ViewFunction )
			{
			case IMAGE_VIEW_FUNCTION_PATIENT:
				strcat( Msg, " on subject study image display." );
				m_pWindowingTableScaled8Bit = (GLfloat*)malloc( 256 * sizeof(GLfloat) );
				m_pInversionTableScaled8Bit = (GLfloat*)malloc( 256 * sizeof(GLfloat) );
				if ( m_pWindowingTableScaled8Bit != 0 )
					LoadWindowingConversionTable( 256.0, 128.0, 1.0 );
				if ( m_pInversionTableScaled8Bit != 0 )
					{
					for ( nGrayValue = 0; nGrayValue < 256; nGrayValue++ )
						{
						CorrectedGrayValue =  1.0 - ( (double)nGrayValue / 255.0 );
						m_pInversionTableScaled8Bit[ nGrayValue ] = (GLfloat)CorrectedGrayValue;
						}
					}
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

		// Reset the coordinate system before modifying
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();

		// Set the clipping volume
		gluOrtho2D( 0.0f, (GLfloat)glWindowWidthInPixels, 0.0f, (GLfloat)glWindowHeightInPixels );
        
		// This will leave the current matrix mode as MODELVIEW.  In BViewer, this is the only matrix
		// mode that is referenced.
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
		}

	return bViewportIsValid;
}


// This table is only used for 8-bit conversions.  The shader handles windowing
// for greater bit depths.
void CImageView::LoadWindowingConversionTable( double WindowWidth, double WindowLevel, double GammaValue )
{
	int					nGrayValue;
	double				CorrectedGrayValue;
	double				WindowMin;
	double				WindowMax;
	double				MaxGrayscaleValue;

	if ( m_pWindowingTableScaled8Bit != 0 && m_pAssignedDiagnosticImage != 0 ) // && m_pAssignedDiagnosticImage -> m_ImageBitDepth == 8 )
		{
		MaxGrayscaleValue = (double)m_pAssignedDiagnosticImage -> m_MaxGrayscaleValue;
		// Account for image grayscales scaled down to 8 bits.
		if ( MaxGrayscaleValue > 255.0 )
			{
			WindowWidth *= 256.0 / MaxGrayscaleValue;
			WindowLevel *= 256.0 / MaxGrayscaleValue;
			MaxGrayscaleValue = 255.0;
			}
		WindowMin = WindowLevel - 0.5 - ( WindowWidth - 1.0 ) / 2.0;
		WindowMax = WindowLevel - 0.5 + ( WindowWidth - 1.0 ) / 2.0;
		for ( nGrayValue = 0; nGrayValue < 256; nGrayValue++ )
			{
			if ( pBViewerCustomization -> m_WindowingAlgorithmSelection == SELECT_LINEAR_WINDOWING )
				{
				if ( (double)nGrayValue <= WindowMin )
					CorrectedGrayValue = 0.0;
				else
					{
					if ( (double)nGrayValue > WindowMax )
						CorrectedGrayValue = MaxGrayscaleValue;
					else
						CorrectedGrayValue = ( ( (double)nGrayValue - ( WindowLevel - 0.5 ) ) / ( WindowWidth - 1 ) + 0.5 ) * MaxGrayscaleValue;
					}
				}
			else if ( pBViewerCustomization -> m_WindowingAlgorithmSelection == SELECT_SIGMOID_WINDOWING )
				{
				CorrectedGrayValue = 256.0 / ( 1.0 + exp( -4.0 * ( (double)nGrayValue - WindowLevel ) / WindowWidth ) );
				}
			// Fold in the gamma conversion, if any.
			CorrectedGrayValue =  pow( (double)CorrectedGrayValue / 256.0, GammaValue );

			if ( CorrectedGrayValue < 0.0 )
				CorrectedGrayValue = 0.0;
			if ( CorrectedGrayValue > 1.0 )
				CorrectedGrayValue = 1.0;
			m_pWindowingTableScaled8Bit[ nGrayValue ] = (GLfloat)CorrectedGrayValue;
			}
		m_pWndDlgBar -> Invalidate();
		m_pWndDlgBar -> UpdateWindow();
		}
}


// Called from CImageFrame::OnButtonShowHistogram().
// Calculate a full histogram accurate for each pixel luminosity, along with a
// viewable histogram of 128 bins, the latter of which reflects the current
// windowing settings.
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
						AdjustedPixelLuminosity = (double)m_pWindowingTableScaled8Bit[ (unsigned int)InputPixelValue ] * (double)MaxGrayscaleValue;
						}
					else
						{
						InputPixelValue = ( (unsigned short*)pInputReadPoint )[ nPixel ];
						AdjustedPixelLuminosity = (double)m_pWindowingTableScaled8Bit[ (unsigned int)( InputPixelValue / 256 ) ] * (double)MaxGrayscaleValue;
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


// The method used to display an image using the OpenGL interface is (1) to load
// the image into a texture buffer, (2) define the display region as a rectangle,
// and (3) use the texture (the image) to paint the rectangle.
// The input image data buffer is at m_pAssignedDiagnosticImage -> m_pImageData.
BOOL CImageView::LoadImageAsTexture()
{
	BOOL				bViewportIsValid;
	GLsizei				ImageWidth;
	GLsizei				ImageHeight;
	GLenum				PixelType;
	GLenum				ColorFormat;
	GLenum				OutputColorFormat;
	CGraphicsAdapter	*pGraphicsAdapter;
	GLhandleARB			hShaderProgram;
	GLint				TextureWidthTestValue;
	
	GetExclusiveRightToRender();
	// If the display method may have been overridden, reset it.
	if ( m_ImageDisplayMethod == IMAGE_DISPLAY_SLOW )
		{
		if ( m_PrevImageDisplayMethod != IMAGE_DISPLAY_UNSPECIFIED && m_PrevImageDisplayMethod != IMAGE_DISPLAY_SLOW )
			m_ImageDisplayMethod = m_PrevImageDisplayMethod;
		}
	bViewportIsValid = InitViewport();
	if ( bViewportIsValid )
		{
		glPushMatrix();
		glLoadIdentity();

		m_bImageHasBeenRendered = FALSE;
		if ( m_ImageDisplayMethod >= IMAGE_DISPLAY_USING_PACKED_8BIT &&
					( m_pAssignedDiagnosticImage != 0 && m_pAssignedDiagnosticImage -> m_ImageBitDepth > 8 ) )
			{
			pGraphicsAdapter = (CGraphicsAdapter*)m_pDisplayMonitor -> m_pGraphicsAdapter;
			if ( pGraphicsAdapter != 0 )
				{
				hShaderProgram  = pGraphicsAdapter -> m_gShaderProgram;

				LoadImageAs16BitGrayscaleTexture();
				//Initialize the variables in the shader program (in the GPU).
				glUseProgramObjectARB( hShaderProgram );
				CheckOpenGLResultAt( __FILE__, __LINE__	);
				// Attach texunit#0 to GrayImageTexture and texunit#1 to RGBLookupTable.
				glUniform1iARB( glGetUniformLocationARB( hShaderProgram, "GrayImageTexture" ), 0 );
				CheckOpenGLResultAt( __FILE__, __LINE__	);
				glUniform1iARB( glGetUniformLocationARB( hShaderProgram, "RGBLookupTable" ), 1 );
				CheckOpenGLResultAt( __FILE__, __LINE__	);
				// Set the image dimensions
				glUniform2fARB( glGetUniformLocationARB( hShaderProgram, "ImageSize" ),
										(float)m_pAssignedDiagnosticImage -> m_ImageWidthInPixels,
										(float)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels );
				CheckOpenGLResultAt( __FILE__, __LINE__	);
				glUseProgramObjectARB( 0 );
				}

			// Init the OpenGl state.
			glShadeModel( GL_SMOOTH );							// Enable Smooth Shading
			glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );				// Black Background
			}
		else
			{
			ColorFormat = (GLenum)m_pAssignedDiagnosticImage -> m_ImageColorFormat;
			OutputColorFormat = ColorFormat;
			ImageWidth = m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
			ImageHeight = m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
			if ( ColorFormat != GL_LUMINANCE && ColorFormat != GL_LUMINANCE_ALPHA )
				{
				// Reduce the image processing to a bare minimum to handle an image in a
				// non-grayscale format.
				if ( m_ImageDisplayMethod != IMAGE_DISPLAY_SLOW )
					m_PrevImageDisplayMethod = m_ImageDisplayMethod;
				m_ImageDisplayMethod = IMAGE_DISPLAY_SLOW;
				}
			// Load the raw image data into the texture buffer.
			if ( m_pAssignedDiagnosticImage -> m_ImageBitDepth == 8 )
				{
				glPixelStorei( GL_UNPACK_SWAP_BYTES, false );	// Swap the bytes in each word.
				glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );		// Set 1-byte pixel row alignment.
				PixelType = GL_UNSIGNED_BYTE;					// This is the size of each color component.

				if ( ColorFormat == GL_LUMINANCE_ALPHA )
					{
					if ( !m_pAssignedDiagnosticImage -> m_bImageHasBeenCompacted )
						{
						m_pAssignedDiagnosticImage -> ReducePixelsToEightBits();
						m_pAssignedDiagnosticImage -> m_ImageColorFormat = GL_LUMINANCE;
						ColorFormat = GL_LUMINANCE;
						OutputColorFormat = GL_LUMINANCE;
						}
					}
				}
			else if ( m_pAssignedDiagnosticImage -> m_ImageBitDepth == 12 )
				{
				glPixelStorei( GL_UNPACK_ALIGNMENT, 2 );		// Set 2-byte pixel row alignment.
				glPixelStorei( GL_UNPACK_SWAP_BYTES, true );	// Swap the bytes in each word.
				PixelType = GL_UNSIGNED_SHORT;
				}
			else if ( m_pAssignedDiagnosticImage -> m_ImageBitDepth == 16 )
				{
				glPixelStorei( GL_UNPACK_ALIGNMENT, 2 );		// Set 2-byte pixel row alignment.
				glPixelStorei( GL_UNPACK_SWAP_BYTES, true );	// Swap the bytes in each word.
				PixelType = GL_UNSIGNED_SHORT;
				}

			// Use texture operations only for grayscale images.
			if ( m_ImageDisplayMethod == IMAGE_DISPLAY_USING_8BIT_TEXTURE )
				{
				// Designate the texture unit to be affected by subsequent texture state operations.
				// (There must be at least 2 texture units available, but there could be more.)
				glActiveTexture( GL_TEXTURE0 );

				// Check if the image is too large to load as a texture.
				glTexImage2D( GL_PROXY_TEXTURE_2D, 0, OutputColorFormat, ImageWidth, ImageHeight, 0,
								ColorFormat, PixelType, m_pAssignedDiagnosticImage -> m_pImageData );
				glGetTexLevelParameteriv( GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &TextureWidthTestValue );
				if ( TextureWidthTestValue == 0 && !m_pAssignedDiagnosticImage -> m_bImageHasBeenDownSampled )
					{
					// If the input image is too large to fit in the available texture memory, downsample
					// it to a lower resolution.  Thus, 300dpi --> 150dpi.
					m_pAssignedDiagnosticImage -> DownSampleImageResolution();
					ImageWidth = m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
					ImageHeight = m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
					}

				glTexImage2D( GL_TEXTURE_2D, 0, OutputColorFormat, ImageWidth, ImageHeight, 0,
								ColorFormat, PixelType, m_pAssignedDiagnosticImage -> m_pImageData );
				CheckOpenGLResultAt( __FILE__, __LINE__	);

				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			
				glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
				glEnable( GL_TEXTURE_2D );
				}
			}
		glPopMatrix();
		}
	else
		AllowOthersToRender();
LogMessage( "LoadImageAsTexture() completed.", MESSAGE_TYPE_SUPPLEMENTARY );

	return bViewportIsValid;
}


// This function is called if enhanced grayscale pixel packing is supported by the
// OpenGL firmware in the video card.
// The input image data buffer is at m_pAssignedDiagnosticImage -> m_pImageData.
// This image is loaded into texture unit 0 as unsigned 8-bit or 16-bit integer pixels.
//
void CImageView::LoadImageAs16BitGrayscaleTexture()
{
	GLsizei			ImageWidth;
	GLsizei			ImageHeight;
	float			BorderColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GLenum			PixelDataType;
	GLenum			PixelFormat;
	
	glDeleteTextures( 1, &m_glImageTextureId );
	// Generate a texture "name" for the image and save it as m_glImageTextureId.
	glGenTextures( 1, &m_glImageTextureId );
	// Designate the texture unit to be affected by subsequent texture state operations.
	// (There must be at least 2 texture units available, but there could be more.)
	glActiveTexture( GL_TEXTURE0 );
	// Bind the image texture name to the 2-dimensional texture target.
	glBindTexture( GL_TEXTURE_2D, m_glImageTextureId );
	// Specify that the source image has 2-byte (16-bit) row alignment.
	glPixelStorei( GL_UNPACK_ALIGNMENT, 2 );
	// Set the texture environment mode for texture replacement.
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	// Set the texture wrapping for the S and T coordinates.
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	// Set the texture pixel scaling functions for when pixels are smaller or larger
	// than texture elements.
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	// Set the texture border color.
	glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BorderColor );
	// Load the image data into the texture.  Each pixel is loaded as an integer,
	// instead of as a set of float RGBA components.  The format of the pixel data
	// is GL_ALPHA_INTEGER_EXT.  The format of the texture data is GL_ALPHA16UI_EXT,
	// which packs 12-bit grayscale data into 24-bit rgb packaging, getting two
	// grayscale pixels for the price of one rgb pixel.
	ImageWidth = (GLsizei)m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
	ImageHeight = (GLsizei)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
	if ( m_pAssignedDiagnosticImage -> m_ImageBitDepth == 8 )
		{
		PixelFormat = (GLenum)m_pAssignedDiagnosticImage -> m_ImageColorFormat;
		PixelDataType = GL_UNSIGNED_BYTE;
		}
	else
		{
		PixelFormat = GL_ALPHA_INTEGER_EXT;
		PixelDataType = GL_UNSIGNED_SHORT;
		}
	// Pack the 12-bit grayscale pixels, two per 24-bit rgb pixel.
	CheckOpenGLResultAt( __FILE__, __LINE__	);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA16UI_EXT, ImageWidth, ImageHeight, 0,
					PixelFormat, PixelDataType, (GLvoid*)m_pAssignedDiagnosticImage -> m_pImageData );
	CheckOpenGLResultAt( __FILE__, __LINE__	);
}


// This function is called after the image has been loaded as a texture, if texturing is supported.
// Here a rectangle is created and scaled to the image.  Any rotation, flip or magnification is performed
// on the rectangle.  The image is applied as a texture to the transformed rectangle.  The result is in
// the frame buffer, from which it is read out into memory.  The transformed image is in the m_pSavedDisplay
// buffer.
//
// These processing steps are done here, so they won't have to be repeated every time the brightness and
// contrast are changed.  This makes the brightness and contrast changes much faster.
void CImageView::PrepareImage()
{
	GLsizei			ViewportRect[ 4 ];
	GLfloat			TranslationX;
	GLfloat			TranslationY;
	double			BaseScaleX;
	double			BaseScaleY;
	GLfloat			CenterOfRotation[ 3 ];
	GLsizei			ColorTableLength;
	double			ScaledWidthOffset;
	double			ScaledHeightOffset;
	unsigned short	nBytesPerPixel;

	glPushMatrix();
	glLoadIdentity();
	glPixelTransferi( GL_MAP_COLOR, GL_FALSE );

	// Set background clearing color to black.
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		
	// Clear the window with current clearing color
	glClear( GL_COLOR_BUFFER_BIT );

	// Only use texture operations for grayscale images.
	if ( m_ImageDisplayMethod == IMAGE_DISPLAY_USING_8BIT_TEXTURE &&
			( m_pAssignedDiagnosticImage != 0 && m_pAssignedDiagnosticImage -> m_ImageBitDepth == 8 ) )
		{
		glGetIntegerv( GL_VIEWPORT, ViewportRect );
		BaseScaleX = (GLfloat)m_pAssignedDiagnosticImage -> m_ScaleFactor;
		BaseScaleY = (GLfloat)m_pAssignedDiagnosticImage -> m_ScaleFactor;

		ScaledWidthOffset = (double)m_pAssignedDiagnosticImage -> m_ImageWidthInPixels * BaseScaleX / 2.0;
		ScaledHeightOffset = (double)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels * BaseScaleY / 2.0;

		TranslationX = (GLfloat)( ScaledWidthOffset - (double)m_pAssignedDiagnosticImage -> m_FocalPoint.x * BaseScaleX );
		TranslationY = (GLfloat)( ScaledHeightOffset - (double)m_pAssignedDiagnosticImage -> m_FocalPoint.y * BaseScaleY );

		// Create transformations for rotation and flipping.
		CenterOfRotation[ 0 ] = (GLfloat)( (double)ViewportRect[ 2 ] / 2.0 );
		CenterOfRotation[ 1 ] = (GLfloat)( (double)ViewportRect[ 3 ] / 2.0 );
		CenterOfRotation[ 2 ] = 0.0;
		glTranslatef( CenterOfRotation[ 0 ] + TranslationX, CenterOfRotation[ 1 ] - TranslationY, CenterOfRotation[ 2 ] );
		glRotatef( ( GLfloat)m_pAssignedDiagnosticImage -> m_RotationAngleInDegrees, 0.0, 0.0, 1.0 );

		// Apply the texture image to the appropriately transformed rectangle defined below.
		// This transformed image is now in the frame buffer.
		// Note:  This actually renders the image.
		InitSquareFrame();
		if ( m_pAssignedDiagnosticImage -> m_bFlipVertically )
			FlipFrameVertically();
		if ( m_pAssignedDiagnosticImage -> m_bFlipHorizontally )
			FlipFrameHorizontally();
		glBegin( GL_QUADS );
			// Create a rectangle to represent the displayed image.
			// (Use counterclockwise winding to view the front face.)
			// Bottom left corner.
			glTexCoord2f( m_SquareFrame.Xbl, m_SquareFrame.Ybl );
			glVertex2i( -(GLint)ScaledWidthOffset, -(GLint)ScaledHeightOffset );
			// Bottom right corner.
			glTexCoord2f( m_SquareFrame.Xbr, m_SquareFrame.Ybr );
			glVertex2i( (GLint)ScaledWidthOffset, -(GLint)ScaledHeightOffset );
			// Top right corner.
			glTexCoord2f( m_SquareFrame.Xtr, m_SquareFrame.Ytr );
			glVertex2i( (GLint)ScaledWidthOffset, (GLint)ScaledHeightOffset );
			// Top left corner.
			glTexCoord2f( m_SquareFrame.Xtl, m_SquareFrame.Ytl );
			glVertex2i( -(GLint)ScaledWidthOffset, (GLint)ScaledHeightOffset );
		glEnd();

		// Texture application is complete.  Turn off texture operations.
		glDisable( GL_TEXTURE_2D );
		// Read the screen image from the frame buffer and save it as the source of
		// the image for brightness and contrast enhancement.
		if ( m_pSavedDisplay != 0 )
			{
			free( m_pSavedDisplay );
			}
		nBytesPerPixel = m_pAssignedDiagnosticImage -> m_SamplesPerPixel * m_pAssignedDiagnosticImage -> m_nBitsAllocated / 8;
		m_pSavedDisplay = (char*)malloc( nBytesPerPixel * ViewportRect[ 2 ] * ViewportRect[ 3 ] );
		if ( m_pSavedDisplay != 0 )
			{
			glPixelStorei( GL_PACK_ALIGNMENT, 1 );		// Set 1-byte pixel alignment.
			// If pixel inversion is requested, perform it during the transfer to memory.
			if ( m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_bColorsInverted )
				{
				ColorTableLength = 256;
				glPixelTransferi( GL_MAP_COLOR, GL_TRUE );
				glPixelMapfv( GL_PIXEL_MAP_R_TO_R, ColorTableLength, m_pInversionTableScaled8Bit );
				glPixelMapfv( GL_PIXEL_MAP_G_TO_G, ColorTableLength, m_pInversionTableScaled8Bit );
				glPixelMapfv( GL_PIXEL_MAP_B_TO_B, ColorTableLength, m_pInversionTableScaled8Bit );
				}
			// Copy the image from the frame buffer into memory, applying the transformations specified above.
			if ( m_pAssignedDiagnosticImage -> m_ImageColorFormat == GL_LUMINANCE || m_pAssignedDiagnosticImage -> m_ImageColorFormat == GL_LUMINANCE_ALPHA )
				{
				glReadPixels( 0, 0, ViewportRect[ 2 ], ViewportRect[ 3 ], GL_RED, GL_UNSIGNED_BYTE, (GLubyte*)m_pSavedDisplay );
				}
			else
				glReadPixels( 0, 0, ViewportRect[ 2 ], ViewportRect[ 3 ], (GLenum)m_pAssignedDiagnosticImage -> m_ImageColorFormat, GL_UNSIGNED_BYTE, (GLubyte*)m_pSavedDisplay );
			CheckOpenGLResultAt( __FILE__, __LINE__	);
			m_SavedDisplayWidth = ViewportRect[ 2 ];
			m_SavedDisplayHeight = ViewportRect[ 3 ];
			glPixelTransferi( GL_MAP_COLOR, GL_FALSE );
			}
		}
	glPopMatrix();
	AllowOthersToRender();
}


void CImageView::RenderImage()
{
	BOOL				bViewportIsValid;
	GLint				ViewportRect[ 4 ];
	GLfloat				CenterOfRotation[ 3 ];
	GLint				RasterPosX;
	GLint				RasterPosY;
	GLfloat				VerticalOrientation;
	GLfloat				HorizontalOrientation;
	GLfloat				BaseScaleX;
	GLfloat				BaseScaleY;
	double				TranslationX;
	double				TranslationY;
	GLsizei				ColorTableLength;
	GLenum				ColorFormat;
	double				ScaledWidthOffset;
	double				ScaledHeightOffset;
	CGraphicsAdapter	*pGraphicsAdapter;
	GLhandleARB			hShaderProgram;
	GLfloat				MaxGrayIndex;
	double				MaxDisplayableGrayscaleValue;
	GLenum				PixelType;

	bViewportIsValid = InitViewport();
	if ( bViewportIsValid )
		{
		glPushMatrix();
		glLoadIdentity();
		glPixelTransferi( GL_MAP_COLOR, GL_FALSE );

		// Set background clearing color to black.
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		MaxDisplayableGrayscaleValue = (GLfloat)( pow( 2.0, m_pDisplayMonitor -> m_GrayScaleBitDepth ) - 1.0 );
		if ( m_pAssignedDiagnosticImage == 0 )
			glClear( GL_COLOR_BUFFER_BIT );
		if ( m_ImageDisplayMethod >= IMAGE_DISPLAY_USING_PACKED_8BIT &&
				( m_pAssignedDiagnosticImage != 0 && m_pAssignedDiagnosticImage -> m_ImageBitDepth > 8 ) )
			{
			pGraphicsAdapter = (CGraphicsAdapter*)m_pDisplayMonitor -> m_pGraphicsAdapter;
			if ( pGraphicsAdapter != 0 )
				{
				hShaderProgram  = pGraphicsAdapter -> m_gShaderProgram;
				CheckOpenGLResultAt( __FILE__, __LINE__	);

				glUseProgramObjectARB( pGraphicsAdapter -> m_gShaderProgram );
				CheckOpenGLResultAt( __FILE__, __LINE__	);
				glEnable( GL_FRAGMENT_PROGRAM_ARB );
				// Attach texunit#0 to GrayImageTexture and texunit#1 to RGBLookupTable.
				CheckOpenGLResultAt( __FILE__, __LINE__	);

				glUniform1iARB( glGetUniformLocationARB( hShaderProgram, "GrayImageTexture" ), 0 );
				CheckOpenGLResultAt( __FILE__, __LINE__	);
				glUniform1iARB( glGetUniformLocationARB( hShaderProgram, "RGBLookupTable" ), 1 );
				CheckOpenGLResultAt( __FILE__, __LINE__	);

				glUniform1fARB( glGetUniformLocationARB( hShaderProgram, "WindowMin" ), (GLfloat)m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowMinPixelAmplitude );
				glUniform1fARB( glGetUniformLocationARB( hShaderProgram, "WindowMax" ), (GLfloat)m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowMaxPixelAmplitude );

				if ( pBViewerCustomization -> m_WindowingAlgorithmSelection == SELECT_SIGMOID_WINDOWING )
					glUniform1iARB( glGetUniformLocationARB( hShaderProgram, "bWindowingIsSigmoidal" ), 1 );
				else
					glUniform1iARB( glGetUniformLocationARB( hShaderProgram, "bWindowingIsSigmoidal" ), 0 );

				if ( m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_bColorsInverted )
					glUniform1iARB( glGetUniformLocationARB( hShaderProgram, "bInvertGrayscale" ), 1 );
				else
					glUniform1iARB( glGetUniformLocationARB( hShaderProgram, "bInvertGrayscale" ), 0 );
				CheckOpenGLResultAt( __FILE__, __LINE__	);
				if ( m_pAssignedDiagnosticImage != 0 )
					MaxGrayIndex = (GLfloat)( 2 << ( m_pAssignedDiagnosticImage -> m_ImageBitDepth - 1 ) );
				else
					MaxGrayIndex = 4096.0f;
				glUniform1fARB( glGetUniformLocationARB( hShaderProgram, "MaxGrayIndex" ), MaxGrayIndex );

				glUniform1fARB( glGetUniformLocationARB( hShaderProgram, "GammaValue" ), (GLfloat)m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_Gamma );
				CheckOpenGLResultAt( __FILE__, __LINE__	);

				glActiveTexture( GL_TEXTURE0 );
				glBindTexture( GL_TEXTURE_2D, m_glImageTextureId );
				glActiveTexture( GL_TEXTURE1 );
				if ( m_ImageDisplayMethod >= IMAGE_DISPLAY_USING_PACKED_10BIT )
					glBindTexture( GL_TEXTURE_1D, pGraphicsAdapter -> m_glLUT12BitTextureId );
				else
					glBindTexture( GL_TEXTURE_1D, pGraphicsAdapter -> m_glLUT8BitTextureId );
				glEnable( GL_TEXTURE_2D );
				glEnable( GL_TEXTURE_1D );
				if ( m_pAssignedDiagnosticImage != 0 )
					{
					glGetIntegerv( GL_VIEWPORT, ViewportRect );
					BaseScaleX = (GLfloat)m_pAssignedDiagnosticImage -> m_ScaleFactor;
					BaseScaleY = (GLfloat)m_pAssignedDiagnosticImage -> m_ScaleFactor;

					ScaledWidthOffset = (double)m_pAssignedDiagnosticImage -> m_ImageWidthInPixels * BaseScaleX / 2.0;
					ScaledHeightOffset = (double)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels * BaseScaleY / 2.0;

					TranslationX = (GLfloat)( ScaledWidthOffset - (double)m_pAssignedDiagnosticImage -> m_FocalPoint.x * BaseScaleX );
					TranslationY = (GLfloat)( ScaledHeightOffset - (double)m_pAssignedDiagnosticImage -> m_FocalPoint.y * BaseScaleY );

					glClear( GL_COLOR_BUFFER_BIT );		// Clear out the currently rendered image from the frame buffer.
					glColor3f( 0.0f, 0.0f, 0.0f );

					// Create transformations for rotation and flipping.
					CenterOfRotation[ 0 ] = (GLfloat)( (double)ViewportRect[ 2 ] / 2.0 );
					CenterOfRotation[ 1 ] = (GLfloat)( (double)ViewportRect[ 3 ] / 2.0 );
					CenterOfRotation[ 2 ] = 0.0;
					glTranslatef( CenterOfRotation[ 0 ] + (GLfloat)TranslationX, CenterOfRotation[ 1 ] - (GLfloat)TranslationY, CenterOfRotation[ 2 ] );
					glRotatef( ( GLfloat)m_pAssignedDiagnosticImage -> m_RotationAngleInDegrees, 0.0, 0.0, 1.0 );

					// Apply the texture image to the appropriately transformed rectangle defined below.
					// This transformed image is now in the frame buffer.
					InitSquareFrame();
					if ( m_pAssignedDiagnosticImage -> m_bFlipVertically )
						FlipFrameVertically();
					if ( m_pAssignedDiagnosticImage -> m_bFlipHorizontally )
						FlipFrameHorizontally();
					glBegin( GL_QUADS );
						// Create a rectangle to represent the displayed image.
						// (Use counterclockwise winding to view the front face.)
						// Bottom left corner.
						glTexCoord2f( m_SquareFrame.Xbl, m_SquareFrame.Ybl );
						glVertex2i( -(GLint)ScaledWidthOffset, -(GLint)ScaledHeightOffset );
						// Bottom right corner.
						glTexCoord2f( m_SquareFrame.Xbr, m_SquareFrame.Ybr );
						glVertex2i( (GLint)ScaledWidthOffset, -(GLint)ScaledHeightOffset );
						// Top right corner.
						glTexCoord2f( m_SquareFrame.Xtr, m_SquareFrame.Ytr );
						glVertex2i( (GLint)ScaledWidthOffset, (GLint)ScaledHeightOffset );
						// Top left corner.
						glTexCoord2f( m_SquareFrame.Xtl, m_SquareFrame.Ytl );
						glVertex2i( -(GLint)ScaledWidthOffset, (GLint)ScaledHeightOffset );
					glEnd();
					}
				glDisable( GL_FRAGMENT_PROGRAM_ARB );
				glUseProgramObjectARB( 0 );
				}
			CheckOpenGLResultAt( __FILE__, __LINE__	);
			glDisable( GL_TEXTURE_2D );
			glDisable( GL_TEXTURE_1D );
			}
		else			// ... if 8-bit display method.  The image will have been converted to 8 bits
						//		of grayscale luminosity when it was loaded from the PNG file, by a call
						//		to ReduceTo8BitGrayscale().  m_ImageBitDepth will have been set to 8.
			{
			if ( m_pAssignedDiagnosticImage != 0 )
				{
				ColorFormat = (GLenum)m_pAssignedDiagnosticImage -> m_ImageColorFormat;
				// This method is about 40% faster than scaling the color matrix (on the development system).
				if ( m_ImageDisplayMethod == IMAGE_DISPLAY_USING_8BIT_TEXTURE )
					{
					glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );		// Set 1-byte pixel alignment.
					PixelType = GL_UNSIGNED_BYTE;
					}
				else if ( m_pAssignedDiagnosticImage -> m_ImageBitDepth == 8 )
					{
					glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );		// Set 1-byte pixel alignment.
					PixelType = GL_UNSIGNED_BYTE;
					}
				else if ( m_pAssignedDiagnosticImage -> m_ImageBitDepth == 12 )
					{
					glPixelStorei( GL_UNPACK_ALIGNMENT, 2 );		// Set 2-byte pixel alignment.
					glPixelStorei( GL_UNPACK_SWAP_BYTES, true );	// Swap the bytes in each word.
					PixelType = GL_UNSIGNED_SHORT;
					}
				else if ( m_pAssignedDiagnosticImage -> m_ImageBitDepth == 16 )
					{
					glPixelStorei( GL_UNPACK_ALIGNMENT, 2 );		// Set 2-byte pixel alignment.
					PixelType = GL_UNSIGNED_SHORT;
					}

				if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_PATIENT && ( m_ImageDisplayMethod == IMAGE_DISPLAY_USING_8BIT_TEXTURE ||
								m_pAssignedDiagnosticImage -> m_ImageBitDepth == 8 ) )
					{
					// Engage the windowing adjustment lookup table.
					ColorTableLength = 256;
					glPixelTransferi( GL_MAP_COLOR, GL_TRUE );
					glPixelMapfv( GL_PIXEL_MAP_R_TO_R, ColorTableLength, m_pWindowingTableScaled8Bit );
					glPixelMapfv( GL_PIXEL_MAP_G_TO_G, ColorTableLength, m_pWindowingTableScaled8Bit );
					glPixelMapfv( GL_PIXEL_MAP_B_TO_B, ColorTableLength, m_pWindowingTableScaled8Bit );
					}

				// Draw the image.
				if ( m_pAssignedDiagnosticImage -> m_pImageData != 0 )
					{
					glGetIntegerv( GL_VIEWPORT, ViewportRect );
					TranslationX = (double)ViewportRect[ 2 ] / 2.0 -
								m_pAssignedDiagnosticImage -> m_FocalPoint.x * m_pAssignedDiagnosticImage -> m_ScaleFactor;
					TranslationY = (double)ViewportRect[ 3 ] / 2.0 -
								m_pAssignedDiagnosticImage -> m_FocalPoint.y * m_pAssignedDiagnosticImage -> m_ScaleFactor;
					if ( m_pAssignedDiagnosticImage -> m_bFlipVertically )
						{
						RasterPosY = ViewportRect[ 3 ] - (GLint)TranslationY;
						VerticalOrientation = -1.0;
						}
					else
						{
						RasterPosY = ViewportRect[ 3 ] - (GLint)( 
									m_pAssignedDiagnosticImage -> m_ImageHeightInPixels *
									m_pAssignedDiagnosticImage -> m_ScaleFactor + TranslationY );
						VerticalOrientation = 1.0;
						}
					if ( m_pAssignedDiagnosticImage -> m_bFlipHorizontally )
						{
						RasterPosX = ViewportRect[ 2 ] + (GLint)TranslationX;
						HorizontalOrientation = -1.0;
						}
					else
						{
						RasterPosX = (GLint)TranslationX;
						HorizontalOrientation = 1.0;
						}

					BaseScaleX = (GLfloat)m_pAssignedDiagnosticImage -> m_ScaleFactor;
					BaseScaleY = (GLfloat)m_pAssignedDiagnosticImage -> m_ScaleFactor;

					glRasterPos2i( 0, 0 );
					if ( m_ImageDisplayMethod != IMAGE_DISPLAY_SLOW && m_pSavedDisplay != 0 && ( m_ImageDisplayMethod == IMAGE_DISPLAY_USING_8BIT_TEXTURE ||
								m_pAssignedDiagnosticImage -> m_ImageBitDepth == 8 ) )
						{
						CheckOpenGLResultAt( __FILE__, __LINE__	);
						glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );		// Set 1-byte pixel alignment.
						glDrawPixels( m_SavedDisplayWidth, m_SavedDisplayHeight, ColorFormat, GL_UNSIGNED_BYTE, (GLubyte*)m_pSavedDisplay );
						CheckOpenGLResultAt( __FILE__, __LINE__	);
						}
					else if ( m_pAssignedDiagnosticImage -> m_ImageWidthInPixels > 0 && m_pAssignedDiagnosticImage -> m_ImageHeightInPixels > 0 &&
								m_pAssignedDiagnosticImage -> m_ImageWidthInPixels < 100000 && m_pAssignedDiagnosticImage ->m_ImageHeightInPixels < 100000 )
						{
						// Use a fake bitmap rendering to relocate the raster position to a possible
						// position which could be invalid to specify with glRasterPos2i.
						glWindowPos2f( (GLfloat)RasterPosX, (GLfloat)RasterPosY );	// This seems to work as well as glBitmap().
						glPixelZoom( HorizontalOrientation * BaseScaleX, VerticalOrientation * BaseScaleY );
						// PixelType describes the pixel data format in the m_pImageData buffer from which the image is to be read.
						// ColorFormat describes the color format of each of these pixels.
						glDrawPixels( m_pAssignedDiagnosticImage -> m_ImageWidthInPixels, m_pAssignedDiagnosticImage -> m_ImageHeightInPixels,
																			ColorFormat, PixelType, m_pAssignedDiagnosticImage -> m_pImageData );
						CheckOpenGLResultAt( __FILE__, __LINE__	);
						glPixelZoom( 1.0, 1.0 );
						}
					}
				}
			glPixelTransferi( GL_MAP_COLOR, GL_FALSE );
			}			// ... end if 8-bit display method.
		glPopMatrix();
		}
	Invalidate( FALSE );
	m_bImageHasBeenRendered = TRUE;
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

		if ( m_pAssignedDiagnosticImage != 0 && m_pAssignedDiagnosticImage -> m_bEnableOverlays )
			RenderImageOverlay( m_hDC, IMAGE_DESTINATION_WINDOW );

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


void CImageView::RenderImageOverlay(  HDC hDC, unsigned long ImageDestination )
{
	GLfloat					ViewportRect[ 4 ];
	double					TranslationX;
	double					TranslationY;
	GLfloat					RasterPosX;
	GLfloat					RasterPosY;
	GLfloat					BaseScaleX;
	GLfloat					BaseScaleY;
	GLfloat					CheckX;
	GLfloat					CheckY;
	GLfloat					DeltaX;
	GLfloat					DeltaY;
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
	int						nFontList;
	CFont					TextFont;
	CFont					BoldFont;
	CFont					SmallFont;
	CFont					SmallItallicFont;
	HGDIOBJ					hSavedFontHandle;
	bool					bFontOk;
	int						FontHeight;
	MEASURED_INTERVAL		*pMeasuredInterval;
	GLfloat					x, y, z;
	double					ScaleFactor;
	BOOL					bConfigurablePart;
	SIGNATURE_BITMAP		*pSignatureBitmap;
	GLenum					BitmapPixelFormat;
	GLenum					BitmapPixelDataType;

	glPushMatrix();
	glLoadIdentity();
	if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_REPORT && m_pAssignedDiagnosticImage != 0 )
		{
		pCurrentStudy = ThisBViewerApp.m_pCurrentStudy;
		if ( pCurrentStudy != 0  )
			{
			if ( ImageDestination == IMAGE_DESTINATION_WINDOW )
				{
				// Set drawing color.
				glColor3f( 0.0f, 0.0f, 0.5f );
				// Set scaling.
				glGetFloatv( GL_VIEWPORT, ViewportRect );
				TranslationX = (double)ViewportRect[ 2 ] / 2.0 - (double)m_pAssignedDiagnosticImage -> m_FocalPoint.x * m_pAssignedDiagnosticImage -> m_ScaleFactor;
				TranslationY = (double)ViewportRect[ 3 ] / 2.0 - (double)m_pAssignedDiagnosticImage -> m_FocalPoint.y * m_pAssignedDiagnosticImage -> m_ScaleFactor;
				BaseScaleX = 2.78f * (GLfloat)m_pAssignedDiagnosticImage -> m_ScaleFactor;
				BaseScaleY = 2.78f * (GLfloat)m_pAssignedDiagnosticImage -> m_ScaleFactor;
				RasterPosX = (GLfloat)TranslationX;
				RasterPosY = ViewportRect[ 3 ] - (GLfloat)( 
							(double)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels * m_pAssignedDiagnosticImage -> m_ScaleFactor + TranslationY );
				FontHeight = (int)( 28.0f * (GLfloat)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels * m_pAssignedDiagnosticImage -> m_ScaleFactor / 1000.0f );
				}
			else if ( ImageDestination == IMAGE_DESTINATION_FILE )
				{
				// Set drawing color.  Since rendering to a DIB, it has to be backwards.
				glColor3f( 0.5f, 0.0f, 0.0f );
				BaseScaleX = 2.78f;
				BaseScaleY = 2.78f;
				RasterPosX = 0;
				RasterPosY = 0;
				FontHeight = (int)( 28.0f * (GLfloat)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels / 1000.0f );
				}
			else if ( ImageDestination == IMAGE_DESTINATION_PRINTER )
				{
				// Rendering to the same DIB here, but this one doesn't have to have
				// the colors reversed.  Go figure.
				glColor3f( 0.0f, 0.0f, 0.5f );
				BaseScaleX = 2.78f;
				BaseScaleY = 2.78f;
				RasterPosX = 0;
				RasterPosY = 0;
				FontHeight = (int)( 28.0f * (GLfloat)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels / 1000.0f );
				}
			glLineWidth( 4 * BaseScaleX );
			DeltaX = 10.0f * BaseScaleX;
			DeltaY = 10.0f * BaseScaleY;
			if ( m_PageNumber == 2 )
				{
				DeltaX = 7.0f * BaseScaleX;
				DeltaY = 7.0f * BaseScaleY;
				}

			glBegin(GL_LINES);
				nBox = 0;
				bConfigurablePart = TRUE;
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
						CheckX = pBoxLocationInfo -> BoxX * BaseScaleX
							+ RasterPosX;
						CheckY = pBoxLocationInfo -> BoxY * BaseScaleY + RasterPosY;
						glVertex2f( CheckX, CheckY + DeltaY );
						glVertex2f( CheckX + DeltaX, CheckY );
						glVertex2f( CheckX, CheckY );
						glVertex2f( CheckX + DeltaX, CheckY + DeltaY );
						}
						
					}
				while ( pBoxLocationInfo -> ResourceSymbol != 0 || nBox == 0 );
			glEnd();

			// Create display lists for font character glyphs 0 through 128.
			bFontOk = ( TextFont.CreateFont(
					-FontHeight,				// nHeight in device units.
					0,							// nWidth - use available aspect ratio
					0,							// nEscapement - make character lines horizontal
					0,							// nOrientation - individual chars are horizontal
					FW_SEMIBOLD,				// nWeight - character stroke thickness
					FALSE,						// bItalic - not italic
					FALSE,						// bUnderline - not underlined
					FALSE,						// cStrikeOut - not a strikeout font
					ANSI_CHARSET,				// nCharSet - normal ansi characters
					OUT_OUTLINE_PRECIS,			// nOutPrecision - choose font type using default search
					CLIP_DEFAULT_PRECIS,		// nClipPrecision - use default clipping
					PROOF_QUALITY,				// nQuality - best possible appearance
					FIXED_PITCH,				// nPitchAndFamily - fixed or variable pitch
					"Dontcare"					// lpszFacename
					) != 0 );

			if ( bFontOk && ::SelectObject( hDC, (HGDIOBJ)(HFONT)( TextFont.GetSafeHandle() ) ) != 0 )
				{
				nFontList = glGenLists( 128 );
				if ( wglUseFontBitmaps( hDC, 0, 128, nFontList ) == FALSE )
					SystemErrorCode = GetLastError();
				glListBase( nFontList );
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
					if ( strlen( TextField ) - pFieldLocationInfo -> nFirstCharacter + 1  < TextLength )
						TextLength = strlen( TextField ) - pFieldLocationInfo -> nFirstCharacter + 1;
					CheckY = ( pFieldLocationInfo -> Y ) * BaseScaleY + RasterPosY;
					for ( nChar = 0; nChar < (int)TextLength; nChar++ )
						{
						CheckX = ( pFieldLocationInfo -> X + ( nChar * pFieldLocationInfo -> CharWidth ) ) * BaseScaleX + RasterPosX;

						glRasterPos2f( CheckX, CheckY );
						glCallLists( 1, GL_UNSIGNED_BYTE, (const GLvoid*)&TextField[ nChar + pFieldLocationInfo -> nFirstCharacter ] );
						CheckOpenGLResultAt( __FILE__, __LINE__	);
						}
					}
				while ( pFieldLocationInfo -> ResourceSymbol != 0 );

				glDeleteLists( nFontList, 128 );
				TextFont.DeleteObject();
				}
			// Create display lists for comment font character glyphs 0 through 128.
			bFontOk = ( TextFont.CreateFont(
					-FontHeight / 2,			// nHeight in device units.
					0,							// nWidth - use available aspect ratio
					0,							// nEscapement - make character lines horizontal
					0,							// nOrientation - individual chars are horizontal
					FW_MEDIUM,					// nWeight - character stroke thickness
					FALSE,						// bItalic - not italic
					FALSE,						// bUnderline - not underlined
					FALSE,						// cStrikeOut - not a strikeout font
					ANSI_CHARSET,				// nCharSet - normal ansi characters
					OUT_OUTLINE_PRECIS,			// nOutPrecision - choose font type using default search
					CLIP_DEFAULT_PRECIS,		// nClipPrecision - use default clipping
					PROOF_QUALITY,				// nQuality - best possible appearance
					DEFAULT_PITCH,				// nPitchAndFamily - fixed or variable pitch
					"Roman"						// lpszFacename
					) != 0 );

			if ( bFontOk && ::SelectObject( hDC, (HGDIOBJ)(HFONT)( TextFont.GetSafeHandle() ) ) != 0 )
				{
				nFontList = glGenLists( 128 );
				wglUseFontBitmaps( hDC, 0, 128, nFontList );
				glListBase( nFontList );
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
							CheckX = ( pCommentFieldLocationInfo -> X ) * BaseScaleX + RasterPosX;
							CheckY = ( pCommentFieldLocationInfo -> Y - ( nLine * pCommentFieldLocationInfo -> LineSpacing ) ) * BaseScaleY + RasterPosY;
							glRasterPos2f( CheckX, CheckY );
							glCallLists( (GLsizei)nCharactersToDisplay, GL_UNSIGNED_BYTE,
											(const GLvoid*)&pTextField[ TextLength - nCharactersRemaining ] );
							CheckOpenGLResultAt( __FILE__, __LINE__	);
							nCharactersRemaining -= nCharactersToDisplay + nSkippedChars;
							}
						}
					}
				while ( pCommentFieldLocationInfo -> ResourceSymbol != 0 );
				}

			if ( bFontOk && ::SelectObject( hDC, (HGDIOBJ)(HFONT)( TextFont.GetSafeHandle() ) ) != 0 )
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
						CheckX = ( pTextFieldLocationInfo -> X ) * BaseScaleX + RasterPosX;
						CheckY = ( pTextFieldLocationInfo -> Y ) * BaseScaleY + RasterPosY;
						glRasterPos2f( CheckX, CheckY );
						glCallLists( (GLsizei)TextLength, GL_UNSIGNED_BYTE, TextField );
						CheckOpenGLResultAt( __FILE__, __LINE__	);
						}
					}
				while ( pTextFieldLocationInfo -> ResourceSymbol != 0 );

				if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
					{
					// Render the client heading information.
					if ( m_PageNumber == 1 && p_CurrentClientInfo != 0 && _stricmp( p_CurrentClientInfo -> Name, "None" ) != 0 )
						{
						bFontOk = ( BoldFont.CreateFont(
								-2 * FontHeight / 3,		// nHeight in device units.
								0,							// nWidth - use available aspect ratio
								0,							// nEscapement - make character lines horizontal
								0,							// nOrientation - individual chars are horizontal
								FW_BOLD,					// nWeight - character stroke thickness
								FALSE,						// bItalic - not italic
								FALSE,						// bUnderline - not underlined
								FALSE,						// cStrikeOut - not a strikeout font
								ANSI_CHARSET,				// nCharSet - normal ansi characters
								OUT_OUTLINE_PRECIS,			// nOutPrecision - choose font type using default search
								CLIP_DEFAULT_PRECIS,		// nClipPrecision - use default clipping
								PROOF_QUALITY,				// nQuality - best possible appearance
								DEFAULT_PITCH,				// nPitchAndFamily - fixed or variable pitch
								"Roman"						// lpszFacename
								) != 0 );
						if ( bFontOk )
							bFontOk = ( SmallFont.CreateFont(
								FontHeight / 2,				// nHeight in device units.
								2 * FontHeight / 10,		// nWidth - use available aspect ratio
								0,							// nEscapement - make character lines horizontal
								0,							// nOrientation - individual chars are horizontal
								FW_MEDIUM,					// nWeight - character stroke thickness
								FALSE,						// bItalic - not italic
								FALSE,						// bUnderline - not underlined
								FALSE,						// cStrikeOut - not a strikeout font
								ANSI_CHARSET,				// nCharSet - normal ansi characters
								OUT_OUTLINE_PRECIS,			// nOutPrecision - choose font type using default search
								CLIP_DEFAULT_PRECIS,		// nClipPrecision - use default clipping
								PROOF_QUALITY,				// nQuality - best possible appearance
								DEFAULT_PITCH,				// nPitchAndFamily - fixed or variable pitch
								"Roman"						// lpszFacename
								) != 0 );
						if ( bFontOk )
							bFontOk = ( SmallItallicFont.CreateFont(
								FontHeight / 2,				// nHeight in device units.
								15 * FontHeight / 80,		// nWidth - use available aspect ratio
								0,							// nEscapement - make character lines horizontal
								0,							// nOrientation - individual chars are horizontal
								FW_MEDIUM,					// nWeight - character stroke thickness
								TRUE,						// bItalic - not italic
								FALSE,						// bUnderline - not underlined
								FALSE,						// cStrikeOut - not a strikeout font
								ANSI_CHARSET,				// nCharSet - normal ansi characters
								OUT_OUTLINE_PRECIS,			// nOutPrecision - choose font type using default search
								CLIP_DEFAULT_PRECIS,		// nClipPrecision - use default clipping
								PROOF_QUALITY,				// nQuality - best possible appearance
								DEFAULT_PITCH,				// nPitchAndFamily - fixed or variable pitch
								"Roman"						// lpszFacename
								) != 0 );

						pDataStructure = (char*)p_CurrentClientInfo;
						nField = -1;
						// Set drawing color.
						glColor3f( 0.0f, 0.0f, 0.0f );
						if ( bFontOk )		// Preselect the enhanced font for the client name field.
							{
							hSavedFontHandle = ::SelectObject( hDC, (HGDIOBJ)(HFONT)( BoldFont.GetSafeHandle() ) );
							nFontList = glGenLists( 128 );
							wglUseFontBitmaps( hDC, 0, 128, nFontList );
							glListBase( nFontList );
							}
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
										break;
									case 1:					// Street address field.
										strcpy( TextField, "" );
										strncat( TextField, pDataFieldValue, 256 - 1 );
										if ( bFontOk )		// Switch to the small font for the client address fields.
											{
											::SelectObject( hDC, (HGDIOBJ)(HFONT)( SmallFont.GetSafeHandle() ) );
											glDeleteLists( nFontList, 128 );
											BoldFont.DeleteObject();
											nFontList = glGenLists( 128 );
											wglUseFontBitmaps( hDC, 0, 128, nFontList );
											glListBase( nFontList );
											}
										break;
									case 3:					// State field.
										strncat( TextField, ", ", 256 - strlen( TextField ) - 1 );
										strncat( TextField, pDataFieldValue, 256 - strlen( TextField ) - 1 );
										break;
									case 4:					// Zip code field.
										strncat( TextField, "     ", 256 - strlen( TextField ) - 1 );
										strncat( TextField, pDataFieldValue, 256 - strlen( TextField ) - 1 );
										break;
									case 6:					// Other contact info field.
										strcpy( TextField, "" );
										strncat( TextField, pDataFieldValue, 256 - 1 );
										if ( bFontOk )		// Switch to the small font for the client address fields.
											{
											::SelectObject( hDC, (HGDIOBJ)(HFONT)( SmallItallicFont.GetSafeHandle() ) );
											glDeleteLists( nFontList, 128 );
											SmallFont.DeleteObject();
											nFontList = glGenLists( 128 );
											wglUseFontBitmaps( hDC, 0, 128, nFontList );
											glListBase( nFontList );
											}
										break;
									default:
										strcpy( TextField, "" );
										strncat( TextField, pDataFieldValue, 256 - 1 );
										break;
									}
								TextLength = strlen( TextField );
								CheckX = ( pClientFieldLocationInfo -> X ) * BaseScaleX + RasterPosX;
								CheckY = ( pClientFieldLocationInfo -> Y ) * BaseScaleY + RasterPosY;
								glRasterPos2f( CheckX, CheckY );
								glCallLists( (GLsizei)TextLength, GL_UNSIGNED_BYTE, TextField );
								CheckOpenGLResultAt( __FILE__, __LINE__	);
								}
							}
						while ( pClientFieldLocationInfo -> X != 0.0 );
						if ( bFontOk )		// If the font was successfully changed, restore it.
							{
							::SelectObject( hDC, hSavedFontHandle );
							glDeleteLists( nFontList, 128 );
							SmallItallicFont.DeleteObject();
							nFontList = glGenLists( 128 );
							wglUseFontBitmaps( hDC, 0, 128, nFontList );
							glListBase( nFontList );
							}
						// Restore drawing color.
						glColor3f( 0.0f, 0.0f, 0.5f );
						}
					}

				// Render the reader's digital signature.
				pSignatureBitmap = pBViewerCustomization -> m_ReaderInfo.pSignatureBitmap;
				if ( pSignatureBitmap != 0 && ( 
							( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL && m_PageNumber == 2 ) ||
							  BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_GENERAL && m_PageNumber == 1 ) )
					{
					glPushMatrix();
					glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );		// Set 1-byte pixel alignment.
					if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
						{
						CheckX = (GLfloat)330.0 * BaseScaleX + RasterPosX;
						CheckY = (GLfloat)42.0 * BaseScaleY + RasterPosY;
						}
					else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
						{
						CheckX = (GLfloat)53.0 * BaseScaleX + RasterPosX;
						CheckY = (GLfloat)103.0 * BaseScaleY + RasterPosY;
						}
					glPixelZoom( BaseScaleX * (GLfloat)250.0 / (GLfloat)pSignatureBitmap -> WidthInPixels,
											BaseScaleY * (GLfloat)32.0 / (GLfloat)pSignatureBitmap -> HeightInPixels );
					if ( pSignatureBitmap -> pImageData != 0 )
						{
						if ( pSignatureBitmap -> BitsPerPixel == 1 )
							{
							glColor3f( 0.0, 0.0, 0.0 );
							glRasterPos2f( CheckX, CheckY );
							glBitmap( pSignatureBitmap -> WidthInPixels, pSignatureBitmap -> HeightInPixels,
										0.0, 0.0, 0.0, 0.0, pSignatureBitmap -> pImageData );
							}
						else if ( pSignatureBitmap -> BitsPerPixel == 8 )
							{
							glRasterPos2f( CheckX, CheckY );
							BitmapPixelFormat = GL_LUMINANCE;
							BitmapPixelDataType = GL_UNSIGNED_BYTE;
							glDrawPixels( pSignatureBitmap -> WidthInPixels, pSignatureBitmap -> HeightInPixels,
										BitmapPixelFormat, BitmapPixelDataType, pSignatureBitmap -> pImageData );
							}
						else if ( pSignatureBitmap -> BitsPerPixel == 24 )
							{
							glRasterPos2f( CheckX, CheckY );
							if ( ImageDestination == IMAGE_DESTINATION_FILE )
								BitmapPixelFormat = GL_RGB;
							else
								BitmapPixelFormat = GL_BGR;
							BitmapPixelDataType = GL_UNSIGNED_BYTE;
							glDrawPixels( pSignatureBitmap -> WidthInPixels, pSignatureBitmap -> HeightInPixels,
										BitmapPixelFormat, BitmapPixelDataType, pSignatureBitmap -> pImageData );
							}
						CheckOpenGLResultAt( __FILE__, __LINE__	);
						}
					glPixelZoom( 1.0, 1.0 );
					glPopMatrix();
					}
				glDeleteLists( nFontList, 128 );
				TextFont.DeleteObject();
				}
			}
		}
	else if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_PATIENT && m_pAssignedDiagnosticImage != 0 )
		{
		GLfloat				CharWidth;
		double				MeasuredLength;
		IMAGE_ANNOTATION	*pImageAnnotationInfo;

		glGetFloatv( GL_VIEWPORT, ViewportRect );


		if ( m_pMeasuredIntervalList != 0 && m_PixelsPerMillimeter > 0.0 )
			{
			CharWidth = 28.0f;
			// Create display lists for font character glyphs 0 through 128.
			bFontOk = ( TextFont.CreateFont(
					-48,						// nHeight in device units.
					0,							// nWidth - use available aspect ratio
					0,							// nEscapement - make character lines horizontal
					0,							// nOrientation - individual chars are horizontal
					FW_BOLD,					// nWeight - character stroke thickness
					FALSE,						// bItalic - not italic
					FALSE,						// bUnderline - not underlined
					FALSE,						// cStrikeOut - not a strikeout font
					ANSI_CHARSET,				// nCharSet - normal ansi characters
					OUT_OUTLINE_PRECIS,			// nOutPrecision - choose font type using default search
					CLIP_DEFAULT_PRECIS,		// nClipPrecision - use default clipping
					PROOF_QUALITY,				// nQuality - best possible appearance
					FIXED_PITCH,				// nPitchAndFamily - fixed or variable pitch
					"Dontcare"					// lpszFacename
					) != 0 );
			if ( bFontOk && ::SelectObject( m_hDC, (HGDIOBJ)(HFONT)( TextFont.GetSafeHandle() ) ) != 0 )
				{
				nFontList = glGenLists( 128 );
				if ( wglUseFontBitmaps( m_hDC, 0, 128, nFontList ) == FALSE )
					SystemErrorCode = GetLastError();
				glListBase( nFontList );
				}
			else
				bFontOk = FALSE;

			z = 0.0f;
			glColor3f( 0.5f, 0.0f, 0.0f );	// Set the line color to dark red.
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
					
				glBegin( GL_LINES );
					// Make a "+" at the starting point.
					glVertex3f( x, ViewportRect[ 3 ] - y, z );
					glVertex3f( x + 20, ViewportRect[ 3 ] - y, z );

					glVertex3f( x, ViewportRect[ 3 ] - y, z );
					glVertex3f( x - 20, ViewportRect[ 3 ] - y, z );

					glVertex3f( x, ViewportRect[ 3 ] - y, z );
					glVertex3f( x, ViewportRect[ 3 ] - y - 20, z );

					glVertex3f( x, ViewportRect[ 3 ] - y, z );
					glVertex3f( x, ViewportRect[ 3 ] - y + 20, z );

					glVertex3f( x, ViewportRect[ 3 ] - y, z );		// Beginning of line segment.
					x = (GLfloat)( (double)pMeasuredInterval -> ScaledEndingPointX * ScaleFactor + TranslationX );
					y = (GLfloat)( (double)pMeasuredInterval -> ScaledEndingPointY * ScaleFactor + TranslationY );
					glVertex3f( x, ViewportRect[ 3 ] - y, z );		// End of line segment.

					// Make a "+" at the ending point.
					glVertex3f( x, ViewportRect[ 3 ] - y, z );
					glVertex3f( x + 20, ViewportRect[ 3 ] - y, z );

					glVertex3f( x, ViewportRect[ 3 ] - y, z );
					glVertex3f( x - 20, ViewportRect[ 3 ] - y, z );

					glVertex3f( x, ViewportRect[ 3 ] - y, z );
					glVertex3f( x, ViewportRect[ 3 ] - y - 20, z );

					glVertex3f( x, ViewportRect[ 3 ] - y, z );
					glVertex3f( x, ViewportRect[ 3 ] - y + 20, z );
				glEnd();

				MeasuredLength = pMeasuredInterval -> Distance / m_PixelsPerMillimeter;
				sprintf( TextField, "%d mm", (int)MeasuredLength );
				TextLength = strlen( TextField );
				y += 30;
				for ( nChar = 0; nChar < (int)TextLength; nChar++ )
					{
					x += CharWidth;

					glRasterPos2f( x, ViewportRect[ 3 ] - y );
					glCallLists( 1, GL_UNSIGNED_BYTE, (const GLvoid*)&TextField[ nChar ] );
					CheckOpenGLResultAt( __FILE__, __LINE__	);
					}
				pMeasuredInterval = pMeasuredInterval -> pNextInterval;
				}
			if ( bFontOk )
				{
				glDeleteLists( nFontList, 128 );
				TextFont.DeleteObject();
				}
			}
			
		// Display annotations if enabled.
		if ( m_bEnableAnnotations && m_pImageAnnotationList != 0 )
			{
			CharWidth = 14.0f;
			// Create display lists for font character glyphs 0 through 128.
			bFontOk = ( TextFont.CreateFont(
					-14,						// nHeight in device units.
					0,							// nWidth - use available aspect ratio
					0,							// nEscapement - make character lines horizontal
					0,							// nOrientation - individual chars are horizontal
					FW_SEMIBOLD,					// nWeight - character stroke thickness
					FALSE,						// bItalic - not italic
					FALSE,						// bUnderline - not underlined
					FALSE,						// cStrikeOut - not a strikeout font
					ANSI_CHARSET,				// nCharSet - normal ansi characters
					OUT_OUTLINE_PRECIS,			// nOutPrecision - choose font type using default search
					CLIP_DEFAULT_PRECIS,		// nClipPrecision - use default clipping
					PROOF_QUALITY,				// nQuality - best possible appearance
					FIXED_PITCH,				// nPitchAndFamily - fixed or variable pitch
					"Dontcare"					// lpszFacename
					) != 0 );
			if ( bFontOk && ::SelectObject( m_hDC, (HGDIOBJ)(HFONT)( TextFont.GetSafeHandle() ) ) != 0 )
				{
				nFontList = glGenLists( 128 );
				if ( wglUseFontBitmaps( m_hDC, 0, 128, nFontList ) == FALSE )
					SystemErrorCode = GetLastError();
				glListBase( nFontList );
				}
			else
				bFontOk = FALSE;

			glColor3f( 0.0f, 1.0f, 0.0f );	// Set the line color to green.
			y = 5.0f;
			pImageAnnotationInfo = m_pImageAnnotationList;
			while ( pImageAnnotationInfo != 0 )
				{
				x = 5.0f;
				TextLength = strlen( pImageAnnotationInfo -> TextField );
				y += 20;
				for ( nChar = 0; nChar < (int)TextLength; nChar++ )
					{
					x += CharWidth;
					glRasterPos2f( x, ViewportRect[ 3 ] - y );
					glCallLists( 1, GL_UNSIGNED_BYTE, (const GLvoid*)&pImageAnnotationInfo -> TextField[ nChar ] );
					CheckOpenGLResultAt( __FILE__, __LINE__	);
					}
				pImageAnnotationInfo = pImageAnnotationInfo -> pNextAnnotation;
				}
			if ( bFontOk )
				{
				glDeleteLists( nFontList, 128 );
				TextFont.DeleteObject();
				}
			}
		}
	glPopMatrix();
}


void CImageView::SaveReport()
{
	BOOL				bNoError = TRUE;
	BITMAPINFO			BitmapInfo;
	HDC					hCompatibleDC;
	HDC					hSavedDC;
	HBITMAP				hBitmap;
	HGLRC				hGLRC;
	HGLRC				hSavedGLRC;
	GLenum				ColorFormat;
	GLenum				OutputColorFormat;
	GLenum				PixelType;
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
	unsigned char		*pDIBImageData;		// Pointer to the pixel data in the DIB.
	CStudy				*pCurrentStudy;
	char				DateTimeString[ 32 ];
	char				DateOfRadiographString[ 32 ];
	char				*pChar;

	if ( m_pAssignedDiagnosticImage != 0 )
		{
		memset( &BitmapInfo, 0, sizeof( BITMAPINFO ) );
		BitmapInfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
		BitmapInfo.bmiHeader.biWidth = m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
		BitmapInfo.bmiHeader.biHeight = m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
		BitmapInfo.bmiHeader.biPlanes = 1;
		BitmapInfo.bmiHeader.biBitCount = 24;
		BitmapInfo.bmiHeader.biCompression = BI_RGB;
		OutputColorFormat = GL_BGR;
		
		// Create a memory device context compatible with the system display.
		hCompatibleDC = CreateCompatibleDC( NULL );

		hBitmap = ::CreateDIBSection( hCompatibleDC, &BitmapInfo, DIB_RGB_COLORS, (void**)&pDIBImageData, NULL, 0 );
		if ( hBitmap != 0 )
			{
			::SelectObject( hCompatibleDC, hBitmap );
			// At this point, the bitmap has been created on the device context and provides
			// the memory for rendering the image.
			SetExportDCPixelFormat( hCompatibleDC );
			hGLRC = wglCreateContext( hCompatibleDC );
			if ( hGLRC == 0 )
				SystemErrorCode = GetLastError();
			hSavedDC = wglGetCurrentDC();
			hSavedGLRC = wglGetCurrentContext();
			if ( wglMakeCurrent( hCompatibleDC, hGLRC ) == FALSE )
				SystemErrorCode = GetLastError();
			else
				{
				glViewport( 0, 0, m_pAssignedDiagnosticImage -> m_ImageWidthInPixels,
									m_pAssignedDiagnosticImage -> m_ImageHeightInPixels );
				glMatrixMode( GL_PROJECTION );
				glLoadIdentity();
				gluOrtho2D( 0.0f, (GLfloat)m_pAssignedDiagnosticImage -> m_ImageWidthInPixels,
							0.0f, (GLfloat)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels );
				glMatrixMode( GL_MODELVIEW );
				glLoadIdentity();
				ColorFormat = (GLenum)m_pAssignedDiagnosticImage -> m_ImageColorFormat;
				// Load the raw image data into the texture buffer.
				if ( m_pAssignedDiagnosticImage -> m_ImageBitDepth == 8 )
					{
					glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );		// Set 1-byte pixel alignment.
					PixelType = GL_UNSIGNED_BYTE;
					}
				else if ( m_pAssignedDiagnosticImage -> m_ImageBitDepth == 12 )
					{
					glPixelStorei( GL_UNPACK_ALIGNMENT, 2 );		// Set 2-byte pixel alignment.
					glPixelStorei( GL_UNPACK_SWAP_BYTES, true );	// Swap the bytes in each word.
					PixelType = GL_UNSIGNED_SHORT;
					}
				else if ( m_pAssignedDiagnosticImage -> m_ImageBitDepth == 16 )
					{
					glPixelStorei( GL_UNPACK_ALIGNMENT, 2 );		// Set 2-byte pixel alignment.
					glPixelStorei( GL_UNPACK_SWAP_BYTES, true );	// Swap the bytes in each word.
					PixelType = GL_UNSIGNED_SHORT;
					}
				CheckOpenGLResultAt( __FILE__, __LINE__	);
				// Transfer the image into the DIB.  Assume the format is GL_RGB and use
				// the format GL_BGR to reverse the colors for the benefit of the DIB.
				glDrawPixels( m_pAssignedDiagnosticImage -> m_ImageWidthInPixels,
								m_pAssignedDiagnosticImage -> m_ImageHeightInPixels,
								OutputColorFormat, PixelType, m_pAssignedDiagnosticImage -> m_pImageData );
				RenderImageOverlay( hCompatibleDC, IMAGE_DESTINATION_FILE );
				glFlush();
				}
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
				
			m_pAssignedDiagnosticImage -> m_pOutputImageData = (unsigned char*)pDIBImageData;
			m_pAssignedDiagnosticImage -> m_OutputImageHeightInPixels = m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
			m_pAssignedDiagnosticImage -> m_OutputImageWidthInPixels = m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
			bNoError = m_pAssignedDiagnosticImage -> WritePNGImageFile( FileSpecForWriting );
			wglMakeCurrent( NULL, NULL );
			wglDeleteContext( hGLRC );
			wglMakeCurrent( hSavedDC, hSavedGLRC );
			::DeleteObject( hBitmap );
			}
		::DeleteDC( hCompatibleDC );
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
}


BOOL CImageView::OpenReportForPrinting( BOOL bShowPrintDialog )
{
	BOOL				bNoError = TRUE;
	DOCINFO				PrinterDocInfo;
	int					nResponseCode;
	unsigned long		nImageWidth;
	unsigned long		nImageHeight;

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
			{
			m_hCompatibleDC = CreateCompatibleDC( NULL );
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
			}
		}
	else
		bNoError = FALSE;

	return bNoError;
}


void CImageView::PrintReportPage( BOOL bUseCurrentStudy )
{
	CWaitCursor			DisplaysHourglass;
	BOOL				bNoError = TRUE;
	GLenum				ColorFormat;
	GLenum				OutputColorFormat;
	GLenum				PixelType;
	int					nResponseCode;
	unsigned long		nImageWidth;
	unsigned long		nImageHeight;
	int					nRastersCopiedToPrinter;

	if ( m_pAssignedDiagnosticImage != 0 )
		{
		nImageWidth = m_pAssignedDiagnosticImage -> m_ImageWidthInPixels;
		nImageHeight = m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
		
		if ( m_hPrintableBitmap != 0 )
			{
			OutputColorFormat = GL_RGB;
			::SelectObject( m_hCompatibleDC, m_hPrintableBitmap );
			if ( wglMakeCurrent( m_hCompatibleDC, m_hGLPrintingRC ) == FALSE )
				{
				SystemErrorCode = GetLastError();
				bNoError = FALSE;
				}
			// At this point, the bitmap has been created on the device context and provides
			// the memory for rendering the image.
			glViewport( 0, 0, nImageWidth, nImageHeight );
			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			gluOrtho2D( 0.0f, (GLfloat)nImageWidth, 0.0f, (GLfloat)nImageHeight );
			glMatrixMode( GL_MODELVIEW );
			glLoadIdentity();

			// Set background clearing color to black.
			glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
			// Clear the window with current clearing color
			glClear( GL_COLOR_BUFFER_BIT );

			ColorFormat = (GLenum)m_pAssignedDiagnosticImage -> m_ImageColorFormat;
			// Load the raw image data into the texture buffer.
			if ( m_pAssignedDiagnosticImage -> m_ImageBitDepth == 8 )
				{
				glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );		// Set 1-byte pixel alignment.
				PixelType = GL_UNSIGNED_BYTE;
				}
			else if ( m_pAssignedDiagnosticImage -> m_ImageBitDepth == 12 )
				{
				glPixelStorei( GL_UNPACK_ALIGNMENT, 2 );		// Set 2-byte pixel alignment.
				glPixelStorei( GL_UNPACK_SWAP_BYTES, true );	// Swap the bytes in each word.
				PixelType = GL_UNSIGNED_SHORT;
				}
			else if ( m_pAssignedDiagnosticImage -> m_ImageBitDepth == 16 )
				{
				glPixelStorei( GL_UNPACK_ALIGNMENT, 2 );		// Set 2-byte pixel alignment.
				glPixelStorei( GL_UNPACK_SWAP_BYTES, true );	// Swap the bytes in each word.
				PixelType = GL_UNSIGNED_SHORT;
				}
			// Set background clearing color to white.
			glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
			
			// Clear the window with current clearing color
			glClear( GL_COLOR_BUFFER_BIT );

			glRasterPos2i( 0, 0 );

			// Transfer the image into the DIB.  Assume the format is GL_RGB and use
			// the format GL_BGR to reverse the colors for the benefit of the DIB.
			glDrawPixels( nImageWidth, nImageHeight, OutputColorFormat,
							PixelType, m_pAssignedDiagnosticImage -> m_pImageData );
			if ( bUseCurrentStudy )
				RenderImageOverlay( m_hCompatibleDC, IMAGE_DESTINATION_PRINTER );
			glFlush();

			m_pAssignedDiagnosticImage -> m_pOutputImageData = (unsigned char*)m_pDIBImageData;
			m_pAssignedDiagnosticImage -> m_OutputImageWidthInPixels = nImageWidth;
			m_pAssignedDiagnosticImage -> m_OutputImageHeightInPixels = nImageHeight;
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


void CImageView::SetDiagnosticImage( CDiagnosticImage *pDiagnosticImage, CStudy *pStudy )
{
	EraseImageAnnotationInfo();
	if ( pDiagnosticImage != 0 )
		{
		m_pAssignedDiagnosticImage = pDiagnosticImage;
		m_Mouse.m_pTargetImage = pDiagnosticImage;
		if ( m_ViewFunction == IMAGE_VIEW_FUNCTION_PATIENT && pStudy != 0 )
			LoadImageAnnotationInfo();
		LogMessage( "Loading image into view.", MESSAGE_TYPE_SUPPLEMENTARY );
		ResetDiagnosticImage( FALSE );
		}
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
			if ( !bRescaleOnly )
				{
				LoadWindowingConversionTable( m_pAssignedDiagnosticImage -> m_OriginalGrayscaleSetting.m_WindowWidth,
																m_pAssignedDiagnosticImage -> m_OriginalGrayscaleSetting.m_WindowCenter,
																m_pAssignedDiagnosticImage -> m_OriginalGrayscaleSetting.m_Gamma );
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
	if ( m_pAssignedDiagnosticImage != 0 )
		{
		memcpy( (char*)&m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting, (char*)pNewGrayscaleSetting, sizeof(IMAGE_GRAYSCALE_SETTING) );
		LoadWindowingConversionTable( m_pAssignedDiagnosticImage ->m_CurrentGrayscaleSetting.m_WindowWidth,
												m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowCenter,
												m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_Gamma );
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
	double					ScaledWidthOffset;
	double					ScaledHeightOffset;
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
				ScaledWidthOffset = (double)m_pAssignedDiagnosticImage -> m_ImageWidthInPixels * ScaleFactor / 2.0;
				ScaledHeightOffset = (double)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels * ScaleFactor / 2.0;
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
	double					ScaledWidthOffset;
	double					ScaledHeightOffset;
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
			ScaledWidthOffset = (double)m_pAssignedDiagnosticImage -> m_ImageWidthInPixels * ScaleFactor / 2.0;
			ScaledHeightOffset = (double)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels * ScaleFactor / 2.0;
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
		{
		if ( LoadImageAsTexture() )
			PrepareImage();
		}
	RepaintFast();

	return TRUE;
}


void CImageView::OnMouseMove( UINT nFlags, CPoint point )
{
	CRect					ClientRect;
	double					ScaleFactor;
	double					ScaledWidthOffset;
	double					ScaledHeightOffset;
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
					ScaledWidthOffset = (double)m_pAssignedDiagnosticImage -> m_ImageWidthInPixels * ScaleFactor / 2.0;
					ScaledHeightOffset = (double)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels * ScaleFactor / 2.0;
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

					m_pActiveMeasurementInterval -> ScaledEndingPointX = (GLfloat)( ( (double)x - HorizontalOrientation * TranslationX ) / ScaleFactor );
					m_pActiveMeasurementInterval -> ScaledEndingPointY = (GLfloat)( ( (double)y - VerticalOrientation * TranslationY ) / ScaleFactor );
					m_pActiveMeasurementInterval -> Distance =
								sqrt( pow( (double)m_pActiveMeasurementInterval -> ScaledEndingPointX -
											(double)m_pActiveMeasurementInterval -> ScaledStartingPointX, 2.0 ) +
										pow( (double)m_pActiveMeasurementInterval -> ScaledEndingPointY -
											(double)m_pActiveMeasurementInterval -> ScaledStartingPointY, 2.0 )	);
					}
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
	LoadWindowingConversionTable( m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowWidth,
				m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowCenter, m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_Gamma );
}


void CImageView::OnDestroy()
{
	CWnd::OnDestroy();
	DeallocateMembers();
}


void CImageView::InitSquareFrame()
{
	m_SquareFrame.Xtl = 0.0;
	m_SquareFrame.Ytl = 1.0;
	m_SquareFrame.Xbl = 0.0;
	m_SquareFrame.Ybl = 0.0;
	m_SquareFrame.Xbr = 1.0;
	m_SquareFrame.Ybr = 0.0;
	m_SquareFrame.Xtr = 1.0;
	m_SquareFrame.Ytr = 1.0;
}


void CImageView::FlipFrameHorizontally()
{
	GLfloat			Temp;

	Temp = m_SquareFrame.Xtl;
	m_SquareFrame.Xtl = m_SquareFrame.Xtr;
	m_SquareFrame.Xtr = Temp;
	Temp = 	m_SquareFrame.Xbl;
	m_SquareFrame.Xbl = m_SquareFrame.Xbr;
	m_SquareFrame.Xbr = Temp;
}


void CImageView::FlipFrameVertically()
{
	GLfloat			Temp;

	Temp = m_SquareFrame.Ytl;
	m_SquareFrame.Ytl = m_SquareFrame.Ybl;
	m_SquareFrame.Ybl = Temp;
	Temp = 	m_SquareFrame.Ybr;
	m_SquareFrame.Ybr = m_SquareFrame.Ytr;
	m_SquareFrame.Ytr = Temp;
}



