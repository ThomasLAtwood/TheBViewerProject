// GraphicsAdapter.cpp : Implementation file for the CGraphicsAdapter
//  class, which implements the interface to the graphics adapters being used
//  on the BViewer workstation.
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
#include "StdAfx.h"
#include "BViewer.h"
#include "Module.h"
#include "ReportStatus.h"
#include "GraphicsAdapter.h"


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		GraphicsAdapterModuleInfo = { MODULE_GRAPHICS, "Graphics Adapter Module", InitGraphicsAdapterModule, CloseGraphicsAdapterModule };

static ERROR_DICTIONARY_ENTRY	GraphicsAdapterErrorCodes[] =
			{
				{ GRAPHICS_ERROR_INSUFFICIENT_TEXTURE_UNITS		, "Insufficient texture resources were available for image calibration." },
				{ 0												, NULL }
			};

static ERROR_DICTIONARY_MODULE		GraphicsAdapterErrorDictionary =
										{
										MODULE_GRAPHICS,
										GraphicsAdapterErrorCodes,
										GRAPHICS_ERROR_DICT_LENGTH,
										0
										};

// This function must be called before any other function in this module.
void InitGraphicsAdapterModule()
{
	LinkModuleToList( &GraphicsAdapterModuleInfo );
	RegisterErrorDictionary( &GraphicsAdapterErrorDictionary );
}


// This function must be called to deallocate memory and close this module.
void CloseGraphicsAdapterModule()
{
}


static 	DWORD			SystemErrorCode = 0;


CGraphicsAdapter::CGraphicsAdapter(void)
{
	m_pDisplayMonitorInfoList = 0;
	m_pNextGraphicsAdapter = 0;
	m_DisplayMonitorCount = 0;
	m_OpenGLSupportLevel = OPENGL_SUPPORT_ABSENT;
	m_gImageSystemsShaderProgram = NULL;
	m_bAdapterInitializationIsComplete = FALSE;
	m_pFunctionWglGetPixelFormatAttribiv = NULL;
	m_glLUT12BitTextureId = 0;
}

CGraphicsAdapter::~CGraphicsAdapter(void)
{
	if ( m_glLUT12BitTextureId != 0 )
		{
		glActiveTexture( TEXTURE_UNIT_GRAYSCALE_LOOKUP );
		glDeleteTextures( 1, &m_glLUT12BitTextureId );
		glActiveTexture( TEXTURE_UNIT_DEFAULT );
		}
}


static BOOL CheckOpenGLResultAt( char *pSourceFile, int SourceLineNumber )
{
	BOOL			bNoError = TRUE;
	GLenum			GLErrorCode;
	char			MsgBuf[ 64 ];

	GLErrorCode = glGetError();
	if ( GLErrorCode != GL_NO_ERROR )
		{
		sprintf( MsgBuf, "GL Error: %s", gluErrorString( GLErrorCode ) );
		LogMessageAt( pSourceFile, SourceLineNumber, MsgBuf, MESSAGE_TYPE_SUPPLEMENTARY );
		bNoError = FALSE;
		}

	return bNoError;
}


// This version test needs to be repeated every time an image is
// reset, since the display could be moved to another monitor (hence,
// another adapter driver) at any time, leading to a potential change
// in the OpenGL capabilities.  Such a move will typically lead to an
// image reset.
//
// glGetString() will return a zero if there is no current OpenGL connection.
//
double CGraphicsAdapter::GetOpenGLVersion()
{
	const GLubyte	*pVersion;
	char			NumberText[ 64 ];
	char			*pChar;
	char			Msg[ 256 ];

	pVersion = glGetString( GL_VERSION );
	if ( pVersion != 0 )
		{
		strcpy( NumberText, "" );
		strncat( NumberText, (const char*)pVersion, 63 );
		strcpy( m_OpenGLVersion, NumberText );
		pChar = strchr( NumberText, '.' );		// Look for second decimal point.
		pChar = strchr( ++pChar, '.' );
		*pChar = '\0';
		m_OpenGLVersionNumber = atof( NumberText );
		sprintf( Msg, "OpenGL version:  %f", m_OpenGLVersionNumber );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		}
	else
		{
		m_OpenGLVersionNumber = 2;
		}

	return m_OpenGLVersionNumber;
}


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
			// The values returned for WGL_PIXEL_TYPE_ARB:
			// WGL_TYPE_RGBA_UNSIGNED_FLOAT_EXT 0x20A8  = 8360
			// WGL_TYPE_RGBA_FLOAT_ARB 0x21A0  = 8608
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


// This pixel format descriptor is used to initialize conventional 8-bit Microsoft windows.
static PIXELFORMATDESCRIPTOR MSWindowsPixelFormatDescriptor =
	{
	sizeof( PIXELFORMATDESCRIPTOR ),// Size of this structure
	1,								// Version of this structure	
	PFD_DRAW_TO_WINDOW |			// Draw to Window (not to bitmap)
	PFD_SUPPORT_OPENGL |			// Support OpenGL calls in window
	PFD_DOUBLEBUFFER,				// Double buffered mode
	PFD_TYPE_RGBA,					// RGBA Color mode
	32,								// Want 30 bit color 
	0,0,0,0,0,0,					// Not used to select mode
	8,0,							// Alpha bits.
	0,0,0,0,0,						// Not used to select mode
	24,								// Size of depth buffer, depth bits.
	0,								// Stencil bits.
	0,								// Not used to select mode
	0,								// Layer type.
	0,								// Not used to select mode
	0,0,0
	};						// Not used to select mode


typedef char* WINAPI	wglGetExtensionsStringARB_type( HDC hdc );


CString					DummyFrameWindowClass = "";


HGLRC CGraphicsAdapter::CreateWglRenderingContext( HDC hTargetDC )
{
	BOOL								bNoError = TRUE;
	CFrameWnd							*pDummyWindow;
	HWND								hDummyWindow;
	HDC									hDummyDC;
	RECT								DummyWindowRect;
	GLenum								GlewResult;
	int									PixelFormat;
	HGLRC								hDummyRC;
	wglGetExtensionsStringARB_type		*pFunctionWglGetExtensionsString;
	const char							*pWglExtensions;
	HGLRC								hRenderingContext = NULL;
	char								SystemErrorMessage[ FULL_FILE_SPEC_STRING_LENGTH ];
	char								Msg[1024];


 	int		RenderingContextAttributes[] =
				{
				WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
				WGL_CONTEXT_MINOR_VERSION_ARB, 3,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
//				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
				WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
//				WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
				0, 0
				};

	if ( DummyFrameWindowClass.GetLength() == 0 )
		DummyFrameWindowClass = AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS, 
			::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), ThisBViewerApp.m_hApplicationIcon );
	DummyWindowRect = CRect( 0, 0, 800, 600 );

	pDummyWindow = new CFrameWnd;

	if ( !pDummyWindow -> CreateEx( WS_EX_OVERLAPPEDWINDOW, (const char*)DummyFrameWindowClass,
					"Dummy OpenGL Window", WS_CLIPCHILDREN | WS_CLIPSIBLINGS, DummyWindowRect, NULL, 0, NULL ))
		{
		bNoError = FALSE;
		}
	else
		{
		pDummyWindow -> UpdateWindow();
		hDummyWindow = pDummyWindow -> m_hWnd;
		}

	if ( bNoError )
		{
		hDummyDC = GetDC( hDummyWindow );
		PixelFormat = ChoosePixelFormat( hDummyDC, &MSWindowsPixelFormatDescriptor );
		bNoError = ( PixelFormat != 0 );
		}
	if ( bNoError )
		bNoError = SetPixelFormat( hDummyDC, PixelFormat, &MSWindowsPixelFormatDescriptor );
	if ( bNoError )
		{
		hDummyRC = (HGLRC)wglCreateContext( hDummyDC );	// Create an OpenGL rendering context.
		bNoError = ( hDummyRC != 0 );
		}
	if ( bNoError )
		bNoError = wglMakeCurrent( hDummyDC, hDummyRC );		// Make the OpenGL rendering context current.
	if ( bNoError )
		{
		// OpenGL extensions are managed by the OpenGL Extension Wrangler Library (GLEW).
		// To be safe, it should be called after each rendering context change.
		GlewResult = glewInit();			// Initialize extension processing.
		bNoError = ( GlewResult == GLEW_OK );
		if ( !bNoError )
			LogMessage( ">>> Error initializing the Glew Library.", MESSAGE_TYPE_SUPPLEMENTARY );
		}
	if ( !bNoError )
		LogMessage( ">>> An error occurred setting up the dummy window for OpenGL initialization.", MESSAGE_TYPE_SUPPLEMENTARY );

	if ( !m_bAdapterInitializationIsComplete )
		{
		if ( bNoError )
			{
			// Find the 30-bit color pixelformat
			pFunctionWglGetExtensionsString = (wglGetExtensionsStringARB_type*)wglGetProcAddress( "wglGetExtensionsStringARB" );
			bNoError = ( pFunctionWglGetExtensionsString != NULL );
			if ( !bNoError )
				LogMessage( ">>> Error acquiring the OpenGL procedure address for wglGetExtensionsStringARB.", MESSAGE_TYPE_SUPPLEMENTARY );
			}
		if ( bNoError )
			{
			pWglExtensions = pFunctionWglGetExtensionsString( hDummyDC );
			bNoError = ( strstr( pWglExtensions, " WGL_ARB_pixel_format " ) != NULL );
			sprintf( Msg, "Wgl pixel format extension present in this graphics adapter:\n      %s", pWglExtensions );
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
			if ( !bNoError )
				LogMessage( ">>> Error:  The OpenGL Wgl extension WGL_ARB_pixel_format was not found.  Extended pixel format is not supported.", MESSAGE_TYPE_SUPPLEMENTARY );
			}
		if ( bNoError )
			{
			m_pFunctionWglGetPixelFormatAttribiv = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress( "wglGetPixelFormatAttribivARB" );
			m_pFunctionWglChoosePixelFormat = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress( "wglChoosePixelFormatARB" );
			m_pFunctionWglCreateContextAttribs = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress( "wglCreateContextAttribsARB" );
			bNoError = ( m_pFunctionWglGetPixelFormatAttribiv != NULL && m_pFunctionWglChoosePixelFormat != NULL && m_pFunctionWglCreateContextAttribs != NULL );
			if ( !bNoError )
				LogMessage( ">>> Error getting OpenGL extended function pointers.", MESSAGE_TYPE_SUPPLEMENTARY );
			}
		if ( bNoError )
			bNoError = CheckOpenGLCapabilities();
		}

	// The extended format selected below is not displayable.  But we need a displayable format for this
	// window to be viewable, so the following function also selects an appropriate 8-bit pixel format.
	if ( bNoError )
		bNoError = Select30BitColorPixelFormat( hTargetDC );

	if ( bNoError )
		{
		hRenderingContext = m_pFunctionWglCreateContextAttribs( hTargetDC, 0, RenderingContextAttributes );
		bNoError = ( hRenderingContext != NULL );
		if ( bNoError )
			LogMessage( "An OpenGL rendering context was successfully created for this window.", MESSAGE_TYPE_SUPPLEMENTARY );
		else
			{
			SystemErrorCode = GetLastSystemErrorMessage( SystemErrorMessage, FULL_FILE_SPEC_STRING_LENGTH - 1 );
			if ( SystemErrorCode != 0 )
				{
				sprintf( Msg, "Error:  System message:  %s", SystemErrorMessage );
				LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
				}
			}
		CheckOpenGLResultAt( __FILE__, __LINE__	);
		}
	if ( hDummyDC != 0 )
		{
		wglMakeCurrent( hDummyDC, NULL );
		if ( hDummyRC != 0 )
			wglDeleteContext( hDummyRC );
		ReleaseDC( hDummyWindow, hDummyDC );
		delete pDummyWindow;
		}

	m_bAdapterInitializationIsComplete = TRUE;

	return hRenderingContext;
}


BOOL CGraphicsAdapter::Select30BitColorPixelFormat( HDC hDC )
{
	BOOL					bNoError = TRUE;
	unsigned int			nMatchingFormats;
	BOOL					bMatchingFormatFound = FALSE;
	int						PixelFormats[ 100 ];
	int						nFormat;
	int						nDisplayablePixelFormat;
	int						nResult;
	char					Msg[ 256 ];
	char					SystemErrorMessage[ FULL_FILE_SPEC_STRING_LENGTH ];

	// The 10 bits per component is specified in the desired attribute list before calling the wglChoosePixelFormat
	// function which returns the matching pixel formats.
	int					AttribsDesired[] =
							{
//							WGL_DRAW_TO_WINDOW_ARB, GL_FALSE,				// Extended formats are not displayable.
							WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
//							WGL_DOUBLE_BUFFER_ARB, GL_FALSE,				// Only supported for displayable pixel formats.
							WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,

							WGL_RED_BITS_ARB, 11,
							WGL_GREEN_BITS_ARB, 11,
							WGL_BLUE_BITS_ARB, 10,
							WGL_ALPHA_BITS_ARB, 0,

/*
							WGL_RED_BITS_ARB, 10,
							WGL_GREEN_BITS_ARB, 10,
							WGL_BLUE_BITS_ARB, 10,
							WGL_ALPHA_BITS_ARB, 2,
*/
							WGL_DEPTH_BITS_ARB, 0,
							0, 0
							};


	// The extended format selected below is not displayable.  But we need a displayable format for this
	// window to be viewable, so go ahead and select an appropriate 8-bit pixel format.  Use the conveniently
	// available MSWindowsPixelFormatDescriptor above.
	nDisplayablePixelFormat = ChoosePixelFormat( hDC, &MSWindowsPixelFormatDescriptor );
	bNoError = ( nDisplayablePixelFormat != 0 );
	nResult = DescribePixelFormat( hDC, nDisplayablePixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &MSWindowsPixelFormatDescriptor );
	bNoError = ( nResult != 0 );
	if ( !bNoError )
		{
		LogMessage( ">>> An error occurred initializing the conventional Windows pixel format descriptor.", MESSAGE_TYPE_ERROR );
		SystemErrorCode = GetLastSystemErrorMessage( SystemErrorMessage, FULL_FILE_SPEC_STRING_LENGTH - 1 );
		if ( SystemErrorCode != 0 )
			{
			sprintf( Msg, "Error:  System message:  %s", SystemErrorMessage );
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
			}
		}
	else
		{
		bNoError = SetPixelFormat( hDC, nDisplayablePixelFormat, &MSWindowsPixelFormatDescriptor );
		if ( !bNoError )
			{
			LogMessage( ">>> An error occurred setting the conventional Windows pixel format.", MESSAGE_TYPE_ERROR );
			SystemErrorCode = GetLastSystemErrorMessage( SystemErrorMessage, FULL_FILE_SPEC_STRING_LENGTH - 1 );
			if ( SystemErrorCode != 0 )
				{
				sprintf( Msg, "Error:  System message:  %s", SystemErrorMessage );
				LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
				}
			}
		}
	// Initialize the pixel format array.
	for ( nFormat = 0; nFormat < 100; nFormat++ )
		PixelFormats[ nFormat ] = 0;
	// Select the closest matching extended pixel format to be used to represent the image to the graphics card.
	bNoError = m_pFunctionWglChoosePixelFormat( hDC, AttribsDesired, NULL, 100, PixelFormats, &nMatchingFormats );
	if ( bNoError )
		{
		bNoError = ( nMatchingFormats != 0 );
		bMatchingFormatFound = bNoError;
		if ( !bNoError )
			LogMessage( ">>> Error:  No acceptable extended pixel formats were found.", MESSAGE_TYPE_ERROR );
		}
	else
		LogMessage( ">>> An error occurred choosing an extended pixel format.", MESSAGE_TYPE_ERROR );

	if ( bNoError )
		{
		m_Selected10BitPixelFormatNumber = PixelFormats[ 0 ];
		sprintf( Msg, "The selected Pixel Format is %d on graphics adapter %s\n", m_Selected10BitPixelFormatNumber, m_DisplayAdapterName );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		}

	//
	// Write the details of the selected pixel format to the log file.
	//
	nFormat = 0;			// The selected pixel format is in the first element of the pixel format array.
	if ( bMatchingFormatFound )
		LogPixelFormat( hDC, PixelFormats[ nFormat ] );
	else					// If no match was found, log all the available extended pixel formats.
		while ( PixelFormats[ nFormat ] != 0 && nFormat < 50 )		// List all the available matching pixel formats.
			{
			LogPixelFormat( hDC, PixelFormats[ nFormat ] );
			nFormat++;
			}

	return bNoError;
}


