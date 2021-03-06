/****************************************************************************
** $file: amanith/2d/ghermitecurve2d.h   0.3.0.0   edited Jan, 30 2006
**
** 2D Hermite curve segment definition.
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

#ifndef GHERMITECURVE2D_H
#define GHERMITECURVE2D_H

/*!
	\file ghermitecurve2d.h
	\brief Header file for 2D Hermite multicurve class.
*/

#include "amanith/2d/gmulticurve2d.h"
#include "amanith/2d/gbeziercurve2d.h"

namespace Amanith {


	// *********************************************************************
	//                           GHermiteCurve2D
	// *********************************************************************
	
	//! GHermiteCurve2D static class descriptor.
	static const GClassID G_HERMITECURVE2D_CLASSID = GClassID("GHermiteCurve2D", 0xF2D8D69F, 0x8413481D, 0xBBA1F099, 0x6F724027);

	/*!
		\struct GHermiteKey2D
		\brief Hermite key structure.

		This structure represents an Hermite key. The key is described by its domain parameter, point position, incoming
		tangent and outcoming tangent.
	*/
	struct GHermiteKey2D {
		//! Domain parameter.
		GReal Parameter;
		//! Key value, a 2D geometric point.
		GPoint2 Value;
		//! Incoming tangent vector.
		GVector2 InTangent;
		//! Outcoming tangent vector.
		GVector2 OutTangent;

		//! Default constructor, set domain parameter at 0, point value at origin, and null tangents.
		GHermiteKey2D() {
			Parameter = 0;
		}
		//! Set constructor, specifying domain parameter and key point; tangents are set to null values.
		GHermiteKey2D(const GReal _Parameter, const GPoint2& _Value) : Parameter(_Parameter), Value(_Value) {
		}
		//! Set constructor, specifying all data.
		GHermiteKey2D(const GReal _Parameter, const GPoint2& _Value, const GVector2& _InTangent,
					const GVector2& _OutTangent) : Parameter(_Parameter), Value(_Value), InTangent(_InTangent),
					OutTangent(_OutTangent) {
		}
		//! Copy constructor.
		GHermiteKey2D(const GHermiteKey2D& Source) {
			Parameter = Source.Parameter;
			Value = Source.Value;
			InTangent = Source.InTangent;
			OutTangent = Source.OutTangent;
		}
		//! Assignment operator.
		GHermiteKey2D& operator =(const GHermiteKey2D& Source) {
			Parameter = Source.Parameter;
			Value = Source.Value;
			InTangent = Source.InTangent;
			OutTangent = Source.OutTangent;
			return *this;
		}
	};


	/*!
		\class GHermiteCurve2D
		\brief This class implements a 2D Hermite multicurve.

		An Hermite multicurve is a curve made of Hermite traits. An Hermite trait is a parametric cubic curve, defined
		by two points and two tangent vectors, as you can see in the image below.
		\image html herm_trait.gif "Hermite trait example"

		This kind of curve is called 'cardinal', it interpolates the two point and the two tangents. From P0's point
		of view, T0 is called 'outcoming' tangent. From P1's point of view, T1 is called 'incoming' tangent.
	*/
	class G_EXPORT GHermiteCurve2D : public GMultiCurve2D {

	private:
		struct GHermiteCallBackData {
			const GHermiteCurve2D* Curve;
			GUInt32 KeyIndex;
			// constructor
			GHermiteCallBackData(const GHermiteCurve2D *_Curve, const GUInt32 _KeyIndex) {
				Curve = _Curve;
				KeyIndex = _KeyIndex;
			}
		};

		//! Array of Hermite keys that build the curve.
		GDynArray<GHermiteKey2D> gKeys;

