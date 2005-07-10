/****************************************************************************
** $file: amanith/2d/gfont.h   0.1.0.0   edited Jun 30 08:00
**
** 2D Font definition.
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

#ifndef GFONT2D_H
#define GFONT2D_H

#include "amanith/gelement.h"
#include "amanith/2d/gbeziercurve2d.h"
#include "amanith/2d/gpath2d.h"
#include "amanith/geometry/gaabox.h"
#include "amanith/geometry/gmatrix.h"

/*!
	\file gfont2d.h
	\brief Font engine header file.
*/
namespace Amanith {

	// *********************************************************************
	//                           GFontCharContour2D
	// *********************************************************************

	/*!
		\class GFontCharContour2D
		\brief This class represents a character closed contour.

		Internally a contour is compactly described by its "main" points and a sequence of flags, associated with main
		points, that clarify each point "degree". A such contour can be flattened using the well know maximum
		deviation schema.\n.
		If one would pass to a classic piecewise Bezier form, just call DecomposeBezier() function. As said before, internal
		representation is more compact, and requires less memory.
		Each point flag has the following format (there's a 1-1 correspondence between points and flags):

		- if bit 0 is unset, the point is 'off' the curve, i.e. a Bezier control point, while it is 'on' when set.
		- bit 1 is meaningful for 'off' points only. If set, it indicates a third-order Bezier arc control point; and
		a second-order control point if unset.

		Here's an ascii art taken from the official FreeType library:
\code

Legend:  O on curve  + off curve
                                        +

                                     __---__
        O-__                      _--       -_
            --__                _-            -
                --__           O               \
                    --__                        O
                        -O
                                 Two `on' points
         Two `on' points       and one `off' point
                                  between them

                      +
        O            __      Two 'on' points with two 'off'
         \          -  -     points between them. The point
          \        /    \    marked '@' is the middle of the
           -      @      \   'off' points, and is a 'virtual
            -_  _-       O   on' point where the curve passes.
              --             It does not appear in the point
              +              list.

\endcode

	*/
	class G_EXPORT GFontCharContour2D {

		friend class GFontChar2D;

	private:
		//! Main points array
		mutable GDynArray<GPoint2> gPoints;
		//! Points flags.
		mutable GDynArray<GInt32> gPointsFlags;
		//! Axes aligned bounding box for this contour.
		GAABox2 gBoundBox;
		//! Flag that specifies if this contours is an hole (G_TRUE value) or a solid one (G_FALSE value).
		mutable GBool gIsHole;

	protected:
		//! Clear the contour (remove degenerative segments, repeated points and so on).
		void BuildGoodContour(const GReal Precision, const GDynArray<GPoint2>& NewPoints,
							  const GDynArray<GInt32>& PointsFlags);
		//! Reverse internal points and flags array.
		void MirrorPoints() const;
		//! This static members is the one that does the effective tessellation (flattening).
		static void DrawContour(const GDynArray<GPoint2>& ContourPoints, const GDynArray<GInt32>& ContourFlags,
								GDynArray<GPoint2>& OutPoints, const GReal Variation);
		// given a Bezier decomposition, find the Bezier arc (index) that has a control point at has
		// got a x component less than specified XValue
		static GInt32 FindLeftArc(const GDynArray<GPoint2>& Points, const GDynArray<GInt32>& Index,
								const GReal XValue, GUInt32& OfsPoint);

