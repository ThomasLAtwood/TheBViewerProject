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
//	Copyright � 2010 CDC
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
	unsigned short			m_GrayScaleBitDepth;
	void					*m_pGraphicsAdapter;
	struct _MonitorInfo		*pNextMonitor;
	} MONITOR_INFO;


class CGraphicsAdapter
{
public:
	CGraphicsAdapter( void );
	~CGraphicsAdapter( void );

// Attributes:
	char				m_DisplayAdapterName[ 128 ];
	MONITOR_INFO		*m_pDisplayMonitorInfoList;		// Monitors attached to this adapter.
	CGraphicsAdapter	*m_pNextGraphicsAdapter;
	int					m_DisplayMonitorCount;
	char				m_OpenGLVersion[ 64 ];
	unsigned long		m_OpenGLSupportLevel;
							#define OPENGL_SUPPORT_UNSPECIFIED		0
							#define OPENGL_SUPPORT_PRIMITIVE		1	// OpenGL version is less than 2.0.
							#define OPENGL_SUPPORT_TEXTURES			2	// OpenGL version os 2.0 or greater.
							#define OPENGL_SUPPORT_PIXEL_PACK		4	// OpenGL supports NVidia's pixel packing.
							#define OPENGL_SUPPORT_COLOR_MATRIX		8	// OpenGL supports color matrix operations.
	GLint				m_MaxTextureUnitsSupportedByGPU;
	GLint				m_MaxTextureSize;
	GLhandleARB			m_gShaderProgram;
	GLuint				m_glLUT8BitTextureId;
	GLuint				m_glLUT12BitTextureId;
	GLuint				m_glModality_LUTTextureId;
	GLuint				m_glVOI_LUTTextureId;
	BOOL				m_bAdapterInitializationIsComplete;
	HGLRC				m_hOpenGLRenderingContext;
	HGPUNV				m_hGPU;


// Method Prototypes:
//
	long					GetOpenGLVersion();
	HGLRC					CheckOpenGLCapabilities( HDC hDC );
	COLORREF				*GenerateRGBLookupTable( BOOL bLimitTo8BitGrayscale );
	void					LoadShaderLookupTablesAsTextures();
	BOOL					LoadShader();
	
};



// Function prototypes.
//

	void					InitGraphicsAdapterModule();
	void					CloseGraphicsAdapterModule();
