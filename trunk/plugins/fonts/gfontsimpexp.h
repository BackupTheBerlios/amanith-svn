/****************************************************************************
** $file: amanith/plugins/fonts/gfontimpext.h   0.1.1.0   edited Sep 24 08:00
**
** 2D Font import/export plugin interface.
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

#ifndef GFONTSIMPEXP_H
#define GFONTSIMPEXP_H

#include "amanith/gimpexp.h"

namespace Amanith {

	// *********************************************************************
	//                                GFontsImpExp
	// *********************************************************************
	static const GClassID G_FONTSIMPEXP_CLASSID = GClassID("GFontsImpExp", 0xC6134AB7, 0x7F62411D, 0xBEB87E4E, 0x1B306A5B);

	class G_PLUGIN_EXPORT GFontsImpExp : public GImpExp {

	private:
		//void *gFace;

		void AddFontsFeatures();

	protected:
		// initialize FreeType library
		static GError InitFTLibrary();
		// load a font file
		GError ReadFont(const GChar8 *FullFileName, const GElement& Element, const GBool OuterCCW,
						const GString& MetricsFile, const GReal Scale);

		// read a font file and fill a GFont with read data
		GError DoRead(const GChar8 *FullFileName, GElement& Element, const GDynArray<GImpExpOption>& ParsedOptions);
		// write to an external font file a specified GFont
		GError DoWrite(const GChar8 *FullFileName, const GElement& Element, const GDynArray<GImpExpOption>& ParsedOptions);

	public:
		// constructor
		GFontsImpExp();
		// constructor
		GFontsImpExp(const GElement* Owner);
		// destructor
		~GFontsImpExp();
		// get class ID
		const GClassID& ClassID() const {
			return G_FONTSIMPEXP_CLASSID;
		}
		// get derived class ID
		const GClassID& DerivedClassID() const {
			return G_IMPEXP_CLASSID;
		}
	};


	// *********************************************************************
	//                             GFontsImpExpProxy
	// *********************************************************************
	class G_PLUGIN_EXPORT GFontsImpExpProxy : public GElementProxy {
	public:
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GFontsImpExp(Owner);
		}
		// get class ID
		const GClassID& ClassID() const {
			return G_FONTSIMPEXP_CLASSID;
		}
		// get derived class ID
		const GClassID& DerivedClassID() const {
			return G_IMPEXP_CLASSID;
		}
	};

	static const GFontsImpExpProxy G_FONTSIMPEXP_PROXY;

};	// end namespace Amanith

#endif
