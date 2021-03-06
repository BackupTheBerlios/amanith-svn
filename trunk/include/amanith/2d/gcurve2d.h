/****************************************************************************
** $file: amanith/2d/gcurve2d.h   0.3.0.0   edited Jan, 30 2006
**
** 2D Base curve segment definition.
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

#ifndef GCURVE2D_H
#define GCURVE2D_H

#include "amanith/gelement.h"
#include "amanith/geometry/gintersect.h"
#include "amanith/geometry/gmatrix.h"
#include "amanith/1d/gcurve1d.h"  // just to import GDerivativeOrder type

/*!
	\file gcurve2d.h
	\brief Header file for 2D curve class.
*/
namespace Amanith {


	// *********************************************************************
	//                             GCurve2D
	// *********************************************************************

	//! GCurve2D static class descriptor.
	static const GClassID G_CURVE2D_CLASSID = GClassID("GCurve2D", 0xCBE1A79C, 0xDE2249A0, 0xA9D39BC0, 0x4E6AA052);

	/*!
		\class GCurve2D
		\brief This class represents a generic parametric continuous 2D curve.

		Formally a 2D parametric curve is a function with a real domain and RxR codomain.

		\f[
			C: [a; b] \rightarrow R^2
		\f]
		
		where [a; b] is a sub-set of R.\n
		So given a number belonging to domain, C is a function that give you a 2D point.
		This class implements some useful differential calculus like Curvature(), Length(), Tangent(), Speed(),
		Variation() and so on.
		Some geometric tools are implemented too, like Cut(), Translate(), Rotate(), Scale(), XForm(), IntersectRay() and
		so on.
		Every curve can be flattened. Flatten operation converts the curve into straight-linesegments. By specifying
		a flatness parameter, the application has control over the number of straight-line segments	used to
		approximate curves. This is very useful for rendering (ex: in OpenGL).
	*/
	class G_EXPORT GCurve2D : public GElement {

	private:
		//! Curve's domain.
		GInterval<GReal> gDomain;

	protected:
		/*
			Get max variation (squared chordal distance) in the range [u0; u1]; here are necessary also
			curve evaluations at the interval ends.

			\param u0 lower bound of interested interval.
			\param u1 upper bound of interested interval.
			\param p0 the point corresponding to the curve evaluation at u0.
			\param p1 the point corresponding to the curve evaluation at u1.
			\note The interval is ensured to be completely inside the curve domain.	<b>This method must be implemented
			by every derived classes</b>.
		
		virtual GReal Variation(const GReal u0, const GReal u1,	const GPoint2& p0, const GPoint2& p1) const = 0;*/

		/*
			Flats the curve specifying a max error/variation (squared chordal distance).

			The default behavior is to split the curve at midpoint, and then call recursively this function on
			both curve arcs until maximum permitted deviation has been reached.
			If you wanna change this behavior just override this function.

			\param u0 lower bound of interested interval
			\param u1 upper bound of interested interval
			\param p0 the point corresponding to the curve evaluation at u0
			\param p1 the point corresponding to the curve evaluation at u1
			\param Contour a dynamic array where this function has to append generated points
			\param MaxDeviation maximum squared chordal distance we wanna reach (maximum permitted deviation).
			\note The interval is ensured to be completely inside the curve domain.
		
		virtual GError Flatten(const GReal u0, const GReal u1, const GPoint2& p0, const GPoint2& p1,
							   GDynArray<GPoint2>& Contour, const GReal MaxDeviation) const;*/
		/*!
			Curve subdivision.

			Cuts the curve at specified parameter, and return left and right arcs.

			\param u domain parameter specifying where to cut the curve.
			\param RightCurve if non-NULL, the function must return the right arc generated by cutting operation.
			\param LeftCurve if non-NULL, the function must return the left arc generated by cutting operation.
			\note The domain parameter is ensured to be completely inside the curve domain. Furthermore RightCurve and
			LeftCurve parameters, if specified, are ensured to be of the same type of this curve.\n
			<b>This method must be implemented	by every derived classes</b>.
		*/
		virtual GError DoCut(const GReal u, GCurve2D *RightCurve, GCurve2D *LeftCurve) const = 0;
		//! Cloning function; this implementation copies the domain.
		GError BaseClone(const GElement& Source);
		//! Static speed evaluation callback (for Length() evaluation).
		static GReal SpeedEvaluationCallBack(const GReal u, void *Data);

