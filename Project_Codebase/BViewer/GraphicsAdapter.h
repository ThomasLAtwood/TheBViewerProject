// GraphicsAdapter.h : Header file defining the structure of the CGraphicsAdapter
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
// UPDATE HISTORY:
//
//	*[1] 05/01/2023 by Tom Atwood
//		Modified the generation of the extended pixel format to use minimal specifications
//		and to separately set up an 8-bit pixel format for designated displays.
//
#pragma once

#define GLEW_STATIC			// Link Glew with the static library.
#include "glew.h"
#include "wglext.h"


#define GRAPHICS_ERROR_INSUFFICIENT_TEXTURE_UNITS	1

#define GRAPHICS_ERROR_DICT_LENGTH					1


#define MAX_GRAPHICS_ADAPTERS		3

typedef struct _MonitorInfo
	{
	RECT					DesktopCoverageRectangle;
	unsigned short			DisplayAssignment;
								#define DISPLAY_ASSIGNMENT_UNSPECIFIED	0
								#define DISPLAY_ASSIGNMENT_PRIMARY		1
								#define DISPLAY_ASSIGNMENT_STANDARDS	2
								#define DISPLAY_ASSIGNMENT_STUDIES		4
	unsigned short			DisplayIdentity;
								#define DISPLAY_IDENTITY_PRIMARY		1
								#define DISPLAY_IDENTITY_IMAGE2			2
								#define DISPLAY_IDENTITY_IMAGE3			3
	unsigned long			m_MonitorWidthInMM;
	unsigned long			m_MonitorHeightInMM;
	unsigned short			m_AssignedRenderingMethod;					// *[1] Copied the following definitions from ImageView.h.
								#define RENDER_METHOD_NOT_SELECTED				0
								#define RENDER_METHOD_8BIT_COLOR				1
								#define	RENDER_METHOD_16BIT_PACKED_GRAYSCALE	2
								#define	RENDER_METHOD_30BIT_COLOR				3
	void					*m_pGraphicsAdapter;
	struct _MonitorInfo		*pNextMonitor;
	} MONITOR_INFO;


class CGraphicsAdapter
{
public:
	CGraphicsAdapter( void );
	~CGraphicsAdapter( void );

// Attributes:
	char				m_DisplayAdapterName[ MAX_CFG_STRING_LENGTH ];
	MONITOR_INFO		*m_pDisplayMonitorInfoList;		// Monitors attached to this adapter.
	CGraphicsAdapter	*m_pNextGraphicsAdapter;
	int					m_DisplayMonitorCount;
	char				m_OpenGLVersion[ 64 ];
	double				m_OpenGLVersionNumber;

	unsigned long		m_OpenGLSupportLevel;
							#define OPENGL_SUPPORT_ABSENT				0	// OpenGL is not supported.
							#define OPENGL_SUPPORT_PRIMITIVE			1	// OpenGL version is less than 3.30.  Default to OPENGL_SUPPORT_ABSENT.
							#define OPENGL_SUPPORT_330					2	// OpenGL version os 2.0 or greater.
	GLint				m_MaxTextureUnitsSupportedByGPU;
	// Texture unit definitions.  The texture creation, rendering and deletion must all refer to the same texture unit.
	// The default texture should always be set back to GL_TEXTURE0, which can be re-referenced multiple times.
							#define	TEXTURE_UNIT_DEFAULT			GL_TEXTURE0
							#define	TEXTURE_UNIT_LOADED_IMAGE		GL_TEXTURE0
							#define	TEXTURE_UNIT_SCREEN_IMAGE		GL_TEXTURE1		// Used for 2nd pass of 30-bit color rendering.
							#define	TEXTURE_UNIT_GRAYSCALE_LOOKUP	GL_TEXTURE2		// Used for 10-bit grayscale RGB lookup table.
							#define	TEXTURE_UNIT_IMAGE_ANNOTATIONS	GL_TEXTURE3
							#define	TEXTURE_UNIT_IMAGE_MEASUREMENTS	GL_TEXTURE4
							#define	TEXTURE_UNIT_REPORT_TEXT		GL_TEXTURE5
							#define	TEXTURE_UNIT_REPORT_SIGNATURE	GL_TEXTURE6
							#define	TEXTURE_UNIT_REPORT_IMAGE		GL_TEXTURE7

							#define	TEXUNIT_NUMBER_DEFAULT				0
							#define	TEXUNIT_NUMBER_LOADED_IMAGE			0
							#define	TEXUNIT_NUMBER_SCREEN_IMAGE			1		// Used for 2nd pass of 30-bit color rendering.
							#define	TEXUNIT_NUMBER_GRAYSCALE_LOOKUP		2		// Used for 10-bit grayscale RGB lookup table.
							#define	TEXUNIT_NUMBER_IMAGE_ANNOTATIONS	3
							#define	TEXUNIT_NUMBER_IMAGE_MEASUREMENTS	4
							#define	TEXUNIT_NUMBER_REPORT_TEXT			5
							#define	TEXUNIT_NUMBER_REPORT_SIGNATURE		6
							#define	TEXUNIT_NUMBER_REPORT_IMAGE			7

	GLint				m_MaxTextureSize;
	GLhandleARB			m_gImageSystemsShaderProgram;
	GLuint				m_glLUT12BitTextureId;
	int					m_Selected10BitPixelFormatNumber;
	BOOL				m_bAdapterInitializationIsComplete;
	HGPUNV				m_hGPU;
	PFNWGLGETPIXELFORMATATTRIBIVARBPROC			m_pFunctionWglGetPixelFormatAttribiv;
	PFNWGLCHOOSEPIXELFORMATARBPROC				m_pFunctionWglChoosePixelFormat;
	PFNWGLCREATECONTEXTATTRIBSARBPROC			m_pFunctionWglCreateContextAttribs;


// Method Prototypes:
//
	double					GetOpenGLVersion();
	BOOL					Select30BitColorPixelFormat( HDC hDC, unsigned long ImageDisplayMethod );				// *[1] Added the display method argument.
	void					LogPixelFormat( HDC hDC, int nPixelFormat );
	HGLRC					CreateWglRenderingContext( HDC hTargetDC, unsigned long ImageDisplayMethod );			// *[1] Added the display method argument.
	BOOL					CheckOpenGLCapabilities();
	unsigned char			*GenerateRGBLookupTable();
	void					Load10BitGrayscaleShaderLookupTablesAsTextures();		// Required for Image Systems shader to convert grayscale to packed RGB.
	
};



// Function prototypes.
//

	void					InitGraphicsAdapterModule();
	void					CloseGraphicsAdapterModule();