	protected:
		//! Sort keys, in ascending order respect to domain parameters.
		void SortKeys();
		/*
			Get max variation (squared chordal distance) in the range [u0; u1]; here are necessary also
			curve evaluations at the interval ends.

			\param u0 lower bound of interested interval
			\param u1 upper bound of interested interval
			\param p0 the point corresponding to the curve evaluation at u0
			\param p1 the point corresponding to the curve evaluation at u1
			\note The interval is ensured to be completely inside the curve domain.
		
		GReal Variation(const GReal u0, const GReal u1,	const GPoint2& p0, const GPoint2& p1) const;*/

		//! Cloning function, copies (physically) a Source Hermite curve into this curve.
		GError BaseClone(const GElement& Source);
		/*!
			Curve subdivision.

			Cuts the curve at specified parameter, and return left and right Hermite arcs.

			\param u domain parameter specifying where to cut the curve.
			\param RightCurve if non-NULL, the function must return the right arc generated by cutting operation.
			\param LeftCurve if non-NULL, the function must return the left arc generated by cutting operation.
			\note The domain parameter is ensured to be completely inside the curve domain. Furthermore RightCurve and
			LeftCurve parameters, if specified, are ensured to be GHermiteCurve2D classes (so cast is type-safe).
		*/
		GError DoCut(const GReal u, GCurve2D *RightCurve, GCurve2D *LeftCurve) const;
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
		GError DoAddPoint(const GReal Parameter, const GPoint2 *NewPoint, GUInt32& Index, GBool& AlreadyExists);
		/*!
			Remove a (key)point from the curve.
			Index is ensured to be valid and we are sure that after removing we'll have (at least) a minimal
			(2-(key)points made) multi-curve.

			\param Index the index of (key)point to be removed. Note that this value is ensured to be always valid.
			\return G_NO_ERROR if the function succeeds, an error code must be returned otherwise.
		*/
		GError DoRemovePoint(const GUInt32 Index);
		/*!
			Calculate tangents using Catmull-Rom schema.
			
			\param Index0 the lower key index, where to begin to calculate tangents.
			\param Index1 the upper key index, where to end to calculate tangents.
		*/
		void CalcCatmullRomTangents(const GUInt32 Index0, const GUInt32 Index1);
		/*!
			Calculate the length of the Index-th Hermite trait, specifying a trait's subdomain range.
		*/
		GReal SegmentLength(const GUInt32 Index, const GReal MinParam, const GReal MaxParam,
							const GReal MaxError) const;
		/*!
			Calculate the curve value for a specified Index-th Hermite trait. The passed domain parameter must
			reside inside the trait's subdomain.
		*/
		GPoint2 SegmentEvaluate(const GUInt32 Index, const GReal Parameter) const;
		/*!
			Calculate the curve derivative for a specified Index-th Hermite trait. The passed domain parameter must
			reside inside the trait's subdomain. The order of derivative is specified through Order parameter.
		*/
		GVector2 SegmentDerivative(const GUInt32 Index, const GDerivativeOrder Order, const GReal Parameter) const;

		GVector2 SegmentTangent(const GUInt32 Index, const GDerivativeOrder Order, const GReal Parameter) const;
		/*
			Get max variation (squared chordal distance) for a specified Hermite trait in the
			range [MinParam; MaxParam].

			\param Index the index of Hermite trait that we test for ray intersection.
			\param MinParam the lower bound of interested domain range; it must be inside trait's subdomain
			\param MaxParam the upper bound of interested domain range; it must be inside trait's subdomain
			\return the estimated squared chordal distance.

		GReal SegmentVariation(const GUInt32 Index, const GReal MinParam, const GReal MaxParam) const;*/