	public:
		//! Set constructor, build a new contour specifying main points and their associated flags
		GFontCharContour2D(const GDynArray<GPoint2>& NewPoints, const GDynArray<GInt32>& PointsFlags);
		//! Copy constructor.
		GFontCharContour2D(const GFontCharContour2D& Source);
		//! Destructor
		~GFontCharContour2D() {
		}
		//! Return G_TRUE if this contours is an hole, G_FALSE otherwise.
		GBool IsHole() const {
			return gIsHole;
		}
		//! Get an axes aligned bounding box for this contour.
		const GAABox2& BoundBox() const {
			return gBoundBox;
		}
		//! Get the points list
		const GDynArray<GPoint2>& Points() const {
			return gPoints;
		}
		//! Get flags array associated to points. There's a 1-1 correspondence between points and flags
		const GDynArray<GInt32>& PointsFlags() const {
			return gPointsFlags;
		}
		//! Get number of points of this contour. This method is provided for convenience.
		GInt32 PointsCount() const {
			return (GInt32)gPoints.size();
		}
		/*!
			Draw (flatten) the contour, specifying a maximum deviation (squared chordal distance)

			\param Points the output array, where generated points will be appended.
			\param Variation maximum variation permitted by this flattening operation.
		*/
		void DrawContour(GDynArray<GPoint2>& Points, const GReal Variation) const;
		/*!
			Draw (flatten) the contour, specifying a maximum deviation (squared chordal distance) and an optional
			affine transformation /used for example by multi-glyph font characters).

			\param Points the output array, where generated points will be appended.
			\param Variation maximum variation permitted by this flattening operation.
			\param Transformation the affine transformation to apply to points.
		*/
		void DrawContour(GDynArray<GPoint2>& Points, const GReal Variation,	const GMatrix23& Transformation) const;
		/*!
			Convert the internal compact contour representation, into a piecewise Bezier form.

			For example, if the contour is converted into 3 Bezier quadratics and a line, this method will append
			11 points (3 for every quadratics and 2 for the line) to Points array, and it will append the values 3, 3, 3 and
			2 to Index array.

			\param Points an output array, where control points will be added
			\param Index the output counters array.
		*/
		void DecomposeBezier(GDynArray<GPoint2>& Points, GDynArray<GInt32>& Index) const;
		/*!
			Convert the contour into a GPath2D representation.

			\param Path the output paths.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
		*/
		GError ConvertToPath(GPath2D& Path) const;
		// Pick a point on curve
		GPoint2 PickPointOnCurve() const;
		GInt32 IntersectionsCount(const GRay2& NormalizedRay) const;
	};

	// *********************************************************************
	//                               GFontChar
	// *********************************************************************

	class GFontChar2D;

	/*!
		\struct GSubChar2D
		\brief This structure describe a subchar used by multy-glyphs font character.
	*/
	struct GSubChar2D {
		//! The glyph index of the subchar
		GInt32 GlyphIndex;
		//! Internal flag (sorry, but it's not documented by FreeType)
		GInt32 Flags;
		//! Transformation to apply to this subchar
		GMatrix23 Transformation;
	};

	/*!
		\struct GGlyphMetrics
		\brief A structure used to model the metrics of a single glyph.

		Values are expressed in font units (or normalized if the font has been read with option "scale=0").
	*/
	struct GGlyphMetrics {
		//! The glyph's width.
		GReal Width;
		//! The glyph's height.
		GReal Height;
		//! Left side bearing for horizontal layout.
		GReal HoriBearingX;
		//! Top side bearing for horizontal layout.
		GReal HoriBearingY;
		//! Advance width for horizontal layout.
		GReal HoriAdvance;
		//! Left side bearing for vertical layout.
		GReal VertBearingX;
		//! Top side bearing for vertical layout.
		GReal VertBearingY;
		//! Advance height for vertical layout.
		GReal VertAdvance;
	};

	// forward reference
	class GFont2D;

	/*!
		\class GFontChar2D
		\brief This class represent a 2D font character.

		Every char is described by glyph's metrics and its closed contours. A font char could be 'plain' or 'composite'.
		A 'plain' character is a character that owns its contours. A 'composite' character (also know a multi-glyph
		character) if just a simple collection (a container) of indexes, every index specify a reference to a 'plain' or
		'composite' character (this lead to a tree system).
	*/
	class G_EXPORT GFontChar2D {

		friend class GFont2D;

	private:
		struct GCharOutline {
			GInt32 OuterContourIndex;
			GDynArray<GInt32> HolesContoursIndexes;
		};

