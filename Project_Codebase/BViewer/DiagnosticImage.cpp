// DiagnosticImage.cpp - Implements the image class that represents and
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
// UPDATE HISTORY:
//
//	*[3] 07/17/2023 by Tom Atwood
//		Fixed code security issues.
//	*[2] 03/15/2023 by Tom Atwood
//		Fixed code security issues.
//	*[1] 01/06/2023 by Tom Atwood
//		Fixed code security issues.
//
//
#include "StdAfx.h"
#include <math.h>
#include <stdio.h>
#include "BViewer.h"
#include "Module.h"
#include "ReportStatus.h"
#include "Access.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "Customization.h"
#include "FrameHeader.h"

extern CBViewerApp			ThisBViewerApp;


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		ImageModuleInfo = { MODULE_IMAGE, "Image Module", InitImageModule, CloseImageModule };


static ERROR_DICTIONARY_ENTRY	ImageErrorCodes[] =
			{
				{ IMAGE_ERROR_INSUFFICIENT_MEMORY		, "An error occurred allocating a memory block for data storage." },
				{ IMAGE_ERROR_FILE_OPEN					, "An error occurred attempting to open an image file." },
				{ IMAGE_ERROR_FILE_IS_NOT_PNG			, "The image file you are trying to open does not appear to have a valid file format." },
				{ IMAGE_ERROR_IMAGE_READ				, "An error occurred attempting to read an image file." },
				{ IMAGE_ERROR_IMAGE_WRITE				, "An error occurred attempting to write an image file." },
				{ 0										, NULL }
			};

static ERROR_DICTIONARY_MODULE		ImageStatusErrorDictionary =
										{
										MODULE_IMAGE,
										ImageErrorCodes,
										IMAGE_ERROR_DICT_LENGTH,
										0
										};

// This function must be called before any other function in this module.
void InitImageModule()
{
	LinkModuleToList( &ImageModuleInfo );
	RegisterErrorDictionary( &ImageStatusErrorDictionary );
}


void CloseImageModule()
{
}


