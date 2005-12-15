/****************************************************************************
** $file: amanith/1d/gpolylinecurve1d.h   0.2.0.0   edited Dec, 12 2005
**
** 1D Polyline curve segment definition.
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

#ifndef GPOLYLINECURVE1D_H
#define GPOLYLINECURVE1D_H

/*!
	\file gpolylinecurve1d.h
	\brief Header file for 1D polyline multicurve class.
*/

#include "amanith/1d/gmulticurve1d.h"

namespace Amanith {


	// *********************************************************************
	//                           GPolyLineCurve1D
	// *********************************************************************

	//! GPolyLineCurve1D static class descriptor.
	static const GClassID G_POLYLINECURVE1D_CLASSID = GClassID("GPolyLineCurve1D", 0xA67F1F38, 0x52E94BEC, 0x9496C506, 0x43FFC734);

	/*!
		\struct GPolyLineKey1D
		\brief 1D Polyline key structure.

		This structure represents a 1D polyline key. The key is described by its domain parameter and its point position.
	*/
	struct GPolyLineKey1D {
		//! Domain parameter.
		GReal Parameter;
		//! Key value.
		GReal Value;

		//! Default constructor, set domain parameter at 0 and point value at origin.
		GPolyLineKey1D() {
			Parameter = 0;
		}
		//! Set constructor, specifying domain parameter and the 2D point value.
		GPolyLineKey1D(const GReal _Parameter, const GReal _Value) : Parameter(_Parameter), Value(_Value) {
		}
		//! Copy constructor.
		GPolyLineKey1D(const GPolyLineKey1D& Source) {
			Parameter = Source.Parameter;
			Value = Source.Value;
		}
		//! Assignment operator.
		GPolyLineKey1D& operator =(const GPolyLineKey1D& Source) {
			Parameter = Source.Parameter;
			Value = Source.Value;
			return *this;
		}
	};


	/*!
		\class GPolyLineCurve1D
		\brief This class implements a 1D polyline multicurve.

		A polyline multicurve is a curve made of straight line traits.
	*/
	class G_EXPORT GPolyLineCurve1D : public GMultiCurve1D {

	private:
		//! Array of polyline keys that build the curve.
		GDynArray<GPolyLineKey1D> gKeys;

	protected:
		//! Sort keys, in ascending order respect to domain parameters.
		void SortKeys();
		//! Cloning function, copies (physically) a Source polyline curve into this curve.
		GError BaseClone(const GElement& Source);
		/*!
			Curve subdivision.

			Cuts the curve at specified parameter, and return left and right polyline arcs.

			\param u domain parameter specifying where to cut the curve.
			\param RightCurve if non-NULL, the function must return the right arc generated by cutting operation.
			\param LeftCurve if non-NULL, the function must return the left arc generated by cutting operation.
			\note The domain parameter is ensured to be completely inside the curve domain. Furthermore RightCurve and
			LeftCurve parameters, if specified, are ensured to be GPolyLineCurve1D classes (so cast is type-safe).
		*/
		GError DoCut(const GReal u, GCurve1D *RightCurve, GCurve1D *LeftCurve) const;
		/*!
			Add a new (key)point for the curve.

			\param Parameter the domain parameter where to add the (key)point.
			\param NewPoint if not specified (NULL value) the point is created on curve. In this case the specified
			domain Parameter must reside completely inside the domain, else an G_OUT_OF_RANGE error is returned.
			If Newpoint is specified (non NULL) the point is created also outside domain range.
			\param Index the position (internal index) occupied by the created (key)point.
			\param AlreadyExists a G_TRUE value means that the point has been created at a domain position already occupied
			by another (key)point. In this case the already existing point is overridden by the created point.
			\return G_NO_ERROR if the function succeeds, an error code otherwise.
		*/
		GError DoAddPoint(const GReal Parameter, const GReal *NewPoint, GUInt32& Index, GBool& AlreadyExists);
		/*!
			Remove a (key)point from the curve.
			Index is ensured to be valid and we are sure that after removing we'll have (at least) a minimal
			(2-(key)points made) multi-curve.

			\param Index the index of (key)point to be removed. Note that this value is ensured to be always valid.
			\return G_NO_ERROR if the function succeeds, an error code must be returned otherwise.
		*/
		GError DoRemovePoint(const GUInt32 Index);
		/*!
			Get domain parameter corresponding to specified (key)point index.
			Index is ensured to be valid.

			\param Index the point index, is ensured to be always valid.
			\param Parameter the outputted domain parameter, corresponding to the specified (key)point index.
			\return G_NO_ERROR if the function succeeds, an error code must be returned otherwise.
		*/
		GError DoGetPointParameter(const GUInt32 Index, GReal& Parameter) const;
		/*!
			Set domain parameter corresponding to specified (key)point index. The final effect is that a (key)point
			is moved from its domain position to another domain position.

			\param Index the point index, is ensured to be always valid.
			\param NewParamValue the new domain parameter, where to move the specified (key)point. It can be outside
			current domain.
			\param NewIndex the new position (internal index) occupied by the moved (key)point. This method	outputs
			this value.
			\param AlreadyExists this method writes to this flag a G_TRUE value if the point has been
			moved in a domain position already occupied by another (key)point. In this case the already existing
			point is overridden by the moved point.
			\return G_NO_ERROR if the function succeeds, an error code otherwise.
		*/
		GError DoSetPointParameter(const GUInt32 Index, const GReal NewParamValue,
								   GUInt32& NewIndex, GBool& AlreadyExists);

