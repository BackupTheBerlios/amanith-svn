/****************************************************************************
** $file: amanith/plugins/fonts/gfontimpext.cpp   0.1.1.0   edited Sep 24 08:00
**
** 2D Font import/export plugin implementation.
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

#ifdef _FONTS_PLUGIN

#include "gfontsimpexp.h"
#include "amanith/2d/gfont2d.h"
#include "amanith/support/gutilities.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/internal/ftgloadr.h>

namespace Amanith {

static FT_Library gFontLibrary;
static GBool gFontLibraryInitialized = G_FALSE;

// *********************************************************************
//                              GFontsImpExp
// *********************************************************************


// constructor
GFontsImpExp::GFontsImpExp() : GImpExp() {

	AddFontsFeatures();
}

// constructor
GFontsImpExp::GFontsImpExp(const GElement* Owner) : GImpExp(Owner) {

	AddFontsFeatures();
}

// destructor
GFontsImpExp::~GFontsImpExp() {
}

// initialize FreeType library
GError GFontsImpExp::InitFTLibrary() {

	FT_Error error;

	if (gFontLibraryInitialized == G_FALSE) {
		error = FT_Init_FreeType(&gFontLibrary);
		if (error)
			return G_MEMORY_ERROR;
		gFontLibraryInitialized = G_TRUE;
	}
	return G_NO_ERROR;
}

void GFontsImpExp::AddFontsFeatures() {

#if defined(_FONTS_TRUETYPE_MODULE) || defined(_FONTS_TYPE42_MODULE)
	AddEntry(GImpExpFeature(G_FONT2D_CLASSID, "TrueType", "ttf", 1, 0, 0, 0, G_IMPEXP_READ));
#endif

#if defined(_FONTS_TYPE1_MODULE) || defined(_FONTS_CID_MODULE)
	AddEntry(GImpExpFeature(G_FONT2D_CLASSID, "Type1", "pfa", 1, 0, 0, 0, G_IMPEXP_READ));
	AddEntry(GImpExpFeature(G_FONT2D_CLASSID, "Type1", "pfb", 1, 0, 0, 0, G_IMPEXP_READ));
#endif

#ifdef _FONTS_CFF_MODULE
	AddEntry(GImpExpFeature(G_FONT2D_CLASSID, "Compact Font Format", "cff", 1, 0, 0, 0, G_IMPEXP_READ));
#endif

#ifdef _FONTS_PFR_MODULE
	AddEntry(GImpExpFeature(G_FONT2D_CLASSID, "Bitstream Portable Font Resource", "pfr", 1, 0, 0, 0, G_IMPEXP_READ));
#endif

#ifdef _FONTS_MAC_MODULE
	AddEntry(GImpExpFeature(G_FONT2D_CLASSID, "DataFork TrtueType", "dfont", 1, 0, 0, 0, G_IMPEXP_READ));
#endif
}

// load and store charsmaps
static void LoadCharsMaps(GFont2D& Font, const FT_Face Face) {

	GUInt32 i, numMaps;
	FT_ULong charCode;
	FT_UInt glyphIndex;
	FT_Error err;
	FT_CharMap *maps;
	FT_CharMap map;
	GEncodedChar encodedChar;
	GCharMap tmpMap;
	
	numMaps = Face->num_charmaps;
	maps = Face->charmaps;

	for (i = 0; i < numMaps; i++) {
		map = maps[i];
		err = FT_Set_Charmap(Face, map);
		if (err == 0) {
			// get first (valid) char code
			charCode = FT_Get_First_Char(Face, &glyphIndex);
			// if glyphIndex is 0, it means that charmap is empty
			if (glyphIndex != 0) {
				tmpMap.CharMap.clear();
				// store encoding informations
				tmpMap.PlatformID = map->platform_id;
				tmpMap.EncodingID = map->encoding_id;
				switch (map->encoding) {
					case FT_ENCODING_MS_SYMBOL: tmpMap.Encoding = G_ENCODING_MS_SYMBOL;
					case FT_ENCODING_UNICODE: tmpMap.Encoding = G_ENCODING_UNICODE;
					case FT_ENCODING_SJIS: tmpMap.Encoding = G_ENCODING_SJIS;
					case FT_ENCODING_GB2312: tmpMap.Encoding = G_ENCODING_GB2312;
					case FT_ENCODING_BIG5: tmpMap.Encoding = G_ENCODING_BIG5;
					case FT_ENCODING_WANSUNG: tmpMap.Encoding = G_ENCODING_WANSUNG;
					case FT_ENCODING_JOHAB: tmpMap.Encoding = G_ENCODING_JOHAB;
					case FT_ENCODING_ADOBE_STANDARD: tmpMap.Encoding = G_ENCODING_ADOBE_STANDARD;
					case FT_ENCODING_ADOBE_EXPERT: tmpMap.Encoding = G_ENCODING_ADOBE_EXPERT;
					case FT_ENCODING_ADOBE_CUSTOM: tmpMap.Encoding = G_ENCODING_ADOBE_CUSTOM;
					case FT_ENCODING_ADOBE_LATIN_1: tmpMap.Encoding = G_ENCODING_ADOBE_LATIN_1;
					case FT_ENCODING_OLD_LATIN_2: tmpMap.Encoding = G_ENCODING_OLD_LATIN_2;
					case FT_ENCODING_APPLE_ROMAN: tmpMap.Encoding = G_ENCODING_APPLE_ROMAN;
					default:
						tmpMap.Encoding = G_ENCODING_NONE;
				}
				while (glyphIndex != 0) {

					encodedChar.CharCode = (GUInt32)charCode;
					encodedChar.GlyphIndex = (GUInt32)(glyphIndex - 0);
					tmpMap.CharMap.push_back(encodedChar);
					charCode = FT_Get_Next_Char(Face, charCode, &glyphIndex);
				}
				// add the loaded charmap to the font
				Font.AddCharMap(tmpMap);
			}
		}
	}
}

// load kerning informations
static void LoadKerning(GFont2D& Font, const FT_Face Face, const GReal Scale, const GString& MetricsFile) {

	FT_Vector v;
	FT_Error err;
	GInt32 i, j, k;
	GKerningEntry entry;
	GDynArray<GKerningEntry> kerningTable;

	if (MetricsFile.length() > 0)
		err = FT_Attach_File(Face, StrUtils::ToAscii(MetricsFile));

	// if the face has not kerning infos just return
	if ((Face->face_flags & FT_FACE_FLAG_KERNING) == 0)
		return;

	j = (GInt32)Face->num_glyphs;
	for	(i = 0; i < j; i++) {
		for (k = 0; k < j; k++) {
			// get kerning for pair (i, k)
			err = FT_Get_Kerning(Face, i, k, FT_KERNING_UNSCALED, &v);
			if ((err == 0) && ((v.x != 0) || (v.y != 0))) {
				entry.GlyphIndexLeft = i;
				entry.GlyphIndexRight = k;
				entry.Kerning.Set((GReal)v.x * Scale, (GReal)v.y * Scale);
				kerningTable.push_back(entry);
			}
		}
	}
	Font.SetKerning(kerningTable);
}

// load a single glyph
static void LoadGlyph(GFont2D& Font, const FT_Face Face, const GInt32 GlyphIndex, const GReal Scale) {

	GInt32 loadFlags;
	GFontChar2D *c;
	GGlyphMetrics tmpMetrics;
	FT_Error error;
	GInt32 i, j, k0, k1, numVert;
	FT_SubGlyphRec subGlyph;
	GPoint2 p;
	GReal x, y;
	GVect<GReal, 2> v;

	#define Fixed1616ToReal(a) ((GReal)a / (GReal)65536)

	// we ignore transformation, scaling and we specify to not recursively load subglyphs
	loadFlags = FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP | FT_LOAD_IGNORE_TRANSFORM |
				FT_LOAD_NO_AUTOHINT | FT_LOAD_NO_RECURSE | FT_LOAD_LINEAR_DESIGN;
	error = FT_Load_Glyph(Face, GlyphIndex, loadFlags);
	if (error)
		return;

	// check for a good vector format
	if ((Face->glyph->format != FT_GLYPH_FORMAT_OUTLINE) && (Face->glyph->format != FT_GLYPH_FORMAT_PLOTTER) &&
		(Face->glyph->format != FT_GLYPH_FORMAT_COMPOSITE))
		return;

	// composite glyph, we have to load subglyphs
	if (Face->glyph->num_subglyphs > 0) {

		GDynArray<GSubChar2D> subChars(Face->glyph->num_subglyphs);

		for (i = 0; i < (GInt32)Face->glyph->num_subglyphs; i++) {
			subGlyph = Face->glyph->subglyphs[i];
			subChars[i].GlyphIndex = subGlyph.index;
			subChars[i].Flags = subGlyph.flags;
			// rotation and scale
			subChars[i].Transformation[G_X][G_X] = Fixed1616ToReal(subGlyph.transform.xx);
			subChars[i].Transformation[G_X][G_Y] = Fixed1616ToReal(subGlyph.transform.xy);
			// x position
			subChars[i].Transformation[G_X][G_Z] = subGlyph.arg1 * Scale;
			// rotation and scale
			subChars[i].Transformation[G_Y][G_X] = Fixed1616ToReal(subGlyph.transform.yx);
			subChars[i].Transformation[G_Y][G_Y] = Fixed1616ToReal(subGlyph.transform.yy);
			// y position
			subChars[i].Transformation[G_Y][G_Z] = subGlyph.arg2 * Scale;
			// last row is an identity
			subChars[i].Transformation[G_Z][G_X] = 0;
			subChars[i].Transformation[G_Z][G_Y] = 0;
			subChars[i].Transformation[G_Z][G_Z] = 1;
		}
		c = Font.AddChar(subChars);
	}
	// simple glyph, lets load contours
	else {
		GDynArray<GPoint2> tmpPoints;
		GDynArray<GInt32> tmpPointsFlags;
		GDynArray<GFontCharContour2D> tmpContours;

		k0 = 0;
		for (i = 0; i < Face->glyph->outline.n_contours; i++) {
			k1 = Face->glyph->outline.contours[i];
			// take into account number of vertices
			numVert = k1 - k0 + 1;
			// we discard a bad contour
			if (numVert < 3)
				continue;
			tmpPoints.resize(numVert);
			tmpPointsFlags.resize(numVert);
			// read i-th segment
			for (j = k0; j <= k1; j++) {
				x = (GReal)Face->glyph->outline.points[j].x;
				y = (GReal)Face->glyph->outline.points[j].y;
				tmpPoints[j - k0].Set(x * Scale, y * Scale);
				tmpPointsFlags[j - k0] = Face->glyph->outline.tags[j];
			}
			// create new contour class
			GFontCharContour2D newContour(tmpPoints, tmpPointsFlags);
			// if there was some 0-length segments, they are not inserted; if at the end we have
			// a bad contour, we can discard it
			if (newContour.PointsCount() >= 3)
				tmpContours.push_back(newContour);
			// next segment
			k0 = k1 + 1;
		}
		c = Font.AddChar(tmpContours);
	}

	if (c) {
		// copy metrics
		tmpMetrics.Height = (GReal)Face->glyph->metrics.height * Scale;
		tmpMetrics.Width = (GReal)Face->glyph->metrics.width * Scale;
		tmpMetrics.HoriAdvance = (GReal)Face->glyph->metrics.horiAdvance * Scale;
		tmpMetrics.HoriBearingX = (GReal)Face->glyph->metrics.horiBearingX * Scale;
		tmpMetrics.HoriBearingY = (GReal)Face->glyph->metrics.horiBearingY * Scale;
		tmpMetrics.VertAdvance = (GReal)Face->glyph->metrics.vertAdvance * Scale;
		tmpMetrics.VertBearingX = (GReal)Face->glyph->metrics.vertBearingX * Scale;
		tmpMetrics.VertBearingY = (GReal)Face->glyph->metrics.vertBearingY * Scale;
		c->SetMetrics(tmpMetrics);
		// load some basic informations
		c->SetLinearHoriAdvance((GReal)Face->glyph->linearHoriAdvance * Scale);
		c->SetLinearVertAdvance((GReal)Face->glyph->linearVertAdvance * Scale);
		v.Set((GReal)Face->glyph->advance.x * Scale, (GReal)Face->glyph->advance.y * Scale);
		c->SetAdvance(v);
		c->SetLSBDelta((GReal)Face->glyph->lsb_delta * Scale);
		c->SetRSBDelta((GReal)Face->glyph->rsb_delta * Scale);
		c->SetEvenOddFill((Face->glyph->outline.flags & FT_OUTLINE_EVEN_ODD_FILL) != 0);
	}
}

// load all glyphs
static void LoadGlyphs(GFont2D& Font, const FT_Face Face, const GReal Scale) {

	GInt32 i, j;

	j = (GInt32)Face->num_glyphs;
	for (i = 0; i < j; i++)
		LoadGlyph(Font, Face, i, Scale);
}

// load a font file
GError GFontsImpExp::ReadFont(const GChar8 *FullFileName, const GElement& Element, const GBool OuterCCW,
							  const GString& MetricsFile, const GReal Scale) {

	FT_Face face = NULL;
	FT_Error error;
	GReal s;
	GFont2D& font = (GFont2D&)Element;

	// just to avoid warnings
	if (OuterCCW)
		face = NULL;

	error = FT_New_Face(gFontLibrary, FullFileName, 0, &face);
	if (error == FT_Err_Unknown_File_Format)
		return G_INVALID_FORMAT;
    else
	if (error)
		return G_READ_ERROR;

	// check if this a good vector font (is not a bitmap font)
	if ((face->face_flags & FT_FACE_FLAG_SCALABLE) == 0) {
		error = FT_Done_Face(face);
		return G_INVALID_FORMAT;
	}

	font.Clear();
	font.SetFamilyName(face->family_name);
	font.SetStyleName(face->style_name);
	font.SetUnitsPerEM(face->units_per_EM);
	// calculate the scale factor; a value of 0 indicates a normalization to square EM [0;1]
	if (Scale == 0)
		s = (GReal)1 / (GReal)face->units_per_EM;
	else
		s = Scale;

	font.SetAscender((GReal)face->ascender * s);
	font.SetDescender((GReal)face->descender * s);
	// we extract external leading, derived from height, ascender and descender
	font.SetExternalLeading((GReal)face->height * s - font.Ascender() - font.Descender());
	font.SetMaxAdvanceWidth((GReal)face->max_advance_width * s);
	font.SetMaxAdvanceHeight((GReal)face->max_advance_height * s);
	font.SetUnderlinePosition((GReal)face->underline_position * s);
	font.SetUnderlineThickness((GReal)face->underline_thickness * s);
	// style flags
	font.SetItalic((face->style_flags & FT_STYLE_FLAG_ITALIC) != 0);
	font.SetBold((face->style_flags & FT_STYLE_FLAG_BOLD) != 0);
	// load and store charmaps
	LoadCharsMaps(font, face);
	// load kerning informations
	LoadKerning(font, face, s, MetricsFile);
	// load all glyphs
	LoadGlyphs(font, face, s);
	// free freetype structures and exit
	error = FT_Done_Face(face);
	return G_NO_ERROR;
}

GError GFontsImpExp::DoRead(const GChar8 *FullFileName, GElement& Element, const GDynArray<GImpExpOption>& ParsedOptions) {

	GString metricsFile;
	GBool outerCCW = G_TRUE;
	GReal scale = 1; // a value of 0 indicates a normalization to square EM [0;1]

	// check each option
	GDynArray<GImpExpOption>::const_iterator it = ParsedOptions.begin();
	for (; it != ParsedOptions.end(); ++it) {
		if (StrUtils::SameText(it->OptionName, "outerccw")) {
			if (StrUtils::SameText(it->OptionValue, "false"))
				outerCCW = G_FALSE;
		}
		else
		if (StrUtils::SameText(it->OptionName, "metricsfile"))
			metricsFile = it->OptionValue;
		else
		if (StrUtils::SameText(it->OptionName, "scale"))
			scale = (GReal)StrUtils::ToDouble(it->OptionValue);
	}
	// init freetype2 library
	InitFTLibrary();
	// read the font
	GError err = ReadFont(FullFileName, Element, outerCCW, metricsFile, scale);
	// destroy freetype2 library
	FT_Done_FreeType(gFontLibrary);
	gFontLibraryInitialized = G_FALSE;
	return err;
}

GError GFontsImpExp::DoWrite(const GChar8 *FullFileName, const GElement& Element,
							 const GDynArray<GImpExpOption>& ParsedOptions) {

	// just to avoid warnings
	if (!FullFileName || ParsedOptions.size() > 0 || Element.Owner())
		return G_MISSED_FEATURE;
	else
		return G_MISSED_FEATURE;
}

// export interface functions
#ifdef _FONTS_PLUGIN_EXTERNAL
G_EXTERN_C G_PLUGIN_EXPORT GUInt32 ProxiesCount() {
	return 1;
}
G_EXTERN_C G_PLUGIN_EXPORT const GElementProxy* ProxyInstance(const GUInt32 Index) {
	if (Index == 0)
		return &G_FONTSIMPEXP_PROXY;
	return NULL;
}
#endif

}

#endif
