/****************************************************************************
** $file: amanith/2d/gmulticurve2d.h   0.3.0.0   edited Jan, 30 2006
**
** 2D Base multicurve segment definition.
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

#ifndef GMULTICURVE2D_H
#define GMULTICURVE2D_H

/*!
	\file gmulticurve2d.h
	\brief Header file for 2D multicurve class.
*/

#include "amanith/2d/gcurve2d.h"

namespace Amanith {


	// *********************************************************************
	//                             GMultiCurve2D
	// *********************************************************************

	//! GMultiCurve2D static class descriptor.
	static const GClassID G_MULTICURVE2D_CLASSID = GClassID("GMultiCurve2D", 0x59BA6FA6, 0x62F943B2, 0xA5548655, 0xA4D349D1);

	/*!
		\class GMultiCurve2D
		\brief This class represents a parametric continuous piece-wise 2D curve.

		The main difference respect to a GCurve2D is that a multicurve is an "homogeneous" piece-wise continuous curve.
		A multicurve is made of curve traits of the same type, each trait is continuously joined to the previous and
		next ones. You can think at a multicurve as a key-based curve. At each key point the curve may or may not be
		globally derivable. But at least it's continuous.\n
		Some kind of multicurve can be useful for shape design (think for example at Hermite curves used in Inkscape
		software), others for animation purposes (think for example at TCB curves).
	*/
	class G_EXPORT GMultiCurve2D : public GCurve2D {

	protected:
		/*!
			Add a new (key)point for the curve. <b>This method must be implemented by all derived classes</b>.

			\param Parameter the domain parameter where to add the (key)point.
			\param NewPoint if not specified (NULL value) the point must be created on curve. In this case the specified
			domain Parameter must reside completely inside the domain, else an G_OUT_OF_RANGE error must be returned.
			If Newpoint is specified (non NULL) the point can be created also outside domain range.
			\param Index the position (internal index) occupied by the created (key)point.
			\param AlreadyExists this method must write to this flag a G_TRUE value if the point has been
			created in a domain position already occupied by another (key)point. In this case the already existing
			point must be overridden by the created point.
			\return G_NO_ERROR if the function succeeds, an error code must be returned otherwise.
		*/
		virtual GError DoAddPoint(const GReal Parameter, const GPoint2 *NewPoint, GUInt32& Index,
								  GBool& AlreadyExists) = 0;
		/*!
			Remove a (key)point from the curve.
			Index is ensured to be valid and we are sure that after removing we'll have (at least) a minimal
			(2-(key)points made) multi-curve. <b>This method must be implemented by all derived classes</b>.

			\param Index the index of (key)point to be removed. Note that this value is ensured to be always valid.
			\return G_NO_ERROR if the function succeeds, an error code must be returned otherwise.
		*/
		virtual GError DoRemovePoint(const GUInt32 Index) = 0;
		//! Cloning function; this implementation do nothing.
		GError BaseClone(const GElement& Source);
		/*!
			Get domain parameter corresponding to specified (key)point index.
			Index is ensured to be valid. <b>This method must be implemented by all derived classes</b>.

			\param Index the point index, is ensured to be always valid.
			\param Parameter the outputted domain parameter, corresponding to the specified (key)point index.
			\return G_NO_ERROR if the function succeeds, an error code must be returned otherwise.
		*/
		virtual GError DoGetPointParameter(const GUInt32 Index, GReal& Parameter) const = 0;
		/*!
			Set domain parameter corresponding to specified (key)point index. The final effect is that a (key)point
			is moved from its domain position to another domain position.
			<b>This method must be implemented by all derived classes</b>.

			\param Index the point index, is ensured to be always valid.
			\param NewParamValue the new domain parameter, where to move the specified (key)point. It must be outside
			current domain.
			\param NewIndex the new position (internal index) occupied by the moved (key)point. This method must
			output this value.
			\param AlreadyExists this method must write to this flag a G_TRUE value if the point has been
			moved in a domain position already occupied by another (key)point. In this case the already existing
			point must be overridden by the moved point.
			\return G_NO_ERROR if the function succeeds, an error code must be returned otherwise.
		*/
		virtual GError DoSetPointParameter(const GUInt32 Index, const GReal NewParamValue,
										   GUInt32& NewIndex, GBool& AlreadyExists) = 0;

