/****************************************************************************
** $file: amanith/2d/gbeziercurve2d.h   0.3.0.0   edited Jan, 30 2006
**
** 2D Bezier curve segment definition.
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

#ifndef GBEZIERCURVE2D_H
#define GBEZIERCURVE2D_H

/*!
	\file gbeziercurve2d.h
	\brief Header file for 2D Bezier curve class.
*/

#include "amanith/2d/gcurve2d.h"

namespace Amanith {


	class GHermiteCurve2D;

	// *********************************************************************
	//                             GBezierCurve2D
	// *********************************************************************

	//! GBezierCurve2D static class descriptor.
	static const GClassID G_BEZIERCURVE2D_CLASSID = GClassID("GBezierCurve2D", 0x8278FDC6, 0x0B354A02, 0x87B0F066, 0x9A9FBF5E);

	/*!
		\class GBezierCurve2D
		\brief A generic degree 2D Bezier curve.

		Originally developed by Pierre Bezier in the 1970's for CAD/CAM operations, this kind of parametric curves is
		very famous soon (for example, they are the underpinnings of the entire Adobe PostScript drawing model).
		The Bezier curve is a parametric function of N points; two endpoints and N-2 'internal' control points. The curve
		connects the endpoints, but doesn't necessarily touch other control points. The general form Bezier equation, which
		describes each point on the curve as a function of domain parameter t, is:

		\f[
			C(t) = \sum_{i=0}^{N} B_{i, N}(t)P_{i}
		\f]

		with the Bernstein basis polynomials defined as:

		\f[
		B_{i, N}(t) = \left( \begin {array}{c}
		N \\
		i \end{array} \right) t^{i}(1-t)^{N-i}
		\f]

		Bezier curves are widely used in computer graphics to model smooth curves. As the curve is completely contained
		in the convex hull of its control points, the points can be graphically displayed and used to manipulate the
		curve intuitively. Affine transformations such as translation, scaling and rotation can be applied on the curve
		by applying the respective transform on the control points of the curve.\n
		The most important Bezier curves are quadratic and cubic curves. Higher degree curves are more expensive to
		evaluate and there is no analytic formula to calculate the roots of polynomials of degree 5 and higher.\n
		Like other Amanith curves, Bezier curves can be flattened using squared chordal distance threshold, and two
		optimized non-recursive flattening schemes were implemented for quadratics and cubics Bezier curves. These optimized
		versions are terrible fast and produce about 34% less segments than classic recursive schemes.
		
		For more details about Bezier curve, please check this
		site: http://astronomy.swin.edu.au/~pbourke/curves/bezier/
	*/
	class G_EXPORT GBezierCurve2D : public GCurve2D {

	private:
		//! Control points
		GDynArray<GPoint2> gPoints;
		//! Dynamic array, containing first order forward differences.
		mutable GDynArray<GPoint2> gForwDiff1;
		//! Dynamic array, containing second order forward differences.
		mutable GDynArray<GPoint2> gForwDiff2;
		//! Modify flag: it signs if a control point (or domain) has been modified (so forward differences must be
		// recalculated when calling derivatives estimation)
		mutable GBool gModified;

		//! Calculate first and second order forward differences
		void BuildForwDiff() const;
		//! Finds inflection points of a cubic Bezier curve; used by Flatten3() method.
		GBool FindInflectionPoints(const GReal A, const GReal B, const GReal C,
								   GReal& Flex1, GReal& Flex2, GReal& Cuspid) const;
		//! Excise inflection points of a cubic Bezier curve; used by Flatten3() method.
		void ExciseInflectionPoint(const GReal Flex, const GReal Flatness, GReal& ParamMinus, GReal& ParamPlus) const;
		//! Flatten a cubic Bezier curve, using parabolic approximation (it presupposes no inflection points)
		void ParabolicApproxBezierPointsNoInflPts(const GReal Flatness, GDynArray<GPoint2>& Contour) const;

	protected:
		/*!
			Get max variation (squared chordal distance) in the range [u0; u1]; here are necessary also
			curve evaluations at the interval ends.

			\param u0 lower bound of interested interval
			\param u1 upper bound of interested interval
			\param p0 the point corresponding to the curve evaluation at u0
			\param p1 the point corresponding to the curve evaluation at u1
			\note The interval is ensured to be completely inside the curve domain.
		*/
		GReal Variation(const GReal u0, const GReal u1,	const GPoint2& p0, const GPoint2& p1) const;

