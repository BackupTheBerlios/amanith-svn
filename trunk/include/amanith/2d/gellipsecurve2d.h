/****************************************************************************
** $file: amanith/2d/gellipsecurve2d.h   0.1.1.0   edited Sep 24 08:00
**
** 2D ellipse curve segment (arc) definition.
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

#ifndef GELLIPSECURVE2D_H
#define GELLIPSECURVE2D_H

/*!
	\file gellipsecurve2d.h
	\brief Header file for 2D ellipse curve/arc class.
*/

#include "amanith/2d/gcurve2d.h"

namespace Amanith {

	// *********************************************************************
	//                             GEllipseCurve2D
	// *********************************************************************

	//! GEllipseCurve2D static class descriptor.
	static const GClassID G_ELLIPSECURVE2D_CLASSID = GClassID("GEllipseCurve2D", 0x8BC11EA8, 0x6C274B35, 0x94AD500B, 0xDBEDA5C5);

	/*!
		\class GEllipseCurve2D
		\brief 2D ellipse arc/curve.

		An ellipse is the curve described implicitly by an equation of the second degree
		Ax^2 + Bxy + Cy^2 + Dx + Ey + F = 0 when the discriminant B^2 - 4AC is less than zero.
		The standard form of the equation of an ellipse is (x/a)^2 + (y/b)^2 = 1, where a and b are the lengths of
		the axes.
		As can be seen from the Cartesian equation for the ellipse, the curve can also be given by a simple
		parametric form analogous to that of a circle, but with the x and y coordinates having different scalings\n\n

		x(t) = a * cos(t)\n
		y(t) = b * sin(t)\n\n

		where t can lie into a compact interval, lets say [t1; t2].
		This is the classic axes-aligned ellipse/arc centered at (0, 0), to express an offset rotation
		(relative to x-axis) and a different center, we must rotate points and add center (cx, cy), resulting in:\n\n

		x(t) = a * cos(ofs) * cos(t) - b * sin(ofs) * sin(t) + cx\n
		y(t) = a * sin(ofs) * cos(t) + b * cos(ofs) * sin(t) + cy\n\n

		with t inside [t1; t2].\n
		We can go from t1 to t2 in 2 directions: counter-clockwise or clockwise, spanning (depending on t1 and t2)
		the smallest or the largest arc.
		
	*/
	class G_EXPORT GEllipseCurve2D : public GCurve2D {

	private:
		//! Center of the ellipse.
		GPoint2 gCenter;
		//! Semi axis X (length).
		GReal gXSemiAxisLength;
		//! Semi axis Y (length).
		GReal gYSemiAxisLength;
		//! Rotation offset angle, it permits non axes-aligned ellipses.
		GReal gOffsetRotation;
		//! Pre-calculated cos/sin of gOffsetRotation angle
		GReal gCosOfsRot, gSinOfsRot;
		//! Start angle corresponding to the domain lower bound.
		GReal gStartAngle;
		//! End angle corresponding to the domain upper bound.
		GReal gEndAngle;
		//! Direction ccw/cw
		GBool gCCW;


	protected:
		//! Cloning function, copies (physically) a Source GEllipseCurve2D curve into this curve.
		GError BaseClone(const GElement& Source);
		/*!
			Curve subdivision.

			Cuts the curve at specified parameter, and return left and right ellipse arcs.

			\param u domain parameter specifying where to cut the curve.
			\param RightCurve if non-NULL, the function must return the right arc generated by cutting operation.
			\param LeftCurve if non-NULL, the function must return the left arc generated by cutting operation.
			\note The domain parameter is ensured to be completely inside the curve domain. Furthermore RightCurve and
			LeftCurve parameters, if specified, are ensured to be GEllipseCurve2D classes (so cast is type-safe).
		*/
		GError DoCut(const GReal u, GCurve2D *RightCurve, GCurve2D *LeftCurve) const;
		/*!
			Given two points P0 and P1, return G_TRUE if two unit circles can be found so that they interpolate
			P0 and P1. In this case centers of these circles are returned in C0 and C1, else a G_FALSE value
			is returned and GrowingFactor will contain the smallest factor to scale P0 and P1 (along P0-P1 direction)
			so that unit circles can be found.
		*/
		static GBool FindUnitCircles(const GPoint2& P0, const GPoint2& P1, GPoint2& C0, GPoint2& C1,
									GReal& GrowingFactor);
		/*!
			Return G_TRUE if specified Angle lies inside angle domain, taking care of cw/ccw direction.
			If so, a ratio value is returned, that corresponds to the percentage of angle spanned starting from
			StartAngle and going to Angle.
		*/
		GBool InsideAngleDomain(const GReal Angle, GReal& Ratio) const;
		//! Map a curve parameter into angle domain.
		GReal MapAngle(const GReal u) const;
		//! Evaluate the ellipse, specifying an angle.
		GPoint2 EvaluateByAngle(const GReal Angle) const;
		/*
			Return G_TRUE if an ellipse arc from StartAngle to EndAngle in CCW direction spans the largest
			angle (greater than PI).
		*/
		static GBool IsLargeArc(const GReal StartAngle, const GReal EndAngle, const GBool CCW);
		//! Given an Angle, it returns the same angle into [0; 2PI] range.
		static GReal FixAngle(const GReal Angle);

