/****************************************************************************
** $file: amanith/src/support/gutilities.cpp   0.2.0.0   edited Dec, 12 2005
**
** Global general purpose utilities.
**
**
** Copyright (C) 2004-2005 Mazatech Inc. All rights reserved.
**
** This file is part of Amanith Framework.
**
** This file may be distributed and/or modified under the terms of the Q Public License
** as defined by Mazatech Inc. of Italy and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Amanith Professional Edition license may use this file in
** accordance with the Amanith Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.mazatech.com or email sales@mazatech.com for
** information about Amanith Commercial License Agreements.
** See http://www.amanith.org/ for opensource version, public forums and news.
**
** Contact info@mazatech.com if any conditions of this licensing are
** not clear to you.
**********************************************************************/

#include "amanith/support/gutilities.h"

#include <cstdlib>  // for getenv function
#include <algorithm>
#include <cstring>
// now ensure includes for ScanPath function
#if defined(G_OS_WIN) && !defined(__CYGWIN__) && !defined(__GNUC__)
	#include <wbemidl.h>
	#include <comdef.h>
	#include <fcntl.h>
	#include <iostream>
	#include <fstream>
#elif defined(G_OS_IRIX)
	#include <sys/types.h>
	#include <sys/dir.h>
#else
    // standard unix and also MinGW for windows has opendir() function here
	#include <dirent.h>
#endif

// now ensure includes for FileExists function
#if defined(G_OS_WIN) && !defined(__CYGWIN__)
    // MinGW for windows has access() function in io.h
	#include <io.h>
#else
	// POSIX, UNIX and so on..
	#include <unistd.h> 
#endif

/*!
	\file gutilities.cpp
	\brief Implementation of generic utilities on strings, files and system information.
*/

namespace Amanith {

GBool SysUtils::gSysInfoExtracted = G_FALSE;
GSystemInfo SysUtils::gStaticSystemInfo;

// *********************************************************************
//                            System utilities
// *********************************************************************

#if defined(G_OS_WIN) && !defined(__CYGWIN__) && !defined(__GNUC__)
GError GetWindowsVersion(GInt32& Major, GInt32& Minor, GInt32& Build, GInt32& PlatformID,
						 GString& OSDescription) {

	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

   // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
   // If that fails, try using the OSVERSIONINFO structure.
   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

   if(!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *) &osvi))) {
      osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
      if (!GetVersionEx((OSVERSIONINFO *)&osvi)) 
			return G_UNKNOWN_ERROR;
   }

   PlatformID = osvi.dwPlatformId;
   Minor = osvi.dwMinorVersion;
   Major = osvi.dwMajorVersion;
   Build = osvi.dwBuildNumber;

	switch (osvi.dwPlatformId) {
		// test for the Windows NT product family.
		case VER_PLATFORM_WIN32_NT:
			// test for the specific product family.
			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
				OSDescription = "Microsoft Windows Server 2003 Family";
			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
				OSDescription = "Microsoft Windows XP";
			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
				OSDescription = "Microsoft Windows 2000";
			if (osvi.dwMajorVersion <= 4)
				OSDescription = "Microsoft Windows NT";

			// Test for specific product on Windows NT 4.0 SP6 and later.
			if (bOsVersionInfoEx) {
				// Test for the workstation type
				if (osvi.wProductType == VER_NT_WORKSTATION) {
					if (osvi.dwMajorVersion == 4)
						OSDescription += " Workstation 4.0";
					else
					if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
						OSDescription += " Home Edition";
					else
						OSDescription += " Professional";
            }
            // Test for the server type
            else
			if (osvi.wProductType == VER_NT_SERVER) {
				if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
					if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
						OSDescription += " Datacenter Edition";
					else
					if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						OSDescription += " Enterprise Edition";
					else
					if (osvi.wSuiteMask == VER_SUITE_BLADE)
						OSDescription += " Web Edition";
					else
						OSDescription += " Standard Edition";
				}
				else
				if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
					if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
						OSDescription += " Datacenter Server";
					else
					if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						OSDescription += " Advanced Server";
					else
						OSDescription += " Server";
				}
				else {  // Windows NT 4.0 
					if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						OSDescription += " Server 4.0, Enterprise Edition";
					else
						OSDescription += " Server 4.0";
				}
			}
		}

		// Test for the Windows 95 product family
		case VER_PLATFORM_WIN32_WINDOWS:
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) {
				OSDescription = "Microsoft Windows 95";
				if (osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B')
					OSDescription += " OSR2";
			}
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
				OSDescription = "Microsoft Windows 98";
				if (osvi.szCSDVersion[1] == 'A')
					OSDescription += " SE";
			}
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90) {
				OSDescription += "Microsoft Windows Millennium Edition";
			} 
			break;

		case VER_PLATFORM_WIN32s:
			OSDescription += "Microsoft Win32s";
			break;
   }
   return G_NO_ERROR; 
}
#endif