		//! The GFont instance that created this character.
		const GFont2D *gFont;
		/*!
			The list of all closed contours that build this character, this array has sense for 'plain' characters
			only. If this is a multi-glyphs character this array is empty.
		*/
		GDynArray<GFontCharContour2D> gContours;
		/*!
			The list of sub-characters that build this character, this array has sense for multi-glyphs characters
			only. For 'plain' characters this array is empty.
		*/
		GDynArray<GSubChar2D> gSubChars;
		mutable GDynArray<GCharOutline> gOutlines;
		/*!
			The metrics of the glyph. The returned values depend on the "scale" loading option.\n
			Values are expressed in font units (or normalized if the font has been read with option "scale=0")
		*/
		GGlyphMetrics gMetrics;
		/*!
			This field holds the linearly scaled horizontal advance width for the glyph (i.e. the scaled and unhinted value
			of the hori advance). This can be important to perform correct WYSIWYG layout.
			
			\note this value is expressed in original font units ((or normalized if the font has been read with
			option "scale=0").
		*/
		GReal gLinearHoriAdvance;
		//! This field holds the linearly scaled vertical advance height for the glyph.
		GReal gLinearVertAdvance;
		//! This is the transformed advance width for the glyph.
		GVect<GReal, 2> gAdvance;
		//! The difference between hinted and unhinted left side bearing while autohinting is active. 0 otherwise.
		GReal gLSBDelta;
		//! The difference between hinted and unhinted right side bearing while autohinting is active. 0 otherwise.
		GReal gRSBDelta;
		/*!
			By default, outlines are filled using the non-zero winding rule. If set to G_TRUE, the outline must
			be filled using the even-odd fill rule to respect font visual characteristics.
		*/
		GBool gEvenOddFill;
		//! Flag that indicates if this char has been initialized (contours labeled and so on...).
		mutable GBool gInitialized;
		
		//! Initialize all contours and labels holes
		void Initialize() const;

		//! Copy constructor.
		GFontChar2D(const GFontChar2D& Source);

	protected:
		//! Remove and free all outlines
		void DeleteOutlines();
		//! Assign to a single labeled hole to its respective outer contour
		GInt32 AssignHoleOutline(const GInt32 HoleContourIndex) const;
		//! Label a single contour as hole or solid (using the crossing line algorithm)
		void LabelContour(const GFontCharContour2D& Contour) const;
		//! Label every contours as hole or solid (using the crossing line algorithm)
		void LabelHolesAndFilled() const;
		//! Swap (toggle) internal hole/solid flag (assigned by the LabelHolesAndFilled() function) according to passed flag.
		void SwapHolesAndFilledLabels(const GBool EvenOddFlag) const;
		//! Reverse points order of each contour, according to its hole/solid flag and cw/ccw flag.
		void FixHolesAndFilledWiseOrder() const;
		//! Assign labeled holes to their respective outer contour
		void SetHolesContainers() const;
		/*!
			Do the effective outline drawing.

			\param Outline the outline to draw
			\param Points the points array, where generated points will be appended
			\param PointsCounters the counters array, for every contour an integer (that represents the number of
			points appended for that contour) will be appended.
			\param Variation the parameter used as maximum permitted variation (squared chordal distance).
		*/
		void DrawOutline(const GCharOutline& Outline, GDynArray<GPoint2>& Points,
						 GDynArray<GInt32>& PointsCounters, const GReal Variation) const;

		/*!
			Do the effective outline drawing.

			\param Outline the outline to draw
			\param Points the points array, where generated points will be appended
			\param PointsCounters the counters array, for every contour an integer (that represents the number of
			points appended for that contour) will be appended.
			\param Variation the parameter used as maximum permitted variation (squared chordal distance).
			\param Transformation the affine transformation to apply to all contours points.
		*/
		void DrawOutline(const GCharOutline& Outline, GDynArray<GPoint2>& Points,
						 GDynArray<GInt32>& PointsCounters, const GReal Variation, const GMatrix23& Transformation) const;


		/*!
			Set constructor, build a 'plain' character.

			\param Owner the font that has created this character.
			\param Contours the array of closed contours that will build the new plain character.
		*/
		GFontChar2D(const GFont2D* Owner, const GDynArray<GFontCharContour2D>& Contours);
		/*!
			Set constructor, build a 'composite' character.

			\param Owner the font that has created this character.
			\param SubChars the collection of sub-characters that will build the new composite character.
		*/
		GFontChar2D(const GFont2D* Owner, const GDynArray<GSubChar2D>& SubChars);
		//! Destructor, free all contours and set null metrics.
		~GFontChar2D();