// Record the details of the indicated pixel format in the log file.
void CGraphicsAdapter::LogPixelFormat( HDC hDC, int nPixelFormat )
{
	BOOL					bNoError = TRUE;
	char					Msg[ 256 ];
	int						nAttribute;
	BOOL					bEndOfList;
	GL_FORMAT_ATTRIBUTE		*pAttributeInfo;

	if ( nPixelFormat != 0 )
		{
		sprintf( Msg, "\nOpenGL attribute values for Pixel Format %d\n", nPixelFormat );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		nAttribute = 0;
		bEndOfList = FALSE;
		do
			{
			pAttributeInfo = &AttributeTable[ nAttribute ];
			bEndOfList = ( pAttributeInfo -> AttributeID == 0 );
			if ( !bEndOfList )
				{
				bNoError = m_pFunctionWglGetPixelFormatAttribiv( hDC, nPixelFormat, 0, 1,
					&pAttributeInfo -> AttributeID, &pAttributeInfo -> AttributeValue );
				sprintf( Msg, "%s:  %d", pAttributeInfo -> AttributeName, pAttributeInfo -> AttributeValue );
				LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
				}
			nAttribute++;
			} while ( !bEndOfList && bNoError );
		}
}


// This function is called once for each graphics card encountered as the
// various windows are initialized.
BOOL CGraphicsAdapter::CheckOpenGLCapabilities()
{
	BOOL			bNoError = TRUE;
	double			OpenGLMajorVersionNumber;
	char			Msg[ 256 ];

	OpenGLMajorVersionNumber = GetOpenGLVersion();
			
	if ( OpenGLMajorVersionNumber < 3.3 )
		m_OpenGLSupportLevel = OPENGL_SUPPORT_PRIMITIVE;		// OpenGL version is less than 3.3.
	else
		m_OpenGLSupportLevel = OPENGL_SUPPORT_330;				// OpenGL supports NVidia's 30-bit color (and 10-bit grayscale) pixel formats.

	glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &m_MaxTextureUnitsSupportedByGPU );
	sprintf( Msg, "    Max OpenGL texture units supported by GPU = %d", m_MaxTextureUnitsSupportedByGPU );
	LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
			
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &m_MaxTextureSize );
	sprintf( Msg, "    Max OpenGL texture size supported by GPU = %d squared.", m_MaxTextureSize );
	LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

	return bNoError;
}