		/*!
			Get max variation (squared chordal distance) for a specified Hermite trait in its domain.

			\param Index the index of Hermite trait that we test for ray intersection.
			\return the estimated squared chordal distance.
		*/
		GReal SegmentVariation(const GUInt32 Index) const;
		/*!
			Intersect an Hermite trait with a normalized ray, and returns a list of intersections.

			\param Index the index of Hermite trait that we test for ray intersection.
			\param NormalizedRay a normalized ray used for intersection test. Ray must be normalized, else incorrect
			results are possible.
			\param Intersections every found intersection will be appended to this array. Each intersection is a 2D
			vector; it has at position 0 the curve parameter (domain) value corresponding to the intersection, and at
			position 1 the ray parameter value corresponding to the intersection.
			\param Precision the precision used to find every solution.
			\param MaxIterations number of max iterations this method can loop for each found solution.
			If the current solution calculus does not reach the specified Precision within MaxIterations iterations, the
			current solution calculus is stopped, the solution is appended into output array, and calculus go for the
			next found solution. The more this value is, the more is the accuracy of each solution (and the more is the time taken from this
			method).
			\note Internally the Hermite trait is transformed into a temporary Bezier equivalent trait. This mapping
			makes possible to exploit all Bezier characteristics.
		*/
		GBool SegmentIntersectRay(const GUInt32 Index, const GRay2& NormalizedRay,
								  GDynArray<GVector2>& Intersections,
								  const GReal Precision, const GUInt32 MaxIterations) const;
		/*!
			Flats an Hermite trait, specifying a max error/variation (squared chordal distance).

			\param Index the index of Hermite trait that we want to flat.
			\param Contour a dynamic array where this function has to append generated points.
			\param MaxDeviation maximum squared chordal distance we wanna reach (maximum permitted deviation).
			\param IncludeLastPoint if G_TRUE the function must append last curve point (the point corresponding to
			domain upper bound parameter). If G_FALSE last point must not be included.
			\note internally the method converts the Hermite trait into a temporary Bezier equivalent trait, then
			call Bezier specific high-optimized flattening routines.
		*/
		GError SegmentFlatten(const GUInt32 Index, GDynArray<GPoint2>& Contour, const GReal MaxDeviation,
							  const GBool IncludeLastPoint) const;
		/*!
			This method do the actual Bezier conversion.

			\param Index the Hermite trait index to be converted. It must be valid.
			\param Result the resulting Bezier curve.
			\return G_NO_ERROR in operation succeeds, an error code otherwise.
		*/
		void SegmentToBezierConversion(const GUInt32 Index, GBezierCurve2D& Result) const;
		// static speed evaluation callback (useful for length evaluation)
		static GReal SegmentSpeedEvaluationCallBack(const GReal u, void *Data);
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
			\param NewIndex the new position (internal index) occupied by the moved (key)point. This method outputs
			this value.
			\param AlreadyExists this method writes to this flag a G_TRUE value if the point has been
			moved in a domain position already occupied by another (key)point. In this case the already existing
			point is overridden by the moved point.
			\return G_NO_ERROR if the function succeeds, an error code otherwise.
		*/
		GError DoSetPointParameter(const GUInt32 Index, const GReal NewParamValue,
								   GUInt32& NewIndex, GBool& AlreadyExists);

