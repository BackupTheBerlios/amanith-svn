/****************************************************************************
** $file: amanith/gimpexp.h   0.1.1.0   edited Sep 24 08:00
**
** Import/Export plugin interface.
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

#ifndef GIMPEXP_H
#define GIMPEXP_H

/*!
	\file gimpexp.h
	\brief Import/Export plugins interface class.
*/

#include "amanith/gelement.h"
#include "amanith/support/gutilities.h"
#include "amanith/gerror.h"

namespace Amanith {

	//! Read/Write ability.
	enum GReadWriteOp {
		//! Feature provides only read operations
		G_IMPEXP_READ,
		//! Feature provides only write operations
		G_IMPEXP_WRITE,
		//! Feature provides read and write operations
		G_IMPEXP_READWRITE
	};

	// *********************************************************************
	//                             GImpExpEntry
	// *********************************************************************

	class G_EXPORT GImpExpFeature {

	private:
		//! Class descriptor of managed objects by this feature
		GClassID gClassID;
		//! Name of the supported format
		GString gFormatName;
		//! Semicolon separated, supported file extensions
		GString gExtensions;
		//! Version number, in the format of Maj.MidMaj.MidMin.Min (ex: 1.3.0.2)
		GInt32 gMaj, gMidMaj, gMidMin, gMin;
		//! I/O abilities
		GReadWriteOp gReadWriteOp;

	public:
		//! Default constructor.
		GImpExpFeature();
		//! Copy constructor.
		GImpExpFeature(const GImpExpFeature& Source);
		//! Set constructor.
		GImpExpFeature(const GClassID Class_ID, const GChar8 *FormatName,
					  const GChar8 *Extensions,
					  const GInt32 Maj, const GInt32 MidMaj = 0, const GInt32 MidMin = 0, const GInt32 Min = 0,
					  const GReadWriteOp ReadWriteOp = G_IMPEXP_READWRITE);
		//! Class descriptor of managed objects by this feature
		const GClassID& ClassID() const {
			return gClassID;
		}
		//! Name of the supported format
		const GChar8 *FormatName() const {
			return StrUtils::ToAscii(gFormatName);
		}
		//! Ask for I/O abilities provided by this feature.
		inline GReadWriteOp ReadWriteOp() const {
			return gReadWriteOp;
		}
		//! Version number in string format
		GString Version() const;
		//! Ask for a format support, specifying the type of operation (read and/or write)
		GBool FormatSupported(const GChar8 *FormatName, const GBool Reading, const GBool Writing) const;
		// Ask for a format support for a specified class ID, specifying the type of operation (read and/or write)
		GBool FormatSupported(const GChar8 *FormatName, const GClassID& ID,
							  const GBool Reading, const GBool Writing) const;
		//! Ask for a file extension support, specifying the type of operation (read and/or write)
		GBool FileExtensionSupported(const GChar8 *FileExt, const GBool Reading, const GBool Writing) const;
		//! Ask for a file extension support for a specified class ID, specifying the type of operation (read and/or write)
		GBool FileExtensionSupported(const GChar8 *FileExt, const GClassID& ID,
									 const GBool Reading, const GBool Writing) const;
		//! Number of supported file extensions
		GUInt32 FileExtensionsCount() const;
		//! Get the Index-th file extension
		GString FileExtension(const GUInt32 Index) const;
	};

	// *********************************************************************
	//                               GImpExp
	// *********************************************************************

	//! GImpExp static class descriptor.
	static const GClassID G_IMPEXP_CLASSID = GClassID("GImpExp", 0x743D5471, 0x07DE4EE3, 0x8BCB8557, 0x86A3035B);

	class G_EXPORT GImpExp : public GElement {

	private:
		//! Internal list of import/export features.
		GDynArray<GImpExpFeature> gFeatures;

