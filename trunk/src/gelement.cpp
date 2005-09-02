/****************************************************************************
** $file: amanith/src/gelement.cpp   0.1.0.0   edited Jun 30 08:00
**
** Base elements implementation.
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

#include "amanith/gelement.h"
#include "amanith/gkernel.h"
#include "amanith/gproperty.h"
#include "amanith/support/gutilities.h"

/*!
	\file gelement.cpp
	\brief Implementation of GElement class.
*/

namespace Amanith {

const GClassID GClassID::Null = GClassID();


// *********************************************************************
//                               GElement
// *********************************************************************

/*!
	\class GElement
	\brief Minimum element class equipped with introspection and cloning system.

	GElement represents the base class that permits introspection and other facilities, like
	chain-cloning system, centralized creation and destruction.\n
	Every derived class must implement this functions:\n
	\n
	ClassID(): to get class descriptor.\n
	DerivedClassID(): to get base class (father class) descriptor.\n
	BaseClone(): to make physical copy possible. A typical implementation of this function includes just the
	copy of private and protected members, and then calls base class (father class) BaseClone() method.
*/

// constructor
GElement::GElement(): gOwner(NULL) {
}

/*!
	Check if owner is valid (it must be described by a G_KERNEL_CLASSID class identifier). If it's valid, we must notify
	him to add this element instance to internal list.
*/
GElement::GElement(const GElement* Owner) {

	if (Owner->IsOfType(G_KERNEL_CLASSID)) {
		gOwner = Owner;
		GKernel *kernel = (GKernel *)gOwner;
		kernel->AddElementReference(this);
	}
	else
		gOwner = NULL;
}

/*!
	If owner is not NULL, then notify him to remove this element instance from internal list.
*/
GElement::~GElement() {

	// is now very important to acknowledge my owner (kernel) that i'm going to be freed.
	if (gOwner) {
		G_ASSERT(gOwner->IsOfType(G_KERNEL_CLASSID) == G_TRUE);
		GKernel *kernel = (GKernel *)gOwner;
		kernel->RemoveElementReference(this);
	}
}

/*!
	Creation is made possible thanks kernel only, using proxies mechanism. If this element has not be created by a
	kernel, then it can't create new elements through this method.
*/
GElement* GElement::CreateNew(const GClassID& Class_ID) const {

	if (gOwner) {
		G_ASSERT(gOwner->IsOfType(G_KERNEL_CLASSID) == G_TRUE);
		GKernel *kernel = (GKernel *)gOwner;
		return (kernel->CreateNew(Class_ID));
	}
	return NULL;
}

/*!
	\param Class_Name a string containing the class name that we wanna know the descriptor of
	\param OutID a class descriptor that will be returned in case of successful operation.
	\return If operation will succeed G_NO_ERROR constant, else an error code.
	\note Class name comparison is case-insensitive.\n
	A return value of G_UNSUPPORTED_CLASSID means that specified Class_Name does not correspond to any
	registered proxies, so the class descriptor cannot be "resolved".
*/
GError GElement::ClassIDFromClassName(const GString& Class_Name, GClassID& OutID) const {

	// first lets see if class name match my class name
	if (StrUtils::SameText(Class_Name, ClassID().IDName())) {
		OutID = ClassID();
		return G_NO_ERROR;
	}
	// now try with owner
	if (gOwner) {
		G_ASSERT(gOwner->IsOfType(G_KERNEL_CLASSID) == G_TRUE);
		GKernel *kernel = (GKernel *)gOwner;
		return kernel->ClassIDFromClassName(Class_Name, OutID);
	}
	return G_UNSUPPORTED_CLASSID;
}

/*!
	If element can't be created a NULL pointer is returned.
	\note Class name comparison is case-insensitive.
*/
GElement* GElement::CreateNew(const GString& Class_Name) {

	GClassID cid;
	GError err;
	
	err = ClassIDFromClassName(Class_Name, cid);
	if (err == G_NO_ERROR)
		return CreateNew(cid);
	return NULL;
}

GBool GElement::IsTypeOfType(const GClassID& InspectedType, const GClassID& Class_ID) const {

	// trivial case
	if (Class_ID == InspectedType)
		return G_TRUE;
	// as an element, i can just say that my ClassID is of DerivedClassID type
	if ((InspectedType == ClassID()) && (Class_ID == DerivedClassID()))
		return G_TRUE;
	// try with owner
	if (gOwner)
		return gOwner->IsTypeOfType(InspectedType, Class_ID);
	// no more chances left
	return G_FALSE;
}

// is an element of a specified type? it does include inheritance inspection
GBool GElement::IsOfType(const GClassID& Class_ID) const {

	// trivial case
	if ((Class_ID == ClassID()) || (Class_ID == DerivedClassID()))
		return G_TRUE;
	return IsTypeOfType(ClassID(), Class_ID);
}

/*!
	If copy can't be done an error code is returned, else G_NO_ERROR.
	The copy schema is subdivided into 3 steps:

	-# If Source is the same object pointed by 'this' pointer, just exit with a G_NO_ERROR code.
	-# If Source is not of the same type of this element, use owner (kernel) to do the copy (using Copy() function).\n
	The kernel will try to do a (physical) type conversion (using registered converters plugins), and then do the
	conversion. For further details see GKernel::Copy().
	-# If Source is of the same type of this element, then it is safe to call BaseClone().

	\note If as Source parameter will be passed a 'this' reference the function just returns a G_NO_ERROR without doing
	nothing. So a call like element->CopyFrom(*element) is not dangerous.

*/
GError GElement::CopyFrom(const GElement& Source) {

	const GElement *ptr	= &Source;

	// check for a self to self copy
	if (ptr == this)
		return G_NO_ERROR;
	// check for a valid (compatible) source
	if (!Source.IsOfType(ClassID())) {
		if (gOwner) {
			G_ASSERT(gOwner->IsOfType(G_KERNEL_CLASSID) == G_TRUE);
			GKernel *kernel = (GKernel *)gOwner;
			return kernel->Copy(Source, *this);
		}
		else
			return G_UNSUPPORTED_CLASSID;
	}
	// now is safe to do a clone
	return BaseClone(Source);
}


// *********************************************************************
//                             GKeyValue
// *********************************************************************
GKeyValue::GKeyValue() {

	SetUndefined();
}

GKeyValue::GKeyValue(const GBool Value) {

	gType = G_BOOL_KEY;
	gTimePos = 0;
	gCustomData = NULL;
	if (Value)
		gValue[0] = (GReal)1;
}

GKeyValue::GKeyValue(const GInt32 Value) {

	gType = G_INT_KEY;
	gTimePos = 0;
	gCustomData = NULL;
	// cast can be ensured "exact" for 23/52 bits (mantissa precision)
	if (Value)
		gValue[0] = (GReal)Value;
}

GKeyValue::GKeyValue(const GReal Value) {

	gType = G_REAL_KEY;
	gTimePos = 0;
	gCustomData = NULL;
	gValue[0] = Value;
}

GKeyValue::GKeyValue(const GVector2& Value) {

	gType = G_VECTOR2_KEY;
	gTimePos = 0;
	gCustomData = NULL;
	gValue.Set(Value[0], Value[1]);
}

GKeyValue::GKeyValue(const GVector3& Value) {

	gType = G_VECTOR3_KEY;
	gTimePos = 0;
	gCustomData = NULL;
	gValue.Set(Value[0], Value[1], Value[2]);
}

GKeyValue::GKeyValue(const GVector4& Value) {

	gType = G_VECTOR4_KEY;
	gTimePos = 0;
	gCustomData = NULL;
	gValue = Value;
}

// constructors with time specification
GKeyValue::GKeyValue(const GTimeValue TimePos, const GBool Value) {
	SetValue(Value);
	SetTimePosition(TimePos);
}

GKeyValue::GKeyValue(const GTimeValue TimePos, const GInt32 Value){
	SetValue(Value);
	SetTimePosition(TimePos);
}

GKeyValue::GKeyValue(const GTimeValue TimePos, const GReal Value) {
	SetValue(Value);
	SetTimePosition(TimePos);
}

GKeyValue::GKeyValue(const GTimeValue TimePos, const GVector2& Value) {
	SetValue(Value);
	SetTimePosition(TimePos);
}

GKeyValue::GKeyValue(const GTimeValue TimePos, const GVector3& Value) {
	SetValue(Value);
	SetTimePosition(TimePos);
}

GKeyValue::GKeyValue(const GTimeValue TimePos, const GVector4& Value) {
	SetValue(Value);
	SetTimePosition(TimePos);
}

void GKeyValue::SetValue(const GBool Value) {

	if (Value)
		gValue[0] = (GReal)1;
	else
		gValue[0] = (GReal)0;
	gType = G_BOOL_KEY;
}


void GKeyValue::SetValue(const GInt32 Value) {

	gValue[0] = (GReal)Value;
	gType = G_INT_KEY;
}


void GKeyValue::SetValue(const GReal Value) {

	gValue[0] = Value;
	gType = G_REAL_KEY;
}

void GKeyValue::SetValue(const GVector2& Value) {

	gValue.Set(Value[0], Value[1]);
	gType = G_VECTOR2_KEY;
}

void GKeyValue::SetValue(const GVector3& Value) {

	gValue.Set(Value[0], Value[1], Value[2]);
	gType = G_VECTOR3_KEY;
}

void GKeyValue::SetValue(const GVector4& Value) {

	gValue = Value;
	gType = G_VECTOR4_KEY;
}

GBool GKeyValue::BoolValue() const {
	if (gValue[0] != 0)
		return G_TRUE;
	return G_FALSE;
}

GInt32 GKeyValue::IntValue() const {
	return (GInt32)gValue[0];
}

GReal GKeyValue::RealValue() const {
	return gValue[0];
}

GVector2 GKeyValue::Vect2Value() const {
	return GVector2(gValue[0], gValue[1]);
}

GVector3 GKeyValue::Vect3Value() const {
	return GVector3(gValue[0], gValue[1], gValue[2]);
}

GVector4 GKeyValue::Vect4Value() const {
	return gValue;
}

// *********************************************************************
//                             GProperty
// *********************************************************************

// default constructor
GProperty::GProperty() : GAnimElement() {

	gName = gUpperName = "";
	gApplyEase = G_TRUE;
	gIsKeyBased = G_TRUE;
	gOORBefore = G_CONSTANT_OOR;
	gOORAfter = G_CONSTANT_OOR;
	gEaseProperty = NULL;
}

GProperty::GProperty(const GElement* Owner) : GAnimElement(Owner) {

	gName = gUpperName = "";
	gApplyEase = G_TRUE;
	gIsKeyBased = G_TRUE;
	gOORBefore = G_CONSTANT_OOR;
	gOORAfter = G_CONSTANT_OOR;
	gEaseProperty = NULL;
}

// destructor
GProperty::~GProperty() {

	if (gEaseProperty)
		delete gEaseProperty;
}

GError GProperty::SetName(const GString& NewName) {

	if (NewName.length() <= 0)
		return G_INVALID_PARAMETER;

	gName = NewName;
	gUpperName = StrUtils::Upper(gName);
	return G_NO_ERROR;
}

// get key, specifying index
GError GProperty::Key(const GUInt32 Index, GKeyValue& OutputKey) const {

	if (!IsKeyBased())
		return G_INVALID_OPERATION;

	if ((GInt32)Index >= KeysCount())
		return G_OUT_OF_RANGE;

	return DoGetKey(Index, OutputKey);
}

GError GProperty::AddKey(const GTimeValue Time, GUInt32& Index, GBool& AlreadyExists) {

	if (!IsKeyBased())
		return G_INVALID_OPERATION;

	GTimeInterval myRange = Domain();

	if (!myRange.IsInInterval(Time))
		return G_OUT_OF_RANGE;

	return DoAddKey(Time, Index, AlreadyExists);
}

GError GProperty::MoveKey(const GUInt32 Index, const GReal NewTimePos, GUInt32& NewIndex, GBool& AlreadyExists) {

	if (!IsKeyBased())
		return G_INVALID_OPERATION;

	if ((GInt32)Index >= KeysCount())
		return G_OUT_OF_RANGE;

	return DoMoveKey(Index, NewTimePos, NewIndex, AlreadyExists);
}

GError GProperty::RemoveKey(const GUInt32 Index) {

	if (!IsKeyBased())
		return G_INVALID_OPERATION;

	if ((GInt32)Index >= KeysCount())
		return G_OUT_OF_RANGE;

	return DoRemoveKey(Index);
}

GError GProperty::SetKeys(const GDynArray<GKeyValue>& Keys) {

	if (!IsKeyBased())
		return G_INVALID_OPERATION;

	if (Keys.size() == 0)
		return G_INVALID_PARAMETER;

	return DoSetKeys(Keys);
}

GError GProperty::SetEaseProperty(const GProperty& EaseProperty) {

	// simple check, ease properties must have a scalar value
	if (EaseProperty.HandledType() != G_REAL_KEY)
		return G_INVALID_PARAMETER;

	// delete current ease property
	if (gEaseProperty)
		delete gEaseProperty;

	// create a new ease property
	gEaseProperty = (GProperty *)CreateNew(EaseProperty.ClassID());
	if (gEaseProperty)
		// clone ease property from specified source
		return gEaseProperty->CopyFrom(EaseProperty);
	else
		return G_MEMORY_ERROR;
}

// get time range
GTimeInterval GProperty::Domain() const {

	if (!IsKeyBased())
		return G_NEVER_TIMEINTERVAL;

	GInt32 i = KeysCount();
	// in this keys return a "never" interval
	if (i <= 0)
		return G_NEVER_TIMEINTERVAL;

	// get first and last key
	GKeyValue k1, k2;

	DoGetKey(0, k1);
	DoGetKey((GUInt32)(i - 1), k2);
	// build time range
	return GTimeInterval(k1.TimePosition(), k2.TimePosition());
}

// base clone function
GError GProperty::BaseClone(const GElement& Source) {

	const GProperty& p = (const GProperty&)Source;

	if (p.gEaseProperty) {
		GProperty *tmpProp = (GProperty *)CreateNew(p.gEaseProperty->ClassID());
		if (tmpProp) {
			GError err = tmpProp->CopyFrom(*(p.gEaseProperty));
			if (err != G_NO_ERROR)
				return err;
			delete gEaseProperty;
			gEaseProperty = tmpProp;
		}
		else
			return G_MEMORY_ERROR;
	}
	gName = p.gName;
	gUpperName = p.gUpperName;
	gApplyEase = p.gApplyEase;
	gOORBefore = p.gOORBefore;
	gOORAfter = p.gOORAfter;
	gIsKeyBased = p.gIsKeyBased;
	return GAnimElement::BaseClone(Source);
}

GError GProperty::Value(GKeyValue& OutputValue, GTimeInterval& ValidInterval, const GTimeValue Time,
						const GValueMethod GetMethod) const {

	GTimeInterval myRange, easeValid, localValid;
	GOORType usedOOR;
	GTimeValue tmpTime, cycledTime;
	GInt32 nCycles;
	GError err;

	//! \todo Implement relative value getting.
	if (GetMethod == G_RELATIVE_VALUE)
		return G_UNSUPPORTED_CLASSID;

	// set validity intervals to 'forvever'
	localValid = G_FOREVER_TIMEINTERVAL;
	myRange = Domain();
	tmpTime = Time;

	// apply ease (time curve)
	if (gApplyEase && gEaseProperty) {
		
		GKeyValue easeValue;

		err = gEaseProperty->Value(easeValue, easeValid, tmpTime, G_ABSOLUTE_VALUE);
		if (err != G_NO_ERROR)
			return err;
		tmpTime = easeValue.RealValue();
	}

	err = G_NO_ERROR;
	if (myRange.IsEmpty() || myRange.IsInInterval(tmpTime))
		// in this case we haven't to use OOR
		err = DoGetValue(OutputValue, localValid, tmpTime, G_ABSOLUTE_VALUE);
	else {
		// lets see if time is in timerange, and get the correct OOR
		if (tmpTime <= myRange.Start())
			usedOOR = OORBefore();
		else
			usedOOR = OORAfter();

		switch (usedOOR) {

			case G_CONSTANT_OOR:
				if (tmpTime < myRange.Start()) {
					err = DoGetValue(OutputValue, localValid, myRange.Start(), G_ABSOLUTE_VALUE);
					localValid.Set(G_MIN_REAL, myRange.Start());
				}
				else {
					err = DoGetValue(OutputValue, localValid, myRange.End(), G_ABSOLUTE_VALUE);
					localValid.Set(myRange.End(), G_MAX_REAL);
				}
				break;

			case G_LOOP_OOR:
				err = DoGetValue(OutputValue, localValid, myRange.CycleValue(tmpTime), G_ABSOLUTE_VALUE);
				break;

			case G_PINGPONG_OOR:
				nCycles = myRange.CyclesCount(tmpTime);
				cycledTime = myRange.CycleValue(tmpTime);
				if (nCycles & 1)
					cycledTime = myRange.End() - (cycledTime - myRange.Start());
				err = DoGetValue(OutputValue, localValid, cycledTime, G_ABSOLUTE_VALUE);
				break;
		}
	}

	ValidInterval = localValid;
	ValidInterval &= easeValid;

	if (ValidInterval.IsEmpty())
		ValidInterval.Set(Time, Time);

	return err;
}

// set property value at a specific time position
GError GProperty::SetValue(const GKeyValue& InputValue, const GValueMethod SetMethod) {

	if (InputValue.KeyType() != HandledType())
		return G_INVALID_PARAMETER;

	GKeyValue tmpKey(InputValue);

	// apply ease (time curve)
	if (gApplyEase && gEaseProperty) {

		GKeyValue easeValue;

		GTimeInterval easeValid = G_FOREVER_TIMEINTERVAL;
		GError err = gEaseProperty->Value(easeValue, easeValid, InputValue.TimePosition(), G_ABSOLUTE_VALUE);
		if (err != G_NO_ERROR)
			return err;
		tmpKey.SetTimePosition(easeValue.RealValue());
	}
	return DoSetValue(tmpKey, SetMethod);
}

// remap a (possible out of range) time into the corresponding one that it's ensured to lie
// inside domain interval
GTimeValue GProperty::OORTime(const GTimeValue Time) const {

	GOORType usedOOR;
	GTimeValue cycledTime;
	GInt32 nCycles;
	GTimeInterval myRange = Domain();

	if (myRange.IsEmpty())
		return myRange.Start();

	// if specified time is already inside domain, then return it
	if (myRange.IsInInterval(Time))
		return Time;

	// lets see if time is in timerange, and get the correct OOR
	if (Time <= myRange.Start())
		usedOOR = OORBefore();
	else
		usedOOR = OORAfter();

	switch (usedOOR) {
		case G_CONSTANT_OOR:
			if (Time < myRange.Start())
				return myRange.Start();
			else
				return myRange.End();
			break;

		case G_LOOP_OOR:
			return myRange.CycleValue(Time);
			break;

		case G_PINGPONG_OOR:
			nCycles = myRange.CyclesCount(Time);
			cycledTime = myRange.CycleValue(Time);
			if (nCycles & 1)
				cycledTime = myRange.End() - (cycledTime - myRange.Start());
			return cycledTime;
			break;
	}
	// just to avoid warnings
	return G_MIN_REAL;
}

// *********************************************************************
//                             GAnimElement
// *********************************************************************

inline bool PropertyNameLE(const GProperty *p1, const GProperty *p2) {

	if (p1->UpperName() <= p2->UpperName())
		return G_TRUE;
	return G_FALSE;
}

// delete and empty all properties
void GAnimElement::DeleteProperties(void) {

	GDynArray<GProperty *>::iterator it = gProperties.begin();

	for (; it != gProperties.end(); ++it) {
		GProperty *p = *it;
		G_ASSERT(p != NULL);
		delete p;
	}
	gProperties.clear();
}

// clone properties
GError GAnimElement::CloneProperties(const GAnimElement& Source) {

	const GProperty *srcProp;
	GProperty *tmpProp;
	GBool alreadyExists;
	GUInt32 i;
	GError err;

	G_ASSERT(this != &Source);

	GDynArray<GProperty *>::const_iterator it = Source.gProperties.begin();
	
	// clone every property
	for (; it != Source.gProperties.end(); ++it) {
		srcProp = *it;
		G_ASSERT(srcProp != NULL);

		tmpProp = AddProperty(srcProp->Name(), srcProp->ClassID(), alreadyExists, i);
		if (tmpProp) {
			// lets do copy
			err = tmpProp->CopyFrom(*srcProp);
			if (err != G_NO_ERROR)
				RemoveProperty(i);
		}
		else
			G_ASSERT(tmpProp != NULL);
	}
	return G_NO_ERROR;
}

// base clone function
GError GAnimElement::BaseClone(const GElement& Source) {

	GError err;
	const GAnimElement& tmpElem = (const GAnimElement &)Source;

	DeleteProperties();
	err = CloneProperties(tmpElem);
	if (err == G_NO_ERROR)
		err = GElement::BaseClone(Source);
	return err;
}

// find a property given its name
GProperty* GAnimElement::FindProperty(const GString& Name, GUInt32& PropIndex) const {

	GProperty tmpProp;
	GDynArray<GProperty *>::const_iterator it;

	// do a binary search
	tmpProp.SetName(Name);
	it = std::lower_bound(gProperties.begin(), gProperties.end(), &tmpProp, PropertyNameLE);
	if (it == gProperties.end())
		return NULL;
	
	if (tmpProp.UpperName() == (*it)->UpperName()) {
		PropIndex = (GUInt32)(it - gProperties.begin());
		return (*it);
	}
	else {
		PropIndex = (GUInt32)(it - gProperties.begin());
		return NULL;
	}
}

// add a property
GProperty* GAnimElement::AddProperty(const GString& Name, const GClassID& ClassID, GBool& AlreadyExist,
									 GUInt32& PropertyIndex) {

	GProperty* tmpProp;

	// first try to see if the property already exists
	tmpProp = FindProperty(Name, PropertyIndex);
	if (tmpProp)
		AlreadyExist = G_TRUE;
	else
		AlreadyExist = G_FALSE;

	// if the property does not exist, lets create a new one
	if (!AlreadyExist) {
		tmpProp = (GProperty *)CreateNew(ClassID);
		// if creation has succeed, set property name and push it into internal list
		if (tmpProp) {
			tmpProp->SetName(Name);
			// insert the property maintaining the order (by name, case-insensitive)
			GDynArray<GProperty *>::iterator it = gProperties.begin();
			it += PropertyIndex;
			gProperties.insert(it, tmpProp);
		}
	}
	return tmpProp;
}

// Get an existing property (given its name)
GProperty* GAnimElement::Property(const GString& Name) const {

	GUInt32 i;

	if (Name.length() <= 0)
		return NULL;
	// do a binary search
	return FindProperty(Name, i);
}

// get an existing property (given its index)
GProperty* GAnimElement::Property(const GUInt32 Index) const {
	
	if (Index >= (GUInt32)gProperties.size())
		return NULL;
	return gProperties[Index];
}

// remove the specified property form internal list
GBool GAnimElement::RemoveProperty(const GString& Name) {

	if (Name.length() <= 0)
		return G_FALSE;

	GUInt32 propIndex;
	GProperty *tmpProp;

	tmpProp = FindProperty(Name, propIndex);
	return RemoveProperty(propIndex);
}

// remove the specified property form internal list
GBool GAnimElement::RemoveProperty(const GUInt32 Index) {

	if (Index >= (GUInt32)gProperties.size())
		return G_FALSE;

	GDynArray<GProperty *>::iterator it = gProperties.begin();
	GProperty *tmpProp;
	
	it += Index;
	tmpProp = *it;
	gProperties.erase(it);
	G_ASSERT(tmpProp != NULL);
	delete tmpProp;
	return G_TRUE;
}


}
