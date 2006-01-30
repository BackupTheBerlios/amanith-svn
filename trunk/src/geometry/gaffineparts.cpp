/****************************************************************************
** $file: amanith/src/geometry/gaffineparts.h   0.3.0.0   edited Jan, 30 2006
**
** Affine parts decomposition.
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

#include "amanith/geometry/gaffineparts.h"
#include "amanith/geometry/gxformconv.h"

/*!
	\file gaffineparts.cpp
	\brief Affine parts decomposition implementation file.
*/

namespace Amanith {

void GAffineParts::DecompAffine(const GMatrix33& AffineMatrix) {

	GMatrix33 Q, S, U;
    GQuaternion p;
    GReal detQ;
	GVector2 eigenValue1, eigenValue2, eigenValue3;
	GVector3 eigenVector1, eigenVector2, eigenVector3;

	GBool b;

	// first polar decompose given affine matrix
    b = DecompPolar(AffineMatrix, Q, detQ, S);
    if (detQ < 0) {
		Q = -Q;
		gDetSign = -1;
	}
	else
		gDetSign = 1;
	// extract essential rotation
    MatrixToRotation(gMainRot, Q);
	// spectral decomposition
	GEigen eigenSolver(S, G_TRUE, eigenValue1, eigenValue2, eigenValue3,
					   eigenVector1, eigenVector2, eigenVector3, G_FALSE);

	// extract stretch factors (eigenvalues are real numbers because S is symmetric with real values)
	gStretchFactors.Set(eigenValue1[0], eigenValue2[0], eigenValue3[0]);

	U[0][0] = eigenVector1[0];
	U[0][1] = eigenVector1[1];
	U[0][2] = eigenVector1[2];
	U[1][0] = eigenVector2[0];
	U[1][1] = eigenVector2[1];
	U[1][2] = eigenVector2[2];
	U[2][0] = eigenVector3[0];
	U[2][1] = eigenVector3[1];
	U[2][2] = eigenVector3[2];

	// extract stretch rotation
    MatrixToRotation(gStretchRot, U);
    p = Snuggle(gStretchRot, gStretchFactors);
    gStretchRot *= p;
}

// constructor
GAffineParts::GAffineParts(const GMatrix33& AffineMatrix) {

	// translation
    gTrans.Set(0, 0, 0);
	DecompAffine(AffineMatrix);
}

// constructor
GAffineParts::GAffineParts(const GMatrix34& AffineMatrix) {

	GMatrix33 affineTmp;

	// translation
    gTrans.Set(AffineMatrix[G_X][G_W], AffineMatrix[G_Y][G_W], AffineMatrix[G_Z][G_W]);
	// extract affine portion
	for (GInt32 i = 0; i < 3; i++)
		for (GInt32 j = 0; j < 3; j++)
			affineTmp[i][j] = AffineMatrix[i][j];
	DecompAffine(affineTmp);
}


// constructor
GAffineParts::GAffineParts(const GMatrix44& AffineMatrix) {

	GMatrix33 affineTmp;

	// translation
    gTrans.Set(AffineMatrix[G_X][G_W], AffineMatrix[G_Y][G_W], AffineMatrix[G_Z][G_W]);
	// extract affine portion
	for (GInt32 i = 0; i < 3; i++)
		for (GInt32 j = 0; j < 3; j++)
			affineTmp[i][j] = AffineMatrix[i][j];
	DecompAffine(affineTmp);
}

/* Given a unit quaternion, q, and a scale vector, k, find a unit quaternion, p,
* which permutes the axes and turns freely in the plane of duplicate scale
* factors, such that q p has the largest possible w component, i.e. the
* smallest possible angle. Permutes k's components to go with q p instead of q.
* See Ken Shoemake and Tom Duff. Matrix Animation and Polar Decomposition.
* Proceedings of Graphics Interface 1992. Details on p. 262-263.
*
* Taken from Graphics Gems IV
*/
GQuaternion GAffineParts::Snuggle(GQuaternion& q, GVector3& k) {

	#define sgn(n, v)   ((n)?-(v):(v))
	#define swap(a, i, j) { a[3]=a[i]; a[i]=a[j]; a[j]=a[3]; }
	#define cycle(a, p)  if (p) {a[3]=a[0]; a[0]=a[1]; a[1]=a[2]; a[2]=a[3];}\
						 else {a[3]=a[2]; a[2]=a[1]; a[1]=a[0]; a[0]=a[3];}
    GQuaternion p;
    GReal ka[4];
    GInt32 i, turn = -1;
    
	ka[G_X] = k[G_X];
	ka[G_Y] = k[G_Y];
	ka[G_Z] = k[G_Z];
    if (ka[G_X] == ka[G_Y]) {
		if (ka[G_X] == ka[G_Z])
			turn = G_W;
		else
			turn = G_Z;
	}
    else {
		if (ka[G_X] == ka[G_Z])
			turn = G_Y;
		else
		if (ka[G_Y] == ka[G_Z])
			turn = G_X;
	}

    if (turn >= 0) {
		GQuaternion qtoz, qp;
		GUInt32 neg[3], win;
		GReal mag[3], t;
		static GQuaternion qxtoz(0, (GReal)G_SQRTHALF, 0, (GReal)G_SQRTHALF);
		static GQuaternion qytoz((GReal)G_SQRTHALF, 0, 0, (GReal)G_SQRTHALF);
		static GQuaternion qppmm((GReal)0.5, (GReal)0.5, -(GReal)0.5, -(GReal)0.5);
		static GQuaternion qpppp((GReal)0.5, (GReal)0.5, (GReal)0.5, (GReal)0.5);
		static GQuaternion qmpmm(-(GReal)0.5, (GReal)0.5, -(GReal)0.5, -(GReal)0.5);
		static GQuaternion qpppm((GReal)0.5, (GReal)0.5, (GReal)0.5, -(GReal)0.5);
		static GQuaternion q0001(0, 0, 0, 1);
		static GQuaternion q1000(1, 0, 0, 0);
		switch (turn) {
			case G_X:
				qtoz = qxtoz;
				q *= qtoz;
				swap(ka, G_X, G_Z)
				break;

			case G_Y:
				qtoz = qytoz;
				q *= qtoz;
				swap(ka, G_Y, G_Z)
				break;

			case G_Z:
				qtoz = q0001;
				break;

			default:
				return (q.Conj());
		}
		q = q.Conj();
		mag[0] = q[G_Z] * q[G_Z] + q[G_W] * q[G_W] - (GReal)0.5;
		mag[1] = q[G_X] * q[G_Z] - q[G_Y] * q[G_W];
		mag[2] = q[G_Y] * q[G_Z] + q[G_X] * q[G_W];
		for (i = 0; i < 3; i++)
			if ((neg[i] = (mag[i] < 0)))
				mag[i] = -mag[i];

		if (mag[0] > mag[1]) {
			if (mag[0] > mag[2])
				win = 0;
			else
				win = 2;
		}
		else {
			if (mag[1] > mag[2])
				win = 1;
			else
				win = 2;
		}
		switch (win) {
			case 0:
				if (neg[0])
					p = q1000;
				else
					p = q0001;
				break;
			
			case 1:
				if (neg[1])
					p = qppmm;
				else
					p = qpppp;
				cycle(ka, 0)
				break;
			case 2:
				if (neg[2])
					p = qmpmm;
				else
					p = qpppm;
				cycle(ka, 1)
				break;
		}
		qp = q * p;
		t = GMath::Sqrt(mag[win] + (GReal)0.5);
		p *= GQuaternion(0, 0, -qp[G_Z] / t, qp[G_W] / t);
		p = qtoz * p.Conj();
	}
	else {
		GReal qa[4], pa[4];
		GUInt32 lo, hi, neg[4], par = 0;
		GReal all, big, two;
		qa[0] = q[G_X];
		qa[1] = q[G_Y];
		qa[2] = q[G_Z];
		qa[3] = q[G_W];
		for (i = 0; i < 4; i++) {
		    pa[i] = 0;
			if ((neg[i] = (qa[i] < 0)))
				qa[i] = -qa[i];
			par ^= neg[i];
		}
		// find two largest components, indexes in hi and lo
		if (qa[0] > qa[1])
			lo = 0;
		else
			lo = 1;
		if (qa[2] > qa[3])
			hi = 2;
		else
			hi = 3;
		if (qa[lo] > qa[hi]) {
			if (qa[lo^1] > qa[hi]) {
				hi = lo;
				lo ^= 1;
			}
			else {
				hi ^= lo;
				lo ^= hi;
				hi ^= lo;
			}
		}
		else {
			if (qa[hi^1] > qa[lo])
				lo = hi^1;
		}
		all = (qa[0] + qa[1] + qa[2] + qa[3]) * (GReal)0.5;
		two = (qa[hi] + qa[lo]) * (GReal)G_SQRTHALF;
		big = qa[hi];
		if (all > two) {
			if (all > big) {
				// all
				for (i = 0; i < 4; i++)
					pa[i] = sgn(neg[i], (GReal)0.5);
				cycle(ka, par)
			}
			else {
				// big
				pa[hi] = sgn(neg[hi], (GReal)1);
			}
		}
		else {
			if (two > big) {
				// two
				pa[hi] = sgn(neg[hi], (GReal)G_SQRTHALF);
				pa[lo] = sgn(neg[lo], (GReal)G_SQRTHALF);
				if (lo > hi) {
					hi ^= lo;
					lo ^= hi;
					hi ^= lo;
				}
				if (hi == G_W) {
					hi = "\001\002\000"[lo];
					lo = 3 - hi - lo;
				}
				swap(ka, hi, lo)
			}
			else {
				// big
				pa[hi] = sgn(neg[hi], (GReal)1);
			}
		}
		p[G_X] = -pa[0];
		p[G_Y] = -pa[1];
		p[G_Z] = -pa[2];
		p[G_W] = pa[3];
    }
    k[G_X] = ka[G_X];
	k[G_Y] = ka[G_Y];
	k[G_Z] = ka[G_Z];
    return p;
	#undef sgn
	#undef swap
	#undef cycle
}

};	// end namespace Amanith
