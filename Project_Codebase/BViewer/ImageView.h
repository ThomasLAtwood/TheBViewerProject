// ImageView.h : Header file defining the structure of the CImageView class, which
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
#pragma once

#include <wingdi.h>
#include "GraphicsAdapter.h"
#include "FrameHeader.h"

#define IMAGEVIEW_ERROR_INSUFFICIENT_MEMORY			1
#define IMAGEVIEW_ERROR_GL_INVALID_ENUM				2
#define IMAGEVIEW_ERROR_GL_INVALID_VALUE			3
#define IMAGEVIEW_ERROR_GL_INVALID_OPERATION		4
#define IMAGEVIEW_ERROR_GL_STACK_OVERFLOW			5
#define IMAGEVIEW_ERROR_GL_STACK_UNDERFLOW			6
#define IMAGEVIEW_ERROR_GL_OUT_OF_MEMORY			7
#define IMAGEVIEW_ERROR_GL_TABLE_TOO_LARGE			8
#define IMAGEVIEW_ERROR_GL_UNSPECIFIED_ERROR		9

#define IMAGEVIEW_ERROR_DICT_LENGTH					9


#define MAX_ANNOTATION_COUNT						10
#define MAX_ANNOTATION_CHARS						128


typedef struct _ImageAnnotation
	{
	char						TextField[ MAX_ANNOTATION_CHARS ];
	struct _ImageAnnotation		*pNextAnnotation;
	} IMAGE_ANNOTATION;

#pragma pack(push)
#pragma pack(1)		// Pack vertex array structure members on 1-byte boundaries.


typedef struct
	{
	float		Xbl, Ybl, Zbl;			// X,Y,Z settings for bottom left corner of rectangle to be rendered.
	float		Xbr, Ybr, Zbr;			// Specified in GPU coordinates, -1 < X < +1, etc.
	float		Xtl, Ytl, Ztl;
	float		Xtr, Ytr, Ztr;
	float		TXbl, TYbl;				// X, Y settings for texture coordinates to overlay displayed rectangle.
	float		TXbr, TYbr;				// Specified in texture coordinates, 0 < X < 1, etc.
	float		TXtl, TYtl;
	float		TXtr, TYtr;
	} OPENGL_VERTEX_RECTANGLE;


#pragma pack(pop)

typedef struct
	{
	unsigned int		TextureID;
	unsigned long		BufferSizeInBytes;
	GLYPHMETRICS		GlyphMetrics;
	char				*pBitmapBuffer;
	} GLYPH_BITMAP_INFO;


#pragma pack(push)
#pragma pack(1)		// Pack vertex array structure members on 1-byte boundaries.


typedef struct
	{
	float		OriginMarkHorizontalBeginX, OriginMarkHorizontalBeginY;
	float		OriginMarkHorizontalEndX, OriginMarkHorizontalEndY;
	float		OriginMarkVerticalBeginX, OriginMarkVerticalBeginY;
	float		OriginMarkVerticalEndX, OriginMarkVerticalEndY;

	float		DestinationMarkHorizontalBeginX, DestinationMarkHorizontalBeginY;
	float		DestinationMarkHorizontalEndX, DestinationMarkHorizontalEndY;
	float		DestinationMarkVerticalBeginX, DestinationMarkVerticalBeginY;
	float		DestinationMarkVerticalEndX, DestinationMarkVerticalEndY;

	float		LineBeginX,LineBeginY;
	float		LineEndX,LineEndY;

	} MEASUREMENT_LINE_VERTICES;

#pragma pack(pop)


typedef struct _MeasuredInterval
	{
	POINT						ScreenStartingPoint;
	POINT						ScreenEndingPoint;
	GLfloat						ScaledStartingPointX;
	GLfloat						ScaledStartingPointY;
	GLfloat						ScaledEndingPointX;
	GLfloat						ScaledEndingPointY;
	double						Distance;
	MEASUREMENT_LINE_VERTICES	MeasurementLineVertexArray;
	struct _MeasuredInterval	*pNextInterval;
	} MEASURED_INTERVAL;



