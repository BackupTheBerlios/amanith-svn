/****************************************************************************
** $file: amanith/numerics/geigen.h   0.1.0.0   edited Jun 30 08:00
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

#ifndef GEIGEN_H
#define GEIGEN_H

#include "amanith/geometry/gmatrix.h"

/*!
	\file geigen.h
	\brief Header file for eigenvalues/eigenvectors solver class.
*/
namespace Amanith {

	/*!
		\class GEigen
		\brief This class implements an eigenvalues/eigenvectors solver for square matrices.

		Symmetric and non-symmetric matrices are supported.
		For real symmetric matrices eigenvalues are all real too, and relative eigenvectors form an orthogonal basis.
	*/
	class G_EXPORT GEigen {

	public:
		/*!
			Constructor for 2x2 matrices.

			Solve eigenvalues/eigenvectors, induced by the specified matrix. Symmetric and non-symmetric matrices are supported.

			\param Matrix the system matrix.
			\param Symmetric if true specifies that matrix is symmetric, false otherwise.
			\param EigenValue1 first eigenvalue. At index 0 there'll be the real part, at index 1 the imaginary part.
			\param EigenValue2 second eigenvalue. At index 0 there'll be the real part, at index 1 the imaginary part.
			\param EigenVector1 the vector corresponding to EigenValue1
			\param EigenVector2 the vector corresponding to EigenValue2
			\param Sort if true eigenvalues (and their corresponding eigenvectors) are returned in descending order, so
			that EigenValue1 is the eigenvalue with the greater module.
		*/
		GEigen(const GMatrix22& Matrix, const GBool Symmetric, GVector2& EigenValue1, GVector2& EigenValue2,
			   GVector2& EigenVector1, GVector2& EigenVector2, const GBool Sort = G_TRUE);
		/*!
			Constructor for 3x3 matrices.

			Solve eigenvalues/eigenvectors, induced by the specified matrix. Symmetric and non-symmetric matrices are supported.

			\param Matrix the system matrix.
			\param Symmetric if true specifies that matrix is symmetric, false otherwise.
			\param EigenValue1 first eigenvalue. At index 0 there'll be the real part, at index 1 the imaginary part.
			\param EigenValue2 second eigenvalue. At index 0 there'll be the real part, at index 1 the imaginary part.
			\param EigenValue3 third eigenvalue. At index 0 there'll be the real part, at index 1 the imaginary part.
			\param EigenVector1 the vector corresponding to EigenValue1
			\param EigenVector2 the vector corresponding to EigenValue2
			\param EigenVector3 the vector corresponding to EigenValue3
			\param Sort if true eigenvalues (and their corresponding eigenvectors) are returned in descending order, so
			that EigenValue1 is the eigenvalue with the greater module.
		*/
		GEigen(const GMatrix33& Matrix, const GBool Symmetric,
			   GVector2& EigenValue1, GVector2& EigenValue2, GVector2& EigenValue3,
			   GVector3& EigenVector1, GVector3& EigenVector2, GVector3& EigenVector3,
			   const GBool Sort = G_TRUE);
		/*!
			Constructor for 4x4 matrices.

			Solve eigenvalues/eigenvectors, induced by the specified matrix. Symmetric and non-symmetric matrices are supported.

			\param Matrix the system matrix.
			\param Symmetric if true specifies that matrix is symmetric, false otherwise.
			\param EigenValue1 first eigenvalue. At index 0 there'll be the real part, at index 1 the imaginary part.
			\param EigenValue2 second eigenvalue. At index 0 there'll be the real part, at index 1 the imaginary part.
			\param EigenValue3 third eigenvalue. At index 0 there'll be the real part, at index 1 the imaginary part.
			\param EigenValue4 fourth eigenvalue. At index 0 there'll be the real part, at index 1 the imaginary part.
			\param EigenVector1 the vector corresponding to EigenValue1
			\param EigenVector2 the vector corresponding to EigenValue2
			\param EigenVector3 the vector corresponding to EigenValue3
			\param EigenVector4 the vector corresponding to EigenValue3
			\param Sort if true eigenvalues (and their corresponding eigenvectors) are returned in descending order, so
			that EigenValue1 is the eigenvalue with the greater module.
		*/
		GEigen(const GMatrix44& Matrix, const GBool Symmetric,
			   GVector2& EigenValue1, GVector2& EigenValue2, GVector2& EigenValue3, GVector2& EigenValue4,
			   GVector4& EigenVector1, GVector4& EigenVector2, GVector4& EigenVector3, GVector4& EigenVector4,
			   const GBool Sort = G_TRUE);
	};
};

#endif
