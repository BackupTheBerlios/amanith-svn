/****************************************************************************
** $file: amanith/geometry/gaxisangle.h   0.3.0.0   edited Jan, 30 2006
**
** Axis-angle definition of a rotation in 3D-space.
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

#ifndef GAXISANG_H_
#define GAXISANG_H_

#include "gvect.h"

/*!
	\file gaxisangle.h
	\brief GAxisAng class header file.
*/
namespace Amanith {

	/*!
		\class GAxisAng
		\brief GAxisAng represents a "twist about an axis", it is used to specify a rotation in 3D-space.

		To some people this rotation format can be more intuitive to specify than matrix, quaternion, Euler angles
		formatted rotation.	GAxisAng is very similar to GQuat, except it is human readable.\n
		For efficiency, you should use GQuat instead (GQuat or GMatrix are preferred).
		The internal data format is an array of 4 DATA_TYPE values. Angle is first, the axis is the last 3.

		\pre Angles are in radians, the axis is usually normalized by the user.
	*/
	template <typename DATA_TYPE>
	class GAxisAng : public GVectBase<DATA_TYPE, 4> {
	public:
		//! Default constructor. Initializes to identity rotation (no rotation).
		GAxisAng() : GVectBase<DATA_TYPE, 4>((DATA_TYPE)0.0, (DATA_TYPE)1.0, (DATA_TYPE)0.0, (DATA_TYPE)0.0) {
		}
		//! Copy constructor.
		GAxisAng(const GAxisAng& Source) : GVectBase<DATA_TYPE, 4>(Source) {
		} 
		//! Data constructor (angle / x, y, z). Angles are in radians.
		GAxisAng(const DATA_TYPE& RadAngle, const DATA_TYPE& X, const DATA_TYPE& Y, const DATA_TYPE& Z)
		: GVectBase<DATA_TYPE, 4>(RadAngle, X, Y, Z) {
		}
		//! Data constructor (angle / vector3). Angles are in radians.
		GAxisAng(const DATA_TYPE& RadAngle, const GVect<DATA_TYPE, 3>& Axis)
		: GVectBase<DATA_TYPE, 4>(RadAngle, Axis[0], Axis[1], Axis[2])	{
		}
		//! Set raw data, angles are in radians.
		void Set(const DATA_TYPE& RadAngle, const DATA_TYPE& X, const DATA_TYPE& Y, const DATA_TYPE& Z) {
			GVectBase<DATA_TYPE, 4>::Set(RadAngle, X, Y, Z);
		}  
		//! Set data, angles are in radians.
		void Set(const DATA_TYPE& RadAngle, const GVect<DATA_TYPE, 3>& Axis) {
			GVectBase<DATA_TYPE, 4>::Set(RadAngle, Axis[0], Axis[1], Axis[2]);
		}   
		/*!
			Set the axis portion of the AxisAngle.

			\param Axis the desired 3D vector axis to rotate about.
		*/
		void SetAxis(const GVect<DATA_TYPE, 3>& Axis) { 
			GVectBase<DATA_TYPE, 4>::operator[](1) = Axis[0];
			GVectBase<DATA_TYPE, 4>::operator[](2) = Axis[1];
			GVectBase<DATA_TYPE, 4>::operator[](3) = Axis[2];
		}
		   
		/*!
			Set the angle (twist) part of the AxisAngle, as a radian value.

			\param RadAngle the desired twist angle, in radians.
		*/
		void SetAngle(const DATA_TYPE& RadAngle) {
			GVectBase<DATA_TYPE, 4>::operator[](0) = RadAngle;
		}
		   
		/*!
			Get the axis portion of the AxisAngle.

			\return a vector of the axis, which may or may not be normalized.
		*/
		GVect<DATA_TYPE, 3> Axis() const {
			return GVect<DATA_TYPE, 3>(GVectBase<DATA_TYPE, 4>::operator[](1),
									   GVectBase<DATA_TYPE, 4>::operator[](2),
									   GVectBase<DATA_TYPE, 4>::operator[](3));
		}
		   
		/*!
			Get the angle (twist) part of the AxisAngle.

			\return the twist value in radians.
		*/
		const DATA_TYPE& Angle() const {
			return GVectBase<DATA_TYPE, 4>::operator[](0);
		}
	};
	
	//! Static axis-angle constant that specifies an identity rotation
	static const GAxisAng<GReal> G_AXISANGLE_IDENTITY(0, 1, 0, 0);
	//! Common axis-angle class, it uses GReal data type.
	typedef GAxisAng<GReal> GAxisAngle;

}; // end of namespace Amanith

#endif