	public:
		//! Default constructor, constructs and empty Hermite curve.
		GHermiteCurve2D();
		//! Constructor with kernel specification, constructs and empty Hermite curve.
		GHermiteCurve2D(const GElement* Owner);
		//! Destructor, free all keys and other internal structures.
		~GHermiteCurve2D();
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
		GPoint2 Point(const GUInt32 Index) const;
		//! Set Index-th (key)point; Index must be valid.
		GError SetPoint(const GUInt32 Index, const GPoint2& NewPoint);
		/*!
			Construct a new Hermite curve, specifying just interpolated (key)points.
			Key tangents will be calculated using a Catmull-Rom schema.

			\param NewPoints the array of points of the curve. Each point will be interpolated (Hermite curves
			passes through key points).
			\param NewMinValue the lower bound of the curve domain.
			\param NewMaxValue the upper bound of the curve domain.
			\param Uniform if G_TRUE, keys will be generated uniformly (spanning the new domain uniformly). If G_FALSE
			keys position will be calculated using length of each Hermite trait.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError SetPoints(const GDynArray<GPoint2>& NewPoints,
						 const GReal NewMinValue, const GReal NewMaxValue, const GBool Uniform = G_FALSE);
		/*!
			Get a key, specifying its index.

			\param Index the index of the key to be retrieved.
			\param KeyValue the variable where the requested key will be outputted.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError Key(const GUInt32 Index, GHermiteKey2D& KeyValue) const;
		//! Get the internal keys array.
		const GDynArray<GHermiteKey2D>& Keys() const {
			return gKeys;
		}
		/*!
			Build the Hermite curve, specifying keys.

			\param NewKeys the keys that will constitute the curve. Can be unsorted by domain values.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note The specified keys array must contain at least 2 keys, else a G_INVALID_PARAMETER error code will be
			returned.
		*/
		GError SetKeys(const GDynArray<GHermiteKey2D>& NewKeys);
		/*!
			Set a new geometric value.

			\param Index the index of the key we want to set. Must be valid.
			\param NewKeyValue the new point 2D position.
			\param InTangent the new incoming 2D tangent vector.
			\param OutTangent the new outcoming 2D tangent vector.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError SetKey(const GUInt32 Index, const GPoint2& NewKeyValue, const GVector2& InTangent,
					  const GVector2& OutTangent);
		/*!
			Recalculate all tangents, using a smoothing schema (Catmull-Rom).

			\param SmoothEnds if G_TRUE, set first and last key tangents to the same value (useful for a looped
			curve).
			\note this method do nothing if there aren't at least 2 keys.
		*/
		void RecalcSmoothTangents(const GBool SmoothEnds = G_TRUE);
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
			Intersect the Hermite curve with a normalized ray, and returns a list of intersections.