	public:
		//! Default constructor, creates an empty ellipse arc/curve.
		GEllipseCurve2D();
		//! Constructor with owner (kernel) parameter, creates an empty ellipse.
		GEllipseCurve2D(const GElement* Owner);
		//! Destructor
		~GEllipseCurve2D();
		//! Clear the arc/curve (free internal structures and set an empty domain).
		void Clear();
		//! Returns number of control points.
		GUInt32 PointsCount() const;
		//! Get Index-th control point; Index must be valid, else a point with infinitive components is returned.
		GPoint2 Point(const GUInt32 Index) const;
		//! Set Index-th control point; Index must be valid.
		GError SetPoint(const GUInt32 Index, const GPoint2& NewPoint);
		/*!
			Build a new ellipse, specifying center, semi axes lengths, an offset rotation angle and angle domain.

			\param Center center of the ellipse.
			\param XSemiAxisLength length of x-axis.
			\param YSemiAxisLength length of y-axis.
			\param OffsetRotation offset rotation relative to x-axis.
			\param StartAngle angle where 'to start' ellipse arc..
			\param EndAngle angle where 'to end' ellipse arc.
			\param CCW if G_TRUE a counter-clockwise direction will be chosen, else clockwise.
		*/
		void SetEllipse(const GPoint2& Center, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
						const GReal OffsetRotation = 0, const GReal StartAngle = 0, const GReal EndAngle = G_2PI,
						const GBool CCW = G_TRUE);
		/*!
			Build a new ellipse, specifying 2 interpolation points, semi axes lengths, an offset rotation angle
			and two flags that specify desired arc (of the 4 possible ones).

			\param P0 first interpolated point.
			\param P1 last interpolated point.
			\param XSemiAxisLength length of x-axis.
			\param YSemiAxisLength length of y-axis.
			\param OffsetRotation offset rotation relative to x-axis.
			\param LargeArc if G_TRUE the largest arc will be chosen to go from P0 to P1, else the smallest one.
			\param CCW if G_TRUE a counter-clockwise direction will be chosen to go from P0 to P1, else clockwise.
		*/
		GError SetEllipse(const GPoint2& P0, const GPoint2& P1, const GReal XSemiAxisLength, const GReal YSemiAxisLength,
						  const GReal OffsetRotation = 0, const GBool LargeArc = G_TRUE, const GBool CCW = G_FALSE);
		//! Get ellipse center.
		inline const GPoint2& Center() const {
			return gCenter;
		}
		//! Set a new ellipse center.
		inline void SetCenter(const GPoint2& NewCenter) {
			gCenter = NewCenter;
		}
		//! Get X semi-axis length.
		inline GReal XSemiAxisLength() const {
			return gXSemiAxisLength;
		}
		//! Set X semi-axis length.
		inline void SetXSemiAxisLength(const GReal NewLength) {
			gXSemiAxisLength = NewLength;
		}
		//! Get Y semi-axis length.
		inline GReal YSemiAxisLength() const {
			return gYSemiAxisLength;
		}
		//! Set Y semi-axis length.
		inline void SetYSemiAxisLength(const GReal NewLength) {
			gYSemiAxisLength = NewLength;
		}
		/*!
			Get X axis, intended as a full axis (it includes offset rotation and semi-length if G_FALSE is specified
			as parameter).
		*/
		GVector2 XAxis(const GBool Normalized = G_TRUE) const;
		/*!
			Get Y axis, intended as a full axis (it includes offset rotation and semi-length if G_FALSE is specified
			as parameter).
		*/
		GVector2 YAxis(const GBool Normalized = G_TRUE) const;
		//! Get offset rotation (in radiants).
		inline GReal OffsetRotation() const {
			return gOffsetRotation;
		}
		//! Set offset rotation (in radiants).
		inline void SetOffsetRotation(const GReal NewOffsetRotation) const {
			gOffsetRotation = NewOffsetRotation;
			gCosOfsRot = GMath::Cos(gOffsetRotation);
			gSinOfsRot = GMath::Sin(gOffsetRotation);
		}
		//! Get angle corresponding to domain lower bound.
		inline GReal StartAngle() const {
			return gStartAngle;
		}
		//! Get angle corresponding to domain upper bound.
		inline GReal EndAngle() const {
			return gEndAngle;
		}
		//! Set a new angle domain.
		void SetAngleDomain(const GReal NewStartAngle, const GReal NewEndAngle);
		/*!
			Intersect the ellipse arc/curve with a normalized ray, and returns a list of intersections.

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
			\note in this implementation Precision and MaxIterations are not used, due to analytical solutions.
		*/
		GBool IntersectRay(const GRay2& NormalizedRay, GDynArray<GVector2>& Intersections,
						   const GReal Precision = G_EPSILON, const GUInt32 MaxIterations = 100) const;
		/*!
			Flats the curve specifying a max error/variation (squared chordal distance).

			\param Contour a dynamic array where this function has to append generated points.
			\param MaxDeviation maximum squared chordal distance we wanna reach (maximum permitted deviation).
			\param IncludeLastPoint if G_TRUE the function must append last curve point (the point corresponding to
			domain upper bound parameter). If G_FALSE last point must not be included.
		*/
		GError Flatten(GDynArray<GPoint2>& Contour, const GReal MaxDeviation, const GBool IncludeLastPoint = G_TRUE) const;
		/*!
			Return G_TRUE if the ellipse arc spans the largest angle, else G_FALSE.
		*/
		inline GBool LargeArc() const {
			return GEllipseCurve2D::IsLargeArc(gStartAngle, gEndAngle, gCCW);
		}
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
			Translates ellipse, specifying a vector offset.

