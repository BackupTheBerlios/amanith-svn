/****************************************************************************
** $file: amanith/src/support/gsvgpathtokenizer.cpp   0.2.0.0   edited Dec, 12 2005
**
** SVG path tokenizer implementation.
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

#include "amanith/support/gsvgpathtokenizer.h"
#include "amanith/support/gutilities.h"

/*!
	\file gsvgpathtokenizer.cpp
	\brief SVG path tokenizer implementation file.
*/

namespace Amanith {

//------------------------------------------------------------------------
static const char gCommands[]   = "+-MmZzLlHhVvCcSsQqTtAaFfPp";
static const char gNumeric[]    = ".Ee0123456789";
static const char gSeparators[] = " ,\t\n\r";

// default constructor
GSVGPathTokenizer::GSVGPathTokenizer() : gCurrentPath(0), gLastNumber(0), gLastCommand(0) {

	InitCharMask(gCommandsMask, gCommands);
	InitCharMask(gNumericMask, gNumeric);
	InitCharMask(gSeparatorsMask, gSeparators);
}

void GSVGPathTokenizer::SetPathString(const GChar8* String) {

	gCurrentPath = String;
	gLastCommand = 0;
	gLastNumber = 0;
	gLastFlag = G_FALSE;
}

// set constructor
GSVGPathTokenizer::GSVGPathTokenizer(const GChar8* PathDesc) : gCurrentPath(0), gLastNumber(0), gLastCommand(0) {

	InitCharMask(gCommandsMask, gCommands);
	InitCharMask(gNumericMask, gNumeric);
	InitCharMask(gSeparatorsMask, gSeparators);
	SetPathString(PathDesc);
}

// set constructor
GSVGPathTokenizer::GSVGPathTokenizer(const GString& PathDesc) : gCurrentPath(0), gLastNumber(0), gLastCommand(0) {

	InitCharMask(gCommandsMask, gCommands);
	InitCharMask(gNumericMask, gNumeric);
	InitCharMask(gSeparatorsMask, gSeparators);
	SetPathString(StrUtils::ToAscii(PathDesc));
}

GBool GSVGPathTokenizer::IsCmd(unsigned c) const {
	return Contains(gCommandsMask, c);
}

GBool GSVGPathTokenizer::IsNum(unsigned c) const {
	return Contains(gNumericMask, c);
}

GBool GSVGPathTokenizer::IsSep(unsigned c) const {
	return Contains(gSeparatorsMask, c);
}

void GSVGPathTokenizer::InitCharMask(GChar8* Mask, const GChar8* CharSet) {

	std::memset(Mask, 0, 256 / 8);

    while (*CharSet) {

        unsigned c = unsigned(*CharSet++) & 0xFF;
        Mask[c >> 3] |= 1 << (c & 7);
    }
}


GBool GSVGPathTokenizer::NextTkn() {

    if (gCurrentPath == NULL)
		return G_FALSE;

    // skip all white spaces and other garbage
    while (*gCurrentPath && !IsCmd(*gCurrentPath) && !IsNum(*gCurrentPath)) {

        if (!IsSep(*gCurrentPath)) {
			G_DEBUG("SVG path tokenizer: invalid character, it will be skipped");
            /*char buf[100];
            sprintf(buf, "path_tokenizer::next : Invalid Character %c", *gCurrentPath);
            throw exception(buf);*/
        }
        gCurrentPath++;
    }

    if (*gCurrentPath == 0)
		return G_FALSE;

    if (IsCmd(*gCurrentPath)) {

        // check if the command is a numeric sign character
        if (*gCurrentPath == '-' || *gCurrentPath == '+')
            return ParseNumber();

        gLastCommand = *gCurrentPath++;
        while (*gCurrentPath && IsSep(*gCurrentPath))
			gCurrentPath++;

        if (*gCurrentPath == 0)
			return G_TRUE;
    }
    return ParseNumber();
}


// get next token (as a number)
GReal GSVGPathTokenizer::NextTknAsReal(GChar8 Command) {

	if (!NextTkn()) {
		//throw exception("parse_path: Unexpected end of path");
		G_DEBUG("SVG path tokenizer: unexpected end of path");
		return 0;
	}

    if (LastCmd() != Command) {

		G_DEBUG("SVG path tokenizer: invalid or missing parameters");
        /*char buf[100];
        sprintf(buf, "parse_path: Command %c: bad or missing parameters", cmd);
        throw exception(buf);*/
    }
    return LastNum();
}

// get next token (as a bool)
GBool GSVGPathTokenizer::NextTknAsBool(GChar8 Command) {

	if (!NextTkn()) {
		//throw exception("parse_path: Unexpected end of path");
		G_DEBUG("SVG path tokenizer: unexpected end of path");
			return G_TRUE;
	}

	if (LastCmd() != Command) {

		G_DEBUG("SVG path tokenizer: invalid or missing parameters");
		/*char buf[100];
		sprintf(buf, "parse_path: Command %c: bad or missing parameters", cmd);
		throw exception(buf);*/
	}
	if (LastNum() != 0)
		return G_TRUE;
	else
		return G_FALSE;
}

GBool GSVGPathTokenizer::ParseNumber() {

    GChar8 buf[256]; // should be enough for any number
    GChar8* buf_ptr = buf;

    // copy all sign characters
    while (buf_ptr < buf + 255 && *gCurrentPath == '-' || *gCurrentPath == '+')
        *buf_ptr++ = *gCurrentPath++;

    // copy all numeric characters
    while (buf_ptr < buf + 255 && IsNum(*gCurrentPath))
        *buf_ptr++ = *gCurrentPath++;

    *buf_ptr = 0;
	gLastNumber = (GReal)StrUtils::ToDouble(buf); //atof(buf);
    return G_TRUE;
}

};  // end namespace Amanith
