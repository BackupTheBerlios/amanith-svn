/****************************************************************************
** $file: D:/Projects/libraries/amanith/tools/plugwizard/gherm2.h   0.1.0.0   edited giu 22 12:32
**
** --Put file description here--
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

#ifndef GHERM2_H
#define GHERM2_H

#include "ghermitecurve2d.h"

/*!
	\file gherm2.h
	\brief Header file for GHerm2 class.
*/

namespace Amanith {

	// *********************************************************************
	//                                GHerm2
	// *********************************************************************

	//! GHerm2 static class descriptor.
	static const GClassID G_HERM2_CLASSID = GClassID("GHerm2", 0xB4203BA2, 0x8FB04842, 0x8290FDAC, 0x148F564A);

	class G_EXPORT GHerm2 : public GHermiteCurve2D {

	protected:
		//! Cloning function
		GError BaseClone(const GElement& Source);

	public:
		//! Default constructor
		GHerm2();
		//! Constructor with owner (kernel) specification
		GHerm2(const GElement* Owner);
		//! Destructor
		~GHerm2();
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_HERM2_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_HERMITECURVE2D_CLASSID;
		}
	}; // end class GHerm2


	// *********************************************************************
	//                              GHerm2Proxy
	// *********************************************************************

	/*!
		\class GHerm2Proxy
		\brief This class implements a GHerm2 proxy (provider).
	*/
	class G_EXPORT GHerm2Proxy : public GElementProxy {
	public:
		/*!
			Create a new element.

			\param Owner the owner (kernel) of the created element. Default value is NULL.
			\return A new created element if operation succeeds, else a NULL pointer.
		*/
		GElement* CreateNew(const GElement* Owner = NULL) {
			return new GHerm2(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_HERM2_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_HERMITECURVE2D_CLASSID;
		}
	}; // end class GHerm2Proxy

	//! Static proxy for GHerm2 class.
	static const GHerm2Proxy G_HERM2_PROXY;

}; // end namespace Amanith

#endif
