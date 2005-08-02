/****************************************************************************
** $file: amanith/src/gimpexp.cpp   0.1.0.0   edited Jun 30 08:00
**
** Import/Export plugin interface implementation.
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

#include "amanith/gimpexp.h"
#include "amanith/support/gutilities.h"

namespace Amanith {


// *********************************************************************
//                             GImpExpFeature
// *********************************************************************

/*!
	\class GImpExpFeature
	\brief Generic import/export feature descriptor.

	GImpExpFeature implements an import/export feature descriptor, and it's used by import/export plugins
	to describe their abilities, and to answer to various queries like "is this format supported for reading"
	or "is this file extension managed?", and much more.\n
	An import/export feature descriptor is made of several informations, like supported format name, list
	of supported file extensions, version of supported file-types and a class descriptor (GClassID) that specify
	what kind of element is supported by this feature.\n
	For example the following feature descriptor describes the ability to read and write JPEG images:\n

	GImpExpFeature(G_PIXELMAP_CLASSID, "Jpeg", "jpg;jpeg", 1, 0, 0, 0, G_IMPEXP_READWRITE)\n

	G_PIXELMAP_CLASSID: Images are managed by GPixelMap	class.\n
	"Jpeg": Format name.\n
	"jpg;jpeg": Supported file extensions (associated to the file format).\n
	1, 0, 0, 0: Version 1.0.0.0.\n
	G_IMPEXP_READWRITE: I/O abilities.
*/

GImpExpFeature::GImpExpFeature() {
}

// (copy) constructor
GImpExpFeature::GImpExpFeature(const GImpExpFeature& Source) {

	gClassID = Source.gClassID;
	gFormatName = Source.gFormatName;
	gExtensions = Source.gExtensions;
	gMaj = Source.gMaj;
	gMidMaj = Source.gMidMaj;
	gMidMin = Source.gMidMin;
	gMin = Source.gMin;
	gReadWriteOp = Source.gReadWriteOp;
}

/*!
	\param Class_ID a class descriptor for the class supported by this feature.
	\param FormatName a string describing supported format.
	\param Extensions a string of semicolon-separated file extensions supported by this feature. (ex: "jpg;jpeg")
	\param Maj, MidMaj, MidMin, Min integer numbers forming format version (ex: 1, 0, 0, 0 for 1.0.0.0 version)
	\param ReadWriteOp value that specifies I/O abilities
*/
GImpExpFeature::GImpExpFeature(const GClassID Class_ID, const GChar8 *FormatName,
							const GChar8 *Extensions,
							const GInt32 Maj, const GInt32 MidMaj,
							const GInt32 MidMin, const GInt32 Min,
							const GReadWriteOp ReadWriteOp) {

	gFormatName = FormatName;
	gExtensions = Extensions;
	gMaj = Maj;
	if (gMaj < 0)
		gMaj = 0;
	gMidMaj = MidMaj;
	gMidMin = MidMin;
	gMin = Min;
	gReadWriteOp = ReadWriteOp;
	gClassID = Class_ID;
}

/*!
	\return a string representing supported format version (ex: "1.0.0.0")
*/
GString GImpExpFeature::Version() const {

	GString vers;

	// major
	vers = StrUtils::ToString(gMaj) + ".";
	// mid-major
	if (gMidMaj >= 0)
		vers += StrUtils::ToString(gMidMaj) + ".";
	else
		vers += "x.";
	// mid-minor
	if (gMidMin >= 0)
		vers = StrUtils::ToString(gMidMin) + ".";
	else
		vers += "x.";
	// minor
	if (gMin >= 0)
		vers = StrUtils::ToString(gMin);
	else
		vers += "x";
	return vers;
}