	protected:
		// option name and its value, maybe it will be transformed into an std::pair...
		struct GImpExpOption {
			GString OptionName;
			GString OptionValue;
			GImpExpOption() {
			}
			GImpExpOption(const GString& Name, const GString& Value) {
				OptionName = Name;
				OptionValue = Value;
			}
		};
		//! Add an import/export feature to the internal list
		void AddEntry(const GImpExpFeature& Entry);
		//! Parse semicolon separated options
		GError ParseOptions(const GChar8 *Options, GDynArray<GImpExpOption>& ParsedOptions) const;
		/*!
			Read an element from external file. <b>Must be implemented by plugins</b>.

			\param FullFileName is a string containing the full file name (path + name + extension) of the file
			we wanna read from. It is ensured that is not empty.
			\param Element a reference to the element that will be filled with information read from file.\n
			It is ensured that it's of a type (ClassID) supported (for read operation) by at least one feature
			held by this plugin.
			\param ParsedOptions an array containing all valid (non empty) options specified for reading operation.
			\return if operation will complete successfully, G_NO_ERROR will be returned, else an error code.
		*/
		virtual GError DoRead(const GChar8 *FullFileName, GElement& Element,
							  const GDynArray<GImpExpOption>& ParsedOptions) = 0;
		/*!
			Write an element to external file. <b>Must be implemented by plugins</b>.

			\param FullFileName is a string containing the full file name (path + name + extension) of the file
			we wanna write to. It is ensured that is not empty.
			\param Element a reference to the element that we wanna export to external file.\n
			It is ensured that it's of a type (ClassID) supported (for write operation) by at least one feature
			held by this plugin.
			\param ParsedOptions an array containing all valid (non empty) options specified for writing operation.
			\return if operation will complete successfully, G_NO_ERROR will be returned, else an error code.
		*/
		virtual GError DoWrite(const GChar8 *FullFileName, const GElement& Element,
							   const GDynArray<GImpExpOption>& ParsedOptions) = 0;

	public:
		//! Default constructor
		GImpExp() : GElement() {
		}
		//! Constructor with owner parameter
		GImpExp(const GElement* Owner) : GElement(Owner) {
		}
		//! Destructor
		~GImpExp() {
		}
		//! Read an element from external file
		GError Read(const GChar8 *FullFileName, GElement& Element, const GChar8 *Options = NULL);
		/*! Write an element to external file. If operation will complete successfully, G_NO_ERROR will be
		returned, else an error code.
		*/
		GError Write(const GChar8 *FullFileName, const GElement& Element, const GChar8 *Options = NULL);
		//! Get all input/output features supported by this plugin
		inline const GDynArray<GImpExpFeature>& Features() const {
			return gFeatures;
		}
		//! Ask for a format support, specifying the type of operation (read and/or write)
		GBool FormatSupported(const GChar8 *FormatName, const GBool Reading, const GBool Writing) const;
		//! Ask for a format support for a specified class ID, specifying the type of operation (read and/or write)
		GBool FormatSupported(const GChar8 *FormatName, const GClassID& ID,
							  const GBool Reading, const GBool Writing) const;
		//! Ask for a file extension support, specifying the type of operation (read and/or write)
		GBool FileExtensionSupported(const GChar8 *FileExt, const GBool Reading, const GBool Writing) const;
		//! Ask for a file extension support for a specified class ID, specifying the type of operation (read and/or write)
		GBool FileExtensionSupported(const GChar8 *FileExt, const GClassID& ID,
									 const GBool Reading, const GBool Writing) const;
		//! Get class descriptor.
		const GClassID& ClassID() const {
			return G_IMPEXP_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
	};


	// *********************************************************************
	//                              GImpExpProxy
	// *********************************************************************

	/*!
		\class GImpExpProxy
		\brief This class implements a GImpExp proxy (provider).

		This proxy does not override CreateNew() method because we don't wanna make a creation of a GImpExp
		class possible (because of virtual and not implemented Read() and Write() methods).
	*/
	class G_EXPORT GImpExpProxy : public GElementProxy {
	public:
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_IMPEXP_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
	};
	//! Static proxy for GImpExp class.
	static const GImpExpProxy G_IMPEXP_PROXY;

};	// end namespace Amanith

#endif
