/****************************************************************************
** $file: amanith/src/numerics/geigen.cpp   0.1.1.0   edited Sep 24 08:00
**
** Eigen values and vectors calculation.
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

#include "amanith/numerics/geigen.h"
#include "amanith/gmath.h"

/*!
	\file geigen.cpp
	\brief Implementation of eigenvalues/eigenvectors solver class.
*/

namespace Amanith {

// constructor
GEigen::GEigen(const GMatrix22& Matrix, const GBool Symmetric,
			   GVector2& EigenValue1, GVector2& EigenValue2, GVector2& EigenVector1, GVector2& EigenVector2,
			   const GBool Sort) {

	GMatrix22 A = Matrix;
	GVector2 d, e, eigenVector;

	if (Symmetric) {
		SymHouseholderTridReduction(A, d, e);
		SymTridQL(A, d, e, Sort);
	}
	else {
		GMatrix22 H = A;
		NonSymHessenbergReduction(A, H);
		NonSymSchurReduction(A, H, d, e);
	}

	EigenValue1.Set(d[0], e[0]);
	EigenValue2.Set(d[1], e[1]);

	EigenVector1.Set(A[G_X][0], A[G_Y][0]);
	EigenVector2.Set(A[G_X][1], A[G_Y][1]);
}

// constructor
GEigen::GEigen(const GMatrix33& Matrix, const GBool Symmetric,
			   GVector2& EigenValue1, GVector2& EigenValue2, GVector2& EigenValue3,
			   GVector3& EigenVector1, GVector3& EigenVector2, GVector3& EigenVector3, const GBool Sort) {

	GMatrix33 A = Matrix;
	GVector3 d, e, eigenVector;

	if (Symmetric) {
		SymHouseholderTridReduction(A, d, e);
		SymTridQL(A, d, e, Sort);
	}
	else {
		GMatrix33 H = A;
		NonSymHessenbergReduction(A, H);
		NonSymSchurReduction(A, H, d, e);
	}

	EigenValue1.Set(d[0], e[0]);
	EigenValue2.Set(d[1], e[1]);
	EigenValue3.Set(d[2], e[2]);

	EigenVector1.Set(A[G_X][0], A[G_Y][0], A[G_Z][0]);
	EigenVector2.Set(A[G_X][1], A[G_Y][1], A[G_Z][1]);
	EigenVector3.Set(A[G_X][2], A[G_Y][2], A[G_Z][2]);
}

// constructor
GEigen::GEigen(const GMatrix44& Matrix, const GBool Symmetric,
			   GVector2& EigenValue1, GVector2& EigenValue2, GVector2& EigenValue3, GVector2& EigenValue4,
			   GVector4& EigenVector1, GVector4& EigenVector2, GVector4& EigenVector3, GVector4& EigenVector4,
			   const GBool Sort) {

	GMatrix44 A = Matrix;
	GVector4 d, e, eigenVector;

	if (Symmetric) {
		SymHouseholderTridReduction(A, d, e);
		SymTridQL(A, d, e, Sort);
	}
	else {
		GMatrix44 H = A;
		NonSymHessenbergReduction(A, H);
		NonSymSchurReduction(A, H, d, e);
	}

	EigenValue1.Set(d[0], e[0]);
	EigenValue2.Set(d[1], e[1]);
	EigenValue3.Set(d[2], e[2]);
	EigenValue4.Set(d[3], e[3]);

	EigenVector1.Set(A[G_X][0], A[G_Y][0], A[G_Z][0], A[G_W][0]);
	EigenVector2.Set(A[G_X][1], A[G_Y][1], A[G_Z][1], A[G_W][1]);
	EigenVector3.Set(A[G_X][2], A[G_Y][2], A[G_Z][2], A[G_W][2]);
	EigenVector4.Set(A[G_X][3], A[G_Y][3], A[G_Z][3], A[G_W][3]);
}

}
