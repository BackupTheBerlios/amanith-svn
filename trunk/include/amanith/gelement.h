/****************************************************************************
** $file: amanith/gelement.h   0.2.0.0   edited Dec, 12 2005
**
** Base elements definition.
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

#ifndef GELEMENT_H
#define GELEMENT_H

#include "amanith/gclassid.h"
#include "amanith/geometry/ginterval.h"
#include "amanith/geometry/gvect.h"
#include "amanith/geometry/gmatrix.h"
#include "amanith/gmath.h"
#include "amanith/support/gutilities.h"

/*!
	\file gelement.h
	\brief GElement header file.
*/

namespace Amanith {

	// *********************************************************************
	//                            GTimeInterval
	// *********************************************************************

	//! Time interval, it's an interval where values are of type GTimeValue
	typedef GInterval<GReal> GTimeInterval;
	//! Time interval with the maximum possible length
	static const GTimeInterval G_FOREVER_TIMEINTERVAL = GTimeInterval(G_MIN_REAL, G_MAX_REAL);
	//! An empty interval
	static const GTimeInterval G_NEVER_TIMEINTERVAL = GTimeInterval(G_MIN_REAL, G_MIN_REAL);


	// *********************************************************************
	//                               GElement
	// *********************************************************************

	//! GElement static class descriptor.
	static const GClassID G_ELEMENT_CLASSID = GClassID("GElement", 0x99699BEC, 0xD171411E, 0xAB48F5DA, 0xD9083C2A);

	class G_EXPORT GElement {

	private:
		//! Kernel, owner of this element
		const GElement* gOwner;

	protected:
		//! Creates a new element given a class id.\n If element can't be created a NULL pointer is returned.
		GElement* CreateNew(const GClassID& Class_ID) const;
		//!	Get class identifier from a class name.
		GError ClassIDFromClassName(const GString& Class_Name, GClassID& OutID) const;
		// Creates a new element given a class name.
		GElement* CreateNew(const GString& Class_Name);
		/*!
			Base clone function. Every derived class must implement this function.\n Source parameter is ensured to be
			of the same class, so casting made inside this function is type-safe. The typical implementation uses a chain
			propagation (towards father classes), relegating this function to the simple copy of specific internal state.\n
			Here's and example taken from GHermiteCurve2D class (this class derives from GMultiCurve2D):
\code
	GError GHermiteCurve2D::BaseClone(const GElement& Source) {

		const GHermiteCurve2D& src = (const GHermiteCurve2D&)Source;

		// copy keys array
		gKeys = src.gKeys;
		// continue cloning passing the control to base (father) class
		return GMultiCurve2D::BaseClone(Source);
}
\endcode
		*/
		virtual GError BaseClone(const GElement& Source) {
			// just to avoid warnings
			if (Source.Owner())
				return G_NO_ERROR;
			return G_NO_ERROR;
		}