CDiagnosticImage::CDiagnosticImage()
{
	m_pImageData = 0;
	m_pOutputImageData = 0;
	strncpy_s( m_CurrentGrayscaleSetting.m_PresetName, MAX_CFG_STRING_LENGTH, "Current Image Grayscale Specifications", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	m_CurrentGrayscaleSetting.m_Gamma = 1.0;
	m_CurrentGrayscaleSetting.m_bColorsInverted = FALSE;
	m_CurrentGrayscaleSetting.m_RelativeMouseHorizontalPosition = 0.0;
	m_CurrentGrayscaleSetting.m_RelativeMouseVerticalPosition = 0.0;
	m_CurrentGrayscaleSetting.m_WindowMinPixelAmplitude = 0.0;
	m_CurrentGrayscaleSetting.m_WindowMaxPixelAmplitude = 0.0;
	strncpy_s( m_OriginalGrayscaleSetting.m_PresetName, MAX_CFG_STRING_LENGTH, "Original Image Grayscale Specifications", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	m_OriginalGrayscaleSetting.m_Gamma = 1.0;
	m_OriginalGrayscaleSetting.m_bColorsInverted = FALSE;
	m_OriginalGrayscaleSetting.m_WindowWidth = 0.0;
	m_OriginalGrayscaleSetting.m_WindowCenter = 0.0;
	m_OriginalGrayscaleSetting.m_RelativeMouseHorizontalPosition = 0.0;
	m_OriginalGrayscaleSetting.m_RelativeMouseVerticalPosition = 0.0;
	m_OriginalGrayscaleSetting.m_WindowMinPixelAmplitude = 0.0;
	m_OriginalGrayscaleSetting.m_WindowMaxPixelAmplitude = 0.0;
	m_ScaleFactor = 1.0;
	m_FullSizeMillimetersPerPixel = 1.0;
	m_RotationQuadrant = 0;
	m_bFlipHorizontally = FALSE;
	m_bFlipVertically = FALSE;
	m_ActualImageHeightInInches = 17.0;
	m_ActualImageWidthInInches = 14.0;
	m_ImageHeightInPixels = 0;
	m_ImageWidthInPixels = 0;
	m_SamplesPerPixel = 0;
	m_ImageColorFormat = 0;
	m_bEnableGammaCorrection = FALSE;
	m_bEnableOverlays = TRUE;
	m_bImageHasBeenDownSampled = FALSE;
	m_bImageHasBeenCompacted = FALSE;
	m_pImageCalibrationInfo = 0;			// Allocated by ReadPNGFileHeader().
	m_MaxGrayscaleValue = 256;
	m_bConvertImageTo8BitGrayscale = FALSE;
	m_LuminosityHistogram.nNumberOfBins = 0;
	m_LuminosityHistogram.AverageBinValue = 0.0;
	m_LuminosityHistogram.pHistogramArray = 0;
	m_LuminosityHistogram.AverageViewableBinValue = 0.0;
}


CDiagnosticImage::~CDiagnosticImage( void )
{
	if ( m_pImageData != 0 )
		{
		free( m_pImageData );
		m_pImageData = 0;
		}
	if ( m_pOutputImageData != 0 )
		{
		free( m_pOutputImageData );
		m_pOutputImageData = 0;
		}
	if ( m_pImageCalibrationInfo != 0 )
		free( m_pImageCalibrationInfo );
	if ( m_LuminosityHistogram.pHistogramArray != 0 )
		{
		free( m_LuminosityHistogram.pHistogramArray );
		m_LuminosityHistogram.pHistogramArray = 0;
		}
}


void CDiagnosticImage::LoadStudyWindowCenterAndWidth()
{
	if ( ThisBViewerApp.m_pCurrentStudy != 0 )
		{
		ThisBViewerApp.m_pCurrentStudy -> m_WindowCenter = m_CurrentGrayscaleSetting.m_WindowCenter;
		ThisBViewerApp.m_pCurrentStudy -> m_WindowWidth = m_CurrentGrayscaleSetting.m_WindowWidth;
		}
}


void CDiagnosticImage::LoadGrayscaleCalibrationParameters()
{
	BOOL			bVOI_LUTIsSpecified;
	BOOL			bWindowingIsSpecified;

	// Set default scale factors.
	m_RescaleSlope = 1.0;
	m_RescaleIntercept = 0.0;

	if ( m_pImageCalibrationInfo != 0 )
		{
		bWindowingIsSpecified = ( ( m_pImageCalibrationInfo -> SpecifiedCalibrationTypes & CALIBRATION_TYPE_VOI_WINDOW ) != 0 );
		bVOI_LUTIsSpecified = ( ( m_pImageCalibrationInfo -> SpecifiedCalibrationTypes & CALIBRATION_TYPE_VOI_LUT ) != 0 );

		m_MaxGrayscaleValue = (unsigned long) pow( (double)2, m_pImageCalibrationInfo -> BitsAllocated ) - 1;
		if ( m_pImageCalibrationInfo -> SpecifiedCalibrationTypes & CALIBRATION_TYPE_MODALITY_RESCALE )
			{
			m_RescaleSlope = m_pImageCalibrationInfo -> RescaleSlope;
			m_RescaleIntercept = m_pImageCalibrationInfo -> RescaleIntercept;
			}
		if ( bWindowingIsSpecified || bVOI_LUTIsSpecified )
			{
			m_OriginalGrayscaleSetting.m_WindowCenter = m_pImageCalibrationInfo -> WindowCenter;
			m_OriginalGrayscaleSetting.m_WindowWidth = m_pImageCalibrationInfo -> WindowWidth;
			}
		else
			{
			m_OriginalGrayscaleSetting.m_WindowCenter = m_MaxGrayscaleValue / 2.0;
			m_OriginalGrayscaleSetting.m_WindowWidth = m_MaxGrayscaleValue + 1;
			}
		m_OriginalGrayscaleSetting.m_bColorsInverted = FALSE;
		m_OriginalGrayscaleSetting.m_Gamma = 1.0;
		m_OriginalGrayscaleSetting.m_RelativeMouseHorizontalPosition = 0.0;
		m_OriginalGrayscaleSetting.m_RelativeMouseVerticalPosition = 0.0;
		m_OriginalGrayscaleSetting.m_WindowMinPixelAmplitude =
						m_OriginalGrayscaleSetting.m_WindowCenter - ( m_OriginalGrayscaleSetting.m_WindowWidth - 1.0 ) / 2.0;
		m_OriginalGrayscaleSetting.m_WindowMaxPixelAmplitude =
						m_OriginalGrayscaleSetting.m_WindowCenter - 0.5 + ( m_OriginalGrayscaleSetting.m_WindowWidth - 1.0 ) / 2.0;
		memcpy( (char*)&m_CurrentGrayscaleSetting, (char*)&m_OriginalGrayscaleSetting, sizeof(IMAGE_GRAYSCALE_SETTING) );
		LoadStudyWindowCenterAndWidth();
		}
}


void CDiagnosticImage::ResetImage( int CanvasWidth, int CanvasHeight, BOOL bDisplayFullSize, double DisplayedPixelsPerMM, BOOL bRescaleOnly )
{
	double				CanvasAspectRatio;

	if ( !bRescaleOnly )
		{
		m_RotationQuadrant = 0;
		m_bFlipHorizontally = FALSE;
		m_bFlipVertically = FALSE;
		m_FocalPoint.x = m_ImageWidthInPixels / 2;
		m_FocalPoint.y = m_ImageHeightInPixels / 2;
		LoadGrayscaleCalibrationParameters();
		}

	m_ImageAspectRatio = (double)m_ImageWidthInPixels / (double)m_ImageHeightInPixels;
	if ( m_ImageAspectRatio < 1.0 )
		{
		m_FullSizeMillimetersPerPixel = m_ActualImageHeightInInches * 25.4 * DisplayedPixelsPerMM / (double)m_ImageHeightInPixels;
		m_ImageHeightControlsScaling = TRUE;
		}
	else
		{
		m_FullSizeMillimetersPerPixel = m_ActualImageWidthInInches * 25.4 * DisplayedPixelsPerMM / (double)m_ImageWidthInPixels;
		m_ImageHeightControlsScaling = FALSE;
		}
	if ( bDisplayFullSize )
		m_ScaleFactor = m_FullSizeMillimetersPerPixel;
	else
		{
		CanvasAspectRatio = (double)CanvasWidth / (double)CanvasHeight;
		if ( CanvasAspectRatio >= m_ImageAspectRatio )
			{
			m_ScaleFactor = (double)CanvasHeight / (double)m_ImageHeightInPixels;
			m_ImageHeightControlsScaling = TRUE;
			}
		else
			{
			m_ScaleFactor = (double)CanvasWidth / (double)m_ImageWidthInPixels;
			m_ImageHeightControlsScaling = FALSE;
			}
		}
}


void CDiagnosticImage::AdjustScale( double MultiplicativeAdjustment )
{
	m_ScaleFactor *= MultiplicativeAdjustment;
}


void CDiagnosticImage::AdjustRotationAngle()
{
	m_RotationQuadrant += 1;
	if ( m_RotationQuadrant >= 4 )
		m_RotationQuadrant -= 4;
}


void CDiagnosticImage::FlipHorizontally()
{
	m_bFlipHorizontally = !m_bFlipHorizontally;
}


void CDiagnosticImage::FlipVertically()
{
	m_bFlipVertically = !m_bFlipVertically;
}


static char				PixelBitsInvertedMsg[] = ", Pixel bits are inverted";
static char				VOI_LUTIsSpecifiedMsg[] = ", A VOI-LUT is specified";

// This function is called immediately after the image has been read from the PNG file and buffered.
// It performs an examination of the pixel data and compares with the characteristics declared in
// the Dicom data elements.  Any detected errors are logged and overridden.
void CDiagnosticImage::AnalyzeImagePixels()
{
	BOOL				bNoError = TRUE;
	BOOL				bVOI_LUTIsSpecified;
	BOOL				bModalityLUTIsSpecified;
	unsigned long		nPixel;
	size_t				nPixelCount;
	unsigned char		*p8BitPixel;
	unsigned short		*p16BitPixel;
	unsigned short		nPixelValue;
	unsigned char		n8BitPixelValue;
	unsigned short		nBitsStored;
	unsigned short		nHighBit;
	unsigned short		nObservedHighBit;
	unsigned short		PixelBitsSet;
	unsigned short		MaxObservedPixelValue;
	unsigned short		MinObservedPixelValue;
	unsigned short		nBit;						// *[1] Changed data type from int to unsigned short.
	char				MinValueBinary[ 20 ];
	char				MaxValueBinary[ 20 ];
	char				Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];

	nPixelCount = m_ImageWidthInPixels * m_ImageHeightInPixels;
	// Get the image characteristic values declared in the Dicom data elements.
	m_nBitsAllocated = m_pImageCalibrationInfo -> BitsAllocated;
	nBitsStored =  m_pImageCalibrationInfo -> BitsStored & 0x00FF;
	nHighBit = ( ( m_pImageCalibrationInfo -> BitsStored >> 8 ) & 0x00FF );
	bModalityLUTIsSpecified = ( ( m_pImageCalibrationInfo -> SpecifiedCalibrationTypes & CALIBRATION_TYPE_MODALITY_LUT ) != 0 );
	bVOI_LUTIsSpecified = ( ( m_pImageCalibrationInfo -> SpecifiedCalibrationTypes & CALIBRATION_TYPE_VOI_LUT ) != 0 );

	PixelBitsSet = 0;
	MaxObservedPixelValue = 0;

	if ( m_ImageBitDepth <= 8 )
		{
		MinObservedPixelValue = 0x00ff;
		p8BitPixel = m_pImageData;

		for ( nPixel = 0; nPixel < nPixelCount; nPixel++ )
			{
			n8BitPixelValue = *p8BitPixel;

			PixelBitsSet |= (unsigned short)n8BitPixelValue;
			if ( m_pImageCalibrationInfo -> PhotometricInterpretation == PMINTERP_MONOCHROME1 )
				n8BitPixelValue = ~n8BitPixelValue; 

			if ( (unsigned short)n8BitPixelValue > MaxObservedPixelValue )
				MaxObservedPixelValue = (unsigned short)n8BitPixelValue;
			if ( (unsigned short)n8BitPixelValue < MinObservedPixelValue )
				MinObservedPixelValue = (unsigned short)n8BitPixelValue;

			p8BitPixel++;
			}
		}
	else
		{
		MinObservedPixelValue = 0xffff;
		p16BitPixel = (unsigned short*)m_pImageData;

		for ( nPixel = 0; nPixel < nPixelCount; nPixel++ )
			{
			nPixelValue = *p16BitPixel;

			PixelBitsSet |= nPixelValue;
			if ( m_pImageCalibrationInfo -> PhotometricInterpretation == PMINTERP_MONOCHROME1 )
				nPixelValue = ~nPixelValue; 

			if ( nPixelValue > MaxObservedPixelValue )
				MaxObservedPixelValue = nPixelValue;
			if ( nPixelValue < MinObservedPixelValue )
				MinObservedPixelValue = nPixelValue;

			p16BitPixel++;
			}
		}

	m_MinObservedPixelValue = MinObservedPixelValue;
	m_MaxObservedPixelValue = MaxObservedPixelValue;

	nObservedHighBit = 0;
	for ( nBit = 0; nBit < 16; nBit++ )
		{
		if ( ( PixelBitsSet & ( 1 << nBit ) ) != 0 )
			nObservedHighBit = nBit;
		}

	// Make backward compatible for images converted to PNG by previous BRetriever versions.
	if ( nHighBit == 0 )
		nHighBit = nObservedHighBit;

	_itoa( m_MinObservedPixelValue, MinValueBinary, 2 );
	_itoa( m_MaxObservedPixelValue, MaxValueBinary, 2 );
	_snprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH,  _TRUNCATE, "Image pixel value range:  Minimum = %s, Maximum = %s, Observed high bit = %d",			// *[2] Replaced sprintf() with _snprintf_s.
																								MinValueBinary, MaxValueBinary, nObservedHighBit );
	LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

	if ( nHighBit != nObservedHighBit && nObservedHighBit == 7 )
		{
		_snprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH,  _TRUNCATE,																						// *[2] Replaced sprintf() with _snprintf_s.
						">>> HighBit declared as %d, versus %d measured value.", nHighBit, nObservedHighBit );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		if ( !bModalityLUTIsSpecified && !bVOI_LUTIsSpecified )
			{
			_snprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH,  _TRUNCATE,																					// *[2] Replaced sprintf() with _snprintf_s.
							">>> Adjusting declared bits stored from %d, to %d.", nBitsStored, nObservedHighBit + 1 );
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
			// Adjust the image pixels to agree with the declared bits stored.
			if ( m_ImageBitDepth > 8 && nHighBit > nObservedHighBit )
				{
				p16BitPixel = (unsigned short*)m_pImageData;

				for ( nPixel = 0; nPixel < nPixelCount; nPixel++ )
					{
					nPixelValue = *p16BitPixel;
					nPixelValue <<= ( nHighBit - nObservedHighBit );
					 *p16BitPixel = nPixelValue;
					p16BitPixel++;
					}
				}
			}
		}

	m_nHighBit = nHighBit;
	m_nBitsStored = nBitsStored;

	_snprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH,  _TRUNCATE,								// *[2] Replaced sprintf() with _snprintf_s.
				"    Image characteristics:  %d bits allocated, %d bits stored, high bit is %d", m_nBitsAllocated, m_nBitsStored, m_nHighBit );
	if ( m_pImageCalibrationInfo -> PhotometricInterpretation == PMINTERP_MONOCHROME1 )
		strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, PixelBitsInvertedMsg, _TRUNCATE );	// *[3] Replaced strcat with strncat_s.
	if ( bVOI_LUTIsSpecified )
		strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, VOI_LUTIsSpecifiedMsg, _TRUNCATE );	// *[3] Replaced strcat with strncat_s.
	strncat_s( Msg, ".", _TRUNCATE );														// *[3] Replaced strcat with strncat_s.
	LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
}


