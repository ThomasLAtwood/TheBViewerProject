// ServiceInterface.h : Header file for interfacing with the operating system's
//	Service Control Manager (SCM).
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

#define SERVICE_INTERFACE_ERROR_OPEN_SCM				1
#define SERVICE_INTERFACE_ERROR_INSTALL_SERVICE			2
#define SERVICE_INTERFACE_ERROR_OPEN_SERVICE			3
#define SERVICE_INTERFACE_ERROR_START_SERVICE			4
#define SERVICE_INTERFACE_ERROR_STOP_SERVICE			5
#define SERVICE_INTERFACE_ERROR_CHECK_SERVICE			6
#define SERVICE_INTERFACE_ERROR_DELETE_SERVICE			7
#define SERVICE_INTERFACE_ERROR_SECURITY				8

#define SERVICE_INTERFACE_ERROR_DICT_LENGTH				8



// Function prototypes.
//
void				InitServiceInterfaceModule();
void				CloseServiceInterfaceModule();

BOOL				CheckServiceSecurity( SC_HANDLE hService );
BOOL				InstallTheService();
BOOL				StartTheService();
BOOL				CheckTheService();
BOOL				StopTheService();
BOOL				RemoveTheService();