	public:
		/*!
			Get number of closed contours owned by this character. If this char is composite, 0 will be returned.

			\todo add an 'exmpand' flag, to make composite characters to be seen as plain character.
		*/
		inline GInt32 ContoursCount() const {
			return (GInt32)gContours.size();
		}
		/*!
			Get the Index-th contour owned by this character. If this char is composite, a NULL pointer will be returned.

			\todo add an 'exmpand' flag, to make composite characters to be seen as plain character.
		*/
		const GFontCharContour2D* Contour(const GUInt32 Index) const;
		//! In the case of composite char, return the number of subchars.
		inline GInt32 SubCharsCount() const {
			return (GInt32)gSubChars.size();
		}
		/*!
			In the case of composite char, get the Index-th subchar.

			\param Index the index of the subchar we're trying to get.
			\param SubCharInfo information about the Index-th subchar.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
		*/
		GError SubChar(const GUInt32 Index, GSubChar2D& SubCharInfo) const;
		/*! 
			Get the number of outlines that compose this character.

			Here the concept of outline has been introduced just for convenience, to permit to third parts
			libraries/renderers to be (i wish!) more comfortable.
			An outline is just a logical grouping of geometric contours that build this character.
			Every group (called outline) is made of:

			- an outer contour (called main contour)
			- a list of holes contours relative (inside) the outer contour.

			An outline can be thought as a brush trait, made by an outer solid contour and its holes.
			This method take care of composite characters, so the number returned is the sum of all sub-chars
			outlines.
		*/
		GInt32 OutlinesCount() const;
		/*!
			Draw a single outline, with a maximum (squared chordal distance) variation specified.

			\param Index the index of outline, must be a valid.
			\param Points the points array, where generated points will be appended
			\param PointsCounters the counters array, for every contour an integer (that represents the number of
			points appended for that contour) will be appended.
			\param Variation the parameter used as maximum permitted variation (squared chordal distance).
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
		*/
		GError DrawOutline(const GInt32 Index, GDynArray<GPoint2>& Points,
						   GDynArray<GInt32>& PointsCounters, const GReal Variation = 1e-4) const;
		/*!
			Convert each contour that builds the character, into a GPath2D representation.

			This method "expand" all sub-characters's contours (for multi-glyph character).
			\param Paths the output array where paths are appended.
			\return G_NO_ERROR if the operation succeeds, an error code otherwise.
			\note each path is created using gFont's kernel. Memory of created paths isn't freed when this character
			is delete, nor when gFont is deleted.
		*/
		GError ConvertToPaths(GDynArray<GPath2D *>& Paths) const;
		//! Get the linearly scaled horizontal advance width.
		GReal LinearHoriAdvance() const {
			return gLinearHoriAdvance;
		}
		//! Set the linearly scaled horizontal advance width.
		void SetLinearHoriAdvance(const GReal NewValue) {
			gLinearHoriAdvance = NewValue;
		}
		//! Get the linearly scaled vertical advance height.
		GReal LinearVertAdvance() const {
			return gLinearVertAdvance;
		}
		//! Set the linearly scaled vertical advance height.
		void SetLinearVertAdvance(const GReal NewValue) {
			gLinearVertAdvance = NewValue;
		}
		//! Get the transformed advance width.
		const GVect<GReal, 2>& Advance() const {
			return gAdvance;
		}
		//! Set the transformed advance width.
		void SetAdvance(const GVect<GReal, 2>& NewValue) {
			gAdvance = NewValue;
		}
		//! Get the difference between hinted and unhinted left side bearing while autohinting is active, 0 otherwise.
		GReal LSBDelta() const {
			return gLSBDelta;
		}
		//! Set the difference between hinted and unhinted left side bearing while autohinting is active, 0 otherwise.
		void SetLSBDelta(const GReal NewValue) {
			gLSBDelta = NewValue;
		}
		//! Get the difference between hinted and unhinted right side bearing while autohinting is active, 0 otherwise.
		GReal RSBDelta() const {
			return gRSBDelta;
		}
		// set the difference between hinted and unhinted right side bearing while autohinting is active, 0 otherwise.
		void SetRSBDelta(const GReal NewValue) {
			gRSBDelta = NewValue;
		}
		//! Get character metrics.
		const GGlyphMetrics& GlyphMetrics() const {
			return gMetrics;
		}
		//! Set the metrics.
		void SetMetrics(const GGlyphMetrics& NewMetrics) {
			gMetrics = NewMetrics;
		}
		//! Get if the filling has to be done using an even-odd rule.
		GBool EvenOddFill() const {
			return gEvenOddFill;
		}
		//! Set even-odd fill flag.
		void SetEvenOddFill(const GBool NewValue) {
			gEvenOddFill = NewValue;
		}
		//! Return G_TRUE if the char is composite, G_FALSE otherwise.
		inline GBool IsComposite() const {
			if (gSubChars.size() > 0)
				return G_TRUE;
			return G_FALSE;
		}
	};

