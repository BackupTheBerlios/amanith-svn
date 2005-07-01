/****************************************************************************
** $file: amanith/numerics/gintegration.h   0.1.0.0   edited Jun 30 08:00
**
** Integration of scalar functions.
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

#ifndef GINTEGRATION_H
#define GINTEGRATION_H

#include "amanith/gglobal.h"
#include "amanith/gmath.h"

/*!
	\file gintegration.h
	\brief Header file for numeric integration class.
*/
namespace Amanith {

	// *********************************************************************
	//                             GIntegration
	// *********************************************************************
	/*!
		\class GIntegration
		\brief This class implements numerical integration of scalar functions.
	*/
	class G_EXPORT GIntegration {

	public:
		//! Type definition of a scalar function callback
		typedef GReal (*GFunction)(const GReal, void*);
		//! Romberg integration for continuous scalar functions.
		static GBool Romberg(GReal& Result,	const GReal u0, const GReal u1, GFunction Function,
							 void *UserData, const GReal MaxError = G_EPSILON);
	};
};

#endif