// could be used for the whole range
unsigned char GrayscaleLookupTable12bit[4096][3] = {
{0, 0, 0}, {0, 0, 1}, {0, 0, 1}, {0, 0, 2}, {0, 0, 2}, {1, 0, 0}, {0, 0, 3}, {1, 0, 1}, {1, 0, 2}, {0, 1, 0}, {2, 0, 0}, {0, 1, 1}, {1, 0, 4}, {0, 1, 2}, {1, 1, 0}, {0, 1, 3}, 
{1, 1, 1}, {0, 1, 4}, {1, 1, 2}, {0, 2, 0}, {1, 1, 3}, {2, 1, 1}, {1, 1, 4}, {2, 1, 2}, {1, 2, 0}, {1, 2, 1}, {3, 1, 1}, {1, 2, 2}, {2, 2, 0}, {1, 2, 3}, {2, 2, 1}, {1, 2, 4}, 
{2, 2, 2}, {1, 3, 0}, {2, 2, 3}, {1, 3, 1}, {2, 2, 4}, {3, 2, 2}, {2, 2, 5}, {3, 2, 3}, {2, 3, 1}, {3, 2, 4}, {2, 3, 2}, {2, 3, 3}, {4, 2, 3}, {2, 3, 4}, {3, 3, 2}, {2, 3, 5}, 
{3, 3, 3}, {2, 4, 1}, {3, 3, 4}, {2, 4, 2}, {3, 3, 5}, {4, 3, 3}, {3, 3, 6}, {4, 3, 4}, {3, 4, 2}, {4, 3, 5}, {3, 4, 3}, {3, 4, 4}, {5, 3, 4}, {3, 4, 5}, {4, 4, 3}, {3, 4, 6}, 
{4, 4, 4}, {3, 5, 2}, {4, 4, 5}, {3, 5, 3}, {4, 4, 6}, {5, 4, 4}, {4, 4, 7}, {5, 4, 5}, {4, 5, 3}, {5, 4, 6}, {4, 5, 4}, {5, 4, 7}, {6, 4, 5}, {4, 5, 6}, {5, 5, 4}, {4, 5, 7}, 
{5, 5, 5}, {4, 6, 3}, {5, 5, 6}, {4, 6, 4}, {5, 5, 7}, {6, 5, 5}, {5, 5, 8}, {6, 5, 6}, {5, 6, 4}, {6, 5, 7}, {5, 6, 5}, {6, 5, 8}, {5, 6, 6}, {5, 6, 7}, {7, 5, 7}, {6, 6, 5}, 
{6, 6, 6}, {5, 7, 4}, {6, 6, 7}, {5, 7, 5}, {6, 6, 8}, {7, 6, 6}, {6, 6, 9}, {7, 6, 7}, {6, 7, 5}, {7, 6, 8}, {6, 7, 6}, {7, 6, 9}, {6, 7, 7}, {6, 7, 8}, {8, 6, 8}, {7, 7, 6}, 
{7, 7, 7}, {6, 8, 5}, {7, 7, 8}, {6, 8, 6}, {7, 7, 9}, {8, 7, 7}, {7, 7, 10}, {8, 7, 8}, {7, 8, 6}, {8, 7, 9}, {7, 8, 7}, {8, 7, 10}, {7, 8, 8}, {8, 8, 6}, {9, 7, 9}, {8, 8, 7}, 
{8, 8, 8}, {7, 9, 6}, {8, 8, 9}, {7, 9, 7}, {8, 8, 10}, {9, 8, 8}, {8, 8, 11}, {9, 8, 9}, {8, 9, 7}, {9, 8, 10}, {8, 9, 8}, {9, 8, 11}, {8, 9, 9}, {9, 9, 7}, {8, 9, 10}, {9, 9, 8}, 
{7, 10, 9}, {9, 9, 9}, {9, 9, 10}, {8, 10, 8}, {9, 9, 11}, {10, 9, 9}, {9, 9, 12}, {10, 9, 10}, {9, 10, 8}, {10, 9, 11}, {9, 10, 9}, {10, 9, 12}, {9, 10, 10}, {10, 10, 8}, {9, 10, 11}, {10, 10, 9}, 
{8, 11, 10}, {10, 10, 10}, {10, 10, 11}, {9, 11, 9}, {10, 10, 12}, {11, 10, 10}, {10, 10, 13}, {11, 10, 11}, {10, 11, 9}, {11, 10, 12}, {10, 11, 10}, {11, 10, 13}, {10, 11, 11}, {11, 11, 9}, {10, 11, 12}, {11, 11, 10}, 
{9, 12, 11}, {11, 11, 11}, {11, 11, 12}, {10, 12, 10}, {11, 11, 13}, {12, 11, 11}, {11, 11, 14}, {12, 11, 12}, {11, 12, 10}, {12, 11, 13}, {11, 12, 11}, {12, 11, 14}, {11, 12, 12}, {12, 12, 10}, {11, 12, 13}, {12, 12, 11}, 
{11, 12, 14}, {12, 12, 12}, {10, 13, 13}, {12, 12, 13}, {12, 12, 14}, {11, 13, 12}, {12, 12, 15}, {13, 12, 13}, {12, 13, 11}, {13, 12, 14}, {12, 13, 12}, {13, 12, 15}, {12, 13, 13}, {13, 13, 11}, {12, 13, 14}, {13, 13, 12}, 
{12, 13, 15}, {13, 13, 13}, {11, 14, 14}, {13, 13, 14}, {12, 14, 12}, {12, 14, 13}, {14, 13, 13}, {14, 13, 14}, {13, 14, 12}, {14, 13, 15}, {13, 14, 13}, {14, 13, 16}, {13, 14, 14}, {14, 14, 12}, {13, 14, 15}, {14, 14, 13}, 
{13, 14, 16}, {14, 14, 14}, {12, 15, 15}, {14, 14, 15}, {13, 15, 13}, {13, 15, 14}, {15, 14, 14}, {15, 14, 15}, {14, 15, 13}, {15, 14, 16}, {14, 15, 14}, {15, 14, 17}, {14, 15, 15}, {15, 15, 13}, {14, 15, 16}, {15, 15, 14}, 
{14, 15, 17}, {15, 15, 15}, {14, 16, 13}, {15, 15, 16}, {14, 16, 14}, {15, 15, 17}, {16, 15, 15}, {14, 16, 16}, {16, 15, 16}, {16, 15, 17}, {15, 16, 15}, {16, 15, 18}, {15, 16, 16}, {16, 16, 14}, {15, 16, 17}, {16, 16, 15}, 
{15, 16, 18}, {16, 16, 16}, {15, 17, 14}, {16, 16, 17}, {15, 17, 15}, {16, 16, 18}, {17, 16, 16}, {15, 17, 17}, {17, 16, 17}, {16, 17, 15}, {16, 17, 16}, {18, 16, 16}, {16, 17, 17}, {17, 17, 15}, {16, 17, 18}, {17, 17, 16}, 
{16, 17, 19}, {17, 17, 17}, {16, 18, 15}, {17, 17, 18}, {16, 18, 16}, {17, 17, 19}, {18, 17, 17}, {16, 18, 18}, {18, 17, 18}, {17, 18, 16}, {17, 18, 17}, {19, 17, 17}, {17, 18, 18}, {18, 18, 16}, {17, 18, 19}, {18, 18, 17}, 
{17, 18, 20}, {18, 18, 18}, {17, 19, 16}, {18, 18, 19}, {17, 19, 17}, {18, 18, 20}, {19, 18, 18}, {18, 18, 21}, {19, 18, 19}, {18, 19, 17}, {18, 19, 18}, {20, 18, 18}, {18, 19, 19}, {19, 19, 17}, {18, 19, 20}, {19, 19, 18}, 
{18, 19, 21}, {19, 19, 19}, {18, 20, 17}, {19, 19, 20}, {18, 20, 18}, {19, 19, 21}, {20, 19, 19}, {19, 19, 22}, {20, 19, 20}, {19, 20, 18}, {20, 19, 21}, {19, 20, 19}, {19, 20, 20}, {21, 19, 20}, {19, 20, 21}, {20, 20, 19}, 
{19, 20, 22}, {20, 20, 20}, {19, 21, 18}, {20, 20, 21}, {19, 21, 19}, {20, 20, 22}, {21, 20, 20}, {20, 20, 23}, {21, 20, 21}, {20, 21, 19}, {21, 20, 22}, {20, 21, 20}, {20, 21, 21}, {22, 20, 21}, {20, 21, 22}, {21, 21, 20}, 
{20, 21, 23}, {21, 21, 21}, {20, 22, 19}, {21, 21, 22}, {20, 22, 20}, {21, 21, 23}, {22, 21, 21}, {21, 21, 24}, {22, 21, 22}, {21, 22, 20}, {22, 21, 23}, {21, 22, 21}, {22, 21, 24}, {23, 21, 22}, {21, 22, 23}, {22, 22, 21}, 
{21, 22, 24}, {22, 22, 22}, {21, 23, 20}, {22, 22, 23}, {21, 23, 21}, {22, 22, 24}, {23, 22, 22}, {22, 22, 25}, {23, 22, 23}, {22, 23, 21}, {23, 22, 24}, {22, 23, 22}, {23, 22, 25}, {22, 23, 23}, {22, 23, 24}, {24, 22, 24}, 
{23, 23, 22}, {23, 23, 23}, {22, 24, 21}, {23, 23, 24}, {22, 24, 22}, {23, 23, 25}, {24, 23, 23}, {23, 23, 26}, {24, 23, 24}, {23, 24, 22}, {24, 23, 25}, {23, 24, 23}, {24, 23, 26}, {23, 24, 24}, {23, 24, 25}, {25, 23, 25}, 
{24, 24, 23}, {24, 24, 24}, {23, 25, 22}, {24, 24, 25}, {23, 25, 23}, {24, 24, 26}, {25, 24, 24}, {24, 24, 27}, {25, 24, 25}, {24, 25, 23}, {25, 24, 26}, {24, 25, 24}, {25, 24, 27}, {24, 25, 25}, {25, 25, 23}, {26, 24, 26}, 
{25, 25, 24}, {25, 25, 25}, {24, 26, 23}, {25, 25, 26}, {24, 26, 24}, {25, 25, 27}, {26, 25, 25}, {25, 25, 28}, {26, 25, 26}, {25, 26, 24}, {26, 25, 27}, {25, 26, 25}, {26, 25, 28}, {25, 26, 26}, {26, 26, 24}, {25, 26, 27}, 
{26, 26, 25}, {24, 27, 26}, {26, 26, 26}, {26, 26, 27}, {25, 27, 25}, {26, 26, 28}, {27, 26, 26}, {26, 26, 29}, {27, 26, 27}, {26, 27, 25}, {27, 26, 28}, {26, 27, 26}, {27, 26, 29}, {26, 27, 27}, {27, 27, 25}, {26, 27, 28}, 
{27, 27, 26}, {25, 28, 27}, {27, 27, 27}, {27, 27, 28}, {26, 28, 26}, {27, 27, 29}, {28, 27, 27}, {27, 27, 30}, {28, 27, 28}, {27, 28, 26}, {28, 27, 29}, {27, 28, 27}, {28, 27, 30}, {27, 28, 28}, {28, 28, 26}, {27, 28, 29}, 
{28, 28, 27}, {26, 29, 28}, {28, 28, 28}, {28, 28, 29}, {27, 29, 27}, {28, 28, 30}, {29, 28, 28}, {28, 28, 31}, {29, 28, 29}, {28, 29, 27}, {29, 28, 30}, {28, 29, 28}, {29, 28, 31}, {28, 29, 29}, {29, 29, 27}, {28, 29, 30}, 
{29, 29, 28}, {28, 29, 31}, {29, 29, 29}, {27, 30, 30}, {29, 29, 30}, {29, 29, 31}, {28, 30, 29}, {29, 29, 32}, {30, 29, 30}, {29, 30, 28}, {30, 29, 31}, {29, 30, 29}, {30, 29, 32}, {29, 30, 30}, {30, 30, 28}, {29, 30, 31}, 
{30, 30, 29}, {29, 30, 32}, {30, 30, 30}, {28, 31, 31}, {30, 30, 31}, {29, 31, 29}, {29, 31, 30}, {31, 30, 30}, {31, 30, 31}, {30, 31, 29}, {31, 30, 32}, {30, 31, 30}, {31, 30, 33}, {30, 31, 31}, {31, 31, 29}, {30, 31, 32}, 
{31, 31, 30}, {30, 31, 33}, {31, 31, 31}, {29, 32, 32}, {31, 31, 32}, {30, 32, 30}, {30, 32, 31}, {32, 31, 31}, {32, 31, 32}, {31, 32, 30}, {32, 31, 33}, {31, 32, 31}, {32, 31, 34}, {31, 32, 32}, {32, 32, 30}, {31, 32, 33}, 
{32, 32, 31}, {31, 32, 34}, {32, 32, 32}, {31, 33, 30}, {32, 32, 33}, {31, 33, 31}, {32, 32, 34}, {33, 32, 32}, {31, 33, 33}, {33, 32, 33}, {33, 32, 34}, {32, 33, 32}, {33, 32, 35}, {32, 33, 33}, {33, 33, 31}, {32, 33, 34}, 
{33, 33, 32}, {32, 33, 35}, {33, 33, 33}, {32, 34, 31}, {33, 33, 34}, {32, 34, 32}, {33, 33, 35}, {34, 33, 33}, {32, 34, 34}, {34, 33, 34}, {33, 34, 32}, {33, 34, 33}, {35, 33, 33}, {33, 34, 34}, {34, 34, 32}, {33, 34, 35}, 
{34, 34, 33}, {33, 34, 36}, {34, 34, 34}, {33, 35, 32}, {34, 34, 35}, {33, 35, 33}, {34, 34, 36}, {35, 34, 34}, {33, 35, 35}, {35, 34, 35}, {34, 35, 33}, {34, 35, 34}, {36, 34, 34}, {34, 35, 35}, {35, 35, 33}, {34, 35, 36}, 
{35, 35, 34}, {34, 35, 37}, {35, 35, 35}, {34, 36, 33}, {35, 35, 36}, {34, 36, 34}, {35, 35, 37}, {36, 35, 35}, {35, 35, 38}, {36, 35, 36}, {35, 36, 34}, {35, 36, 35}, {37, 35, 35}, {35, 36, 36}, {36, 36, 34}, {35, 36, 37}, 
{36, 36, 35}, {35, 36, 38}, {36, 36, 36}, {35, 37, 34}, {36, 36, 37}, {35, 37, 35}, {36, 36, 38}, {37, 36, 36}, {36, 36, 39}, {37, 36, 37}, {36, 37, 35}, {37, 36, 38}, {36, 37, 36}, {36, 37, 37}, {38, 36, 37}, {36, 37, 38}, 
{37, 37, 36}, {36, 37, 39}, {37, 37, 37}, {36, 38, 35}, {37, 37, 38}, {36, 38, 36}, {37, 37, 39}, {38, 37, 37}, {37, 37, 40}, {38, 37, 38}, {37, 38, 36}, {38, 37, 39}, {37, 38, 37}, {37, 38, 38}, {39, 37, 38}, {37, 38, 39}, 
{38, 38, 37}, {37, 38, 40}, {38, 38, 38}, {37, 39, 36}, {38, 38, 39}, {37, 39, 37}, {38, 38, 40}, {39, 38, 38}, {38, 38, 41}, {39, 38, 39}, {38, 39, 37}, {39, 38, 40}, {38, 39, 38}, {39, 38, 41}, {40, 38, 39}, {38, 39, 40}, 
{39, 39, 38}, {38, 39, 41}, {39, 39, 39}, {38, 40, 37}, {39, 39, 40}, {38, 40, 38}, {39, 39, 41}, {40, 39, 39}, {39, 39, 42}, {40, 39, 40}, {39, 40, 38}, {40, 39, 41}, {39, 40, 39}, {40, 39, 42}, {39, 40, 40}, {39, 40, 41}, 
{41, 39, 41}, {40, 40, 39}, {40, 40, 40}, {39, 41, 38}, {40, 40, 41}, {39, 41, 39}, {40, 40, 42}, {41, 40, 40}, {40, 40, 43}, {41, 40, 41}, {40, 41, 39}, {41, 40, 42}, {40, 41, 40}, {41, 40, 43}, {40, 41, 41}, {40, 41, 42}, 
{42, 40, 42}, {41, 41, 40}, {41, 41, 41}, {40, 42, 39}, {41, 41, 42}, {40, 42, 40}, {41, 41, 43}, {42, 41, 41}, {41, 41, 44}, {42, 41, 42}, {41, 42, 40}, {42, 41, 43}, {41, 42, 41}, {42, 41, 44}, {41, 42, 42}, {42, 42, 40}, 
{43, 41, 43}, {42, 42, 41}, {42, 42, 42}, {41, 43, 40}, {42, 42, 43}, {41, 43, 41}, {42, 42, 44}, {43, 42, 42}, {42, 42, 45}, {43, 42, 43}, {42, 43, 41}, {43, 42, 44}, {42, 43, 42}, {43, 42, 45}, {42, 43, 43}, {43, 43, 41}, 
{42, 43, 44}, {43, 43, 42}, {41, 44, 43}, {43, 43, 43}, {43, 43, 44}, {42, 44, 42}, {43, 43, 45}, {44, 43, 43}, {43, 43, 46}, {44, 43, 44}, {43, 44, 42}, {44, 43, 45}, {43, 44, 43}, {44, 43, 46}, {43, 44, 44}, {44, 44, 42}, 
{43, 44, 45}, {44, 44, 43}, {42, 45, 44}, {44, 44, 44}, {44, 44, 45}, {43, 45, 43}, {44, 44, 46}, {45, 44, 44}, {44, 44, 47}, {45, 44, 45}, {44, 45, 43}, {45, 44, 46}, {44, 45, 44}, {45, 44, 47}, {44, 45, 45}, {45, 45, 43}, 
{44, 45, 46}, {45, 45, 44}, {43, 46, 45}, {45, 45, 45}, {45, 45, 46}, {44, 46, 44}, {45, 45, 47}, {46, 45, 45}, {45, 45, 48}, {46, 45, 46}, {45, 46, 44}, {46, 45, 47}, {45, 46, 45}, {46, 45, 48}, {45, 46, 46}, {46, 46, 44}, 
{45, 46, 47}, {46, 46, 45}, {45, 46, 48}, {46, 46, 46}, {44, 47, 47}, {46, 46, 47}, {46, 46, 48}, {45, 47, 46}, {46, 46, 49}, {47, 46, 47}, {46, 47, 45}, {47, 46, 48}, {46, 47, 46}, {47, 46, 49}, {46, 47, 47}, {47, 47, 45}, 
{46, 47, 48}, {47, 47, 46}, {46, 47, 49}, {47, 47, 47}, {45, 48, 48}, {47, 47, 48}, {46, 48, 46}, {46, 48, 47}, {48, 47, 47}, {48, 47, 48}, {47, 48, 46}, {48, 47, 49}, {47, 48, 47}, {48, 47, 50}, {47, 48, 48}, {48, 48, 46}, 
{47, 48, 49}, {48, 48, 47}, {47, 48, 50}, {48, 48, 48}, {46, 49, 49}, {48, 48, 49}, {47, 49, 47}, {47, 49, 48}, {49, 48, 48}, {49, 48, 49}, {48, 49, 47}, {49, 48, 50}, {48, 49, 48}, {49, 48, 51}, {48, 49, 49}, {49, 49, 47}, 
{48, 49, 50}, {49, 49, 48}, {48, 49, 51}, {49, 49, 49}, {48, 50, 47}, {49, 49, 50}, {48, 50, 48}, {49, 49, 51}, {50, 49, 49}, {48, 50, 50}, {50, 49, 50}, {50, 49, 51}, {49, 50, 49}, {50, 49, 52}, {49, 50, 50}, {50, 50, 48}, 
{49, 50, 51}, {50, 50, 49}, {49, 50, 52}, {50, 50, 50}, {49, 51, 48}, {50, 50, 51}, {49, 51, 49}, {50, 50, 52}, {51, 50, 50}, {49, 51, 51}, {51, 50, 51}, {50, 51, 49}, {50, 51, 50}, {52, 50, 50}, {50, 51, 51}, {51, 51, 49}, 
{50, 51, 52}, {51, 51, 50}, {50, 51, 53}, {51, 51, 51}, {50, 52, 49}, {51, 51, 52}, {50, 52, 50}, {51, 51, 53}, {52, 51, 51}, {50, 52, 52}, {52, 51, 52}, {51, 52, 50}, {51, 52, 51}, {53, 51, 51}, {51, 52, 52}, {52, 52, 50}, 
{51, 52, 53}, {52, 52, 51}, {51, 52, 54}, {52, 52, 52}, {51, 53, 50}, {52, 52, 53}, {51, 53, 51}, {52, 52, 54}, {53, 52, 52}, {52, 52, 55}, {53, 52, 53}, {52, 53, 51}, {52, 53, 52}, {54, 52, 52}, {52, 53, 53}, {53, 53, 51}, 
{52, 53, 54}, {53, 53, 52}, {52, 53, 55}, {53, 53, 53}, {52, 54, 51}, {53, 53, 54}, {52, 54, 52}, {53, 53, 55}, {54, 53, 53}, {53, 53, 56}, {54, 53, 54}, {53, 54, 52}, {54, 53, 55}, {53, 54, 53}, {53, 54, 54}, {55, 53, 54}, 
{53, 54, 55}, {54, 54, 53}, {53, 54, 56}, {54, 54, 54}, {53, 55, 52}, {54, 54, 55}, {53, 55, 53}, {54, 54, 56}, {55, 54, 54}, {54, 54, 57}, {55, 54, 55}, {54, 55, 53}, {55, 54, 56}, {54, 55, 54}, {54, 55, 55}, {56, 54, 55}, 
{54, 55, 56}, {55, 55, 54}, {54, 55, 57}, {55, 55, 55}, {54, 56, 53}, {55, 55, 56}, {54, 56, 54}, {55, 55, 57}, {56, 55, 55}, {55, 55, 58}, {56, 55, 56}, {55, 56, 54}, {56, 55, 57}, {55, 56, 55}, {56, 55, 58}, {57, 55, 56}, 
{55, 56, 57}, {56, 56, 55}, {55, 56, 58}, {56, 56, 56}, {55, 57, 54}, {56, 56, 57}, {55, 57, 55}, {56, 56, 58}, {57, 56, 56}, {56, 56, 59}, {57, 56, 57}, {56, 57, 55}, {57, 56, 58}, {56, 57, 56}, {57, 56, 59}, {56, 57, 57}, 
{56, 57, 58}, {58, 56, 58}, {57, 57, 56}, {57, 57, 57}, {56, 58, 55}, {57, 57, 58}, {56, 58, 56}, {57, 57, 59}, {58, 57, 57}, {57, 57, 60}, {58, 57, 58}, {57, 58, 56}, {58, 57, 59}, {57, 58, 57}, {58, 57, 60}, {57, 58, 58}, 
{57, 58, 59}, {59, 57, 59}, {58, 58, 57}, {58, 58, 58}, {57, 59, 56}, {58, 58, 59}, {57, 59, 57}, {58, 58, 60}, {59, 58, 58}, {58, 58, 61}, {59, 58, 59}, {58, 59, 57}, {59, 58, 60}, {58, 59, 58}, {59, 58, 61}, {58, 59, 59}, 
{59, 59, 57}, {60, 58, 60}, {59, 59, 58}, {59, 59, 59}, {58, 60, 57}, {59, 59, 60}, {58, 60, 58}, {59, 59, 61}, {60, 59, 59}, {59, 59, 62}, {60, 59, 60}, {59, 60, 58}, {60, 59, 61}, {59, 60, 59}, {60, 59, 62}, {59, 60, 60}, 
{60, 60, 58}, {59, 60, 61}, {60, 60, 59}, {58, 61, 60}, {60, 60, 60}, {60, 60, 61}, {59, 61, 59}, {60, 60, 62}, {61, 60, 60}, {60, 60, 63}, {61, 60, 61}, {60, 61, 59}, {61, 60, 62}, {60, 61, 60}, {61, 60, 63}, {60, 61, 61}, 
{61, 61, 59}, {60, 61, 62}, {61, 61, 60}, {59, 62, 61}, {61, 61, 61}, {61, 61, 62}, {60, 62, 60}, {61, 61, 63}, {62, 61, 61}, {61, 61, 64}, {62, 61, 62}, {61, 62, 60}, {62, 61, 63}, {61, 62, 61}, {62, 61, 64}, {61, 62, 62}, 
{62, 62, 60}, {61, 62, 63}, {62, 62, 61}, {60, 63, 62}, {62, 62, 62}, {62, 62, 63}, {61, 63, 61}, {62, 62, 64}, {63, 62, 62}, {62, 62, 65}, {63, 62, 63}, {62, 63, 61}, {63, 62, 64}, {62, 63, 62}, {63, 62, 65}, {62, 63, 63}, 
{63, 63, 61}, {62, 63, 64}, {63, 63, 62}, {62, 63, 65}, {63, 63, 63}, {61, 64, 64}, {63, 63, 64}, {63, 63, 65}, {62, 64, 63}, {63, 63, 66}, {64, 63, 64}, {63, 64, 62}, {64, 63, 65}, {63, 64, 63}, {64, 63, 66}, {63, 64, 64}, 
{64, 64, 62}, {63, 64, 65}, {64, 64, 63}, {63, 64, 66}, {64, 64, 64}, {62, 65, 65}, {64, 64, 65}, {63, 65, 63}, {63, 65, 64}, {65, 64, 64}, {65, 64, 65}, {64, 65, 63}, {65, 64, 66}, {64, 65, 64}, {65, 64, 67}, {64, 65, 65}, 
{65, 65, 63}, {64, 65, 66}, {65, 65, 64}, {64, 65, 67}, {65, 65, 65}, {63, 66, 66}, {65, 65, 66}, {64, 66, 64}, {64, 66, 65}, {66, 65, 65}, {66, 65, 66}, {65, 66, 64}, {66, 65, 67}, {65, 66, 65}, {66, 65, 68}, {65, 66, 66}, 
{66, 66, 64}, {65, 66, 67}, {66, 66, 65}, {65, 66, 68}, {66, 66, 66}, {65, 67, 64}, {66, 66, 67}, {65, 67, 65}, {66, 66, 68}, {67, 66, 66}, {65, 67, 67}, {67, 66, 67}, {67, 66, 68}, {66, 67, 66}, {67, 66, 69}, {66, 67, 67}, 
{67, 67, 65}, {66, 67, 68}, {67, 67, 66}, {66, 67, 69}, {67, 67, 67}, {66, 68, 65}, {67, 67, 68}, {66, 68, 66}, {67, 67, 69}, {68, 67, 67}, {66, 68, 68}, {68, 67, 68}, {67, 68, 66}, {67, 68, 67}, {69, 67, 67}, {67, 68, 68}, 
{68, 68, 66}, {67, 68, 69}, {68, 68, 67}, {67, 68, 70}, {68, 68, 68}, {67, 69, 66}, {68, 68, 69}, {67, 69, 67}, {68, 68, 70}, {69, 68, 68}, {67, 69, 69}, {69, 68, 69}, {68, 69, 67}, {68, 69, 68}, {70, 68, 68}, {68, 69, 69}, 
{69, 69, 67}, {68, 69, 70}, {69, 69, 68}, {68, 69, 71}, {69, 69, 69}, {68, 70, 67}, {69, 69, 70}, {68, 70, 68}, {69, 69, 71}, {70, 69, 69}, {69, 69, 72}, {70, 69, 70}, {69, 70, 68}, {69, 70, 69}, {71, 69, 69}, {69, 70, 70}, 
{70, 70, 68}, {69, 70, 71}, {70, 70, 69}, {69, 70, 72}, {70, 70, 70}, {69, 71, 68}, {70, 70, 71}, {69, 71, 69}, {70, 70, 72}, {71, 70, 70}, {70, 70, 73}, {71, 70, 71}, {70, 71, 69}, {71, 70, 72}, {70, 71, 70}, {70, 71, 71}, 
{72, 70, 71}, {70, 71, 72}, {71, 71, 70}, {70, 71, 73}, {71, 71, 71}, {70, 72, 69}, {71, 71, 72}, {70, 72, 70}, {71, 71, 73}, {72, 71, 71}, {71, 71, 74}, {72, 71, 72}, {71, 72, 70}, {72, 71, 73}, {71, 72, 71}, {71, 72, 72}, 
{73, 71, 72}, {71, 72, 73}, {72, 72, 71}, {71, 72, 74}, {72, 72, 72}, {71, 73, 70}, {72, 72, 73}, {71, 73, 71}, {72, 72, 74}, {73, 72, 72}, {72, 72, 75}, {73, 72, 73}, {72, 73, 71}, {73, 72, 74}, {72, 73, 72}, {73, 72, 75}, 
{74, 72, 73}, {72, 73, 74}, {73, 73, 72}, {72, 73, 75}, {73, 73, 73}, {72, 74, 71}, {73, 73, 74}, {72, 74, 72}, {73, 73, 75}, {74, 73, 73}, {73, 73, 76}, {74, 73, 74}, {73, 74, 72}, {74, 73, 75}, {73, 74, 73}, {74, 73, 76}, 
{73, 74, 74}, {73, 74, 75}, {75, 73, 75}, {74, 74, 73}, {74, 74, 74}, {73, 75, 72}, {74, 74, 75}, {73, 75, 73}, {74, 74, 76}, {75, 74, 74}, {74, 74, 77}, {75, 74, 75}, {74, 75, 73}, {75, 74, 76}, {74, 75, 74}, {75, 74, 77}, 
{74, 75, 75}, {74, 75, 76}, {76, 74, 76}, {75, 75, 74}, {75, 75, 75}, {74, 76, 73}, {75, 75, 76}, {74, 76, 74}, {75, 75, 77}, {76, 75, 75}, {75, 75, 78}, {76, 75, 76}, {75, 76, 74}, {76, 75, 77}, {75, 76, 75}, {76, 75, 78}, 
{75, 76, 76}, {76, 76, 74}, {77, 75, 77}, {76, 76, 75}, {76, 76, 76}, {75, 77, 74}, {76, 76, 77}, {75, 77, 75}, {76, 76, 78}, {77, 76, 76}, {76, 76, 79}, {77, 76, 77}, {76, 77, 75}, {77, 76, 78}, {76, 77, 76}, {77, 76, 79}, 
{76, 77, 77}, {77, 77, 75}, {76, 77, 78}, {77, 77, 76}, {75, 78, 77}, {77, 77, 77}, {77, 77, 78}, {76, 78, 76}, {77, 77, 79}, {78, 77, 77}, {77, 77, 80}, {78, 77, 78}, {77, 78, 76}, {78, 77, 79}, {77, 78, 77}, {78, 77, 80}, 
{77, 78, 78}, {78, 78, 76}, {77, 78, 79}, {78, 78, 77}, {76, 79, 78}, {78, 78, 78}, {78, 78, 79}, {77, 79, 77}, {78, 78, 80}, {79, 78, 78}, {78, 78, 81}, {79, 78, 79}, {78, 79, 77}, {79, 78, 80}, {78, 79, 78}, {79, 78, 81}, 
{78, 79, 79}, {79, 79, 77}, {78, 79, 80}, {79, 79, 78}, {77, 80, 79}, {79, 79, 79}, {79, 79, 80}, {78, 80, 78}, {79, 79, 81}, {80, 79, 79}, {79, 79, 82}, {80, 79, 80}, {79, 80, 78}, {80, 79, 81}, {79, 80, 79}, {80, 79, 82}, 
{79, 80, 80}, {80, 80, 78}, {79, 80, 81}, {80, 80, 79}, {79, 80, 82}, {80, 80, 80}, {78, 81, 81}, {80, 80, 81}, {80, 80, 82}, {79, 81, 80}, {80, 80, 83}, {81, 80, 81}, {80, 81, 79}, {81, 80, 82}, {80, 81, 80}, {81, 80, 83}, 
{80, 81, 81}, {81, 81, 79}, {80, 81, 82}, {81, 81, 80}, {80, 81, 83}, {81, 81, 81}, {79, 82, 82}, {81, 81, 82}, {80, 82, 80}, {80, 82, 81}, {82, 81, 81}, {82, 81, 82}, {81, 82, 80}, {82, 81, 83}, {81, 82, 81}, {82, 81, 84}, 
{81, 82, 82}, {82, 82, 80}, {81, 82, 83}, {82, 82, 81}, {81, 82, 84}, {82, 82, 82}, {80, 83, 83}, {82, 82, 83}, {81, 83, 81}, {81, 83, 82}, {83, 82, 82}, {83, 82, 83}, {82, 83, 81}, {83, 82, 84}, {82, 83, 82}, {83, 82, 85}, 
{82, 83, 83}, {83, 83, 81}, {82, 83, 84}, {83, 83, 82}, {82, 83, 85}, {83, 83, 83}, {82, 84, 81}, {83, 83, 84}, {82, 84, 82}, {83, 83, 85}, {84, 83, 83}, {82, 84, 84}, {84, 83, 84}, {84, 83, 85}, {83, 84, 83}, {84, 83, 86}, 
{83, 84, 84}, {84, 84, 82}, {83, 84, 85}, {84, 84, 83}, {83, 84, 86}, {84, 84, 84}, {83, 85, 82}, {84, 84, 85}, {83, 85, 83}, {84, 84, 86}, {85, 84, 84}, {83, 85, 85}, {85, 84, 85}, {84, 85, 83}, {84, 85, 84}, {86, 84, 84}, 
{84, 85, 85}, {85, 85, 83}, {84, 85, 86}, {85, 85, 84}, {84, 85, 87}, {85, 85, 85}, {84, 86, 83}, {85, 85, 86}, {84, 86, 84}, {85, 85, 87}, {86, 85, 85}, {84, 86, 86}, {86, 85, 86}, {85, 86, 84}, {85, 86, 85}, {87, 85, 85}, 
{85, 86, 86}, {86, 86, 84}, {85, 86, 87}, {86, 86, 85}, {85, 86, 88}, {86, 86, 86}, {85, 87, 84}, {86, 86, 87}, {85, 87, 85}, {86, 86, 88}, {87, 86, 86}, {86, 86, 89}, {87, 86, 87}, {86, 87, 85}, {86, 87, 86}, {88, 86, 86}, 
{86, 87, 87}, {87, 87, 85}, {86, 87, 88}, {87, 87, 86}, {86, 87, 89}, {87, 87, 87}, {86, 88, 85}, {87, 87, 88}, {86, 88, 86}, {87, 87, 89}, {88, 87, 87}, {87, 87, 90}, {88, 87, 88}, {87, 88, 86}, {88, 87, 89}, {87, 88, 87}, 
{87, 88, 88}, {89, 87, 88}, {87, 88, 89}, {88, 88, 87}, {87, 88, 90}, {88, 88, 88}, {87, 89, 86}, {88, 88, 89}, {87, 89, 87}, {88, 88, 90}, {89, 88, 88}, {88, 88, 91}, {89, 88, 89}, {88, 89, 87}, {89, 88, 90}, {88, 89, 88}, 
{88, 89, 89}, {90, 88, 89}, {88, 89, 90}, {89, 89, 88}, {88, 89, 91}, {89, 89, 89}, {88, 90, 87}, {89, 89, 90}, {88, 90, 88}, {89, 89, 91}, {90, 89, 89}, {89, 89, 92}, {90, 89, 90}, {89, 90, 88}, {90, 89, 91}, {89, 90, 89}, 
{90, 89, 92}, {91, 89, 90}, {89, 90, 91}, {90, 90, 89}, {89, 90, 92}, {90, 90, 90}, {89, 91, 88}, {90, 90, 91}, {89, 91, 89}, {90, 90, 92}, {91, 90, 90}, {90, 90, 93}, {91, 90, 91}, {90, 91, 89}, {91, 90, 92}, {90, 91, 90}, 
{91, 90, 93}, {90, 91, 91}, {90, 91, 92}, {92, 90, 92}, {91, 91, 90}, {91, 91, 91}, {90, 92, 89}, {91, 91, 92}, {90, 92, 90}, {91, 91, 93}, {92, 91, 91}, {91, 91, 94}, {92, 91, 92}, {91, 92, 90}, {92, 91, 93}, {91, 92, 91}, 
{92, 91, 94}, {91, 92, 92}, {91, 92, 93}, {93, 91, 93}, {92, 92, 91}, {92, 92, 92}, {91, 93, 90}, {92, 92, 93}, {91, 93, 91}, {92, 92, 94}, {93, 92, 92}, {92, 92, 95}, {93, 92, 93}, {92, 93, 91}, {93, 92, 94}, {92, 93, 92}, 
{93, 92, 95}, {92, 93, 93}, {93, 93, 91}, {94, 92, 94}, {93, 93, 92}, {93, 93, 93}, {92, 94, 91}, {93, 93, 94}, {92, 94, 92}, {93, 93, 95}, {94, 93, 93}, {93, 93, 96}, {94, 93, 94}, {93, 94, 92}, {94, 93, 95}, {93, 94, 93}, 
{94, 93, 96}, {93, 94, 94}, {94, 94, 92}, {93, 94, 95}, {94, 94, 93}, {92, 95, 94}, {94, 94, 94}, {94, 94, 95}, {93, 95, 93}, {94, 94, 96}, {95, 94, 94}, {94, 94, 97}, {95, 94, 95}, {94, 95, 93}, {95, 94, 96}, {94, 95, 94}, 
{95, 94, 97}, {94, 95, 95}, {95, 95, 93}, {94, 95, 96}, {95, 95, 94}, {93, 96, 95}, {95, 95, 95}, {95, 95, 96}, {94, 96, 94}, {95, 95, 97}, {96, 95, 95}, {95, 95, 98}, {96, 95, 96}, {95, 96, 94}, {96, 95, 97}, {95, 96, 95}, 
{96, 95, 98}, {95, 96, 96}, {96, 96, 94}, {95, 96, 97}, {96, 96, 95}, {94, 97, 96}, {96, 96, 96}, {96, 96, 97}, {95, 97, 95}, {96, 96, 98}, {97, 96, 96}, {96, 96, 99}, {97, 96, 97}, {96, 97, 95}, {97, 96, 98}, {96, 97, 96}, 
{97, 96, 99}, {96, 97, 97}, {97, 97, 95}, {96, 97, 98}, {97, 97, 96}, {96, 97, 99}, {97, 97, 97}, {95, 98, 98}, {97, 97, 98}, {97, 97, 99}, {96, 98, 97}, {97, 97, 100}, {98, 97, 98}, {97, 98, 96}, {98, 97, 99}, {97, 98, 97}, 
{98, 97, 100}, {97, 98, 98}, {98, 98, 96}, {97, 98, 99}, {98, 98, 97}, {97, 98, 100}, {98, 98, 98}, {96, 99, 99}, {98, 98, 99}, {97, 99, 97}, {97, 99, 98}, {99, 98, 98}, {99, 98, 99}, {98, 99, 97}, {99, 98, 100}, {98, 99, 98}, 
{99, 98, 101}, {98, 99, 99}, {99, 99, 97}, {98, 99, 100}, {99, 99, 98}, {98, 99, 101}, {99, 99, 99}, {97, 100, 100}, {99, 99, 100}, {98, 100, 98}, {98, 100, 99}, {100, 99, 99}, {100, 99, 100}, {99, 100, 98}, {100, 99, 101}, {99, 100, 99}, 
{100, 99, 102}, {99, 100, 100}, {100, 100, 98}, {99, 100, 101}, {100, 100, 99}, {99, 100, 102}, {100, 100, 100}, {99, 101, 98}, {100, 100, 101}, {99, 101, 99}, {100, 100, 102}, {101, 100, 100}, {99, 101, 101}, {101, 100, 101}, {101, 100, 102}, {100, 101, 100}, 
{101, 100, 103}, {100, 101, 101}, {101, 101, 99}, {100, 101, 102}, {101, 101, 100}, {100, 101, 103}, {101, 101, 101}, {100, 102, 99}, {101, 101, 102}, {100, 102, 100}, {101, 101, 103}, {102, 101, 101}, {100, 102, 102}, {102, 101, 102}, {101, 102, 100}, {101, 102, 101}, 
{103, 101, 101}, {101, 102, 102}, {102, 102, 100}, {101, 102, 103}, {102, 102, 101}, {101, 102, 104}, {102, 102, 102}, {101, 103, 100}, {102, 102, 103}, {101, 103, 101}, {102, 102, 104}, {103, 102, 102}, {101, 103, 103}, {103, 102, 103}, {102, 103, 101}, {102, 103, 102}, 
{104, 102, 102}, {102, 103, 103}, {103, 103, 101}, {102, 103, 104}, {103, 103, 102}, {102, 103, 105}, {103, 103, 103}, {102, 104, 101}, {103, 103, 104}, {102, 104, 102}, {103, 103, 105}, {104, 103, 103}, {103, 103, 106}, {104, 103, 104}, {103, 104, 102}, {103, 104, 103}, 
{105, 103, 103}, {103, 104, 104}, {104, 104, 102}, {103, 104, 105}, {104, 104, 103}, {103, 104, 106}, {104, 104, 104}, {103, 105, 102}, {104, 104, 105}, {103, 105, 103}, {104, 104, 106}, {105, 104, 104}, {104, 104, 107}, {105, 104, 105}, {104, 105, 103}, {105, 104, 106}, 
{104, 105, 104}, {104, 105, 105}, {106, 104, 105}, {104, 105, 106}, {105, 105, 104}, {104, 105, 107}, {105, 105, 105}, {104, 106, 103}, {105, 105, 106}, {104, 106, 104}, {105, 105, 107}, {106, 105, 105}, {105, 105, 108}, {106, 105, 106}, {105, 106, 104}, {106, 105, 107}, 
{105, 106, 105}, {105, 106, 106}, {107, 105, 106}, {105, 106, 107}, {106, 106, 105}, {105, 106, 108}, {106, 106, 106}, {105, 107, 104}, {106, 106, 107}, {105, 107, 105}, {106, 106, 108}, {107, 106, 106}, {106, 106, 109}, {107, 106, 107}, {106, 107, 105}, {107, 106, 108}, 
{106, 107, 106}, {107, 106, 109}, {108, 106, 107}, {106, 107, 108}, {107, 107, 106}, {106, 107, 109}, {107, 107, 107}, {106, 108, 105}, {107, 107, 108}, {106, 108, 106}, {107, 107, 109}, {108, 107, 107}, {107, 107, 110}, {108, 107, 108}, {107, 108, 106}, {108, 107, 109}, 
{107, 108, 107}, {108, 107, 110}, {107, 108, 108}, {107, 108, 109}, {109, 107, 109}, {108, 108, 107}, {108, 108, 108}, {107, 109, 106}, {108, 108, 109}, {107, 109, 107}, {108, 108, 110}, {109, 108, 108}, {108, 108, 111}, {109, 108, 109}, {108, 109, 107}, {109, 108, 110}, 
{108, 109, 108}, {109, 108, 111}, {108, 109, 109}, {108, 109, 110}, {110, 108, 110}, {109, 109, 108}, {109, 109, 109}, {108, 110, 107}, {109, 109, 110}, {108, 110, 108}, {109, 109, 111}, {110, 109, 109}, {109, 109, 112}, {110, 109, 110}, {109, 110, 108}, {110, 109, 111}, 
{109, 110, 109}, {110, 109, 112}, {109, 110, 110}, {110, 110, 108}, {111, 109, 111}, {110, 110, 109}, {110, 110, 110}, {109, 111, 108}, {110, 110, 111}, {109, 111, 109}, {110, 110, 112}, {111, 110, 110}, {110, 110, 113}, {111, 110, 111}, {110, 111, 109}, {111, 110, 112}, 
{110, 111, 110}, {111, 110, 113}, {110, 111, 111}, {111, 111, 109}, {110, 111, 112}, {111, 111, 110}, {109, 112, 111}, {111, 111, 111}, {111, 111, 112}, {110, 112, 110}, {111, 111, 113}, {112, 111, 111}, {111, 111, 114}, {112, 111, 112}, {111, 112, 110}, {112, 111, 113}, 
{111, 112, 111}, {112, 111, 114}, {111, 112, 112}, {112, 112, 110}, {111, 112, 113}, {112, 112, 111}, {110, 113, 112}, {112, 112, 112}, {112, 112, 113}, {111, 113, 111}, {112, 112, 114}, {113, 112, 112}, {112, 112, 115}, {113, 112, 113}, {112, 113, 111}, {113, 112, 114}, 
{112, 113, 112}, {113, 112, 115}, {112, 113, 113}, {113, 113, 111}, {112, 113, 114}, {113, 113, 112}, {111, 114, 113}, {113, 113, 113}, {113, 113, 114}, {112, 114, 112}, {113, 113, 115}, {114, 113, 113}, {113, 113, 116}, {114, 113, 114}, {113, 114, 112}, {114, 113, 115}, 
{113, 114, 113}, {114, 113, 116}, {113, 114, 114}, {114, 114, 112}, {113, 114, 115}, {114, 114, 113}, {113, 114, 116}, {114, 114, 114}, {112, 115, 115}, {114, 114, 115}, {114, 114, 116}, {113, 115, 114}, {114, 114, 117}, {115, 114, 115}, {114, 115, 113}, {115, 114, 116}, 
{114, 115, 114}, {115, 114, 117}, {114, 115, 115}, {115, 115, 113}, {114, 115, 116}, {115, 115, 114}, {114, 115, 117}, {115, 115, 115}, {113, 116, 116}, {115, 115, 116}, {114, 116, 114}, {114, 116, 115}, {116, 115, 115}, {116, 115, 116}, {115, 116, 114}, {116, 115, 117}, 
{115, 116, 115}, {116, 115, 118}, {115, 116, 116}, {116, 116, 114}, {115, 116, 117}, {116, 116, 115}, {115, 116, 118}, {116, 116, 116}, {114, 117, 117}, {116, 116, 117}, {115, 117, 115}, {115, 117, 116}, {117, 116, 116}, {117, 116, 117}, {116, 117, 115}, {117, 116, 118}, 
{116, 117, 116}, {117, 116, 119}, {116, 117, 117}, {117, 117, 115}, {116, 117, 118}, {117, 117, 116}, {116, 117, 119}, {117, 117, 117}, {116, 118, 115}, {117, 117, 118}, {116, 118, 116}, {117, 117, 119}, {118, 117, 117}, {116, 118, 118}, {118, 117, 118}, {118, 117, 119}, 
{117, 118, 117}, {118, 117, 120}, {117, 118, 118}, {118, 118, 116}, {117, 118, 119}, {118, 118, 117}, {117, 118, 120}, {118, 118, 118}, {117, 119, 116}, {118, 118, 119}, {117, 119, 117}, {118, 118, 120}, {119, 118, 118}, {117, 119, 119}, {119, 118, 119}, {118, 119, 117}, 
{118, 119, 118}, {120, 118, 118}, {118, 119, 119}, {119, 119, 117}, {118, 119, 120}, {119, 119, 118}, {118, 119, 121}, {119, 119, 119}, {118, 120, 117}, {119, 119, 120}, {118, 120, 118}, {119, 119, 121}, {120, 119, 119}, {118, 120, 120}, {120, 119, 120}, {119, 120, 118}, 
{119, 120, 119}, {121, 119, 119}, {119, 120, 120}, {120, 120, 118}, {119, 120, 121}, {120, 120, 119}, {119, 120, 122}, {120, 120, 120}, {119, 121, 118}, {120, 120, 121}, {119, 121, 119}, {120, 120, 122}, {121, 120, 120}, {120, 120, 123}, {121, 120, 121}, {120, 121, 119}, 
{120, 121, 120}, {122, 120, 120}, {120, 121, 121}, {121, 121, 119}, {120, 121, 122}, {121, 121, 120}, {120, 121, 123}, {121, 121, 121}, {120, 122, 119}, {121, 121, 122}, {120, 122, 120}, {121, 121, 123}, {122, 121, 121}, {121, 121, 124}, {122, 121, 122}, {121, 122, 120}, 
{122, 121, 123}, {121, 122, 121}, {121, 122, 122}, {123, 121, 122}, {121, 122, 123}, {122, 122, 121}, {121, 122, 124}, {122, 122, 122}, {121, 123, 120}, {122, 122, 123}, {121, 123, 121}, {122, 122, 124}, {123, 122, 122}, {122, 122, 125}, {123, 122, 123}, {122, 123, 121}, 
{123, 122, 124}, {122, 123, 122}, {122, 123, 123}, {124, 122, 123}, {122, 123, 124}, {123, 123, 122}, {122, 123, 125}, {123, 123, 123}, {122, 124, 121}, {123, 123, 124}, {122, 124, 122}, {123, 123, 125}, {124, 123, 123}, {123, 123, 126}, {124, 123, 124}, {123, 124, 122}, 
{124, 123, 125}, {123, 124, 123}, {124, 123, 126}, {125, 123, 124}, {123, 124, 125}, {124, 124, 123}, {123, 124, 126}, {124, 124, 124}, {123, 125, 122}, {124, 124, 125}, {123, 125, 123}, {124, 124, 126}, {125, 124, 124}, {124, 124, 127}, {125, 124, 125}, {124, 125, 123}, 
{125, 124, 126}, {124, 125, 124}, {125, 124, 127}, {124, 125, 125}, {124, 125, 126}, {126, 124, 126}, {125, 125, 124}, {125, 125, 125}, {124, 126, 123}, {125, 125, 126}, {124, 126, 124}, {125, 125, 127}, {126, 125, 125}, {125, 125, 128}, {126, 125, 126}, {125, 126, 124}, 
{126, 125, 127}, {125, 126, 125}, {126, 125, 128}, {125, 126, 126}, {125, 126, 127}, {127, 125, 127}, {126, 126, 125}, {126, 126, 126}, {125, 127, 124}, {126, 126, 127}, {125, 127, 125}, {126, 126, 128}, {127, 126, 126}, {126, 126, 129}, {127, 126, 127}, {126, 127, 125}, 
{127, 126, 128}, {126, 127, 126}, {127, 126, 129}, {126, 127, 127}, {127, 127, 125}, {128, 126, 128}, {127, 127, 126}, {127, 127, 127}, {126, 128, 125}, {127, 127, 128}, {126, 128, 126}, {127, 127, 129}, {128, 127, 127}, {127, 127, 130}, {128, 127, 128}, {127, 128, 126}, 
{128, 127, 129}, {127, 128, 127}, {128, 127, 130}, {127, 128, 128}, {128, 128, 126}, {127, 128, 129}, {128, 128, 127}, {126, 129, 128}, {128, 128, 128}, {128, 128, 129}, {127, 129, 127}, {128, 128, 130}, {129, 128, 128}, {128, 128, 131}, {129, 128, 129}, {128, 129, 127}, 
{129, 128, 130}, {128, 129, 128}, {129, 128, 131}, {128, 129, 129}, {129, 129, 127}, {128, 129, 130}, {129, 129, 128}, {127, 130, 129}, {129, 129, 129}, {129, 129, 130}, {128, 130, 128}, {129, 129, 131}, {130, 129, 129}, {129, 129, 132}, {130, 129, 130}, {129, 130, 128}, 
{130, 129, 131}, {129, 130, 129}, {130, 129, 132}, {129, 130, 130}, {130, 130, 128}, {129, 130, 131}, {130, 130, 129}, {128, 131, 130}, {130, 130, 130}, {130, 130, 131}, {129, 131, 129}, {130, 130, 132}, {131, 130, 130}, {130, 130, 133}, {131, 130, 131}, {130, 131, 129}, 
{131, 130, 132}, {130, 131, 130}, {131, 130, 133}, {130, 131, 131}, {131, 131, 129}, {130, 131, 132}, {131, 131, 130}, {130, 131, 133}, {131, 131, 131}, {129, 132, 132}, {131, 131, 132}, {131, 131, 133}, {130, 132, 131}, {131, 131, 134}, {132, 131, 132}, {131, 132, 130}, 
{132, 131, 133}, {131, 132, 131}, {132, 131, 134}, {131, 132, 132}, {132, 132, 130}, {131, 132, 133}, {132, 132, 131}, {131, 132, 134}, {132, 132, 132}, {130, 133, 133}, {132, 132, 133}, {131, 133, 131}, {131, 133, 132}, {133, 132, 132}, {133, 132, 133}, {132, 133, 131}, 
{133, 132, 134}, {132, 133, 132}, {133, 132, 135}, {132, 133, 133}, {133, 133, 131}, {132, 133, 134}, {133, 133, 132}, {132, 133, 135}, {133, 133, 133}, {131, 134, 134}, {133, 133, 134}, {132, 134, 132}, {132, 134, 133}, {134, 133, 133}, {134, 133, 134}, {133, 134, 132}, 
{134, 133, 135}, {133, 134, 133}, {134, 133, 136}, {133, 134, 134}, {134, 134, 132}, {133, 134, 135}, {134, 134, 133}, {133, 134, 136}, {134, 134, 134}, {133, 135, 132}, {134, 134, 135}, {133, 135, 133}, {134, 134, 136}, {135, 134, 134}, {133, 135, 135}, {135, 134, 135}, 
{135, 134, 136}, {134, 135, 134}, {135, 134, 137}, {134, 135, 135}, {135, 135, 133}, {134, 135, 136}, {135, 135, 134}, {134, 135, 137}, {135, 135, 135}, {134, 136, 133}, {135, 135, 136}, {134, 136, 134}, {135, 135, 137}, {136, 135, 135}, {134, 136, 136}, {136, 135, 136}, 
{135, 136, 134}, {135, 136, 135}, {137, 135, 135}, {135, 136, 136}, {136, 136, 134}, {135, 136, 137}, {136, 136, 135}, {135, 136, 138}, {136, 136, 136}, {135, 137, 134}, {136, 136, 137}, {135, 137, 135}, {136, 136, 138}, {137, 136, 136}, {135, 137, 137}, {137, 136, 137}, 
{136, 137, 135}, {136, 137, 136}, {138, 136, 136}, {136, 137, 137}, {137, 137, 135}, {136, 137, 138}, {137, 137, 136}, {136, 137, 139}, {137, 137, 137}, {136, 138, 135}, {137, 137, 138}, {136, 138, 136}, {137, 137, 139}, {138, 137, 137}, {137, 137, 140}, {138, 137, 138}, 
{137, 138, 136}, {137, 138, 137}, {139, 137, 137}, {137, 138, 138}, {138, 138, 136}, {137, 138, 139}, {138, 138, 137}, {137, 138, 140}, {138, 138, 138}, {137, 139, 136}, {138, 138, 139}, {137, 139, 137}, {138, 138, 140}, {139, 138, 138}, {138, 138, 141}, {139, 138, 139}, 
{138, 139, 137}, {139, 138, 140}, {138, 139, 138}, {138, 139, 139}, {140, 138, 139}, {138, 139, 140}, {139, 139, 138}, {138, 139, 141}, {139, 139, 139}, {138, 140, 137}, {139, 139, 140}, {138, 140, 138}, {139, 139, 141}, {140, 139, 139}, {139, 139, 142}, {140, 139, 140}, 
{139, 140, 138}, {140, 139, 141}, {139, 140, 139}, {139, 140, 140}, {141, 139, 140}, {139, 140, 141}, {140, 140, 139}, {139, 140, 142}, {140, 140, 140}, {139, 141, 138}, {140, 140, 141}, {139, 141, 139}, {140, 140, 142}, {141, 140, 140}, {140, 140, 143}, {141, 140, 141}, 
{140, 141, 139}, {141, 140, 142}, {140, 141, 140}, {141, 140, 143}, {142, 140, 141}, {140, 141, 142}, {141, 141, 140}, {140, 141, 143}, {141, 141, 141}, {140, 142, 139}, {141, 141, 142}, {140, 142, 140}, {141, 141, 143}, {142, 141, 141}, {141, 141, 144}, {142, 141, 142}, 
{141, 142, 140}, {142, 141, 143}, {141, 142, 141}, {142, 141, 144}, {141, 142, 142}, {141, 142, 143}, {143, 141, 143}, {142, 142, 141}, {142, 142, 142}, {141, 143, 140}, {142, 142, 143}, {141, 143, 141}, {142, 142, 144}, {143, 142, 142}, {142, 142, 145}, {143, 142, 143}, 
{142, 143, 141}, {143, 142, 144}, {142, 143, 142}, {143, 142, 145}, {142, 143, 143}, {142, 143, 144}, {144, 142, 144}, {143, 143, 142}, {143, 143, 143}, {142, 144, 141}, {143, 143, 144}, {142, 144, 142}, {143, 143, 145}, {144, 143, 143}, {143, 143, 146}, {144, 143, 144}, 
{143, 144, 142}, {144, 143, 145}, {143, 144, 143}, {144, 143, 146}, {143, 144, 144}, {144, 144, 142}, {145, 143, 145}, {144, 144, 143}, {144, 144, 144}, {143, 145, 142}, {144, 144, 145}, {143, 145, 143}, {144, 144, 146}, {145, 144, 144}, {144, 144, 147}, {145, 144, 145}, 
{144, 145, 143}, {145, 144, 146}, {144, 145, 144}, {145, 144, 147}, {144, 145, 145}, {145, 145, 143}, {144, 145, 146}, {145, 145, 144}, {143, 146, 145}, {145, 145, 145}, {145, 145, 146}, {144, 146, 144}, {145, 145, 147}, {146, 145, 145}, {145, 145, 148}, {146, 145, 146}, 
{145, 146, 144}, {146, 145, 147}, {145, 146, 145}, {146, 145, 148}, {145, 146, 146}, {146, 146, 144}, {145, 146, 147}, {146, 146, 145}, {144, 147, 146}, {146, 146, 146}, {146, 146, 147}, {145, 147, 145}, {146, 146, 148}, {147, 146, 146}, {146, 146, 149}, {147, 146, 147}, 
{146, 147, 145}, {147, 146, 148}, {146, 147, 146}, {147, 146, 149}, {146, 147, 147}, {147, 147, 145}, {146, 147, 148}, {147, 147, 146}, {145, 148, 147}, {147, 147, 147}, {147, 147, 148}, {146, 148, 146}, {147, 147, 149}, {148, 147, 147}, {147, 147, 150}, {148, 147, 148}, 
{147, 148, 146}, {148, 147, 149}, {147, 148, 147}, {148, 147, 150}, {147, 148, 148}, {148, 148, 146}, {147, 148, 149}, {148, 148, 147}, {147, 148, 150}, {148, 148, 148}, {146, 149, 149}, {148, 148, 149}, {148, 148, 150}, {147, 149, 148}, {148, 148, 151}, {149, 148, 149}, 
{148, 149, 147}, {149, 148, 150}, {148, 149, 148}, {149, 148, 151}, {148, 149, 149}, {149, 149, 147}, {148, 149, 150}, {149, 149, 148}, {148, 149, 151}, {149, 149, 149}, {147, 150, 150}, {149, 149, 150}, {148, 150, 148}, {148, 150, 149}, {150, 149, 149}, {150, 149, 150}, 
{149, 150, 148}, {150, 149, 151}, {149, 150, 149}, {150, 149, 152}, {149, 150, 150}, {150, 150, 148}, {149, 150, 151}, {150, 150, 149}, {149, 150, 152}, {150, 150, 150}, {148, 151, 151}, {150, 150, 151}, {149, 151, 149}, {149, 151, 150}, {151, 150, 150}, {151, 150, 151}, 
{150, 151, 149}, {151, 150, 152}, {150, 151, 150}, {151, 150, 153}, {150, 151, 151}, {151, 151, 149}, {150, 151, 152}, {151, 151, 150}, {150, 151, 153}, {151, 151, 151}, {150, 152, 149}, {151, 151, 152}, {150, 152, 150}, {151, 151, 153}, {152, 151, 151}, {150, 152, 152}, 
{152, 151, 152}, {152, 151, 153}, {151, 152, 151}, {152, 151, 154}, {151, 152, 152}, {152, 152, 150}, {151, 152, 153}, {152, 152, 151}, {151, 152, 154}, {152, 152, 152}, {151, 153, 150}, {152, 152, 153}, {151, 153, 151}, {152, 152, 154}, {153, 152, 152}, {151, 153, 153}, 
{153, 152, 153}, {152, 153, 151}, {152, 153, 152}, {154, 152, 152}, {152, 153, 153}, {153, 153, 151}, {152, 153, 154}, {153, 153, 152}, {152, 153, 155}, {153, 153, 153}, {152, 154, 151}, {153, 153, 154}, {152, 154, 152}, {153, 153, 155}, {154, 153, 153}, {152, 154, 154}, 
{154, 153, 154}, {153, 154, 152}, {153, 154, 153}, {155, 153, 153}, {153, 154, 154}, {154, 154, 152}, {153, 154, 155}, {154, 154, 153}, {153, 154, 156}, {154, 154, 154}, {153, 155, 152}, {154, 154, 155}, {153, 155, 153}, {154, 154, 156}, {155, 154, 154}, {154, 154, 157}, 
{155, 154, 155}, {154, 155, 153}, {154, 155, 154}, {156, 154, 154}, {154, 155, 155}, {155, 155, 153}, {154, 155, 156}, {155, 155, 154}, {154, 155, 157}, {155, 155, 155}, {154, 156, 153}, {155, 155, 156}, {154, 156, 154}, {155, 155, 157}, {156, 155, 155}, {155, 155, 158}, 
{156, 155, 156}, {155, 156, 154}, {156, 155, 157}, {155, 156, 155}, {155, 156, 156}, {157, 155, 156}, {155, 156, 157}, {156, 156, 155}, {155, 156, 158}, {156, 156, 156}, {155, 157, 154}, {156, 156, 157}, {155, 157, 155}, {156, 156, 158}, {157, 156, 156}, {156, 156, 159}, 
{157, 156, 157}, {156, 157, 155}, {157, 156, 158}, {156, 157, 156}, {156, 157, 157}, {158, 156, 157}, {156, 157, 158}, {157, 157, 156}, {156, 157, 159}, {157, 157, 157}, {156, 158, 155}, {157, 157, 158}, {156, 158, 156}, {157, 157, 159}, {158, 157, 157}, {157, 157, 160}, 
{158, 157, 158}, {157, 158, 156}, {158, 157, 159}, {157, 158, 157}, {158, 157, 160}, {159, 157, 158}, {157, 158, 159}, {158, 158, 157}, {157, 158, 160}, {158, 158, 158}, {157, 159, 156}, {158, 158, 159}, {157, 159, 157}, {158, 158, 160}, {159, 158, 158}, {158, 158, 161}, 
{159, 158, 159}, {158, 159, 157}, {159, 158, 160}, {158, 159, 158}, {159, 158, 161}, {158, 159, 159}, {158, 159, 160}, {160, 158, 160}, {159, 159, 158}, {159, 159, 159}, {158, 160, 157}, {159, 159, 160}, {158, 160, 158}, {159, 159, 161}, {160, 159, 159}, {159, 159, 162}, 
{160, 159, 160}, {159, 160, 158}, {160, 159, 161}, {159, 160, 159}, {160, 159, 162}, {159, 160, 160}, {159, 160, 161}, {161, 159, 161}, {160, 160, 159}, {160, 160, 160}, {159, 161, 158}, {160, 160, 161}, {159, 161, 159}, {160, 160, 162}, {161, 160, 160}, {160, 160, 163}, 
{161, 160, 161}, {160, 161, 159}, {161, 160, 162}, {160, 161, 160}, {161, 160, 163}, {160, 161, 161}, {161, 161, 159}, {162, 160, 162}, {161, 161, 160}, {161, 161, 161}, {160, 162, 159}, {161, 161, 162}, {160, 162, 160}, {161, 161, 163}, {162, 161, 161}, {161, 161, 164}, 
{162, 161, 162}, {161, 162, 160}, {162, 161, 163}, {161, 162, 161}, {162, 161, 164}, {161, 162, 162}, {162, 162, 160}, {161, 162, 163}, {162, 162, 161}, {160, 163, 162}, {162, 162, 162}, {162, 162, 163}, {161, 163, 161}, {162, 162, 164}, {163, 162, 162}, {162, 162, 165}, 
{163, 162, 163}, {162, 163, 161}, {163, 162, 164}, {162, 163, 162}, {163, 162, 165}, {162, 163, 163}, {163, 163, 161}, {162, 163, 164}, {163, 163, 162}, {161, 164, 163}, {163, 163, 163}, {163, 163, 164}, {162, 164, 162}, {163, 163, 165}, {164, 163, 163}, {163, 163, 166}, 
{164, 163, 164}, {163, 164, 162}, {164, 163, 165}, {163, 164, 163}, {164, 163, 166}, {163, 164, 164}, {164, 164, 162}, {163, 164, 165}, {164, 164, 163}, {162, 165, 164}, {164, 164, 164}, {164, 164, 165}, {163, 165, 163}, {164, 164, 166}, {165, 164, 164}, {164, 164, 167}, 
{165, 164, 165}, {164, 165, 163}, {165, 164, 166}, {164, 165, 164}, {165, 164, 167}, {164, 165, 165}, {165, 165, 163}, {164, 165, 166}, {165, 165, 164}, {164, 165, 167}, {165, 165, 165}, {163, 166, 166}, {165, 165, 166}, {165, 165, 167}, {164, 166, 165}, {165, 165, 168}, 
{166, 165, 166}, {165, 166, 164}, {166, 165, 167}, {165, 166, 165}, {166, 165, 168}, {165, 166, 166}, {166, 166, 164}, {165, 166, 167}, {166, 166, 165}, {165, 166, 168}, {166, 166, 166}, {164, 167, 167}, {166, 166, 167}, {165, 167, 165}, {165, 167, 166}, {167, 166, 166}, 
{167, 166, 167}, {166, 167, 165}, {167, 166, 168}, {166, 167, 166}, {167, 166, 169}, {166, 167, 167}, {167, 167, 165}, {166, 167, 168}, {167, 167, 166}, {166, 167, 169}, {167, 167, 167}, {165, 168, 168}, {167, 167, 168}, {166, 168, 166}, {166, 168, 167}, {168, 167, 167}, 
{168, 167, 168}, {167, 168, 166}, {168, 167, 169}, {167, 168, 167}, {168, 167, 170}, {167, 168, 168}, {168, 168, 166}, {167, 168, 169}, {168, 168, 167}, {167, 168, 170}, {168, 168, 168}, {167, 169, 166}, {168, 168, 169}, {167, 169, 167}, {168, 168, 170}, {169, 168, 168}, 
{167, 169, 169}, {169, 168, 169}, {169, 168, 170}, {168, 169, 168}, {169, 168, 171}, {168, 169, 169}, {169, 169, 167}, {168, 169, 170}, {169, 169, 168}, {168, 169, 171}, {169, 169, 169}, {168, 170, 167}, {169, 169, 170}, {168, 170, 168}, {169, 169, 171}, {170, 169, 169}, 
{168, 170, 170}, {170, 169, 170}, {169, 170, 168}, {169, 170, 169}, {171, 169, 169}, {169, 170, 170}, {170, 170, 168}, {169, 170, 171}, {170, 170, 169}, {169, 170, 172}, {170, 170, 170}, {169, 171, 168}, {170, 170, 171}, {169, 171, 169}, {170, 170, 172}, {171, 170, 170}, 
{169, 171, 171}, {171, 170, 171}, {170, 171, 169}, {170, 171, 170}, {172, 170, 170}, {170, 171, 171}, {171, 171, 169}, {170, 171, 172}, {171, 171, 170}, {170, 171, 173}, {171, 171, 171}, {170, 172, 169}, {171, 171, 172}, {170, 172, 170}, {171, 171, 173}, {172, 171, 171}, 
{171, 171, 174}, {172, 171, 172}, {171, 172, 170}, {171, 172, 171}, {173, 171, 171}, {171, 172, 172}, {172, 172, 170}, {171, 172, 173}, {172, 172, 171}, {171, 172, 174}, {172, 172, 172}, {171, 173, 170}, {172, 172, 173}, {171, 173, 171}, {172, 172, 174}, {173, 172, 172}, 
{172, 172, 175}, {173, 172, 173}, {172, 173, 171}, {173, 172, 174}, {172, 173, 172}, {172, 173, 173}, {174, 172, 173}, {172, 173, 174}, {173, 173, 172}, {172, 173, 175}, {173, 173, 173}, {172, 174, 171}, {173, 173, 174}, {172, 174, 172}, {173, 173, 175}, {174, 173, 173}, 
{173, 173, 176}, {174, 173, 174}, {173, 174, 172}, {174, 173, 175}, {173, 174, 173}, {173, 174, 174}, {175, 173, 174}, {173, 174, 175}, {174, 174, 173}, {173, 174, 176}, {174, 174, 174}, {173, 175, 172}, {174, 174, 175}, {173, 175, 173}, {174, 174, 176}, {175, 174, 174}, 
{174, 174, 177}, {175, 174, 175}, {174, 175, 173}, {175, 174, 176}, {174, 175, 174}, {175, 174, 177}, {176, 174, 175}, {174, 175, 176}, {175, 175, 174}, {174, 175, 177}, {175, 175, 175}, {174, 176, 173}, {175, 175, 176}, {174, 176, 174}, {175, 175, 177}, {176, 175, 175}, 
{175, 175, 178}, {176, 175, 176}, {175, 176, 174}, {176, 175, 177}, {175, 176, 175}, {176, 175, 178}, {175, 176, 176}, {175, 176, 177}, {177, 175, 177}, {176, 176, 175}, {176, 176, 176}, {175, 177, 174}, {176, 176, 177}, {175, 177, 175}, {176, 176, 178}, {177, 176, 176}, 
{176, 176, 179}, {177, 176, 177}, {176, 177, 175}, {177, 176, 178}, {176, 177, 176}, {177, 176, 179}, {176, 177, 177}, {176, 177, 178}, {178, 176, 178}, {177, 177, 176}, {177, 177, 177}, {176, 178, 175}, {177, 177, 178}, {176, 178, 176}, {177, 177, 179}, {178, 177, 177}, 
{177, 177, 180}, {178, 177, 178}, {177, 178, 176}, {178, 177, 179}, {177, 178, 177}, {178, 177, 180}, {177, 178, 178}, {178, 178, 176}, {179, 177, 179}, {178, 178, 177}, {178, 178, 178}, {177, 179, 176}, {178, 178, 179}, {177, 179, 177}, {178, 178, 180}, {179, 178, 178}, 
{178, 178, 181}, {179, 178, 179}, {178, 179, 177}, {179, 178, 180}, {178, 179, 178}, {179, 178, 181}, {178, 179, 179}, {179, 179, 177}, {178, 179, 180}, {179, 179, 178}, {177, 180, 179}, {179, 179, 179}, {179, 179, 180}, {178, 180, 178}, {179, 179, 181}, {180, 179, 179}, 
{179, 179, 182}, {180, 179, 180}, {179, 180, 178}, {180, 179, 181}, {179, 180, 179}, {180, 179, 182}, {179, 180, 180}, {180, 180, 178}, {179, 180, 181}, {180, 180, 179}, {178, 181, 180}, {180, 180, 180}, {180, 180, 181}, {179, 181, 179}, {180, 180, 182}, {181, 180, 180}, 
{180, 180, 183}, {181, 180, 181}, {180, 181, 179}, {181, 180, 182}, {180, 181, 180}, {181, 180, 183}, {180, 181, 181}, {181, 181, 179}, {180, 181, 182}, {181, 181, 180}, {179, 182, 181}, {181, 181, 181}, {181, 181, 182}, {180, 182, 180}, {181, 181, 183}, {182, 181, 181}, 
{181, 181, 184}, {182, 181, 182}, {181, 182, 180}, {182, 181, 183}, {181, 182, 181}, {182, 181, 184}, {181, 182, 182}, {182, 182, 180}, {181, 182, 183}, {182, 182, 181}, {181, 182, 184}, {182, 182, 182}, {180, 183, 183}, {182, 182, 183}, {182, 182, 184}, {181, 183, 182}, 
{182, 182, 185}, {183, 182, 183}, {182, 183, 181}, {183, 182, 184}, {182, 183, 182}, {183, 182, 185}, {182, 183, 183}, {183, 183, 181}, {182, 183, 184}, {183, 183, 182}, {182, 183, 185}, {183, 183, 183}, {181, 184, 184}, {183, 183, 184}, {182, 184, 182}, {182, 184, 183}, 
{184, 183, 183}, {184, 183, 184}, {183, 184, 182}, {184, 183, 185}, {183, 184, 183}, {184, 183, 186}, {183, 184, 184}, {184, 184, 182}, {183, 184, 185}, {184, 184, 183}, {183, 184, 186}, {184, 184, 184}, {182, 185, 185}, {184, 184, 185}, {183, 185, 183}, {183, 185, 184}, 
{185, 184, 184}, {185, 184, 185}, {184, 185, 183}, {185, 184, 186}, {184, 185, 184}, {185, 184, 187}, {184, 185, 185}, {185, 185, 183}, {184, 185, 186}, {185, 185, 184}, {184, 185, 187}, {185, 185, 185}, {184, 186, 183}, {185, 185, 186}, {184, 186, 184}, {185, 185, 187}, 
{186, 185, 185}, {184, 186, 186}, {186, 185, 186}, {186, 185, 187}, {185, 186, 185}, {186, 185, 188}, {185, 186, 186}, {186, 186, 184}, {185, 186, 187}, {186, 186, 185}, {185, 186, 188}, {186, 186, 186}, {185, 187, 184}, {186, 186, 187}, {185, 187, 185}, {186, 186, 188}, 
{187, 186, 186}, {185, 187, 187}, {187, 186, 187}, {186, 187, 185}, {186, 187, 186}, {188, 186, 186}, {186, 187, 187}, {187, 187, 185}, {186, 187, 188}, {187, 187, 186}, {186, 187, 189}, {187, 187, 187}, {186, 188, 185}, {187, 187, 188}, {186, 188, 186}, {187, 187, 189}, 
{188, 187, 187}, {186, 188, 188}, {188, 187, 188}, {187, 188, 186}, {187, 188, 187}, {189, 187, 187}, {187, 188, 188}, {188, 188, 186}, {187, 188, 189}, {188, 188, 187}, {187, 188, 190}, {188, 188, 188}, {187, 189, 186}, {188, 188, 189}, {187, 189, 187}, {188, 188, 190}, 
{189, 188, 188}, {188, 188, 191}, {189, 188, 189}, {188, 189, 187}, {188, 189, 188}, {190, 188, 188}, {188, 189, 189}, {189, 189, 187}, {188, 189, 190}, {189, 189, 188}, {188, 189, 191}, {189, 189, 189}, {188, 190, 187}, {189, 189, 190}, {188, 190, 188}, {189, 189, 191}, 
{190, 189, 189}, {189, 189, 192}, {190, 189, 190}, {189, 190, 188}, {190, 189, 191}, {189, 190, 189}, {189, 190, 190}, {191, 189, 190}, {189, 190, 191}, {190, 190, 189}, {189, 190, 192}, {190, 190, 190}, {189, 191, 188}, {190, 190, 191}, {189, 191, 189}, {190, 190, 192}, 
{191, 190, 190}, {190, 190, 193}, {191, 190, 191}, {190, 191, 189}, {191, 190, 192}, {190, 191, 190}, {190, 191, 191}, {192, 190, 191}, {190, 191, 192}, {191, 191, 190}, {190, 191, 193}, {191, 191, 191}, {190, 192, 189}, {191, 191, 192}, {190, 192, 190}, {191, 191, 193}, 
{192, 191, 191}, {191, 191, 194}, {192, 191, 192}, {191, 192, 190}, {192, 191, 193}, {191, 192, 191}, {192, 191, 194}, {193, 191, 192}, {191, 192, 193}, {192, 192, 191}, {191, 192, 194}, {192, 192, 192}, {191, 193, 190}, {192, 192, 193}, {191, 193, 191}, {192, 192, 194}, 
{193, 192, 192}, {192, 192, 195}, {193, 192, 193}, {192, 193, 191}, {193, 192, 194}, {192, 193, 192}, {193, 192, 195}, {192, 193, 193}, {192, 193, 194}, {194, 192, 194}, {193, 193, 192}, {193, 193, 193}, {192, 194, 191}, {193, 193, 194}, {192, 194, 192}, {193, 193, 195}, 
{194, 193, 193}, {193, 193, 196}, {194, 193, 194}, {193, 194, 192}, {194, 193, 195}, {193, 194, 193}, {194, 193, 196}, {193, 194, 194}, {193, 194, 195}, {195, 193, 195}, {194, 194, 193}, {194, 194, 194}, {193, 195, 192}, {194, 194, 195}, {193, 195, 193}, {194, 194, 196}, 
{195, 194, 194}, {194, 194, 197}, {195, 194, 195}, {194, 195, 193}, {195, 194, 196}, {194, 195, 194}, {195, 194, 197}, {194, 195, 195}, {195, 195, 193}, {196, 194, 196}, {195, 195, 194}, {195, 195, 195}, {194, 196, 193}, {195, 195, 196}, {194, 196, 194}, {195, 195, 197}, 
{196, 195, 195}, {195, 195, 198}, {196, 195, 196}, {195, 196, 194}, {196, 195, 197}, {195, 196, 195}, {196, 195, 198}, {195, 196, 196}, {196, 196, 194}, {195, 196, 197}, {196, 196, 195}, {194, 197, 196}, {196, 196, 196}, {196, 196, 197}, {195, 197, 195}, {196, 196, 198}, 
{197, 196, 196}, {196, 196, 199}, {197, 196, 197}, {196, 197, 195}, {197, 196, 198}, {196, 197, 196}, {197, 196, 199}, {196, 197, 197}, {197, 197, 195}, {196, 197, 198}, {197, 197, 196}, {195, 198, 197}, {197, 197, 197}, {197, 197, 198}, {196, 198, 196}, {197, 197, 199}, 
{198, 197, 197}, {197, 197, 200}, {198, 197, 198}, {197, 198, 196}, {198, 197, 199}, {197, 198, 197}, {198, 197, 200}, {197, 198, 198}, {198, 198, 196}, {197, 198, 199}, {198, 198, 197}, {196, 199, 198}, {198, 198, 198}, {198, 198, 199}, {197, 199, 197}, {198, 198, 200}, 
{199, 198, 198}, {198, 198, 201}, {199, 198, 199}, {198, 199, 197}, {199, 198, 200}, {198, 199, 198}, {199, 198, 201}, {198, 199, 199}, {199, 199, 197}, {198, 199, 200}, {199, 199, 198}, {198, 199, 201}, {199, 199, 199}, {197, 200, 200}, {199, 199, 200}, {199, 199, 201}, 
{198, 200, 199}, {199, 199, 202}, {200, 199, 200}, {199, 200, 198}, {200, 199, 201}, {199, 200, 199}, {200, 199, 202}, {199, 200, 200}, {200, 200, 198}, {199, 200, 201}, {200, 200, 199}, {199, 200, 202}, {200, 200, 200}, {198, 201, 201}, {200, 200, 201}, {199, 201, 199}, 
{199, 201, 200}, {201, 200, 200}, {201, 200, 201}, {200, 201, 199}, {201, 200, 202}, {200, 201, 200}, {201, 200, 203}, {200, 201, 201}, {201, 201, 199}, {200, 201, 202}, {201, 201, 200}, {200, 201, 203}, {201, 201, 201}, {199, 202, 202}, {201, 201, 202}, {200, 202, 200}, 
{200, 202, 201}, {202, 201, 201}, {202, 201, 202}, {201, 202, 200}, {202, 201, 203}, {201, 202, 201}, {202, 201, 204}, {201, 202, 202}, {202, 202, 200}, {201, 202, 203}, {202, 202, 201}, {201, 202, 204}, {202, 202, 202}, {201, 203, 200}, {202, 202, 203}, {201, 203, 201}, 
{202, 202, 204}, {203, 202, 202}, {201, 203, 203}, {203, 202, 203}, {203, 202, 204}, {202, 203, 202}, {203, 202, 205}, {202, 203, 203}, {203, 203, 201}, {202, 203, 204}, {203, 203, 202}, {202, 203, 205}, {203, 203, 203}, {202, 204, 201}, {203, 203, 204}, {202, 204, 202}, 
{203, 203, 205}, {204, 203, 203}, {202, 204, 204}, {204, 203, 204}, {203, 204, 202}, {203, 204, 203}, {205, 203, 203}, {203, 204, 204}, {204, 204, 202}, {203, 204, 205}, {204, 204, 203}, {203, 204, 206}, {204, 204, 204}, {203, 205, 202}, {204, 204, 205}, {203, 205, 203}, 
{204, 204, 206}, {205, 204, 204}, {203, 205, 205}, {205, 204, 205}, {204, 205, 203}, {204, 205, 204}, {206, 204, 204}, {204, 205, 205}, {205, 205, 203}, {204, 205, 206}, {205, 205, 204}, {204, 205, 207}, {205, 205, 205}, {204, 206, 203}, {205, 205, 206}, {204, 206, 204}, 
{205, 205, 207}, {206, 205, 205}, {205, 205, 208}, {206, 205, 206}, {205, 206, 204}, {205, 206, 205}, {207, 205, 205}, {205, 206, 206}, {206, 206, 204}, {205, 206, 207}, {206, 206, 205}, {205, 206, 208}, {206, 206, 206}, {205, 207, 204}, {206, 206, 207}, {205, 207, 205}, 
{206, 206, 208}, {207, 206, 206}, {206, 206, 209}, {207, 206, 207}, {206, 207, 205}, {207, 206, 208}, {206, 207, 206}, {206, 207, 207}, {208, 206, 207}, {206, 207, 208}, {207, 207, 206}, {206, 207, 209}, {207, 207, 207}, {206, 208, 205}, {207, 207, 208}, {206, 208, 206}, 
{207, 207, 209}, {208, 207, 207}, {207, 207, 210}, {208, 207, 208}, {207, 208, 206}, {208, 207, 209}, {207, 208, 207}, {207, 208, 208}, {209, 207, 208}, {207, 208, 209}, {208, 208, 207}, {207, 208, 210}, {208, 208, 208}, {207, 209, 206}, {208, 208, 209}, {207, 209, 207}, 
{208, 208, 210}, {209, 208, 208}, {208, 208, 211}, {209, 208, 209}, {208, 209, 207}, {209, 208, 210}, {208, 209, 208}, {209, 208, 211}, {210, 208, 209}, {208, 209, 210}, {209, 209, 208}, {208, 209, 211}, {209, 209, 209}, {208, 210, 207}, {209, 209, 210}, {208, 210, 208}, 
{209, 209, 211}, {210, 209, 209}, {209, 209, 212}, {210, 209, 210}, {209, 210, 208}, {210, 209, 211}, {209, 210, 209}, {210, 209, 212}, {209, 210, 210}, {209, 210, 211}, {211, 209, 211}, {210, 210, 209}, {210, 210, 210}, {209, 211, 208}, {210, 210, 211}, {209, 211, 209}, 
{210, 210, 212}, {211, 210, 210}, {210, 210, 213}, {211, 210, 211}, {210, 211, 209}, {211, 210, 212}, {210, 211, 210}, {211, 210, 213}, {210, 211, 211}, {210, 211, 212}, {212, 210, 212}, {211, 211, 210}, {211, 211, 211}, {210, 212, 209}, {211, 211, 212}, {210, 212, 210}, 
{211, 211, 213}, {212, 211, 211}, {211, 211, 214}, {212, 211, 212}, {211, 212, 210}, {212, 211, 213}, {211, 212, 211}, {212, 211, 214}, {211, 212, 212}, {212, 212, 210}, {213, 211, 213}, {212, 212, 211}, {212, 212, 212}, {211, 213, 210}, {212, 212, 213}, {211, 213, 211}, 
{212, 212, 214}, {213, 212, 212}, {212, 212, 215}, {213, 212, 213}, {212, 213, 211}, {213, 212, 214}, {212, 213, 212}, {213, 212, 215}, {212, 213, 213}, {213, 213, 211}, {212, 213, 214}, {213, 213, 212}, {211, 214, 213}, {213, 213, 213}, {213, 213, 214}, {212, 214, 212}, 
{213, 213, 215}, {214, 213, 213}, {213, 213, 216}, {214, 213, 214}, {213, 214, 212}, {214, 213, 215}, {213, 214, 213}, {214, 213, 216}, {213, 214, 214}, {214, 214, 212}, {213, 214, 215}, {214, 214, 213}, {212, 215, 214}, {214, 214, 214}, {214, 214, 215}, {213, 215, 213}, 
{214, 214, 216}, {215, 214, 214}, {214, 214, 217}, {215, 214, 215}, {214, 215, 213}, {215, 214, 216}, {214, 215, 214}, {215, 214, 217}, {214, 215, 215}, {215, 215, 213}, {214, 215, 216}, {215, 215, 214}, {213, 216, 215}, {215, 215, 215}, {215, 215, 216}, {214, 216, 214}, 
{215, 215, 217}, {216, 215, 215}, {215, 215, 218}, {216, 215, 216}, {215, 216, 214}, {216, 215, 217}, {215, 216, 215}, {216, 215, 218}, {215, 216, 216}, {216, 216, 214}, {215, 216, 217}, {216, 216, 215}, {215, 216, 218}, {216, 216, 216}, {214, 217, 217}, {216, 216, 217}, 
{216, 216, 218}, {215, 217, 216}, {216, 216, 219}, {217, 216, 217}, {216, 217, 215}, {217, 216, 218}, {216, 217, 216}, {217, 216, 219}, {216, 217, 217}, {217, 217, 215}, {216, 217, 218}, {217, 217, 216}, {216, 217, 219}, {217, 217, 217}, {215, 218, 218}, {217, 217, 218}, 
{216, 218, 216}, {216, 218, 217}, {218, 217, 217}, {218, 217, 218}, {217, 218, 216}, {218, 217, 219}, {217, 218, 217}, {218, 217, 220}, {217, 218, 218}, {218, 218, 216}, {217, 218, 219}, {218, 218, 217}, {217, 218, 220}, {218, 218, 218}, {216, 219, 219}, {218, 218, 219}, 
{217, 219, 217}, {217, 219, 218}, {219, 218, 218}, {219, 218, 219}, {218, 219, 217}, {219, 218, 220}, {218, 219, 218}, {219, 218, 221}, {218, 219, 219}, {219, 219, 217}, {218, 219, 220}, {219, 219, 218}, {218, 219, 221}, {219, 219, 219}, {218, 220, 217}, {219, 219, 220}, 
{218, 220, 218}, {219, 219, 221}, {220, 219, 219}, {218, 220, 220}, {220, 219, 220}, {220, 219, 221}, {219, 220, 219}, {220, 219, 222}, {219, 220, 220}, {220, 220, 218}, {219, 220, 221}, {220, 220, 219}, {219, 220, 222}, {220, 220, 220}, {219, 221, 218}, {220, 220, 221}, 
{219, 221, 219}, {220, 220, 222}, {221, 220, 220}, {219, 221, 221}, {221, 220, 221}, {220, 221, 219}, {220, 221, 220}, {222, 220, 220}, {220, 221, 221}, {221, 221, 219}, {220, 221, 222}, {221, 221, 220}, {220, 221, 223}, {221, 221, 221}, {220, 222, 219}, {221, 221, 222}, 
{220, 222, 220}, {221, 221, 223}, {222, 221, 221}, {220, 222, 222}, {222, 221, 222}, {221, 222, 220}, {221, 222, 221}, {223, 221, 221}, {221, 222, 222}, {222, 222, 220}, {221, 222, 223}, {222, 222, 221}, {221, 222, 224}, {222, 222, 222}, {221, 223, 220}, {222, 222, 223}, 
{221, 223, 221}, {222, 222, 224}, {223, 222, 222}, {222, 222, 225}, {223, 222, 223}, {222, 223, 221}, {222, 223, 222}, {224, 222, 222}, {222, 223, 223}, {223, 223, 221}, {222, 223, 224}, {223, 223, 222}, {222, 223, 225}, {223, 223, 223}, {222, 224, 221}, {223, 223, 224}, 
{222, 224, 222}, {223, 223, 225}, {224, 223, 223}, {223, 223, 226}, {224, 223, 224}, {223, 224, 222}, {224, 223, 225}, {223, 224, 223}, {223, 224, 224}, {225, 223, 224}, {223, 224, 225}, {224, 224, 223}, {223, 224, 226}, {224, 224, 224}, {223, 225, 222}, {224, 224, 225}, 
{223, 225, 223}, {224, 224, 226}, {225, 224, 224}, {224, 224, 227}, {225, 224, 225}, {224, 225, 223}, {225, 224, 226}, {224, 225, 224}, {224, 225, 225}, {226, 224, 225}, {224, 225, 226}, {225, 225, 224}, {224, 225, 227}, {225, 225, 225}, {224, 226, 223}, {225, 225, 226}, 
{224, 226, 224}, {225, 225, 227}, {226, 225, 225}, {225, 225, 228}, {226, 225, 226}, {225, 226, 224}, {226, 225, 227}, {225, 226, 225}, {226, 225, 228}, {227, 225, 226}, {225, 226, 227}, {226, 226, 225}, {225, 226, 228}, {226, 226, 226}, {225, 227, 224}, {226, 226, 227}, 
{225, 227, 225}, {226, 226, 228}, {227, 226, 226}, {226, 226, 229}, {227, 226, 227}, {226, 227, 225}, {227, 226, 228}, {226, 227, 226}, {227, 226, 229}, {226, 227, 227}, {226, 227, 228}, {228, 226, 228}, {227, 227, 226}, {227, 227, 227}, {226, 228, 225}, {227, 227, 228}, 
{226, 228, 226}, {227, 227, 229}, {228, 227, 227}, {227, 227, 230}, {228, 227, 228}, {227, 228, 226}, {228, 227, 229}, {227, 228, 227}, {228, 227, 230}, {227, 228, 228}, {227, 228, 229}, {229, 227, 229}, {228, 228, 227}, {228, 228, 228}, {227, 229, 226}, {228, 228, 229}, 
{227, 229, 227}, {228, 228, 230}, {229, 228, 228}, {228, 228, 231}, {229, 228, 229}, {228, 229, 227}, {229, 228, 230}, {228, 229, 228}, {229, 228, 231}, {228, 229, 229}, {229, 229, 227}, {230, 228, 230}, {229, 229, 228}, {229, 229, 229}, {228, 230, 227}, {229, 229, 230}, 
{228, 230, 228}, {229, 229, 231}, {230, 229, 229}, {229, 229, 232}, {230, 229, 230}, {229, 230, 228}, {230, 229, 231}, {229, 230, 229}, {230, 229, 232}, {229, 230, 230}, {230, 230, 228}, {229, 230, 231}, {230, 230, 229}, {228, 231, 230}, {230, 230, 230}, {230, 230, 231}, 
{229, 231, 229}, {230, 230, 232}, {231, 230, 230}, {230, 230, 233}, {231, 230, 231}, {230, 231, 229}, {231, 230, 232}, {230, 231, 230}, {231, 230, 233}, {230, 231, 231}, {231, 231, 229}, {230, 231, 232}, {231, 231, 230}, {229, 232, 231}, {231, 231, 231}, {231, 231, 232}, 
{230, 232, 230}, {231, 231, 233}, {232, 231, 231}, {231, 231, 234}, {232, 231, 232}, {231, 232, 230}, {232, 231, 233}, {231, 232, 231}, {232, 231, 234}, {231, 232, 232}, {232, 232, 230}, {231, 232, 233}, {232, 232, 231}, {230, 233, 232}, {232, 232, 232}, {232, 232, 233}, 
{231, 233, 231}, {232, 232, 234}, {233, 232, 232}, {232, 232, 235}, {233, 232, 233}, {232, 233, 231}, {233, 232, 234}, {232, 233, 232}, {233, 232, 235}, {232, 233, 233}, {233, 233, 231}, {232, 233, 234}, {233, 233, 232}, {232, 233, 235}, {233, 233, 233}, {231, 234, 234}, 
{233, 233, 234}, {233, 233, 235}, {232, 234, 233}, {233, 233, 236}, {234, 233, 234}, {233, 234, 232}, {234, 233, 235}, {233, 234, 233}, {234, 233, 236}, {233, 234, 234}, {234, 234, 232}, {233, 234, 235}, {234, 234, 233}, {233, 234, 236}, {234, 234, 234}, {232, 235, 235}, 
{234, 234, 235}, {233, 235, 233}, {233, 235, 234}, {235, 234, 234}, {235, 234, 235}, {234, 235, 233}, {235, 234, 236}, {234, 235, 234}, {235, 234, 237}, {234, 235, 235}, {235, 235, 233}, {234, 235, 236}, {235, 235, 234}, {234, 235, 237}, {235, 235, 235}, {233, 236, 236}, 
{235, 235, 236}, {234, 236, 234}, {234, 236, 235}, {236, 235, 235}, {236, 235, 236}, {235, 236, 234}, {236, 235, 237}, {235, 236, 235}, {236, 235, 238}, {235, 236, 236}, {236, 236, 234}, {235, 236, 237}, {236, 236, 235}, {235, 236, 238}, {236, 236, 236}, {235, 237, 234}, 
{236, 236, 237}, {235, 237, 235}, {236, 236, 238}, {237, 236, 236}, {235, 237, 237}, {237, 236, 237}, {237, 236, 238}, {236, 237, 236}, {237, 236, 239}, {236, 237, 237}, {237, 237, 235}, {236, 237, 238}, {237, 237, 236}, {236, 237, 239}, {237, 237, 237}, {236, 238, 235}, 
{237, 237, 238}, {236, 238, 236}, {237, 237, 239}, {238, 237, 237}, {236, 238, 238}, {238, 237, 238}, {237, 238, 236}, {237, 238, 237}, {239, 237, 237}, {237, 238, 238}, {238, 238, 236}, {237, 238, 239}, {238, 238, 237}, {237, 238, 240}, {238, 238, 238}, {237, 239, 236}, 
{238, 238, 239}, {237, 239, 237}, {238, 238, 240}, {239, 238, 238}, {237, 239, 239}, {239, 238, 239}, {238, 239, 237}, {238, 239, 238}, {240, 238, 238}, {238, 239, 239}, {239, 239, 237}, {238, 239, 240}, {239, 239, 238}, {238, 239, 241}, {239, 239, 239}, {238, 240, 237}, 
{239, 239, 240}, {238, 240, 238}, {239, 239, 241}, {240, 239, 239}, {239, 239, 242}, {240, 239, 240}, {239, 240, 238}, {239, 240, 239}, {241, 239, 239}, {239, 240, 240}, {240, 240, 238}, {239, 240, 241}, {240, 240, 239}, {239, 240, 242}, {240, 240, 240}, {239, 241, 238}, 
{240, 240, 241}, {239, 241, 239}, {240, 240, 242}, {241, 240, 240}, {240, 240, 243}, {241, 240, 241}, {240, 241, 239}, {241, 240, 242}, {240, 241, 240}, {240, 241, 241}, {242, 240, 241}, {240, 241, 242}, {241, 241, 240}, {240, 241, 243}, {241, 241, 241}, {240, 242, 239}, 
{241, 241, 242}, {240, 242, 240}, {241, 241, 243}, {242, 241, 241}, {241, 241, 244}, {242, 241, 242}, {241, 242, 240}, {242, 241, 243}, {241, 242, 241}, {241, 242, 242}, {243, 241, 242}, {241, 242, 243}, {242, 242, 241}, {241, 242, 244}, {242, 242, 242}, {241, 243, 240}, 
{242, 242, 243}, {241, 243, 241}, {242, 242, 244}, {243, 242, 242}, {242, 242, 245}, {243, 242, 243}, {242, 243, 241}, {243, 242, 244}, {242, 243, 242}, {243, 242, 245}, {244, 242, 243}, {242, 243, 244}, {243, 243, 242}, {242, 243, 245}, {243, 243, 243}, {242, 244, 241}, 
{243, 243, 244}, {242, 244, 242}, {243, 243, 245}, {244, 243, 243}, {243, 243, 246}, {244, 243, 244}, {243, 244, 242}, {244, 243, 245}, {243, 244, 243}, {244, 243, 246}, {243, 244, 244}, {243, 244, 245}, {245, 243, 245}, {244, 244, 243}, {244, 244, 244}, {243, 245, 242}, 
{244, 244, 245}, {243, 245, 243}, {244, 244, 246}, {245, 244, 244}, {244, 244, 247}, {245, 244, 245}, {244, 245, 243}, {245, 244, 246}, {244, 245, 244}, {245, 244, 247}, {244, 245, 245}, {244, 245, 246}, {246, 244, 246}, {245, 245, 244}, {245, 245, 245}, {244, 246, 243}, 
{245, 245, 246}, {244, 246, 244}, {245, 245, 247}, {246, 245, 245}, {245, 245, 248}, {246, 245, 246}, {245, 246, 244}, {246, 245, 247}, {245, 246, 245}, {246, 245, 248}, {245, 246, 246}, {246, 246, 244}, {247, 245, 247}, {246, 246, 245}, {246, 246, 246}, {245, 247, 244}, 
{246, 246, 247}, {245, 247, 245}, {246, 246, 248}, {247, 246, 246}, {246, 246, 249}, {247, 246, 247}, {246, 247, 245}, {247, 246, 248}, {246, 247, 246}, {247, 246, 249}, {246, 247, 247}, {247, 247, 245}, {246, 247, 248}, {247, 247, 246}, {245, 248, 247}, {247, 247, 247}, 
{247, 247, 248}, {246, 248, 246}, {247, 247, 249}, {248, 247, 247}, {247, 247, 250}, {248, 247, 248}, {247, 248, 246}, {248, 247, 249}, {247, 248, 247}, {248, 247, 250}, {247, 248, 248}, {248, 248, 246}, {247, 248, 249}, {248, 248, 247}, {246, 249, 248}, {248, 248, 248}, 
{248, 248, 249}, {247, 249, 247}, {248, 248, 250}, {249, 248, 248}, {248, 248, 251}, {249, 248, 249}, {248, 249, 247}, {249, 248, 250}, {248, 249, 248}, {249, 248, 251}, {248, 249, 249}, {249, 249, 247}, {248, 249, 250}, {249, 249, 248}, {247, 250, 249}, {249, 249, 249}, 
{249, 249, 250}, {248, 250, 248}, {249, 249, 251}, {250, 249, 249}, {249, 249, 252}, {250, 249, 250}, {249, 250, 248}, {250, 249, 251}, {249, 250, 249}, {250, 249, 252}, {249, 250, 250}, {250, 250, 248}, {249, 250, 251}, {250, 250, 249}, {249, 250, 252}, {250, 250, 250}, 
{248, 251, 251}, {250, 250, 251}, {250, 250, 252}, {249, 251, 250}, {250, 250, 253}, {251, 250, 251}, {250, 251, 249}, {251, 250, 252}, {250, 251, 250}, {251, 250, 253}, {250, 251, 251}, {251, 251, 249}, {250, 251, 252}, {251, 251, 250}, {250, 251, 253}, {251, 251, 251}, 
{249, 252, 252}, {251, 251, 252}, {250, 252, 250}, {250, 252, 251}, {252, 251, 251}, {252, 251, 252}, {251, 252, 250}, {252, 251, 253}, {251, 252, 251}, {252, 251, 254}, {251, 252, 252}, {252, 252, 250}, {251, 252, 253}, {252, 252, 251}, {251, 252, 254}, {252, 252, 252}, 
{250, 253, 253}, {252, 252, 253}, {251, 253, 251}, {251, 253, 252}, {253, 252, 252}, {253, 252, 253}, {252, 253, 251}, {253, 252, 254}, {252, 253, 252}, {253, 252, 255}, {252, 253, 253}, {253, 253, 251}, {252, 253, 254}, {253, 253, 252}, {252, 253, 255}, {253, 253, 253}, 
{252, 254, 251}, {253, 253, 254}, {252, 254, 252}, {253, 253, 255}, {254, 253, 253}, {252, 254, 254}, {254, 253, 254}, {254, 253, 255}, {253, 254, 253}, {254, 254, 251}, {253, 254, 254}, {254, 254, 252}, {253, 254, 255}, {254, 254, 253}, {253, 255, 251}, {254, 254, 254}, 
{253, 255, 252}, {254, 254, 255}, {253, 255, 253}, {254, 255, 251}, {255, 254, 254}, {253, 255, 255}, {255, 254, 255}, {254, 255, 253}, {254, 255, 254}, {255, 255, 252}, {254, 255, 255}, {255, 255, 253}, {255, 255, 253}, {255, 255, 254}, {255, 255, 254}, {255, 255, 255}, 
};