		//! Returns the number of intersection between control polygon and X axis.
		GInt32 CrossingCountX() const;
		/*!
			Compute intersections between this curve and the X axis (1, 0).

			\param Intersections every found intersection will be appended to this array.
			\param Precision the precision used to find every solution.
			\param MaxIterations number of max iterations this method can loop for each found solution.
			If the current solution calculus does not reach the specified Precision within MaxIterations iterations, the
			current solution calculus is stopped, the solution is appended into output array, and calculus go for the
			next found solution.
			The more this value is, the more is the accuracy of each solution (and the more is the time taken from this
			method).
			\note this function uses the Brent method to refine each found intersection. For more information about
			Brent's method, pleas check this site http://mathworld.wolfram.com/BrentsMethod.html.
		*/
		GBool IntersectXRay(GDynArray<GVector2>& Intersections, const GReal Precision, const GUInt32 MaxIterations) const;
		//! Cloning function, copies (physically) a Source Bezier curve into this curve.
		GError BaseClone(const GElement& Source);
		/*!
			Curve subdivision.

			Cuts the curve at specified parameter, and return left and right Bezier arcs.

			\param u domain parameter specifying where to cut the curve.
			\param RightCurve if non-NULL, the function must return the right arc generated by cutting operation.
			\param LeftCurve if non-NULL, the function must return the left arc generated by cutting operation.
			\note The domain parameter is ensured to be completely inside the curve domain. Furthermore RightCurve and
			LeftCurve parameters, if specified, are ensured to be GBezierCurve2D classes (so cast is type-safe).
		*/
		GError DoCut(const GReal u, GCurve2D *RightCurve, GCurve2D *LeftCurve) const;
		/*!
			Optimized flattening for quadratic Bezier curves.

			Theory: find the distance between points on the curve and 'corresponding' points on the line.
			One way to do this, is to notice that a Bezier curve swept by a 't' parameter (0 < t  < 1)\n\n

			\f[
				B(t) = (1 - t)^2 P_0 + 2t(1 - t)P_1 + t^2 P_2
			\f]

			is approximated by a straight line (swept at a constant velocity by 't')\n\n

			\f[
				L(t) = (1 - t)P_0 + t P_2
			\f]

			It can be shown that Max |B(t) - L(t)|^2 is at t = 1/2. This kind of distance can be assumed to be the
			squared chordal distance.\n
			Now, we wanna find a 't' value where to cut the Bezier curve, so that left arc will have a maximum squared
			chordal distance less then a specified epsilon (so a straight line between its start and end points is a
			good flat segment). This leads to:\n\n

			\f[
				\left| t^2 (-\frac{1}{4} P_0 + \frac{1}{2} P_1 - \frac{1}{4} P_2) \right|^2 <= \epsilon
			\f]
			
			let

			\f[
				k = -\frac{1}{4} P_0 + \frac{1}{2} P_1 - \frac{1}{4} P_2
			\f]

			the disequation becomes:\n

			\f[
				t^4 <= \frac{\epsilon}{|k|^2}
			\f]

			and the solution is:
			\f[
				t = \frac{\epsilon}{\sqrt[4]{|k|^2}}
			\f]

			If t is > 1, then the entire curve can be approximated by a straight line.

			\param Contour a dynamic array where this function has to append generated points.
			\param MaxDeviation maximum squared chordal distance we wanna reach (maximum permitted deviation).
			\param IncludeLastPoint if G_TRUE the function must append last curve point (the point corresponding to
			domain upper bound parameter). If G_FALSE last point must not be included.
		*/
		GError Flatten2(GDynArray<GPoint2>& Contour, const GReal MaxDeviation, const GBool IncludeLastPoint) const;
		/*!
			Optimized flattening for cubic Bezier curves.

			The implementation is based on "Precise Flattening of Cubic Bezier Segments", Thomas Hain, Ahmad, Athar L., and
			Langan David, Canadian Conference on Computational Geometry, Montreal, Canada, Aug 9–11, 2004.\n
			Paper can be found on prof. Hain homepage: http://www.cis.usouthal.edu/~hain/Publications.htm \n
			The trick here is to do a parabolic approximation, taking care that a cubic polynomial can have flex and
			cuspid point. Finding these points, and cutting the curve, every single slice can be approximated by a
			parabolic trait.

			\param Contour a dynamic array where this function has to append generated points.
			\param MaxDeviation maximum squared chordal distance we wanna reach (maximum permitted deviation).
			\param IncludeLastPoint if G_TRUE the function must append last curve point (the point corresponding to
			domain upper bound parameter). If G_FALSE last point must not be included.
		*/
		GError Flatten3(GDynArray<GPoint2>& Contour, const GReal MaxDeviation, const GBool IncludeLastPoint) const;
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
		*/
		GError Flatten(const GReal u0, const GReal u1, const GPoint2& p0, const GPoint2& p1,
					   GDynArray<GPoint2>& Contour, const GReal MaxDeviation) const;