/*!
	\note It works only for Windows platform, and it uses WMI.
*/
GError SysUtils::MachineGUID(GString& Identifier) {

	#if defined(G_OS_WIN) && !defined(__CYGWIN__) && !defined(__GNUC__)

		GInt32 maj, min, build, platfID;
		GUInt32 i, j, k;
		GString osDesc, machineGuid;
		GUChar8 c1, c;
		char *strPath;
		ULONG uCount = 1, uReturned;
		_bstr_t bstrPath;
		HRESULT hRes;
		VARIANT v;
		IWbemClassObject * pClassObject = NULL;
		IWbemLocator * pIWbemLocator = NULL;
		IWbemServices * pWbemServices = NULL;
		IEnumWbemClassObject * pEnumObject  = NULL;
		GError err;

		// get os version
		err = GetWindowsVersion(maj, min, build, platfID, osDesc);
		if (err != G_NO_ERROR)
			return err;

		CoInitialize(NULL);
		// security needs to be initialized in XP first (even on NT kernels)
		// so we must check and care of this
		if (platfID == VER_PLATFORM_WIN32_NT) {
			hRes = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE,
										NULL, EOAC_NONE, 0);
			if (hRes != S_OK && hRes != RPC_E_TOO_LATE)
				return G_PERMISSION_DENIED;
		}

		BSTR bstrNamespace = (L"root\\cimv2");

		if (CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL ,
							CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IUnknown ,
							(void **)&pIWbemLocator) != S_OK)
			return G_PERMISSION_DENIED;

		if (pIWbemLocator->ConnectServer(bstrNamespace, NULL, NULL, NULL, 0, NULL, NULL, &pWbemServices) != S_OK)
			return G_PERMISSION_DENIED;

		// disk information
		BSTR strQuery = (L"Select Signature from win32_DiskDrive");
		BSTR strQL = (L"WQL");
		hRes = pWbemServices->ExecQuery(strQL, strQuery, WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumObject);
		if (hRes != S_OK)
			return G_PERMISSION_DENIED;
		hRes = pEnumObject->Reset();
		if (hRes != S_OK)
			return G_PERMISSION_DENIED;
		hRes = pEnumObject->Next(10000, uCount, &pClassObject, &uReturned);
		if (hRes != S_OK)
			return G_PERMISSION_DENIED;

		BSTR strClassProp = SysAllocString(L"Signature");
		while (hRes == S_OK) {
			hRes = pClassObject->Get(strClassProp, 0, &v, 0, 0);
			if (hRes != S_OK)
				return G_PERMISSION_DENIED;
			bstrPath = &v;
			strPath = (char*)bstrPath;
			machineGuid += GString(strPath);
			VariantClear(&v);
			hRes = pEnumObject->Next(10000, uCount, &pClassObject, &uReturned);
		}
		SysFreeString(strClassProp);

		// mac addresses
		strQuery = (L"Select MACAddress from win32_NetworkAdapter where AdapterTypeID=0");
		strQL = (L"WQL");
		hRes = pWbemServices->ExecQuery(strQL, strQuery, WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumObject);
		if (hRes != S_OK)
			return G_PERMISSION_DENIED;
		hRes = pEnumObject->Reset();
		if (hRes != S_OK)
			return G_PERMISSION_DENIED;
		hRes = pEnumObject->Next(10000, uCount, &pClassObject, &uReturned);
		if (hRes != S_OK)
			return G_PERMISSION_DENIED;
		strClassProp = SysAllocString(L"MACAddress");
		while (hRes == S_OK) {
			hRes = pClassObject->Get(strClassProp, 0, &v, 0, 0);
			if (hRes != S_OK)
				return G_PERMISSION_DENIED;
			bstrPath = &v;
			strPath = (char*)bstrPath;
			machineGuid += GString(strPath);
			VariantClear(&v);
			hRes = pEnumObject->Next(10000, uCount, &pClassObject, &uReturned);
		}
		SysFreeString(strClassProp);

		// mother board
		strQuery = (L"Select Product from win32_BaseBoard");
		strQL = (L"WQL");
		hRes = pWbemServices->ExecQuery(strQL, strQuery, WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumObject);
		if (hRes != S_OK)
			return G_PERMISSION_DENIED;
		hRes = pEnumObject->Reset();
		if (hRes != S_OK)
			return G_PERMISSION_DENIED;
		hRes = pEnumObject->Next(10000, uCount, &pClassObject, &uReturned);
		if (hRes != S_OK)
			return G_PERMISSION_DENIED;
		strClassProp = SysAllocString(L"Product");
		while (hRes == S_OK) {
			hRes = pClassObject->Get(strClassProp, 0, &v, 0, 0);
			if (hRes != S_OK)
				return G_PERMISSION_DENIED;
			bstrPath = &v;
			strPath = (char*)bstrPath;
			machineGuid += GString(strPath);
			VariantClear(&v);
			hRes = pEnumObject->Next(10000, uCount, &pClassObject, &uReturned);
		}
		SysFreeString(strClassProp);
			
		pIWbemLocator->Release();
		pWbemServices->Release();
		pEnumObject->Release();
		pClassObject->Release();
		CoUninitialize();

		// machine guid
		Identifier = "";

		j = (GUInt32)(16 - (machineGuid.length() % 16));
		for (i = 0; i < j; i++)
			machineGuid += GChar8(0);

		j = (GUInt32)(machineGuid.length()) / 16;
		for (i = 0; i < 16; i++) {
			c = 0;
			for (k = 0; k < j; k++) {
				c1 = machineGuid[k * 16 + i];
				c ^= c1;
			}
			Identifier += StrUtils::ToHex(c, 2);
			if ((i == 3) || (i == 5) || (i == 7) || (i == 9))
				Identifier += '-';
		}
		return G_NO_ERROR;

	#else
		Identifier = "";
		return G_MISSED_FEATURE;
	#endif
}

