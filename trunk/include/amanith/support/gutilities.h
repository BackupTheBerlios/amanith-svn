/****************************************************************************
** $file: amanith/support/gutilities.h   0.3.0.0   edited Jan, 30 2006
**
** Global general purpose utilities.
**
**
** Copyright (C) 2004-2006 Mazatech Inc. All rights reserved.
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

#ifndef GUTILITIES_H
#define GUTILITIES_H

#include "amanith/gglobal.h"
#include "amanith/gerror.h"
#include "amanith/geometry/gvect.h"
#include "amanith/geometry/gquat.h"
#include <algorithm>

/*!
	\file gutilities.h
	\brief Interfaces for generic utilities on strings, files and system information.
*/
namespace Amanith {

	// *********************************************************************
	//                            System utilities
	// *********************************************************************

	/*!
		\class SysUtils
		\brief A collection of generic cross-platform system utilities.
	*/
	class G_EXPORT SysUtils {
	private:
		//! 
		static GSystemInfo gStaticSystemInfo;
		static GBool gSysInfoExtracted;

	public:
		//! Get system information.
		static GBool SystemInfo(GSystemInfo& SysInfo);
		//! Get path where Amanith framework is installed. It uses the environment AMANITHDIR variable.
		static GString AmanithPath();
		//! Get path where standard Amanith plugins are located. It uses the environment AMANITHDIR variable.
		static GString AmanithPluginsPath();
		//! Get a unique machine identifier.
		static GError MachineGUID(GString& Identifier);
		//! Redirect I/O text to console, for Win32 applications.
		static void RedirectIOToConsole();
	};


	// *********************************************************************
	//                            String utilities
	// *********************************************************************

	/*!
		\class StrUtils
		\brief A collection of cross-platform generic string utilities.
	*/
	class G_EXPORT StrUtils {
	public:
		//! Returns lowered string
		static GString Lower(const GString& Str);
		//! Returns uppered string
		static GString Upper(const GString& Str);
		//! Returns a string that contains the 'HowMany' rightmost characters of the string 'Str'
		static GString Right(const GString& Str, const GUInt32 HowMany);
		//! Returns a substring that contains the 'HowMany' leftmost characters of the string 'Str'
		static GString Left(const GString& Str, const GUInt32 HowMany);
		//! Replace every 'Before' char occurrence with 'After' char.
		static GString Replace(const GString& Str, const GChar8 Before, const GChar8 After);
		/*!
			Searches a string in a forward direction for the first occurrence of a substring that
			matches a specified sequence of characters.

			\param Source the source string, where to search for StrToFind.
			\param StrToFind the string for which the member function is to search.
			\param CaseInsensitive G_TRUE if comparison must be done case-insensitive, else G_FALSE.
			\param StartOffset index of the position at which the search is to begin.
			\param PosFound the index of the first character of the substring searched for when successful.
			\return G_TRUE if substring has been found, else G_FALSE.
		*/
		static GBool Find(const GString& Source, const GString& StrToFind, const GBool CaseInsensitive = G_TRUE,
						  const GUInt32 StartOffset = 0, GUInt32 *PosFound = NULL);
		//! Extract the path portion of specified parameter.
		static GString ExtractFilePath(const GString& FullFileName);
		/*!
			Extract the file name portion of specified parameter. If IncludeExtension is set to G_TRUE, the
			file extension will be included too.
		*/
		static GString ExtractFileName(const GString& FullFileName, const GBool IncludeExtension = G_TRUE);
		//! Extract the file extension portion of specified parameter.
		static GString ExtractFileExt(const GString& FullFileName);
		//! Fix a path for the current OS
		static GString OSFixPath(const GString& GeneralPath, const GBool InsureFinalDelimiter = G_FALSE);
		//! Case-insensitive string comparison
		static GBool SameText(const GString& Str1, const GString& Str2);
		//! Purge a string; remove all dusty characters specified in 'Dust' string and return the purged string
		static GString Purge(const GString& DustySource, const GString& Dust);
		//! Convert a vector into string format
		template<typename DATA_TYPE, GUInt32 SIZE>
		static GString ToString(const GVectBase<DATA_TYPE, SIZE>& v, const GString& Separator = ",",
								const GChar8 *Format = NULL) {

			GString s = "";

			for (GUInt32 i = 0; i < SIZE - 1; ++i)
				s += ToString(v[i], Format) + Separator;
			s = s + ToString(v[SIZE - 1], Format);
			return s;
		}
		//! Convert a quaternion into string format
		template<typename DATA_TYPE>
		static GString ToString(const GQuat<DATA_TYPE>& q, const GString& Separator = ",",
								const GChar8 *Format = NULL) {

			GString s = "";

			for (GUInt32 i = 0; i < 3; i++)
				s += ToString(q[i], Format) + Separator;
			s = s + ToString(q[3], Format);
			return s;
		}
		//! Convert an integer into string format
		static GString ToString(const GInt32 Value, const GChar8 *Format = "%d");
		//! Convert an integer into string format
		static GString ToString(const GUInt32 Value, const GChar8 *Format = "%d");
		//! Convert an integer into string format
		static GString ToString(const GInt16 Value, const GChar8 *Format = "%d");
		//! Convert an integer into string format
		static GString ToString(const GUInt16 Value, const GChar8 *Format = "%d");
		//! Convert an integer into string format
		static GString ToString(const GInt8 Value, const GChar8 *Format = "%d");
		//! Convert an integer into string format
		static GString ToString(const GUInt8 Value, const GChar8 *Format = "%d");
		//! Convert a float into string format
		static GString ToString(const GFloat Value, const GChar8 *Format = "%f");
		//! Convert a double into string format
		static GString ToString(const GDouble Value, const GChar8 *Format = "%f");
		//! Converts the contents of a string as a C-style, null-terminated string
		static const GChar8 *ToAscii(const GString& Value);
		/*!
			Converts a string (representing a number) into a GInt32 number; the
			return value is 0 if the input cannot be converted to a value of that type
		*/
		static GInt32 ToInt(const GString& Value);
		/*!
			Converts a string (representing a number) into a GLong number; the
			return value is 0 if the input cannot be converted to a value of that type
		*/
		static GLong ToLong(const GString& Value);
		/*!
			Converts a string (representing a number) into a GDouble number; the
			return value is 0 the input cannot be converted to a value of that type
		*/
		static GDouble ToDouble(const GString& Value);
		//! Convert a number to its hexadecimal string representation
		static GString ToHex(const GUInt32 Number, const GUInt32 Width);
		//! Split a string into a sequence of strings, specifying split separator
		static GStringList Split(const GString& Value, const GString& Separator, GBool Empties = G_FALSE);
		//! Joins the string list into a single string with each element separated by the string InterStr
		static GString Merge(const GStringList& InputList, const GChar8 *InterStr = "");
		//! Set vector values from string representation
		template<GUInt32 SIZE>
		static GError FromString(const GString& SourceStr, GVectBase<GReal, SIZE>& Vect) {

			GVectBase<GReal, SIZE> vaux;
			GUInt32 i;
			GStringList::const_iterator it;
			// split components
			GStringList list = StrUtils::Split(SourceStr, ",");

			// we MUST have SIZE components
			if (list.size() != SIZE)
				return G_INVALID_FORMAT;
			// extract each component
			i = 0;
			for (it = list.begin(); it != list.end(); ++it, ++i)
				vaux[i] = (GReal)StrUtils::ToDouble(*it);

			Vect = vaux;
			return G_NO_ERROR;
		}
		//! Set quaternion values from string representation
		static GError FromString(const GString& SourceStr, GQuaternion& Quat);
	};