	public:
		//! Default constructor, creates an empty Bezier.
		GBezierCurve2D();
		//! Constructor with owner (kernel) parameter, creates an empty Bezier.
		GBezierCurve2D(const GElement* Owner);
		//! Destructor
		~GBezierCurve2D();
		//! Clear the curve (remove control points, free internal structures and set an empty domain).
		void Clear();
		//! Returns number of control points.
		GUInt32 PointsCount() const;
		//! Get curve degree; this value is equal to PointsCount() - 1.
		GInt32 Degree() const;
		//! Get control points array.
		const GDynArray<GPoint2>& Points() const {
			return gPoints;
		}
		//! Get Index-th control point; Index must be valid, else a point with infinitive components is returned.
		GPoint2 Point(const GUInt32 Index) const;
		//! Set Index-th control point; Index must be valid.
		GError SetPoint(const GUInt32 Index, const GPoint2& NewPoint);
		/*!
			Build the Bezier curve, specifying control points. This is the only method to build a Bezier curve.

			The curve will have a (|NewPoints| - 1) degree.
			\param NewPoints control points that will build this Bezier curve. At least two points must be specified, else
			curve can't be built.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note Domain won't be changed.
		*/
		GError SetPoints(const GDynArray<GPoint2>& NewPoints);
		/*!
			Build a quadratic Bezier curve, specifying its three control points.
			This method is provided for convenience.

			\note Domain won't be changed.
		*/
		GError SetPoints(const GPoint2& P0, const GPoint2& P1, const GPoint2& P2);
		/*!
			Build a cubic Bezier curve, specifying its four control points.
			This method is provided for convenience.

			\note Domain won't be changed.
		*/
		GError SetPoints(const GPoint2& P0, const GPoint2& P1, const GPoint2& P2, const GPoint2& P3);
		/*!
			Set curve domain, and sign this Bezier curve as modified.

			\param NewMinValue the lower bound of the new domain.
			\param NewMaxValue the upper bound of the new domain.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note if NewMinValue is greater than NewMaxValue they will be swapped, so any combination of values is valid.
		*/
		GError SetDomain(const GReal NewMinValue, const GReal NewMaxValue);
		/*!
			Increases by one the degree of the curve (without changing its shape).

			More details about Bezier degree elevation can be found
			here: http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/Bezier/bezier-elev.html

			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError HigherDegree();
		/*!
			Increases by HowManyTimes the degree of the curve (without changing its shape).

			More details about Bezier degree elevation can be found
			here: http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/Bezier/bezier-elev.html

			\param HowManyTimes the degree elevation amount.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note this an "ad hoc" implementation, it does not call HowManyTimes times the HigherDegree() function.
		*/
		GError HigherDegree(const GInt32 HowManyTimes);
		/*!
			Increases by one the degree of the curve (without changing its shape), and put the result into the
			specified output Bezier curve.

			More details about Bezier degree elevation can be found
			here: http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/Bezier/bezier-elev.html

			\param OutputCurve the curve used as destination.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError HigherDegree(GBezierCurve2D& OutputCurve) const;

		/*!
			Increases by HowManyTimes the degree of the curve (without changing its shape), and put the result into the
			specified output Bezier curve.

			More details about Bezier degree elevation can be found
			here: http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/Bezier/bezier-elev.html

			\param HowManyTimes the degree elevation amount.
			\param OutputCurve the curve used as destination.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note this an "ad hoc" implementation, it does not call HowManyTimes times the HigherDegree() function.
		*/
		GError HigherDegree(const GInt32 HowManyTimes, GBezierCurve2D& OutputCurve) const;

