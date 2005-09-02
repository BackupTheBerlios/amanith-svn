/****************************************************************************
** $file: amanith/gproperty.h   0.1.0.0   edited Jun 30 08:00
**
** Basic animated properties definition.
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

#ifndef GPROPERTY_H
#define GPROPERTY_H

#include "amanith/1d/ghermitecurve1d.h"
#include "amanith/1d/gpolylinecurve1d.h"

/*!
	\file gproperty.h
	\brief Header file for basic animated properties.
*/

namespace Amanith {

	// *********************************************************************
	//                          GHermiteProperty1D
	// *********************************************************************

	//! GScalarProperty static class descriptor.
	static const GClassID G_HERMITEPROPERTY1D_CLASSID = GClassID("GHermiteProperty1D", 0xC224EAE5, 0x8AF4406D, 0xA4D1B6A9, 0xEA3871C5);

	class G_EXPORT GHermiteProperty1D : public GProperty {

	private:
		GHermiteCurve1D gInterpolationCurve;

	protected:
		// here is ensured that Index is valid;
		GError DoGetKey(const GUInt32 Index, GKeyValue& OutputKey) const;
		// add a point ON curve; Time is ensured to be inside domain;
		GError DoAddKey(const GTimeValue Time, GUInt32& Index, GBool& AlreadyExists);
		// here is ensured that Index is valid
		GError DoMoveKey(const GUInt32 Index, const GReal NewTimePos, GUInt32& NewIndex, GBool& AlreadyExists);
		// Index is ensured to be valid
		GError DoRemoveKey(const GUInt32 Index);
		// get local value; Time is ensured to be inside life-interval.
		GError DoGetValue(GKeyValue& OutputValue, GTimeInterval& ValidInterval, const GTimeValue Time,
						  const GValueMethod GetMethod) const;
		// set local value; InputValue.TimePosition can be outside range, behavior is to append key
		// and expand domain
		GError DoSetValue(const GKeyValue& InputValue, const GValueMethod SetMethod);
		// get number of keys
		GInt32 DoGetKeysCount() const;
		// build a new keys track; the specified array is ensure to contain at least 1 key
		GError DoSetKeys(const GDynArray<GKeyValue>& Keys);

		//! Cloning function, copies (physically) a Source GScalarProperty.
		GError BaseClone(const GElement& Source);

