// Calibration.h - Defines the functions and data structures that extract the 
// image calibration data from the image file and package it for reading by
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
#pragma once


// This structure specifies any grayscale image calibration parameters included within the Dicom
// information in the image file.  The contents of this structure are loaded by BRetriever from
// the Dicom header information in the image file.  This info is passed on to BViewer within the
// extracted image file.
typedef struct
	{
	char					Label[ 8 ];		// Set to "BVCALIBR" for file read recognition.
											// Starting with BViewer 1.1m this is changed to
											// "BVCALIB2" to indicate an enlarged structure
											// containing the Manufacturer name.
	unsigned long			SpecifiedCalibrationTypes;
								#define CALIBRATION_TYPE_NONE_PRESENT			0x00000000
								// If the following is specified, then the CALIBRATION_TYPE_MODALITY_LUT
								// type must not be present and vice-versa.  Whichever is present, it
								// must be applied before any VOI calibration.
								#define CALIBRATION_TYPE_MODALITY_RESCALE		0x00000001
								// Modality look-up table.
								#define CALIBRATION_TYPE_MODALITY_LUT			0x00000002
								// Only one of the following two VOI calibrations may be applied.  If
								// more than one is present, they represent alternatives.  The VOI calibration
								// is to be applied following any modality calibration.  The
								// CALIBRATION_TYPE_VOI_WINDOW type specifies a window-center/window-width
								// rescaling.
								#define CALIBRATION_TYPE_VOI_WINDOW				0x00000004
								// VOI (Value Of Interest) look-up table.
								#define CALIBRATION_TYPE_VOI_LUT				0x00000008
								#define CALIBRATION_ACTIVE_MODALITY_LUT			0x00000020
								#define CALIBRATION_ACTIVE_VOI_LUT				0x00000080
	unsigned long			PhotometricInterpretation;
								#define PMINTERP_UNSPECIFIED					0x00000000
								// Pixel data represent a single monochrome image plane. The minimum sample
								// value is intended to be displayed as white after any VOI gray scale
								// transformations have been performed. This value may be used only when
								// Samples per Pixel (0028,0002) has a value of 1. 
								#define PMINTERP_MONOCHROME1					0x00000001
								// Pixel data represent a single monochrome image plane. The minimum sample
								// value is intended to be displayed as black after any VOI gray scale
								// transformations have been performed. This value may be used only when
								// Samples per Pixel (0028,0002) has a value of 1.
								#define PMINTERP_MONOCHROME2					0x00000002
	unsigned short			ImageRows;
	unsigned short			ImageColumns;
	unsigned short			BitsAllocated;
	unsigned short			BitsStored;					// The upper byte is HighBit.
	char					Modality[ 2 ];
	BOOL					bPixelValuesAreSigned;
	double					RescaleIntercept;
	double					RescaleSlope;
	char					ModalityOutputUnits[ 64 ];	// "OD":  LUT number / 1000.0 is optical density.  "HU" is Hounsfield Units (for CT).
														// "US" is unspecified.
														// This field applies to either the rescale or the LUT calibration type,
														// whichever is present.
	unsigned long			ModalityLUTElementCount;
	unsigned short			ModalityLUTThresholdPixelValue;		// This value and any lower values in the image are mapped
																// to the first value in the LUT.
	unsigned short			ModalityLUTBitDepth;				// Either 8 or 16.
	unsigned long			ModalityLUTDataBufferSize;
	void					*pModalityLUTData;			// Pointer to a memory buffer containing the LUT data.
	double					WindowCenter;
	double					WindowWidth;
	unsigned long			WindowFunction;
								#define WINDOW_FUNCTION_NOT_SPECIFIED		0
								#define WINDOW_FUNCTION_LINEAR				1
								#define WINDOW_FUNCTION_SIGMOID				2
	unsigned long			VOI_LUTElementCount;
	unsigned short			VOI_LUTThresholdPixelValue;			// This value and any lower values in the image are mapped
																// to the first value in the LUT.
	unsigned short			VOI_LUTBitDepth;					// Either 8 or 16.
	unsigned long			VOI_LUTDataBufferSize;
	char					Manufacturer[ 64 ];
	void					*pVOI_LUTData;			// Pointer to a memory buffer containing the LUT data.
	} IMAGE_CALIBRATION_INFO;