// From the Dicom standard, volume PS 3.4
// N.2.1.1	Modality LUT
// The Modality LUT operation applies only to grayscale values.
// The Modality LUT transformation transforms the manufacturer dependent pixel values into pixel values
// which are meaningful for the modality and which are manufacturer independent (e.g., Hounsfield number
// for CT modalities, Optical Density for film digitizers).  These may represent physical units or be
// dimensionless. The Modality LUT in the Presentation State is modality dependent and is analogous to
// the same module in an Image.
//
// Notes:	1. In some cases, such as the CT Image Storage SOP Class, the same conceptual step as the
//				Modality LUT is specified in another form, for example as Rescale Slope and Rescale
//				Intercept Attributes in the CT Image Module, though the Modality LUT Module is not
//				part of the CT Image IOD.
//			2. Image pixel values with a value of Pixel Padding Value (0028,0120) in the referenced
//				image, or within the range specified by Pixel Padding Value (0028,0120) and Pixel
//				Padding Range Limit (0028,0121) (if present in the referenced image) shall be accounted
//				for prior to entry to the Modality LUT stage. See the definition of Pixel Padding Value
//				in PS 3.3. Neither Pixel Padding Value (0028,0120) nor Pixel Padding Range Limit
//				(0028,0121) are encoded in the Presentation State Instance.
//
// In the case of a linear transformation, the Modality LUT is described by the Rescale Slope (0028,1053)
// and Rescale Intercept (0028,1052).  In the case of a non-linear transformation, the Modality LUT is
// described by the Modality LUT Sequence. The rules for application of the Modality LUT are defined
// in PS 3.3 Modality LUT Module.
//
// If the Modality LUT or equivalent Attributes are part of both the Image and the Presentation State,
// then the Presentation State Modality LUT shall be used instead of the Image Modality LUT or equivalent
// Attributes in the Image. If the Modality LUT is not present in the Presentation State it shall be
// assumed to be an identity transformation. Any Modality LUT or equivalent Attributes in the Image
// shall not be used.


// This function responds to the Photometric Interpretation and Pixel Representation
// Dicom element specifications to produce an image with grayscale that is normalized
// to 8-bits or 16-bits of significance, according to m_ImageBitDepth, and where
// a "1" bit value is brighter than a "0".  Any modality LUT or rescale specifications
// are applied.
void CDiagnosticImage::ApplyModalityLUT()
{
	BOOL				bNoError = TRUE;
	unsigned long		nPixel;
	size_t				nPixelCount;
	unsigned char		*p8BitPixel;
	char				*ps8BitPixel;
	unsigned short		*p16BitPixel;
	short				*ps16BitPixel;
	double				fPixelValue;
	unsigned short		nPixelValue;
	unsigned char		n8BitPixelValue;
	BOOL				bModalityRescaleIsSpecified;
	BOOL				bModalityLUTIsSpecified;
	double				RescaleSlope;
	double				RescaleIntercept;
	unsigned char		*pRaw8bitLookupData;
	unsigned short		*pRaw16bitLookupData;
	size_t				nModalityLUTLength;
	double				MaxPixelValue;
	double				Bias;
	unsigned short		SignExtensionMask;
	unsigned short		SignExtension8BitMask;
	BOOL				bFujiPhotoWindowingException;
	
	MaxPixelValue = (double)( 1 << m_ImageBitDepth );
	nPixelCount = m_ImageWidthInPixels * m_ImageHeightInPixels;
	nModalityLUTLength = m_pImageCalibrationInfo -> ModalityLUTElementCount;
	if ( nModalityLUTLength == 0 )
		nModalityLUTLength = 65536;
	bModalityRescaleIsSpecified = ( ( m_pImageCalibrationInfo -> SpecifiedCalibrationTypes & CALIBRATION_TYPE_MODALITY_RESCALE ) != 0 );
	bModalityLUTIsSpecified = ( ( m_pImageCalibrationInfo -> SpecifiedCalibrationTypes & CALIBRATION_TYPE_MODALITY_LUT ) != 0 );
	if ( m_pImageCalibrationInfo -> bPixelValuesAreSigned )
		Bias = (double)( 1 << ( m_nBitsStored - 1 ) );
	else
		Bias = 0.0;

	if ( bNoError && m_ImageBitDepth <= 8 )			// Process 8-bit pixels.
		{
		p8BitPixel = m_pImageData;
		pRaw8bitLookupData = (unsigned char*)m_pImageCalibrationInfo -> pModalityLUTData;
		SignExtension8BitMask = (-1) << m_nBitsStored;
		if ( bModalityLUTIsSpecified )
			{
			for ( nPixel = 0; nPixel < nPixelCount; nPixel++ )
				{
				n8BitPixelValue = *p8BitPixel;
				n8BitPixelValue -= *( (char*)&m_pImageCalibrationInfo -> ModalityLUTThresholdPixelValue );
				// Suppress any sign extension.
				n8BitPixelValue &= ~SignExtension8BitMask;
				// Perform the lookup, using the input pixel value as the index into the LUT.
				if ( n8BitPixelValue <= 0 )
					n8BitPixelValue = pRaw8bitLookupData[ 0 ];
				else
					{
					if ( n8BitPixelValue <= nModalityLUTLength )
						n8BitPixelValue = pRaw8bitLookupData[ n8BitPixelValue ];
					else
						n8BitPixelValue = pRaw8bitLookupData[ nModalityLUTLength - 1 ];
					}
				// Restore the modified pixel into the image buffer.
				*p8BitPixel = n8BitPixelValue;
				p8BitPixel++;
				}
			}
		else
			{
			RescaleSlope = m_pImageCalibrationInfo -> RescaleSlope;
			RescaleIntercept = m_pImageCalibrationInfo -> RescaleIntercept;
			if ( RescaleSlope == 0.0 )
				RescaleSlope = 1.0;
			for ( nPixel = 0; nPixel < nPixelCount; nPixel++ )
				{
				if ( m_pImageCalibrationInfo -> bPixelValuesAreSigned )
					{
					ps8BitPixel = (char*)p8BitPixel;
					fPixelValue = (double)(*ps8BitPixel);
					}
				else
					fPixelValue = (double)(*p8BitPixel);
				fPixelValue = Bias + fPixelValue;

				if ( bModalityRescaleIsSpecified )
					fPixelValue = ( fPixelValue * RescaleSlope ) - RescaleIntercept + 0.5;
				if ( fPixelValue < 0.0 )
					fPixelValue = 0.0;
				else if ( fPixelValue >= MaxPixelValue )
					fPixelValue = MaxPixelValue - 1;
				*p8BitPixel = (unsigned char)fPixelValue;
				p8BitPixel++;
				}
			}
		p8BitPixel = m_pImageData;
		}
	else if ( bNoError )			// Process 16-bit pixels.
		{
		bFujiPhotoWindowingException = ( _strnicmp( m_pImageCalibrationInfo -> Manufacturer, "FUJI PHOTO", 10 ) == 0 &&  _stricmp( m_pImageCalibrationInfo -> Modality, "CR" ) == 0 );
		p16BitPixel = (unsigned short*)m_pImageData;
		pRaw16bitLookupData = (unsigned short*)m_pImageCalibrationInfo -> pModalityLUTData;
		SignExtensionMask = (-1) << m_nBitsStored;
		if ( bModalityLUTIsSpecified )
			{
			for ( nPixel = 0; nPixel < nPixelCount; nPixel++ )
				{
				nPixelValue = *p16BitPixel;
				nPixelValue -= *( (short*)&m_pImageCalibrationInfo -> ModalityLUTThresholdPixelValue );
				// Suppress any sign extension.
				nPixelValue &= ~SignExtensionMask;
				// Perform the lookup, using the input pixel value as the index into the LUT.
				if ( nPixelValue <= 0 )
					nPixelValue = pRaw16bitLookupData[ 0 ];
				else
					{
					if ( nPixelValue <= nModalityLUTLength )
						nPixelValue = pRaw16bitLookupData[ nPixelValue ];
					else
						nPixelValue = pRaw16bitLookupData[ nModalityLUTLength - 1 ];
					}
				// Restore the modified pixel into the image buffer.
				*p16BitPixel = nPixelValue;
				p16BitPixel++;
				}
			}
		else
			{
			RescaleSlope = m_pImageCalibrationInfo -> RescaleSlope;
			if ( RescaleSlope == 0.0 )
				RescaleSlope = 1.0;
			RescaleIntercept = m_pImageCalibrationInfo -> RescaleIntercept;
			for ( nPixel = 0; nPixel < nPixelCount; nPixel++ )
				{
				if ( m_pImageCalibrationInfo -> bPixelValuesAreSigned )
					{
					ps16BitPixel = (short*)p16BitPixel;
					fPixelValue = (double)(*ps16BitPixel);
					}
				else
					fPixelValue = (double)(*p16BitPixel);
				fPixelValue = Bias + fPixelValue;

				if ( bModalityRescaleIsSpecified && !bFujiPhotoWindowingException )
					fPixelValue = ( ( fPixelValue * RescaleSlope ) + RescaleIntercept + 0.5 );
				if ( fPixelValue < 0.0 )
					fPixelValue = 0.0;
				else if ( fPixelValue >= MaxPixelValue )
					fPixelValue = MaxPixelValue - 1;

				*p16BitPixel = (unsigned short)fPixelValue;
				p16BitPixel++;
				}
			}
		}
}


