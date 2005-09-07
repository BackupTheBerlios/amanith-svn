/****************************************************************************
** $file: amanith/src/2d/gtracer2d.cpp   0.1.0.0   edited Jun 30 08:00
**
** 2D Bitmap tracer (vectorizer) implementation.
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

#include "amanith/support/gutilities.h"
#include "amanith/2d/gtracer2d.h"
#include "amanith/geometry/gxform.h"
#include "amanith/geometry/garea.h"
#include "amanith/numerics/geigen.h"
#include "amanith/2d/gpolylinecurve2d.h"
#include "amanith/2d/gbeziercurve2d.h"

/*!
	\file gtracer2d.cpp
	\brief 2D bitmap vectorizer implementation.
*/

namespace Amanith {

#define CURVETO 1
#define CORNER 2

// *********************************************************************
//                          GTracedContour
// *********************************************************************
void GTracedContour::DrawContour(GDynArray<GPoint2>& Points, const GReal Variation) const {

	GUInt32 i, j = (GUInt32)gPointFlags.size(), k;
	GPoint2 cursor;
	GBezierCurve2D bezDrawer;
	GInt32 flag1, flag2;
	ThreePoints c;

	if (j == 0)
		return;

	c = gPoints[j - 1];
	cursor = c.Points[2];

	for (i = 0; i < j; i++) {
		if (i == j - 1)
			k = 0;
		else
			k = i + 1;
		flag1 = gPointFlags[i];
		flag2 = gPointFlags[k];
		if (flag1 == CORNER) {
			Points.push_back(gPoints[i].Points[1]);
			if (flag2 == CORNER)
				// corner - corner
				cursor = gPoints[k].Points[1];
			else
				// corner - Bezier
				cursor = gPoints[i].Points[2];
		}
		else {
			bezDrawer.SetPoints(cursor, gPoints[i].Points[0], gPoints[i].Points[1], gPoints[i].Points[2]);
			bezDrawer.Flatten(Points, Variation, G_FALSE);
			cursor = gPoints[i].Points[2];
		}
	}
}

GError GTracedContour::ConvertToPath(GPath2D& Path) const {

	GUInt32 i, j = (GUInt32)gPointFlags.size(), w;
	GPoint2 cursor;
	GReal u, u0, step;
	GBezierCurve2D tmpBezier;
	GPolyLineCurve2D tmpPolyline;
	GDynArray<GPoint2> polyPts;
	GError err;
	ThreePoints c;
	GPoint2 a, b;

	if (j == 0)
		return G_INVALID_OPERATION;

	c = gPoints[j - 1];
	cursor = c.Points[2];

	u = 0;
	step = (GReal)1 / (GReal)j;

	for (i = 0; i < j; i++) {

		if (gPointFlags[i] == CORNER) {
			u0 = u;
			polyPts.clear();
			w = i;
			a = cursor;
			polyPts.push_back(cursor);
			do {
				a = gPoints[w].Points[1];
				polyPts.push_back(gPoints[w].Points[1]);
				u += step;
				w++;
				i++;
			} while(w < j && gPointFlags[w] == CORNER);
			if (w < j) {
				// at this point we are sure that w-th segment is a Bezier
				b = gPoints[w - 1].Points[2];
				cursor = gPoints[w - 1].Points[2];
				polyPts.push_back(gPoints[w - 1].Points[2]);
			}
			else {
				b = gPoints[w - 1].Points[2];
				polyPts.push_back(gPoints[w - 1].Points[2]);
			}


			i--;
			tmpPolyline.SetPoints(polyPts, u0, u);
			err = Path.AppendCurve(tmpPolyline);
		}
		else {
			tmpBezier.SetPoints(cursor, gPoints[i].Points[0], gPoints[i].Points[1], gPoints[i].Points[2]);
			tmpBezier.SetDomain(u, u + step);
			err = Path.AppendCurve(tmpBezier);

			cursor = gPoints[i].Points[2];
			u += step;
		}
	}

	Path.ClosePath();
	return G_NO_ERROR;
}

// *********************************************************************
//                            GTracer2D
// *********************************************************************

GBool GTracer2D::FindBlackPixel(const GPixelMap& Image, const GUChar8 WhiteColor,
								 const GInt32 StartY, GPoint<GInt32, 2>& PixelCoords) {

	GInt32 x, x2, y = StartY, ofs;
	GUChar8 *pixels;

	pixels = (GUChar8 *)Image.Pixels();
	while (y >= 0) {
		for (x = 0; x < Image.Width(); x++) {
			ofs = y * Image.Width() + x;
			// pixel found
			if (pixels[ofs] != WhiteColor) {
				x2 = x + 1;
				ofs++;
				while ((x2 < Image.Width()) && (pixels[ofs] != WhiteColor)) {
					x2++;
					ofs++;
				}
				PixelCoords[G_X] = x2;
				PixelCoords[G_Y] = y;
				return G_TRUE;
			}
		}
		y--;
	}
	// pixel not found
	return G_FALSE;
}

GBool GTracer2D::BlackDominance(const GPixelMap& Image, const GPoint<GInt32, 2>& Center,
								const GUChar8 WhiteColor, const GInt32 MaxRadius) {

	GInt32 x, y, bCount;
	GUInt32 pixel;

	for (y = 2; y < MaxRadius; y++) {

		bCount = 0;

		for (x = -y + 1; x <= y - 1; x++) {
			Image.Pixel(Center[G_X] + x, Center[G_Y] + y - 1, pixel);
			if (pixel != WhiteColor)
				bCount++;
			else
				bCount--;
			Image.Pixel(Center[G_X] + y - 1, Center[G_Y] + x - 1, pixel);
			if (pixel != WhiteColor)
				bCount++;
			else
				bCount--;
			Image.Pixel(Center[G_X] + x - 1, Center[G_Y] - y, pixel);
			if (pixel != WhiteColor)
				bCount++;
			else
				bCount--;
			Image.Pixel(Center[G_X] - y, Center[G_Y] + x, pixel);
			if (pixel != WhiteColor)
				bCount++;
			else
				bCount--;
		}
		if (bCount > 0)
			return G_TRUE;
		else
		if (bCount < 0)
			return G_FALSE;
	}
	return G_FALSE;
}

GError GTracer2D::BuildPath(const GPixelMap& Image, const GPoint<GInt32, 2>& StartPoint, const GUChar8 WhiteColor,
							const GInt32 Direction, const GTurnPolicy TurnPolicy,
							const GInt32 MaxRadius,	PixelPath& Path) {

	GInt32 tmp;
	GUInt32 p1, p2;
	GPoint<GInt32, 2> moveDir(0, -1);
	GPoint<GInt32, 2> padDir(-1, -1);
	GPoint<GInt32, 2> curPoint(StartPoint);
	GPoint<GInt32, 2> tmpDir;

	Path.Points.clear();
	Path.Length = 0;
	Path.Area = 0;
	Path.Direction = Direction;

	while (1) {
		// add point to path
		Path.Points.push_back(curPoint);
		Path.Length++;
		// move to next point
		curPoint += moveDir;
		// update area	
		Path.Area += curPoint[G_X] * moveDir[G_Y];
		// test for completed path
		if (curPoint == StartPoint)
			return G_NO_ERROR;

		// determine next direction
		Image.Pixel(curPoint[G_X] + (moveDir[G_X] + moveDir[G_Y] - 1) / 2,
					curPoint[G_Y] + (moveDir[G_Y] - moveDir[G_X] - 1) / 2, p1);

		Image.Pixel(curPoint[G_X] + (moveDir[G_X] - moveDir[G_Y] - 1) / 2,
					curPoint[G_Y] + (moveDir[G_Y] + moveDir[G_X] - 1) / 2, p2);

		// we have a choice of whether to take a left turn or a right turn; so lets use turn policy
		if ((p1 == WhiteColor) && (p2 != WhiteColor)) {
			if ((TurnPolicy == G_CONNECT_WHITE && Direction == 1) ||
				(TurnPolicy == G_CONNECT_BLACK && Direction == -1) ||
				(TurnPolicy == G_CONNECT_DOMINANT && !BlackDominance(Image, curPoint, WhiteColor, MaxRadius)) ||
				(TurnPolicy == G_CONNECT_NOT_DOMINANT && BlackDominance(Image, curPoint, WhiteColor, MaxRadius))) {
				// right turn
				tmp = moveDir[G_X];
				moveDir[G_X] = moveDir[G_Y];
				moveDir[G_Y] = -tmp;
			}
			// left turn
			else {
				tmp = moveDir[G_X];
				moveDir[G_X] = -moveDir[G_Y];
				moveDir[G_Y] = tmp;
			}
		}
		else
		// right turn
		if (p1 == WhiteColor) {
			tmp = moveDir[G_X];
			moveDir[G_X] = moveDir[G_Y];
			moveDir[G_Y] = -tmp;
		}
		else
		// left turn
		if (p2 != WhiteColor) {
			tmp = moveDir[G_X];
			moveDir[G_X] = -moveDir[G_Y];
			moveDir[G_Y] = tmp;
		}
	}
	return G_UNKNOWN_ERROR;
}

GError GTracer2D::XorUpdate(const PixelPath& Path, GPixelMap& DestImage, const GUChar8 WhiteColor) {

	GInt32 xNew, yNew, i, yOld, j, k, ofs, minY;
	GUChar8 *pixel8, gray;

	if (Path.Length <= 0)
		return G_NO_ERROR;

	pixel8 = (GUChar8 *)DestImage.Pixels();
	j = (GInt32)Path.Points.size();
	yOld = Path.Points[0][G_Y];
	for (i = 0; i < j; i++) {
		xNew = Path.Points[i][G_X];
		yNew = Path.Points[i][G_Y];
		if (yNew != yOld) {
			minY = GMath::Min(yOld, yNew);
			ofs = minY * DestImage.Width();
			for (k = 0; k < xNew; k++) {
				gray = pixel8[ofs + k];
				if (gray != WhiteColor)
					pixel8[ofs + k] = WhiteColor;
				else
					pixel8[ofs + k] = (WhiteColor + 1) & 0xFF;
			}
			yOld = yNew;
		}
	}
	return G_NO_ERROR;
}

GError GTracer2D::StatisticalSums(const PixelPath& Path, GDynArray< GPoint<GReal, 5> >& SumsTable,
								  GInt32& X0, GInt32& Y0) {

	GInt32 i, x, y, x0, y0, n = Path.Length;

	// allocate enough space
	SumsTable.resize(Path.Length + 1);
	// start point
	X0 = x0 = Path.Points[0][G_X];
	Y0 = y0 = Path.Points[0][G_Y];

	SumsTable[0][0] = 0;
	SumsTable[0][1] = 0;
	SumsTable[0][2] = 0;
	SumsTable[0][3] = 0;
	SumsTable[0][4] = 0;
	// preparatory computation for later fast summing
	for (i = 0; i < n; i++) {
		x = Path.Points[i][G_X] - x0;
		y = Path.Points[i][G_Y] - y0;
		SumsTable[i + 1][0] = SumsTable[i][0] + x;					// x
		SumsTable[i + 1][1] = SumsTable[i][1] + y;					// y
		SumsTable[i + 1][2] = SumsTable[i][2] + GMath::Sqr(x);		// x^2
		SumsTable[i + 1][3] = SumsTable[i][3] + (x * y);			// xy
		SumsTable[i + 1][4] = SumsTable[i][4] + GMath::Sqr(y);		// y^2
	}
	return G_NO_ERROR;
}

static inline int PotraceMod(int a, int n) {
  return a>=n ? a%n : a>=0 ? a : n-1-(-1-a)%n;
}

// return 1 if a <= b < c < a, in a cyclic sense (mod n)
static inline int PotraceCyclic(int a, int b, int c) {
	if (a<=c)
		return (a<=b && b<c);
	else
		return (a<=b || b<c);
}

GError GTracer2D::FurthestLinSubPaths(const PixelPath& Path, GDynArray<GInt32>& SubPathsTable) {

	const GPoint<GInt32, 2> *pt = &(Path.Points[0]);
	GInt32 n = Path.Length;
	GInt32 i, j, k, k1;
	GInt32 ct[4], dir;
	GVect<GInt32, 2> constraint[2];
	GVect<GInt32, 2> cur;
	GVect<GInt32, 2> off;
	GDynArray<GInt32> pivk(n);
	GDynArray<GInt32> nc(n);
	GVect<GInt32, 2> dk;
	GInt32 a, b, c, d;

	k = 0;
	for (i = n - 1; i >= 0; i--) {
		if (pt[i][G_X] != pt[k][G_X] && pt[i][G_Y] != pt[k][G_Y])
			k = i + 1;  // necessarily i<n-1 in this case
		nc[i] = k;
	}

	SubPathsTable.resize(n);

	// determine pivot points: for each i, let pivk[i] be the furthest k
	// such that all j with i<j<k lie on a line connecting i, k
	  
	for (i = n - 1; i >= 0; i--) {
		ct[0] = ct[1] = ct[2] = ct[3] = 0;

		// keep track of "directions" that have occurred
		dir = (3 + 3 * (pt[PotraceMod(i + 1, n)][G_X] - pt[i][G_X]) + (pt[PotraceMod(i + 1, n)][G_Y] - pt[i][G_Y])) / 2;
		ct[dir]++;

		constraint[0].Set(0, 0);
		constraint[1].Set(0, 0);

		// find the next k such that no straight line from i to k
		k = nc[i];
		k1 = i;
		while (1) {
			dir = (3 + 3 * GMath::Sign(pt[k][G_X] - pt[k1][G_X]) + GMath::Sign(pt[k][G_Y] - pt[k1][G_Y])) / 2;
			ct[dir]++;
			// if all four "directions" have occurred, cut this path
			if (ct[0] && ct[1] && ct[2] && ct[3]) {
				pivk[i] = k1;
				goto foundk;
			}

			cur = pt[k] - pt[i];

			// see if current constraint is violated
			if (Cross(constraint[0], cur) < 0 || Cross(constraint[1], cur) > 0)
				goto constraint_viol;

			// else, update constraint
			if (GMath::Abs(cur[G_X]) <= 1 && GMath::Abs(cur[G_Y]) <= 1) {
				// no constraint
			}
			else {
				off[G_X] = cur[G_X] + ((cur[G_Y] >= 0 && (cur[G_Y] > 0 || cur[G_X] < 0)) ? 1 : -1);
				off[G_Y] = cur[G_Y] + ((cur[G_X] <= 0 && (cur[G_X] < 0 || cur[G_Y] < 0)) ? 1 : -1);
				if (Cross(constraint[0], off) >= 0)
					constraint[0] = off;
				off[G_X] = cur[G_X] + ((cur[G_Y] <= 0 && (cur[G_Y] < 0 || cur[G_X] < 0)) ? 1 : -1);
				off[G_Y] = cur[G_Y] + ((cur[G_X] >= 0 && (cur[G_X] > 0 || cur[G_Y] < 0)) ? 1 : -1);
				if (Cross(constraint[1], off) <= 0)
					constraint[1] = off;
			}
			k1 = k;
			k = nc[k1];
			if (!PotraceCyclic(k, i, k1))
				break;
		}
	constraint_viol:
		// k1 was the last "corner" satisfying the current constraint, and
		// k is the first one violating it. We now need to find the last
		// point along k1..k which satisfied the constraint
		dk.Set(GMath::Sign(pt[k][G_X] - pt[k1][G_X]), GMath::Sign(pt[k][G_Y] - pt[k1][G_Y]));
		cur = pt[k1] - pt[i];
		// find largest integer j such that xprod(constraint[0], cur+j*dk)
		// >= 0 and xprod(constraint[1], cur+j*dk) <= 0. Use bilinearity of xprod
		a = Cross(constraint[0], cur);
		b = Cross(constraint[0], dk);
		c = Cross(constraint[1], cur);
		d = Cross(constraint[1], dk);
		// find largest integer j such that a+j*b>=0 and c+j*d<=0. This	can be solved with integer arithmetic
		j = G_MAX_INT32;
		if (b < 0)
			j = GMath::FloorDiv(a, -b);
		if (d > 0)
			j = GMath::Min(j, GMath::FloorDiv(-c, d));
		pivk[i] = PotraceMod(k1 + j, n);
	foundk:
		;
	}

	// clean up: for each i, let SubPathsTable[i] be the largest k such that for
	// all i' with i<=i'<k, i'<k<=pivk[i']
	j = pivk[n - 1];
	SubPathsTable[n - 1] = j;
	for (i = n - 2; i >= 0; i--) {
		if (PotraceCyclic(i + 1, pivk[i], j))
			j = pivk[i];
		SubPathsTable[i] = j;
	}

	for (i = n - 1; PotraceCyclic(PotraceMod(i + 1, n), j, SubPathsTable[i]); i--)
		SubPathsTable[i] = j;

	return G_NO_ERROR;
}

GReal GTracer2D::Penalty(const PixelPath& Path, const GDynArray< GPoint<GReal, 5> >& SumsTable,
						GInt32 i, GInt32 j) {

	GInt32 n = Path.Length;
	const GPoint<GInt32, 2> *pt = &(Path.Points[0]);
	const GPoint<GReal, 5> *sums = &(SumsTable[0]);

	GReal x, y, x2, xy, y2;
	GReal k, invk;
	GReal a, b, c, s;
	GReal px, py, ex, ey;
	GInt32 r = 0;

	if (j >= n) {
		j -= n;
		r += 1;
	}
	x  = sums[j + 1][0] - sums[i][0] + r * sums[n][0];
	y  = sums[j + 1][1] - sums[i][1] + r * sums[n][1];
	x2 = sums[j + 1][2] - sums[i][2] + r * sums[n][2];
	xy = sums[j + 1][3] - sums[i][3] + r * sums[n][3];
	y2 = sums[j + 1][4] - sums[i][4] + r * sums[n][4];
	k = (GReal)(j + 1 - i + r * n);
	invk = (GReal)1 / k;
	px = (pt[i][G_X] + pt[j][G_X]) / (GReal)2 - pt[0][G_X];
	py = (pt[i][G_Y] + pt[j][G_Y]) / (GReal)2 - pt[0][G_Y];
	ey = (GReal)(pt[j][G_X] - pt[i][G_X]);
	ex = (GReal)(-(pt[j][G_Y] - pt[i][G_Y]));
	a = ((x2 - 2 * x * px) * invk + px * px);
	b = ((xy - x * py - y * px) * invk + px * py);
	c = ((y2 - 2 * y * py) * invk + py * py);
	s = ex * ex * a + 2 * ex * ey * b + ey * ey * c;
	return GMath::Sqrt(s);
}

GError GTracer2D::BestPolygon(const PixelPath& Path, const GDynArray<GInt32>& lon,
							  const GDynArray< GPoint<GReal, 5> >& SumsTable,
							  GDynArray<GInt32>& Polygon) {

	GInt32 i, j, m, k;     
	GInt32 n = Path.Length;

	GDynArray<GReal> pen(n + 1);
	GDynArray<GInt32> prev(n + 1);
	GDynArray<GInt32> clip0(n);
	GDynArray<GInt32> clip1(n + 1);
	GDynArray<GInt32> seg0(n + 1);
	GDynArray<GInt32> seg1(n + 1);
	GReal thispen, best;
	GInt32 c;

	// calculate clipped paths
	for (i = 0; i < n; i++) {
		c = PotraceMod(lon[PotraceMod(i - 1, n)] - 1, n);
		if (c == i)
			c = PotraceMod(i + 1, n);
		if (c < i)
			clip0[i] = n;
		else
			clip0[i] = c;
	}
	// calculate backwards path clipping, non-cyclic. j <= clip0[i] iff	clip1[j] <= i, for i,j=0..n
	j = 1;
	for (i = 0; i < n; i++) {
		while (j <= clip0[i])
			clip1[j++] = i;
	}
	// calculate seg0[j] = longest path from 0 with j segments
	i = 0;
	for (j = 0; i < n; j++) {
		seg0[j] = i;
		i = clip0[i];
	}
	seg0[j] = n;
	m = j;
	// calculate seg1[j] = longest path to n with m-j segments
	i = n;
	for (j = m; j > 0; j--) {
		seg1[j] = i;
		i = clip1[i];
	}
	seg1[0] = 0;
	// now find the shortest path with m segments, based on penalty3
	pen[0] = 0;
	for (j = 1; j <= m; j++) {
		for (i = seg1[j]; i <= seg0[j]; i++) {
			best = -1;
			for (k = seg0[j - 1]; k >= clip1[i]; k--) {
				thispen = Penalty(Path, SumsTable, k, i) + pen[k];
				if (best < 0 || thispen < best) {
					prev[i] = k;
					best = thispen;
				}
			}
			pen[i] = best;
		}
	}
	Polygon.resize(m);
	// read off shortest path 
	for (i = n, j = m - 1; i > 0; j--) {
		i = prev[i];
		Polygon[j] = i;
	}
	return G_NO_ERROR;
}


/* determine the center and slope of the line i..j. Assume i<j. Needs
   "sum" components of p to be set. */
void GTracer2D::BestFitSlope(const PixelPath& Path, const GDynArray< GPoint<GReal, 5> >& SumsTable,
						   GInt32 i, GInt32 j, GPoint2 *ctr, GVector2 *dir) {

	GInt32 n = Path.Length;
	const GPoint<GReal, 5> *sums = &SumsTable[0];
	GReal x, y, x2, xy, y2, k, invk;
	GInt32 r = 0;

	while (j >= n) {
		j -= n;
		r += 1;
	}
	while (i >= n) {
		i -= n;
		r -= 1;
	}
	while (j < 0) {
		j += n;
		r -= 1;
	}
	while (i < 0) {
		i += n;
		r += 1;
	}
	x  = sums[j + 1][0] - sums[i][0] + r * sums[n][0];
	y  = sums[j + 1][1] - sums[i][1] + r * sums[n][1];
	x2 = sums[j + 1][2] - sums[i][2] + r * sums[n][2];
	xy = sums[j + 1][3] - sums[i][3] + r * sums[n][3];
	y2 = sums[j + 1][4] - sums[i][4] + r * sums[n][4];

	k = (GReal)(j + 1 - i + r * n);
	invk = (GReal)1 / k;
	ctr->Set(x * invk, y * invk);

	GMatrix22 correlMatrix;
	correlMatrix[0][0] = (x2 - x * x * invk) * invk;
	correlMatrix[0][1] = correlMatrix[1][0] = (xy - x * y * invk) * invk;
	correlMatrix[1][1] = (y2 - y * y * invk) * invk;

	GVector2 eigenValue1, eigenValue2, eigenVector1, eigenVector2;
	GEigen solver(correlMatrix, G_TRUE, eigenValue1, eigenValue2, eigenVector1, eigenVector2, G_TRUE);

	// Potrace uses swapped slopes (ex: y-slope instead of x-slope)
	// dir->Set(-eigenVectors[0][G_Y], eigenVectors[0][G_X]);
	dir->Set(-eigenVector1[G_Y], eigenVector1[G_X]);
}

GError GTracer2D::AdjustVertices(const PixelPath& Path, const GDynArray< GPoint<GReal, 5> >& SumsTable,
								 const GDynArray<GInt32>& Polygon,
								 const GInt32 X0, const GInt32 Y0, PrivateCurve& Curve) {

	GInt32 m = (GInt32)Polygon.size();
	const GInt32 *po = &Polygon[0];
	GInt32 n = Path.Length;
	const GPoint<GInt32, 2> *pt = &(Path.Points[0]);
	GInt32 x0 = X0, y0 = Y0, i, j, k, l;
	GDynArray<GPoint2> ctr(m);
	GDynArray<GVector2> dir(m);
	GDynArray<GMatrix33> q(m);
	GPoint3 v;
	GReal d;
	GPoint2 s;

	// initialize curve
	Curve.Tags.resize(m);
	Curve.CurvePoints.resize(m);
	Curve.Vertexes.resize(m);
	  
	// calculate "optimal" point-slope representation for each line	segment
	for (i = 0; i < m; i++) {
		j = po[PotraceMod(i + 1, m)];
		j = PotraceMod(j - po[i], n) + po[i];
		BestFitSlope(Path, SumsTable, po[i], j, &ctr[i], &dir[i]);
	}

	// represent each line segment as a singular quadratic form; the
	//	distance of a point (x,y) from the line segment will be
	//	(x,y,1)Q(x,y,1)^t, where Q=q[i]
	for (i = 0; i < m; i++) {
		d = dir[i].LengthSquared();
		if (d == 0)
			q[i].Set(0, 0, 0, 0, 0, 0, 0, 0, 0);
		else {
			v.Set(dir[i][G_Y], -dir[i][G_X], dir[i][G_X] * ctr[i][G_Y] - dir[i][G_Y] * ctr[i][G_X]);
			for (l = 0; l < 3; l++) {
				for (k = 0; k < 3; k++)
					q[i][l][k] = v[l] * v[k] / d;
			}
		}
	}

	// now calculate the "intersections" of consecutive segments.
	// Instead of using the actual intersection, we find the point
	// within a given unit square which minimizes the square distance to the two lines
	for (i = 0; i < m; i++) {

		GMatrix33 Q;
		GPoint2 w;
		GReal dx, dy;
		GReal det;
		GReal min, cand; /* minimum and candidate for minimum of quad. form */
		GReal xmin, ymin;	/* coordinates of minimum */
		GInt32 z;

		// let s be the vertex, in coordinates relative to x0/y0
		s.Set((GReal)(pt[po[i]][G_X] - x0), (GReal)(pt[po[i]][G_Y] - y0));

		// intersect segments i-1 and i
		j = PotraceMod(i - 1, m);
		// add quadratic forms i and j
		Q = q[j] + q[i];
	    
		while(1) {
			// minimize the quadratic form Q on the unit square find intersection
			det = Q[0][0] * Q[1][1] - Q[0][1] * Q[1][0];
			if (det != 0) {
				w.Set((-Q[0][2] * Q[1][1] + Q[1][2] * Q[0][1]) / det, (Q[0][2] * Q[1][0] - Q[1][2] * Q[0][0]) / det);
				break;
			}
			// matrix is singular - lines are parallel. Add another,
			// orthogonal axis, through the center of the unit square
			if (Q[0][0] > Q[1][1])
				v.Set(-Q[0][1], Q[0][0]);
			else
			if (Q[1][1])
				v.Set(-Q[1][1], Q[1][0]);
			else
				v.Set(1, 0);

			d = GMath::Sqr(v[0]) + GMath::Sqr(v[1]);
			v[2] = - v[1] * s[G_Y] - v[0] * s[G_X];
			for (l = 0; l < 3; l++) {
				for (k = 0; k < 3; k++)
					Q[l][k] += v[l] * v[k] / d;
			}
		}
		dx = GMath::Abs(w[G_X] - s[G_X]);
		dy = GMath::Abs(w[G_Y] - s[G_Y]);
		if (dx <= (GReal)0.5 && dy <= (GReal)0.5) {
			Curve.Vertexes[i].Set(w[G_X] + (GReal)x0, w[G_Y] + (GReal)y0);
			continue;
		}
		// the minimum was not in the unit square; now minimize quadratic on boundary of square
		min = QuadraticForm(Q, s);
		xmin = s[G_X];
		ymin = s[G_Y];

		if (Q[0][0] == 0)
			goto Xfix;
		for (z = 0; z < 2; z++) {   // value of the y-coordinate
			w[G_Y] = s[G_Y] - (GReal)0.5 + z;
			w[G_X] = -(Q[0][1] * w[G_Y] + Q[0][2]) / Q[0][0];
			dx = GMath::Abs(w[G_X] - s[G_X]);
			cand = QuadraticForm(Q, w);
			if (dx <= (GReal)0.5 && cand < min) {
				min = cand;
				xmin = w[G_X];
				ymin = w[G_Y];
			}
		}
	Xfix:
		if (Q[1][1] == 0)
			goto Corners;
		for (z = 0; z < 2; z++) {   // value of the x-coordinate
			w[G_X] = s[G_X] - (GReal)0.5 + z;
			w[G_Y] = -(Q[1][0] * w[G_X] + Q[1][2]) / Q[1][1];
			dy = GMath::Abs(w[G_Y] - s[G_Y]);
			cand = QuadraticForm(Q, w);
			if (dy <= (GReal)0.5 && cand < min) {
				min = cand;
				xmin = w[G_X];
				ymin = w[G_Y];
			}
		}
	Corners:
		// check four corners
		for (l = 0; l < 2; l++) {
			for (k = 0; k < 2; k++) {
				w[G_X] = s[G_X] - (GReal)0.5 + l;
				w[G_Y] = s[G_Y] - (GReal)0.5 + k;
				cand = QuadraticForm(Q, w);
				if (cand < min) {
					min = cand;
					xmin = w[G_X];
					ymin = w[G_Y];
				}
			}
		}
		Curve.Vertexes[i].Set(xmin + (GReal)x0, ymin + (GReal)y0);
		continue;
	}
	return G_NO_ERROR;
}

// ddenom/dpara have the property that the square of radius 1 centered
// at p1 intersects the line p0p2 iff |dpara(p0,p1,p2)| <= ddenom(p0,p2)
static inline GReal ddenom(const GPoint2& p0, const GPoint2& p2) {
	
	GPoint<GInt32, 2> r(-GMath::Sign(p2[G_Y] - p0[G_Y]), GMath::Sign(p2[G_X] - p0[G_X]));
	return (GReal)r[G_Y] * (p2[G_X] - p0[G_X]) - (GReal)r[G_X] * (p2[G_Y] - p0[G_Y]);
}

GError GTracer2D::SmoothCurve(PrivateCurve *curve, GInt32 sign, GReal alphamax, const GReal Scale,
							  const GReal ImageHeight) {

	GInt32 m = (GInt32)curve->Tags.size();
	GInt32 i, j, k;
	GReal dd, denom, alpha;
	GPoint2 p2, p3, p4;
	GPoint2 v1, v2, v3;

	if (sign == -1) {
		// reverse orientation of negative paths
		for (i = 0, j = m - 1; i < j; i++, j--) {
			GPoint2 tmp(curve->Vertexes[i]);
			curve->Vertexes[i] = curve->Vertexes[j];
			curve->Vertexes[j] = tmp;
		}
	}

	// examine each vertex and find its best fit
	for (i = 0; i < m; i++) {
		j = PotraceMod(i + 1, m);
		k = PotraceMod(i + 2, m);

		p4 = GMath::Lerp((GReal)0.5, curve->Vertexes[k], curve->Vertexes[j]);

		denom = ddenom(curve->Vertexes[i], curve->Vertexes[k]);
		if (denom != 0) {
			dd = TwiceSignedArea(curve->Vertexes[i], curve->Vertexes[j], curve->Vertexes[k]) / denom;
			dd = GMath::Abs(dd);
			alpha = dd > 1 ? (1 - (GReal)1 / dd) : 0;
			alpha = alpha / (GReal)0.75;
		}
		else
			alpha = (GReal)4/(GReal)3;

		if (alpha > alphamax) {  // pointed corner
			curve->Tags[j] = CORNER;
			curve->CurvePoints[j].Points[1].Set(curve->Vertexes[j][G_X], ImageHeight - 1 - curve->Vertexes[j][G_Y]);
			curve->CurvePoints[j].Points[1] *= Scale;
			curve->CurvePoints[j].Points[2].Set(Scale * p4[G_X], Scale * (ImageHeight - 1 - p4[G_Y]));
		}
		else {
			alpha = GMath::Clamp(alpha, (GReal)0.55, (GReal)1);
			p2 = GMath::Lerp((GReal)0.5 + (GReal)0.5 * alpha, curve->Vertexes[i], curve->Vertexes[j]);
			p3 = GMath::Lerp((GReal)0.5 + (GReal)0.5 * alpha, curve->Vertexes[k], curve->Vertexes[j]);
			curve->Tags[j] = CURVETO;
			curve->CurvePoints[j].Points[0].Set(p2[G_X] * Scale, (ImageHeight - 1 - p2[G_Y]) * Scale);
			curve->CurvePoints[j].Points[1].Set(p3[G_X] * Scale, (ImageHeight - 1 - p3[G_Y]) * Scale);
			curve->CurvePoints[j].Points[2].Set(p4[G_X] * Scale, (ImageHeight - 1 - p4[G_Y]) * Scale);
		}
	}
	return G_NO_ERROR;
}

GError GTracer2D::VectorizePath(PixelPath& Path, PrivateCurve& Curve, const GInt32 Sign, const GReal AlphaMax,
								const GReal Scale, const GReal ImageHeight) {

	GError err;
	GDynArray< GPoint<GReal, 5> > sumsTable;
	GDynArray<GInt32> subPathsTable;
	GDynArray<GInt32> polygon;
	GInt32 x0, y0;

	err = ShiftPath(Path);
	if (err != G_NO_ERROR)
		return err;

	err = StatisticalSums(Path, sumsTable, x0, y0);
	if (err != G_NO_ERROR)
		return err;

	err = FurthestLinSubPaths(Path, subPathsTable);
	if (err != G_NO_ERROR)
		return err;

	err = BestPolygon(Path, subPathsTable, sumsTable, polygon);
	if (err != G_NO_ERROR)
		return err;

	err = AdjustVertices(Path, sumsTable, polygon, x0, y0, Curve);
	if (err != G_NO_ERROR)
		return err;

	err = SmoothCurve(&Curve, Sign, AlphaMax, Scale, ImageHeight);
	if (err != G_NO_ERROR)
		return err;

	return G_NO_ERROR;
}

GInt32 GTracer2D::SecondSeg(const PixelPath& Path, const GInt32 StartPoint,
							const GInt32 Direction) {

	GPoint<GInt32, 2> pOld, pNew;
	GInt32 acc = 0, i, j, k, k1;

	pOld = Path.Points[StartPoint];
	i = (GInt32)Path.Points.size();
	j = StartPoint + i - 1;
	for (k = StartPoint + 1; k <= j; k++) {
		k1 = k % i;
		pNew = Path.Points[k1];
		if ((Direction == G_X) && (pNew[G_Y] != pOld[G_Y]))
			return acc;
		else
		if ((Direction == G_Y) && (pNew[G_X] != pOld[G_X]))
			return acc;
		else {
			acc++;
			pOld = pNew;
		}
	}
	return acc;
}

GError GTracer2D::ShiftPath(PixelPath& Path) {

	GInt32 i, j, k, ofs, dir, acc;
	GPoint<GInt32, 2> corner, pOld, pNew;
	GDynArray< GPoint<GInt32, 2> > bestCorners;
	
	j = (GInt32)Path.Points.size();
	ofs = G_Y;

	pOld = Path.Points[0];
	dir = G_Y;
	acc = 0;

	for (i = 1; i < j; i++) {
		pNew = Path.Points[i];
		if ((dir == G_Y) && (pNew[G_X] != pOld[G_X])) {
			dir = G_X;
			acc *= SecondSeg(Path, i - 1, dir);
			corner.Set(i - 1, acc);
			bestCorners.push_back(corner);
			acc = 1;
		}
		else
		if ((dir == G_X) && (pNew[G_Y] != pOld[G_Y])) {
			dir = G_Y;
			acc *= SecondSeg(Path, i - 1, dir);
			corner.Set(i - 1, acc);
			bestCorners.push_back(corner);
			acc = 1;
		}
		else
			acc++;
		pOld = pNew;
	}

	acc = 0;
	k = 0;
	j = (GInt32)bestCorners.size();
	for (i = 0; i < j; i++) {
		corner = bestCorners[i];
		if (corner[1] > acc) {
			acc = corner[1];
			k = corner[0];
		}
	}

	if (acc < 2)
		k = 0;

	j = (GInt32)Path.Points.size();
	GDynArray< GPoint<GInt32, 2> > newPoints(j);
	for (i = 0; i < j; i++) {
		ofs = (i + k) % j;
		newPoints[i] = Path.Points[ofs];
	}
	Path.Points = newPoints;

	return G_NO_ERROR;
}

GError GTracer2D::Trace(const GPixelMap& Image, GDynArray<GTracedContour>& Paths,
						const GUChar8 WhiteColor, const GTurnPolicy TurnPolicy,
						const GInt32 MaxRadius, const GInt32 MinArea, const GReal Alpha) {

	// we only wanna single channel images
	if (!Image.IsPaletted() && !Image.IsGrayScale())
		return G_INVALID_FORMAT;
	
	GPixelMap imageCopy;
	GInt32 y, dir;
	GUInt32 pixel;
	GPoint<GInt32, 2> blackPixel;
	PixelPath path;
	GDynArray<PixelPath> pathList;
	GError err;
	GReal globalScale;
	PrivateCurve curve;

	// calculate a scale that make vectorized paths normalized
	globalScale = (GReal)(GMath::Max(GMath::Abs(Image.Width()), GMath::Abs(Image.Height())));
	globalScale = (GReal)1 / globalScale;

	// make a physical copy of input image
	Image.ResizeCanvas(1, 1, 1, 1, imageCopy, WhiteColor);

	// iterate through components
	y = imageCopy.Height() - 1;
	while (FindBlackPixel(imageCopy, WhiteColor, y, blackPixel)) { 
		// calculate the sign by looking at the original
		Image.Pixel(blackPixel[G_X] - 1, blackPixel[G_Y] - 1, pixel);
		y = blackPixel[G_Y];
		if (pixel != WhiteColor)
			// black pixel, go up
			dir = -1;
		else
			// white pixel, go down
			dir = 1;

		// calculate the path
		blackPixel[G_Y] += 1;

		err = BuildPath(imageCopy, blackPixel, WhiteColor, dir, TurnPolicy, MaxRadius, path);
		if (err != G_NO_ERROR)
			return err;

		path.Area = GMath::Abs(path.Area);
		// update buffered image
		XorUpdate(path, imageCopy, WhiteColor);
		// if path was "fat" enough, lets append it
		if (path.Area >= MinArea) {
			//pathList.push_back(path);
			err = VectorizePath(path, curve, dir, Alpha, globalScale, (GReal)Image.Height());
			if (err == G_NO_ERROR) {
				GTracedContour c(curve.Tags, curve.CurvePoints);
				if (err == G_NO_ERROR)
					Paths.push_back(c);
			}
		}
	}
	return G_NO_ERROR;
}

#undef CURVETO
#undef CORNER

};	// end namespace Amanith
