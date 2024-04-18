// Calibration.cpp - Implements the functionality for extracting the 
// image calibration data from the image file and packaging it for reading by
// BViewer.
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
//	*[1] 03/12/2024 by Tom Atwood
//		Fixed security issues.
//
//
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif


#include <math.h>
#include "Module.h"
#include "ReportStatus.h"
#include "Dicom.h"

//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO					ImageCalibrateModuleInfo = { MODULE_CALIBRATE, "Image Calibrate Module", InitImageCalibrateModule, CloseImageCalibrateModule };


static ERROR_DICTIONARY_ENTRY	ImageCalibrateErrorCodes[] =
			{
				{ CALIBRATE_ERROR_LUT_FORMAT				, "A format error was encountered in decoding the Dicom LUT descriptor." },
				{ 0											, NULL }
			};


static ERROR_DICTIONARY_MODULE		ImageCalibrateStatusErrorDictionary =
										{
										MODULE_CALIBRATE,
										ImageCalibrateErrorCodes,
										CALIBRATE_ERROR_DICT_LENGTH,
										0
										};


// This function must be called before any other function in this module.
void InitImageCalibrateModule()
{
	LinkModuleToList( &ImageCalibrateModuleInfo );
	RegisterErrorDictionary( &ImageCalibrateStatusErrorDictionary );
}


void CloseImageCalibrateModule()
{
}


