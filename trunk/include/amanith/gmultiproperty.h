/****************************************************************************
** $file: amanith/gmultiproperty.h   0.2.0.0   edited Dec, 12 2005
**
** Basic animated multi-properties definition.
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

#ifndef GMULTIPROPERTY_H
#define GMULTIPROPERTY_H

#include "amanith/gproperty.h"


/*!
	\file gmultiproperty.h
	\brief Header file for common multi properties.
*/

namespace Amanith {

	// *********************************************************************
	//                         GMultiProperty1D
	// *********************************************************************

	//! GMultiProperty1D static class descriptor.
	static const GClassID G_MULTIPROPERTY1D_CLASSID = GClassID("GMultiProperty1D", 0xB654D382, 0x60B34BCD, 0xBB5159A5, 0x4816A30A);

	/*!
		\class GMultiProperty1D
		\brief This class represent a multi-property, intended as a container of properties of the same type.
	*/
	class G_EXPORT GMultiProperty1D : public GProperty {

	private:
		GClassID gSubPropertiesType;

	protected:
		//! Cloning function, copies (physically) a Source GMultiProperty1D class.
		GError BaseClone(const GElement& Source);
		/*!
			Construct a new properties sub set, using specified labels and sub-properties type.

			\param SubPropertiesCount number of sub-properties to create. It must be positive (greater than 0).
			\param SubPropertiesType type descriptor of sub-properties.
			\param DefaultValue the default value associated with each property. If an invalid value is
			specified (it is not compatible with the value type handled by the SubPropertiesType ClassID), a
			0(s) default value will be used.
			\param Labels a list of semicolon separated sub-properties labels. If empty, the a default schema is
			used; in particular labels will be a sequential natural numbers ("0", "1", "2", ...).
			\note the number of labels must	match SubPropertiesCount parameter, elsewhere the default label
			schema will be used.
		*/
		GError Init(const GUInt32 SubPropertiesCount, const GClassID& SubPropertiesType, 
					const GKeyValue& DefaultValue, const GString& Labels = "");