GBool SysUtils::SystemInfo(GSystemInfo& SysInfo) {

	GUInt32 wordSize;
    GBool be16, be32;

	if (gSysInfoExtracted) {
		SysInfo = gStaticSystemInfo;
		return G_TRUE;
    }

	// word size
    wordSize = 0;
    GULong n = (GULong)(~0);
    while (n) {
		wordSize++;
		n /= 2;
    }

	// byte order
    short ns = 0x1234;
    GInt32 nl = 0x12345678;

    unsigned char *p = (unsigned char *)(&ns);	// 16-bit integer
    be16 = *p == 0x12;
    p = (unsigned char *)(&nl);			// 32-bit integer
    if (p[0] == 0x12 && p[1] == 0x34 && p[2] == 0x56 && p[3] == 0x78)
		be32 = G_TRUE;
    else
    if (p[0] == 0x78 && p[1] == 0x56 && p[2] == 0x34 && p[3] == 0x12)
		be32 = G_FALSE;
    else
		be32 = !be16;
	// ensure correctness operation
    if ((be16 == be32) && ((wordSize == 64) || (wordSize == 32) || (wordSize == 16))) {
		gSysInfoExtracted = G_TRUE;
		gStaticSystemInfo.WordSize = wordSize;
		if (be32)
			gStaticSystemInfo.EndianType = G_BIG_ENDIAN;
		else
			gStaticSystemInfo.EndianType = G_LITTLE_ENDIAN;
		#if defined(G_OS_WIN) && !defined(__CYGWIN__)
			gStaticSystemInfo.TrailerPathDelimiter = '\\';
		#else
			gStaticSystemInfo.TrailerPathDelimiter = '/';
		#endif
		// dynamic library file extension
		#if defined(G_OS_WIN) && !defined(__CYGWIN__)
			gStaticSystemInfo.DynamicLibExtension = "dll";
		#elif defined(G_OS_MAC)
			gStaticSystemInfo.DynamicLibExtension = "dylib";
		#elif defined(G_OS_HPUX)
			gStaticSystemInfo.DynamicLibExtension = "sl";
		#elif defined(G_OS_SOLARIS)
			gStaticSystemInfo.DynamicLibExtension = "so";
		#elif defined(G_OS_IRIX)
			gStaticSystemInfo.DynamicLibExtension = "so";
		#else
			gStaticSystemInfo.DynamicLibExtension = "so";
		#endif

		SysInfo = gStaticSystemInfo;
		return G_TRUE;
	}
	return G_FALSE;
}

GString SysUtils::AmanithPath() {

#if defined(G_OS_WIN) && _MSC_VER >= 1400
	GChar8 *c = NULL;
	size_t bufSize;
	errno_t envErr = _dupenv_s(&c, &bufSize, "AMANITHDIR");
	if (!c || envErr)
		return GString("");
#else
	GChar8 *c = getenv("AMANITHDIR");
	if (!c)
		return GString("");
#endif

	GString s = GString(c);
#if defined(G_OS_WIN) && _MSC_VER >= 1400
	std::free(c);
#endif
	// fix path for current OS and insure a final path delimiter ('/' or '\' depending on OS)
	return StrUtils::OSFixPath(s, G_TRUE);
}

GString SysUtils::AmanithPluginsPath() {

	GString s = SysUtils::AmanithPath();
	if (s.length() <= 0)
		return GString("");
	s += "plugins";
	// fix path for current OS and insure a final path delimiter ('/' or '\' depending on OS)
	return StrUtils::OSFixPath(s, G_TRUE);
}

void SysUtils::RedirectIOToConsole() {

#if defined(G_OS_WIN)

	// maximum number of lines the output console should have
	static const WORD MAX_CONSOLE_LINES = 500;

	int hConHandle;
	HANDLE lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	std::FILE *fp;

	// allocate a console for this app
	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),	&coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),	coninfo.dwSize);
	// redirect unbuffered STDOUT to the console
	lStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);
	// redirect unbuffered STDIN to the console
	lStdHandle = GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "r");
	*stdin = *fp;
	setvbuf(stdin, NULL, _IONBF, 0);
	// redirect unbuffered STDERR to the console
	lStdHandle = GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	*stderr = *fp;
	setvbuf(stderr, NULL, _IONBF, 0);
	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
	std::ios::sync_with_stdio();
#endif
}

// *********************************************************************
//                            String utilities
// *********************************************************************

// returns lowered string
GString StrUtils::Lower(const GString& Str) {

	GString s(Str);
	transform(Str.begin(), Str.end(), s.begin(), tolower);
	return s;
}
// returns uppered string
GString StrUtils::Upper(const GString& Str) {

	GString s(Str);
	transform(Str.begin(), Str.end(), s.begin(), toupper);
	return s;
}
// returns a string that contains the 'HowMany' rightmost characters of the string 'Str'
GString StrUtils::Right(const GString& Str, const GUInt32 HowMany) {

	return Str.substr(Str.length() - HowMany);
}
// returns a substring that contains the 'HowMany' leftmost characters of the string 'Str'
GString StrUtils::Left(const GString& Str, const GUInt32 HowMany) {

	return Str.substr(0, HowMany);
}