	public:
		//! Default constructor, set owner to NULL.
		GElement();
		//! Constructor, specifying owner.
		GElement(const GElement* Owner);
		//! Destructor
		virtual ~GElement();
		//! Is an element of a specified type?\n It does include inheritance inspection (using owner / kernel)
		GBool IsOfType(const GClassID& Class_ID) const;
		/*!
			Type-safe cast.\n This function cast this element to the type specified by Class_ID parameter.
			If cast can be done (it means that this element is of Class_ID type or is derived by Class_ID type), a
			non-null element pointer (equal to 'this') is returned. If cast cannot be done a NULL pointer is returned.
		*/
		GElement* As(const GClassID& Class_ID) {
			if (IsOfType(Class_ID))
				return this;
			return NULL;
		}
		/*!
			Type-safe cast (const version).\n This function cast this element to the type specified by Class_ID parameter.
			If cast can be done (it means that this element is of Class_ID type or is derived by Class_ID type), a
			non-null element pointer (equal to 'this') is returned. If cast cannot be done a NULL pointer is returned.
		*/
		const GElement* As(const GClassID& Class_ID) const {
			if (IsOfType(Class_ID))
				return this;
			return NULL;
		}
		//! Is 'InspectedType' derived-from/of 'Class_ID' type?
		virtual GBool IsTypeOfType(const GClassID& InspectedType, const GClassID& Class_ID) const;
		//! Physical copy, using a source element.
		GError CopyFrom(const GElement& Source);
		/*!
			Physical copy into 'Destination' element.\n
			If copy can't be done an error code is returned, else G_NO_ERROR.

			\note This function is provided for convenience, it's a simple Destination.CopyFrom(*this).
		*/
		inline GError CopyTo(GElement& Destination) const {
			return Destination.CopyFrom(*this);
		}
		/*!
			Assignment operator.
			\note It uses CopyFrom() function.
		*/
		inline GElement& operator =(const GElement& Source) {

			GError err = CopyFrom(Source);
			if (err != G_NO_ERROR) {
				G_DEBUG("GElement assignment operator, CopyFrom has returned this error code: " + StrUtils::ToString(err));
			}
			return *this;
		}
		//! Get element owner (kernel).
		inline const GElement* Owner() const {
			return gOwner;
		}
		//! Get class descriptor
		virtual const GClassID& ClassID() const {
			return G_ELEMENT_CLASSID;
		}
		//! Get base class (father class) descriptor
		virtual const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
		//! Is a root class?\n Root class is a class where derived class descriptor is the same of the inspected class.
		GBool IsRootClass() const {
			if (ClassID() == DerivedClassID())
				return G_TRUE;
			return G_FALSE;
		}
	};


	// *********************************************************************
	//                            GElementProxy
	// *********************************************************************