	// *********************************************************************
	//                                GFont2D
	// *********************************************************************

	//! GFont2D static class descriptor.
	static const GClassID G_FONT2D_CLASSID = GClassID("GFont2D", 0xD3F1328E, 0xFA0B4A55, 0x8FB7DB72, 0xFEDB9458);

	//! An association between a char code to its glyph index (for a charsmap).
	struct GEncodedChar {
		//! The char code (maybe be an ascii code or a unicode or whatever specific for charsmap).
		GUInt32 CharCode;
		//! Glyph index corresponding to charcode.
		GUInt32 GlyphIndex;
	};

	//! Charsmap types
	enum GCharMapEncoding {
		//! Reserved
		G_ENCODING_NONE,
		/*!
			Corresponds to the Microsoft Symbol encoding, used to encode mathematical symbols in the 32..255 character
			code range.
		*/
		G_ENCODING_MS_SYMBOL,
		/*!
			Corresponds to the Unicode character set. This value covers all versions of the Unicode repertoire, including
			ASCII and  Latin-1. Most fonts include a Unicode charsmap, but not all of them.
		*/
		G_ENCODING_UNICODE,
		//! Corresponds to Japanese SJIS encoding
		G_ENCODING_SJIS,
		//! Corresponds to an encoding system for Simplified Chinese as used used in mainland China.
		G_ENCODING_GB2312,
		//! Corresponds to an encoding system for Traditional Chinese as used in Taiwan and Hong Kong
		G_ENCODING_BIG5,
		//! Corresponds to the Korean encoding system known as Wansung
		G_ENCODING_WANSUNG,
		/*!
			The Korean standard character set (KS C-5601-1992), which corresponds to MS Windows code page 1361. This
			character set includes all possible Hangeul character combinations.
		*/
		G_ENCODING_JOHAB,
		/*!
			Corresponds to the Adobe Standard encoding, as found in Type 1, CFF, and OpenType/CFF fonts. It is
			limited to 256 character codes.
		*/
		G_ENCODING_ADOBE_STANDARD,
		/*!
			Corresponds to the Adobe Expert encoding, as found in Type 1, CFF, and OpenType/CFF fonts. It is
			limited to 256 character codes.
		*/
		G_ENCODING_ADOBE_EXPERT,
		/*!
			Corresponds to a custom encoding, as found in Type 1, CFF, and OpenType/CFF fonts. It is limited to
			256 character codes.
		*/
		G_ENCODING_ADOBE_CUSTOM,
		// Corresponds to a Latin-1 encoding as defined in a Type 1 Postscript font. It is limited to 256 character codes
		G_ENCODING_ADOBE_LATIN_1,
		//! This value is deprecated and was never used nor reported by FreeType. Don't use or test for it.
		G_ENCODING_OLD_LATIN_2,
		/*!
			Corresponds to the 8-bit Apple roman encoding. Many TrueType and OpenType fonts contain a charsmap for
			this encoding, since older versions of Mac OS are able to use it.
		*/
		G_ENCODING_APPLE_ROMAN
	};