	// *********************************************************************
	//                            File utilities
	// *********************************************************************

	/*!
		\class FileUtils
		\brief A collection of cross-platform generic file utilities.
	*/
	class G_EXPORT FileUtils {
	public:
		//! Read an external file into a memory buffer.
		static GError ReadFile(const GChar8 *FileName, GDynArray<GChar8>& Buffer);
		//! Write a memory buffer to an external file.
		static GError WriteFile(const GChar8 *FileName, const GDynArray<GChar8>& Buffer, const GBool OverWrite = G_FALSE);
		//! Scan a directory to collect the list of all included files
		static GError ScanPath(GStringList& FoundFiles, const GChar8 *Directory, const GBool ScanRecursively,
							   const GBool AddPath = G_TRUE, const GChar8 *FileExt = NULL);
		//! Check for a file existence
		static GBool FileExists(const GChar8 *FullFileName);
	};

	/*!
		Reverses the order of the elements within a range. Example:
\code
		GDynArray<GInt32> a;

		a.push_back(0);
		a.push_back(1);
		a.push_back(2);
		a.push_back(3);
		a.push_back(4);

		ReverseArray(a, 2, 7);
		// now array has this sequence of elements
		//
		// a[0] == 0
		// a[1] == 1
		// a[2] == 4
		// a[3] == 3
		// a[4] == 2
\endcode
		\param Array the elements's container.
		\param FromIndex the lower bound (index) of reverse range
		\param ToIndex the upper bound (index) of reverse range
		\note If specified range is out of permitted range, just valid element positions will be reversed.
	*/
	template<typename DATA_TYPE>
	void ReverseArray(GDynArray<DATA_TYPE>& Array, const GUInt32 FromIndex, const GUInt32 ToIndex) {

		typename GDynArray<DATA_TYPE>::iterator itBegin = Array.begin();
		typename GDynArray<DATA_TYPE>::iterator itEnd = Array.begin();

		itBegin += FromIndex;
		itEnd += (ToIndex + 1);
		std::reverse(itBegin, itEnd);
	}
};	// end namespace Amanith

#endif