#pragma pack(push)
#pragma pack(1)		// Pack vertex array structure members on 1-byte boundaries.

// OpenGL vertices for rendering an "X".  The order is important.
typedef struct
	{
	float		FwdSlashXbl, FwdSlashYbl;			// X,Y settings for bottom left corner of quad to be rendered.
	float		FwdSlashXbr, FwdSlashYbr;			// Specified in GPU coordinates, -1 < X < +1, etc.
	float		FwdSlashXtl, FwdSlashYtl;
	float		FwdSlashXtr, FwdSlashYtr;
	float		BkwdSlashXbl, BkwdSlashYbl;
	float		BkwdSlashXbr, BkwdSlashYbr;
	float		BkwdSlashXtl, BkwdSlashYtl;
	float		BkwdSlashXtr, BkwdSlashYtr;
	} REPORT_CHECKMARK_VERTICES;
#pragma pack(pop)




typedef void (APIENTRY *DEBUGPROC)( GLenum Source, GLenum Type, GLuint Id, GLenum Severity, GLsizei Length, const GLchar *Message, void *UserParam );

// CImageView

class CImageView : public CWnd
{
public:
	CImageView();
	virtual ~CImageView();

	int					m_nPixelFormat;
	HGLRC				m_hRC;				// OpenGL permanent rendering context.
	HDC					m_hDC;				// Private GDI device context.

	MONITOR_INFO		*m_pDisplayMonitor;
	RECT				m_WindowSizeInPixels;
	GLuint				m_OffScreenFrameBufferID;
	GLuint				m_ReportFormFrameBufferID;
	GLuint				m_ReportFormRenderBufferID;

	// 30-bit color is rendered as a 2-stage process.  Packed grayscale uses only the first (loaded image) texture.
	GLuint				m_LoadedImageTextureID;
	GLuint				m_ScreenImageTextureID;
//	GLuint				m_ImageTextureID[ 2 ];
//							#define	LOADED_IMAGE_TEXTURE			0
//							#define	SCREEN_IMAGE_TEXTURE			1
	double				m_DisplayedPixelsPerMM;
	unsigned long		m_DefaultImageSize;
							#define IMAGE_VIEW_FULL_SIZE			1
							#define IMAGE_VIEW_FIT_TO_SCREEN		2
	CDiagnosticImage	*m_pAssignedDiagnosticImage;
	char				m_ViewName[ 32 ];
	unsigned long		m_ViewFunction;
							#define IMAGE_VIEW_FUNCTION_PATIENT		1
							#define IMAGE_VIEW_FUNCTION_STANDARD	2
							#define IMAGE_VIEW_FUNCTION_REPORT		3
	CFrameHeader		*m_pWndDlgBar;
	int					m_PageNumber;
	PRINTDLG			m_UserPrintInfoInput;
	CDC					m_PrinterDC;
	HDC					m_hCompatibleDC;
	HDC					m_hSavedDC;
	HGLRC				m_hGLPrintingRC;
	HGLRC				m_hSavedGLRC;
	HBITMAP				m_hPrintableBitmap;
	BITMAPINFO			m_PrintableBitmapInfo;
	char				m_ReportDateTimeString[ 32 ];
	unsigned char		*m_pDIBImageData;			// Pointer to the pixel data in the printable DIB.
	CMouse				m_Mouse;

