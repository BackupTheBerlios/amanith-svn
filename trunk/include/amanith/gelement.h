/****************************************************************************
** $file: amanith/gelement.h   0.1.0.0   edited Jun 30 08:00
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
	//                             GAnimElement
	// *********************************************************************
	static const GClassID G_ANIMELEMENT_CLASSID = GClassID("GAnimElement", 0x8B98ACC9, 0x577E4416, 0xB9230A4A, 0xE4762EEE);
	// forward declaration
	class GProperty;

	class G_EXPORT GAnimElement : public GElement {
	private:
		// internal list of properties
		GDynArray<GProperty*> gProperties;

		// delete and empty all properties
		void DeleteProperties(void);
		// clone properties
		GError CloneProperties(const GAnimElement& Source);

	protected:
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
		// Add a property
		GProperty* AddProperty(const GString& Name, const GClassID& ClassID, GBool& AlreadyExist,
							   GUInt32& PropertyIndex);
		// Get an existing property (given its name)
		GProperty* Property(const GString& Name) const;
		// get an existing property (given its index)
		GProperty* Property(const GUInt32 Index) const;
		// remove the specified property form internal list
		GBool RemoveProperty(const GString& Name);
		// remove the specified property form internal list
		GBool RemoveProperty(const GUInt32 Index);
		// get full list of properties
		const GDynArray<GProperty*>& Properties() const {
			return gProperties;
		}
		//! Get number of properties.
		inline const GInt32 PropertiesCount() const {
			return (GInt32)gProperties.size();
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
	//                              GKeyValue
	// *********************************************************************

	// key types
	enum GKeyType {
		G_UNDEFINED_KEY,
		G_BOOL_KEY,
		G_INT_KEY,
		G_REAL_KEY,
		G_VECTOR2_KEY,
		G_VECTOR3_KEY,
		G_VECTOR4_KEY
	};

	class G_EXPORT GKeyValue {

		friend class GProperty;

	private:
		GKeyType gType;
		GTimeValue gTimePos;
		GVector4 gValue;
		void* gCustomData;

	protected:
		void SetUndefined() {
			gType = G_UNDEFINED_KEY;
			gValue.Set(G_MIN_REAL, G_MIN_REAL, G_MIN_REAL, G_MIN_REAL);
			gTimePos = G_MIN_REAL;
			gCustomData = NULL;
		}

	public:
		GKeyValue();
		GKeyValue(const GBool Value);
		GKeyValue(const GInt32 Value);
		GKeyValue(const GReal Value);
		GKeyValue(const GVector2& Value);
		GKeyValue(const GVector3& Value);
		GKeyValue(const GVector4& Value);
		GKeyValue(const GTimeValue TimePos, const GBool Value);
		GKeyValue(const GTimeValue TimePos, const GInt32 Value);
		GKeyValue(const GTimeValue TimePos, const GReal Value);
		GKeyValue(const GTimeValue TimePos, const GVector2& Value);
		GKeyValue(const GTimeValue TimePos, const GVector3& Value);
		GKeyValue(const GTimeValue TimePos, const GVector4& Value);

		void SetValue(const GBool Value);
		void SetValue(const GInt32 Value);
		void SetValue(const GReal Value);//, const GReal InTangent = 0, const GReal OutTangent = 0);
		void SetValue(const GVector2& Value);//, const GVector2& InTangent = G_NULL_POINT2, const GVector2& OutTangent = G_NULL_POINT2);
		void SetValue(const GVector3& Value);//, const GVector3& InTangent = G_NULL_POINT3, const GVector3& OutTangent = G_NULL_POINT3);
		void SetValue(const GVector4& Value);//, const GVector4& InTangent = G_NULL_POINT4, const GVector4& OutTangent = G_NULL_POINT4);
		void SetCustomData(void* CustomData) {
			gCustomData = CustomData;
		}
		// get key type
		inline GKeyType KeyType() const {
			return gType;
		}
		void SetKeyType(const GKeyType NewType) {
			gType = NewType;
		}
		// get key position (timeline position)
		inline GTimeValue TimePosition() const {
			return gTimePos;
		}
		void SetTimePosition(const GTimeValue NewTimePos) {
			gTimePos = NewTimePos;
		}
		GBool BoolValue() const;
		GInt32 IntValue() const;
		GReal RealValue() const;
		GVector2 Vect2Value() const;
		GVector3 Vect3Value() const;
		GVector4 Vect4Value() const;
		// get custom data associated with this key
		inline void *CustomData() const {
			return gCustomData;
		}
	};


	// *********************************************************************
	//                             GProperty
	// *********************************************************************
	static const GClassID G_PROPERTY_CLASSID = GClassID("GProperty", 0xF7858DAE, 0xAACB4E8A, 0x8F8F65C3, 0x9695F42E);

	// OOR (Out Of Range) type definition
	enum GOORType {
		G_CONSTANT_OOR,
		G_LOOP_OOR,
		G_PINGPONG_OOR
	};

	// used for get/set value
	enum GValueMethod {
		G_ABSOLUTE_VALUE,
		G_RELATIVE_VALUE
	};

	class G_EXPORT GProperty : public GAnimElement {

		friend class GAnimElement;

	private:
		GString gName;
		GString gUpperName;
		GBool gApplyEase;
		GOORType gOORBefore;
		GOORType gOORAfter;
		GBool gIsKeyBased;
		GProperty *gEaseProperty;

	protected:
		GError SetName(const GString& NewName);

		inline void SetIsKeyBased(const GBool IsKeyBased) {
			gIsKeyBased = IsKeyBased;
		}

		// here is ensured that Index is valid; <b>This method MUST be implemented by all keybased properties</b>
		virtual GError DoGetKey(const GUInt32 Index, GKeyValue& OutputKey) const {
			// just to avoid warnings...
			if (Index == 0) {
			}
			// signal that this method has not been implemented
			OutputKey.SetUndefined();
			return G_MISSED_FEATURE;
		}

		// get local value; Time is ensured to be inside life-interval.
		virtual GError DoGetValue(GKeyValue& OutputValue, GTimeInterval& ValidInterval, const GTimeValue Time,
								  const GValueMethod GetMethod) const {

			OutputValue.SetUndefined();
			// just to avoid warnings...
			if (ValidInterval.IsEmpty() && Time == 0 && GetMethod == G_ABSOLUTE_VALUE) {
			}
			return G_MISSED_FEATURE;
		}
		// set local value; InputValue.TimePosition can be outside range, behavior is to append key
		// and expand domain
		virtual GError DoSetValue(const GKeyValue& InputValue, const GValueMethod SetMethod) {
			// just to avoid warnings...
			if (InputValue.TimePosition() == 0 && SetMethod == G_ABSOLUTE_VALUE) {
			}
			return G_MISSED_FEATURE;
		}

		// add a point ON curve, Time is ensured to be inside domain;
		// <b>This method must be implemented by all keybased properties</b>
		virtual GError DoAddKey(const GTimeValue Time, GUInt32& Index, GBool& AlreadyExists) {
			// just to avoid warnings...
			if (Time == 0 && Index && AlreadyExists) {
			}
			return G_MISSED_FEATURE;
		}
		// here is ensured that Index is valid; <b>This method must be implemented by all keybased properties</b>
		virtual GError DoMoveKey(const GUInt32 Index, const GReal NewTimePos, GUInt32& NewIndex, GBool& AlreadyExists) {
			// just to avoid warnings...
			if (Index && NewTimePos > 0 && NewIndex && AlreadyExists) {
			}
			return G_MISSED_FEATURE;
		}
		// Index is ensured to be valid; <b>This method must be implemented by all keybased properties</b>
		virtual GError DoRemoveKey(const GUInt32 Index) {
			// just to avoid warnings...
			if (Index) {
			}
			return G_MISSED_FEATURE;
		}

		// <b>this method MUST be implemented by all keybased properties</b>
		virtual GInt32 DoGetKeysCount() const {
			return -1;
		}

		// <b>this method MUST be implemented by all keybased properties</b>
		virtual GError DoSetKeys(const GDynArray<GKeyValue>& Keys) {
			// just to avoid warnings
			if (Keys.size() > 0) {
			}
			return G_MISSED_FEATURE;
		}

		// basic cloning function
		GError BaseClone(const GElement& Source);

	public:
		// default constructor
		GProperty();
		// default constructor with owner
		GProperty(const GElement* Owner);
		// destructor, delete all keys and internal ease curve (if it exists)
		~GProperty();
		// get property name
		inline const GString& Name() const {
			return gName;
		}
		// get the upper-case version of property name; used by GAnimElement to do a fast property search by name
		inline const GString& UpperName() const {
			return gUpperName;
		}
		// get time domain range; <b>Every procedural property MUST implement this method</b>
		virtual GTimeInterval Domain() const;
		// true if keybased, procedural otherwise
		inline GBool IsKeyBased() const {
			return gIsKeyBased;
		}
		// number of keys, -1 if property is not keybased
		inline GInt32 KeysCount() const {
			if (gIsKeyBased)
				return DoGetKeysCount();
			return -1;
		}
		// get key, specifying index
		GError Key(const GUInt32 Index, GKeyValue& OutputKey) const;
		// get a key at a specified time; if the key does not exists it returns NULL pointer;
		// if the key exists, the "index" parameter tell its internal array position
		//GError Key(const GTimeValue Time, GKey& OutputKey, GUInt32& Index) const;

		GError AddKey(const GTimeValue Time, GUInt32& Index, GBool& AlreadyExists);
		GError MoveKey(const GUInt32 Index, const GReal NewTimePos,	GUInt32& NewIndex, GBool& AlreadyExists);
		GError RemoveKey(const GUInt32 Index);
		GError SetKeys(const GDynArray<GKeyValue>& Keys);

		virtual GError Value(GKeyValue& OutputValue, GTimeInterval& ValidInterval,
							 const GTimeValue Time = 0, const GValueMethod GetMethod = G_ABSOLUTE_VALUE) const;
		// set the property value (add a new key)
		virtual GError SetValue(const GKeyValue& InputValue, const GValueMethod SetMethod = G_ABSOLUTE_VALUE);

		// get ease property
		inline GProperty* EaseProperty() const {
			return gEaseProperty;
		}
		// set ease property
		GError SetEaseProperty(const GProperty& EaseProperty);
		// if keybased, am i applying easy spline?
		inline const GBool ApplyEase() {
			if (!gIsKeyBased)
				return G_FALSE;
			return gApplyEase;
		}
		// have i to apply easy spline?
		inline void SetApplyEasy (const GBool Apply) {
			gApplyEase = Apply;
		}
		// get oor (out of range) before
		inline GOORType OORBefore() const {
			return gOORBefore;
		}
		// set oor (out of range) before
		inline void SetOORBefore(const GOORType NewOOR) {
			gOORBefore = NewOOR;
		}
		// get oor (out of range) after
		inline GOORType OORAfter() const {
			return gOORAfter;
		}
		// set oor (out of range) after
		inline void SetOORAfter(const GOORType NewOOR) {
			gOORAfter = NewOOR;
		}
		// remap a (possible out of range) time into the corresponding one that it's ensured to lie
		// inside domain interval
		GTimeValue OORTime(const GTimeValue Time) const;
		// <b>Every property MUST implement this method</b>
		virtual GKeyType HandledType() const {
			return G_UNDEFINED_KEY;
		}
	};

	// *********************************************************************
	//                              GPropertyProxy
	// *********************************************************************
	class G_EXPORT GPropertyProxy : public GElementProxy {
	public:
		//! Get class descriptor of elements type "provided" by this proxy.
		inline const GClassID& ClassID() const {
			return G_PROPERTY_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		inline const GClassID& DerivedClassID() const {
			return G_ANIMELEMENT_CLASSID;
		}
	};

	static const GPropertyProxy G_PROPERTY_PROXY;

};	// end namespace Amanith

#endif
