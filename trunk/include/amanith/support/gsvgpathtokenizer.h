/****************************************************************************
** $file: amanith/include/support/gsvgpathtokenizer.cpp   0.3.0.0   edited Jan, 30 2006
**
** SVG path tokenizer class definition.
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

#ifndef GSVGPATHTOKENIZER_H
#define GSVGPATHTOKENIZER_H

#include "amanith/gglobal.h"

/*!
	\file gsvgpathtokenizer.h
	\brief SVG path tokenizer header file.
*/
namespace Amanith {

    class G_EXPORT GSVGPathTokenizer {

	private:
		GChar8 gSeparatorsMask[256/8];
		GChar8 gCommandsMask[256/8];
		GChar8 gNumericMask[256/8];
		const GChar8* gCurrentPath;
		GReal gLastNumber;
		GChar8 gLastCommand;
		GBool gLastFlag;

		// initializes a character mask
		static void InitCharMask(GChar8* Mask, const GChar8* CharSet);
		// tells if specified character is contained into specified mask
		inline GBool Contains(const GChar8* Mask, unsigned c) const {
			return (Mask[(c >> 3) & (256 / 8 - 1)] & (1 << (c & 7))) != 0;
		}
		// return G_TRUE if specified character is a command
		GBool IsCmd(unsigned c) const;
		// return G_TRUE if specified character is a number or numeric sign
		GBool IsNum(unsigned c) const;
		// return G_TRUE if specified character is a separator
		GBool IsSep(unsigned c) const;
		// set internal path description
		void SetPathString(const GChar8* String);
		// parse number
		GBool ParseNumber();

     public:
		// default constructor
        GSVGPathTokenizer();
		// set constructor
		GSVGPathTokenizer(const GChar8* PathDesc);
		// set constructor
		GSVGPathTokenizer(const GString& PathDesc);
		// get next token
        GBool NextTkn();
		// get next token (as a number)
        GReal NextTknAsReal(GChar8 Command);
		// get next token (as a bool)
		GBool NextTknAsBool(GChar8 Command);
		// get last parsed command
        GChar8 LastCmd() const {
			return gLastCommand;
		}
		// get last parsed number
        GReal LastNum() const {
			return gLastNumber;
		}
		// get last parsed boolean flag
		GBool LastFlg() const {
			return gLastFlag;
		}
    };

};	// end namespace Amanith

#endif