	BOOL				m_bEnableMeasure;
	MEASURED_INTERVAL	*m_pActiveMeasurementInterval;
	MEASURED_INTERVAL	*m_pMeasuredIntervalList;
	double				m_PixelsPerMillimeter;		// Copied from diagnostic image, but editable by measurement tool calibration.
	BOOL				m_bEnableAnnotations;

private:
	BOOL				m_bTheMouseIsOverTheImage;
	HCURSOR				m_hDefaultCursor;
	double				m_TimeOfLastFastPaint;
	double				m_TimeOfLastPaint;
	BOOL				m_bRenderingCurrentlyBusy;
	BOOL				m_bImageHasBeenRendered;
	unsigned long		m_ImageDisplayMethod;
								#define RENDER_METHOD_NOT_SELECTED				0
								#define RENDER_METHOD_8BIT_COLOR				1
								#define	RENDER_METHOD_16BIT_PACKED_GRAYSCALE	2
								#define	RENDER_METHOD_30BIT_COLOR				3

	unsigned long				m_PrevImageDisplayMethod;
	IMAGE_ANNOTATION			*m_pImageAnnotationList;
	OPENGL_VERTEX_RECTANGLE		m_VertexRectangle;
	OPENGL_VERTEX_RECTANGLE		m_ScreenVertexRectangle;
	OPENGL_VERTEX_RECTANGLE		m_CharacterGlyphVertexRectangle;
	unsigned int				m_VertexBufferID[ 2 ];
									#define SCREEN_VERTEXES			0
									#define IMAGE_VERTEXES			1
	unsigned int				m_VertexAttributesID[ 2 ];
	GLuint						m_TextureCoordBufferID[ 2 ];
	GLuint						m_TextureCoordAttributesID;
	REPORT_CHECKMARK_VERTICES	m_XMarkVertexArray;

	GLuint						m_g30BitColorShaderProgram;
	GLuint						m_g30BitScreenShaderProgram;
	GLuint						m_g10BitGrayscaleShaderProgram;
	GLuint						m_gImageAnnotationShaderProgram;
	GLuint						m_gImageMeasurementShaderProgram;
	GLuint						m_gLineDrawingShaderProgram;
	GLuint						m_gReportTextShaderProgram;
	GLuint						m_gReportSignatureShaderProgram;
	GLuint						m_gReportFormShaderProgram;