			\param NormalizedRay a normalized ray used for intersection test. Ray must be normalized, else incorrect
			results are possible.
			\param Intersections every found intersection will be appended to this array. Each intersection is a 2D
			vector; it has at position 0 the curve parameter (domain) value corresponding to the intersection, and at
			position 1 the ray parameter value corresponding to the intersection.
			\param Precision the precision used to find every solution.
			\param MaxIterations number of max iterations this method can loop for each found solution.
			If the current solution calculus does not reach the specified Precision within MaxIterations iterations, the
			current solution calculus is stopped, the solution is appended into output array, and calculus go for the
			next found solution. The more this value is, the more is the accuracy of each solution (and the more is the time taken from this
			method).
			\note Internally every Hermite trait is transformed into a temporary Bezier equivalent trait. This mapping
			makes possible to exploit all Bezier characteristics.
		*/
		GBool IntersectRay(const GRay2& NormalizedRay, GDynArray<GVector2>& Intersections,
						   const GReal Precision = G_EPSILON, const GUInt32 MaxIterations = 100) const;
		/*!
			Returns the length of the curve between the 2 specified global domain values.

			The implementation take care of every Hermite trait. Because at each key point the curve can be (in general)
			only continuous (and not differentiable), we must calculate the length as the sum of single interested
			Hermite traits length.

			\param u0 the lower bound of integral
			\param u1 the upper bound of integral
			\param MaxError the maximum relative error (precision) at witch we wanna calculate length.
			\return The length of curve, calculated in the domain interval [u0; u1].
			\note Before integration, the specified interval is checked and clamped to be be valid for this curve domain.
		*/
		GReal Length(const GReal u0, const GReal u1, const GReal MaxError = G_EPSILON) const;
		/*! 
			Return the curve value calculated at specified domain parameter.

			\param u the domain parameter at witch we wanna evaluate curve value.
			\note if specified domain parameter is out of domain, StartPoint() or EndPoint() are returned (depending of
			witch side the parameter is out).
		*/
		GPoint2 Evaluate(const GReal u) const;
		/*!
			Return the curve derivative calculated at specified domain parameter.

			\param Order the order of derivative.
			\param u the domain parameter at witch we wanna evaluate curve derivative.
			\note specified domain parameter is clamped by domain interval.
		*/
		GVector2 Derivative(const GDerivativeOrder Order, const GReal u) const;
		/*!
			Return the curve derivative calculated at specified domain parameter. This method differs from
			the one of base GCurve2D class in the number of returned values. This is due to the possibility
			that the curve is continuous but not derivable (in the sense that left and right derivatives
			are different).

			\param Order the order of derivative.
			\param u the domain parameter at witch we wanna evaluate curve derivative.
			\param LeftDerivative the left derivative.
			\param RightDerivative the right derivative.
			\note specified domain parameter is clamped by domain interval.
		*/
		void DerivativeLR(const GDerivativeOrder Order, const GReal u,
						  GVector2& LeftDerivative, GVector2& RightDerivative) const;
		/*!
			Hermite to cubic Bezier conversion.

			This method converts an Hermite trait into an equivalent cubic Bezier representation. The conversion can be
			easily derived equaling their power series representations.

			\param Index the Hermite trait index to be converted (the trait will be the one defined by (key)points
			Index and Index+1). It must be valid.
			\param Result the resulting Bezier curve.
			\return G_NO_ERROR in operation succeeds, an error code otherwise.
		*/
		GError SegmentToBezier(const GUInt32 Index, GBezierCurve2D& Result);
		/*!
			Flats the curve specifying a max error/variation (squared chordal distance).

			\param Contour a dynamic array where this function has to append generated points.
			\param MaxDeviation maximum squared chordal distance we wanna reach (maximum permitted deviation).
			\param IncludeLastPoint if G_TRUE the function must append last curve point (the point corresponding to
			domain upper bound parameter). If G_FALSE last point must not be included.
			\note internally the method converts each Hermite trait into a temporary Bezier equivalent trait, then
			call Bezier specific high-optimized flattening routines.
		*/
		GError Flatten(GDynArray<GPoint2>& Contour, const GReal MaxDeviation, const GBool IncludeLastPoint = G_TRUE) const;
		/*!
			Get variation (squared chordal distance) in the current domain range.
		*/
		GReal Variation() const;
		/*!
			Apply an affine transformation to all key points and tangents.

			\param Matrix a 2x3 matrix, specifying the affine transformation.
			\note the leftmost 2x2 matrix contains the rotation/scale portion, the last column vector contains the
			translation.
		*/
		void XForm(const GMatrix23& Matrix);
		/*!
			Apply full transformation to all key points and tangents.

			\param Matrix a 3x3 matrix, specifying the transformation.
			\param DoProjection if G_TRUE the projective transformation (described by the last row vector of matrix) will
			be done. In this case all transformed vertexes will be divided by the last W component. If G_FALSE only the
			affine portion will be used for transformation, and no projective division will be executed.
		*/
		void XForm(const GMatrix33& Matrix, const GBool DoProjection = G_TRUE);
		//! Get class descriptor.
		inline const GClassID& ClassID() const {
			return G_HERMITECURVE2D_CLASSID;
		}
		//! Get base class (father class) descriptor.
		inline const GClassID& DerivedClassID() const {
			return G_MULTICURVE2D_CLASSID;
		}
	};


	// *********************************************************************
	//                        GHermiteCurve2DProxy
	// *********************************************************************

	/*!
		\class GHermiteCurve2DProxy
		\brief This class implements a GHermiteCurve2D proxy (provider).

		This proxy provides the creation of GHermiteCurve2D class instances.
	*/
	class G_EXPORT GHermiteCurve2DProxy : public GElementProxy {
	public:
		//! Creates a new GHermiteCurve2D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GHermiteCurve2D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_HERMITECURVE2D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_MULTICURVE2D_CLASSID;
		}
	};
	//! Static proxy for GHermiteCurve2D class.
	static const GHermiteCurve2DProxy G_HERMITECURVE2D_PROXY;

};	// end namespace Amanith

#endif