	public:
		//! Default constructor, set owner to NULL.
		GHermiteProperty1D();
		//! Constructor, specifying owner.
		GHermiteProperty1D(const GElement* Owner);
		//! Destructor
		~GHermiteProperty1D();
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_HERMITEPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_ANIMELEMENT_CLASSID;
		}
		GKeyType HandledType() const {
			return G_REAL_KEY;
		}
	};


	// *********************************************************************
	//                       GHermiteProperty1DProxy
	// *********************************************************************
	/*!
		\class GHermiteProperty1DProxy
		\brief This class implements a GHermiteProperty1D proxy (provider).

		This proxy provides the creation of GHermiteProperty1D class instances.
	*/
	class G_EXPORT GHermiteProperty1DProxy : public GElementProxy {
	public:
		//! Creates a new GHermiteProperty1D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GHermiteProperty1D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_HERMITEPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_ANIMELEMENT_CLASSID;
		}
	};
	//! Static proxy for GHermiteProperty1D class.
	static const GHermiteProperty1DProxy G_HERMITEPROPERTY1D_PROXY;



	// *********************************************************************
	//                          GLinearProperty1D
	// *********************************************************************

	//! GScalarProperty static class descriptor.
	static const GClassID G_LINEARPROPERTY1D_CLASSID = GClassID("GLinearProperty1D", 0xB62BE7B5, 0x85534825, 0x88454EE7, 0x55FA0227);

	class G_EXPORT GLinearProperty1D : public GProperty {

	private:
		GPolyLineCurve1D gInterpolationCurve;

	protected:
		// here is ensured that Index is valid;
		GError DoGetKey(const GUInt32 Index, GKeyValue& OutputKey) const;
		// add a point ON curve; Time is ensured to be inside domain;
		GError DoAddKey(const GTimeValue Time, GUInt32& Index, GBool& AlreadyExists);
		// here is ensured that Index is valid
		GError DoMoveKey(const GUInt32 Index, const GReal NewTimePos, GUInt32& NewIndex, GBool& AlreadyExists);
		// Index is ensured to be valid
		GError DoRemoveKey(const GUInt32 Index);
		// get local value; Time is ensured to be inside life-interval.
		GError DoGetValue(GKeyValue& OutputValue, GTimeInterval& ValidInterval, const GTimeValue Time,
						  const GValueMethod GetMethod) const;
		// set local value; InputValue.TimePosition can be outside range, behavior is to append key
		// and expand domain
		GError DoSetValue(const GKeyValue& InputValue, const GValueMethod SetMethod);
		// get number of keys
		GInt32 DoGetKeysCount() const;
		// build a new keys track; the specified array is ensure to contain at least 1 key
		GError DoSetKeys(const GDynArray<GKeyValue>& Keys);

		//! Cloning function, copies (physically) a Source GScalarProperty.
		GError BaseClone(const GElement& Source);

	public:
		//! Default constructor, set owner to NULL.
		GLinearProperty1D();
		//! Constructor, specifying owner.
		GLinearProperty1D(const GElement* Owner);
		//! Destructor
		~GLinearProperty1D();
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_LINEARPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_ANIMELEMENT_CLASSID;
		}
		GKeyType HandledType() const {
			return G_REAL_KEY;
		}
	};

	// *********************************************************************
	//                      GLinearProperty1DProxy
	// *********************************************************************
	/*!
		\class GLinearProperty1DProxy
		\brief This class implements a GLinearProperty1D proxy (provider).

		This proxy provides the creation of GLinearProperty1D class instances.
	*/
	class G_EXPORT GLinearProperty1DProxy : public GElementProxy {
	public:
		//! Creates a new GLinearProperty1D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GLinearProperty1D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_LINEARPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_ANIMELEMENT_CLASSID;
		}
	};
	//! Static proxy for GLinearProperty1D class.
	static const GLinearProperty1DProxy G_LINEARPROPERTY1D_PROXY;


	// *********************************************************************
	//                         GConstantProperty1D
	// *********************************************************************

	//! GScalarProperty static class descriptor.
	static const GClassID G_CONSTANTPROPERTY1D_CLASSID = GClassID("GConstantProperty1D", 0x911AE34F, 0x0F1E4D62, 0xADA8F342, 0x4E6CC17D);

	class G_EXPORT GConstantProperty1D : public GProperty {

	private:
		GPolyLineCurve1D gInterpolationCurve;

	protected:
		// here is ensured that Index is valid;
		GError DoGetKey(const GUInt32 Index, GKeyValue& OutputKey) const;
		// add a point ON curve; Time is ensured to be inside domain;
		GError DoAddKey(const GTimeValue Time, GUInt32& Index, GBool& AlreadyExists);
		// here is ensured that Index is valid
		GError DoMoveKey(const GUInt32 Index, const GReal NewTimePos, GUInt32& NewIndex, GBool& AlreadyExists);
		// Index is ensured to be valid
		GError DoRemoveKey(const GUInt32 Index);
		// get local value; Time is ensured to be inside life-interval.
		GError DoGetValue(GKeyValue& OutputValue, GTimeInterval& ValidInterval, const GTimeValue Time,
						  const GValueMethod GetMethod) const;
		// set local value; InputValue.TimePosition can be outside range, behavior is to append key
		// and expand domain
		GError DoSetValue(const GKeyValue& InputValue, const GValueMethod SetMethod);
		// get number of keys
		GInt32 DoGetKeysCount() const;
		// build a new keys track; the specified array is ensure to contain at least 1 key
		GError DoSetKeys(const GDynArray<GKeyValue>& Keys);

		//! Cloning function, copies (physically) a Source GScalarProperty.
		GError BaseClone(const GElement& Source);

	public:
		//! Default constructor, set owner to NULL.
		GConstantProperty1D();
		//! Constructor, specifying owner.
		GConstantProperty1D(const GElement* Owner);
		//! Destructor
		~GConstantProperty1D();
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_CONSTANTPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_ANIMELEMENT_CLASSID;
		}
		GKeyType HandledType() const {
			return G_REAL_KEY;
		}
	};

	// *********************************************************************
	//                      GConstantProperty1DProxy
	// *********************************************************************
	/*!
		\class GLinearProperty1DProxy
		\brief This class implements a GLinearProperty1D proxy (provider).

		This proxy provides the creation of GConstantProperty1D class instances.
	*/
	class G_EXPORT GConstantProperty1DProxy : public GElementProxy {
	public:
		//! Creates a new GLinearProperty1D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GConstantProperty1D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_CONSTANTPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_ANIMELEMENT_CLASSID;
		}
	};
	//! Static proxy for GConstantProperty1D class.
	static const GConstantProperty1DProxy G_CONSTANTPROPERTY1D_PROXY;


};	// end namespace Amanith

#endif