	GLYPH_BITMAP_INFO			m_AnnotationFontGlyphBitmapArray[ 128 ];
	GLfloat						m_AnnotationCharHeight;
	GLYPH_BITMAP_INFO			m_MeasurementFontGlyphBitmapArray[ 128 ];
	GLfloat						m_MeasurementCharHeight;
	GLYPH_BITMAP_INFO			m_ReportFontGlyphBitmapArray[ 128 ];
	unsigned int				m_ReportVertexBufferID;
	unsigned int				m_ReportVertexAttributesID;


// Method prototypes:
//
public:
	void					InitializeImageVertices();
	void					DeleteImageVertices();
	BOOL					InitializeOffScreenFrameBuffer();
	void					RemoveOffScreenFrameBuffer();
	void					DeallocateMembers();
	void					SetDiagnosticImage( CDiagnosticImage *pDiagnosticImage, CStudy *pStudy );
	void					ResetDiagnosticImage( BOOL bRescaleOnly );
	void					UpdateImageGrayscaleDisplay( IMAGE_GRAYSCALE_SETTING *pNewGrayscaleSetting );
	void					RepaintFast();
	void					LoadCurrentImageSettingsIntoEditBoxes();

// Overrides
	//{{AFX_VIRTUAL(CImageView)
	//}}AFX_VIRTUAL

protected:
	DECLARE_MESSAGE_MAP()
	void					GetExclusiveRightToRender();
	void					AllowOthersToRender();

public:
	//{{AFX_MSG(CImageView)
	afx_msg void			OnPaint();
	afx_msg void			OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void			OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void			OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void			OnRButtonUp( UINT nFlags, CPoint point );
	afx_msg BOOL			OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
	afx_msg void			OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void			OnSize( UINT nType, int cx, int cy );
	afx_msg int				OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void			OnDestroy();
	//}}AFX_MSG

public:
	BOOL					CheckOpenGLResultAt( char *pSourceFile, int SourceLineNumber );
	void					SetUpDebugContext();
	void					EstablishImageDisplayMode();
	BOOL					InitViewport();
	BOOL					CreateGrayscaleHistogram();
	double					CalculateGrayscaleHistogramMeanLuminosity();
	void					SetImageGrayscalePreference( double WindowWidthSetting, double WindowLevelSetting );
	BOOL					LoadImageAsTexture();
	void					PrepareImage();
	BOOL					LoadGPUShaderPrograms();
	BOOL					PrepareGPUShaderProgram( char *pVertexShaderSourceCode, char *pFragmentShaderSourceCode, GLuint *pShaderProgram );
	void					InitializeAndLoadTheImageTexture();
	void					RenderImage();
	void					ClearDiagnosticImage();
	void					SetDCPixelFormat( HDC hDC );
	void					SetExportDCPixelFormat( HDC hDC );
	void					CreateReportImage( unsigned long ImageDestination, BOOL bUseCurrentStudy );
	void					DeleteReportImage();
	void					SaveReport();
	BOOL					OpenReportForPrinting( BOOL bShowPrintDialog );
	void					PrintReportPage( BOOL bUseCurrentStudy );
	void					CloseReportForPrinting();
	void					EraseImageAnnotationInfo();
	void					LoadImageAnnotationInfo();
	void					CreateImageAnnotationFontGlyphs( HDC hDC );
	void					DeleteImageAnnotationFontGlyphs();
	void					RenderImageAnnotations(  HDC hDC );
	void					RenderReportTextString( GLuint hShaderProgram, GLYPH_BITMAP_INFO *GlyphBitmapArray, char *pTextString, unsigned int VertexBufferID,
														unsigned int VertexAttributesID, float x, float y, GLfloat Color[ 3 ] );
	void					RenderTextString( GLuint hShaderProgram, GLuint TextureUnit, GLYPH_BITMAP_INFO *GlyphBitmapArray, char *pTextString, unsigned int VertexBufferID,
														unsigned int VertexAttributesID, float x, float y, GLfloat Color[ 3 ] );
	void					CreateImageMeasurementFontGlyphs( HDC hDC );
	void					DeleteImageMeasurementFontGlyphs();
	void					RenderImageMeasurementLines();
	void					RenderImageMeasurements();
	BOOL					CreateReportFontGlyphs( HDC hDC, int FontHeight, int FontWidth, int FontWeight, BOOL bItalic, char FontPitch, char* pFontName );
	void					DeleteReportFontGlyphs();
	void					CreateReportTextVertices( GLuint hShaderProgram );
	void					DeleteReportTextVertices( GLuint hShaderProgram );
	void					RenderReportCheckmark();
	void					CreateSignatureTexture();
	void					RenderSignatureTexture( GLuint hShaderProgram, SIGNATURE_BITMAP *pSignatureBitmap, unsigned int VertexBufferID,
											unsigned int VertexAttributesID, float x, float y, float ScaledBitmapWidth, float ScaledBitmapHeight );
	void					RenderReport(  HDC hDC, unsigned long ImageDestination );
								#define IMAGE_DESTINATION_WINDOW		1
								#define IMAGE_DESTINATION_FILE			2
								#define IMAGE_DESTINATION_PRINTER		3
	unsigned int			CreateReportFormTexture();
	BOOL					InitializReportFormFrameBuffer();
	void					RenderReportFormTexture( GLuint hShaderProgram, unsigned int TextureID, unsigned int VertexBufferID,
											unsigned int VertexAttributesID, float x, float y, float ScaledBitmapWidth, float ScaledBitmapHeight );
	void					InitImageVertexRectangle( float XMin, float XMax, float YMin, float YMax, float ViewportAspectRatio );
	void					InitScreenVertexSquareFrame();
	void					InitCharacterGlyphVertexRectangle( float XMin, float XMax, float YMin, float YMax );
	void					FlipFrameHorizontally();
	void					FlipFrameVertically();
};

// Function prototypes:
//
	void					InitImageViewModule();
	void					CloseImageViewModule();