	/*! \class GElementProxy
		\brief This class implements a GElement proxy (provider).

		Through a GElementProxy it's possible to create a GElement class (or an its derived class), specifying eventually
		an owner.\n GElementProxy is at the base of the plugin system, based on a central manager that is implemented by
		GKernel class.\n
		Every derived class (proxies), must implement CreateNew() method to make class creation possible. This is generally
		true, except in cases where class has got some virtual (and non implemented methods). Even if class has got
		all methods implemented, proxies could introduce an intermediate "virtual / non instantiable" layer that
		could be used in some useful situations.
	*/
	class G_EXPORT GElementProxy {
	public:
		//! Create a new element. Every derived class must implements this method.
		virtual GElement* CreateNew(const GElement* Owner = NULL) const {
			if (!Owner)
				return NULL;
			return NULL;
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		virtual const GClassID& ClassID() const {
			return G_ELEMENT_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		virtual const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
		/*!
			Does this proxy manages a root class?\n Root class is a class where
			derived class descriptor is the same of the inspected class.
		*/
		GBool IsRootClass() const {
			if (ClassID() == DerivedClassID())
				return G_TRUE;
			return G_FALSE;
		}
		//! Version of this proxy.
		virtual GInt32 Version() const {
			return G_AMANITH_VERSION;
		}
		//! Destructor, this empty virtual declaration make gcc 4.0 happy with warnings.
		virtual ~GElementProxy() {
		}
	};

	//! Static proxy for GElement class.
	static const GElementProxy G_ELEMENT_PROXY;


	// *********************************************************************
	//                              GKeyValue
	// *********************************************************************

	//! Key value types
	enum GKeyType {
		//! Undefined value.
		G_UNDEFINED_KEY,
		//! Boolean value.
		G_BOOL_KEY,
		//! Integer value.
		G_INT_KEY,
		//! Real value.
		G_REAL_KEY,
		//! 2D vector/point value.
		G_VECTOR2_KEY,
		//! 3D vector/point value.
		G_VECTOR3_KEY,
		//! 4D vector/point value.
		G_VECTOR4_KEY
	};

	/*!
		\class GKeyValue
		\brief Generic key value.

		This class embed a generic key value. It's a sort of 'variant' type, that supports common values
		used in a generic keyframer. This class supports also an associated time position value, that can be
		used by some methods.
	*/
	class G_EXPORT GKeyValue {

		friend class GProperty;

	private:
		GKeyType gType;
		GTimeValue gTimePos;
		GVector4 gValue;

	protected:
		//! Set the current key value as undefined.
		void SetUndefined() {
			gType = G_UNDEFINED_KEY;
			gValue.Set(G_MIN_REAL, G_MIN_REAL, G_MIN_REAL, G_MIN_REAL);
			gTimePos = G_MIN_REAL;
		}

	public:
		//! Default constructor, set the key value as undefined.
		GKeyValue();
		//! Set constructor, set a new boolean key value.
		GKeyValue(const GBool Value);
		//! Set constructor, set a new integer key value.
		GKeyValue(const GInt32 Value);
		//! Set constructor, set a new real key value.
		GKeyValue(const GReal Value);
		//! Set constructor, set a new 2D vector/point key value.
		GKeyValue(const GVector2& Value);
		//! Set constructor, set a new 3D vector/point key value.
		GKeyValue(const GVector3& Value);
		//! Set constructor, set a new 4D vector/point key value.
		GKeyValue(const GVector4& Value);
		//! Set constructor, set a new boolean key value and a time-position.
		GKeyValue(const GTimeValue TimePos, const GBool Value);
		//! Set constructor, set a new integer key value and a time-position.
		GKeyValue(const GTimeValue TimePos, const GInt32 Value);
		//! Set constructor, set a new real key value and a time-position.
		GKeyValue(const GTimeValue TimePos, const GReal Value);
		//! Set constructor, set a new 2D vector/point key value and a time-position.
		GKeyValue(const GTimeValue TimePos, const GVector2& Value);
		//! Set constructor, set a new 3D vector/point key value and a time-position.
		GKeyValue(const GTimeValue TimePos, const GVector3& Value);
		//! Set constructor, set a new 4D vector/point key value and a time-position.
		GKeyValue(const GTimeValue TimePos, const GVector4& Value);
		//! Set a new boolean value.
		void SetValue(const GBool Value);
		//! Set a new integer value.
		void SetValue(const GInt32 Value);
		//! Set a new real value.
		void SetValue(const GReal Value);
		//! Set a new 2D vector/point value.
		void SetValue(const GVector2& Value);
		//! Set a new 3D vector/point value.
		void SetValue(const GVector3& Value);
		//! Set a new 4D vector/point value.
		void SetValue(const GVector4& Value);
		//! Get key type.
		inline GKeyType KeyType() const {
			return gType;
		}
		//! Set key type.
		void SetKeyType(const GKeyType NewType) {
			gType = NewType;
		}
		//! Get key position (timeline position)
		inline GTimeValue TimePosition() const {
			return gTimePos;
		}
		//! Set key position (timeline position)
		void SetTimePosition(const GTimeValue NewTimePos) {
			gTimePos = NewTimePos;
		}
		//! Get the current value as a boolean value.
		GBool BoolValue() const;
		//! Get the current value as an integer value.
		GInt32 IntValue() const;
		//! Get the current value as a real value.
		GReal RealValue() const;
		//! Get the current value as a 2D vector/point value.
		GVector2 Vect2Value() const;
		//! Get the current value as a 3D vector/point value.
		GVector3 Vect3Value() const;
		//! Get the current value as a 4D vector/point value.
		GVector4 Vect4Value() const;
	};


	// *********************************************************************
	//                             GAnimElement
	// *********************************************************************
	//! GAnimElement static class descriptor.
	static const GClassID G_ANIMELEMENT_CLASSID = GClassID("GAnimElement", 0x8B98ACC9, 0x577E4416, 0xB9230A4A, 0xE4762EEE);

	// forward declaration
	class GProperty;

	/*!
		\class GAnimElement
		\brief This class implements an 'animated' element.

		This class represents a container of animated properties. Properties can be added and removed, as
		well as renamed. Each property has a name, that is granted unique inside each animated element.
		Through an GAnimElement, contained properties can be extracted using their index or their unique name.
		A property is itself an animated element so properties can be grouped for a better logical separation and
		management.
	*/
	class G_EXPORT GAnimElement : public GElement {
	private:
		// internal list of properties
		GDynArray<GProperty*> gProperties;

	protected:
		// delete and empty all properties
		void DeleteProperties(void);
		// clone properties
		GError CloneProperties(const GAnimElement& Source);
		// find a property given its name
		GProperty* FindProperty(const GString& Name, GUInt32& PropIndex) const;
		// base clone function
		GError BaseClone(const GElement& Source);

	public:
		//! Default constructor, set owner to NULL. Internal properties array is initialized as empty.
		GAnimElement() : GElement() {
		}
		//! Constructor, specifying owner. Internal properties array is initialized as empty.
		GAnimElement(const GElement* Owner) : GElement(Owner) {
		}
		//! Destructor, remove and free all owned properties.
		virtual ~GAnimElement() {
			// delete all properties
			DeleteProperties();
		}
		/*!
			Add a property to this animated element.

			\param Name the property name, it must be non-empty.
			\param ClassID a class descriptor identifying the property to be add. This class id must be
			relative to a GProperty derived class.
			\param DefaultValue the default value associated with the property. If an invalid value is
			specified (it is not compatible with the value type handled by the ClassID-described property), a
			0(s) default value will be used.
			\param AlreadyExist if a property with the same specified Name already exists, this returned flag
			will be	G_TRUE, G_FALSE otherwise.
			\param PropertyIndex this returned value will contain the added/existing property index.
			\return the property pointer, NULL if the operation has failed (memory errors or something worst).
			\note <b>Name comparisons are done case-insensitive</b>.
		*/
		GProperty* AddProperty(const GString& Name, const GClassID& ClassID, const GKeyValue& DefaultValue,
							   GBool& AlreadyExist, GUInt32& PropertyIndex);
		/*!
			Get an existing property, given its name.

			\return the property pointer if found, NULL otherwise.
			\note <b>Name comparisons are done case-insensitive</b>.
		*/
		GProperty* Property(const GString& Name) const;
		/*!
			Get an existing property, given its index.

			\return the property pointer if found; NULL if specified Index is not valid.
		*/
		GProperty* Property(const GUInt32 Index) const;
		/*!
			Remove the specified property from internal list.

			\param Name the name of the property to be deleted and removed.
			\return G_TRUE if the property did exist, G_FALSE otherwise.
			\note <b>Name comparisons are done case-insensitive</b>.
		*/
		GBool RemoveProperty(const GString& Name);
		/*!
			Remove the specified property from internal list.

			\param Index index of the property to be deleted and removed.
			\return G_TRUE if the property did exist (the specified Index was valid), G_FALSE otherwise.
		*/
		GBool RemoveProperty(const GUInt32 Index);
		//! Remove and free all properties.
		inline void RemoveProperties() {
			// delete all properties
			DeleteProperties();
		}
		/*!
			Rename a property.

			\param CurrentName the name specifying the property to be renamed.
			\param NewName the new name to associate to the property.
			\return G_NO_ERROR if the operation succeeds, G_ENTRY_ALREADY_EXISTS if a property with the specified
			NewName already exists. G_INVALID_PARAMETER if a property with the specified CurrentName doesn't
			exists.
		*/
		GError RenameProperty(const GString& CurrentName, const GString& NewName);
		//! Get full list of properties.
		const GDynArray<GProperty*>& Properties() const {
			return gProperties;
		}
		//! Get number of properties.
		inline const GUInt32 PropertiesCount() const {
			return (GUInt32)gProperties.size();
		}
		//! Get class descriptor
		inline const GClassID& ClassID() const {
			return G_ANIMELEMENT_CLASSID;
		}
		//! Get base class (father class) descriptor
		inline const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
	};

	// *********************************************************************
	//                          GAnimElementProxy
	// *********************************************************************
	/*!
		\class GAnimElementProxy
		\brief This class implements a GAnimElement proxy (provider).

		This proxy does not override CreateNew() method because we don't wanna make a creation of a GAnimElement
		class possible (because of pure virtual  methods).
	*/
	class G_EXPORT GAnimElementProxy : public GElementProxy {
	public:
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_ANIMELEMENT_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
	};
	//! Static proxy for GAnimElement class.
	static const GAnimElementProxy G_ANIMELEMENT_PROXY;



	// *********************************************************************
	//                             GProperty
	// *********************************************************************
	//! GProperty static class descriptor.
	static const GClassID G_PROPERTY_CLASSID = GClassID("GProperty", 0xF7858DAE, 0xAACB4E8A, 0x8F8F65C3, 0x9695F42E);

	//! Available OOR (Out Of Range) type definition
	enum GOORType {
		//! Constant
		G_CONSTANT_OOR,
		//! Loop (also known as 'wrap')
		G_LOOP_OOR,
		//! Ping-pong
		G_PINGPONG_OOR
	};

	//! Gettin/Setting value behaviors.
	enum GValueMethod {
		//! Specified get/set value is absolute.
		G_ABSOLUTE_VALUE,
		//! Specified get/set value is relative (not yet well supported).
		G_RELATIVE_VALUE
	};


	/*!
		\class GProperty
		\brief This class represents a generic animated property.

		An animated property is an 'entity' that changes its value during the time.
		Every GProperty has a name that identifies it, as well as an handled value type that represent the
		type of value that the property maintains (for example boolean, real, and so on).
		A property can be key-based or procedural, it can be checked using IsKeyBased() method.
		Key-based properties are the more used ones, and there are several methods to build and manipulate
		them. To get the property value at a specific time position, Value() method must be used.
		To set the property value at a specific time position, SetValue() method must be used.\n Properties
		supports 2 important features, that are OOR (Out-Of-Range) behaviors and ease curves.
		OOR behaviors make sure that if we wanna get a value of a property outside its time domain (defined
		by its keys), we can get it, using a constant, loop or ping-pong time mapping.
		Ease curve can be used to remap time into itself (also knows as a time-spline or time-map), and produce
		accelerations and decelerations (and also time inversion!) during the property life.
	*/
	class G_EXPORT GProperty : public GAnimElement {

		friend class GAnimElement;

	private:
		//! Property name.
		GString gName;
		//! Property upper name.
		GString gUpperName;
		//! If G_TRUE it indicated that ease must be applied (if exists).
		GBool gApplyEase;
		//! OOR behavior for time positions before domain lower bound.
		GOORType gOORBefore;
		//! OOR behavior for time positions after domain upper bound.
		GOORType gOORAfter;
		//! Flag indicating if the property is keybased or not.
		GBool gIsKeyBased;
		//! Ease property, if specified.
		GProperty *gEaseProperty;
		//! Cached value, used to store a valid value when the last remained key is going to be removed/deleted.
		GKeyValue gCachedValue;

	protected:
		//! Set property name
		GError SetName(const GString& NewName);
		//! Set keybased internal flag.
		inline void SetIsKeyBased(const GBool IsKeyBased) {
			gIsKeyBased = IsKeyBased;
		}
		//! Here is ensured that Index is valid; <b>This method MUST be implemented by all keybased properties</b>.
		virtual GError DoGetKey(const GUInt32 Index, GKeyValue& OutputKey) const {
			// just to avoid warnings...
			if (Index == 0) {
			}
			// signal that this method has not been implemented
			OutputKey.SetUndefined();
			return G_MISSED_FEATURE;
		}
		//! Here is ensured that Index is valid; <b>This method MUST be implemented by all keybased properties</b>.
		virtual GError DoSetKey(const GUInt32 Index, const GKeyValue& NewKeyValue) {
			// just to avoid warnings...
			if (Index == 0 && NewKeyValue.KeyType()) {
			}
			// signal that this method has not been implemented
			return G_MISSED_FEATURE;
		}
		/*!
			Get property value at a specified time position.

			\param OutputValue the returned property value.
			\param ValidInterval the validity interval to update; property validity interval should be intersected
			with this interval.
			\param TimePos the time position where to sample the property value. In this method it is ensured
			to be inside domain interval.
			\param GetMethod the method used to get value.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note <b>This method MUST be implemented by all properties</b>.
		*/
		virtual GError DoGetValue(GKeyValue& OutputValue, GTimeInterval& ValidInterval, const GTimeValue TimePos,
								  const GValueMethod GetMethod) const {

			OutputValue.SetUndefined();
			// just to avoid warnings...
			if (ValidInterval.IsEmpty() && TimePos == 0 && GetMethod == G_ABSOLUTE_VALUE) {
			}
			return G_MISSED_FEATURE;
		}
		/*!
			Set the property value at a specified time position.
			
			This method adds a new key at the specified time position, with the passed value.
			\param InputValue the value to set.
			\param TimePos the time position where to set the property value.
			\param SetMethod method used to set the value. For now, the only supported method is
			G_ABSOLUTE_VALUE.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note TimePos can be outside range, behavior is to append (at front/back) created key
			and expand domain. <b>This method has sense only for key-based properties, and MUST be
			implemented by all key-based properties</b>.
		*/
		virtual GError DoSetValue(const GKeyValue& InputValue, const GTimeValue TimePos, const GValueMethod SetMethod) {
			// just to avoid warnings...
			if (InputValue.RealValue() == 0 && SetMethod == G_ABSOLUTE_VALUE && TimePos) {
			}
			return G_MISSED_FEATURE;
		}
		/*! 
			Add a point on curve, TimePos is ensured to be inside domain.
			Index will be the index occupied by the created key.
			AlreadyExists will be G_TRUE if at the specified TimePos there was already an existing key.
			<b>This method must be implemented by all keybased properties</b>.
		*/
		virtual GError DoAddKey(const GTimeValue TimePos, GUInt32& Index, GBool& AlreadyExists) {
			// just to avoid warnings...
			if (TimePos == 0 && Index && AlreadyExists) {
			}
			return G_MISSED_FEATURE;
		}
		/*!
			Move a key to a different time position.

			\param Index the key that we wanna move, it is ensured valid.
			\param NewTimePos the new time position where to move the Index-th key.
			\param NewIndex the new index position associated to the moved key.
			\param AlreadyExists G_TRUE if at specified NewTimePos there was already a key. In this key it
			will be overridden by the moved key.
			\note <b>This method must be implemented by all keybased properties</b>.
		*/
		virtual GError DoMoveKey(const GUInt32 Index, const GReal NewTimePos, GUInt32& NewIndex, GBool& AlreadyExists) {
			// just to avoid warnings...
			if (Index && NewTimePos > 0 && NewIndex && AlreadyExists) {
			}
			return G_MISSED_FEATURE;
		}
		/*!
			Index is ensured to be valid; <b>This method must be implemented by all keybased properties</b>.
		*/
		virtual GError DoRemoveKey(const GUInt32 Index) {
			// just to avoid warnings...
			if (Index) {
			}
			return G_MISSED_FEATURE;
		}
		//! Return the number of keys (if keybased). <b>this method MUST be implemented by all keybased properties</b>
		virtual GInt32 DoGetKeysCount() const {
			return -1;
		}
		/*!
			Set all keys (they don't need to be sorted by time).

			This function rebuilds the property animated track, specifying all the new keys.
			
			\param Keys the array of keys, it is ensured to contain at least one entry.
			\note <b>Every key must be filled with value and time position. This method MUST be implemented
			by all keybased properties</b>.
		*/
		virtual GError DoSetKeys(const GDynArray<GKeyValue>& Keys) {
			// just to avoid warnings
			if (Keys.size() > 0) {
			}
			return G_MISSED_FEATURE;
		}

		//! Cloning function, copies (physically) a Source GProperty class.
		GError BaseClone(const GElement& Source);

	public:
		//! Default constructor
		GProperty();
		//! Default constructor with owner specification.
		GProperty(const GElement* Owner);
		//! Destructor, delete all keys and internal ease curve (if it exists).
		~GProperty();
		/*
			Clear the property, the default implementation is:

			- for keybased properties: remove all keys, calling RemoveKey method; then delete all sub-properties.
			- for non keybased property: nothing.

			NB: ease property will not be removed nor deleted. The default/cached value will be the value
			associated to the first key.
		*/
		virtual void Clear();
		//! Get property name.
		inline const GString& Name() const {
			return gName;
		}
		//! Get the upper-case version of property name; used by GAnimElement to do a fast property search by name.
		inline const GString& UpperName() const {
			return gUpperName;
		}
		//! Get time domain range; <b>Every procedural property MUST implement this method</b>.
		virtual GTimeInterval Domain() const;
		//! Get default value.
		inline const GKeyValue& DefaultValue() const {
			return gCachedValue;
		}
		/*!
			Set default value. If passed value is not of the same type as HandledType(), a default 0(s) value
			will be used (only if current default value is undefined).
		*/
		inline void SetDefaultValue(const GKeyValue& NewValue) {

			if (NewValue.KeyType() != HandledType()) {
				if (gCachedValue.KeyType() == G_UNDEFINED_KEY) {
					// apply a default 0 value
					gCachedValue.SetValue(GVector4(0, 0, 0, 0));
					gCachedValue.SetKeyType(HandledType());
				}
			}
			else
				gCachedValue = NewValue;
		}
		//! G_TRUE if the property is keybased, G_FALSE if the property is procedural.
		inline GBool IsKeyBased() const {
			return gIsKeyBased;
		}
		//! Number of keys, -1 if property is not keybased.
		inline GInt32 KeysCount() const {
			if (gIsKeyBased)
				return DoGetKeysCount();
			return -1;
		}
		/*!
			Get a key value, specifying index.
			OutputKey will contain the value and the time-position.
		*/
		GError Key(const GUInt32 Index, GKeyValue& OutputKey) const;
		//! Set a key value, specifying key index (NewKeyValue time position will be ignored, just value will be used).
		GError SetKey(const GUInt32 Index, const GKeyValue& NewKeyValue);
		/*!
			Add a key on curve.

			\param Time the time position where to add the key.
			\param Index index of the created key.
			\param AlreadyExists G_TRUE if at specified Time position a key was already existing. In this case
			all remains untouched.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError AddKey(const GTimeValue Time, GUInt32& Index, GBool& AlreadyExists);
		/*!
			Move a key to a different time position.

			\param Index the key that we wanna move, it must be valid.
			\param NewTimePos the new time position where to move the Index-th key.
			\param NewIndex the new index position associated to the moved key.
			\param AlreadyExists G_TRUE if at specified NewTimePos there was already a key. In this key it
			will be overridden by the moved key.
		*/
		GError MoveKey(const GUInt32 Index, const GReal NewTimePos,	GUInt32& NewIndex, GBool& AlreadyExists);
		//! Remove a key by index.
		GError RemoveKey(const GUInt32 Index);
		/*!
			Set all keys (they don't need to be sorted by time).

			This function rebuilds the property animated track, specifying all the new keys.
			The specified keys can be in any time-order, they will be sorted internally.
			\param Keys the array of keys, it must contain at least one entry.
			\note <b>Every key must be filled with value and time position.</b>.
		*/
		GError SetKeys(const GDynArray<GKeyValue>& Keys);
		/*!
			Remove all keys.
			
			\note: the default/cached value will be the value associated to the first key.
		*/
		GError RemoveKeys();
		/*!
			Get property value at a specified time position.

			\param OutputValue the returned property value.
			\param ValidInterval the validity interval to update; property validity interval should be intersected
			with this interval.
			\param TimePos the time position where to sample the property value.
			\param GetMethod the method used to get value. For now, the only supported method is
			G_ABSOLUTE_VALUE.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		virtual GError Value(GKeyValue& OutputValue, GTimeInterval& ValidInterval,
							 const GTimeValue TimePos = 0, const GValueMethod GetMethod = G_ABSOLUTE_VALUE) const;
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
		virtual GError SetValue(const GKeyValue& InputValue, const GTimeValue TimePos = 0,
								const GValueMethod SetMethod = G_ABSOLUTE_VALUE);
		/*!
			Get ease property, if exists.
			
			\note Please do not call delete on returned pointer; if you want to remove ease property
			please call RemoveEaseProperty() method.
		*/
		inline GProperty* EaseProperty() const {
			return gEaseProperty;
		}
		/*!
			Set ease property.

			The specified EaseProperty must have an HandledType() equal to G_REAL_KEY. Internally this method
			creates a new ease value and copies EaseProperty, so the specified parameter won't be touched.
		*/
		GError SetEaseProperty(const GProperty& EaseProperty);
		//! Remove and delete ease property.
		void RemoveEaseProperty();
		//! If the property is keybased, this flag specify if easy spline must be applied or not.
		inline const GBool ApplyEase() {
			if (!gIsKeyBased)
				return G_FALSE;
			return gApplyEase;
		}
		//! If the property is keybased, this methods permit to specify if easy spline must be applied or not.
		inline void SetApplyEasy (const GBool Apply) {
			gApplyEase = Apply;
		}
		//! Get OOR (out of range) behavior for time positions before (time)domain lower bound.
		inline GOORType OORBefore() const {
			return gOORBefore;
		}
		//! Set OOR (out of range) behavior for time positions before (time)domain lower bound.
		inline void SetOORBefore(const GOORType NewOOR) {
			gOORBefore = NewOOR;
		}
		//! Get OOR (out of range) behavior for time positions after (time)domain upper bound.
		inline GOORType OORAfter() const {
			return gOORAfter;
		}
		//! Set OOR (out of range) behavior for time positions after (time)domain upper bound.
		inline void SetOORAfter(const GOORType NewOOR) {
			gOORAfter = NewOOR;
		}
		/*!
			Remap a (possible out of range) time into the corresponding one that it's ensured to lie
			inside domain interval.
		*/
		GTimeValue OORTime(const GTimeValue TimePos) const;
		//! Get handled key values. <b>Every property MUST implement this method</b>.
		virtual GKeyType HandledType() const {
			return G_UNDEFINED_KEY;
		}
		//! Get class descriptor
		inline const GClassID& ClassID() const {
			return G_PROPERTY_CLASSID;
		}
		//! Get base class (father class) descriptor
		inline const GClassID& DerivedClassID() const {
			return G_ANIMELEMENT_CLASSID;
		}
	};

	// *********************************************************************
	//                              GPropertyProxy
	// *********************************************************************
	/*!
		\class GPropertyProxy
		\brief This class implements a GProperty proxy (provider).

		This proxy provides the creation of GProperty class instances.
	*/
	class G_EXPORT GPropertyProxy : public GElementProxy {
	public:
		//! Creates a new GProperty instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GProperty(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		inline const GClassID& ClassID() const {
			return G_PROPERTY_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		inline const GClassID& DerivedClassID() const {
			return G_ANIMELEMENT_CLASSID;
		}
	};
	//! Static proxy for GProperty class.
	static const GPropertyProxy G_PROPERTY_PROXY;

};	// end namespace Amanith

#endif