GString StrUtils::Replace(const GString& Str, const GChar8 Before, const GChar8 After) {

	GString s(Str);
	std::replace(s.begin(), s.end(), Before, After);
	return s;
}

GBool StrUtils::Find(const GString& Source, const GString& StrToFind, const GBool CaseInsensitive,
					 const GUInt32 StartOffset, GUInt32 *PosFound) {

	GInt32 pos;

	if (CaseInsensitive) {

		GString src = StrUtils::Lower(Source);
		GString toFind = StrUtils::Lower(StrToFind);
		pos = (GInt32)src.find(toFind, StartOffset);
	}
	else
		pos = (GInt32)Source.find(StrToFind, StartOffset);

	if (pos < 0)
		return G_FALSE;
	else {
		if (PosFound)
			*PosFound = (GUInt32)pos;
		return G_TRUE;
	}
}

/*!
	Every slash present in the path, will be set according to the current OS type ('\' for Windows, '/' for *nix). Example:
\code
	// suppose we are on a Windows platform
	GString path = StrUtils::OSFixPath("c:\windows/system32", G_TRUE);
	// now path contains "c:\windows\system32\"
\endcode

	\param GeneralPath the path string that we wanna fix
	\param InsureFinalDelimiter if G_TRUE, return fixed path is ensured to include as last character a path delimiter
*/
GString StrUtils::OSFixPath(const GString& GeneralPath, const GBool InsureFinalDelimiter) {

	GSystemInfo sysInfo;
	GString res;

	if (SysUtils::SystemInfo(sysInfo)) {
		if (sysInfo.TrailerPathDelimiter == '/')
			res = StrUtils::Replace(GeneralPath, '\\', '/');
		else
			res = StrUtils::Replace(GeneralPath, '/', '\\');
		// we can insure that returned path ends with a TrailerPathDelimiter char
		if (InsureFinalDelimiter == G_TRUE) {
			const GChar8 lastChar = GeneralPath[GeneralPath.length() - 1];
			if (lastChar != sysInfo.TrailerPathDelimiter)
				res += sysInfo.TrailerPathDelimiter;
		}
		return res;
	}
	return GeneralPath;
}

/*!
	\note The returned string will have all slash characters set according to the current OS type ('\' for
	Windows, '/' for *nix).
	\sa Lets see the lowlevel_stuff example located at $AMANITHDIR/examples/others/lowlevel_stuff.
*/
GString StrUtils::ExtractFilePath(const GString& FullFileName) {

	GString tmpStr = Replace(FullFileName, '\\', '/');
	GInt32 pos = (GInt32)tmpStr.find_last_of('/');
	GString s = "";
	if (pos >= 0)
		s = Left(tmpStr, pos + 1);
	return StrUtils::OSFixPath(s);
}

/*!
	\sa Lets see the lowlevel_stuff example located at $AMANITHDIR/examples/others/lowlevel_stuff.
*/
GString StrUtils::ExtractFileName(const GString& FullFileName, const GBool IncludeExtension) {

	GString tmpStr = Replace(FullFileName, '\\', '/');
	GInt32 pos = (GInt32)tmpStr.find_last_of('/');
	GString s = FullFileName;
	if (pos >= 0)
		s = Right(tmpStr, (GUInt32)tmpStr.length() - pos - 1);
	// in this case we must cut away extension
	if (IncludeExtension == G_FALSE) {
		pos = (GInt32)s.find_first_of('.');
		if (pos >= 0)
			s = Left(s, pos);
	}
	return StrUtils::OSFixPath(s);
}

/*!
	\sa Lets see the lowlevel_stuff example located at $AMANITHDIR/examples/others/lowlevel_stuff.
*/
GString StrUtils::ExtractFileExt(const GString& FullFileName) {

	GString tmpStr = ExtractFileName(FullFileName);
	GInt32 pos = (GInt32)tmpStr.find_first_of('.');
	GString s = "";
	if (pos >= 0)
		s = Right(tmpStr, (GUInt32)tmpStr.length() - pos - 1);
	return s;
}

// case-insensitive string comparison
GBool StrUtils::SameText(const GString& Str1, const GString& Str2) {
	return (Upper(Str1) == Upper(Str2));
}

/*!
	Example:
\code
	GString s = StrUtils::Purge("username@"_[_$]amanith.org", "$_[]");
	// now s contains the string "username@amanith.org"
\endcode
*/
GString StrUtils::Purge(const GString& DustySource, const GString& Dust) {

	GString purged = GString("");
	GInt32 j = (GInt32)DustySource.length();

	if ((j <= 0) || (Dust.length() == 0))
		return purged;
	for (GInt32 i = 0; i < j; i++) {
		GChar8 ch = DustySource[i];
		// if a dusty char is not found, then add it to the purged string
		GInt32 p = (GInt32)Dust.find(ch, 0);
		if (p < 0)
			purged += ch;
	}
	return purged;
}