/*!
	\param FormatName a string that contains the format we'are asking support for. Comparison is case-insensitive
	\param Reading if true we are asking for "read operation supported for requested format". False if we are
	not interested to know if reading operation is supported for the specified format.
	\param Writing if true we are asking for "write operation supported for requested format". False if we are
	not interested to know if writing operation is supported for the specified format.
	\return a boolean that represents the response to the query induced by this function.
*/
GBool GImpExpFeature::FormatSupported(const GChar8 *FormatName, const GBool Reading, const GBool Writing) const {

	if ((!FormatName) || (!Reading && !Writing))
		return G_FALSE;

	GString fName1 = FormatName;

	if (StrUtils::SameText(gFormatName, fName1)) {
		if (Reading && Writing) {
			if (gReadWriteOp == G_IMPEXP_READWRITE)
				return G_TRUE;
			return G_FALSE;
		}
		else
		if (Reading) {
			if (gReadWriteOp == G_IMPEXP_READWRITE || gReadWriteOp == G_IMPEXP_READ)
				return G_TRUE;
			return G_FALSE;
		}
		else {
			if (gReadWriteOp == G_IMPEXP_READWRITE || gReadWriteOp == G_IMPEXP_WRITE)
				return G_TRUE;
			return G_FALSE;
		}
	}
	return G_FALSE;
}

/*!
	\param FormatName a string that contains the format we'are asking support for. Comparison is case-insensitive
	\param ID a class descriptor, used as a filter in addition with Reading and Writing parameters.
	\param Reading if true we are asking for "read operation supported for requested format". False if we are
	not interested to know if reading operation is supported for the specified format.
	\param Writing if true we are asking for "write operation supported for requested format". False if we are
	not interested to know if writing operation is supported for the specified format.
	\return a boolean that represents the response to the query induced by this function.
*/
GBool GImpExpFeature::FormatSupported(const GChar8 *FormatName, const GClassID& ID,
									  const GBool Reading, const GBool Writing) const {

	if (FormatSupported(FormatName, Reading, Writing) && gClassID == ID)
		return G_TRUE;
	return G_FALSE;
}

/*!
	\param FileExt a string that contains the file extension (without dot!) we'are asking support for (ex: "jpg").\n
	Comparison is case-insensitive.
	\param Reading if true we are asking for "read operation supported for requested file extension". False if we are
	not interested to know if reading operation is supported for the specified file extension.
	\param Writing if true we are asking for "write operation supported for requested file extension". False if we are
	not interested to know if writing operation is supported for the specified file extension.
	\return a boolean that represents the response to the query induced by this function.
*/
GBool GImpExpFeature::FileExtensionSupported(const GChar8 *FileExt, const GBool Reading, const GBool Writing) const {

	if ((!FileExt) || (!Reading && !Writing))
		return G_FALSE;

	GStringList exts = StrUtils::Split(gExtensions, ";", G_FALSE);
	GString fExt = FileExt;

	GStringList::const_iterator it;
	for (it = exts.begin(); it != exts.end(); ++it) {
		if (StrUtils::SameText(*it, fExt)) {
			if (Reading && Writing) {
				if (gReadWriteOp == G_IMPEXP_READWRITE)
					return G_TRUE;
				return G_FALSE;
			}
			else
			if (Reading) {
				if (gReadWriteOp == G_IMPEXP_READWRITE || gReadWriteOp == G_IMPEXP_READ)
					return G_TRUE;
				return G_FALSE;
			}
			else {
				if (gReadWriteOp == G_IMPEXP_READWRITE || gReadWriteOp == G_IMPEXP_WRITE)
					return G_TRUE;
				return G_FALSE;
			}
		}
	}
	return G_FALSE;
}

/*!
	\param FileExt a string that contains the file extension (without dot!) we'are asking support for (ex: "jpg").\n
	Comparison is case-insensitive.
	\param ID a class descriptor, used as a filter in addition with Reading and Writing parameters.
	\param Reading if true we are asking for "read operation supported for requested file extension". False if we are
	not interested to know if reading operation is supported for the specified file extension.
	\param Writing if true we are asking for "write operation supported for requested file extension". False if we are
	not interested to know if writing operation is supported for the specified file extension.
	\return a boolean that represents the response to the query induced by this function.
*/
GBool GImpExpFeature::FileExtensionSupported(const GChar8 *FileExt, const GClassID& ID,
											 const GBool Reading, const GBool Writing) const {

	if (FileExtensionSupported(FileExt, Reading, Writing) && gClassID == ID)
		return G_TRUE;
	return G_FALSE;
}