	public:
		//! Default constructor, constructs and empty curve.
		GCurve2D();
		//! Constructor with kernel specification, constructs and empty curve.
		GCurve2D(const GElement* Owner);
		//! Destructor, it calls Clear() function.
		virtual ~GCurve2D();
		//! Get domain lower bound (it corresponds to the start point).
		inline GReal DomainStart() const {
			return gDomain.Start();
		}
		//! Get domain upper bound (it corresponds to the end point).
		inline GReal DomainEnd() const {
			return gDomain.End();
		}
		//! Get curve domain.
		inline const GInterval<GReal>& Domain() const {
			return gDomain;
		}
		//! Set curve domain
		virtual GError SetDomain(const GReal NewMinValue, const GReal NewMaxValue) {
			gDomain.Set(NewMinValue, NewMaxValue);
			return G_NO_ERROR;
		}
		//! Returns number of points of the curve (typically control points).
		virtual GUInt32 PointsCount() const = 0;
		/*!
			Clear the curve, it makes this curve to be 'empty'.

			The default implementation is to set internal domain as [-inf, -inf].
			Every further implementation should be done ensuring that 2 or more	consecutive calls to this
			method do not make the class to crash.

			\note <b>this method must be implemented by all derived classes if some other internal structures or
			data have been allocated</b>.
		*/
		virtual void Clear();
		//! Get Index-th point. <b>This method must be implemented by all derived classes</b>.
		virtual GPoint2 Point(const GUInt32 Index) const = 0;
		//! Set Index-th point. <b>This method must be implemented by all derived classes</b>.
		virtual GError SetPoint(const GUInt32 Index, const GPoint2& NewValue) = 0;
		/*!
			Intersect the curve with a ray, and returns a list of intersections.

			\param NormalizedRay a normalized ray used for intersection test. If the ray has not been
			normalized incorrect results are possible.
			\param Intersections every found intersection will be appended to this array. Each intersection is a 2D
			vector; it has at position 0 the curve parameter (domain) value corresponding to the intersection, and at
			position 1 the ray parameter value corresponding to the intersection.
			\param Precision the precision used to find every solution.
			\param MaxIterations number of max iterations this method can loop for each found solution.
			If the current solution calculus does not reach the specified Precision within MaxIterations iterations, the
			current solution calculus is stopped, the solution is appended into output array, and calculus go for the
			next found solution.
			The more this value is, the more is the accuracy of each solution (and the more is the time taken from this
			method).
			\note <b>this method must be implemented by all derived classes</b>.
		*/
		virtual GBool IntersectRay(const GRay2& NormalizedRay, GDynArray<GVector2>& Intersections,
								   const GReal Precision = G_EPSILON, const GUInt32 MaxIterations = 100) const = 0;
		/*!
			Flats the curve specifying a max error/variation (squared chordal distance).

			\param Contour a dynamic array where this function has to append generated points.
			\param MaxDeviation maximum squared chordal distance we wanna reach (maximum permitted deviation).
			\param IncludeLastPoint if G_TRUE the function must append last curve point (the point corresponding to
			domain upper bound parameter). If G_FALSE last point must not be included.
			\note <b>this method must be implemented by all derived classes</b>.
		*/
		virtual GError Flatten(GDynArray<GPoint2>& Contour, const GReal MaxDeviation,
							   const GBool IncludeLastPoint = G_TRUE) const = 0;
		/*! 
			Return the curve value calculated at specified domain parameter.
			
			\param u the domain parameter at witch we wanna evaluate curve value.
			\note <b>this method must be implemented by all derived classes</b>.
		*/
		virtual GPoint2 Evaluate(const GReal u) const = 0;
		/*! 
			Return the curve derivative calculated at specified domain parameter.

			\param Order the order of derivative
			\param u the domain parameter at witch we wanna evaluate curve derivative.
			\note <b>this method must be implemented by all derived classes</b>.
		*/
		virtual GVector2 Derivative(const GDerivativeOrder Order, const GReal u) const = 0;
		/*!
			Giving CurvePos = Length(t), this function solves for t = Inverse(Length(s))

			This function uses the Newton method to find the numerical root solution. This method has a quadratic
			convergence.

			\param Result the solution, the domain parameter that correspond to the specified position on curve.
			\param CurvePos the position along the curve track.
			\param MaxError the wanted result precision, it must be a positive value. The less this parameter is, the
			more iteration have to be done by this method.
			\param MaxIterations the number of max iterations. If the specified MaxError has not been reached in
			MaxIterations iterations, this method exits.
			\return G_TRUE if solution has been found (respecting MaxError parameter) before MaxIterations iterations.
			G_FALSE otherwise.
			\note If CurvePos is less than 0, the lower bound of domain in returned. If CurvePos is greater than
			total curve length, then the upper bound of domain is returned.
		*/
		virtual GBool GlobalParameter(GReal& Result, const GReal CurvePos, const GReal MaxError = G_EPSILON,
									  const GUInt32 MaxIterations = 100) const;
		/*!
			Returns the length of the curve between the 2 specified domain parameter values.
			
			Formally, a length of a differentiable curve is the finite integral of first derivative module, over the
			desired interval:

			\f[
				L=\int_{u_0}^{u_1} |{C^.}(t)| dt
			\f]


			This implementation uses a Romberg integration schema.

			\param u0 the lower bound of integral
			\param u1 the upper bound of integral
			\param MaxError the maximum relative error (precision) at witch we wanna calculate length.
			\return The length of curve, calculated in the domain interval [u0; u1].
			\note Before integration, the specified interval is checked and clamped to be be valid for this curve domain.\n
			This method is virtual because for some specific curves (ex: polylines) this implementation couldn't be the
			faster one nor the correct (this implementation require a differentiable curve to be correct).
		*/
		virtual GReal Length(const GReal u0, const GReal u1, const GReal MaxError = G_EPSILON) const;
		//! Returns the total length of the curve. It uses Length() function passing it the whole domain as interval.
		GReal TotalLength(const GReal MaxError = G_EPSILON) const {
			return Length(DomainStart(), DomainEnd(), MaxError);
		}
		/*!
			Cuts (subdivides) the curve at specified domain parameter, and return left and right arcs.
			
			\param u domain parameter specifying where to cut the curve.
			\param RightCurve if non-NULL, the function must return the right arc generated by cutting operation.
			\param LeftCurve if non-NULL, the function must return the left arc generated by cutting operation.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note if specified domain parameter is out of the domain, an G_OUT_OF_RANGE error code is returned.
			Furthermore RightCurve and LeftCurve parameters, if specified, must be of the same type of this curve.
		*/			
		GError Cut(const GReal u, GCurve2D *RightCurve, GCurve2D *LeftCurve) const;
		/*!
			Cuts a curve slice corresponding to specified domain interval, and return the curve arc.

			\param u0 lower domain parameter specifying where to begin to cut the curve.
			\param u1 upper domain parameter specifying where to end to cut the curve.
			\param OutCurve the curve arc generated by cutting operation.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note specified domain interval is clamped by the valid interval for this curve. OutCurve parameter must
			be of the same type of this curve. If u0 > u1 they will be swapped before doing cut operation (so, for
			example, Cut(0.1, 0.8) is equal to (0.8, 0.1).
		*/
		GError Cut(const GReal u0, const GReal u1, GCurve2D *OutCurve) const;
		/*!
			Cuts (subdivides) the curve at specified curve (length) parameter, and return left and right arcs.

			\param CurvePos the curve parameter specifying where to cut the curve.
			\param RightCurve if non-NULL, the function must return the right arc generated by cutting operation.
			\param LeftCurve if non-NULL, the function must return the left arc generated by cutting operation.
			\param MaxError the precision to use to do the inverse mapping between curve position and domain value.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note if specified curve parameter is out of the length-domain, an G_OUT_OF_RANGE error code is returned.
			Furthermore RightCurve and LeftCurve parameters, if specified, must be of the same type of this curve.
		*/
		GError CutByLength(const GReal CurvePos, GCurve2D *RightCurve, GCurve2D *LeftCurve,
						   const GReal MaxError = G_EPSILON) const;
		/*!
			Cuts a curve slice corresponding to specified curve (length) interval, and return the curve arc.

			\param CurvePos0 lower curve (length) parameter specifying where to begin to cut the curve.
			\param CurvePos1 upper curve (length) parameter specifying where to end to cut the curve.
			\param Curve the curve arc generated by cutting operation.
			\param MaxError the precision to use to do the inverse mapping between curve position and domain value.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note specified domain interval is clamped by the valid interval for this curve. OutCurve parameter must
			be of the same type of this curve.
		*/
		GError CutByLength(const GReal CurvePos0, const GReal CurvePos1, GCurve2D *Curve,
						   const GReal MaxError = G_EPSILON) const;
		/*!
			Get tangent vector, specifying domain parameter.

			The returned value is the normalized (unit length) tangent vector.
		*/
		GVector2 Tangent(const GReal u) const;
		/*!
			Get curve normal, specifying domain parameter.
			
			The returned value is a normalized vector perpendicular to the curve Tangent().
		*/
		GVector2 Normal(const GReal u) const;
		/*!
			Get curve curvature, specifying domain parameter.\n For detailed information about curvature, please check this
			site http://planetmath.org/encyclopedia/CurvatureOfACurve.html
		*/
		GReal Curvature(const GReal u) const;
		/*!
			Get curve speed, specifying domain parameter.

			With 'speed', here's intended the length of the curve's first derivative vector.
		*/
		GReal Speed(const GReal u) const;
		/*!
			Get variation (squared chordal distance) in the current domain range.

			\note <b>This method must be implemented by all derived classes</b>.
		*/
		virtual GReal Variation() const = 0;
		//! Get start point of curve; this is the point corresponding to the domain lower bound.
		inline GPoint2 StartPoint() const {
			return Point(0);
		}
		//! Get end point of curve; this is the point corresponding to the domain upper bound.
		inline GPoint2 EndPoint() const {
			return Point(PointsCount() - 1);
		}
		//! Set start point of curve; this is the point corresponding to the domain lower bound.
		inline void SetStartPoint(const GPoint2& NewValue) {
			SetPoint(0, NewValue);
		}
		//! Set end point of curve; this is the point corresponding to the domain upper bound.
		inline void SetEndPoint(const GPoint2& NewValue) {
			SetPoint(PointsCount() - 1, NewValue);
		}
		/*!
			Translates all curve points by the specified vector offset.

			\param Translation the offset vector, added to each curve point.
			\note default implementation builds a matrix from specified translation and then call XForm() function.
		*/
		virtual void Translate(const GVector2& Translation);
		/*!
			Rotate all curve points around a pivot point. The rotation will occur in counter-clockwise
			direction for positive angles.

			\param Pivot the pivot point (the center of rotation)
			\param RadAmount the rotation (in radians) amount.
			\note default implementation builds a matrix from specified pivot and angle, and then call XForm() function.
		*/
		virtual void Rotate(const GPoint2& Pivot, const GReal RadAmount);
		/*!
			Scale all curve points around a pivot point.

			\param Pivot the pivot point (the center of scaling)
			\param XScaleAmount the scale factor used for X axis
			\param YScaleAmount the scale factor used for Y axis
			\note default implementation builds a matrix from specified pivot and factors, and then
			call XForm() function.
		*/
		virtual void Scale(const GPoint2& Pivot, const GReal XScaleAmount, const GReal YScaleAmount);
		/*!
			Apply an affine transformation to all curve points.

			\param Matrix a 2x3 matrix, specifying the affine transformation.
			\note the leftmost 2x2 matrix contains the rotation/scale portion, the last column vector contains the
			translation.
		*/
		virtual void XForm(const GMatrix23& Matrix);
		/*!
			Apply full transformation to all curve points.

			\param Matrix a 3x3 matrix, specifying the transformation.
			\param DoProjection if G_TRUE the projective transformation (described by the last row vector of matrix) will
			be done. In this case all transformed vertexes will be divided by the last W component. If G_FALSE only the
			affine portion will be used for transformation, and no projective division will be executed.
		*/
		virtual void XForm(const GMatrix33& Matrix, const GBool DoProjection = G_TRUE);
		//! Get class descriptor
		inline const GClassID& ClassID() const {
			return G_CURVE2D_CLASSID;
		}
		//! Get base class (father class) descriptor
		inline const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
	};


	// *********************************************************************
	//                             GCurve2DProxy
	// *********************************************************************
	/*!
		\class GCurve2DProxy
		\brief This class implements a GCurve2D proxy (provider).

		This proxy does not override CreateNew() method because we don't wanna make a creation of a GCurve2D
		class possible (because of pure virtual  methods).
	*/
	class G_EXPORT GCurve2DProxy : public GElementProxy {
	public:
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_CURVE2D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
	};

	//! Static proxy for GCurve2D class.
	static const GCurve2DProxy G_CURVE2D_PROXY;

};	// end namespace Amanith

#endif