// convert an integer into string format
GString StrUtils::ToString(const GInt32 Value, const GChar8 *Format) {

	GChar8 buffer[16];

#if defined(G_OS_WIN) && _MSC_VER >= 1400
	if (!Format)
		sprintf_s(buffer, 16, "%d", Value);
	else
		sprintf_s(buffer, 16, Format, Value);
#else
	if (!Format)
		std::sprintf(buffer, "%d", Value);
	else
		std::sprintf(buffer, Format, Value);
#endif
	return GString(buffer);
}
// convert an integer into string format
GString StrUtils::ToString(const GUInt32 Value, const GChar8 *Format) {

	GChar8 buffer[16];

#if defined(G_OS_WIN) && _MSC_VER >= 1400
	if (!Format)
		sprintf_s(buffer, 16, "%d", Value);
	else
		sprintf_s(buffer, 16, Format, Value);
#else
	if (!Format)
		std::sprintf(buffer, "%d", Value);
	else
		std::sprintf(buffer, Format, Value);
#endif
	return GString(buffer);
}
// convert an integer into string format
GString StrUtils::ToString(const GInt16 Value, const GChar8 *Format) {

	GChar8 buffer[16];

#if defined(G_OS_WIN) && _MSC_VER >= 1400
	if (!Format)
		sprintf_s(buffer, 16, "%d", Value);
	else
		sprintf_s(buffer, 16, Format, Value);
#else
	if (!Format)
		std::sprintf(buffer, "%d", Value);
	else
		std::sprintf(buffer, Format, Value);
#endif
	return GString(buffer);
}
// convert an integer into string format
GString StrUtils::ToString(const GUInt16 Value, const GChar8 *Format) {

	GChar8 buffer[16];

#if defined(G_OS_WIN) && _MSC_VER >= 1400
	if (!Format)
		sprintf_s(buffer, 16, "%d", Value);
	else
		sprintf_s(buffer, 16, Format, Value);
#else
	if (!Format)
		std::sprintf(buffer, "%d", Value);
	else
		std::sprintf(buffer, Format, Value);
#endif
	return GString(buffer);
}
// convert an integer into string format
GString StrUtils::ToString(const GInt8 Value, const GChar8 *Format) {

	GChar8 buffer[16];
	
#if defined(G_OS_WIN) && _MSC_VER >= 1400
	if (!Format)
		sprintf_s(buffer, 16, "%d", Value);
	else
		sprintf_s(buffer, 16, Format, Value);
#else
	if (!Format)
		std::sprintf(buffer, "%d", Value);
	else
		std::sprintf(buffer, Format, Value);
#endif
	return GString(buffer);
}
// convert an integer into string format
GString StrUtils::ToString(const GUInt8 Value, const GChar8 *Format) {

	GChar8 buffer[16];

#if defined(G_OS_WIN) && _MSC_VER >= 1400
	if (!Format)
		sprintf_s(buffer, 16, "%d", Value);
	else
		sprintf_s(buffer, 16, Format, Value);
#else
	if (!Format)
		std::sprintf(buffer, "%d", Value);
	else
		std::sprintf(buffer, Format, Value);
#endif
	return GString(buffer);
}
// convert a float into string format
GString StrUtils::ToString(const GFloat Value, const GChar8 *Format) {

	GChar8 buffer[16];

#if defined(G_OS_WIN) && _MSC_VER >= 1400
	if (!Format)
		sprintf_s(buffer, 16, "%f", Value);
	else
		sprintf_s(buffer, 16, Format, Value);
#else
	if (!Format)
		std::sprintf(buffer, "%f", Value);
	else
		std::sprintf(buffer, Format, Value);
#endif
	return GString(buffer);
}
// convert a double into string format
GString StrUtils::ToString(const GDouble Value, const GChar8 *Format) {

	GChar8 buffer[64];

#if defined(G_OS_WIN) && _MSC_VER >= 1400
	if (!Format)
		sprintf_s(buffer, 64, "%f", Value);
	else
		sprintf_s(buffer, 64, Format, Value);
#else
	if (!Format)
		std::sprintf(buffer, "%f", Value);
	else
		std::sprintf(buffer, Format, Value);
#endif
	return GString(buffer);
}
// converts the contents of a string as a C-style, null-terminated string
const GChar8 *StrUtils::ToAscii(const GString& Value) {

	return Value.c_str();
}

// converts a string into an integer
GInt32 StrUtils::ToInt(const GString& Value) {

	return (GInt32)std::atoi(StrUtils::ToAscii(Value));
}

// converts a string (representing a number) into an long number
GLong StrUtils::ToLong(const GString& Value) {

	return std::atol(StrUtils::ToAscii(Value));
}

// converts a string into a double
GDouble StrUtils::ToDouble(const GString& Value) {

	return (GDouble)std::atof(StrUtils::ToAscii(Value));
}

