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


typedef struct
	{
	GLfloat		Xtl;
	GLfloat		Ytl;
	GLfloat		Xbl;
	GLfloat		Ybl;
	GLfloat		Xbr;
	GLfloat		Ybr;
	GLfloat		Xtr;
	GLfloat		Ytr;
	} SQUARE_FRAME;


typedef struct _MeasuredInterval
	{
	POINT						ScreenStartingPoint;
	POINT						ScreenEndingPoint;
	GLfloat						ScaledStartingPointX;
	GLfloat						ScaledStartingPointY;
	GLfloat						ScaledEndingPointX;
	GLfloat						ScaledEndingPointY;
	double						Distance;
	struct _MeasuredInterval	*pNextInterval;
	} MEASURED_INTERVAL;


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
	double				m_DisplayedPixelsPerMM;
	unsigned long		m_DefaultImageSize;
							#define IMAGE_VIEW_FULL_SIZE			1
							#define IMAGE_VIEW_FIT_TO_SCREEN		2
	CDiagnosticImage	*m_pAssignedDiagnosticImage;
	char				*m_pSavedDisplay;
	GLsizei				m_SavedDisplayWidth;
	GLsizei				m_SavedDisplayHeight;
	GLfloat				*m_pWindowingTableScaled8Bit;
	GLfloat				*m_pInversionTableScaled8Bit;
	BOOL				m_bScaleToTextureBuffer;
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
	unsigned char		*m_pDIBImageData;		// Pointer to the pixel data in the printable DIB.
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
	GLuint				m_glImageTextureId;
	unsigned long		m_ImageDisplayMethod;
							#define IMAGE_DISPLAY_UNSPECIFIED			0
							// IMAGE_DISPLAY_SLOW is used when the OpenGL version is less than 2.0 or texturing
							// is not appropriate, such as for color images.
							#define IMAGE_DISPLAY_SLOW					1
							// IMAGE_DISPLAY_USING_8BIT_TEXTURE is used when the OpenGL version is
							// at least 2.0, but when the pixel-packing extensions are not available
							// in the graphics processor.
							#define IMAGE_DISPLAY_USING_8BIT_TEXTURE	2
							// The following display methods are used when the OpenGL version is
							// at least 2.0 and the pixel-packing extensions are available
							// in the graphics processor.  The grayscale bit depth number is determined
							// mainly by the capabilities of the display monitor.
							#define IMAGE_DISPLAY_USING_PACKED_8BIT		3
							#define IMAGE_DISPLAY_USING_PACKED_10BIT	4
							#define IMAGE_DISPLAY_USING_PACKED_12BIT	5
							#define IMAGE_DISPLAY_USING_PACKED_16BIT	6
	unsigned long		m_PrevImageDisplayMethod;
	SQUARE_FRAME		m_SquareFrame;
	IMAGE_ANNOTATION	*m_pImageAnnotationList;


// Method prototypes:
//
public:
	void					DeallocateMembers();
	void					EraseImageAnnotationInfo();
	void					LoadImageAnnotationInfo();
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
	void					EstablishImageDisplayMode();
	BOOL					InitViewport();
	void					LoadWindowingConversionTable( double WindowWidth, double WindowLevel, double GammaValue );
	BOOL					CreateGrayscaleHistogram();
	double					CalculateGrayscaleHistogramMeanLuminosity();
	void					SetImageGrayscalePreference( double WindowWidthSetting, double WindowLevelSetting );
	BOOL					LoadImageAsTexture();
	void					LoadImageAs16BitGrayscaleTexture();
	void					PrepareImage();
	void					RenderImage();
	void					ClearDiagnosticImage();
	void					SetDCPixelFormat( HDC hDC );
	void					SetExportDCPixelFormat( HDC hDC );
	void					RenderImageOverlay(  HDC hDC, unsigned long ImageDestination );
								#define IMAGE_DESTINATION_WINDOW		1
								#define IMAGE_DESTINATION_FILE			2
								#define IMAGE_DESTINATION_PRINTER		3
	void					SaveReport();
	BOOL					OpenReportForPrinting( BOOL bShowPrintDialog );
	void					PrintReportPage( BOOL bUseCurrentStudy );
	void					CloseReportForPrinting();
	void					InitSquareFrame();
	void					FlipFrameHorizontally();
	void					FlipFrameVertically();
};

// Function prototypes:
//
	void					InitImageViewModule();
	void					CloseImageViewModule();


