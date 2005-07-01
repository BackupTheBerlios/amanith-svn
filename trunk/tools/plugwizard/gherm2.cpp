/****************************************************************************
** $file: D:/Projects/libraries/amanith/tools/plugwizard/gherm2.cpp   0.1.0   edited giu 22 12:32
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

#include "gherm2.h"

/*!
	\file gherm2.cpp
	\brief Implementation file for GHerm2 class.
*/

namespace Amanith {

// *********************************************************************
//                                GHerm2
// *********************************************************************

// constructor
GHerm2::GHerm2() : GHermiteCurve2D() {
}

// constructor
GHerm2::GHerm2(const GElement* Owner) : GHermiteCurve2D(Owner) {
}

// destructor
GHerm2::~GHerm2() {
}

// cloning function
GError GHerm2::BaseClone(const GElement& Source) {

	// don't forget to do a real implementation!
	const GHerm2& src = (const GHerm2&)Source;

	// continue cloning passing the control to base (father) class
	GHermiteCurve2D::BaseClone(Source);
}

}; // end namespace Amanith