unsigned char *CGraphicsAdapter::GenerateRGBLookupTable()
{
    unsigned char		*LookupTable = NULL;
	int				GrayIndex;

	LookupTable = (unsigned char*)calloc( 1, 4096 * 3 );
   if ( LookupTable != 0 )
        {
  		// Load the lookup table.  This generates the "best fit" 12-bit grayscale to RGB conversion table.
		// (This amounts to a unique encoding of the grayscale values, assuming the backend
		// decoder uses the same table.)
		for ( GrayIndex = 0; GrayIndex < 4096; GrayIndex++ )
			{
			LookupTable[ GrayIndex * 3 ] = GrayscaleLookupTable12bit[ GrayIndex ][ 0 ];
			LookupTable[ GrayIndex * 3 + 1 ] = GrayscaleLookupTable12bit[ GrayIndex ][ 1 ];
			LookupTable[ GrayIndex * 3 + 2 ] = GrayscaleLookupTable12bit[ GrayIndex ][ 2 ];
			}
        }

    return LookupTable;
}

// Generate and load a pair of lookup tables for converting from 12-bit grayscale
// to RGBA.
void CGraphicsAdapter::Load10BitGrayscaleShaderLookupTablesAsTextures()
{
	unsigned char	*pRGBLookupTable;
	GLuint			LookupTableSize;
	
	LookupTableSize = 4096;
	// Create the lookup table for 12-bit grayscale pixel-packing conversions to RGB.
	pRGBLookupTable = GenerateRGBLookupTable();
	if ( pRGBLookupTable != 0 )
		{
		// Designate the texture unit to be affected by subsequent texture state operations.
		glActiveTexture( TEXTURE_UNIT_GRAYSCALE_LOOKUP );
		// Generate a texture "name" for the grayscale lookup table and save it as m_glLUT12BitTextureId.
		glGenTextures( 1, &m_glLUT12BitTextureId );
		// Bind the image texture name to the 1-dimensional texture target.
		glBindTexture( GL_TEXTURE_1D, m_glLUT12BitTextureId );
		// Specify that the source image has 4-byte (32-bit) row alignment.
		glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
		CheckOpenGLResultAt( __FILE__, __LINE__	);
		// Set the texture environment mode for texture replacement.
//		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
		CheckOpenGLResultAt( __FILE__, __LINE__	);
		// Set the texture wrapping for the S and T coordinates.
		glTexParameterf( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
		glTexParameterf( GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
		CheckOpenGLResultAt( __FILE__, __LINE__	);
		// Disable interpolation for the lookup table.
		glTexParameterf( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameterf( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		CheckOpenGLResultAt( __FILE__, __LINE__	);
		// Load the lookup table contents into the one-dimensional "texture".
		glTexImage1D( GL_TEXTURE_1D, 0, GL_RGB8, LookupTableSize, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)pRGBLookupTable );
		
		CheckOpenGLResultAt( __FILE__, __LINE__	);
		free( pRGBLookupTable );
		glActiveTexture( TEXTURE_UNIT_DEFAULT );
		}

	if ( m_MaxTextureUnitsSupportedByGPU < 4 )
		RespondToError( MODULE_GRAPHICS, GRAPHICS_ERROR_INSUFFICIENT_TEXTURE_UNITS );
}







