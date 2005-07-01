/****************************************************************************
** $file: amanith/gclassid.h   0.1.0.0   edited Jun 30 08:00
**
** Class identifier definition.
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

#ifndef GCLASSID_H
#define GCLASSID_H

/*!
	\file gclassid.h
	\brief Amanith class descriptor, used by the introspection system.
*/

#include "support/gutilities.h"

namespace Amanith {

	// *********************************************************************
	//                              GClassID
	// *********************************************************************

	/*! \class GClassID
		\brief Amanith class descriptor, used by the introspection system.

		GClassID represents a class descriptor, and it's used to describe a class.\n
		Description includes a numeric unique identifier (made of 4 integer numbers)
		and a class name.
	*/
	class G_EXPORT GClassID {

	private:
		//! Numeric unique identifier, made by 4 integer numbers.
		GUInt32 gID1, gID2, gID3, gID4;
		//! Class name
		GString gIDName;
	
	public:
		//! Empty Constructor
		GClassID() : gID1(0xFFFFFFFF), gID2(0xFFFFFFFF), gID3(0xFFFFFFFF), gID4(0xFFFFFFFF),
					 gIDName("") {
		}
		//! Set constructor
		GClassID(const GChar8 *NewIDName,
				 const GUInt32 NewID1, const GUInt32 NewID2, const GUInt32 NewID3, const GUInt32 NewID4)
				 : gID1(NewID1), gID2(NewID2), gID3(NewID3), gID4(NewID4) {
			 gIDName = StrUtils::Purge(NewIDName, "|£$%&/=ËÈ‡ÏÚ˘ß∞Á^()[]{}");
		}
		//! Copy constructor
		inline GClassID(const GClassID& CID) {
			CopyFrom(CID);
		}
		//! Destructor
		~GClassID() {
		}
		//! Copy operation
		void CopyFrom(const GClassID& CID) {
			gID1 = CID.gID1;
			gID2 = CID.gID2;
			gID3 = CID.gID3;
			gID4 = CID.gID4;
			gIDName = CID.gIDName;
		}
		//! Class name identifier
		inline const GChar8 *IDName() const {
			return StrUtils::ToAscii(gIDName);
		}
		//! Equal operator
		inline GBool operator ==(const GClassID& CID) const {
			if ((gID1 != CID.gID1) || (gID2 != CID.gID2) || (gID3 != CID.gID3) || (gID4 != CID.gID4))
				return 0;
			return 1;
		}
		//! Not equal operator
		inline GBool operator !=(const GClassID& CID) const {
			if ((gID1 != CID.gID1) || (gID2 != CID.gID2) || (gID3 != CID.gID3) || (gID4 != CID.gID4))
				return 1;
			return 0;
		}
		//! Assignment operator
		inline GClassID& operator =(const GClassID& CID) {
			gID1 = CID.gID1;
			gID2 = CID.gID2;
			gID3 = CID.gID3;
			gID4 = CID.gID4;
			gIDName = CID.gIDName;
			return *this;
		}
		//! Static NULL class id
		static const GClassID Null;
		//! Returns is this class descriptor is NULL (it means that it not describe any class)
		inline GBool IsNull() const {
			if (operator ==(GClassID::Null))
				return G_TRUE;
			return G_FALSE;
		}
	};

};	// end namespace Amanith

#endif
