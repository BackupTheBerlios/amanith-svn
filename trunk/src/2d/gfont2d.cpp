/****************************************************************************
** $file: amanith/src/2d/gfont.cpp   0.2.0.0   edited Dec, 12 2005
**
** 2D Font implementation.
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

#include "amanith/geometry/gintersect.h"
#include "amanith/2d/gfont2d.h"
#include "amanith/2d/gpolylinecurve2d.h"
#include "amanith/geometry/gxform.h"
#include "amanith/gkernel.h"

/*!
	\file gfont2d.cpp
	\brief Font engine implementation.
*/

namespace Amanith {

// *********************************************************************
//                           GFontCharContour2D
// *********************************************************************

// constructor
GFontCharContour2D::GFontCharContour2D(const GDynArray<GPoint2>& NewPoints,
								   const GDynArray<GInt32>& PointsFlags) {

	if (NewPoints.size() != PointsFlags.size())
		return;

	BuildGoodContour(G_EPSILON, NewPoints, PointsFlags);
	// build bound box (axis aligned)
	gBoundBox.SetMinMax(gPoints);

}

void GFontCharContour2D::BuildGoodContour(const GReal Precision, const GDynArray<GPoint2>& NewPoints,
										const GDynArray<GInt32>& PointsFlags) {

	GInt32 first = 0, last = (GInt32)NewPoints.size();
	GInt32 k1 = first, k2 = k1 + 1, k3, k4, k, skip_next = 0;
	GInt32 on1 = (PointsFlags[k1] & 1), on2 = (PointsFlags[k2] & 1), on3, on4, isCubic;
	GInt32 firstOn1 = on1;
	GInt32 f1 = PointsFlags[k1], f2 = PointsFlags[k2], f3, f4;
	GPoint2 p1(NewPoints[k1]), p2(NewPoints[k2]), p3, p23, p12, p4, p;
	GBezierCurve2D tmpBez;
	GReal l;

	for (k = first + 1; k <= last; ++k) {
		if (k == last) {
			if (firstOn1)
				k3 = first;
			else
				k3 = first + 1;
		}
		else {
			k3 = k + 1;
			if (k3 >= last)
				k3 = first;
		}
		on3 = (PointsFlags[k3] & 1);
		f3 = PointsFlags[k3];
		p3 = NewPoints[k3];
		if (!skip_next) {
			if (on1) {
				if (on2) {
					// this is a simple line
					if (GMath::Abs(Length(p1 - p2)) >= Precision) {
						gPoints.push_back(p1);
						gPointsFlags.push_back(f1);
					}
				}
				else {
					if (on3) {
						tmpBez.SetPoints(p1, p2, p3);
						l = tmpBez.TotalLength();
						if (l < Precision) {
							p = tmpBez.Evaluate((GReal)0.5);
							gPoints.push_back(p);
							gPointsFlags.push_back(1);
							p3 = p;
							skip_next = 3;
						}
						else {
							gPoints.push_back(p1);
							gPointsFlags.push_back(f1);
						}
					}
					else {
						isCubic = f2 & 2;
						if (!isCubic) {
							p23 = (p2 + p3) * (GReal)0.5;
							tmpBez.SetPoints(p1, p2, p23);
							l = tmpBez.TotalLength();
							if (l < Precision) {
								p = tmpBez.Evaluate((GReal)0.5);
								gPoints.push_back(p);
								gPointsFlags.push_back(1);
								p2 = p;
							}
							else {
								gPoints.push_back(p1);
								gPointsFlags.push_back(f1);
							}
						}
						// if is cubic just push p1
						else {
							k4 = k3 + 1;
							if (k4 >= last)
								k4 = first;
							on4 = (PointsFlags[k4] & 1);
							G_ASSERT(on4 != 0);
							f4 = PointsFlags[k4];
							p4 = NewPoints[k4];
							tmpBez.SetPoints(p1, p2, p3, p4);
							l = tmpBez.TotalLength();
							if (l < Precision) {
								p = tmpBez.Evaluate((GReal)0.5);
								gPoints.push_back(p);
								gPointsFlags.push_back(1);
								p3 = p;
								skip_next = 3;
								// after 2 skips, it will result in a line segment built by p and p4
								// and it will have a length less than Precision, so it will be discarded
							}
							else {
								gPoints.push_back(p1);
								gPointsFlags.push_back(f1);
							}
						}
					}
				}
			}
			else {
				if (on2) {
					gPoints.push_back(p1);
					gPointsFlags.push_back(f1);
				}
				else {
					isCubic = f2 & 2;
					if (isCubic) {
						gPoints.push_back(p1);
						gPointsFlags.push_back(f1);
					}
					else {
						if (on3) {
							gPoints.push_back(p1);
							gPointsFlags.push_back(f1);
							p12 = (p1 + p2) * (GReal)0.5;
							tmpBez.SetPoints(p12, p2, p3);
							l = tmpBez.TotalLength();
							if (l < Precision) {
								p = tmpBez.Evaluate((GReal)0.5);
								gPoints.push_back(p);
								gPointsFlags.push_back(1);
								p3 = p;
								skip_next = 3;

							}
						}
						else {
							gPoints.push_back(p1);
							gPointsFlags.push_back(f1);
							isCubic = f1 & 2;
							if (!isCubic) {
								p12 = (p1 + p2) * (GReal)0.5;
								p23 = (p2 + p3) * (GReal)0.5;
								tmpBez.SetPoints(p12, p2, p23);
								l = tmpBez.TotalLength();
								if (l < Precision) {
									p = tmpBez.Evaluate((GReal)0.5);
									gPoints.push_back(p);
									gPointsFlags.push_back(1);
									p2 = p;
								}
							}
						}
					}
				}
			}
		}
		k1 = k2;
		k2 = k3;
		p1 = p2;
		p2 = p3;
		f1 = f2;
		f2 = f3;
		on1 = on2;
		on2 = on3;
		if (skip_next > 0)
			skip_next--;
	}
}

// copy constructor
GFontCharContour2D::GFontCharContour2D(const GFontCharContour2D& Source) {

	gPoints = Source.gPoints;
	gBoundBox = Source.gBoundBox;
	gIsHole = Source.gIsHole;
	gPointsFlags = Source.gPointsFlags;
}


void GFontCharContour2D::DrawContour(const GDynArray<GPoint2>& ContourPoints, const GDynArray<GInt32>& ContourFlags,
								     GDynArray<GPoint2>& OutPoints, const GReal Variation) {

	#define FLATBEZIER3(a, b, c) \
		Drawer.SetPoints(a, b, c); \
		Drawer.Flatten(OutPoints, Variation, G_FALSE);

	#define FLATBEZIER4(a, b, c, d) \
		Drawer.SetPoints(a, b, c, d); \
		Drawer.Flatten(OutPoints, Variation, G_FALSE);

	GBezierCurve2D Drawer;
	GInt32 first = 0, last = (GInt32)ContourPoints.size();
	GInt32 k1 = first, k2 = k1 + 1, k3, k4, k, skip_next = 0;
	GInt32 on1 = (ContourFlags[k1] & 1), on2 = (ContourFlags[k2] & 1), on3, isCubic;
	GInt32 firstOn1 = on1;
	GPoint2 p1(ContourPoints[k1]), p2(ContourPoints[k2]), p3, p23, p12, p4;

	for (k = first + 1; k <= last; ++k) {
		if (k == last) {
			if (firstOn1)
				k3 = first;
			else
				k3 = first + 1;
		}
		else {
			k3 = k + 1;
			if (k3 >= last)
				k3 = first;
		}
		on3 = (ContourFlags[k3] & 1);
		p3 = ContourPoints[k3];
		if (!skip_next) {
			if (on1) {
				if (on2)
					// this is a simple line
					OutPoints.push_back(p1);
				else {
					if (on3) {
						FLATBEZIER3(p1, p2, p3);
						skip_next = 2;
					}
					else {
						isCubic = ContourFlags[k3] & 2;
						if (!isCubic) {
							p23 = (p2 + p3) * (GReal)0.5;
							FLATBEZIER3(p1, p2, p23);
						}
						else {
							k4 = k3 + 1;
							if (k4 >= last)
								k4 = first;
							p4 = ContourPoints[k4];
							FLATBEZIER4(p1, p2, p3, p4);
							skip_next = 3;
							// the third skip next will be done automatically, because
							// the routine will find a sequence of on1 = 0 and on2 = 1
						}
					}
				}
			}
			else {
				if (on2) {
				}
				else {
					isCubic = ContourFlags[k2] & 2;
					if (isCubic)
						skip_next = 2;
					else {
						if (on3) {
							p12 = (p1 + p2) * (GReal)0.5;
							FLATBEZIER3(p12, p2, p3);
							skip_next = 2;
						}
						else {
							p12 = (p1 + p2) * (GReal)0.5;
							p23 = (p2 + p3) * (GReal)0.5;
							FLATBEZIER3(p12, p2, p23);
						}
					}
				}
			}
		}
		k1 = k2;
		k2 = k3;
		p1 = p2;
		p2 = p3;
		on1 = on2;
		on2 = on3;
		if (skip_next > 0)
			skip_next--;
	}
	#undef FLATBEZIER3
	#undef FLATBEZIER4
}

void GFontCharContour2D::DrawContour(GDynArray<GPoint2>& Points, const GReal Variation) const {

	if (Variation <= 0)
		return;

	DrawContour(gPoints, gPointsFlags, Points, Variation);
}

void GFontCharContour2D::DrawContour(GDynArray<GPoint2>& Points, const GReal Variation,
								   const GMatrix33& Transformation) const {

	if (Variation <= 0)
	   return;

	GInt32 i, j = (GInt32)gPoints.size();
	GDynArray<GPoint2> tmpPoints(j);
	GPoint2 p, h;

	for (i = 0; i < j; i++) {
		p = gPoints[i];
		h = Transformation * p;
		tmpPoints[i] = h;
	}
	DrawContour(tmpPoints, gPointsFlags, Points, Variation);
}

void GFontCharContour2D::DecomposeBezier(GDynArray<GPoint2>& Points, GDynArray<GInt32>& Index) const {

	#define DECOMP2(a, b) \
		Points.push_back(a); \
		Points.push_back(b); \
		Index.push_back(2)

	#define DECOMP3(a, b, c) \
		Points.push_back(a); \
		Points.push_back(b); \
		Points.push_back(c); \
		Index.push_back(3)

	#define DECOMP4(a, b, c, d) \
		Points.push_back(a); \
		Points.push_back(b); \
		Points.push_back(c); \
		Points.push_back(d); \
		Index.push_back(4)

	GInt32 first = 0, last = (GInt32)gPoints.size();
	GInt32 k1 = first, k2 = k1 + 1, k3, k4, k, skip_next = 0;
	GInt32 on1 = (gPointsFlags[k1] & 1), on2 = (gPointsFlags[k2] & 1), on3, isCubic;
	GInt32 firstOn1 = on1;
	GPoint2 p1(gPoints[k1]), p2(gPoints[k2]), p3, p23, p12, p4;

	for (k = first + 1; k <= last; ++k) {
		if (k == last) {
			if (firstOn1)
				k3 = first;
			else
				k3 = first + 1;
		}
		else {
			k3 = k + 1;
			if (k3 >= last)
				k3 = first;
		}
		on3 = (gPointsFlags[k3] & 1);
		p3 = gPoints[k3];
		if (!skip_next) {
			if (on1) {
				if (on2) {
					// this is a simple line
					DECOMP2(p1, p2);
				}
				else {
					if (on3) {
						DECOMP3(p1, p2, p3);
						skip_next = 2;
					}
					else {
						isCubic = gPointsFlags[k3] & 2;
						if (!isCubic) {
							p23 = (p2 + p3) * (GReal)0.5;
							DECOMP3(p1, p2, p23);
						}
						else {
							k4 = k3 + 1;
							if (k4 >= last)
								k4 = first;
							p4 = gPoints[k4];
							DECOMP4(p1, p2, p3, p4);
							skip_next = 3;
							// the third skip next will be done automatically, because
							// the routine will find a sequence of on1 = 0 and on2 = 1
						}
					}
				}
			}
			else {
				if (on2) {
				}
				else {
					isCubic = gPointsFlags[k2] & 2;
					if (isCubic)
						skip_next = 2;
					else {
						if (on3) {
							p12 = (p1 + p2) * (GReal)0.5;
							DECOMP3(p12, p2, p3);
							skip_next = 2;
						}
						else {
							p12 = (p1 + p2) * (GReal)0.5;
							p23 = (p2 + p3) * (GReal)0.5;
							DECOMP3(p12, p2, p23);
						}
					}
				}
			}
		}
		k1 = k2;
		k2 = k3;
		p1 = p2;
		p2 = p3;
		on1 = on2;
		on2 = on3;
		if (skip_next > 0)
			skip_next--;
	}
	#undef DECOMP2
	#undef DECOMP3
	#undef DECOMP4
}

GPoint2 GFontCharContour2D::PickPointOnCurve() const {

	GInt32 on0, on1, on2, oni, isCubic;
	GUInt32 i, j;
	GPoint2 p0(gPoints[0]), p1(gPoints[1]), p2(gPoints[2]), p;

	on0 = gPointsFlags[0] & 1;
	if (on0)
		return p0;
	on1 = gPointsFlags[1] & 1;
	if (on1)
		return p1;
	on2 = gPointsFlags[2] & 1;
	if (on2)
		return p2;
	isCubic = gPointsFlags[0] & 2;
	if (isCubic) {
		// in this case we are sure that an on point will be found
		j = (GUInt32)gPoints.size();
		for (i = 3; i < j; i++) {
			oni = gPointsFlags[i] & 1;
			if (oni)
				return gPoints[i];
		}
		return G_NULL_POINT2;
	}
	else {
		p = (p0 + p1) * (GReal)0.5;
		return p;
	}
}

// given a Bezier decomposition, find the Bezier arc (index) that has a control point at has
// got a x component less than specified XValue
GInt32 GFontCharContour2D::FindLeftArc(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& Index,
									 const GReal XValue, GUInt32& OfsPoint) {

	 GUInt32 i, j, ofs0, k, q;
	 GPoint2 p, p1, p2;

	 j = (GUInt32)Index.size();
	 ofs0 = 0;
	 for (i = 0; i < j; i++) {
		 k = Index[i];
		 for (q = 0; q < k; q++) {
			 p = Points[ofs0 + q];
			 if (p[G_X] < XValue) {

				p1 = Points[ofs0];
				p2 = Points[ofs0 + k - 1];
				if ((p1[G_X] > XValue) && (p2[G_X] > XValue)) {
					OfsPoint = ofs0;
					return i;
				}
				else {
					OfsPoint = ofs0 + k;
					return (i + 1);
				}
			 }
		 }
		 ofs0 += k;
	 }
	 return -1;
}

GInt32 GFontCharContour2D::IntersectionsCount(const GRay2& NormalizedRay) const {

	GDynArray<GPoint2> tmpPoints;
	GDynArray<GInt32> tmpIndex;
	GDynArray<GVector2> tmpIntersections;
	GIntersectInfo2 intInfo;
	GBezierCurve2D tmpBezier;
	GLineSegment2 tmpSegm;
	GUInt32 i, j, k, ofs0, intCount, q, w, bezCount, ofs, hh, ww;
	GPoint2 p1, p2, p3, p4, p, pu0, pu1;
	GVector2 v;
	GUInt32 intersFlags;
	GReal u, u0, u1, a, b, c, dotDirections, fSign, den;
	GBool bInt;
	GReal localIntersParams[2];
	GInt32 currentSgn, tmpSgn, arcIdx;

	DecomposeBezier(tmpPoints, tmpIndex);
	currentSgn = -2;
	intCount = 0;
	p = NormalizedRay.Origin();
	// find a left Bezier arc, and take the next one
	arcIdx = FindLeftArc(tmpPoints, tmpIndex, p[G_X], ofs);
	if (arcIdx < 0)
		return 0;
	//ofs0 = ofs + tmpIndex[arcIdx];
	//arcIdx++;
	ofs0 = ofs;

	j = (GUInt32)tmpIndex.size();
	ww = (GUInt32)tmpPoints.size();
	for (i = (GUInt32)arcIdx; i < j + (GUInt32)arcIdx; i++) {
		hh = i % j;
		k = tmpIndex[hh];
		// this segment is a line
		if (k == 2) {
			p1 = tmpPoints[ofs0++];
			p2 = tmpPoints[ofs0++];
			v = p2 - p1;
			v.Normalize();
			dotDirections = Dot(v, NormalizedRay.Direction());
			// segment is parallel to ray
			if ((GReal)1 - GMath::Abs(dotDirections) < G_EPSILON) {
				/*
				// are end points at the same yvalue of ray.origin?
				if ((GMath::Abs(p1[G_Y] - p[G_Y]) < G_EPSILON) && (GMath::Abs(p2[G_Y] - p[G_Y]) < G_EPSILON)) {
					// sort x coordinates of end points
					x1 = x2 = p1[G_X];
					if (p2[G_X] > p1[G_X])
						x2 = p2[G_X];
					else
						x1 = p2[G_X];
					if ((p[G_X] > x1) && (p[G_X] < x2)) {
						// this is a degenerative case, treated away with boolean operations
					}
				}*/
			}
			else {
				tmpSegm.SetStartPoint(p1);
				tmpSegm.SetEndPoint(p2);
				bInt = Intersect(NormalizedRay, tmpSegm, intersFlags, localIntersParams);
				if (bInt) {
					tmpSgn = GMath::Sign(p2[G_Y] - p1[G_Y]);
					if (tmpSgn != currentSgn) {
						intCount++;
					}
					currentSgn = tmpSgn;
				}
			}
		}
		// this is a Bezier arc (quadric or cubic)
		else {
			tmpIntersections.clear();
			p1 = tmpPoints[ofs0++];
			p2 = tmpPoints[ofs0++];
			p3 = tmpPoints[ofs0++];
			// cubic arc
			if (k == 4) {
				p4 = tmpPoints[ofs0++];
				tmpBezier.SetPoints(p1, p2, p3, p4);
				bInt = tmpBezier.IntersectRay(NormalizedRay, tmpIntersections);
				if (!bInt)
					goto nextArc;
				bezCount = (GUInt32)tmpIntersections.size();
				a = p1[G_Y] + p2[G_Y] - 3 * p3[G_Y] + p4[G_Y];
				b = 2 * (p3[G_Y] - p2[G_Y]);
				c = p2[G_Y] - p1[G_Y];
				w = GMath::QuadraticFormula(u0, u1, a, b, c);
				// lets sort optimum
				if (u1 < u0) {
					u = u1;
					u1 = u0;
					u0 = u;
				}
				pu0 = tmpBezier.Evaluate(u0);
				pu1 = tmpBezier.Evaluate(u1);
				for (q = 0; q < bezCount; q++) {
					u = tmpIntersections[q][G_X];
					if ((u == 0) || (u == 1)) {
						if (u == 0)
							tmpSgn = GMath::Sign(pu0[G_Y] - p1[G_Y]);
						else
							tmpSgn = GMath::Sign(p4[G_Y] - pu1[G_Y]);
						if (currentSgn != tmpSgn) {
							intCount++;
							currentSgn = tmpSgn;
						}
					}
					else {
						if ((u != u0) && (u != u1)) {
							intCount++;
							// we have to update sign
							if ((u > 0) && (u < u0))
								currentSgn = GMath::Sign(pu0[G_Y] - p1[G_Y]);
							else
							if ((u > u0) && (u < u1))
								currentSgn = GMath::Sign(pu1[G_Y] - pu0[G_Y]);
							else
							if ((u > u1) && (u < 1))
								currentSgn = GMath::Sign(p4[G_Y] - pu1[G_Y]);
						}
						else {
							// this is the case of 2 maximum points coincident
							if (w == 1) {
								// we have to update sign
								currentSgn = GMath::Sign(p4[G_Y] - pu0[G_Y]);
								// check if we can hold intersection
								fSign = (p4[G_Y] - p[G_Y]) * (p1[G_Y] - p[G_Y]);
								if (fSign < 0) {
									intCount++;
								}
							}
							else {
								// we have to discard the solution, but we must update sign
								if (u == u0)
									currentSgn = GMath::Sign(pu1[G_Y] - pu0[G_Y]);
								else
									currentSgn = GMath::Sign(p4[G_Y] - pu1[G_Y]);
							}
						}
					}
				}
			}
			// quadric
			else {
				tmpBezier.SetPoints(p1, p2, p3);
				bInt = tmpBezier.IntersectRay(NormalizedRay, tmpIntersections);
				if (!bInt)
					goto nextArc;
				bezCount = (GUInt32)tmpIntersections.size();
				// we have to update sign of last intersection
				if (bezCount == 2) {
					den = p1[G_Y] - 2 * p2[G_Y] + p3[G_Y];
					if (den != 0) {
						// this is the value that maximize tmpBezier.y
						u = (p1[G_Y] - p2[G_Y]) / den;
						if ((u > 0) && (u < 1)) {
							p4 = tmpBezier.Evaluate(u);
							currentSgn = GMath::Sign(p3[G_Y] - p4[G_Y]);
						}
					}
				}
				if (bezCount == 1) {
					fSign = (p1[G_Y] - p[G_Y]) * (p3[G_Y] - p[G_Y]);
					if (fSign < 0) {
						intCount++;
						currentSgn = GMath::Sign(p3[G_Y] - p1[G_Y]);
					}
					else
					// we can think this arc as a line joining p1 - p3 
					if (GMath::Abs(fSign) < G_EPSILON) {
						tmpSgn = GMath::Sign(p3[G_Y] - p1[G_Y]);
						if (tmpSgn != currentSgn) {
							intCount++;
						}
						currentSgn = tmpSgn;
					}
					else {
						// this is a tangent case, we can discard it
					}
				}
			}
		}
nextArc:
		if (ofs0 >= ww)
			ofs0 = 0;
	}

	return intCount;
}

GError GFontCharContour2D::ConvertToPath(GPath2D& Path) const {

	GDynArray<GPoint2> pts;
	GDynArray<GInt32> idx;
	GInt32 i, j, k, w, ofs;
	GBezierCurve2D tmpBezier;
	GPolyLineCurve2D tmpPolyline;
	GDynArray<GPoint2> polyPts;
	GError err;

	GReal step, u, u0;

	DecomposeBezier(pts, idx);
	Path.Clear();
	j = (GInt32)idx.size();

	u = 0;
	step = (GReal)1 / (GReal)j;
	ofs = 0;
	for (i = 0; i < j; i++) {
		k = idx[i];
		if (k == 4) {
			tmpBezier.SetPoints(pts[ofs], pts[ofs + 1], pts[ofs + 2], pts[ofs + 3]);
			tmpBezier.SetDomain(u, u + step);
			err = Path.AppendCurve(tmpBezier);
			if (err != G_NO_ERROR)
				return err;

			u += step;
			ofs += 4;
		}
		else
		if (k == 3) {
			tmpBezier.SetPoints(pts[ofs], pts[ofs + 1], pts[ofs + 2]);
			tmpBezier.SetDomain(u, u + step);
			err = Path.AppendCurve(tmpBezier);
			if (err != G_NO_ERROR)
				return err;

			u += step;
			ofs += 3;
		}
		else
		if (k == 2) {
			u0 = u;
			polyPts.clear();
			w = i;
			do {
				polyPts.push_back(pts[ofs]);
				ofs += 2;
				u += step;
				w++;
				i++;
			} while(w < j && idx[w] == 2);
			polyPts.push_back(pts[ofs - 1]);

			i--;
			tmpPolyline.SetPoints(polyPts, u0, u);
			err = Path.AppendCurve(tmpPolyline);
			if (err != G_NO_ERROR)
				return err;
		}
	}
	Path.ClosePath();
	return G_NO_ERROR;
}

void GFontCharContour2D::MirrorPoints() const {

	ReverseArray(gPoints, 1, (GUInt32)(gPoints.size() - 1));
	ReverseArray(gPointsFlags, 1, (GUInt32)(gPointsFlags.size() - 1));
}

// *********************************************************************
//                              GFontChar
// *********************************************************************

// copy constructor
GFontChar2D::GFontChar2D(const GFontChar2D& Source) {

	gFont = Source.gFont;
	gContours = Source.gContours;
	gSubChars = Source.gSubChars;
	gMetrics = Source.gMetrics;
	gLinearHoriAdvance = Source.gLinearHoriAdvance;
	gLinearVertAdvance = Source.gLinearVertAdvance;
	gAdvance = Source.gAdvance;
	gLSBDelta = Source.gLSBDelta;
	gRSBDelta = Source.gRSBDelta;
	gEvenOddFill = Source.gEvenOddFill;
	gInitialized = Source.gInitialized;
}

// constructor
GFontChar2D::GFontChar2D(const GFont2D* Owner, const GDynArray<GFontCharContour2D>& Contours) {

	gInitialized = G_FALSE;
	gMetrics.Width = 0;
	gMetrics.Height = 0;
	gMetrics.HoriBearingX = 0;
	gMetrics.HoriBearingY = 0;
	gMetrics.HoriAdvance = 0;
	gMetrics.VertBearingX = 0;
	gMetrics.VertBearingY = 0;
	gMetrics.VertAdvance = 0;
	gLinearHoriAdvance = 0;
	gLinearVertAdvance = 0;
	gAdvance.Set(0, 0);
	gLSBDelta = 0;
	gRSBDelta = 0;
	gEvenOddFill = G_FALSE;
	gFont = Owner;
	gContours = Contours;
}

// constructor
GFontChar2D::GFontChar2D(const GFont2D* Owner, const GDynArray<GSubChar2D>& SubChars) {

	gInitialized = G_FALSE;
	gMetrics.Width = 0;
	gMetrics.Height = 0;
	gMetrics.HoriBearingX = 0;
	gMetrics.HoriBearingY = 0;
	gMetrics.HoriAdvance = 0;
	gMetrics.VertBearingX = 0;
	gMetrics.VertBearingY = 0;
	gMetrics.VertAdvance = 0;
	gLinearHoriAdvance = 0;
	gLinearVertAdvance = 0;
	gAdvance.Set(0, 0);
	gLSBDelta = 0;
	gRSBDelta = 0;
	gEvenOddFill = G_FALSE;
	gFont = Owner;
	gSubChars = SubChars;
}

// destructor
GFontChar2D::~GFontChar2D() {

	DeleteContours();
}

void GFontChar2D::DeleteContours() {

	GUInt32 i, j;

	// delete all contours
	j = (GUInt32)gContours.size();
	for (i = 0; i < j; ++i) {
		GFontCharContour2D& c = gContours[i];
		c.gPoints.clear();
		c.gPointsFlags.clear();
	}
	gContours.clear();
}


void GFontChar2D::Initialize() const {
	
	// make a good vectorization
	LabelHolesAndFilled();
	SwapHolesAndFilledLabels(gEvenOddFill);
	FixHolesAndFilledWiseOrder();
}

// convert each contour that builds the char, into a path representation
GError GFontChar2D::ConvertToPaths(GDynArray<GPath2D *>& Paths) const {

	GInt32 i, j, k;
	const GFontChar2D *c;
	GPath2D *tmpPath;
	GError err;

	if (gInitialized == G_FALSE) {
		// make a good vectorization; if this char is composite, it's a good choice to not do
		// an initialization, because there aren't contours. So initialization would be just a
		// waste of time
		if (!IsComposite())
			Initialize();
		gInitialized = G_TRUE;
	}

	if (IsComposite()) {
		// owner (font) is necessary to get sub chars pointers
		if (!gFont)
			return G_MEMORY_ERROR;
	
		j = SubCharsCount();
		for (i = 0; i < j; i++) {
			// get subchar
			c = gFont->CharByIndex(gSubChars[i].GlyphIndex);
			if (c) {
				err = c->ConvertToPaths(Paths);
				if (err != G_NO_ERROR)
					return err;
			}
		}
	}
	else {
		if (!gFont || !gFont->Owner())
			return G_UNSUPPORTED_CLASSID;
		GKernel *ker = (GKernel *)gFont->Owner();


		j = (GInt32)gContours.size();
		for (i = 0; i < j; i++) {
			// try to crate a new path
			tmpPath = (GPath2D *)ker->CreateNew(G_PATH2D_CLASSID);
			// roll back, removing all previous created paths
			if (!tmpPath) {
				for (k = 0; k < i; k++) {
					tmpPath = Paths.back();
					delete tmpPath;
					Paths.pop_back();
				}
				return G_UNSUPPORTED_CLASSID;
			}
			err = gContours[i].ConvertToPath(*tmpPath);
			if (err != G_NO_ERROR) {
				for (k = 0; k < i; k++) {
					tmpPath = Paths.back();
					delete tmpPath;
					Paths.pop_back();
				}
				return err;
			}
			Paths.push_back(tmpPath);
		}
	}
	return G_NO_ERROR;
}

void GFontChar2D::LabelContour(const GFontCharContour2D& Contour) const {

	GUInt32 i, j, numIntersections, k;
	GPoint2 p;
	GRay2 ray;
	GDynArray<GReal> monChain;

	j = (GUInt32)gContours.size();
	p = Contour.PickPointOnCurve();
	ray.SetOrigin(p);
	ray.SetDirection(GVector2(1, 0));
	k = 0;
	for (i = 0; i < j; i++) {
		const GFontCharContour2D& c = gContours[i];
		// we are not interested in intersection between Contour and itself
		if (&c == &Contour)
			continue;
		// test for a potentially intersected contour
		if ((c.BoundBox().Min()[G_X] < p[G_X]) && (c.BoundBox().Max()[G_X] < p[G_X]))
			continue;
		if ((c.BoundBox().Min()[G_Y] < p[G_Y]) && (c.BoundBox().Max()[G_Y] < p[G_Y]))
			continue;
		if ((c.BoundBox().Min()[G_Y] > p[G_Y]) && (c.BoundBox().Max()[G_Y] > p[G_Y]))
			continue;

		if ((Contour.BoundBox().Min()[G_X] >= c.BoundBox().Min()[G_X]) &&
			(Contour.BoundBox().Max()[G_X] <= c.BoundBox().Max()[G_X]) &&
			(Contour.BoundBox().Min()[G_Y] >= c.BoundBox().Min()[G_Y]) &&
			(Contour.BoundBox().Max()[G_Y] <= c.BoundBox().Max()[G_Y])) {
			
			numIntersections = c.IntersectionsCount(ray);
			k += numIntersections;
		}
	}
	if (k == 0)
		Contour.gIsHole = G_FALSE;
	else {
		// Contour is an hole if k is odd
		if (k & 1)
			Contour.gIsHole = G_TRUE;
		else
			Contour.gIsHole = G_FALSE;
	}
}

void GFontChar2D::LabelHolesAndFilled() const {

	GUInt32 i, j;

	j = (GUInt32)gContours.size();
	for (i = 0; i < j; i++)
		LabelContour(gContours[i]);
}

void GFontChar2D::SwapHolesAndFilledLabels(const GBool EvenOddFlag) const {

	GUInt32 i, j;

	j = (GUInt32)gContours.size();
	for (i = 0; i < j; i++) {
		const GFontCharContour2D& c = gContours[i];
		if (EvenOddFlag)
			c.gIsHole = !c.gIsHole;
	}
}

void GFontChar2D::FixHolesAndFilledWiseOrder() const {

	GBool clockWise;
	GUInt32 i, j;
	GPoint2 tmpPoint;

	j = (GUInt32)gContours.size();
	for (i = 0; i < j; i++) {
		const GFontCharContour2D& c = gContours[i];
		clockWise = IsClockWise(c.Points());
		if (c.IsHole() && clockWise)
			continue;
		if (!c.IsHole() && !clockWise)
			continue;
		// lets mirror points
		c.MirrorPoints();
	}
}

GError GFontChar2D::SubChar(const GUInt32 Index, GSubChar2D& SubCharInfo) const {

	if (Index >= gSubChars.size())
		return G_OUT_OF_RANGE;

	if (gInitialized == G_FALSE) {
		// make a good vectorization; if this char is composite, it's a good choice to not do
		// an initialization, because there aren't contours. So initialization would be just a
		// waste of time
		if (!IsComposite())
			Initialize();
		gInitialized = G_TRUE;
	}
	SubCharInfo = gSubChars[Index];
	return G_NO_ERROR;
}

const GFontCharContour2D* GFontChar2D::Contour(const GUInt32 Index) const {

	if (Index >= gContours.size())
		return NULL;
	if (gInitialized == G_FALSE) {
		// make a good vectorization; if this char is composite, it's a good choice to not do
		// an initialization, because there aren't contours. So initialization would be just a
		// waste of time
		if (!IsComposite())
			Initialize();
		gInitialized = G_TRUE;
	}
	return &(gContours[Index]);
}

// *********************************************************************
//                              GFont
// *********************************************************************

// constructor
GFont2D::GFont2D() : GElement() {

	gSubFontsCount = -1;
	gUnitsPerEM = 0;
	gAscender = 0;
	gDescender = 0;
	gExternalLeading = 0;
	gMaxAdvanceWidth = -1;
	gMaxAdvanceHeight = -1;
	gUnderlinePosition = -1;
	gUnderlineThickness = -1;
	gItalic = G_FALSE;
	gBold = G_FALSE;
}

// constructor
GFont2D::GFont2D(const GElement* Owner) : GElement(Owner) {

	gSubFontsCount = -1;
	gUnitsPerEM = 0;
	gAscender = 0;
	gDescender = 0;
	gExternalLeading = 0;
	gMaxAdvanceWidth = -1;
	gMaxAdvanceHeight = -1;
	gUnderlinePosition = -1;
	gUnderlineThickness = -1;
	gItalic = G_FALSE;
	gBold = G_FALSE;
}

// destructor
GFont2D::~GFont2D() {

	Clear();
}

// delete every instanced char
void GFont2D::DeleteChars() {

	GFontChar2D *c;
	GUInt32 i, j;

	j = (GUInt32)gChars.size();
	for (i = 0; i < j; i++) {
		c = gChars[i];
		delete c;
	}
	gChars.clear();
}

// delete kerning informations
void GFont2D::DeleteKerning() {

	gKerningTable.clear();
}

// sort function for kerning sorting
inline bool KerningCompare(const GKerningEntry& Arg1, const GKerningEntry& Arg2) {

	if (Arg1.GlyphIndexLeft < Arg2.GlyphIndexLeft)
		return true;
	else
	if (Arg1.GlyphIndexLeft > Arg2.GlyphIndexLeft)
		return false;
	else {
		if (Arg1.GlyphIndexRight < Arg2.GlyphIndexRight)
			return true;
		else
		if (Arg1.GlyphIndexRight > Arg2.GlyphIndexRight)
			return false;
		else
			return true;
	}
}

// set kerning infos
void GFont2D::SetKerning(const GDynArray<GKerningEntry>& NewKerningTable) {

	GUInt32 i = (GUInt32)NewKerningTable.size();

	if (i == 0)
		gKerningTable.clear();
	else {
		gKerningTable = NewKerningTable;
		// we have to do a sort of every NewKerningTable entry
		std::sort(gKerningTable.begin(), gKerningTable.end(), KerningCompare);
	}
}

// clone chars
GError GFont2D::CloneChars(const GDynArray<GFontChar2D *>& Chars, GDynArray<GFontChar2D *>& Destination) {

	GUInt32 i, j;

	j = (GUInt32)Chars.size();
	for (i = 0; i < j; i++) {
		GFontChar2D *newChar = new(std::nothrow) GFontChar2D(*(Chars[i]));
		if (!newChar)
			return G_MEMORY_ERROR;
		newChar->gFont = this;
		Destination.push_back(newChar);
	}
	return G_NO_ERROR;
}

// cloning routine
GError GFont2D::BaseClone(const GElement& Source) {

	const GFont2D& k = (const GFont2D&)Source;
	GError err;
	GDynArray<GFontChar2D *> tmpChars;

	// clear the font (free allocated memory)

	err = CloneChars(k.gChars, tmpChars);
	if (err != G_NO_ERROR) {
		GUInt32 i, j = (GUInt32)tmpChars.size();
		for (i = 0; i < j; i++) {
			GFontChar2D *c = tmpChars[i];
			delete c;
		}
		return err;
	}

	Clear();

	gFileName = k.gFileName;
	gFamilyName = k.gFamilyName;
	gStyleName = k.gStyleName;
	gSubFontsCount = k.gSubFontsCount;
	gUnitsPerEM = k.gUnitsPerEM;
	gAscender = k.gAscender;
	gDescender = k.gDescender;
	gExternalLeading = k.gExternalLeading;
	gMaxAdvanceWidth = k.gMaxAdvanceWidth;
	gMaxAdvanceHeight = k.gMaxAdvanceHeight;
	gUnderlinePosition = k.gUnderlinePosition;
	gUnderlineThickness = k.gUnderlineThickness;
	gItalic = k.gItalic;
	gBold = k.gBold;
	gChars = tmpChars;
	gCharsMaps = k.gCharsMaps;
	gKerningTable = k.gKerningTable;

	return G_NO_ERROR;
}

const GFontChar2D* GFont2D::CharByIndex(const GInt32 Index) const {

	const GFontChar2D *c;

	if ((Index < 0) || (Index >= CharsCount()))
		return NULL;
	c = gChars[Index];
	return c;
}

// add a char map
GError GFont2D::AddCharMap(const GCharMap& NewCharMap) {

	GUInt32 i, j, found;

	j = (GUInt32)gCharsMaps.size();
	found = 0;
	for (i = 0; i < j; i++) {
		if ((gCharsMaps[i].PlatformID == NewCharMap.PlatformID) &&
			(gCharsMaps[i].EncodingID == NewCharMap.EncodingID) &&
			(gCharsMaps[i].Encoding == NewCharMap.Encoding))
			found = 1;
	}
	if (!found) {
		gCharsMaps.push_back(NewCharMap);
		return G_NO_ERROR;
	}
	else
		return G_ENTRY_ALREADY_EXISTS;
}

// remove a char map
GError GFont2D::RemoveCharMap(const GInt32 CharMapIndex) {

	if ((CharMapIndex < 0) || (CharMapIndex >= CharMapsCount()))
		return G_OUT_OF_RANGE;

	GDynArray<GCharMap>::iterator it = gCharsMaps.begin();

	it += CharMapIndex;
	(*it).CharMap.clear();
	gCharsMaps.erase(it);
	return G_NO_ERROR;
}

// remove all char maps
void GFont2D::RemoveCharsMaps() {

	GUInt32 i, j;

	j = (GUInt32)gCharsMaps.size();
	for (i = 0; i < j; i++)
		gCharsMaps[i].CharMap.clear();
	gCharsMaps.clear();
}

inline bool EncodedCharCmp(const GEncodedChar& Char1, const GEncodedChar& Char2) {

	if (Char1.CharCode < Char2.CharCode)
		return true;
	return false;
}

// return (glyph) index, given a charcode
GInt32 GFont2D::CharIndexByCharCode(const GUInt32 CharCode, const GUInt32 CharMapIndex) const {

	GInt32 i;
	GEncodedChar tmpChar;

	i = CharMapsCount();
	if (i <= 0)
		return -1;

	if (CharMapIndex >= (GUInt32)i)
		return -1;

	tmpChar.CharCode = CharCode;

	GDynArray<GEncodedChar>::const_iterator it;
	
	it = std::lower_bound(gCharsMaps[CharMapIndex].CharMap.begin(), gCharsMaps[CharMapIndex].CharMap.end(),
						  tmpChar, EncodedCharCmp);
	if (it != gCharsMaps[CharMapIndex].CharMap.end() && it->CharCode == CharCode)
		return (it->GlyphIndex);
	else
		return -1;
}

const GFontChar2D* GFont2D::CharByCode(const GUInt32 CharCode, const GUInt32 CharMapIndex) const {

	GInt32 i;

	i = CharIndexByCharCode(CharCode, CharMapIndex);
	if (i >= 0)
		return gChars[i];
	return NULL;
}

// load a font file
GError GFont2D::Load(const GChar8 *FileName, const GChar8 *Options, const GChar8 *FormatName) {

	if (Owner()) {
		GKernel *k = (GKernel *)Owner();
		return k->Load(FileName, *this, Options, FormatName);
	}
	return G_MISSING_KERNEL;
}

const GPoint2& GFont2D::KerningByIndex(const GUInt32 LeftIndex, const GUInt32 RightIndex) const {

	GUInt32 first, last, pivot;
	GInt32 j;
	GKerningEntry entry;

	j = CharsCount();
	if (j <= 0)
		return G_NULL_POINT2;

	if ((LeftIndex >= (GUInt32)j) || (RightIndex >= (GUInt32)j))
		return G_NULL_POINT2;

	// do a binary search
	first = 0;
	last = (GUInt32)gKerningTable.size();
	while (last - first > 1) {
		pivot = (first + last) / 2;
		entry = gKerningTable[pivot];
		if ((entry.GlyphIndexLeft == LeftIndex) && (entry.GlyphIndexRight == RightIndex))
			return entry.Kerning;
		else
		if (LeftIndex > entry.GlyphIndexLeft)
			first = pivot;
		else {
			if (LeftIndex < entry.GlyphIndexLeft)
				last = pivot;
			else {
				// in this case LeftIndex == entry.GlyphIndexLeft, and we have to check second index
				if (RightIndex > entry.GlyphIndexRight)
					first = pivot;
				else
					last = pivot;
			}
		}
	}
	// check for first entry
	entry = gKerningTable[first];
	if ((entry.GlyphIndexLeft == LeftIndex) && (entry.GlyphIndexRight == RightIndex))
		return entry.Kerning;
	// check for last entry
	entry = gKerningTable[last];
	if ((entry.GlyphIndexLeft == LeftIndex) && (entry.GlyphIndexRight == RightIndex))
		return entry.Kerning;
	return G_NULL_POINT2;
}

const GPoint2& GFont2D::KerningByCharCode(const GUInt32 LeftGlyph, const GUInt32 RightIndex,
										const GUInt32 CharMapIndex) const {

	GInt32 leftIdx, rightIdx;

	// get glyph index
	leftIdx = CharIndexByCharCode(LeftGlyph, CharMapIndex);
	rightIdx = CharIndexByCharCode(RightIndex, CharMapIndex);
	if ((leftIdx >= 0) && (rightIdx >= 0))
		return KerningByIndex(leftIdx, rightIdx);
	return G_NULL_POINT2;
}

// insert a new char, based on specified contours
GFontChar2D* GFont2D::AddChar(const GDynArray<GFontCharContour2D>& Contours) {

	GFontChar2D *c;

	c = new GFontChar2D(this, Contours);
	gChars.push_back(c);
	return c;
}

// insert a new char, based on specified subchars
GFontChar2D* GFont2D::AddChar(const GDynArray<GSubChar2D>& SubChars) {

	GFontChar2D *c;

	c = new GFontChar2D(this, SubChars);
	gChars.push_back(c);
	return c;
}

// remove a char
GError GFont2D::RemoveChar(const GInt32 Index) {

	GFontChar2D *c;
	GDynArray<GFontChar2D *>::iterator it;

	if ((Index < 0) || (Index >= CharsCount()))
		return G_OUT_OF_RANGE;

	it = gChars.begin();
	c = *it;
	// remove char form internal list
	gChars.erase(it);
	// delete memory
	delete c;
	// return without errors
	return G_NO_ERROR;
}

// clear the font; it eliminates all chars, charsmaps and kerning informations
void GFont2D::Clear() {

	// delete chars
	DeleteChars();
	// delete kerning infos
	DeleteKerning();
	// delete charsmaps
	RemoveCharsMaps();

	gSubFontsCount = -1;
	gUnitsPerEM =0;
	gAscender = 0;
	gDescender = 0;
	gExternalLeading = 0;
	gMaxAdvanceWidth = -1;
	gMaxAdvanceHeight = -1;
	gUnderlinePosition = -1;
	gUnderlineThickness = -1;
	gItalic = G_FALSE;
	gBold = G_FALSE;
}

}