/*!
	\return Number of file extensions "provided" by this feature.
*/
GUInt32 GImpExpFeature::FileExtensionsCount() const {

	GStringList sList = StrUtils::Split(gExtensions, ";", G_FALSE);
	return (GUInt32)sList.size();
}

/*!
	\param Index specify what file extension we wanna retrieve. It must be in the range [0; FileExtensionsCount() - 1]
	\return a string containing the requested file extension. In case of invalid Index an empty string is returned.
*/
GString GImpExpFeature::FileExtension(const GUInt32 Index) const {

	GStringList sList = StrUtils::Split(gExtensions, ";", G_FALSE);
	if (Index >= sList.size())
		return GString("");

	GStringList::const_iterator it = sList.begin();
	for (GUInt32 i = 0; i < Index; i++)
		it++;
	return (*it);
}

// *********************************************************************
//                                GImpExp
// *********************************************************************

/*!
	\class GImpExp
	\brief Import/Export plugin interface class.

	GImpExp represents the base class for an import/export plugin.\n
	Every plugin of this type must be derived from this class and, according to its I/O abilities, it must
	implement Read and/or Write functions.
*/

/*!
	\param Entry the new feature that we wanna add to the internal list
*/
void GImpExp::AddEntry(const GImpExpFeature& Entry) {
	gFeatures.push_back(Entry);
}

/*!
	\param Options a string containing semicolon-separated options. Each option must be in the format of
	name=value (ex: "forcewritting=true;allowblanks=false;version=3")
	\param ParsedOptions the list of parsed options given in output.
	\return An error code if operation cannot be concluded successfully, else G_NO_ERROR
*/
GError GImpExp::ParseOptions(const GChar8 *Options, GDynArray<GImpExpOption>& ParsedOptions) const {
	
	GStringList optionsList = StrUtils::Split(Options, ";", G_FALSE);
	GStringList option;
	GString optionName, optionValue;

	GStringList::const_iterator it = optionsList.begin();
	for (; it != optionsList.end(); ++it) {
		option = StrUtils::Split(*it, "=", G_FALSE);
		if (option.size() == 2)
			ParsedOptions.push_back(GImpExpOption(option.front(), option.back()));
		else
			return G_INVALID_PARAMETER;
	}
	return G_NO_ERROR;
}

/*!
	\param FormatName a string that contains the format we'are asking support for. Comparison is case-insensitive
	\param Reading if true we are asking for "read operation supported for requested format". False if we are
	not interested to know if reading operation is supported for the specified format.
	\param Writing if true we are asking for "write operation supported for requested format". False if we are
	not interested to know if writing operation is supported for the specified format.
	\return a boolean that represents the response to the query induced by this function.
*/
GBool GImpExp::FormatSupported(const GChar8 *FormatName, const GBool Reading, const GBool Writing) const {

	if ((!FormatName) || (!Reading && !Writing))
		return G_FALSE;

	GDynArray<GImpExpFeature>::const_iterator it;
	for (it = gFeatures.begin(); it != gFeatures.end(); ++it) {
		if (it->FormatSupported(FormatName, Reading, Writing))
			return G_TRUE;
	}
	return G_FALSE;
}

/*!
	\param FormatName a string that contains the format we'are asking support for. Comparison is case-insensitive
	\param ID a class descriptor, used as a filter in addition with Reading and Writing parameters.
	\param Reading if true we are asking for "read operation supported for requested format". False if we are
	not interested to know if reading operation is supported for the specified format.
	\param Writing if true we are asking for "write operation supported for requested format". False if we are
	not interested to know if writing operation is supported for the specified format.
	\return a boolean that represents the response to the query induced by this function.
*/
GBool GImpExp::FormatSupported(const GChar8 *FormatName, const GClassID& ID,
							   const GBool Reading, const GBool Writing) const {

	if ((!FormatName) || (!Reading && !Writing))
		return G_FALSE;

	GDynArray<GImpExpFeature>::const_iterator it;
	for (it = gFeatures.begin(); it != gFeatures.end(); ++it) {
		if (it->FormatSupported(FormatName, ID, Reading, Writing))
			return G_TRUE;
	}
	return G_FALSE;
}