	public:
		//! Default constructor, constructs and empty polyline curve.
		GPolyLineCurve1D();
		//! Constructor with kernel specification, constructs and empty polyline curve.
		GPolyLineCurve1D(const GElement* Owner);
		//! Destructor, free all keys and other internal structures.
		~GPolyLineCurve1D();
		//! Clear the curve (remove keys, free internal structures and set an empty domain).
		void Clear();
		//! Returns number of key points.
		inline GUInt32 PointsCount() const {
			return (GUInt32)gKeys.size();
		}
		/*!
			Given a domain value, it returns the span index that includes it.

			\param Param the domain parameter
			\param KeyIndex the lower key index of the interval where Param is included. Param is contained
			in the interval [KeyIndex, KeyIndex+1)
			\return G_TRUE if the domain value is inside the current domain, G_FALSE otherwise.
			\note if Param is equal to DomainEnd(), then the index KeyIndex+1 is not valid. So you must
			check this case before calling ParamToKeyIndex.
		*/
		GBool ParamToKeyIndex(const GReal Param, GUInt32& KeyIndex) const;
		//! Get Index-th key point; Index must be valid, else a point with infinitive components is returned.
		GReal Point(const GUInt32 Index) const;
		//! Set Index-th (key)point; Index must be valid.
		GError SetPoint(const GUInt32 Index, const GReal NewPoint);
		/*!
			Construct a new polyline curve, specifying just interpolated (key)points.

			\param NewPoints the array of points of the curve. Each point will be interpolated (polyline curves
			passes through key points).
			\param NewMinValue the lower bound of the curve domain.
			\param NewMaxValue the upper bound of the curve domain.
			\param Uniform if G_TRUE, keys will be generated uniformly (spanning the new domain uniformly). If G_FALSE
			keys position will be calculated using a chord-length parametrization.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError SetPoints(const GDynArray<GReal>& NewPoints,
						 const GReal NewMinValue, const GReal NewMaxValue, const GBool Uniform = G_FALSE);
		/*!
			Get a key, specifying its index.

			\param Index the index of the key to be retrieved.
			\param KeyValue the variable where the requested key will be outputted.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError Key(const GUInt32 Index, GPolyLineKey1D& KeyValue) const;
		//! Get the internal keys array.
		const GDynArray<GPolyLineKey1D>& Keys() const {
			return gKeys;
		}
		/*!
			Build the polyline curve, specifying keys.

			\param NewKeys the keys that will constitute the curve. Can be unsorted by domain values.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note The specified keys array must contain at least 2 keys, else a G_INVALID_PARAMETER error code will be
			returned.
		*/
		GError SetKeys(const GDynArray<GPolyLineKey1D>& NewKeys);
		/*!
			Set a new geometric value.

			\param Index the index of the key we want to set. Must be valid.
			\param NewKeyValue the new value.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError SetKey(const GUInt32 Index, const GReal NewKeyValue);
		/*!
			Set curve domain.

			All (key)points domain positions will be scaled and shifted to respect the new domain.
			The reparametrized curve is geometrically the same curve but parametrically it is different. In particular
			internal knots (key domain values) are scale/shifted, and derivatives module (magnitudes) are scaled
			too.

			\param NewMinValue the lower bound of the new domain.
			\param NewMaxValue the upper bound of the new domain.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note if NewMinValue is greater than NewMaxValue they will be swapped, so any combination of values is valid.
		*/
		GError SetDomain(const GReal NewMinValue, const GReal NewMaxValue);
		/*!
			Returns the length of the curve between the 2 specified global domain values.

			\param u0 the lower bound of integral
			\param u1 the upper bound of integral
			\param MaxError the maximum relative error (precision) at witch we wanna calculate length.
			\return The length of curve, calculated in the domain interval [u0; u1].
			\note For polyline curves the MaxError parameter is not used, but still required for consistent public
			interface.
		*/
		GReal Length(const GReal u0, const GReal u1, const GReal MaxError = G_EPSILON) const;
		/*! 
			Return the curve value calculated at specified domain parameter.

			\param u the domain parameter at witch we wanna evaluate curve value.
			\note if specified domain parameter is out of domain, StartPoint() or EndPoint() are returned (depending of
			witch side the parameter is out).
		*/
		GReal Evaluate(const GReal u) const;
		/*!
			Return the curve derivative calculated at specified domain parameter.

			\param Order the order of derivative.
			\param u the domain parameter at witch we wanna evaluate curve derivative.
			\note specified domain parameter is clamped by domain interval.
		*/
		GReal Derivative(const GDerivativeOrder Order, const GReal u) const;
		/*!
			Return the curve derivative calculated at specified domain parameter. This method differs from
			the one of base GCurve1D class in the number of returned values. This is due to the possibility
			that the curve is continuous but not derivable (in the sense that left and right derivatives
			are different).

			\param Order the order of derivative.
			\param u the domain parameter at witch we wanna evaluate curve derivative.
			\param LeftDerivative the left derivative.
			\param RightDerivative the right derivative.
			\note specified domain parameter is clamped by domain interval.
		*/
		void DerivativeLR(const GDerivativeOrder Order, const GReal u,
						  GReal& LeftDerivative, GReal& RightDerivative) const;
		//! Get class descriptor.
		inline const GClassID& ClassID() const {
			return G_POLYLINECURVE1D_CLASSID;
		}
		//! Get base class (father class) descriptor.
		inline const GClassID& DerivedClassID() const {
			return G_MULTICURVE1D_CLASSID;
		}
	};


	// *********************************************************************
	//                        GPolyLineCurve1DProxy
	// *********************************************************************

	/*!
		\class GPolyLineCurve1DProxy
		\brief This class implements a GPolyLineCurve1D proxy (provider).

		This proxy provides the creation of GPolyLineCurve1D class instances.
	*/
	class G_EXPORT GPolyLineCurve1DProxy : public GElementProxy {
	public:
		//! Creates a new GPolyLineCurve1D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GPolyLineCurve1D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_POLYLINECURVE1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_MULTICURVE1D_CLASSID;
		}
	};
	//! Static proxy for GPolyLineCurve1D class.
	static const GPolyLineCurve1DProxy G_POLYLINECURVE1D_PROXY;

};	// end namespace Amanith

#endif
