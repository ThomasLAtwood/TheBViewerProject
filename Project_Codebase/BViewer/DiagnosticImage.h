// DiagnosticImage.h - Defines the image class that represents and
// describes an image to be displayed.
//
// An image is read into a CDiagnosticImage object from a (possibly
// modified) PNG file under the control of a CImageFrame object.
// It is displayed via an ImageView object.
//
// A CDiagnosticImage object may be distinguished by its type as
// an ILO Standard image, a Subject Study image, or a Report Image.
// Standard and Study images may have prefixed information, such
// as calibration data, that must be stripped out of the file before
// a valid PNG image is produced.
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
#include "GrayscaleSetting.h"

#pragma pack(push)
#pragma pack(1)		// Pack calibration structure members on 1-byte boundaries.
#include "Calibration.h"
#pragma pack(pop)


#define IMAGE_ERROR_INSUFFICIENT_MEMORY			1
#define IMAGE_ERROR_FILE_OPEN					2
#define IMAGE_ERROR_FILE_IS_NOT_PNG				3
#define IMAGE_ERROR_IMAGE_READ					4
#define IMAGE_ERROR_IMAGE_WRITE					5

#define IMAGE_ERROR_DICT_LENGTH					5



class CDiagnosticImage
{
public:
	CDiagnosticImage();
	virtual ~CDiagnosticImage( void );

public:
	IMAGE_GRAYSCALE_SETTING	m_CurrentGrayscaleSetting;
	IMAGE_GRAYSCALE_SETTING	m_OriginalGrayscaleSetting;
	unsigned long			m_MaxGrayscaleValue;
	double					m_RescaleSlope;
	double					m_RescaleIntercept;

	// The focal point is the pixel coordinate location on the image that lies at
	// the center of the display rectangle.  This is the point about which rotations
	// occur, and which is kept centered when the image is being magnified.
	POINT					m_FocalPoint;
	unsigned long			m_ImageWidthInPixels;
	unsigned long			m_ImageHeightInPixels;
	double					m_ImageAspectRatio;
	int						m_ImageBitDepth;
	unsigned short			m_SamplesPerPixel;
	unsigned short			m_nBitsAllocated;
	unsigned short			m_nBitsStored;
	unsigned short			m_nHighBit;
	unsigned short			m_MaxObservedPixelValue;
	unsigned short			m_MinObservedPixelValue;
	int						m_ImageColorFormat;
	IMAGE_CALIBRATION_INFO	*m_pImageCalibrationInfo;			// Allocated by ReadPNGFileHeader().
	unsigned char			*m_pImageData;
	BOOL					m_bEnableGammaCorrection;
	BOOL					m_bEnableOverlays;
	double					m_ScaleFactor;
	BOOL					m_ImageHeightControlsScaling;
	double					m_FullSizeMillimetersPerPixel;
	unsigned short			m_RotationQuadrant;
	BOOL					m_bFlipHorizontally;
	BOOL					m_bFlipVertically;
	double					m_ActualImageHeightInInches;
	double					m_ActualImageWidthInInches;
	double					m_PixelsPerMillimeter;

	HISTOGRAM_DATA			m_LuminosityHistogram;			
	unsigned char			*m_pOutputImageData;
	unsigned long			m_OutputImageWidthInPixels;
	unsigned long			m_OutputImageHeightInPixels;
	BOOL					m_bConvertImageTo8BitGrayscale;
	BOOL					m_bImageHasBeenDownSampled;
	BOOL					m_bImageHasBeenCompacted;


// Method prototypes:
//
	void			ResetImage( int CanvasWidth, int CanvasHeight, BOOL bFullSize, double DisplayedPixelsPerMM, BOOL bRescaleOnly );
	void			LoadStudyWindowCenterAndWidth();
	void			LoadGrayscaleCalibrationParameters();
	void			AdjustScale( double MultiplicativeAdjustment );
	void			AdjustRotationAngle();
	void			FlipHorizontally();
	void			FlipVertically();
	void			AnalyzeImagePixels();
	void			ApplyModalityLUT();
	void			ApplyVOI_LUT();
	BOOL			ExtractUncompressedImageToFile( char *pFileSpec );
	void			ReduceTo8BitGrayscale();
	void			ReducePixelsToEightBits();
	void			DownSampleImageResolution();
	BOOL			ReadPNGImageFile( char *pFileSpec, MONITOR_INFO *pDisplayMonitor, unsigned long ImageContentType );
	BOOL			WritePNGImageFile( char *pFileSpec );
};


// Function prototypes:
	void			InitImageModule();
	void			CloseImageModule();
	BOOL			ReadPNGFileHeader( FILE *pImageFile, IMAGE_CALIBRATION_INFO **ppImageCalibrationInfo );