/*!
	Example:
\code
	// s will contain the string "0x00007F"
	GString s = StrUtils::ToHex(127, 8);
	// r will contain the string "0x10000"
	GString r = StrUtils::ToHex(65536, 3);
	
\endcode
	\param Number the number to convert into hexadecimal string representation
	\param Width length of the output string;
	\note The output string will have a length of exactly Width, including '0x' prefix and extra characters
	will be set to '0'; just in case that hexadecimal representation needs more than Width characters, then the
	output string will have a length greater than Width.
*/
GString StrUtils::ToHex(const GUInt32 Number, const GUInt32 Width) {

	GChar8 buffer[16];
	GString s;
	GUInt32 i, j;

#if defined(G_OS_WIN) && _MSC_VER >= 1400
	sprintf_s(buffer, 16, "%X", Number);
#else
	std::sprintf(buffer, "%X", Number);
#endif

	s = buffer;
	j = (GUInt32)s.length();
	if (j < Width) {
		for (i = 0; i < Width - j; i++)
			s = '0' + s;
	}
	return s;
}

/*!
	Splits the string Value into strings wherever the Separator expression occurs, and returns the list of those strings. 
	If Empties is G_TRUE, an empty string is inserted in the list wherever the separator matches twice without
	intervening text. 
	For example, if you split the string "a,,b,c" on commas, Split() returns the three-item list "a", "b", "c" if
	Empties is G_FALSE (the default), and the four-item list "a", "", "b", "c" if Empties is G_TRUE. 
*/
GStringList StrUtils::Split(const GString& Value, const GString& Separator, GBool Empties) {

	GStringList strList;
	GString nullStr;
	GInt32 pos0, pos1, dlen;

	dlen = (GInt32)Separator.length();
	pos0 = 0;
	pos1 = (GInt32)Value.find(Separator, pos0);

	while (pos1 >= 0) {
		//GString s = Value.substr(pos0, pos1);
		GString s = Value.substr(pos0, pos1 - pos0);
		if ((s.length() > 0) || Empties)
			strList.push_back(s);
		pos0 = pos1 + dlen;
		pos1 = (GInt32)Value.find(Separator, pos0);
	}
	GString s = Value.substr(pos0);
	if ((s.length() > 0) || Empties)
		strList.push_back(s);
	return strList;
}

// merge all strings
GString StrUtils::Merge(const GStringList& InputList, const GChar8 *InterStr) {

	GString s, sep = InterStr;

	if (InputList.size() <= 0)
		return GString("");

	GStringList::const_iterator it = InputList.begin();
	s = *it;
	for (; it != InputList.end(); ++it)
		s += (sep + *it);
	return s;
}

/*!
	Example:
\code
	GError err;
	GVector3 v;
	// v will have components set to, respectively, X = 3, Y = 4.5, Z = 0
	err = StrUtils::FromString("3, 4.5, 0", v);
	// this call fails with an error code of G_INVALID_FORMAT, because of non-matching components number
	err = StrUtils::FromString("0, 1", v);
\endcode
*/

/*
template<GUInt32 SIZE>
GError StrUtils::FromString(const GString& SourceStr, GVectBase<GReal, SIZE>& Vect) {

	GVectBase<GReal, SIZE> vaux;
	GUInt32 i;
	GStringList::const_iterator it;
	// split components
	GStringList list = StrUtils::Split(',', SourceStr);

	// we MUST have SIZE components
	if (list.size() != SIZE)
		return G_INVALID_FORMAT;
	// extract each component
	i = 0;
	for (it = list.begin(); it != list.end(); ++it, ++i)
		vaux[i] = (GReal)StrUtils::ToDouble(*it);

	Vect = vaux;
	return G_NO_ERROR;
}*/

/*!
	Example:
\code
	GError err;
	GQuaternion q;
	// q will have components set to, respectively, X = 3, Y = 4.5, Z = 0, W = 1
	err = StrUtils::FromString("3, 4.5, 0, 1", q);
	// this call fails with an error code of G_INVALID_FORMAT, because of non-matching components number
	err = StrUtils::FromString("0, 1, 2, 5, 6", q);
\endcode
*/
GError StrUtils::FromString(const GString& SourceStr, GQuaternion& Quat) {

	GQuaternion qaux;
	GUInt32 i;
	GStringList::const_iterator it;
	// split components
	GStringList list = StrUtils::Split(SourceStr, ",", G_FALSE);

	// we MUST have SIZE components
	if (list.size() != 4)
		return G_INVALID_FORMAT;
	// extract each component
	i = 0;
	for (it = list.begin(); it != list.end(); ++it, i++)
		qaux[i] = (GReal)StrUtils::ToDouble(*it);

	Quat = qaux;
	return G_NO_ERROR;
}

// *********************************************************************
//                            File utilities
// *********************************************************************

/*!
	\param FullFileName the filename to check the existence of. It must be non NULL.
	\return G_TRUe if file exists, G_FALSE otherwise.
	\note To check the file existence it has been used the non-ANSI access() function. This function is present on Windows,
	SUNOS 5.4+, LINUX 1.1.46+ and conforming to SVID, AT&T, POSIX, X/OPEN, and BSD 4.3.
*/
GBool FileUtils::FileExists(const GChar8 *FullFileName) {

	if (!FullFileName || std::strlen(FullFileName) <= 0)
		return G_FALSE;
	// check existence mode
#if defined(G_OS_WIN) && _MSC_VER >= 1400
	if (_access(FullFileName, 0) == 0)
		return G_TRUE;
	return G_FALSE;
#else
	if (::access(FullFileName, 0) == 0)
		return G_TRUE;
	return G_FALSE;
#endif
}