	public:
		//! Default constructor, constructs and empty curve.
		GMultiCurve2D();
		//! Constructor with kernel specification, constructs and empty curve.
		GMultiCurve2D(const GElement* Owner);
		//! Destructor
		virtual ~GMultiCurve2D();
		/*!
			Get domain parameter corresponding to specified (key)point index.

			\param Index the point index, must be valid (else an G_OUT_OF_RANGE error will be returned).
			\param Parameter the outputted domain parameter, corresponding to the specified (key)point index.
			\return G_NO_ERROR if the function succeeds, an error code otherwise.
		*/
		GError PointParameter(const GUInt32 Index, GReal& Parameter) const;
		/*!
			Set domain parameter corresponding to specified (key)point index. The final effect is that a (key)point
			is moved from its domain position to another domain position.

			\param Index the point index, must be valid (else an G_OUT_OF_RANGE error will be returned).
			\param NewParamValue the new domain parameter, where to move the specified (key)point.
			\param NewIndex the new position (internal index) occupied by the moved (key)point.
			\param AlreadyExists if G_TRUE, it means that the point has been moved to a domain position already
			occupied by	another (key)point. In this case the already existing point will be overridden by the moved
			point.
			\return G_NO_ERROR if the function succeeds, an error code otherwise.
			\note if the specified new domain position is out of current domain range, then the current domain
			will be	update to include the specified domain value.
		*/
		GError SetPointParameter(const GUInt32 Index, const GReal NewParamValue,
								 GUInt32& NewIndex, GBool& AlreadyExists);
		/*!
			Add point on curve, at the specified domain parameter; in case of successful operation, the inserted point
			index (into internal array) will be returned.

			\param Parameter the domain parameter where to add the point on. It must be in the domain range, else
			an G_OUT_OF_RANGE error code will be returned.
			\param Index the position (internal index) occupied by the created (key)point.
			\param AlreadyExists if G_TRUE, it means that the point has been created in a domain position already
			occupied by	another (key)point. In this case the already existing point will be overridden by the created
			point.
			\return G_NO_ERROR if the function succeeds, an error code otherwise.
		*/
		GError AddPoint(const GReal Parameter, GUInt32& Index, GBool& AlreadyExists);
		/*!
			Add a new (key)point, at the specified domain parameter; in case of successful operation, the inserted point
			index (into internal array) will be returned.

			\param Parameter the domain parameter where to add the point on.
			\param Point the geometrical position of the new created point.
			\param Index the position (internal index) occupied by the created (key)point.
			\param AlreadyExists if G_TRUE, it means that the point has been created in a domain position already
			occupied by	another (key)point. In this case the already existing point will be overridden by the created
			point.
			\return G_NO_ERROR if the function succeeds, an error code otherwise.
			\note the specified domain value can be outside the current domain; in this case the domain will be
			updated to include the specified domain value.
		*/
		GError AddPoint(const GReal Parameter, const GPoint2& Point, GUInt32& Index, GBool& AlreadyExists);
		/*!
			Remove a (key)point from curve. The domain will be updated in the case that the specified point to be
			removed is first or last one.

			\param Index the index of the point to be removed. It must be valid, else a G_OUT_OF_RANGE error code
			will be returned.
			\note If the curve were made of just 2 (key)points, calling this function will clear the entire
			curve, because a multicurve must be composed of at least 2 points.
		*/
		GError RemovePoint(const GUInt32 Index);
		/*!
			Return the curve derivative calculated at specified domain parameter. This method differs from
			the one of base GCurve2D class in the number of returned values. This is due to the possibility
			that the curve is continuous but not derivable (in the sense that left and right derivatives
			are different).

			\param Order the order of derivative.
			\param u the domain parameter at witch we wanna evaluate curve derivative.
			\param LeftDerivative the left derivative.
			\param RightDerivative the right derivative.
			\note specified domain parameter is clamped by domain interval; the default behavior is to return
			LeftDerivative = RightDerivative = Derivative(Order, u).
		*/
		virtual void DerivativeLR(const GDerivativeOrder Order, const GReal u,
								  GVector2& LeftDerivative, GVector2& RightDerivative) const {
			LeftDerivative = RightDerivative = this->Derivative(Order, u);
		}
		/*!
			Return the curve tangent calculated at specified domain parameter. This method differs from
			the one of base GCurve2D class in the number of returned values. This is due to the possibility
			that the curve is continuous but not derivable (in the sense that left and right derivatives
			are different).

			\param u the domain parameter at witch we wanna evaluate curve tangent(s).
			\param LeftTangent the left normalized (unit length) tangent vector.
			\param RightTangent the right normalized (unit length) tangent vector.
			\note specified domain parameter is clamped by domain interval.
		*/
		void TangentLR(const GReal u, GVector2& LeftTangent, GVector2& RightTangent) const;
		/*!
			Return the curve normal calculated at specified domain parameter. This method differs from
			the one of base GCurve2D class in the number of returned values. This is due to the possibility
			that the curve is continuous but not derivable (in the sense that left and right derivatives
			are different).

			\param u the domain parameter at witch we wanna evaluate curve normal(s).
			\param LeftNormal the left normalized vector perpendicular to the curve (left)tangent.
			\param RightNormal the right normalized vector perpendicular to the curve (right)tangent.
			\note specified domain parameter is clamped by domain interval.
		*/
		void NormalLR(const GReal u, GVector2& LeftNormal, GVector2& RightNormal) const;
		/*!
			Return the curve curvature calculated at specified domain parameter. This method differs from
			the one of base GCurve2D class in the number of returned values. This is due to the possibility
			that the curve is continuous but not derivable (in the sense that left and right derivatives
			are different).

			\param u the domain parameter at witch we wanna evaluate curve curvature(s).
			\param LeftCurvature the left curvature.
			\param RightCurvature the right curvature.
			\note specified domain parameter is clamped by domain interval.
		*/
		void CurvatureLR(const GReal u, GReal& LeftCurvature, GReal& RightCurvature) const;
		/*!
			Return the curve speed calculated at specified domain parameter. 
			With 'speed', here's intended the length of the curve's first derivative vector.
			This method differs from the one of base GCurve2D class in the number of returned
			values. This is due to the possibility that the curve is continuous but not derivable (in the
			sense that left and right derivatives are different).

			\param u the domain parameter at witch we wanna evaluate curve speed(s).
			\param LeftSpeed the speed corresponding to the left derivative vector.
			\param RightSpeed the speed corresponding to the right derivative vector.
			\note specified domain parameter is clamped by domain interval.
		*/
		void SpeedLR(const GReal u, GReal& LeftSpeed, GReal& RightSpeed) const;

		//! Get class descriptor
		inline const GClassID& ClassID() const {
			return G_MULTICURVE2D_CLASSID;
		}
		//! Get base class (father class) descriptor
		inline const GClassID& DerivedClassID() const {
			return G_CURVE2D_CLASSID;
		}
	};


	// *********************************************************************
	//                           GMultiCurve2DProxy
	// *********************************************************************

	/*!
		\class GMultiCurve2DProxy
		\brief This class implements a GMultiCurve2D proxy (provider).

		This proxy does not override CreateNew() method because we don't wanna make a creation of a GMultiCurve2D
		class possible (because of pure virtual  methods).
	*/
	class G_EXPORT GMultiCurve2DProxy : public GElementProxy {
	public:
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_MULTICURVE2D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_CURVE2D_CLASSID;
		}
	};
	//! Static proxy for GMultiCurve2D class.
	static const GMultiCurve2DProxy G_MULTICURVE2D_PROXY;

};	// end namespace Amanith

#endif
