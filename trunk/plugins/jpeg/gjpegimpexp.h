/****************************************************************************
** $file: amanith/plugins/jpeg/gjpegimpext.h   0.3.0.0   edited Jan, 30 2006
**
** 2D Pixelmap JPEG import/export plugin interface.
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

#ifndef GJPEGIMPEXP_H
#define GJPEGIMPEXP_H

#include "amanith/gimpexp.h"

namespace Amanith {

	// *********************************************************************
	//                                GJpegImpExp
	// *********************************************************************
	static const GClassID G_JPEGIMPEXP_CLASSID = GClassID("GJpegImpExp", 0x8C2CDAD1, 0x2BAD483C, 0xBB481886, 0xBCA2A1BE);

	class G_PLUGIN_EXPORT GJpegImpExp : public GImpExp {

	private:
		void AddJpegFeatures();
		GError RawJpegLoad(const GChar8 *FileName, GInt32& Width, GInt32& Height, GInt32& NumComponents,
						   GUChar8 **OutBuffer);
		GError RawJpegSave(const GChar8 *FileName, const GInt32 Width, const GInt32 Height,
						   const GInt32 NumComponents,
						   const GUChar8 *Buffer, const GInt32 Quality, const GBool ProgressiveEncoding);

	protected:
		GError ReadJpeg(const GChar8 *FullFileName, GElement& Element);
		GError WriteJpeg(const GChar8 *FullFileName, const GElement& Element, const GInt32 Quality,
						 const GBool ProgressiveEncoding = G_FALSE);

		// read a jpeg file and fill a GPixelMap with read data
		GError DoRead(const GChar8 *FullFileName, GElement& Element, const GDynArray<GImpExpOption>& ParsedOptions);
		// write to an external jpeg file a specified GPixelMap
		GError DoWrite(const GChar8 *FullFileName, const GElement& Element, const GDynArray<GImpExpOption>& ParsedOptions);

	public:
		// constructor
		GJpegImpExp();
		// constructor
		GJpegImpExp(const GElement* Owner);
		// destructor
		~GJpegImpExp();
		// get class ID
		const GClassID& ClassID() const {
			return G_JPEGIMPEXP_CLASSID;
		}
		// get derived class ID
		const GClassID& DerivedClassID() const {
			return G_IMPEXP_CLASSID;
		}
	};


	// *********************************************************************
	//                           GJpegImpExpProxy
	// *********************************************************************
	class G_PLUGIN_EXPORT GJpegImpExpProxy : public GElementProxy {
	public:
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GJpegImpExp(Owner);
		}
		// get class ID
		const GClassID& ClassID() const {
			return G_JPEGIMPEXP_CLASSID;
		}
		// get derived class ID
		const GClassID& DerivedClassID() const {
			return G_IMPEXP_CLASSID;
		}
	};

	static const GJpegImpExpProxy G_JPEGIMPEXP_PROXY;

};	// end namespace Amanith

#endif