void CDiagnosticImage::ApplyVOI_LUT()
{
	BOOL				bNoError = TRUE;
	unsigned long		nPixel;
	size_t				nPixelCount;
	unsigned char		*p8BitPixel;
	unsigned short		*p16BitPixel;
	unsigned short		nPixelValue;
	unsigned short		nLUTIndex;
	unsigned char		n8BitPixelValue;
	BOOL				bVOI_LUTIsSpecified;
	double				WindowWidth;
	double				WindowCenter;
	unsigned char		*pRaw8bitLookupData;
	unsigned short		*pRaw16bitLookupData;
	size_t				nVOI_LUTLength;
	double				ObservedMaxPixelValue;
	double				MaxPixelValue;
	double				RescaleRatio;
	double				RescalePowersOfTwo;
	int					ExtraRescaleBits;
	int					nBitShift;
	unsigned short		PixelMask;
	unsigned short		PixelBitsSet;
	unsigned short		ProcessedPixelBitsSet;
	unsigned short		MaxObservedPixelValue;
	unsigned short		MinObservedPixelValue;
	unsigned short		MaxRawPixelValue;
	unsigned short		MinRawPixelValue;
	int					nBit;
	unsigned char		VOI_Threshold8BitValue;
	double				Bias;
	double				VOI_LUTThresholdPixelValue;
	short				*pVOI_LUTThresholdPixelValue;
	BOOL				bPixelIsBelowLookupThreshold;
	BOOL				bFujiPhotoWindowingException;

	nPixelCount = m_ImageWidthInPixels * m_ImageHeightInPixels;
	nVOI_LUTLength = m_pImageCalibrationInfo -> VOI_LUTElementCount;
	if ( nVOI_LUTLength == 0 )
		nVOI_LUTLength = 65536;
	bVOI_LUTIsSpecified = ( ( m_pImageCalibrationInfo -> SpecifiedCalibrationTypes & CALIBRATION_TYPE_VOI_LUT ) != 0 );
	nBitShift = m_pImageCalibrationInfo -> BitsAllocated - ( m_nHighBit + 1 );
	// Compensate for the possibility that the presence of modality rescaling causes the
	// maximum pixel value to exceed BitsStored.
	PixelMask = 0;
	for ( nBit = 0; nBit <= m_nHighBit; nBit++ )
		PixelMask |= ( 1 << nBit );
	if ( m_pImageCalibrationInfo -> bPixelValuesAreSigned )
		Bias = (double)( 1 << ( 15 ) );
	else
		Bias = 0.0;

	if ( bNoError && m_ImageBitDepth <= 8 )			// Process 8-bit pixels.
		{
		p8BitPixel = m_pImageData;
		pRaw8bitLookupData = (unsigned char*)m_pImageCalibrationInfo -> pVOI_LUTData;
		pRaw16bitLookupData = (unsigned short*)m_pImageCalibrationInfo -> pVOI_LUTData;
		if ( bVOI_LUTIsSpecified )
			{
			if ( m_pImageCalibrationInfo -> VOI_LUTBitDepth == 8 )
				VOI_Threshold8BitValue = (unsigned char)m_pImageCalibrationInfo -> VOI_LUTThresholdPixelValue;
			else
				VOI_Threshold8BitValue = *( (unsigned char*)&m_pImageCalibrationInfo -> VOI_LUTThresholdPixelValue + 1 );
			for ( nPixel = 0; nPixel < nPixelCount; nPixel++ )
				{
				n8BitPixelValue = *p8BitPixel;
				n8BitPixelValue -= VOI_Threshold8BitValue;
				if ( m_pImageCalibrationInfo -> VOI_LUTBitDepth == 8 )
					{
					if ( n8BitPixelValue <= 0 )
						n8BitPixelValue = pRaw8bitLookupData[ 0 ];
					else if ( (unsigned short)n8BitPixelValue <= nVOI_LUTLength )
						n8BitPixelValue = pRaw8bitLookupData[ n8BitPixelValue ];
					else
						n8BitPixelValue = pRaw8bitLookupData[ nVOI_LUTLength - 1 ];
					}
				else
					{
					if ( n8BitPixelValue <= 0 )
						n8BitPixelValue = *( (unsigned char*)&pRaw16bitLookupData[ 0 ] + 1 );
					else if ( (unsigned short)n8BitPixelValue <= nVOI_LUTLength )
						n8BitPixelValue = *( (unsigned char*)&pRaw16bitLookupData[ n8BitPixelValue ] + 1 );
					else
						n8BitPixelValue = *( (unsigned char*)&pRaw16bitLookupData[ nVOI_LUTLength - 1 ] + 1 );
					}
				if ( m_pImageCalibrationInfo -> PhotometricInterpretation == PMINTERP_MONOCHROME1 )
					*p8BitPixel = 255 - n8BitPixelValue;
				else
					*p8BitPixel = n8BitPixelValue;
				p8BitPixel++;
				}
			}
		// Enable windowing, even if there is a VOI-LUT, in which case BRetriever will have
		// set default, wide-open windowing values.
		MaxPixelValue = 255.0;
		WindowWidth = m_pImageCalibrationInfo -> WindowWidth;
		WindowCenter = m_pImageCalibrationInfo -> WindowCenter;
		if ( nBitShift > 0 )
			{
			WindowWidth *= pow( (double)2, nBitShift );
			WindowCenter *= pow( (double)2, nBitShift );
			}
		if ( WindowWidth == 0 || bVOI_LUTIsSpecified )
			{
			WindowWidth = MaxPixelValue;
			WindowCenter = MaxPixelValue / 2;
			}
		WindowCenter = Bias + WindowCenter;

		m_CurrentGrayscaleSetting.m_WindowCenter = WindowCenter;
		m_CurrentGrayscaleSetting.m_WindowWidth = WindowWidth;
		m_MaxGrayscaleValue = (unsigned long)MaxPixelValue;
		m_pImageCalibrationInfo -> WindowWidth = WindowWidth;
		m_pImageCalibrationInfo -> WindowCenter = WindowCenter;
		}
	else if ( bNoError )			// Process 16-bit pixels.
		{
		// The FUJIFILM Corporation manufacturer is modified if ImageProcessingModificationFlag private element is present.
		bFujiPhotoWindowingException = ( _strnicmp( m_pImageCalibrationInfo -> Manufacturer, "FUJI PHOTO", 10 ) == 0 &&  _stricmp( m_pImageCalibrationInfo -> Modality, "CR" ) == 0 );
		p16BitPixel = (unsigned short*)m_pImageData;
		pRaw16bitLookupData = (unsigned short*)m_pImageCalibrationInfo -> pVOI_LUTData;
		// Provide for compensation if the image values have been left shifted to align with
		// a 16-bit image.
		PixelBitsSet = 0;
		ProcessedPixelBitsSet = 0;
		MaxObservedPixelValue = 0;
		MinObservedPixelValue = 0xffff;
		MaxRawPixelValue = 0;
		MinRawPixelValue = 0xffff;
		if ( bVOI_LUTIsSpecified )
			{
			if ( m_pImageCalibrationInfo -> bPixelValuesAreSigned )
				{
				pVOI_LUTThresholdPixelValue = (short*)&m_pImageCalibrationInfo -> VOI_LUTThresholdPixelValue;
				VOI_LUTThresholdPixelValue = (double)(*pVOI_LUTThresholdPixelValue);
				}
			else
				VOI_LUTThresholdPixelValue = (double)m_pImageCalibrationInfo -> VOI_LUTThresholdPixelValue;
			VOI_LUTThresholdPixelValue = (unsigned short)( ( Bias / pow( (double)2, nBitShift ) ) + VOI_LUTThresholdPixelValue );

			for ( nPixel = 0; nPixel < nPixelCount; nPixel++ )
				{
				nPixelValue = *p16BitPixel;
				PixelBitsSet |= nPixelValue;
				if ( nPixelValue > MaxRawPixelValue )
					MaxRawPixelValue = nPixelValue;
				if ( nPixelValue < MinRawPixelValue )
					MinRawPixelValue = nPixelValue;
				bPixelIsBelowLookupThreshold = nPixelValue <= (unsigned short)VOI_LUTThresholdPixelValue;
				nLUTIndex = nPixelValue - (unsigned short)VOI_LUTThresholdPixelValue;
				// Look up the adjusted pixel value.
				if ( bPixelIsBelowLookupThreshold )
					nPixelValue = pRaw16bitLookupData[ 0 ];
				else
					{
					if ( nLUTIndex < nVOI_LUTLength )
						nPixelValue = ( pRaw16bitLookupData[ nLUTIndex ] );
					else
						nPixelValue = pRaw16bitLookupData[ nVOI_LUTLength - 1 ];
					}
				if ( nPixelValue > MaxObservedPixelValue )
					MaxObservedPixelValue = nPixelValue;
				if ( nPixelValue < MinObservedPixelValue )
					MinObservedPixelValue = nPixelValue;
				ProcessedPixelBitsSet |= nPixelValue;
				*p16BitPixel = nPixelValue;
				p16BitPixel++;
				}
			m_MinObservedPixelValue = MinObservedPixelValue;
			m_MaxObservedPixelValue = MaxObservedPixelValue;
			// Since some manufacturers require pixel shifting following the processing and others don't, rather
			// than have separate algorithms for each manufacturer, just shift all the pixels as necessary to
			// get non-zero contributions in bit 15.  Disable any shifting prior to this.
			MaxPixelValue = 65535.0;

			ObservedMaxPixelValue = ProcessedPixelBitsSet;
			// Calculate the log ( base 2 ) of the ratio.
			RescaleRatio = ( MaxPixelValue + 1 ) / ( ObservedMaxPixelValue + 1 );
			RescalePowersOfTwo = log( RescaleRatio ) / log( 2.0 );
			ExtraRescaleBits = (int)RescalePowersOfTwo;
			nBitShift = ExtraRescaleBits;
			if ( nBitShift > 0 )
				{
				p16BitPixel = (unsigned short*)m_pImageData;
				for ( nPixel = 0; nPixel < nPixelCount; nPixel++ )
					{
					nPixelValue = *p16BitPixel;
					nPixelValue <<= nBitShift;
					*p16BitPixel = nPixelValue;
					p16BitPixel++;
					}
				}

			if ( m_pImageCalibrationInfo -> PhotometricInterpretation == PMINTERP_MONOCHROME1 )
				{
				p16BitPixel = (unsigned short*)m_pImageData;
				for ( nPixel = 0; nPixel < nPixelCount; nPixel++ )
					{
					nPixelValue = *p16BitPixel;
					nPixelValue = ( ~nPixelValue ); 
					*p16BitPixel = nPixelValue;
					p16BitPixel++;
					}
				}
			}
		else
			{
			p16BitPixel = (unsigned short*)m_pImageData;
			for ( nPixel = 0; nPixel < nPixelCount; nPixel++ )
				{
				nPixelValue = *p16BitPixel;
				nPixelValue <<= nBitShift;
				if ( m_pImageCalibrationInfo -> PhotometricInterpretation == PMINTERP_MONOCHROME1 )
					nPixelValue = ~nPixelValue; 
				*p16BitPixel = nPixelValue;
				p16BitPixel++;
				}
			}
		// Enable windowing, even if there is a VOI-LUT, in which case BRetriever will have
		// set default, wide-open windowing values.
		MaxPixelValue = 65535.0;
		WindowWidth = m_pImageCalibrationInfo -> WindowWidth;
		WindowCenter = m_pImageCalibrationInfo -> WindowCenter;
		if ( nBitShift > 0 )
			{
			WindowWidth *= pow( (double)2, nBitShift );
			WindowCenter *= pow( (double)2, nBitShift );
			}
		if ( WindowWidth == 0 || bVOI_LUTIsSpecified || _stricmp( m_pImageCalibrationInfo -> Modality, "CT" ) == 0 || bFujiPhotoWindowingException )
			{
			WindowWidth = MaxPixelValue;
			WindowCenter = MaxPixelValue / 2;
			}
		WindowCenter = Bias + WindowCenter;

		m_CurrentGrayscaleSetting.m_WindowCenter = WindowCenter;
		m_CurrentGrayscaleSetting.m_WindowWidth = WindowWidth;
		m_MaxGrayscaleValue = (unsigned long)MaxPixelValue;
		m_pImageCalibrationInfo -> WindowWidth = WindowWidth;
		m_pImageCalibrationInfo -> WindowCenter = WindowCenter;
		}
}