	/*!
		\struct GCharMap
		\brief This structure describes a characters map.

		A font object contains one or more tables, called charsmaps, that are used to convert character codes to
		glyph indexes. For example, most TrueType fonts contain two charsmaps. One is used to convert Unicode character
		codes to glyph indexes, the other is used to convert Apple Roman encoding into glyph indexes.
		Such fonts can then be used either on Windows (which uses Unicode) and Macintosh (which uses Apple Roman). Note
		also that a given charsmap might not map to all the glyphs present in the font.
	*/
	struct GCharMap {
		//! The associative map that link "char code" -> "glyph index". This
		GDynArray<GEncodedChar> CharMap;
		/*!
			An ID number describing the platform for the following encoding ID. This comes directly from the
			FreeType library (docs say: "This comes directly from the TrueType specification and should be
			emulated for other formats").
		*/
		GUInt32 PlatformID;
		/*!
			A platform specific encoding number. This also comes from the FreeType library (docs say: "This comes from
			TrueType specification and should be emulated similarly").
		*/
		GUInt32 EncodingID;
		//! A tag identifying the charsmap
		GCharMapEncoding Encoding;
	};

	/*!
		\struct GKerningEntry
		\brief Describes the kerning to be used for two characters.

		Kerning is the process of adjusting the position of two subsequent glyph images in a string of text in order
		to improve the general appearance of text. Basically, it means that when the glyph for an ‘A’ is followed by
		the glyph for a ‘V’, the space between them can be slightly reduced to avoid extra ‘diagonal whitespace’.
	*/
	struct GKerningEntry {
		//! Glyph index for 'left' glyph
		GUInt32 GlyphIndexLeft;
		//! Glyph index for 'right' glyph
		GUInt32 GlyphIndexRight;
		//! Kerning to use for the glyphs couple (GlyphIndexLeft, GlyphIndexRight).
		GPoint2 Kerning;
	};

	/*!
		\class GFont2D
		\brief 2D font class manager.

		This class is the main class used to manage 2D font. Thought this class fonts can be created and accessed to
		get informations (char maps, kerning table, and so on). Every character has its own metrics and geometric
		contours.

	*/
	class G_EXPORT GFont2D : public GElement {

	private:
		//! Filename of external file used to load this font, set by Load() function
		GString gFileName;
		//! Font family name
		GString gFamilyName;
		//! Font style name
		GString gStyleName;
		//! Number of sub-font (unsupported)
		GInt32 gSubFontsCount;
		//! Number of units per EM
		GUInt32 gUnitsPerEM;
		//! Get the font's ascender; it is the vertical distance from the baseline to the topmost point of any glyph in the face
		GReal gAscender;
		//! Get the font's descender; it is the vertical distance from the baseline to the bottommost point of any glyph in the face
		GReal gDescender;
		//! External leading (also known as "line gap")
		GReal gExternalLeading;
		//! The maximal advance width, in font units, for all glyphs in this font.
		GReal gMaxAdvanceWidth;
		//! The maximal advance height, in font units, for all glyphs in this font
		GReal gMaxAdvanceHeight;
		//! The position, in font units, of the underline line for this font. It's the center of the underlining stem.
		GReal gUnderlinePosition;
		//! The thickness, in font units, of the underline for this font.
		GReal gUnderlineThickness;
		//! Italic style flag
		GBool gItalic;
		//! Bold style flag
		GBool gBold;
		//! Array of font characters.
		GDynArray<GFontChar2D *> gChars;
		//! Array of font characters maps
		GDynArray<GCharMap> gCharsMaps;
		//! Kerning table
		GDynArray<GKerningEntry> gKerningTable;

	protected:
		//! Clone characters
		GError CloneChars(const GDynArray<GFontChar2D *>& Chars, GDynArray<GFontChar2D *>& Destination);
		//! Cloning function, copy (physically) a Source GFont2D element to this font instance.
		GError BaseClone(const GElement& Source);
		//! Remove and delete all instanced chars.
		void DeleteChars();
		//! Delete kerning informations.
		void DeleteKerning();