/*!
	\param FoundFiles the files found during scanning
	\param Directory the path to scan, it must be non NULL.
	\param ScanRecursively if G_TRUE, the scan operation will be propagated recursively on all sub-directories
	\param AddPath if G_TRUE the full files names (complete with full path) will be appended into the output list. If
	G_FALSE path will not be included into strings.
	\param FileExt an optional filter, if non NULL only files that match this file extension will be added to the output
	list
	\note The passed output list is not cleared, every item will be appended.
	\return G_NO_ERROR if the operation will succeed, else an error code.
*/
GError FileUtils::ScanPath(GStringList& FoundFiles, const GChar8 *Directory, const GBool ScanRecursively,
						   const GBool AddPath, const GChar8 *FileExt) {

	GString fName, fExt, checkPath;
	GError err;

	// check for an invalid path name
	if (!Directory)
		return G_INVALID_PARAMETER;
	GString sPath = Directory;
	if (sPath.length() <= 0)
		return G_INVALID_PARAMETER;
	// fix path to insure OS functions correctness
	GString fixedPath = StrUtils::OSFixPath(sPath, G_TRUE);
	sPath = fixedPath;

	#if defined(G_OS_WIN) && !defined(__CYGWIN__) && !defined(__GNUC__)
		// search for all files
		sPath += '*';
		WIN32_FIND_DATAA data;
		// find first file
		HANDLE handle = FindFirstFileA(StrUtils::ToAscii(sPath), &data);
		if (handle == INVALID_HANDLE_VALUE)
			return G_READ_ERROR;
		// loop through
		do {
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (ScanRecursively == G_TRUE) {
					// we don't wanna 'current' and 'previous' directory
					checkPath = data.cFileName;
					if ((checkPath != ".") && (checkPath != "..")) {
						err = ScanPath(FoundFiles, StrUtils::ToAscii(fixedPath + checkPath), ScanRecursively, AddPath, FileExt);
						if (err != G_NO_ERROR)
							return err;
					}
				}
			}
			else {
				fName = data.cFileName;
				if (FileExt) {
					fExt = StrUtils::ExtractFileExt(fName);
					if (StrUtils::SameText(fExt, FileExt)) {
						// add full path, if specified
						if (AddPath)
							FoundFiles.push_back(fixedPath + fName);
						else
							FoundFiles.push_back(fName);
					}
				}
				else {
					// add full path, if specified
					if (AddPath)
						FoundFiles.push_back(fixedPath + fName);
					else
						FoundFiles.push_back(fName);
				}
			}
		} while (FindNextFileA(handle, &data) != 0);
		FindClose(handle);
		return G_NO_ERROR;
	// IRIX does not have a d_type field, so the only possible solution is
	// to try to enter into a single file entry (as if it would be a directory)
	#elif defined(G_OS_IRIX)
		DIR *handle = opendir(StrUtils::ToAscii(fixedPath));
		if (handle) {
			direct *rc;
			while((rc = readdir(handle)) != NULL) {
				// first check if this is a directory entry
				checkPath = rc->d_name;
				DIR *tmpHandle = opendir(StrUtils::ToAscii(fixedPath + checkPath));
				if (tmpHandle) {
					closedir(tmpHandle);
					if (ScanRecursively == G_TRUE) {
						// we don't wanna 'current' and 'previous' directory
						if ((checkPath != ".") && (checkPath != "..")) {
							err = ScanPath(FoundFiles, StrUtils::ToAscii(fixedPath + checkPath), ScanRecursively, AddPath, FileExt);
							if (err != G_NO_ERROR)
								return err;
						}
					}
				}
				else {
					fName = rc->d_name;
					if (FileExt) {
						fExt = StrUtils::ExtractFileExt(fName);
						if (StrUtils::SameText(fExt, FileExt)) {
							// add full path, if specified
							if (AddPath)
								FoundFiles.push_back(fixedPath + fName);
							else
								FoundFiles.push_back(fName);
						}
					}
					else {
						// add full path, if specified
						if (AddPath)
							FoundFiles.push_back(fixedPath + fName);
						else
							FoundFiles.push_back(fName);
					}
				}
			}
			closedir(handle);
			return G_NO_ERROR;
		}
		return G_READ_ERROR;
	#elif defined(G_OS_WIN) && defined(__GNUC__)
		// MinGW (windows) does not have a d_type field, so the only possible solution is
		// to try to enter into a single file entry (as if it would be a directory)
		DIR *handle = opendir(StrUtils::ToAscii(fixedPath));
		if (handle) {
			dirent *rc;
			while((rc = readdir(handle)) != NULL) {
				// first check if this is a directory entry
				checkPath = rc->d_name;
				DIR *tmpHandle = opendir(StrUtils::ToAscii(fixedPath + checkPath));
				if (tmpHandle) {
					closedir(tmpHandle);
					if (ScanRecursively == G_TRUE) {
						// we don't wanna 'current' and 'previous' directory
						if ((checkPath != ".") && (checkPath != "..")) {
							err = ScanPath(FoundFiles, StrUtils::ToAscii(fixedPath + checkPath), ScanRecursively, AddPath, FileExt);
							if (err != G_NO_ERROR)
								return err;
						}
					}
				}
				else {
					fName = rc->d_name;
					if (FileExt) {
						fExt = StrUtils::ExtractFileExt(fName);
						if (StrUtils::SameText(fExt, FileExt)) {
							// add full path, if specified
							if (AddPath)
								FoundFiles.push_back(fixedPath + fName);
							else
								FoundFiles.push_back(fName);
						}
					}
					else {
						// add full path, if specified
						if (AddPath)
							FoundFiles.push_back(fixedPath + fName);
						else
							FoundFiles.push_back(fName);
					}
				}
			}
			closedir(handle);
			return G_NO_ERROR;
		}
		return G_READ_ERROR;
	#else
        DIR *handle = opendir(StrUtils::ToAscii(fixedPath));
        if (handle) {
            dirent *rc;
            while((rc = readdir(handle)) != NULL) {
				if (rc->d_type == DT_DIR) {
					if (ScanRecursively == G_TRUE) {
						// we don't wanna 'current' and 'previous' directory
						checkPath = rc->d_name;
						if ((checkPath != ".") && (checkPath != "..")) {
							err = ScanPath(FoundFiles, StrUtils::ToAscii(fixedPath + checkPath), ScanRecursively, AddPath, FileExt);
							if (err != G_NO_ERROR)
								return err;
						}
					}
				}
				else {
					fName = rc->d_name;
					if (FileExt) {
						fExt = StrUtils::ExtractFileExt(fName);
						if (StrUtils::SameText(fExt, FileExt)) {
							// add full path, if specified
							if (AddPath)
								FoundFiles.push_back(fixedPath + fName);
							else
								FoundFiles.push_back(fName);
						}
					}
					else {
						// add full path, if specified
						if (AddPath)
							FoundFiles.push_back(fixedPath + fName);
						else
							FoundFiles.push_back(fName);
					}
				}
            }
            closedir(handle);
			return G_NO_ERROR;
        }
		return G_READ_ERROR;
	#endif
}