			\param Translation the offset vector.
			\note implementation 'moves' the center.
		*/
		void Translate(const GVector2& Translation);
		/*!
			Rotate ellipse around a pivot point.  The rotation will occur in counter-clockwise
			direction for positive angles.

			\param Pivot the pivot point (the center of rotation).
			\param RadAmount the rotation (in radians) amount.
		*/
		void Rotate(const GPoint2& Pivot, const GReal RadAmount);
		/*!
			Scale ellipse, using a pivot point (that will affect the center) and scaling factors.

			\param Pivot the pivot point (the center of scaling).
			\param XScaleAmount the scale factor used for X axis (intended as (1, 0) Cartesian axis).
			\param YScaleAmount the scale factor used for Y axis (intended as (0, 1) Cartesian axis).
		*/
		void Scale(const GPoint2& Pivot, const GReal XScaleAmount, const GReal YScaleAmount);
		/*!
			Apply an affine transformation to the ellipse.

			\param Matrix a 2x3 matrix, specifying the affine transformation.
			\note the leftmost 2x2 matrix contains the rotation/scale portion, the last column vector contains the
			translation.
		*/
		void XForm(const GMatrix23& Matrix);
		/*!
			Apply full transformation to the ellipse.

			\param Matrix a 3x3 matrix, specifying the transformation.
			\param DoProjection if G_TRUE the projective transformation (described by the last row vector of matrix) will
			be done. In this case all transformed vertexes will be divided by the last W component. If G_FALSE only the
			affine portion will be used for transformation, and no projective division will be executed.
		*/
		void XForm(const GMatrix33& Matrix, const GBool DoProjection = G_TRUE);
		//! Get class descriptor.
		inline const GClassID& ClassID() const {
			return G_ELLIPSECURVE2D_CLASSID;
		}
		//! Get base class (father class) descriptor.
		inline const GClassID& DerivedClassID() const {
			return G_CURVE2D_CLASSID;
		}
	};


	// *********************************************************************
	//                           GEllipseCurve2DProxy
	// *********************************************************************

	/*!
		\class GEllipseCurve2DProxy
		\brief This class implements a GEllipseCurve2D proxy (provider).

		This proxy provides the creation of GEllipseCurve2D class instances.
	*/
	class G_EXPORT GEllipseCurve2DProxy : public GElementProxy {
	public:
		//! Creates a new GEllipseCurve2D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GEllipseCurve2D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_ELLIPSECURVE2D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_CURVE2D_CLASSID;
		}
	};
	//! Static proxy for GEllipseCurve2D class.
	static const GEllipseCurve2DProxy G_ELLIPSECURVE2D_PROXY;

};	// end namespace Amanith

#endif