	public:
		//! Default constructor. Creates an empty font.
		GFont2D();
		//! Constructor with owner (kernel) specification. Creates an empty font.
		GFont2D(const GElement* Owner);
		//! Destructor. It frees all characters, kerning infos and charsmaps.
		~GFont2D();
		/*!
			Load a 2D font from external file (using kernel plugins). This method is provided for convenience.

			Internally this method look for kernel that has created this instances. If such kernel exists, call
			kernel->Load() function passing specified parameters, else return a G_MISSING_KERNEL error code.

			\param FileName is a string containing the full file name (path + name + extension) of the file we wanna read
			from. It must be non-empty.
			\param Options a string containing a semicolon-separated options. Each option must be in the format name=value.
			\param FormatName an optional filter; it's case insensitive.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
		*/
		GError Load(const GChar8 *FileName, const GChar8 *Options = NULL, const GChar8 *FormatName = NULL);
		//! Return the number of characters present in this font.
		GInt32 CharsCount() const {
			return (GInt32)gChars.size();
		}
		//! Add new char to this font, specifying contours.
		GFontChar2D* AddChar(const GDynArray<GFontCharContour2D>& Contours);
		//! Add a new char to this font, specifying sub-characters
		GFontChar2D* AddChar(const GDynArray<GSubChar2D>& SubChars);
		//! Remove a char from this font.
		GError RemoveChar(const GInt32 Index);
		//! Clear the font; it eliminates all chars, charsmaps and kerning informations.
		void Clear();
		//! Get a character, specifying its (glyph)index.
		const GFontChar2D* CharByIndex(const GInt32 Index) const;
		/*!
			Get a character, specifying charcode for the CharMapIndex-th charmap.

			\return a pointer to the requested character, or NULL if requested character does not exists (or a bad
			CharMapIndex parameter has been specified).
		*/
		const GFontChar2D* CharByCode(const GUInt32 CharCode, const GUInt32 CharMapIndex) const;
		//! Get the Index-th charsmap reference.
		const GCharMap& CharMap(const GInt32 Index) const;
		//! Get the number of charsmaps present in the the font.
		GInt32 CharMapsCount() const {
			return (GInt32)gCharsMaps.size();
		}
		//! Add a char map to this font.
		GError AddCharMap(const GCharMap& NewCharMap);
		//! Remove a char map from this font.
		GError RemoveCharMap(const GInt32 CharMapIndex);
		//! Remove all char maps from this font.
		void RemoveCharsMaps();
		/*!
			Get (glyph)index, given a valid charcode for a specified charsmap.

			\return the character index corresponding to the charcode, a -1 return value indicates an error.
		*/
		GInt32 CharIndexByCharCode(const GUInt32 CharCode, const GUInt32 CharMapIndex) const;
		//! Get font family name
		const GString& FamilyName() const {
			return gFamilyName;
		}
		//! Set font family name
		inline void SetFamilyName(const GString& NewFamilyName) {
			gFamilyName = NewFamilyName;
		}
		//! Get font style name.
		inline const GString& StyleName() const {
			return gStyleName;
		}
		//! Set font style name
		inline void SetStyleName(const GString& NewStyleName) {
			gStyleName = NewStyleName;
		}
		//! Specifies if the font is italic.
		GBool IsItalic() const {
			return gItalic;
		}
		//! Set italic style flag
		inline void SetItalic(const GBool NewValue) {
			gItalic = NewValue;
		}
		//! Specifies if the font is bold
		inline GBool IsBold() const {
			return gBold;
		}
		//! Set bold style flag
		inline void SetBold(const GBool NewValue) {
			gBold = NewValue;
		}
		//! If G_TRUe is returned, it indicates that the font contains kerning information.
		inline GBool HasKerningInfos() const {
			//return gKerningInfos;
			if (gKerningTable.size() > 0)
				return G_TRUE;
			return G_FALSE;
		}
		//! Set kerning informations for this font.
		void SetKerning(const GDynArray<GKerningEntry>& NewKerningTable);
		//! Get kerning vector, specifying the couple of(glyphs)index.
		const GPoint2& KerningByIndex(const GUInt32 LeftIndex, const GUInt32 RightIndex) const;
		//! Get kerning vector, specifying the couple of character codes and the charsmap to use for mapping.
		const GPoint2& KerningByCharCode(const GUInt32 LeftGlyph, const GUInt32 RightIndex,
										 const GUInt32 CharMapIndex) const;
		/*!
			Get the number of font units per EM square for this font; this is typically 2048 for
			TrueType fonts, 1000 for Type1 fonts.
		*/
		inline GUInt32 UnitsPerEM() const {
			return gUnitsPerEM;
		}
		//! Set the number of font units per EM square.
		inline void SetUnitsPerEM(const GUInt32 NewValue) {
			gUnitsPerEM = NewValue;
		}
		/*!
			Get the font's ascender; it is the vertical distance from the baseline to the topmost
			point of any glyph in the face. This field's value is positive, expressed in font units.
		*/
		inline GReal Ascender() const {
			return gAscender;
		}
		//! Set font's ascender value.
		inline void SetAscender(const GReal NewValue) {
			gAscender = NewValue;
		}
		/*!
			Get the font's descender; it is the vertical distance from the baseline to the bottommost point
			of any glyph in the face. This field's value is negative for values below the baseline. It is expressed
			in font units.
		*/
		inline GReal Descender() const {
			return gDescender;
		}
		//! Set font's descender value
		inline void SetDescender(const GReal NewValue) {
			gDescender = NewValue;
		}
		//! Get external leading (also known as "line gap")
		inline GReal ExternalLeading() const {
			return gExternalLeading;
		}
		//! Set external leading
		inline void SetExternalLeading(const GReal NewValue) {
			gExternalLeading = NewValue;
		}
		/*!
			Get the font's height; it is the vertical distance from one baseline to the next when writing
			several lines of text. Its value is always positive, expressed in font units.
			The value can be computed as `ascender + descender + line_gap' where the value of `line_gap'
			is also called `external leading'.
		*/
		inline GReal Height() const {
			return gAscender + gDescender + gExternalLeading;
		}
		/*!
			Get the maximal advance width, in font units, for all glyphs in this font. This can be used to make
			word wrapping computations faster.
		*/
		inline GReal MaxAdvanceWidth() const {
			return gMaxAdvanceWidth;
		}
		//! Set the maximal advance width
		void SetMaxAdvanceWidth(const GReal NewValue) {
			gMaxAdvanceWidth = NewValue;
		}
		/*!
			Get the maximal advance height, in font units, for all glyphs in this font. This is only relevant
			for vertical layouts, and should be set to the `height' for fonts that do not provide vertical metrics.
		*/
		GReal MaxAdvanceHeight() const {
			return gMaxAdvanceHeight;
		}
		//! Set the maximal advance height.
		void SetMaxAdvanceHeight(const GReal NewValue) {
			gMaxAdvanceHeight = NewValue;
		}
		/*!
			Get the position, in font units, of the underline line for this font. It's the center
			of the underlining stem.
		*/
		GReal UnderlinePosition() const {
			return gUnderlinePosition;
		}
		//! Set the position of the underline line for this font
		void SetUnderlinePosition(const GReal NewValue) {
			gUnderlinePosition = NewValue;
		}
		//! Get the thickness, in font units, of the underline for this font.
		GReal UnderlineThickness() const {
			return gUnderlineThickness;
		}
		//! Set the thickness of the underline for this font
		void SetUnderlineThickness(const GReal NewValue) {
			gUnderlineThickness = NewValue;
		}
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_FONT2D_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
	};


	// *********************************************************************
	//                            GFont2DProxy
	// *********************************************************************

	/*!
		\class GFont2DProxy
		\brief This class implements a GFont2DProxy proxy (provider).

		This proxy provides the creation of GFont2D class instances.
	*/

	class G_EXPORT GFont2DProxy : public GElementProxy {
	public:
		//! Creates a new GFont2D instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GFont2D(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_FONT2D_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
	};
	//! Static proxy for GFont2D class.
	static const GFont2DProxy G_FONT2D_PROXY;

};	// end namespace Amanith

#endif