GError FileUtils::ReadFile(const GChar8 *FileName, GDynArray<GChar8>& Buffer) {

	GInt32 fileSize;
	GInt32 readBytes;

	if (!FileName || std::strlen(FileName) <= 0)
		return G_INVALID_PARAMETER;
	// open the file
#if defined(G_OS_WIN) && _MSC_VER >= 1400
	std::FILE *file = NULL;
	errno_t openErr = fopen_s(&file, FileName, "rb");
	if (!file || openErr)
		return G_PERMISSION_DENIED;
#else
	std::FILE *file = std::fopen(FileName, "rb");
	if (!file)
		return G_PERMISSION_DENIED;
#endif

	// calculate file size
	GInt32 seek = std::fseek(file, 0, SEEK_END);
	if (seek != 0) {
		std::fclose(file);
		return G_READ_ERROR;
	}
	fileSize = (GInt32)std::ftell(file);
	if (fileSize <= 0) {
		Buffer.resize(0);
		std::fclose(file);
		return G_READ_ERROR;
	}
	// reset file pointer at beginning of file	
	seek = std::fseek(file, 0, SEEK_SET);
	if (seek != 0) {
		std::fclose(file);
		return G_READ_ERROR;
	}
	// resize buffer, so it can contain whole file
	Buffer.resize(fileSize);
	GDynArray<GChar8>::iterator it = Buffer.begin();
	void *data = (void *)(&(*it));
	// read the file
	readBytes = (GInt32)std::fread(data, 1, fileSize, file);
	// check if file was completely read
	std::fclose(file);
	if (readBytes < fileSize) {
		Buffer.resize(readBytes);
		return G_READ_ERROR;
	}
	return G_NO_ERROR;
}

GError FileUtils::WriteFile(const GChar8 *FileName, const GDynArray<GChar8>& Buffer, const GBool OverWrite) {

	GInt32 fileSize;
	GInt32 writtenBytes;

	if (!FileName || std::strlen(FileName) <= 0)
		return G_INVALID_PARAMETER;

	// file already exists and we can't overwrite it
	if (FileExists(FileName) && !OverWrite)
		return G_FILE_ALREADY_EXISTS;

#if defined(G_OS_WIN) && _MSC_VER >= 1400
	// now open file for write operations
	std::FILE *file = NULL;
	errno_t openErr = fopen_s(&file, FileName, "wb");
	if (!file || openErr)
		return G_PERMISSION_DENIED;
#else
	// now open file for write operations
	std::FILE *file = std::fopen(FileName, "wb");
	if (!file)
		return G_PERMISSION_DENIED;
#endif
	// extract buffer data pointer
	fileSize = (GInt32)Buffer.size();
	GDynArray<GChar8>::const_iterator it = Buffer.begin();
	const void *data = (const void *)(&(*it));
	writtenBytes = (GInt32)std::fwrite(data, 1, fileSize, file);
	// check if file was completely written
	std::fclose(file);
	if (writtenBytes < fileSize)
		return G_WRITE_ERROR;
	return G_NO_ERROR;
}

}