/*!
	\param FileExt a string that contains the file extension we'are asking support for. Comparison is case-insensitive
	\param Reading if true we are asking for "read operation supported for requested file extension". False if we are
	not interested to know if reading operation is supported for the specified file extension.
	\param Writing if true we are asking for "write operation supported for requested file extension". False if we are
	not interested to know if writing operation is supported for the specified file extension.
	\return a boolean that represents the response to the query induced by this function.
*/
GBool GImpExp::FileExtensionSupported(const GChar8 *FileExt, const GBool Reading, const GBool Writing) const {

	if ((!FileExt) || (!Reading && !Writing))
		return G_FALSE;

	GDynArray<GImpExpFeature>::const_iterator it;
	for (it = gFeatures.begin(); it != gFeatures.end(); ++it) {
		if (it->FileExtensionSupported(FileExt, Reading, Writing))
			return G_TRUE;
	}
	return G_FALSE;
}

/*!
	\param FileExt a string that contains the file extension we'are asking support for. Comparison is case-insensitive
	\param ID a class descriptor, used as a filter in addition with Reading and Writing parameters.
	\param Reading if true we are asking for "read operation supported for requested file extension". False if we are
	not interested to know if reading operation is supported for the specified file extension.
	\param Writing if true we are asking for "write operation supported for requested file extension". False if we are
	not interested to know if writing operation is supported for the specified file extension.
	\return a boolean that represents the response to the query induced by this function.
*/
GBool GImpExp::FileExtensionSupported(const GChar8 *FileExt, const GClassID& ID,
									  const GBool Reading, const GBool Writing) const {

	if ((!FileExt) || (!Reading && !Writing))
		return G_FALSE;

	GDynArray<GImpExpFeature>::const_iterator it;
	for (it = gFeatures.begin(); it != gFeatures.end(); ++it) {
		if (it->FileExtensionSupported(FileExt, ID, Reading, Writing))
			return G_TRUE;
	}
	return G_FALSE;
}

/*!
	\param FullFileName is a string containing the full file name (path + name + extension) of the file we wanna read from.
	It must be non-empty.
	\param Element a reference to the element that will be filled with information read from file.
	\param Options a string containing a semicolon-separated options. Each option must be in the format name=value.\n
	(ex: "allowblanks=false;version=3"
	\return if operation will complete successfully, G_NO_ERROR will be returned, else an error code.
*/
GError GImpExp::Read(const GChar8 *FullFileName, GElement& Element, const GChar8 *Options) {

	// check for an empty filename
	GString fileName = FullFileName;
	if (fileName.length() <= 0)
		return G_INVALID_PARAMETER;

	GDynArray<GImpExpOption> parsedOptions;
	GError err;

	err = G_NO_ERROR;
	if (Options)
		err = ParseOptions(Options, parsedOptions);

	if (err == G_NO_ERROR)
		err = DoRead(FullFileName, Element, parsedOptions);
	return err;
}

/*!
	\param FullFileName is a string containing the full file name (path + name + extension) of the file we wanna write to.
	It must be non-empty.
	\param Element a reference to the element that we wanna export to external file.
	\param Options a string containing a semicolon-separated options. Each option must be in the format name=value.\n
	(ex: "forcewritting=true;allowblanks=false;version=3"
	\return if operation will complete successfully, G_NO_ERROR will be returned, else an error code.
*/
GError GImpExp::Write(const GChar8 *FullFileName, const GElement& Element, const GChar8 *Options) {

	// check for an empty filename
	GString fileName = FullFileName;
	if (fileName.length() <= 0)
		return G_INVALID_PARAMETER;

	GDynArray<GImpExpOption> parsedOptions;
	GError err;

	err = G_NO_ERROR;
	if (Options)
		err = ParseOptions(Options, parsedOptions);

	if (err == G_NO_ERROR)
		err = DoWrite(FullFileName, Element, parsedOptions);
	return err;
}

}