// This function assumes that the modality and VOI conversions have normalized the
// 16-bit image to have a full 16-bit grayscale depth.  Therefore no shifting has
// to be done.
void CDiagnosticImage::ReduceTo8BitGrayscale()
	{
	unsigned long		nPixel;
	size_t				nPixelCount;
	unsigned char		*p8BitPixel;
	unsigned short		*p16BitPixel;
	unsigned short		PixelValue;
	unsigned char		*pPixelValue;

	nPixelCount = m_ImageWidthInPixels * m_ImageHeightInPixels;
	p16BitPixel = (unsigned short*)m_pImageData;
	p8BitPixel = (unsigned char*)m_pImageData;
	for ( nPixel = 0; nPixel < nPixelCount; nPixel++ )
		{
		PixelValue = *p16BitPixel;
		pPixelValue = ( (unsigned char*)&PixelValue ) + 1;
		*p8BitPixel = *pPixelValue;
		p8BitPixel++;
		p16BitPixel++;
		}
	m_ImageBitDepth = 8;
	m_nBitsAllocated = 8;
	m_nBitsStored = 8;
	m_nHighBit = 7;
}


// If the input image is too large to fit in the available texture memory, downsample
// it to a lower resolution.
void CDiagnosticImage::DownSampleImageResolution()
{
	unsigned long		nInputRow;
	unsigned long		nInputPixel;
	size_t				ReducedImageSize;
	unsigned char		*pCurrentRow;
	unsigned char		*pNextRow;
	unsigned char		*pOutputImage;
	unsigned char		*pOutputPixel;
	unsigned short		*pCurrentWordRow;
	unsigned short		*pNextWordRow;
	unsigned short		*pOutputWordPixel;
	unsigned long		OutputImageWidthInPixels;
	unsigned long		OutputImageHeightInPixels;
	
	OutputImageWidthInPixels = m_ImageWidthInPixels / 2;
	OutputImageHeightInPixels = m_ImageHeightInPixels / 2;
	ReducedImageSize = OutputImageWidthInPixels * OutputImageHeightInPixels;
	if ( m_ImageBitDepth > 8 )
		ReducedImageSize *= 2;
	pOutputImage = (unsigned char*)malloc( ReducedImageSize );
	if ( pOutputImage != 0 )
		{
		if ( m_ImageBitDepth <= 8 )
			{
			pCurrentRow = m_pImageData;
			pOutputPixel = pOutputImage;
			for ( nInputRow = 0; nInputRow < 2 * OutputImageHeightInPixels; nInputRow += 2 )
				{
				pNextRow = &pCurrentRow[ m_ImageWidthInPixels ];
				for ( nInputPixel = 0; nInputPixel < 2 * OutputImageWidthInPixels; nInputPixel += 2 )
					{
					*pOutputPixel = ( pCurrentRow[ nInputPixel ] + pCurrentRow[ nInputPixel + 1 ] +
										pNextRow[ nInputPixel ] + pNextRow[ nInputPixel + 1 ] ) / 4;
					pOutputPixel++;
					}
				pCurrentRow += 2 * m_ImageWidthInPixels;
				}
			}
		else
			{
			pCurrentWordRow = (unsigned short*)m_pImageData;
			pOutputWordPixel = (unsigned short*)pOutputImage;
			for ( nInputRow = 0; nInputRow < 2 * OutputImageHeightInPixels; nInputRow += 2 )
				{
				pNextWordRow = &pCurrentWordRow[ m_ImageWidthInPixels ];
				for ( nInputPixel = 0; nInputPixel < 2 * OutputImageWidthInPixels; nInputPixel += 2 )
					{
					*pOutputWordPixel = ( pCurrentWordRow[ nInputPixel ] + pCurrentWordRow[ nInputPixel + 1 ] +
										pNextWordRow[ nInputPixel ] + pNextWordRow[ nInputPixel + 1 ] ) / 4;
					pOutputWordPixel++;
					}
				pCurrentWordRow += 2 * m_ImageWidthInPixels;
				}
			}
		free( m_pImageData );
		m_ImageWidthInPixels = OutputImageWidthInPixels;
		m_ImageHeightInPixels = OutputImageHeightInPixels;
		m_ScaleFactor *= 2;
		m_FocalPoint.x /= 2;
		m_FocalPoint.y /= 2;
		m_pImageData = pOutputImage;
		LogMessage( "    The image resolution was reduced 50 percent to enable fitting in limited graphics memory.", MESSAGE_TYPE_SUPPLEMENTARY );		// *[2] Replaced sprintf() with LogMessage.
		}

	m_bImageHasBeenDownSampled = TRUE;
}


// If the input image is 8-bit pixels packed into 16-bit words, eliminate the alpha bytes.
void CDiagnosticImage::ReducePixelsToEightBits()
{
	unsigned long		nInputRow;
	unsigned long		nInputPixel;
	size_t				ReducedImageSize;
	unsigned short		*pCurrentRow;
	unsigned short		InputPixel;
	unsigned char		*pOutputImage;
	unsigned char		*pOutputPixel;
	
	if ( !m_bImageHasBeenCompacted && m_ImageBitDepth == 8 )
		{
		ReducedImageSize = m_ImageWidthInPixels * m_ImageHeightInPixels;
		pOutputImage = (unsigned char*)malloc( ReducedImageSize );
		if ( pOutputImage != 0 )
			{
			pCurrentRow = (unsigned short*)m_pImageData;
			pOutputPixel = pOutputImage;
			for ( nInputRow = 0; nInputRow < m_ImageHeightInPixels; nInputRow++ )
				{
				for ( nInputPixel = 0; nInputPixel < m_ImageWidthInPixels; nInputPixel++ )
					{
					InputPixel = pCurrentRow[ nInputPixel ];
					*pOutputPixel = (unsigned char)InputPixel;
					pOutputPixel++;
					}
				pCurrentRow += m_ImageWidthInPixels;
				}
			free( m_pImageData );
			m_pImageData = pOutputImage;
			LogMessage( "    The image was compacted to 8-bit bytes.", MESSAGE_TYPE_SUPPLEMENTARY );				// *[2] Replaced sprintf() with LogMessage.
			}
		}

	m_bImageHasBeenCompacted = TRUE;
}


#pragma pack(push)
#pragma pack(16)		// Pack structure members on 16-byte boundaries for faster access.
#include "png.h"
#pragma pack(pop)