void LoadImageCalibrationData( void *pDicomElementStructure, void *pDicomHeaderStructure )
{
	DICOM_ELEMENT			*pDicomElement;
	DICOM_HEADER_SUMMARY	*pDicomHeader;
	size_t					LUTBufferSize;
	int						nBitCount;
	unsigned short			HighBit;

	pDicomElement = (DICOM_ELEMENT*)pDicomElementStructure;
	pDicomHeader = (DICOM_HEADER_SUMMARY*)pDicomHeaderStructure;
	if ( pDicomElement -> Tag.Group == 0x0008 && pDicomElement -> Tag.Element == 0x0060 )
		{
		pDicomHeader -> CalibrationInfo.Modality[ 0 ] = pDicomElement -> pConvertedValue[ 0 ];
		pDicomHeader -> CalibrationInfo.Modality[ 1 ] = pDicomElement -> pConvertedValue[ 1 ];
		}
	if ( pDicomElement -> Tag.Group == 0x0008 && pDicomElement -> Tag.Element == 0x0070 )
		{
		 pDicomHeader -> CalibrationInfo.Manufacturer[0] = '\0';														// *[1] Eliminate call to strcpy.
		strncat_s( pDicomHeader -> CalibrationInfo.Manufacturer, 64, pDicomElement -> pConvertedValue, _TRUNCATE );		// *[1] Replaced strncat with strncat_s.
		}
	if ( pDicomElement -> Tag.Group == 0x50F1 && pDicomElement -> Tag.Element == 0x1020 )
		{
		// Modify FUJIFILM Corporation manufacturer if ImageProcessingModificationFlag private element is present.
		if ( _stricmp( pDicomHeader -> CalibrationInfo.Manufacturer, "FUJIFILM Corporation" ) == 0 )
			{
			strncat_s( pDicomHeader -> CalibrationInfo.Manufacturer, 64, "_", _TRUNCATE );								// *[1] Replaced strcat with strncat_s.
			strncat_s( pDicomHeader -> CalibrationInfo.Manufacturer, 64, pDicomElement -> pConvertedValue, 2 );			// *[1] Replaced strncat with strncat_s.
			}
		}
	if ( pDicomElement -> Tag.Group == 0x0028 )
		switch ( pDicomElement -> Tag.Element )
			{
			case 0x0004:		// PhotometricInterpretation
				if ( _stricmp( pDicomElement -> pConvertedValue, "MONOCHROME1" ) == 0 )
					pDicomHeader -> CalibrationInfo.PhotometricInterpretation = PMINTERP_MONOCHROME1;
				else if ( _stricmp( pDicomElement -> pConvertedValue, "MONOCHROME2" ) == 0 )
					pDicomHeader -> CalibrationInfo.PhotometricInterpretation = PMINTERP_MONOCHROME2;
				else if ( _stricmp( pDicomElement -> pConvertedValue, "RGB" ) == 0 )
					pDicomHeader -> CalibrationInfo.PhotometricInterpretation = PMINTERP_RGB;
				else
					pDicomHeader -> CalibrationInfo.PhotometricInterpretation = PMINTERP_UNSPECIFIED;
				break;
			case 0x0010:		// Rows
				pDicomHeader -> CalibrationInfo.ImageRows = *( (unsigned short*)pDicomElement -> pConvertedValue);		// *[1] Recast to eliminate data type mismatch.
				break;
			case 0x0011:		// Columns
				pDicomHeader -> CalibrationInfo.ImageColumns = *( (unsigned short*)pDicomElement -> pConvertedValue);	// *[1] Recast to eliminate data type mismatch.
				break;
			case 0x0100:		// BitsAllocated
				pDicomHeader -> CalibrationInfo.BitsAllocated = *( (unsigned short*)pDicomElement -> pConvertedValue);	// *[1] Recast to eliminate data type mismatch.
				if ( pDicomHeader -> CalibrationInfo.BitsAllocated > 16 )
					{
					pDicomHeader -> CalibrationInfo.BitsAllocated = 16;
					*( (short*)pDicomElement -> pConvertedValue) = 16;
					*pDicomHeader -> BitsAllocated = (unsigned short)16;
					}
				break;
			case 0x0101:		// BitsStored
				pDicomHeader -> CalibrationInfo.BitsStored = *( (unsigned short*)pDicomElement -> pConvertedValue);		// *[1] Recast to eliminate data type mismatch.
				if ( pDicomHeader -> CalibrationInfo.BitsStored > 16 )
					{
					pDicomHeader -> CalibrationInfo.BitsStored = 16;
					*( (short*)pDicomElement -> pConvertedValue) = 16;
					*pDicomHeader -> BitsStored = (unsigned short)16;
					}
				break;
			case 0x0102:		// HighBit
				HighBit = *( (unsigned short*)pDicomElement -> pConvertedValue);										// *[1] Recast to eliminate data type mismatch.
				pDicomHeader -> CalibrationInfo.BitsStored |= ( HighBit << 8 ) & 0xFF00;
				break;
			case 0x0103:		// PixelRepresentation
				pDicomHeader -> CalibrationInfo.bPixelValuesAreSigned = ( *( (short*)pDicomElement -> pConvertedValue) != 0 );
				break;

			case 0x1050:		// WindowCenter
				pDicomHeader -> CalibrationInfo.WindowCenter = atof( pDicomElement -> pConvertedValue );
				pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes |= CALIBRATION_TYPE_VOI_WINDOW;
				break;
			case 0x1051:		// WindowWidth
				pDicomHeader -> CalibrationInfo.WindowWidth = atof( pDicomElement -> pConvertedValue );
				pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes |= CALIBRATION_TYPE_VOI_WINDOW;
				break;

			case 0x1052:		// RescaleIntercept
				pDicomHeader -> CalibrationInfo.RescaleIntercept = atof( pDicomElement -> pConvertedValue );
				pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes |= CALIBRATION_TYPE_MODALITY_RESCALE;
				break;
			case 0x1053:		// RescaleSlope
				pDicomHeader -> CalibrationInfo.RescaleSlope = atof( pDicomElement -> pConvertedValue );
				pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes |= CALIBRATION_TYPE_MODALITY_RESCALE;
				break;
			case 0x1054:		// RescaleType
				strcpy_s( pDicomHeader -> CalibrationInfo.ModalityOutputUnits, "" );
				strncat_s( pDicomHeader -> CalibrationInfo.ModalityOutputUnits, pDicomElement -> pConvertedValue, 63 );
				pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes |= CALIBRATION_TYPE_MODALITY_RESCALE;
				break;

			case 0x1056:		// VOILUTFunction
				if ( _stricmp( pDicomElement -> pConvertedValue, "LINEAR" ) == 0 )
					pDicomHeader -> CalibrationInfo.WindowFunction |= WINDOW_FUNCTION_LINEAR;
				else if ( _stricmp( pDicomElement -> pConvertedValue, "SIGMOID" ) == 0 )
					pDicomHeader -> CalibrationInfo.WindowFunction |= WINDOW_FUNCTION_SIGMOID;
				else
					pDicomHeader -> CalibrationInfo.WindowFunction = WINDOW_FUNCTION_NOT_SPECIFIED;
				pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes |= CALIBRATION_TYPE_VOI_WINDOW;
				break;

			case 0x3000:		// ModalityLUTSequence
				// Set the flag that indicates that any following LUT information relates to
				// the modality LUT.
				pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes |= CALIBRATION_ACTIVE_MODALITY_LUT;
				pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes &= ~CALIBRATION_ACTIVE_VOI_LUT;
				break;
			case 0x3010:		// VOILUTSequence
				// Set the flag that indicates that any following LUT information relates to
				// the VOI LUT.
				pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes &= ~CALIBRATION_ACTIVE_MODALITY_LUT;
				pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes |= CALIBRATION_ACTIVE_VOI_LUT;
				break;
			case 0x3002:		// LUTDescriptor
				if ( pDicomElement -> ValueRepresentation == US && pDicomElement -> ValueMultiplicity == 3 )
					{
					if ( pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes & CALIBRATION_ACTIVE_MODALITY_LUT )
						{
						pDicomHeader -> CalibrationInfo.ModalityLUTElementCount =
									(unsigned long)( *( (unsigned short*)pDicomElement -> pConvertedValue ) );
						pDicomHeader -> CalibrationInfo.ModalityLUTThresholdPixelValue =
									(unsigned short)( *( (unsigned short*)pDicomElement -> pConvertedValue + 1 ) );
						pDicomHeader -> CalibrationInfo.ModalityLUTBitDepth =
									(unsigned short)( *( (unsigned short*)pDicomElement -> pConvertedValue + 2 ) );
						pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes |= CALIBRATION_TYPE_MODALITY_LUT;
						}
					else if ( ( pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes & CALIBRATION_ACTIVE_VOI_LUT ) &&
																	pDicomHeader -> CalibrationInfo.pVOI_LUTData == 0 )		// If multiple VOI-LUT tables, only take the first one.
						{
						pDicomHeader -> CalibrationInfo.VOI_LUTElementCount =
									(unsigned long)( *( (unsigned short*)pDicomElement -> pConvertedValue ) );
						pDicomHeader -> CalibrationInfo.VOI_LUTThresholdPixelValue =
									(unsigned short)( *( (unsigned short*)pDicomElement -> pConvertedValue + 1 ) );
						pDicomHeader -> CalibrationInfo.VOI_LUTBitDepth =
									(unsigned short)( *( (unsigned short*)pDicomElement -> pConvertedValue + 2 ) );
						pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes |= CALIBRATION_TYPE_VOI_LUT;
						}
					}
				else
					{
					// Generate an error message.
					RespondToError( MODULE_CALIBRATE, CALIBRATE_ERROR_LUT_FORMAT );
					// Cancel the current LUT flag.
					if ( pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes & CALIBRATION_ACTIVE_MODALITY_LUT )
						pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes &= ~CALIBRATION_TYPE_MODALITY_LUT;
					else if ( pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes & CALIBRATION_ACTIVE_VOI_LUT )
						pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes &= ~CALIBRATION_TYPE_VOI_LUT;
					}
				break;
			case 0x3004:		// ModalityLUTType
				strcpy_s( pDicomHeader -> CalibrationInfo.ModalityOutputUnits, "" );
				strncat_s( pDicomHeader -> CalibrationInfo.ModalityOutputUnits, pDicomElement -> pConvertedValue, 63 );
				pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes |= CALIBRATION_TYPE_MODALITY_LUT;
				break;
			case 0x3006:		// LUTData
				if ( pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes & CALIBRATION_ACTIVE_MODALITY_LUT )
					{
					LUTBufferSize = pDicomElement -> ValueLength;
					pDicomHeader -> CalibrationInfo.ModalityLUTDataBufferSize = (unsigned long)LUTBufferSize;
					pDicomHeader -> CalibrationInfo.pModalityLUTData = malloc( LUTBufferSize );
					if ( pDicomHeader -> CalibrationInfo.pModalityLUTData != 0 )
						memcpy( pDicomHeader -> CalibrationInfo.pModalityLUTData, (char*)pDicomElement -> pConvertedValue, LUTBufferSize );
					}
				else if ( ( pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes & CALIBRATION_ACTIVE_VOI_LUT ) &&
																pDicomHeader -> CalibrationInfo.pVOI_LUTData == 0 )		// If multiple VOI-LUT tables, only take the first one.
					{
					LUTBufferSize = pDicomElement -> ValueLength;
					pDicomHeader -> CalibrationInfo.VOI_LUTDataBufferSize = (unsigned long)LUTBufferSize;
					pDicomHeader -> CalibrationInfo.pVOI_LUTData = malloc( LUTBufferSize );
					if ( pDicomHeader -> CalibrationInfo.pVOI_LUTData != 0 )
						memcpy( pDicomHeader -> CalibrationInfo.pVOI_LUTData, (char*)pDicomElement -> pConvertedValue, LUTBufferSize );
					if ( pDicomHeader -> CalibrationInfo.WindowWidth == 0 )
						{
						HighBit =  ( ( pDicomHeader -> CalibrationInfo.BitsStored >> 8 ) & 0x00FF );
						nBitCount = HighBit + 1;
						pDicomHeader -> CalibrationInfo.WindowWidth = pow( (double)2, nBitCount );
						pDicomHeader -> CalibrationInfo.WindowCenter = pDicomHeader -> CalibrationInfo.WindowWidth / 2.0;
						}
					}
				break;
			}
}