	public:
		//! Default constructor, set owner to NULL.
		GMultiProperty1D();
		//! Constructor, specifying owner.
		GMultiProperty1D(const GElement* Owner);
		//! Destructor
		~GMultiProperty1D();
		//! Get type description of contained properties
		inline const GClassID& SubPropertiesType() const {
			return gSubPropertiesType;
		}
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_PROPERTY_CLASSID;
		}
	};

	// *********************************************************************
	//                         GMultiProperty1DProxy
	// *********************************************************************
	/*!
		\class GMultiProperty1DProxy
		\brief This class implements a GMultiProperty1D proxy (provider).

		This proxy does not override CreateNew() method because we don't wanna make a creation of a GMultiProperty1D
		class possible. This is because Init method, that must be called in constructors of derived classes (see,
		for example, GTwoHermiteProperty1D class).
	*/
	class G_EXPORT GMultiProperty1DProxy : public GElementProxy {
	public:
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_PROPERTY_CLASSID;
		}
	};
	//! Static proxy for GMultiProperty1D class.
	static const GMultiProperty1DProxy G_MULTIPROPERTY1D_PROXY;



	// *********************************************************************
	//                       GTwoHermiteProperty1D
	// *********************************************************************

	//! GTwoHermiteProperty1D static class descriptor.
	static const GClassID G_TWOHERMITEPROPERTY1D_CLASSID = GClassID("GTwoHermiteProperty1D", 0xD6BD3203, 0xCA9D4A7A, 0xA3C6510B, 0xF6BEB7D1);

	/*!
		\class GTwoHermiteProperty1D
		\brief This class represent a 2-dimensional Hermite multi-property, a container of two
		GHermiteProperty1D properties. They are accessible with the default names "x" and "y".
	*/
	class G_EXPORT GTwoHermiteProperty1D : public GMultiProperty1D {

	public:
		//! Default constructor, set owner to NULL.
		GTwoHermiteProperty1D();
		//! Constructor, specifying owner.
		GTwoHermiteProperty1D(const GElement* Owner);
		//! Destructor
		~GTwoHermiteProperty1D();
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_TWOHERMITEPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};

	// *********************************************************************
	//                       GTwoHermiteProperty1DProxy
	// *********************************************************************
	/*!
		\class GTwoHermiteProperty1DProxy
		\brief This class implements a GTwoHermiteProperty proxy (provider).

		This proxy provides the creation of GTwoHermiteProperty1D class instances.
	*/
	class G_EXPORT GTwoHermiteProperty1DProxy : public GElementProxy {
	public:
		//! Creates a new GTwoHermiteProperty1D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GTwoHermiteProperty1D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_TWOHERMITEPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};
	//! Static proxy for GTwoHermiteProperty1D class.
	static const GTwoHermiteProperty1DProxy G_TWOHERMITEPROPERTY1D_PROXY;


	// *********************************************************************
	//                       GThreeHermiteProperty1D
	// *********************************************************************

	//! GThreeHermiteProperty1D static class descriptor.
	static const GClassID G_THREEHERMITEPROPERTY1D_CLASSID = GClassID("GThreeHermiteProperty1D", 0x0754D2C8, 0x6C9A4CCE, 0x86C6A732, 0x9977F96E);

	/*!
		\class GThreeHermiteProperty1D
		\brief This class represent a 3-dimensional Hermite multi-property, a container of three
		GHermiteProperty1D properties. They are accessible with the default names "x", "y" and "z".
	*/
	class G_EXPORT GThreeHermiteProperty1D : public GMultiProperty1D {

	public:
		//! Default constructor, set owner to NULL.
		GThreeHermiteProperty1D();
		//! Constructor, specifying owner.
		GThreeHermiteProperty1D(const GElement* Owner);
		//! Destructor
		~GThreeHermiteProperty1D();
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_THREEHERMITEPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};

	// *********************************************************************
	//                       GThreeHermiteProperty1DProxy
	// *********************************************************************
	/*!
		\class GThreeHermiteProperty1DProxy
		\brief This class implements a GThreeHermiteProperty proxy (provider).

		This proxy provides the creation of GThreeHermiteProperty1D class instances.
	*/
	class G_EXPORT GThreeHermiteProperty1DProxy : public GElementProxy {
	public:
		//! Creates a new GThreeHermiteProperty1D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GThreeHermiteProperty1D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_THREEHERMITEPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};
	//! Static proxy for GThreeHermiteProperty1D class.
	static const GThreeHermiteProperty1DProxy G_THREEHERMITEPROPERTY1D_PROXY;


	// *********************************************************************
	//                       GFourHermiteProperty1D
	// *********************************************************************

	//! GFourHermiteProperty1D static class descriptor.
	static const GClassID G_FOURHERMITEPROPERTY1D_CLASSID = GClassID("GFourHermiteProperty1D", 0x717390B9, 0xA5774A37, 0x87DACC89, 0xBDDC1627);

	/*!
		\class GFourHermiteProperty1D
		\brief This class represent a 4-dimensional Hermite multi-property, a container of four
		GHermiteProperty1D properties. They are accessible with the default names "x", "y", "z" and "w".
	*/
	class G_EXPORT GFourHermiteProperty1D : public GMultiProperty1D {

	public:
		//! Default constructor, set owner to NULL.
		GFourHermiteProperty1D();
		//! Constructor, specifying owner.
		GFourHermiteProperty1D(const GElement* Owner);
		//! Destructor
		~GFourHermiteProperty1D();
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_FOURHERMITEPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};

	// *********************************************************************
	//                       GFourHermiteProperty1DProxy
	// *********************************************************************
	/*!
		\class GFourHermiteProperty1DProxy
		\brief This class implements a GFourHermiteProperty proxy (provider).

		This proxy provides the creation of GFourHermiteProperty1D class instances.
	*/
	class G_EXPORT GFourHermiteProperty1DProxy : public GElementProxy {
	public:
		//! Creates a new GFourHermiteProperty1D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GFourHermiteProperty1D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_FOURHERMITEPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};
	//! Static proxy for GFourHermiteProperty1D class.
	static const GFourHermiteProperty1DProxy G_FOURHERMITEPROPERTY1D_PROXY;



	// *********************************************************************
	//                       GTwoLinearProperty1D
	// *********************************************************************

	//! GTwoLinearProperty1D static class descriptor.
	static const GClassID G_TWOLINEARPROPERTY1D_CLASSID = GClassID("GTwoLinearProperty1D", 0x3AC73EB5, 0xD7B1478B, 0x8E9FC33B, 0x02D72F0F);

	/*!
		\class GTwoLinearProperty1D
		\brief This class represent a 2-dimensional linear multi-property, a container of two
		GLinearProperty1D properties. They are accessible with the default names "x" and "y".
	*/
	class G_EXPORT GTwoLinearProperty1D : public GMultiProperty1D {

	public:
		//! Default constructor, set owner to NULL.
		GTwoLinearProperty1D();
		//! Constructor, specifying owner.
		GTwoLinearProperty1D(const GElement* Owner);
		//! Destructor
		~GTwoLinearProperty1D();
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_TWOLINEARPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};

	// *********************************************************************
	//                       GTwoLinearProperty1DProxy
	// *********************************************************************
	/*!
		\class GTwoLinearProperty1DProxy
		\brief This class implements a GTwoLinearProperty1D proxy (provider).

		This proxy provides the creation of GTwoLinearProperty1D class instances.
	*/
	class G_EXPORT GTwoLinearProperty1DProxy : public GElementProxy {
	public:
		//! Creates a new GTwoLinearProperty1D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GTwoLinearProperty1D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_TWOLINEARPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};
	//! Static proxy for GTwoLinearProperty1D class.
	static const GTwoLinearProperty1DProxy G_TWOLINEARPROPERTY1D_PROXY;


	// *********************************************************************
	//                       GThreeLinearProperty1D
	// *********************************************************************

	//! GThreeLinearProperty1D static class descriptor.
	static const GClassID G_THREELINEARPROPERTY1D_CLASSID = GClassID("GThreeLinearProperty1D", 0xE7035327, 0x6E214B81, 0xB7A242E2, 0x9F840B34);

	/*!
		\class GThreeLinearProperty1D
		\brief This class represent a 3-dimensional linear multi-property, a container of three
		GLinearProperty1D properties. They are accessible with the default names "x", "y" and "z".
	*/
	class G_EXPORT GThreeLinearProperty1D : public GMultiProperty1D {

	public:
		//! Default constructor, set owner to NULL.
		GThreeLinearProperty1D();
		//! Constructor, specifying owner.
		GThreeLinearProperty1D(const GElement* Owner);
		//! Destructor
		~GThreeLinearProperty1D();
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_THREELINEARPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};

	// *********************************************************************
	//                       GThreeLinearProperty1DProxy
	// *********************************************************************
	/*!
		\class GThreeLinearProperty1DProxy
		\brief This class implements a GThreeLinearProperty1D proxy (provider).

		This proxy provides the creation of GThreeLinearProperty1D class instances.
	*/
	class G_EXPORT GThreeLinearProperty1DProxy : public GElementProxy {
	public:
		//! Creates a new GThreeLinearProperty1D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GThreeLinearProperty1D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_THREELINEARPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};
	//! Static proxy for GThreeHermiteProperty1D class.
	static const GThreeLinearProperty1DProxy G_THREELINEARPROPERTY1D_PROXY;



	// *********************************************************************
	//                       GFourLinearProperty1D
	// *********************************************************************

	//! GFourLinearProperty1D static class descriptor.
	static const GClassID G_FOURLINEARPROPERTY1D_CLASSID = GClassID("GFourLinearProperty1D", 0x32682822, 0x0FEF483C, 0xAF0C07AC, 0x16D467EE);

	/*!
		\class GFourLinearProperty1D
		\brief This class represent a 4-dimensional linear multi-property, a container of four
		GLinearProperty1D properties. They are accessible with the default names "x", "y", "z" and "w".
	*/
	class G_EXPORT GFourLinearProperty1D : public GMultiProperty1D {

	public:
		//! Default constructor, set owner to NULL.
		GFourLinearProperty1D();
		//! Constructor, specifying owner.
		GFourLinearProperty1D(const GElement* Owner);
		//! Destructor
		~GFourLinearProperty1D();
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_FOURLINEARPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};

	// *********************************************************************
	//                       GFourLinearProperty1DProxy
	// *********************************************************************
	/*!
		\class GFourLinearProperty1DProxy
		\brief This class implements a GFourLinearProperty1D proxy (provider).

		This proxy provides the creation of GFourLinearProperty1D class instances.
	*/
	class G_EXPORT GFourLinearProperty1DProxy : public GElementProxy {
	public:
		//! Creates a new GFourLinearProperty1D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GFourLinearProperty1D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_FOURLINEARPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};
	//! Static proxy for GFourHermiteProperty1D class.
	static const GFourLinearProperty1DProxy G_FOURLINEARPROPERTY1D_PROXY;


	// *********************************************************************
	//                       GTwoConstantProperty1D
	// *********************************************************************

	//! GTwoConstantProperty1D static class descriptor.
	static const GClassID G_TWOCONSTANTPROPERTY1D_CLASSID = GClassID("GTwoConstantProperty1D", 0x8471E976, 0x817D4F43, 0x8A202AA8, 0x0DE55C4E);

	/*!
		\class GTwoConstantProperty1D
		\brief This class represent a 2-dimensional constant multi-property, a container of two
		GConstantProperty1D properties. They are accessible with the default names "x" and "y".
	*/
	class G_EXPORT GTwoConstantProperty1D : public GMultiProperty1D {

	public:
		//! Default constructor, set owner to NULL.
		GTwoConstantProperty1D();
		//! Constructor, specifying owner.
		GTwoConstantProperty1D(const GElement* Owner);
		//! Destructor
		~GTwoConstantProperty1D();
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_TWOCONSTANTPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};

	// *********************************************************************
	//                       GTwoConstantProperty1DProxy
	// *********************************************************************
	/*!
		\class GTwoConstantProperty1DProxy
		\brief This class implements a GTwoConstantProperty1D proxy (provider).

		This proxy provides the creation of GTwoConstantProperty1D class instances.
	*/
	class G_EXPORT GTwoConstantProperty1DProxy : public GElementProxy {
	public:
		//! Creates a new GTwoConstantProperty1D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GTwoConstantProperty1D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_TWOCONSTANTPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};
	//! Static proxy for GTwoConstantProperty1D class.
	static const GTwoConstantProperty1DProxy G_TWOCONSTANTPROPERTY1D_PROXY;


	// *********************************************************************
	//                       GThreeConstantProperty1D
	// *********************************************************************

	//! GThreeConstantProperty1D static class descriptor.
	static const GClassID G_THREECONSTANTPROPERTY1D_CLASSID = GClassID("GThreeConstantProperty1D", 0xF4CCF61F, 0x208D4C8B, 0x80DAA479, 0x12DEBE23);

	/*!
		\class GThreeConstantProperty1D
		\brief This class represent a 3-dimensional constant multi-property, a container of three
		GConstantProperty1D properties. They are accessible with the default names "x", "y" and "z".
	*/
	class G_EXPORT GThreeConstantProperty1D : public GMultiProperty1D {

	public:
		//! Default constructor, set owner to NULL.
		GThreeConstantProperty1D();
		//! Constructor, specifying owner.
		GThreeConstantProperty1D(const GElement* Owner);
		//! Destructor
		~GThreeConstantProperty1D();
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_THREECONSTANTPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};

	// *********************************************************************
	//                       GThreeConstantProperty1DProxy
	// *********************************************************************
	/*!
		\class GThreeConstantProperty1DProxy
		\brief This class implements a GThreeConstantProperty1D proxy (provider).

		This proxy provides the creation of GThreeConstantProperty1D class instances.
	*/
	class G_EXPORT GThreeConstantProperty1DProxy : public GElementProxy {
	public:
		//! Creates a new GThreeConstantProperty1D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GThreeConstantProperty1D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_THREECONSTANTPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};
	//! Static proxy for GThreeConstantProperty1D class.
	static const GThreeConstantProperty1DProxy G_THREECONSTANTPROPERTY1D_PROXY;


	// *********************************************************************
	//                       GFourConstantProperty1D
	// *********************************************************************

	//! GFourConstantProperty1D static class descriptor.
	static const GClassID G_FOURCONSTANTPROPERTY1D_CLASSID = GClassID("GFourConstantProperty1D", 0x13D2DDAE, 0xFF144B3F, 0x8185427B, 0xFE8ACA4B);

	/*!
		\class GFourConstantProperty1D
		\brief This class represent a 4-dimensional constant multi-property, a container of four
		GConstantProperty1D properties. They are accessible with the default names "x", "y", "z" and "w".
	*/
	class G_EXPORT GFourConstantProperty1D : public GMultiProperty1D {

	public:
		//! Default constructor, set owner to NULL.
		GFourConstantProperty1D();
		//! Constructor, specifying owner.
		GFourConstantProperty1D(const GElement* Owner);
		//! Destructor
		~GFourConstantProperty1D();
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_FOURCONSTANTPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};

	// *********************************************************************
	//                       GFourConstantProperty1DProxy
	// *********************************************************************
	/*!
		\class GFourConstantProperty1DProxy
		\brief This class implements a GFourConstantProperty1D proxy (provider).

		This proxy provides the creation of GFourConstantProperty1D class instances.
	*/
	class G_EXPORT GFourConstantProperty1DProxy : public GElementProxy {
	public:
		//! Creates a new GFourConstantProperty1D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GFourConstantProperty1D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_FOURCONSTANTPROPERTY1D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_MULTIPROPERTY1D_CLASSID;
		}
	};
	//! Static proxy for GFourConstantProperty1D class.
	static const GFourConstantProperty1DProxy G_FOURCONSTANTPROPERTY1D_PROXY;


};	// end namespace Amanith

#endif