		/*!
			Decreases by one the degree of the curve.

			The algorithm implemented is an interesting variation of the classical odd/even points extrapolation.
			Our idea is completely original, and shows better results.
			For more details about Bezier degree reduction can be found
			here: http://anziamj.austms.org.au/V36/part4/park/p399.html

			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError LowerDegree();
		/*!
			Decreases by one the degree of the curve, and put the result into the specified output Bezier curve.

			The algorithm implemented is an interesting variation of the classical odd/even points extrapolation.
			Our idea is completely original, and shows better results.
			For more details about Bezier degree reduction can be found
			here: http://anziamj.austms.org.au/V36/part4/park/p399.html

			\param OutputCurve the curve used as destination.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError LowerDegree(GBezierCurve2D& OutputCurve) const;
		/*!
			Intersect the Bezier curve with a normalized ray, and returns a list of intersections.

			Here's the implemented algorithm:\n

			-# First transform all control points into "ray space", so that	line is starting at (0, 0) and have
			a direction of (1, 0) (X axis).
			-# At this point Variation diminishing can be exploited. After point (1) number of intersection
			ray-controlpolygon is a simple y-change sign test.
			-# A recursive loop splits the curve until every possible solution are isolated.
				- if the curve piece (its control polygon) does not intersect x-axis just discard it and exit.
				- if the curve piece (its control polygon) intersects more than once x-axis split this
				piece (say at midpoint parameter) into right and left arcs. Call the function for left arc
				and then for right arc (this order is the most clever, so solutions are automatically ordered in
				ascending order).
				- if the curve piece intersects (its control polygon intersects) just once x-axis, the Brents's method
				is used to catch the solution. This method  requires curve evaluation only, its "root bracketing" and
				overall it can't diverge, so the solution is always find with good convergence. 
			
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
		*/
		GBool IntersectRay(const GRay2& NormalizedRay, GDynArray<GVector2>& Intersections,
						   const GReal Precision = G_EPSILON, const GUInt32 MaxIterations = 100) const;
		//!	Returns number of intersection between control polygon and a ray.
		GInt32 CrossingCount(const GRay2& Ray) const;
		/*!
			Returns control polygon length.

			\param FromIndex lower index of considered control points.
			\param ToIndex upper index of considered control points.
			\return control polygon length, for specified control points range.
		*/
		GReal ControlPolygonLength(const GUInt32 FromIndex, const GUInt32 ToIndex) const;
		/*!
			Flats the curve specifying a max error/variation (squared chordal distance).

			\param Contour a dynamic array where this function has to append generated points.
			\param MaxDeviation maximum squared chordal distance we wanna reach (maximum permitted deviation).
			\param IncludeLastPoint if G_TRUE the function must append last curve point (the point corresponding to
			domain upper bound parameter). If G_FALSE last point must not be included.
			\note if this Bezier curve is quadratic or cubic, then respective optimized methods Flatten2() / Flatten3()
			are	called, for best performance assurance.
		*/
		GError Flatten(GDynArray<GPoint2>& Contour, const GReal MaxDeviation, const GBool IncludeLastPoint = G_TRUE) const;
		/*!
			Get variation (squared chordal distance) in the current domain range.
		*/
		GReal Variation() const;
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
			Cubic Bezier to Hermite conversion.

			This method converts a cubic Bezier trait into an Hermite representation. The conversion can be
			easily derived equaling their power series representations.

			\return G_NO_ERROR in operation succeeds, an error code otherwise.
			\note if this Bezier curve is not cubic an G_INVALID_OPERATION error code is returned.
		*/
		GError ConvertToHermite(GHermiteCurve2D& Curve) const;
		//! Get class descriptor.
		inline const GClassID& ClassID() const {
			return G_BEZIERCURVE2D_CLASSID;
		}
		//! Get base class (father class) descriptor.
		inline const GClassID& DerivedClassID() const {
			return G_CURVE2D_CLASSID;
		}
	};


	// *********************************************************************
	//                           GBezierCurve2DProxy
	// *********************************************************************

	/*!
		\class GBezierCurve2DProxy
		\brief This class implements a GBezierCurve2D proxy (provider).

		This proxy provides the creation of GBezierCurve2D class instances.
	*/
	class G_EXPORT GBezierCurve2DProxy : public GElementProxy {
	public:
		//! Creates a new GBezierCurve2D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GBezierCurve2D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_BEZIERCURVE2D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_CURVE2D_CLASSID;
		}
	};
	//! Static proxy for GBezierCurve2D class.
	static const GBezierCurve2DProxy G_BEZIERCURVE2D_PROXY;

};	// end namespace Amanith

#endif