BOOL ReadPNGFileHeader( FILE *pImageFile, IMAGE_CALIBRATION_INFO **ppImageCalibrationInfo )
{
	BOOL					bNoError = TRUE;
	BOOL					bOriginalCalibrationHeader;
	BOOL					bBViewer11mCalibrationHeader;
	size_t					CalibrationHeaderSizeInBytes;
	unsigned char			PNGSignature[ 8 ];
	long					nBytesRead;
	size_t					BufferSize;
	int						SystemErrorNumber;
	int						Result;							// *[2] Added for error check.

#pragma pack(push)
#pragma pack(1)		// Pack calibration structure members on 1-byte boundaries.
	IMAGE_CALIBRATION_INFO	*pImageCalibrationInfo = 0;		// *[2] Initialize pointer	
#pragma pack(pop)

	// Check if the first 8 bytes constitute a valid PNG file signature.
	bOriginalCalibrationHeader = FALSE;
	bBViewer11mCalibrationHeader = FALSE;
	nBytesRead = (long)fread_s( PNGSignature, 8, 1, 8, pImageFile );					// *[2] Converted from fread to fread_s.
	if ( nBytesRead != 8 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_IMAGE, IMAGE_ERROR_IMAGE_READ );
		SystemErrorNumber = ferror( pImageFile );
		if ( SystemErrorNumber != 0 )
			LogMessage( strerror( SystemErrorNumber ), MESSAGE_TYPE_ERROR );
		}
	else
		{
		bOriginalCalibrationHeader = ( memcmp( PNGSignature, "BVCALIBR", 8 ) == 0 );
		if ( bOriginalCalibrationHeader )
			CalibrationHeaderSizeInBytes = sizeof(IMAGE_CALIBRATION_INFO) - 64;
		bBViewer11mCalibrationHeader = ( memcmp( PNGSignature, "BVCALIB2", 8 ) == 0 );
		if ( bBViewer11mCalibrationHeader )
			CalibrationHeaderSizeInBytes = sizeof(IMAGE_CALIBRATION_INFO);
		}
	 if ( bOriginalCalibrationHeader || bBViewer11mCalibrationHeader )
		{
		// Read the image calibration information off the top.
		Result = fseek( pImageFile, -8L, SEEK_CUR );	// Reposition to the file before the label.   *[2] Get result of fseek() call.
		if ( Result != 0 )								// *[2] Check for error.
			bNoError = FALSE;							// *[2]
		if ( bNoError )
			{
			pImageCalibrationInfo = (IMAGE_CALIBRATION_INFO*)malloc( CalibrationHeaderSizeInBytes );
			if ( ppImageCalibrationInfo != 0 )
				*ppImageCalibrationInfo = pImageCalibrationInfo;
			if ( pImageCalibrationInfo != 0 )
				{
				nBytesRead = (long)fread_s( (char*)pImageCalibrationInfo, sizeof(IMAGE_CALIBRATION_INFO), 1, CalibrationHeaderSizeInBytes, pImageFile );	// *[2] Converted from fread to fread_s.
				bNoError = ( nBytesRead == CalibrationHeaderSizeInBytes );
				if ( bNoError )
					{
					BufferSize = (size_t)pImageCalibrationInfo -> ModalityLUTDataBufferSize;								// *[1]
					if ( BufferSize > 0 && BufferSize < 4000000 )															// *[1] Added a check for max buffer size.
						{
						pImageCalibrationInfo -> pModalityLUTData = malloc( BufferSize );									// *[1] Used the size_t data type for buffer allocation.
						if ( pImageCalibrationInfo -> pModalityLUTData != 0 )
							{
							nBytesRead = (long)fread_s( pImageCalibrationInfo -> pModalityLUTData, BufferSize, 1,
														pImageCalibrationInfo -> ModalityLUTDataBufferSize, pImageFile );	// *[2] Converted from fread to fread_s.
							bNoError = ( nBytesRead == pImageCalibrationInfo -> ModalityLUTDataBufferSize );
							}
						else
							{
							RespondToError( MODULE_IMAGE, IMAGE_ERROR_INSUFFICIENT_MEMORY );
							bNoError = FALSE;
							}
						}
					}
				else
					{
					RespondToError( MODULE_IMAGE, IMAGE_ERROR_IMAGE_READ );
					SystemErrorNumber = ferror( pImageFile );
					if ( SystemErrorNumber != 0 )
						LogMessage( strerror( SystemErrorNumber ), MESSAGE_TYPE_ERROR );
					}
				if ( bNoError )
					{
					BufferSize = (size_t)pImageCalibrationInfo -> VOI_LUTDataBufferSize;								// *[1]
					if ( BufferSize > 0 && BufferSize < 4000000 )														// *[1] Added a check for max buffer size.
						{
						pImageCalibrationInfo -> pVOI_LUTData = malloc( BufferSize );									// *[1] Used the size_t data type for buffer allocation.
						if ( pImageCalibrationInfo -> pVOI_LUTData != 0 )
							{
							nBytesRead = (long)fread_s( pImageCalibrationInfo -> pVOI_LUTData, BufferSize, 1,
														pImageCalibrationInfo -> VOI_LUTDataBufferSize, pImageFile );	// *[2] Converted from fread to fread_s.
							bNoError = ( nBytesRead == pImageCalibrationInfo -> VOI_LUTDataBufferSize );
							if ( !bNoError )
								{
								RespondToError( MODULE_IMAGE, IMAGE_ERROR_IMAGE_READ );
								SystemErrorNumber = ferror( pImageFile );
								if ( SystemErrorNumber != 0 )
									LogMessage( strerror( SystemErrorNumber ), MESSAGE_TYPE_ERROR );
								}
							}
						else
							{
							RespondToError( MODULE_IMAGE, IMAGE_ERROR_INSUFFICIENT_MEMORY );
							bNoError = FALSE;
							}
						}
					}
				if ( bNoError )
					{
					nBytesRead = (long)fread_s( PNGSignature, 8, 1, 8, pImageFile );									// *[2] Converted from fread to fread_s.
					if ( nBytesRead != 8 )
						{
						bNoError = FALSE;
						RespondToError( MODULE_IMAGE, IMAGE_ERROR_IMAGE_READ );
						SystemErrorNumber = ferror( pImageFile );
						if ( SystemErrorNumber != 0 )
							LogMessage( strerror( SystemErrorNumber ), MESSAGE_TYPE_ERROR );
						}
					}
				}
			else
				{
				RespondToError( MODULE_IMAGE, IMAGE_ERROR_INSUFFICIENT_MEMORY );
				bNoError = FALSE;
				}
			}
		}
	if ( bNoError && png_sig_cmp( PNGSignature, 0, 8 ) != 0 )
		{
		RespondToError( MODULE_IMAGE, IMAGE_ERROR_FILE_IS_NOT_PNG );
		bNoError = FALSE;
		}
	// If the calibration data is not going to be used, delete it.
	if ( ppImageCalibrationInfo == 0 && pImageCalibrationInfo != 0 )
		{
		if ( pImageCalibrationInfo -> pVOI_LUTData != 0 )
			free( pImageCalibrationInfo -> pVOI_LUTData );
		if ( pImageCalibrationInfo -> pModalityLUTData != 0 )
			free( pImageCalibrationInfo -> pModalityLUTData );
		free( pImageCalibrationInfo );
		Result = fseek( pImageFile, -8L, SEEK_CUR );	// Reposition before the PNG signature.   *[2] Get result of fseek() call.
		if ( Result != 0 )								// *[2] Check for error.
			bNoError = FALSE;							// *[2]
		}

	return bNoError;
}


