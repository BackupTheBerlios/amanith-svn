/****************************************************************************
** $file: amanith/gproperty.h   0.2.0.0   edited Dec, 12 2005
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

	//! GHermiteProperty1D static class descriptor.
	static const GClassID G_HERMITEPROPERTY1D_CLASSID = GClassID("GHermiteProperty1D", 0xC224EAE5, 0x8AF4406D, 0xA4D1B6A9, 0xEA3871C5);

	/*!
		\class GHermiteProperty1D
		\brief A key-frame based property, that uses an Hermite interpolation schema.

		This animated property handles scalar real type, and interpolates between keys using an Hermite
		interpolation schema.
	*/
	class G_EXPORT GHermiteProperty1D : public GProperty {

	private:
		GHermiteCurve1D gInterpolationCurve;

	protected:
		//! Get a key value by index; here is ensured that Index is valid.
		GError DoGetKey(const GUInt32 Index, GKeyValue& OutputKey) const;
		//! Set a key value by index; here is ensured that Index is valid.
		GError DoSetKey(const GUInt32 Index, const GKeyValue& NewKeyValue);
		/*! 
			Add a point on curve, TimePos is ensured to be inside domain.
			Index will be the index occupied by the created key.
			AlreadyExists will be G_TRUE if at the specified TimePos there was already an existing key.
		*/
		GError DoAddKey(const GTimeValue TimePos, GUInt32& Index, GBool& AlreadyExists);
		/*!
			Move a key to a different time position.

			\param Index the key that we wanna move, it is ensured valid.
			\param NewTimePos the new time position where to move the Index-th key.
			\param NewIndex the new index position associated to the moved key.
			\param AlreadyExists G_TRUE if at specified NewTimePos there was already a key. In this key it
			will be overridden by the moved key.
		*/
		GError DoMoveKey(const GUInt32 Index, const GReal NewTimePos, GUInt32& NewIndex, GBool& AlreadyExists);
		/*!
			Remove a key by index.

			\param Index key index, is ensured to be valid.
		*/
		GError DoRemoveKey(const GUInt32 Index);
		/*!
			Get property value at a specified time position.

			\param OutputValue the returned property value.
			\param ValidInterval the validity interval to update; property validity interval should be intersected
			with this interval.
			\param TimePos the time position where to sample the property value. In this method it is ensured
			to be inside domain interval.
			\param GetMethod the method used to get value.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError DoGetValue(GKeyValue& OutputValue, GTimeInterval& ValidInterval, const GTimeValue TimePos,
						  const GValueMethod GetMethod) const;
		/*!
			Set the property value at a specified time position.

			This method adds a new key at the specified time position, with the passed value.
			\param InputValue the value to set.
			\param TimePos the time position where to set the property value.
			\param SetMethod method used to set the value. For now, the only supported method is
			G_ABSOLUTE_VALUE.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note TimePos can be outside range, behavior is to append (at front/back) created key
			and expand domain.
		*/
		GError DoSetValue(const GKeyValue& InputValue, const GTimeValue TimePos, const GValueMethod SetMethod);
		//! Return the number of keys.
		GInt32 DoGetKeysCount() const;
		/*!
			Set all keys (they don't need to be sorted by time).

			This function rebuilds the property animated track, specifying all the new keys.
			The specified keys can be in any time-order, they will be sorted internally.

			\param Keys the array of keys, it is ensured to contain at least one entry.
			\note <b>Every key must be filled with value and time position</b>.
		*/
		GError DoSetKeys(const GDynArray<GKeyValue>& Keys);
		//! Cloning function, copies (physically) a Source GHermiteProperty1D class.
		GError BaseClone(const GElement& Source);

	public:
		//! Default constructor, set owner to NULL.
		GHermiteProperty1D();
		//! Constructor, specifying owner.
		GHermiteProperty1D(const GElement* Owner);
		//! Destructor
		~GHermiteProperty1D();
		/*!
			Recalculate all keys tangents, using a smoothing schema (Catmull-Rom).

			\param SmoothEnds if G_TRUE, set first and last key tangents to the same value (useful for a looped
			curve).
			\note this method do nothing if there aren't at least 2 keys.
		*/
		inline void RecalcSmoothTangents(const GBool SmoothEnds = G_TRUE) {
			gInterpolationCurve.RecalcSmoothTangents(SmoothEnds);
		}
		//! Get a full Hermite key, specifying index.
		GError HermiteKey(const GUInt32 Index, GHermiteKey1D& OutputKey) const;
		//! Set a full Hermite key, specifying index and all values.
		GError SetHermiteKey(const GUInt32 Index, const GReal NewKeyValue, const GReal InTangent, const GReal OutTangent);
		//! Set all keys (they don't need to be sorted by time), specifying full Hermite values.
		GError SetHermiteKeys(const GDynArray<GHermiteKey1D>& Keys);
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_HERMITEPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_PROPERTY_CLASSID;
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
			return G_PROPERTY_CLASSID;
		}
	};
	//! Static proxy for GHermiteProperty1D class.
	static const GHermiteProperty1DProxy G_HERMITEPROPERTY1D_PROXY;



	// *********************************************************************
	//                          GLinearProperty1D
	// *********************************************************************

	//! GLinearProperty1D static class descriptor.
	static const GClassID G_LINEARPROPERTY1D_CLASSID = GClassID("GLinearProperty1D", 0xB62BE7B5, 0x85534825, 0x88454EE7, 0x55FA0227);

	/*!
		\class GLinearProperty1D
		\brief A key-frame based property, that uses a linear interpolation schema.

		This animated property handles scalar real type, and interpolates between keys using a linear
		interpolation schema.
	*/
	class G_EXPORT GLinearProperty1D : public GProperty {

	private:
		GPolyLineCurve1D gInterpolationCurve;

	protected:
		//! Get a key value by index; here is ensured that Index is valid.
		GError DoGetKey(const GUInt32 Index, GKeyValue& OutputKey) const;
		//! Set a key value by index; here is ensured that Index is valid.
		GError DoSetKey(const GUInt32 Index, const GKeyValue& NewKeyValue);
		/*! 
			Add a point on curve, TimePos is ensured to be inside domain.
			Index will be the index occupied by the created key.
			AlreadyExists will be G_TRUE if at the specified TimePos there was already an existing key.
		*/
		GError DoAddKey(const GTimeValue TimePos, GUInt32& Index, GBool& AlreadyExists);
		/*!
			Move a key to a different time position.

			\param Index the key that we wanna move, it is ensured valid.
			\param NewTimePos the new time position where to move the Index-th key.
			\param NewIndex the new index position associated to the moved key.
			\param AlreadyExists G_TRUE if at specified NewTimePos there was already a key. In this key it
			will be overridden by the moved key.
		*/
		GError DoMoveKey(const GUInt32 Index, const GReal NewTimePos, GUInt32& NewIndex, GBool& AlreadyExists);
		/*!
			Remove a key by index.
			
			\param Index key index, is ensured to be valid.
		*/
		GError DoRemoveKey(const GUInt32 Index);
		/*!
			Get property value at a specified time position.

			\param OutputValue the returned property value.
			\param ValidInterval the validity interval to update; property validity interval should be intersected
			with this interval.
			\param TimePos the time position where to sample the property value. In this method it is ensured
			to be inside domain interval.
			\param GetMethod the method used to get value.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError DoGetValue(GKeyValue& OutputValue, GTimeInterval& ValidInterval, const GTimeValue TimePos,
						  const GValueMethod GetMethod) const;
		/*!
			Set the property value at a specified time position.

			This method adds a new key at the specified time position, with the passed value.
			\param InputValue the value to set.
			\param TimePos the time position where to set the property value.
			\param SetMethod method used to set the value. For now, the only supported method is
			G_ABSOLUTE_VALUE.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note TimePos can be outside range, behavior is to append (at front/back) created key
			and expand domain.
		*/
		GError DoSetValue(const GKeyValue& InputValue, const GTimeValue TimePos, const GValueMethod SetMethod);
		//! Return the number of keys.
		GInt32 DoGetKeysCount() const;
		/*!
			Set all keys (they don't need to be sorted by time).

			This function rebuilds the property animated track, specifying all the new keys.
			The specified keys can be in any time-order, they will be sorted internally.

			\param Keys the array of keys, it is ensured to contain at least one entry.
			\note <b>Every key must be filled with value and time position</b>.
		*/
		GError DoSetKeys(const GDynArray<GKeyValue>& Keys);
		//! Cloning function, copies (physically) a Source GLinearProperty1D class.
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
			return G_PROPERTY_CLASSID;
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
			return G_PROPERTY_CLASSID;
		}
	};
	//! Static proxy for GLinearProperty1D class.
	static const GLinearProperty1DProxy G_LINEARPROPERTY1D_PROXY;


	// *********************************************************************
	//                         GConstantProperty1D
	// *********************************************************************

	//! GConstantProperty1D static class descriptor.
	static const GClassID G_CONSTANTPROPERTY1D_CLASSID = GClassID("GConstantProperty1D", 0x911AE34F, 0x0F1E4D62, 0xADA8F342, 0x4E6CC17D);

	/*!
		\class GConstantProperty1D
		\brief A key-frame based property, that uses a constant interpolation schema.

		This animated property handles scalar real type, and interpolates between keys using a constant
		interpolation schema.
	*/
	class G_EXPORT GConstantProperty1D : public GProperty {

	private:
		GPolyLineCurve1D gInterpolationCurve;

	protected:
		//! Get a key value by index; here is ensured that Index is valid.
		GError DoGetKey(const GUInt32 Index, GKeyValue& OutputKey) const;
		//! Set a key value by index; here is ensured that Index is valid.
		GError DoSetKey(const GUInt32 Index, const GKeyValue& NewKeyValue);
		/*! 
			Add a point on curve, TimePos is ensured to be inside domain.
			Index will be the index occupied by the created key.
			AlreadyExists will be G_TRUE if at the specified TimePos there was already an existing key.
		*/
		GError DoAddKey(const GTimeValue TimePos, GUInt32& Index, GBool& AlreadyExists);
		/*!
			Move a key to a different time position.

			\param Index the key that we wanna move, it is ensured valid.
			\param NewTimePos the new time position where to move the Index-th key.
			\param NewIndex the new index position associated to the moved key.
			\param AlreadyExists G_TRUE if at specified NewTimePos there was already a key. In this key it
			will be overridden by the moved key.
		*/
		GError DoMoveKey(const GUInt32 Index, const GReal NewTimePos, GUInt32& NewIndex, GBool& AlreadyExists);
		/*!
			Remove a key by index.

			\param Index key index, is ensured to be valid.
		*/
		GError DoRemoveKey(const GUInt32 Index);
		/*!
			Get property value at a specified time position.

			\param OutputValue the returned property value.
			\param ValidInterval the validity interval to update; property validity interval should be intersected
			with this interval.
			\param TimePos the time position where to sample the property value. In this method it is ensured
			to be inside domain interval.
			\param GetMethod the method used to get value.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError DoGetValue(GKeyValue& OutputValue, GTimeInterval& ValidInterval, const GTimeValue TimePos,
						  const GValueMethod GetMethod) const;
		/*!
			Set the property value at a specified time position.

			This method adds a new key at the specified time position, with the passed value.
			\param InputValue the value to set.
			\param TimePos the time position where to set the property value.
			\param SetMethod method used to set the value. For now, the only supported method is
			G_ABSOLUTE_VALUE.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note TimePos can be outside range, behavior is to append (at front/back) created key
			and expand domain.
		*/
		GError DoSetValue(const GKeyValue& InputValue, const GTimeValue TimePos, const GValueMethod SetMethod);
		//! Return the number of keys.
		GInt32 DoGetKeysCount() const;
		/*!
			Set all keys (they don't need to be sorted by time).

			This function rebuilds the property animated track, specifying all the new keys.
			The specified keys can be in any time-order, they will be sorted internally.

			\param Keys the array of keys, it is ensured to contain at least one entry.
			\note <b>Every key must be filled with value and time position</b>.
		*/
		GError DoSetKeys(const GDynArray<GKeyValue>& Keys);
		//! Cloning function, copies (physically) a Source GConstantProperty1D class.
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
			return G_PROPERTY_CLASSID;
		}
		GKeyType HandledType() const {
			return G_REAL_KEY;
		}
	};

	// *********************************************************************
	//                      GConstantProperty1DProxy
	// *********************************************************************
	/*!
		\class GConstantProperty1DProxy
		\brief This class implements a GConstantProperty1D proxy (provider).

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
			return G_PROPERTY_CLASSID;
		}
	};
	//! Static proxy for GConstantProperty1D class.
	static const GConstantProperty1DProxy G_CONSTANTPROPERTY1D_PROXY;


};	// end namespace Amanith

#endif
