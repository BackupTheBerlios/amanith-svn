/****************************************************************************
** $file: amanith/support/gblowfish.h   0.1.0.0   edited Jun 30 08:00
**
** Blowfish encryption algorithm interface.
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

#ifndef GBLOWFISH_H
#define GBLOWFISH_H

#include "amanith/gglobal.h"

/*!
	\file gblowfish.h
	\brief Header file for GBlowFish class (encryption tool).
*/

namespace Amanith {

	// *********************************************************************
	//                              GBlowFish
	// *********************************************************************
	class G_EXPORT GBlowFish {
	private:
		GULong *PArray;
		GULong (* SBoxes)[256];
		void BlowfishEncipher(GULong *xl, GULong *xr);
		void BlowfishDecipher(GULong *xl, GULong *xr);

	public:
		//! Constructor
		GBlowFish();
		//! Destructor
		~GBlowFish();
		//! Initialize internal structures based on specified key string.
		void Initialize(const GUChar8 *Key, GInt32 KeyLength);
		GULong GetOutputLength(const GULong InputLong);
		//! Encode a buffer
		GULong Encode(GUChar8 *InputBuffer, GUChar8 *OutputBuffer, GULong InputSize);
		//! Decode a buffer
		void Decode(GUChar8 *InputBuffer, GUChar8 *OutputBuffer, GULong InputSize);

	};

};	// end namespace Amanith

#endif