BOOL CDiagnosticImage::ReadPNGImageFile( char *pFileSpec, MONITOR_INFO *pDisplayMonitor, unsigned long ImageContentType )
{
	BOOL					bNoError = TRUE;
	CGraphicsAdapter		*pGraphicsAdapter;
	FILE					*pImageFile;
	unsigned int			sig_read = 0;
	unsigned long			ImageRowBytes;
    png_bytep				*pRowPointers = 0;
	int						ColorType;
	int						nRow;
	char					Msg[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					TextLine[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					SuggestionMsg[ FULL_FILE_SPEC_STRING_LENGTH ];

#pragma pack(push)
#pragma pack(16)		// Pack structure members on 16-byte boundaries for faster access.
	png_struct		*pPngConfig = 0;			// *[2] Initialize pointer.
	png_info		*pPngImageInfo = 0;			// *[2] Initialize pointer.
#pragma pack(pop)

	SuggestionMsg[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
	pGraphicsAdapter = (CGraphicsAdapter*)pDisplayMonitor -> m_pGraphicsAdapter;
	if ( ImageContentType == IMAGE_FRAME_FUNCTION_STANDARD )
		pImageFile = OpenILOStandardImageFile( pFileSpec );
	else
		pImageFile = fopen( pFileSpec, "rb" );
	if ( pImageFile == 0 )
		{
		strncpy_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "This ", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		switch ( ImageContentType )
			{
			case IMAGE_FRAME_FUNCTION_PATIENT:
				strncat_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "subject study", _TRUNCATE );															// *[3] Replaced strcat with strncat_s.
				strncpy_s( SuggestionMsg, FULL_FILE_SPEC_STRING_LENGTH, "Try deleting this study and\nhaving it resent.\n", _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
				break;
			case IMAGE_FRAME_FUNCTION_STANDARD:
				strncat_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "standard", _TRUNCATE );																// *[3] Replaced strcat with strncat_s.
				strncpy_s( SuggestionMsg, FULL_FILE_SPEC_STRING_LENGTH, "Try reinstalling your ILO standard image files.\n", _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
				break;
			case IMAGE_FRAME_FUNCTION_REPORT:
				strncat_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "report", _TRUNCATE );																// *[3] Replaced strcat with strncat_s.
				strncpy_s( SuggestionMsg, FULL_FILE_SPEC_STRING_LENGTH, "Click on the \"Show Report\" button to create the report.\n", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
				break;
			}
		strncat_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, " image file could not be opened.\n\n", _TRUNCATE );											// *[3] Replaced strcat with strncat_s.
		if ( ImageContentType == IMAGE_FRAME_FUNCTION_STANDARD )
			strncat_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "The \"Demo\" image is being substituted.\n", _TRUNCATE );								// *[3] Replaced strcat with strncat_s.
		LogMessage( Msg, MESSAGE_TYPE_ERROR );
		ThisBViewerApp.NotifyUserOfImageFileError( IMAGE_ERROR_FILE_OPEN, Msg, SuggestionMsg );
		bNoError = FALSE;
		sprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, ">>> Unable to open %s for reading.", pFileSpec );											// *[1] Replaced sprintf with sprintf_s.
		LogMessage( Msg, MESSAGE_TYPE_ERROR );
		}
	else
		{
		bNoError = ReadPNGFileHeader( pImageFile, &m_pImageCalibrationInfo );
		}
	if ( bNoError )
		{
		// Create and initialize the png_struct with the desired error handler
		// functions.  If you want to use the default stderr and longjump method,
		// you can supply NULL for the last three parameters.  We also supply the
		// the compiler header file version, so that we know if the application
		// was compiled with a compatible version of the library.
		pPngConfig = png_create_read_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );
		if ( pPngConfig == 0 )
			{
			RespondToError( MODULE_IMAGE, IMAGE_ERROR_INSUFFICIENT_MEMORY );
			bNoError = FALSE;
			}
		}
	if ( bNoError )
		{
		// Allocate/initialize the memory for image information.
		pPngImageInfo = png_create_info_struct( pPngConfig );
		if ( pPngImageInfo == 0 )
			{
			RespondToError( MODULE_IMAGE, IMAGE_ERROR_INSUFFICIENT_MEMORY );
			bNoError = FALSE;
			png_destroy_read_struct( &pPngConfig, png_infopp_NULL, png_infopp_NULL );
			}
		}
	if ( bNoError )
		{
		// Set error handling if you are using the setjmp/longjmp method (this is
		// the normal method of doing things with libpng).  setjmp() must be called
		// in every function that calls a PNG-reading libpng function.
		if ( setjmp( png_jmpbuf( pPngConfig ) ) )
			{
			// If we get here, we had a problem reading the file.
			RespondToError( MODULE_IMAGE, IMAGE_ERROR_IMAGE_READ );
			// Free all of the memory associated with the pPngConfig and pPngImageInfo.
			png_destroy_read_struct( &pPngConfig, &pPngImageInfo, png_infopp_NULL );
			bNoError = FALSE;
			}
		}
	if ( bNoError )
		{
		// Set up the input control.
		png_init_io( pPngConfig, pImageFile );
		// Indicate that the first 8 bytes have already been read as a signature.
		png_set_sig_bytes( pPngConfig, 8 );
		// Read all of the PNG information that precedes the image data.
		png_read_info( pPngConfig, pPngImageInfo );
		// Expose some of the formatting information.
		png_get_IHDR( pPngConfig, pPngImageInfo, &m_ImageWidthInPixels, &m_ImageHeightInPixels, &m_ImageBitDepth, &ColorType, NULL, NULL, NULL );
		m_nBitsAllocated = (unsigned short)m_ImageBitDepth;			// *[1] Forced data type conversion.
		m_PixelsPerMillimeter = (double)m_ImageWidthInPixels / ( m_ActualImageWidthInInches * 25.4 );
		m_MaxGrayscaleValue = ( 1 << m_ImageBitDepth );

		sprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "Read new image file %s:\n", pFileSpec );								// *[1] Replaced sprintf with sprintf_s.
		sprintf_s( TextLine, FULL_FILE_SPEC_STRING_LENGTH, "                        Bit depth: %d,  W: %d  H: %d pixels",
															m_ImageBitDepth, m_ImageWidthInPixels, m_ImageHeightInPixels );	// *[1] Replaced sprintf with sprintf_s.
		strncat_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, TextLine, _TRUNCATE );												// *[3] Replaced strcat with strncat_s.
		switch( ColorType )
			{
			case PNG_COLOR_TYPE_GRAY:
				m_ImageColorFormat = GL_RED;
				m_SamplesPerPixel = 1;
				strncat_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "    Color Type = simple luminance.", _TRUNCATE );			// *[3] Replaced strcat with strncat_s.
				break;
			case PNG_COLOR_TYPE_GRAY_ALPHA:
				m_ImageColorFormat = GL_RG;
				m_SamplesPerPixel = 2;
				strncat_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "    Color Type = luminance alpha.", _TRUNCATE );				// *[3] Replaced strcat with strncat_s.
				break;
			case PNG_COLOR_TYPE_PALETTE:
				m_ImageColorFormat = GL_COLOR_INDEX;
				m_SamplesPerPixel = 1;
				strncat_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "    Color Type = color index.", _TRUNCATE );					// *[3] Replaced strcat with strncat_s.
			case PNG_COLOR_TYPE_RGB:
				m_ImageColorFormat = GL_RGB;
				m_SamplesPerPixel = 3;
				strncat_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "    Color Type = RGB.", _TRUNCATE );							// *[3] Replaced strcat with strncat_s.
				break;
			case PNG_COLOR_TYPE_RGB_ALPHA:
				m_ImageColorFormat = GL_RGBA;
				m_SamplesPerPixel = 4;
				strncat_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "    Color Type = RGBA.", _TRUNCATE );						// *[3] Replaced strcat with strncat_s.
				break;
			default:
				m_ImageColorFormat = GL_RGB;
				m_SamplesPerPixel = 3;
				strncat_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "    Color Type = other:  Default to RGB.", _TRUNCATE );		// *[3] Replaced strcat with strncat_s.
				break;
			}
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		if ( pPngImageInfo -> bit_depth > 8 )
			{
			png_set_swap( pPngConfig );
			// During reading, transform any 16-bit images to 8 bits per pixel if displaying via a
			// graphics card that either doesn't support OpenGL or supports an OpenGL version < 3.30.
			if ( pGraphicsAdapter -> m_OpenGLSupportLevel != OPENGL_SUPPORT_330 )
				m_bConvertImageTo8BitGrayscale = TRUE;
			}
		// If any transformations are to be performed on reading, they should be designated
		// before the following function call:
		png_read_update_info( pPngConfig, pPngImageInfo );

		ImageRowBytes = png_get_rowbytes( pPngConfig, pPngImageInfo );
		png_get_channels( pPngConfig, pPngImageInfo );						// *[1] Removed unused return value assignment.
		m_pImageData = (unsigned char*)malloc( ImageRowBytes * m_ImageHeightInPixels );
		if ( m_pImageData == 0 )
			{
			RespondToError( MODULE_IMAGE, IMAGE_ERROR_INSUFFICIENT_MEMORY );
			// Free all of the memory associated with the pPngConfig and pPngImageInfo.
			png_destroy_read_struct( &pPngConfig, &pPngImageInfo, png_infopp_NULL );
			bNoError = FALSE;
			}
		}
	if ( bNoError )
		{
		pRowPointers = (png_bytep*)malloc( m_ImageHeightInPixels * sizeof( png_bytep ) );
		if ( pRowPointers == 0 )
			{
			RespondToError( MODULE_IMAGE, IMAGE_ERROR_INSUFFICIENT_MEMORY );
			// Free all of the memory associated with the pPngConfig and pPngImageInfo.
			png_destroy_read_struct( &pPngConfig, &pPngImageInfo, png_infopp_NULL );
			free( m_pImageData );
			m_pImageData = 0;
			bNoError = FALSE;
			}
		}
	if ( bNoError )
		{
		// Set up the row pointers to point into the image buffer.
		for ( nRow = 0;  nRow < (int)m_ImageHeightInPixels;  nRow++ )
			// Read the image in flipped vertically, since that is the way OpenGL prefers it.
			pRowPointers[ nRow ] = m_pImageData + ( ( (int)m_ImageHeightInPixels - nRow - 1 ) * ImageRowBytes );
		// Read the image into memory.
		png_read_image( pPngConfig, pRowPointers );

		free( pRowPointers );
		pRowPointers = 0;
		png_destroy_read_struct( &pPngConfig, &pPngImageInfo, png_infopp_NULL );
		}
	if ( pImageFile != 0 )
		{
		fclose( pImageFile );
		if ( !bNoError )
			ThisBViewerApp.NotifyUserOfImageFileError( IMAGE_ERROR_IMAGE_READ, "This image file\ncould not be deciphered.\n\n", SuggestionMsg );
		}
	if ( bNoError && m_pImageCalibrationInfo != 0 )
		{
		AnalyzeImagePixels();
		ApplyModalityLUT();
		ApplyVOI_LUT();
		// The following function can be called if there is a need to extract the raw,
		// uncompressed image from the image buffer after VOI_LUT, etc., have been
		// applied.  The raw image will be written to a ".raw" file in the folder
		// where the image ".png" file of the same file name is saved:  ...\Images
		// The image file is prefixed by 3 unsigned long values, total image bytes,
		// pixel columns and pixel rows.  The image is recorded upside-down.  The
		// Bits Stored value is 16.
//		bNoError = ExtractUncompressedImageToFile( pFileSpec );
		}
	if ( !bNoError )
		{
		sprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "Error reading new image file %s:  Bit depth = %d.", pFileSpec, m_ImageBitDepth );	// *[1] Replaced sprintf with sprintf_s.
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		}
	if ( m_bConvertImageTo8BitGrayscale )
		{
		LogMessage( "    Convert image to 8-bit grayscale for limited functionality display.", MESSAGE_TYPE_SUPPLEMENTARY );
		ReduceTo8BitGrayscale();
		}

	return bNoError;
}


