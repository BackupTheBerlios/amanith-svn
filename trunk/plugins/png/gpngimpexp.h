/****************************************************************************
** $file: amanith/plugins/png/gpngimpext.h   0.2.0.0   edited Dec, 12 2005
**
** 2D Pixelmap PNG import/export plugin interface.
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

#ifndef GPNGIMPEXP_H
#define GPNGIMPEXP_H

#include "amanith/gimpexp.h"
#include "amanith/2d/gpixelmap.h"

namespace Amanith {

	// *********************************************************************
	//                                GPngImpExp
	// *********************************************************************
	static const GClassID G_PNGIMPEXP_CLASSID = GClassID("GPngImpExp", 0x2DA6D415, 0x88484320, 0xBA1BB2EC, 0xD0B206B7);

	class G_PLUGIN_EXPORT GPngImpExp : public GImpExp {

	private:
		void AddPngFeatures();
		GError RawPngLoad(const GChar8 *FileName, GInt32& Width, GInt32& Height, GPixelFormat& PixelFormat,
						  GUChar8 **OutBuffer, GInt32& NumPaletteEntries, GUInt32 **PaletteBuffer,
						  const GBool ExpandPalette);
		GError RawPngSave(const GChar8 *FileName, const GInt32 Width, const GInt32 Height,
						  const GPixelFormat PixelFormat, const GUChar8 *Buffer,
						  const GInt32 NumPaletteEntries, const GUInt32 *PaletteBuffer,
						  const GBool Interlaced);

	protected:
		GError ReadPng(const GChar8 *FullFileName, GElement& Element, const GBool ExpandPalette = false);
		GError WritePng(const GChar8 *FullFileName, const GElement& Element,
						const GBool Interlaced = G_FALSE);

		// read a jpeg file and fill a GPixelMap with read data
		GError DoRead(const GChar8 *FullFileName, GElement& Element, const GDynArray<GImpExpOption>& ParsedOptions);
		// write to an external jpeg file a specified GPixelMap
		GError DoWrite(const GChar8 *FullFileName, const GElement& Element, const GDynArray<GImpExpOption>& ParsedOptions);

	public:
		// constructor
		GPngImpExp();
		// constructor
		GPngImpExp(const GElement* Owner);
		// destructor
		~GPngImpExp();
		// get class ID
		const GClassID& ClassID() const {
			return G_PNGIMPEXP_CLASSID;
		}
		// get derived class ID
		const GClassID& DerivedClassID() const {
			return G_IMPEXP_CLASSID;
		}
	};


	// *********************************************************************
	//                            GPngImpExpProxy
	// *********************************************************************
	class G_PLUGIN_EXPORT GPngImpExpProxy : public GElementProxy {
	public:
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GPngImpExp(Owner);
		}
		// get class ID
		const GClassID& ClassID() const {
			return G_PNGIMPEXP_CLASSID;
		}
		// get derived class ID
		const GClassID& DerivedClassID() const {
			return G_IMPEXP_CLASSID;
		}
	};

	static const GPngImpExpProxy G_PNGIMPEXP_PROXY;

};	// end namespace Amanith

#endif
