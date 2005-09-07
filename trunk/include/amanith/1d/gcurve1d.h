/****************************************************************************
** $file: amanith/1d/gcurve1d.h   0.1.0.0   edited Jun 30 08:00
**
** 1D Base curve segment definition.
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

#ifndef GCURVE1D_H
#define GCURVE1D_H

#include "amanith/gelement.h"

/*!
	\file gcurve1d.h
	\brief Header file for 1D curve class.
*/
namespace Amanith {


	// *********************************************************************
	//                             GCurve1D
	// *********************************************************************

	//! GCurve2D static class descriptor.
	static const GClassID G_CURVE1D_CLASSID = GClassID("GCurve1D", 0x84644355, 0xF1244B8E, 0xBFC3D59C, 0x842F089F);

	//! Order of derivative
	enum GDerivativeOrder {
		//! First order derivative
		G_FIRST_ORDER_DERIVATIVE = 1,
		//! Second order derivative
		G_SECOND_ORDER_DERIVATIVE = 2
	};

	/*!
		\class GCurve1D
		\brief This class represents a generic parametric continuous 1D curve.

		Formally a 1D parametric curve is a function with a real domain and real codomain.

		\f[
			C: [a; b] \rightarrow R
		\f]
		
		where [a; b] is a sub-set of R.\n
		So given a number belonging to domain, C is a function that give you a scalar value.
		This class implements some useful differential calculus like Length(), Tangent(), Speed() and so on.
		Some geometric tools are implemented too, like Cut(), Translate(), Scale(), and	so on.
	*/
	class G_EXPORT GCurve1D : public GElement {

	private:
		//! Curve's domain.
		GInterval<GReal> gDomain;

	protected:
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
		virtual GError DoCut(const GReal u, GCurve1D *RightCurve, GCurve1D *LeftCurve) const = 0;
		//! Cloning function; this implementation copies the domain.
		GError BaseClone(const GElement& Source);
		//! Static speed evaluation callback (for Length() evaluation).
		static GReal SpeedEvaluationCallBack(const GReal u, void *Data);

	public:
		//! Default constructor, constructs and empty curve.
		GCurve1D();
		//! Constructor with kernel specification, constructs and empty curve.
		GCurve1D(const GElement* Owner);
		//! Destructor
		virtual ~GCurve1D();
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
			Clear the curve (remove control points and internal structures)
			\note <b>this method must be implemented by all derived classes</b>.
		*/
		virtual void Clear() = 0;
		//! Get Index-th point. <b>This method must be implemented by all derived classes</b>.
		virtual GReal Point(const GUInt32 Index) const = 0;
		//! Set Index-th point. <b>This method must be implemented by all derived classes</b>.
		virtual GError SetPoint(const GUInt32 Index, const GReal NewValue) = 0;
		/*! 
			Return the curve value calculated at specified domain parameter.
			
			\param u the domain parameter at witch we wanna evaluate curve value.
			\note <b>this method must be implemented by all derived classes</b>.
		*/
		virtual GReal Evaluate(const GReal u) const = 0;
		/*! 
			Return the curve derivative calculated at specified domain parameter.

			\param Order the order of derivative
			\param u the domain parameter at witch we wanna evaluate curve derivative.
			\note <b>this method must be implemented by all derived classes</b>.
		*/
		virtual GReal Derivative(const GDerivativeOrder Order, const GReal u) const = 0;
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
		GError Cut(const GReal u, GCurve1D *RightCurve, GCurve1D *LeftCurve) const;
		/*!
			Cuts a curve slice corresponding to specified domain interval, and return the curve arc.

			\param u0 lower domain parameter specifying where to begin to cut the curve.
			\param u1 upper domain parameter specifying where to end to cut the curve.
			\param OutCurve the curve arc generated by cutting operation.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note specified domain interval is clamped by the valid interval for this curve. OutCurve parameter must
			be of the same type of this curve.
		*/
		GError Cut(const GReal u0, const GReal u1, GCurve1D *OutCurve) const;
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
		GError CutByLength(const GReal CurvePos, GCurve1D *RightCurve, GCurve1D *LeftCurve,
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
		GError CutByLength(const GReal CurvePos0, const GReal CurvePos1, GCurve1D *Curve,
						   const GReal MaxError = G_EPSILON) const;
		/*!
			Get tangent value, specifying domain parameter.

			\note for 1D curves, Tangent() and Speed() always return the same value.
		*/
		GReal Tangent(const GReal u) const;
		/*!
			Get curve speed, specifying domain parameter.

			\note for 1D curves, Tangent() and Speed() always return the same value.
		*/
		GReal Speed(const GReal u) const;
		//! Get start point of curve; this is the point corresponding to the domain lower bound.
		inline GReal StartPoint() const {
			return Point(0);
		}
		//! Get end point of curve; this is the point corresponding to the domain upper bound.
		inline GReal EndPoint() const {
			return Point(PointsCount() - 1);
		}
		//! Set start point of curve; this is the point corresponding to the domain lower bound.
		inline void SetStartPoint(const GReal NewValue) {
			SetPoint(0, NewValue);
		}
		//! Set end point of curve; this is the point corresponding to the domain upper bound.
		inline void SetEndPoint(const GReal NewValue) {
			SetPoint(PointsCount() - 1, NewValue);
		}
		/*!
			Translates all curve points by the specified scalar offset.

			\param Translation the offset value, added to each curve point.
		*/
		void Translate(const GReal Translation);
		/*!
			Scale all curve points around a pivot point.

			\param Pivot the pivot point (the center of scaling).
			\param ScaleAmount the scale factor.
			\note default behavior is to scale all point using Point() and SetPoint() methods.
		*/
		virtual void Scale(const GReal Pivot, const GReal ScaleAmount);
		//! Get class descriptor
		inline const GClassID& ClassID() const {
			return G_CURVE1D_CLASSID;
		}
		//! Get base class (father class) descriptor
		inline const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
	};


	// *********************************************************************
	//                             GCurve1DProxy
	// *********************************************************************
	/*!
		\class GCurve1DProxy
		\brief This class implements a GCurve1D proxy (provider).

		This proxy does not override CreateNew() method because we don't wanna make a creation of a GCurve1D
		class possible (because of pure virtual  methods).
	*/
	class G_EXPORT GCurve1DProxy : public GElementProxy {
	public:
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_CURVE1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
	};

	//! Static proxy for GCurve1D class.
	static const GCurve1DProxy G_CURVE1D_PROXY;

};	// end namespace Amanith

#endif