BOOL CDiagnosticImage::WritePNGImageFile( char *pFileSpec )
{
	BOOL					bNoError = TRUE;
	FILE					*pOutputImageFile;
	long					nImageOutputBitDepth;
	long					nImagePixelsPerRow;
	long					nImageBytesPerRow;
	long					nImageRows;
	long					nRow;
	long					nImageRow;
	char					Msg[ FULL_FILE_SPEC_STRING_LENGTH ];

#pragma pack(push)
#pragma pack(16)		// Pack structure members on 16-byte boundaries for faster access.
	png_struct				*pPngConfig;
	png_info				*pPngImageInfo;
	png_color_8				PngSignificantBits;
#pragma pack(pop)
	unsigned char			**pRows;

	pRows = 0;
	pPngConfig = 0;
	pPngImageInfo = 0;
	pOutputImageFile = fopen( pFileSpec, "wb" );
	if ( pOutputImageFile == 0 )
		{
		ThisBViewerApp.NotifyUserOfImageFileError( IMAGE_ERROR_FILE_OPEN, "An error occurred opening a\nreport file for saving.\n\n", "" );
		bNoError = FALSE;
		sprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, ">>> Unable to open %s for saving.", pFileSpec );	// *[1] Replaced sprintf with sprintf_s.
		LogMessage( Msg, MESSAGE_TYPE_ERROR );
		}
	else
		{
		nImagePixelsPerRow = (long)m_OutputImageWidthInPixels;
		nImageBytesPerRow = nImagePixelsPerRow * 3;
		nImageOutputBitDepth = 8;
		// Create an array of pointers to the image pixel rows to be input by the PNG conversion.
		pRows = (unsigned char**)malloc( m_OutputImageHeightInPixels * sizeof(char*) );
		if ( pRows == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_IMAGE, IMAGE_ERROR_INSUFFICIENT_MEMORY );
			}
		else
			{
			for( nRow = 0; nRow < (long)m_OutputImageHeightInPixels; nRow++ )
				{
				nImageRow = m_OutputImageHeightInPixels - nRow - 1;
				pRows[ nRow ] = (unsigned char*)( m_pOutputImageData + ( nImageRow * nImageBytesPerRow ) );
				}
			}
		nImageRows = (long)m_OutputImageHeightInPixels;
		}
	if ( bNoError )
		{

		// Create and initialize the png_struct with the desired error handler
		// functions.  If you want to use the default stderr and longjump method,
		// you can supply NULL for the last three parameters.
		pPngConfig = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
		if ( pPngConfig == NULL )
			{
			RespondToError( MODULE_IMAGE, IMAGE_ERROR_INSUFFICIENT_MEMORY );
			fclose( pOutputImageFile );
			pOutputImageFile = 0;
			bNoError = FALSE;
			}
		}
	if ( bNoError )
		{
		// Allocate/initialize the image information data.
		pPngImageInfo = png_create_info_struct( pPngConfig );
		if ( pPngImageInfo == NULL )
			{
			RespondToError( MODULE_IMAGE, IMAGE_ERROR_INSUFFICIENT_MEMORY );
			fclose( pOutputImageFile );
			pOutputImageFile = 0;
			png_destroy_write_struct( &pPngConfig,  png_infopp_NULL );
			bNoError = FALSE;
			}
		}
	if ( bNoError )
		{
		// Set error handling if you aren't supplying your own
		// error handling functions in the png_create_write_struct() call.
		if ( setjmp( png_jmpbuf( pPngConfig ) ) )
			{
			// If we get here, we had a problem reading the file.
			RespondToError( MODULE_IMAGE, IMAGE_ERROR_IMAGE_WRITE );
			fclose( pOutputImageFile );
			pOutputImageFile = 0;
			png_destroy_write_struct( &pPngConfig, &pPngImageInfo );
			bNoError = FALSE;
			}
		}
	if ( bNoError )
		{
		// Set up the output control for using standard C streams.
		png_init_io( pPngConfig, pOutputImageFile );
		// Set the image information here.  Width and height are up to 2^31,
		// bit_depth is one of 1, 2, 4, 8, or 16.
		png_set_IHDR( pPngConfig, pPngImageInfo, nImagePixelsPerRow, nImageRows, nImageOutputBitDepth,
						PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );
		// Write the file header information.
		png_write_info( pPngConfig, pPngImageInfo );

		PngSignificantBits.alpha = 0;
		PngSignificantBits.blue = 8;
		PngSignificantBits.green = 8;
		PngSignificantBits.red = 8;
		PngSignificantBits.gray = 0;
		// Create an output chunk to indicate the original image grayscale bit depth.
		png_set_sBIT( pPngConfig, pPngImageInfo, &PngSignificantBits );
		}
	png_write_rows( pPngConfig, (png_bytepp)pRows, m_OutputImageHeightInPixels );
	if ( pOutputImageFile != 0 )
		png_write_end( pPngConfig, pPngImageInfo );
	// Clean up after the write, and free any memory allocated.
	if ( pPngConfig != 0 && pPngImageInfo != 0 )
		png_destroy_write_struct( &pPngConfig, &pPngImageInfo );
	if ( pOutputImageFile != 0 )
		fclose( pOutputImageFile );
	if ( pRows != 0 )
		free( pRows );

	return bNoError;
}

BOOL CDiagnosticImage::ExtractUncompressedImageToFile( char *pFileSpec )
{
	BOOL					bNoError = TRUE;
	char					FileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	FILE					*pOutputImageFile;
	char					Msg[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					*pExtension;
	unsigned short			*p16BitPixel;
	unsigned long			nPixel;
	unsigned long			ImageSizeInBytes;
	size_t					nPixelCount;
	size_t					nBytesToWrite;
	size_t					nBytesWritten;
	double					GammaValue;
	double					GammaCorrectedValue;

	strncpy_s( FileSpec, FULL_FILE_SPEC_STRING_LENGTH, pFileSpec, _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
	pExtension = strrchr( FileSpec, '.' );
	if ( pExtension != 0 )
		*pExtension = '\0';
	strncat_s( FileSpec, FULL_FILE_SPEC_STRING_LENGTH, ".raw", _TRUNCATE );								// *[3] Replaced strcat with strncat_s.
	pOutputImageFile = fopen( FileSpec, "wb" );
	if ( pOutputImageFile == 0 )
		{
		ThisBViewerApp.NotifyUserOfImageFileError( IMAGE_ERROR_FILE_OPEN, "An error occurred opening a\nraw image file for saving.\n\n", "" );
		bNoError = FALSE;
		sprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, ">>> Unable to open %s for saving.", FileSpec );	// *[1] Replaced sprintf with sprintf_s.
		LogMessage( Msg, MESSAGE_TYPE_ERROR );
		}
	else
		{
		p16BitPixel = (unsigned short*)m_pImageData;
		nPixelCount = m_ImageWidthInPixels * m_ImageHeightInPixels;
		ImageSizeInBytes = nPixelCount * 2;
		nBytesToWrite = sizeof( unsigned long);
		nBytesWritten = 0;
		nBytesWritten += fwrite( &ImageSizeInBytes, 1, nBytesToWrite, pOutputImageFile );
		nBytesWritten += fwrite( &m_ImageWidthInPixels, 1, nBytesToWrite, pOutputImageFile );
		nBytesWritten += fwrite( &m_ImageHeightInPixels, 1, nBytesToWrite, pOutputImageFile );
		bNoError = ( nBytesWritten == 3 * sizeof(unsigned long) );
		GammaValue = 0.7;
		for ( nPixel = 0; nPixel < nPixelCount && bNoError; nPixel++ )
			{
			nBytesToWrite = 2;

			GammaCorrectedValue = 65536.0 * pow( (double)( *p16BitPixel ) / 65536.0, GammaValue );
			if ( GammaCorrectedValue < 0.0 )
				GammaCorrectedValue = 0.0;
			else if ( GammaCorrectedValue > 65535.0 )
				GammaCorrectedValue = 65535.0;
			*p16BitPixel = (unsigned short)GammaCorrectedValue;

			nBytesWritten = fwrite( p16BitPixel, 1, nBytesToWrite, pOutputImageFile );
			bNoError = ( nBytesWritten == nBytesToWrite );
			p16BitPixel++;
			}
		fclose( pOutputImageFile );
		}
	if ( !bNoError )
		{
		sprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, ">>> Error writing to raw image output file %s.", FileSpec );	// *[1] Replaced sprintf with sprintf_s.
		LogMessage( Msg, MESSAGE_TYPE_ERROR );
		}

	return bNoError;
}

