/****************************************************************************
** $file: amanith/geometry/gaffineparts.h   0.1.1.0   edited Sep 24 08:00
**
** Affine parts decomposition.
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

#ifndef GAFFINEPARTS_H
#define GAFFINEPARTS_H

#include "amanith/geometry/gquat.h"
#include "amanith/geometry/gvect.h"
#include "amanith/geometry/gmatrix.h"
#include "amanith/numerics/geigen.h"

/*!
	\file gaffineparts.h
	\brief Affine parts decomposition header file.
*/
namespace Amanith {

	/*!
		\class GAffineParts
		\brief Affine matrix polar decomposition

		Decompose 3 x 3, 3 x 4 and 4 x 4 affine matrices as TFRUK(U'), where T contains the
		translation components, R contains the main rotation, U contains space where scaling is done, K contains
		scale factors, and F contains the sign of the determinant.
		Assumes matrix transforms column vectors in right-handed coordinates.\n
		See Ken Shoemake and Tom Duff. Matrix Animation and Polar Decomposition. Proceedings of Graphics Interface 1992.
	*/
	class G_EXPORT GAffineParts {

	private:
		//! Translation components
		GVector3 gTrans;
		//! Essential rotation
		GQuaternion gMainRot;
		//! Stretch rotation
		GQuaternion gStretchRot;
		//! Stretch factors
		GVector3 gStretchFactors;
		//! Sign of determinant
		GReal gDetSign;

	protected:
		/*
			Spectral Axis Adjustment\n
			Given a unit quaternion, q, and a scale vector, k, find a unit quaternion, p,
			which permutes the axes and turns freely in the plane of duplicate scale
			factors, such that q p has the largest possible w component, i.e. the
			smallest possible angle. Permutes k's components to go with q p instead of q.
			See Ken Shoemake and Tom Duff. Matrix Animation and Polar Decomposition.
			Proceedings of Graphics Interface 1992. Details on p. 262-263.
		*/
		GQuaternion Snuggle(GQuaternion& q, GVector3& k);
		//! Do the affine decomposition
		void DecompAffine(const GMatrix33& AffineMatrix);

	public:
		//! Constructor for 3x3 matrices
		GAffineParts(const GMatrix33& AffineMatrix);
		//! Constructor for 3x4 matrices
		GAffineParts(const GMatrix34& AffineMatrix);
		//! Constructor for 4x4 matrices
		GAffineParts(const GMatrix44& AffineMatrix);
		//! Get translation component
		inline GVector3 Translation() const {
			return gTrans;
		}
		//! Get main rotation
		inline GQuaternion MainRotation() const {
			return gMainRot;
		}
		//! Get stretch rotation, it specifies the space where scaling is done
		inline GQuaternion StretchRotation() const {
			return gStretchRot;
		}
		//! Get stretch factors
		inline GVector3 StretchFactors() const {
			return gStretchFactors;
		}
		//! Get sign of determinant
		inline GReal DeterminantSign() const {
			return gDetSign;
		}
	};

};	// end namespace Amanith

#endif
