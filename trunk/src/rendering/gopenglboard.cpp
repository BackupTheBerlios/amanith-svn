/****************************************************************************
** $file: amanith/src/rendering/gopenglboard.cpp   0.2.0.0   edited Dec, 12 2005
**
** OpenGL based draw board implementation.
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

#include "amanith/rendering/gopenglboard.h"
#include "amanith/geometry/gxform.h"
#include "amanith/geometry/gxformconv.h"

/*!
	\file gopenglboard.cpp
	\brief OpenGL based draw board implementation file.
*/

namespace Amanith {

#ifdef _DEBUG
void GOpenGLBoard::DumpStencilBuffer(const GChar8 *FileName) {

	GUInt32 x, y, w, h;
	Viewport(x, y, w, h);

	GLubyte *buf;
	std::FILE *f = NULL;

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	buf = new GLubyte[w * h];
	std::memset(buf, 0, w * h);
	glReadPixels(0, 0, w, h, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, buf);

	// open the file
#if defined(G_OS_WIN) && _MSC_VER >= 1400
	errno_t openErr = fopen_s(&f, FileName, "wb");
	if (f && !openErr) {
		std::fwrite(buf, 1, w * h, f);
		std::fflush(f);
		std::fclose(f);
	}
#else
	f = std::fopen(FileName, "wb");
	if (f) {
		std::fwrite(buf, 1, w*h, f);
		std::fflush(f);
		std::fclose(f);
	}
#endif
	delete [] buf;
}
#endif

// *********************************************************************
//                        GOpenGLCachedDrawing
// *********************************************************************

// invalidate the cache, freeing associated (video) memory
void GOpenGLCachedDrawing::Invalidate() {

	GOpenGLCacheEntry entry;
	GUInt32 i, j = (GUInt32)gEntries.size();

	for (i = 0; i < j; ++i) {
		entry = gEntries[i];
		if (entry.FillDisplayList != 0)
			glDeleteLists(entry.FillDisplayList, 1);
		if (entry.StrokeDisplayList != 0)
			glDeleteLists(entry.StrokeDisplayList, 1);
	}
	gEntries.clear();
}

// constructor, it build an empty cache slot
GOpenGLCachedDrawing::GOpenGLCachedDrawing() {
}

// destructor, it invalidates cached shapes and frees memory
GOpenGLCachedDrawing::~GOpenGLCachedDrawing() {

	Invalidate();
}

// get the number of cached shapes
GUInt32 GOpenGLCachedDrawing::CacheEntriesCount() const {

	return ((GUInt32)gEntries.size());
}


// *********************************************************************
//                             GOpenGLBoard
// *********************************************************************

struct GNamedSVGcolor {
	GChar8 Name[22];
	GVector4 RGBA;
};

#ifdef DOUBLE_REAL_TYPE

static const GNamedSVGcolor SVGColors[147] = {

	{ "aliceblue", GVector4(0.941, 0.973, 1.000, 1.000) },
	{ "antiquewhite", GVector4(0.980, 0.922, 0.843, 1.000) },
	{ "aqua", GVector4(0.000, 1.000, 1.000, 1.000) },
	{ "aquamarine", GVector4(0.498, 1.000, 0.831, 1.000) },
	{ "azure", GVector4(0.941, 1.000, 1.000, 1.000) },
	{ "beige", GVector4(0.961, 0.961, 0.863, 1.000) },
	{ "bisque", GVector4(1.000, 0.894, 0.769, 1.000) },
	{ "black", GVector4(0.000, 0.000, 0.000, 1.000) },
	{ "blanchedalmond", GVector4(1.000, 0.922, 0.804, 1.000) },
	{ "blue", GVector4(0.000, 0.000, 1.000, 1.000) },
	{ "blueviolet", GVector4(0.541, 0.169, 0.886, 1.000) },
	{ "brown", GVector4(0.647, 0.165, 0.165, 1.000) },
	{ "burlywood", GVector4(0.871, 0.722, 0.529, 1.000) },
	{ "cadetblue", GVector4(0.373, 0.620, 0.627, 1.000) },
	{ "chartreuse", GVector4(0.498, 1.000, 0.000, 1.000) },
	{ "chocolate", GVector4(0.824, 0.412, 0.118, 1.000) },
	{ "coral", GVector4(1.000, 0.498, 0.314, 1.000) },
	{ "cornflowerblue", GVector4(0.392, 0.584, 0.929, 1.000) },
	{ "cornsilk", GVector4(1.000, 0.973, 0.863, 1.000) },
	{ "crimson", GVector4(0.863, 0.078, 0.235, 1.000) },
	{ "cyan", GVector4(0.000, 1.000, 1.000, 1.000) },
	{ "darkblue", GVector4(0.000, 0.000, 0.545, 1.000) },
	{ "darkcyan", GVector4(0.000, 0.545, 0.545, 1.000) },
	{ "darkgoldenrod", GVector4(0.722, 0.525, 0.043, 1.000) },
	{ "darkgray", GVector4(0.663, 0.663, 0.663, 1.000) },
	{ "darkgreen", GVector4(0.000, 0.392, 0.000, 1.000) },
	{ "darkgrey", GVector4(0.663, 0.663, 0.663, 1.000) },
	{ "darkkhaki", GVector4(0.741, 0.718, 0.420, 1.000) },
	{ "darkmagenta", GVector4(0.545, 0.000, 0.545, 1.000) },
	{ "darkolivegreen", GVector4(0.333, 0.420, 0.184, 1.000) },
	{ "darkorange", GVector4(1.000, 0.549, 0.000, 1.000) },
	{ "darkorchid", GVector4(0.600, 0.196, 0.800, 1.000) },
	{ "darkred", GVector4(0.545, 0.000, 0.000, 1.000) },
	{ "darksalmon", GVector4(0.914, 0.588, 0.478, 1.000) },
	{ "darkseagreen", GVector4(0.561, 0.737, 0.561, 1.000) },
	{ "darkslateblue", GVector4(0.282, 0.239, 0.545, 1.000) },
	{ "darkslategray", GVector4(0.184, 0.310, 0.310, 1.000) },
	{ "darkslategrey", GVector4(0.184, 0.310, 0.310, 1.000) },
	{ "darkturquoise", GVector4(0.000, 0.808, 0.820, 1.000) },
	{ "darkviolet", GVector4(0.580, 0.000, 0.827, 1.000) },
	{ "deeppink", GVector4(1.000, 0.078, 0.576, 1.000) },
	{ "deepskyblue", GVector4(0.000, 0.749, 1.000, 1.000) },
	{ "dimgray", GVector4(0.412, 0.412, 0.412, 1.000) },
	{ "dimgrey", GVector4(0.412, 0.412, 0.412, 1.000) },
	{ "dodgerblue", GVector4(0.118, 0.565, 1.000, 1.000) },
	{ "firebrick", GVector4(0.698, 0.133, 0.133, 1.000) },
	{ "floralwhite", GVector4(1.000, 0.980, 0.941, 1.000) },
	{ "forestgreen", GVector4(0.133, 0.545, 0.133, 1.000) },
	{ "fuchsia", GVector4(1.000, 0.000, 1.000, 1.000) },
	{ "gainsboro", GVector4(0.863, 0.863, 0.863, 1.000) },
	{ "ghostwhite", GVector4(0.973, 0.973, 1.000, 1.000) },
	{ "gold", GVector4(1.000, 0.843, 0.000, 1.000) },
	{ "goldenrod", GVector4(0.855, 0.647, 0.125, 1.000) },
	{ "gray", GVector4(0.502, 0.502, 0.502, 1.000) },
	{ "green", GVector4(0.000, 0.502, 0.000, 1.000) },
	{ "greenyellow", GVector4(0.678, 1.000, 0.184, 1.000) },
	{ "grey", GVector4(0.502, 0.502, 0.502, 1.000) },
	{ "honeydew", GVector4(0.941, 1.000, 0.941, 1.000) },
	{ "hotpink", GVector4(1.000, 0.412, 0.706, 1.000) },
	{ "indianred", GVector4(0.804, 0.361, 0.361, 1.000) },
	{ "indigo", GVector4(0.294, 0.000, 0.510, 1.000) },
	{ "ivory", GVector4(1.000, 1.000, 0.941, 1.000) },
	{ "khaki", GVector4(0.941, 0.902, 0.549, 1.000) },
	{ "lavender", GVector4(0.902, 0.902, 0.980, 1.000) },
	{ "lavenderblush", GVector4(1.000, 0.941, 0.961, 1.000) },
	{ "lawngreen", GVector4(0.486, 0.988, 0.000, 1.000) },
	{ "lemonchiffon", GVector4(1.000, 0.980, 0.804, 1.000) },
	{ "lightblue", GVector4(0.678, 0.847, 0.902, 1.000) },
	{ "lightcoral", GVector4(0.941, 0.502, 0.502, 1.000) },
	{ "lightcyan", GVector4(0.878, 1.000, 1.000, 1.000) },
	{ "lightgoldenrodyellow", GVector4(0.980, 0.980, 0.824, 1.000) },
	{ "lightgray", GVector4(0.827, 0.827, 0.827, 1.000) },
	{ "lightgreen", GVector4(0.565, 0.933, 0.565, 1.000) },
	{ "lightgrey", GVector4(0.827, 0.827, 0.827, 1.000) },
	{ "lightpink", GVector4(1.000, 0.714, 0.757, 1.000) },
	{ "lightsalmon", GVector4(1.000, 0.627, 0.478, 1.000) },
	{ "lightseagreen", GVector4(0.125, 0.698, 0.667, 1.000) },
	{ "lightskyblue", GVector4(0.529, 0.808, 0.980, 1.000) },
	{ "lightslategray", GVector4(0.467, 0.533, 0.600, 1.000) },
	{ "lightslategrey", GVector4(0.467, 0.533, 0.600, 1.000) },
	{ "lightsteelblue", GVector4(0.690, 0.769, 0.871, 1.000) },
	{ "lightyellow", GVector4(1.000, 1.000, 0.878, 1.000) },
	{ "lime", GVector4(0.000, 1.000, 0.000, 1.000) },
	{ "limegreen", GVector4(0.196, 0.804, 0.196, 1.000) },
	{ "linen", GVector4(0.980, 0.941, 0.902, 1.000) },
	{ "magenta", GVector4(1.000, 0.000, 1.000, 1.000) },
	{ "maroon", GVector4(0.502, 0.000, 0.000, 1.000) },
	{ "mediumaquamarine", GVector4(0.400, 0.804, 0.667, 1.000) },
	{ "mediumblue", GVector4(0.000, 0.000, 0.804, 1.000) },
	{ "mediumorchid", GVector4(0.729, 0.333, 0.827, 1.000) },
	{ "mediumpurple", GVector4(0.576, 0.439, 0.859, 1.000) },
	{ "mediumseagreen", GVector4(0.235, 0.702, 0.443, 1.000) },
	{ "mediumslateblue", GVector4(0.482, 0.408, 0.933, 1.000) },
	{ "mediumspringgreen", GVector4(0.000, 0.980, 0.604, 1.000) },
	{ "mediumturquoise", GVector4(0.282, 0.820, 0.800, 1.000) },
	{ "mediumvioletred", GVector4(0.780, 0.082, 0.522, 1.000) },
	{ "midnightblue", GVector4(0.098, 0.098, 0.439, 1.000) },
	{ "mintcream", GVector4(0.961, 1.000, 0.980, 1.000) },
	{ "mistyrose", GVector4(1.000, 0.894, 0.882, 1.000) },
	{ "moccasin", GVector4(1.000, 0.894, 0.710, 1.000) },
	{ "navajowhite", GVector4(1.000, 0.871, 0.678, 1.000) },
	{ "navy", GVector4(0.000, 0.000, 0.502, 1.000) },
	{ "oldlace", GVector4(0.992, 0.961, 0.902, 1.000) },
	{ "olive", GVector4(0.502, 0.502, 0.000, 1.000) },
	{ "olivedrab", GVector4(0.420, 0.557, 0.137, 1.000) },
	{ "orange", GVector4(1.000, 0.647, 0.000, 1.000) },
	{ "orangered", GVector4(1.000, 0.271, 0.000, 1.000) },
	{ "orchid", GVector4(0.855, 0.439, 0.839, 1.000) },
	{ "palegoldenrod", GVector4(0.933, 0.910, 0.667, 1.000) },
	{ "palegreen", GVector4(0.596, 0.984, 0.596, 1.000) },
	{ "paleturquoise", GVector4(0.686, 0.933, 0.933, 1.000) },
	{ "palevioletred", GVector4(0.859, 0.439, 0.576, 1.000) },
	{ "papayawhip", GVector4(1.000, 0.937, 0.835, 1.000) },
	{ "peachpuff", GVector4(1.000, 0.855, 0.725, 1.000) },
	{ "peru", GVector4(0.804, 0.522, 0.247, 1.000) },
	{ "pink", GVector4(1.000, 0.753, 0.796, 1.000) },
	{ "plum", GVector4(0.867, 0.627, 0.867, 1.000) },
	{ "powderblue", GVector4(0.690, 0.878, 0.902, 1.000) },
	{ "purple", GVector4(0.502, 0.000, 0.502, 1.000) },
	{ "red", GVector4(1.000, 0.000, 0.000, 1.000) },
	{ "rosybrown", GVector4(0.737, 0.561, 0.561, 1.000) },
	{ "royalblue", GVector4(0.255, 0.412, 0.882, 1.000) },
	{ "saddlebrown", GVector4(0.545, 0.271, 0.075, 1.000) },
	{ "salmon", GVector4(0.980, 0.502, 0.447, 1.000) },
	{ "sandybrown", GVector4(0.957, 0.643, 0.376, 1.000) },
	{ "seagreen", GVector4(0.180, 0.545, 0.341, 1.000) },
	{ "seashell", GVector4(1.000, 0.961, 0.933, 1.000) },
	{ "sienna", GVector4(0.627, 0.322, 0.176, 1.000) },
	{ "silver", GVector4(0.753, 0.753, 0.753, 1.000) },
	{ "skyblue", GVector4(0.529, 0.808, 0.922, 1.000) },
	{ "slateblue", GVector4(0.416, 0.353, 0.804, 1.000) },
	{ "slategray", GVector4(0.439, 0.502, 0.565, 1.000) },
	{ "slategrey", GVector4(0.439, 0.502, 0.565, 1.000) },
	{ "snow", GVector4(1.000, 0.980, 0.980, 1.000) },
	{ "springgreen", GVector4(0.000, 1.000, 0.498, 1.000) },
	{ "steelblue", GVector4(0.275, 0.510, 0.706, 1.000) },
	{ "tan", GVector4(0.824, 0.706, 0.549, 1.000) },
	{ "teal", GVector4(0.000, 0.502, 0.502, 1.000) },
	{ "thistle", GVector4(0.847, 0.749, 0.847, 1.000) },
	{ "tomato", GVector4(1.000, 0.388, 0.278, 1.000) },
	{ "turquoise", GVector4(0.251, 0.878, 0.816, 1.000) },
	{ "violet", GVector4(0.933, 0.510, 0.933, 1.000) },
	{ "wheat", GVector4(0.961, 0.871, 0.702, 1.000) },
	{ "white", GVector4(1.000, 1.000, 1.000, 1.000) },
	{ "whitesmoke", GVector4(0.961, 0.961, 0.961, 1.000) },
	{ "yellow", GVector4(1.000, 1.000, 0.000, 1.000) },
	{ "yellowgreen", GVector4(0.604, 0.804, 0.196, 1.000) }
};

#else

static const GNamedSVGcolor SVGColors[147] = {

	{ "aliceblue", GVector4(0.941f, 0.973f, 1.000f, 1.000f) },
	{ "antiquewhite", GVector4(0.980f, 0.922f, 0.843f, 1.000f) },
	{ "aqua", GVector4(0.000f, 1.000f, 1.000f, 1.000f) },
	{ "aquamarine", GVector4(0.498f, 1.000f, 0.831f, 1.000f) },
	{ "azure", GVector4(0.941f, 1.000f, 1.000f, 1.000f) },
	{ "beige", GVector4(0.961f, 0.961f, 0.863f, 1.000f) },
	{ "bisque", GVector4(1.000f, 0.894f, 0.769f, 1.000f) },
	{ "black", GVector4(0.000f, 0.000f, 0.000f, 1.000f) },
	{ "blanchedalmond", GVector4(1.000f, 0.922f, 0.804f, 1.000f) },
	{ "blue", GVector4(0.000f, 0.000f, 1.000f, 1.000f) },
	{ "blueviolet", GVector4(0.541f, 0.169f, 0.886f, 1.000f) },
	{ "brown", GVector4(0.647f, 0.165f, 0.165f, 1.000f) },
	{ "burlywood", GVector4(0.871f, 0.722f, 0.529f, 1.000f) },
	{ "cadetblue", GVector4(0.373f, 0.620f, 0.627f, 1.000f) },
	{ "chartreuse", GVector4(0.498f, 1.000f, 0.000f, 1.000f) },
	{ "chocolate", GVector4(0.824f, 0.412f, 0.118f, 1.000f) },
	{ "coral", GVector4(1.000f, 0.498f, 0.314f, 1.000f) },
	{ "cornflowerblue", GVector4(0.392f, 0.584f, 0.929f, 1.000f) },
	{ "cornsilk", GVector4(1.000f, 0.973f, 0.863f, 1.000f) },
	{ "crimson", GVector4(0.863f, 0.078f, 0.235f, 1.000f) },
	{ "cyan", GVector4(0.000f, 1.000f, 1.000f, 1.000f) },
	{ "darkblue", GVector4(0.000f, 0.000f, 0.545f, 1.000f) },
	{ "darkcyan", GVector4(0.000f, 0.545f, 0.545f, 1.000f) },
	{ "darkgoldenrod", GVector4(0.722f, 0.525f, 0.043f, 1.000f) },
	{ "darkgray", GVector4(0.663f, 0.663f, 0.663f, 1.000f) },
	{ "darkgreen", GVector4(0.000f, 0.392f, 0.000f, 1.000f) },
	{ "darkgrey", GVector4(0.663f, 0.663f, 0.663f, 1.000f) },
	{ "darkkhaki", GVector4(0.741f, 0.718f, 0.420f, 1.000f) },
	{ "darkmagenta", GVector4(0.545f, 0.000f, 0.545f, 1.000f) },
	{ "darkolivegreen", GVector4(0.333f, 0.420f, 0.184f, 1.000f) },
	{ "darkorange", GVector4(1.000f, 0.549f, 0.000f, 1.000f) },
	{ "darkorchid", GVector4(0.600f, 0.196f, 0.800f, 1.000f) },
	{ "darkred", GVector4(0.545f, 0.000f, 0.000f, 1.000f) },
	{ "darksalmon", GVector4(0.914f, 0.588f, 0.478f, 1.000f) },
	{ "darkseagreen", GVector4(0.561f, 0.737f, 0.561f, 1.000f) },
	{ "darkslateblue", GVector4(0.282f, 0.239f, 0.545f, 1.000f) },
	{ "darkslategray", GVector4(0.184f, 0.310f, 0.310f, 1.000f) },
	{ "darkslategrey", GVector4(0.184f, 0.310f, 0.310f, 1.000f) },
	{ "darkturquoise", GVector4(0.000f, 0.808f, 0.820f, 1.000f) },
	{ "darkviolet", GVector4(0.580f, 0.000f, 0.827f, 1.000f) },
	{ "deeppink", GVector4(1.000f, 0.078f, 0.576f, 1.000f) },
	{ "deepskyblue", GVector4(0.000f, 0.749f, 1.000f, 1.000f) },
	{ "dimgray", GVector4(0.412f, 0.412f, 0.412f, 1.000f) },
	{ "dimgrey", GVector4(0.412f, 0.412f, 0.412f, 1.000f) },
	{ "dodgerblue", GVector4(0.118f, 0.565f, 1.000f, 1.000f) },
	{ "firebrick", GVector4(0.698f, 0.133f, 0.133f, 1.000f) },
	{ "floralwhite", GVector4(1.000f, 0.980f, 0.941f, 1.000f) },
	{ "forestgreen", GVector4(0.133f, 0.545f, 0.133f, 1.000f) },
	{ "fuchsia", GVector4(1.000f, 0.000f, 1.000f, 1.000f) },
	{ "gainsboro", GVector4(0.863f, 0.863f, 0.863f, 1.000f) },
	{ "ghostwhite", GVector4(0.973f, 0.973f, 1.000f, 1.000f) },
	{ "gold", GVector4(1.000f, 0.843f, 0.000f, 1.000f) },
	{ "goldenrod", GVector4(0.855f, 0.647f, 0.125f, 1.000f) },
	{ "gray", GVector4(0.502f, 0.502f, 0.502f, 1.000f) },
	{ "green", GVector4(0.000f, 0.502f, 0.000f, 1.000f) },
	{ "greenyellow", GVector4(0.678f, 1.000f, 0.184f, 1.000f) },
	{ "grey", GVector4(0.502f, 0.502f, 0.502f, 1.000f) },
	{ "honeydew", GVector4(0.941f, 1.000f, 0.941f, 1.000f) },
	{ "hotpink", GVector4(1.000f, 0.412f, 0.706f, 1.000f) },
	{ "indianred", GVector4(0.804f, 0.361f, 0.361f, 1.000f) },
	{ "indigo", GVector4(0.294f, 0.000f, 0.510f, 1.000f) },
	{ "ivory", GVector4(1.000f, 1.000f, 0.941f, 1.000f) },
	{ "khaki", GVector4(0.941f, 0.902f, 0.549f, 1.000f) },
	{ "lavender", GVector4(0.902f, 0.902f, 0.980f, 1.000f) },
	{ "lavenderblush", GVector4(1.000f, 0.941f, 0.961f, 1.000f) },
	{ "lawngreen", GVector4(0.486f, 0.988f, 0.000f, 1.000f) },
	{ "lemonchiffon", GVector4(1.000f, 0.980f, 0.804f, 1.000f) },
	{ "lightblue", GVector4(0.678f, 0.847f, 0.902f, 1.000f) },
	{ "lightcoral", GVector4(0.941f, 0.502f, 0.502f, 1.000f) },
	{ "lightcyan", GVector4(0.878f, 1.000f, 1.000f, 1.000f) },
	{ "lightgoldenrodyellow", GVector4(0.980f, 0.980f, 0.824f, 1.000f) },
	{ "lightgray", GVector4(0.827f, 0.827f, 0.827f, 1.000f) },
	{ "lightgreen", GVector4(0.565f, 0.933f, 0.565f, 1.000f) },
	{ "lightgrey", GVector4(0.827f, 0.827f, 0.827f, 1.000f) },
	{ "lightpink", GVector4(1.000f, 0.714f, 0.757f, 1.000f) },
	{ "lightsalmon", GVector4(1.000f, 0.627f, 0.478f, 1.000f) },
	{ "lightseagreen", GVector4(0.125f, 0.698f, 0.667f, 1.000f) },
	{ "lightskyblue", GVector4(0.529f, 0.808f, 0.980f, 1.000f) },
	{ "lightslategray", GVector4(0.467f, 0.533f, 0.600f, 1.000f) },
	{ "lightslategrey", GVector4(0.467f, 0.533f, 0.600f, 1.000f) },
	{ "lightsteelblue", GVector4(0.690f, 0.769f, 0.871f, 1.000f) },
	{ "lightyellow", GVector4(1.000f, 1.000f, 0.878f, 1.000f) },
	{ "lime", GVector4(0.000f, 1.000f, 0.000f, 1.000f) },
	{ "limegreen", GVector4(0.196f, 0.804f, 0.196f, 1.000f) },
	{ "linen", GVector4(0.980f, 0.941f, 0.902f, 1.000f) },
	{ "magenta", GVector4(1.000f, 0.000f, 1.000f, 1.000f) },
	{ "maroon", GVector4(0.502f, 0.000f, 0.000f, 1.000f) },
	{ "mediumaquamarine", GVector4(0.400f, 0.804f, 0.667f, 1.000f) },
	{ "mediumblue", GVector4(0.000f, 0.000f, 0.804f, 1.000f) },
	{ "mediumorchid", GVector4(0.729f, 0.333f, 0.827f, 1.000f) },
	{ "mediumpurple", GVector4(0.576f, 0.439f, 0.859f, 1.000f) },
	{ "mediumseagreen", GVector4(0.235f, 0.702f, 0.443f, 1.000f) },
	{ "mediumslateblue", GVector4(0.482f, 0.408f, 0.933f, 1.000f) },
	{ "mediumspringgreen", GVector4(0.000f, 0.980f, 0.604f, 1.000f) },
	{ "mediumturquoise", GVector4(0.282f, 0.820f, 0.800f, 1.000f) },
	{ "mediumvioletred", GVector4(0.780f, 0.082f, 0.522f, 1.000f) },
	{ "midnightblue", GVector4(0.098f, 0.098f, 0.439f, 1.000f) },
	{ "mintcream", GVector4(0.961f, 1.000f, 0.980f, 1.000f) },
	{ "mistyrose", GVector4(1.000f, 0.894f, 0.882f, 1.000f) },
	{ "moccasin", GVector4(1.000f, 0.894f, 0.710f, 1.000f) },
	{ "navajowhite", GVector4(1.000f, 0.871f, 0.678f, 1.000f) },
	{ "navy", GVector4(0.000f, 0.000f, 0.502f, 1.000f) },
	{ "oldlace", GVector4(0.992f, 0.961f, 0.902f, 1.000f) },
	{ "olive", GVector4(0.502f, 0.502f, 0.000f, 1.000f) },
	{ "olivedrab", GVector4(0.420f, 0.557f, 0.137f, 1.000f) },
	{ "orange", GVector4(1.000f, 0.647f, 0.000f, 1.000f) },
	{ "orangered", GVector4(1.000f, 0.271f, 0.000f, 1.000f) },
	{ "orchid", GVector4(0.855f, 0.439f, 0.839f, 1.000f) },
	{ "palegoldenrod", GVector4(0.933f, 0.910f, 0.667f, 1.000f) },
	{ "palegreen", GVector4(0.596f, 0.984f, 0.596f, 1.000f) },
	{ "paleturquoise", GVector4(0.686f, 0.933f, 0.933f, 1.000f) },
	{ "palevioletred", GVector4(0.859f, 0.439f, 0.576f, 1.000f) },
	{ "papayawhip", GVector4(1.000f, 0.937f, 0.835f, 1.000f) },
	{ "peachpuff", GVector4(1.000f, 0.855f, 0.725f, 1.000f) },
	{ "peru", GVector4(0.804f, 0.522f, 0.247f, 1.000f) },
	{ "pink", GVector4(1.000f, 0.753f, 0.796f, 1.000f) },
	{ "plum", GVector4(0.867f, 0.627f, 0.867f, 1.000f) },
	{ "powderblue", GVector4(0.690f, 0.878f, 0.902f, 1.000f) },
	{ "purple", GVector4(0.502f, 0.000f, 0.502f, 1.000f) },
	{ "red", GVector4(1.000f, 0.000f, 0.000f, 1.000f) },
	{ "rosybrown", GVector4(0.737f, 0.561f, 0.561f, 1.000f) },
	{ "royalblue", GVector4(0.255f, 0.412f, 0.882f, 1.000f) },
	{ "saddlebrown", GVector4(0.545f, 0.271f, 0.075f, 1.000f) },
	{ "salmon", GVector4(0.980f, 0.502f, 0.447f, 1.000f) },
	{ "sandybrown", GVector4(0.957f, 0.643f, 0.376f, 1.000f) },
	{ "seagreen", GVector4(0.180f, 0.545f, 0.341f, 1.000f) },
	{ "seashell", GVector4(1.000f, 0.961f, 0.933f, 1.000f) },
	{ "sienna", GVector4(0.627f, 0.322f, 0.176f, 1.000f) },
	{ "silver", GVector4(0.753f, 0.753f, 0.753f, 1.000f) },
	{ "skyblue", GVector4(0.529f, 0.808f, 0.922f, 1.000f) },
	{ "slateblue", GVector4(0.416f, 0.353f, 0.804f, 1.000f) },
	{ "slategray", GVector4(0.439f, 0.502f, 0.565f, 1.000f) },
	{ "slategrey", GVector4(0.439f, 0.502f, 0.565f, 1.000f) },
	{ "snow", GVector4(1.000f, 0.980f, 0.980f, 1.000f) },
	{ "springgreen", GVector4(0.000f, 1.000f, 0.498f, 1.000f) },
	{ "steelblue", GVector4(0.275f, 0.510f, 0.706f, 1.000f) },
	{ "tan", GVector4(0.824f, 0.706f, 0.549f, 1.000f) },
	{ "teal", GVector4(0.000f, 0.502f, 0.502f, 1.000f) },
	{ "thistle", GVector4(0.847f, 0.749f, 0.847f, 1.000f) },
	{ "tomato", GVector4(1.000f, 0.388f, 0.278f, 1.000f) },
	{ "turquoise", GVector4(0.251f, 0.878f, 0.816f, 1.000f) },
	{ "violet", GVector4(0.933f, 0.510f, 0.933f, 1.000f) },
	{ "wheat", GVector4(0.961f, 0.871f, 0.702f, 1.000f) },
	{ "white", GVector4(1.000f, 1.000f, 1.000f, 1.000f) },
	{ "whitesmoke", GVector4(0.961f, 0.961f, 0.961f, 1.000f) },
	{ "yellow", GVector4(1.000f, 1.000f, 0.000f, 1.000f) },
	{ "yellowgreen", GVector4(0.604f, 0.804f, 0.196f, 1.000f) }
};
#endif

/*
Radial fragment program
Parameters:	0 = [(Focus - Center).x, (Focus - Center).y, 0, 0]
			1 = [(F-C).LengthSquared - Radius^2, 0, 0, 0]
			2 = [a00, a01, a10, a11]
			3 = [a02, 0, a12, 0]
			4 = [1, 1, 1, Alpha]
			5 = texture u coordinate scaling
*/
static const char *const RadialProgram =
    "!!ARBfp1.0"
	"OPTION ARB_precision_hint_nicest;"
	"PARAM c[6] = { program.local[0..5] };"
    "TEMP R0;"
	"MAD R0, fragment.position.xyxy, c[2], c[3];"
	"ADD R0.x, R0.x, R0.y;"
	"ADD R0.y, R0.z, R0.w;"
	"MOV R0.zw, R0.xyxy;"
    "MUL R0.xy, R0, R0;"
    "MUL R0.zw, R0, c[0].xyxy;"
    "ADD R0.x, R0, R0.y;"
    "ADD R0.z, R0, R0.w;"
    "MOV R0.w, R0.z;"
    "MUL R0.w, R0, R0.w;"
	"MAD R0.w, -R0.x, c[1].x, R0.w;"
    "RSQ R0.w, R0.w;"
    "RCP R0.w, R0.w;"
	"ADD R0.z, -R0.z, R0.w;"
	"RCP R0.z, R0.z;"
	"MUL R0.x, R0, R0.z;"
	"MUL R0.x, R0, c[5];"
	"TEX R0, R0, texture[0], 1D;"
	"MUL result.color, R0, c[4];"
    "END"
    "\0";

/*
Conical fragment program
Parameters:	0 = [cos, sin, -sin, cos]
			1 = [Center.x, Center.y, 0, 0]
			2 = [0.5, 0.5, 0.5, 0.5]
			3 = [1, 1, 1, Alpha]
			4 = [a00, a01, a10, a11]
			5 = [a02, 0, a12, 0]
*/
static const char *const ConicalProgram =
    "!!ARBfp1.0"
	"OPTION ARB_precision_hint_nicest;"
	"PARAM c[6] = { program.local[0..5] };"
    "TEMP R0;"
	"MAD R0, fragment.position.xyxy, c[4], c[5];"
	"ADD R0.x, R0.x, R0.y;"
	"ADD R0.y, R0.z, R0.w;"
	"SUB R0, R0.xyxy, c[1].xyxy;"
	"MUL R0, R0, c[0];"
	"ADD R0.xy, R0.x, R0.y;"
	"ADD R0.zw, R0.z, R0.w;"
	"MUL R0.x, R0.x, R0.x;"
	"MAD R0.z, R0.z, R0.z, R0.x;"
	"RSQ R0.z, R0.z;"
	"MUL R0, R0, R0.z;"
	"MAD R0, R0, c[2], c[2];"
	"TEX R0, R0.ywyw, texture[0], 2D;"
	"MUL result.color, R0, c[3];"
    "END"
    "\0";

static const char HTMLColorMask[] = "0123456789abcdef";

// atan2 lookup table
void GOpenGLBoard::GenerateAtan2LookupTable() {

	GInt32 size = gAtan2LookupTableSize;
	GFloat halfSizef = (GFloat)(size / 2);
	GInt32 sizeSqr = size * size;
	GInt32 ofs0 = 0;
	gAtan2LookupTable = new(std::nothrow) GFloat[sizeSqr];
	G_ASSERT(gAtan2LookupTable != NULL);

	for (GInt32 y = 0; y < size; y++) {
		GFloat ry = (GFloat)y - halfSizef;
		for (GInt32 x = 0; x < size; x++) {
			GFloat rx = (GFloat)x - halfSizef;
			GFloat atan2Val = GMath::Atan2(ry, rx);
			if (atan2Val < 0)
				atan2Val = (GFloat)G_2PI + atan2Val;
			GInt32 ofs = ofs0 + x;
			G_ASSERT(ofs >= 0 && ofs < sizeSqr);
			gAtan2LookupTable[ofs] = atan2Val / (GFloat)G_2PI;
		}
		ofs0 += size;
	}
}

void GOpenGLBoard::SetShadersEnabled(const GBool Enabled) {

	if (!gExtManager->IsArbProgramsSupported() || (Enabled == gFragmentProgramsSupport))
		return;

	gFragmentProgramsSupport = Enabled;
	// we have to mark all gradients as modified
	if (Enabled == G_TRUE) {

		GUInt32 i, j = (GUInt32)gGradients.size();
		for (i = 0; i < j; i++) {
			GOpenGLGradientDesc *g = gGradients[i];
			g->SetColorInterpolationModified(G_TRUE);
		}
	}

}

void GOpenGLBoard::SetRectTextureEnabled(const GBool Enabled) {

	// we can't change it inside group (during a group drawing)
	if (InsideGroup() && TargetMode() == G_COLOR_MODE)
		return;

	if (!gExtManager->IsRectTextureSupported() || (Enabled == gRectTexturesSupport))
		return;

	gRectTexturesSupport = Enabled;
}

GOpenGLBoard::GOpenGLBoard(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY,
						   const GUInt32 Width, const GUInt32 Height) : GDrawBoard() {

	gExtManager = new(std::nothrow) GOpenglExt();
	GUInt32 stencilBits = gExtManager->StencilBits();

	InitDrawStyle();

	// verify if we can do group opacity
	if (stencilBits < 4)
		gGroupOpacitySupport = G_FALSE;
	else
		gGroupOpacitySupport = G_TRUE;

	// verify if we can clip using stencil buffer
	if (stencilBits >= 4) {
		gClipMasksSupport = G_TRUE;
		// for example, if we have an 8bit stencil buffer, stencil mask will be 127
		gStencilMask = (1 << (stencilBits - 1)) - 1;
		gStencilDualMask = (~gStencilMask);
		// we must reserve 3 masks for internal use
		gMaxTopStencilValue = gStencilMask - 3;
	}
	else
		gClipMasksSupport = G_FALSE;

	gTopStencilValue = 0;
	// set "old" state of clipping operations
	gFirstClipMaskReplace = G_FALSE;
	// set a flag that tells if, when we are inside a group, something has been drawn
	gIsFirstGroupDrawing = G_FALSE;

	// extract mirrored repeat support
	gMirroredRepeatSupport = gExtManager->IsMirroredRepeatSupported();

	// set current cache slot to NULL
	gCacheSlot = NULL;

	// ask for number of multisamples
	if (gExtManager->MultiSamples() > 0)
		gMultiSamplePresent = G_TRUE;
	else
		gMultiSamplePresent = G_FALSE;

	// fragment programs support
	gFragmentProgramsSupport = gExtManager->IsArbProgramsSupported();
	// rectangular textures support
	gRectTexturesSupport = G_FALSE;
	const GChar8 *vendorStr = NULL;
	vendorStr = (const GChar8 *)glGetString(GL_VENDOR);
	if (vendorStr) {
		// from our tests, it seems that NVidia cards have good rectangular textures support
		if (StrUtils::Find(vendorStr, "NVIDIA"))
			gRectTexturesSupport = gExtManager->IsRectTextureSupported();
	}

	gAtan2LookupTable = NULL;
	gAtan2LookupTableSize = 256;
	gRadGradGLProgram = 0;
	gConGradGLProgram = 0;
	if (gFragmentProgramsSupport) {
		// generate radial gradient fragment program
		glGenProgramsARB(1, &gRadGradGLProgram);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gRadGradGLProgram);
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
							(GLsizei)std::strlen(RadialProgram), (const GLbyte *)RadialProgram);
		// check for errors
		if (GL_INVALID_OPERATION == glGetError()) {
			// find the error position
			GLint errPos;
			glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errPos);
			const GLubyte *errString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			if (errString) {
				G_DEBUG((const GChar8 *)errString);
			}
		}
		// generate atan2 lookup table
		GenerateAtan2LookupTable();
		// generate conical gradient fragment program
		glGenProgramsARB(1, &gConGradGLProgram);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gConGradGLProgram);
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
							(GLsizei)std::strlen(ConicalProgram), (const GLbyte *)ConicalProgram);
		// check for errors
		if (GL_INVALID_OPERATION == glGetError()) {
			// find the error position
			GLint errPos;
			glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errPos);
			const GLubyte *errString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			if (errString) {
				G_DEBUG((const GChar8 *)errString);
			}
		}
	}

	// build HTML validator mask (for color characters)
	BuildHTMLMask();

	// set logical and physical viewports
	SetViewport(LowLeftCornerX, LowLeftCornerY, Width, Height);
	SetProjection((GReal)LowLeftCornerX, (GReal)(LowLeftCornerX + Width), (GReal)LowLeftCornerY, (GReal)(LowLeftCornerY + Height)); 
	// set rendering quality to normal, as default
	SetRenderingQuality(G_NORMAL_RENDERING_QUALITY);

	glDisable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

	// useful fields for SVG-like drawing functions
	gOldPointsSize = 0;
	gInsideSVGPath = G_FALSE;
	gInsideSVGPath = G_FALSE;
}

GOpenGLBoard::~GOpenGLBoard() {

	DeleteGradients();
	DeletePatterns();
	DeleteCacheSlots();

	if (gAtan2LookupTable)
		delete [] gAtan2LookupTable;
	if (gExtManager)
		delete gExtManager;
}
//----------------------------- CACHE

void GOpenGLBoard::DoDrawCacheEntry(const GDrawStyle& Style, const GOpenGLCacheEntry& CacheEntry) {

	if (CacheEntry.FillDisplayList == 0 && CacheEntry.StrokeDisplayList == 0)
		return;

	#define DRAW_STROKE \
		G_ASSERT(CacheEntry.StrokeDisplayList != 0); \
		glCallList(CacheEntry.StrokeDisplayList);

	#define DRAW_FILL \
		if (CacheEntry.FillDisplayList != 0) \
			glCallList(CacheEntry.FillDisplayList);

	// calculate bound box
	GAABox2 tmpBox(CacheEntry.Box);

	GBool doublePass = SetGLClipEnabled(TargetMode(), ClipOperation());

	if (TargetMode() == G_CLIP_MODE) {

		// draw fill
		if (Style.FillEnabled()) {
			DRAW_FILL
		}
		// draw stroke
		if (Style.StrokeEnabled()) {
			DRAW_STROKE
		}
		// take care of replace operation
		if (!InsideGroup())
			UpdateClipMasksState();

		if (!InsideGroup())
			gClipMasksBoxes.push_back(tmpBox);
		else {
			// build initial group box
			if (gIsFirstGroupDrawing)
				gGroupBox = tmpBox;
			else {
				// expand group box
				gGroupBox.ExtendToInclude(tmpBox.Min());
				gGroupBox.ExtendToInclude(tmpBox.Max());
			}
		}
		gIsFirstGroupDrawing = G_FALSE;
		return;
	}

	// in color mode, if we are inside a GroupBegin() / GroupEnd() constructor and group opacity is 0
	// do not draw anything
	if (InsideGroup() && GroupOpacity() <= 0 && gGroupOpacitySupport)
		return;

	if (Style.FillEnabled()) {
		// set fill style using OpenGL
		GBool useDepth = UseFillStyle(Style);
		// take care of group opacity, first we have to write into stencil buffer
		if (doublePass) {
			GroupFirstPass();
			// draw fill
			DRAW_FILL
			StencilEnableTop();
		}
		if (useDepth)
			PushDepthMask();
		// draw fill
		DRAW_FILL
		// geometric radial gradient and transparent entities uses depth clip, so we must pop off clip mask
		if (useDepth)
			DrawAndPopDepthMask(tmpBox, Style, G_TRUE);
	}

	// take care to enable stencil test if necessary
	SetGLClipEnabled(TargetMode(), ClipOperation());

	if (Style.StrokeEnabled()) {
		// set stroke style using OpenGL
		GBool useDepth = UseStrokeStyle(Style);
		// take care of group opacity, first we have to write into stencil buffer
		if (doublePass) {
			GroupFirstPass();
			// draw stroke
			DRAW_STROKE
			StencilEnableTop();
		}
		if (useDepth)
			PushDepthMask();
		// draw stroke
		DRAW_STROKE
		// geometric radial gradient and transparent entities uses depth clip, so we must pop off clip mask
		if (useDepth)
			DrawAndPopDepthMask(tmpBox, Style, G_FALSE);
	}

	#undef DRAW_STROKE
	#undef DRAW_FILL
}

void GOpenGLBoard::DoDrawCacheEntries(GDrawStyle& Style, const GUInt32 FirstEntryIndex, const GUInt32 LastEntryIndex) {

	if (!Style.StrokeEnabled() && !Style.FillEnabled())
		return;

	// update style
	UpdateStyle((GOpenGLDrawStyle&)Style);

	GOpenGLCachedDrawing *slot = (GOpenGLCachedDrawing *)CacheSlot();
	G_ASSERT(slot != NULL);

	for (GUInt32 i = FirstEntryIndex; i <= LastEntryIndex; ++i)
		DoDrawCacheEntry(Style, slot->gEntries[i]);
}

GCachedDrawing *GOpenGLBoard::CreateCacheSlot() {

	GOpenGLCachedDrawing *slot = new(std::nothrow) GOpenGLCachedDrawing();
	if (slot)
		gCacheSlots.push_back(slot);
	return (GCachedDrawing *)slot;
}

GCachedDrawing *GOpenGLBoard::CacheSlot() const {

	return (GCachedDrawing *)gCacheSlot;
}

void GOpenGLBoard::SetCacheSlot(GCachedDrawing *Slot) {

	gCacheSlot = (GOpenGLCachedDrawing *)Slot;
}
//----------------------------- CACHE

void GOpenGLBoard::DeleteGradients() {

	GDynArray<GOpenGLGradientDesc *>::iterator it = gGradients.begin();

	for (; it != gGradients.end(); ++it) {
		GOpenGLGradientDesc *gradient = *it;
		G_ASSERT(gradient);
		delete gradient;
	}
	gGradients.clear();

	if (gFragmentProgramsSupport) {
		glDeleteProgramsARB(1, &gRadGradGLProgram);
		glDeleteProgramsARB(1, &gConGradGLProgram);
	}
}

void GOpenGLBoard::DeletePatterns() {

	GDynArray<GOpenGLPatternDesc *>::iterator it = gPatterns.begin();

	for (; it != gPatterns.end(); ++it) {
		GOpenGLPatternDesc *pattern = *it;
		G_ASSERT(pattern);
		delete pattern;
	}
	gPatterns.clear();
}

void GOpenGLBoard::DeleteCacheSlots() {

	GDynArray<GOpenGLCachedDrawing *>::iterator it = gCacheSlots.begin();

	for (; it != gCacheSlots.end(); ++it) {
		GOpenGLCachedDrawing *slot = *it;
		G_ASSERT(slot);
		delete slot;
	}
	gCacheSlots.clear();
}

// read only parameters
GUInt32 GOpenGLBoard::MaxDashCount() const {

	return G_MAX_UINT16;
}

/*GUInt32 GOpenGLBoard::MaxKernelSize() const {

	if (gExtManager)
		return GMath::Min(gExtManager->MaxConvolutionWidth(), gExtManager->MaxConvolutionHeight());
	else
		return 0;
}

GUInt32 GOpenGLBoard::MaxSeparableKernelSize() const {

	return MaxKernelSize();
}*/

GUInt32 GOpenGLBoard::MaxColorKeys() const {

	return MaxImageWidth();
}

GUInt32 GOpenGLBoard::MaxImageWidth() const {

	if (gExtManager)
		return gExtManager->MaxTextureSize();
	else
		return 0;
}

GUInt32 GOpenGLBoard::MaxImageHeight() const {

	if (gExtManager)
		return gExtManager->MaxTextureSize();
	else
		return 0;
}

GUInt32 GOpenGLBoard::MaxImageBytes() const {

	return (MaxImageWidth() * MaxImageHeight() * 4);
}

void GOpenGLBoard::UpdateDeviation(const GRenderingQuality Quality) {

	#define LOW_QUALITY_PIXEL_DEVIATION (1.4 * 1.4)
	#define NORMAL_QUALITY_PIXEL_DEVIATION (0.5 * 0.5)
	#define HIGH_QUALITY_PIXEL_DEVIATION (0.2 * 0.2)

	switch (Quality) {

		case G_LOW_RENDERING_QUALITY:
			gDeviation = CalcDeviation(LOW_QUALITY_PIXEL_DEVIATION);
			break;

		case G_NORMAL_RENDERING_QUALITY:
			gDeviation = CalcDeviation(NORMAL_QUALITY_PIXEL_DEVIATION);
			break;

		case G_HIGH_RENDERING_QUALITY:
			gDeviation = CalcDeviation(HIGH_QUALITY_PIXEL_DEVIATION);
			break;
	}
	G_ASSERT(gDeviation > 0);
	gFlateness = GMath::Sqrt(gDeviation);

	#undef LOW_QUALITY_PIXEL_DEVIATION
	#undef NORMAL_QUALITY_PIXEL_DEVIATION
	#undef HIGH_QUALITY_PIXEL_DEVIATION
}

void GOpenGLBoard::DoSetRenderingQuality(const GRenderingQuality Quality) {

	UpdateDeviation(Quality);
	if (gMultiSamplePresent) {
		if (Quality == G_LOW_RENDERING_QUALITY)
			glDisable(GL_MULTISAMPLE_ARB);
		else
			glEnable(GL_MULTISAMPLE_ARB);
	}
}

void GOpenGLBoard::DoSetTargetMode(const GTargetMode Mode) {

	// just to avoid warning
	if (Mode) {
	}
}

void GOpenGLBoard::DoSetClipOperation(const GClipOperation Operation) {

	// just to avoid warning
	if (Operation) {
	}
}

void GOpenGLBoard::DoSetClipEnabled(const GBool Enabled) {

	// just to avoid warning
	if (Enabled) {
	}
}

void GOpenGLBoard::DoSetGroupOpacity(const GReal Opacity) {

	// just to avoid warning
	if (Opacity) {
	}
}

void GOpenGLBoard::DoFlush() {

	glFlush();
}

void GOpenGLBoard::DoFinish() {

	glFinish();
}

void GOpenGLBoard::DoClear(const GReal Red, const GReal Green, const GReal Blue, const GBool ClearClipMasks) {

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	if (gClipMasksSupport) {
		glClearColor((GLclampf)Red, (GLclampf)Green, (GLclampf)Blue, 1.0f);
		glClearDepth(1.0);
		if (ClearClipMasks) {
			glClearStencil((GLint)0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			gTopStencilValue = 0;
			gClipMasksBoxes.clear();
		}
		else
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	else {
		glClearColor((GLclampf)Red, (GLclampf)Green, (GLclampf)Blue, 1.0f);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

GBool GOpenGLBoard::SetGLClipEnabled(const GTargetMode Mode, const GClipOperation Operation) {

	if (gClipMasksSupport) {

		// write to the stencil using current clip operation
		if (Mode == G_CLIP_MODE) {

			switch (Operation) {
				case G_REPLACE_CLIP:
					// take care of an overflow into masks stack
					if (gTopStencilValue >= gMaxTopStencilValue) {
						glClearStencil((GLint)0);
						glClear(GL_STENCIL_BUFFER_BIT);
						gTopStencilValue = 0;
					}
					StencilReplace();
					break;

				case G_INTERSECTION_CLIP:
					StencilPush();
					break;
			}
			return G_FALSE;
		}
		// color mode
		else {
			if (!InsideGroup()) {
				StencilEnableTop();
				return G_FALSE;
			}
			else {
				if (GroupOpacity() < 1 && GroupOpacity() > 0 && !gGLGroupRect.IsEmpty && gGroupOpacitySupport) {
					// we have to do a double-pass algorithm (first write into stencil, then into color buffer)
					return G_TRUE;
				}
				else {
					StencilEnableTop();
					return G_FALSE;
				}
			}
		}
	}
	return G_FALSE;
}

void GOpenGLBoard::DoSetViewport(const GUInt32 LowLeftCornerX, const GUInt32 LowLeftCornerY,
								 const GUInt32 Width, const GUInt32 Height) {

	 glViewport(LowLeftCornerX, LowLeftCornerY, Width, Height);
	 UpdateDeviation(RenderingQuality());
}

void GOpenGLBoard::DoSetProjection(const GReal Left, const GReal Right, const GReal Bottom, const GReal Top) {

/*	
	To better understand this is an equivalent code to glOrtho

	GMatrix44 m;

	m[0][0] = 2.0 / (Right - Left);
	m[0][3] = -(Right + Left) / (Right - Left);

	m[1][1] = 2.0 / (Top - Bottom);
	m[1][3] = -(Top + Bottom) / (Top - Bottom);

	GReal zNear = -1;
	GReal zFar = 0;

	m[2][2] = -2 / (zFar - zNear);
	m[2][3] = (-(zFar + zNear) / (zFar - zNear));*/

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho((GLdouble)Left, (GLdouble)Right, (GLdouble)Bottom, (GLdouble)Top, (GLdouble)-1, (GLdouble)0);

	UpdateDeviation(RenderingQuality());
}

// calculate (squared) deviation given a (squared) pixel deviation
GReal GOpenGLBoard::CalcDeviation(const GReal PixelDeviation) {

	GReal rX = GMath::Abs(gProjection[G_X] - gProjection[G_Y]) / gViewport[G_Z];
	GReal rY = GMath::Abs(gProjection[G_Z] - gProjection[G_W]) / gViewport[G_W];

	return (PixelDeviation * GMath::Sqr(GMath::Min(rX, rY)));
}

// calculate (squared) pixel deviation given a (squared) deviation
GReal GOpenGLBoard::CalcPixelDeviation(const GReal Deviation) {

	GReal rX = gViewport[G_Z] / GMath::Abs(gProjection[G_X] - gProjection[G_Y]);
	GReal rY = gViewport[G_W] / GMath::Abs(gProjection[G_Z] - gProjection[G_W]);

	return (Deviation * GMath::Sqr(GMath::Min(rX, rY)));
}

void GOpenGLBoard::BuildHTMLMask() {

	std::memset(gHTMLMask, 0, 32);
	const GChar8 *charSet = HTMLColorMask;

	while (*charSet) {
		unsigned c = unsigned(*charSet++) & 0xFF;
		gHTMLMask[c >> 3] |= 1 << (c & 7);
	}
}

GBool GOpenGLBoard::IsValidHTMLColorChar(const GUChar8 Char) {

	return (gHTMLMask[(Char >> 3) & (31)] & (1 << (Char & 7))) != 0;
}

int CompareSVGColor(const void* Col1, const void* Col2)
{
	return strcmp(((GNamedSVGcolor *)Col1)->Name, ((GNamedSVGcolor *)Col2)->Name);
}

GVector4 GOpenGLBoard::ColorFromString(const GString& Color) {

	if (Color.length() <= 0)
		return GVector4(0, 0, 0, 1);

	#define HEX1REAL(hex, output) \
		if (IsValidHTMLColorChar(hex)) \
			strNum[2] = hex; \
		else \
			strNum[2] = '0'; \
		strNum[3] = '0'; \
		intValue = 0; \
		sscanf(strNum, "%x", &intValue); \
		output = (GReal)intValue / (GReal)255;

	#define HEX2REAL(hex1, hex2, output) \
		if (IsValidHTMLColorChar(hex1)) \
			strNum[2] = hex1; \
		else \
			strNum[2] = '0'; \
		if (IsValidHTMLColorChar(hex2)) \
			strNum[3] = hex2; \
		else \
			strNum[3] = '0'; \
		intValue = 0; \
		sscanf(strNum, "%x", &intValue); \
		output = (GReal)intValue / (GReal)255;


	GString lowerStr = StrUtils::Lower(Color);
	GUInt32 l = (GUInt32)lowerStr.length();
	GUChar8 *cStr = (GUChar8 *)StrUtils::ToAscii(lowerStr);
	GChar8 strNum[5] = { '0', 'x', '0', '0', 0 };
	GUInt32 intValue;

	GReal red, green, blue, alpha;

	// HTML color
	if (cStr[0] == '#') {
		switch (l) {
			// test for #RRGGBBAA
			case 9:
				HEX2REAL(cStr[1], cStr[2], red)
				HEX2REAL(cStr[3], cStr[4], green)
				HEX2REAL(cStr[5], cStr[6], blue)
				HEX2REAL(cStr[7], cStr[8], alpha)
				return GVector4(red, green, blue, alpha);
				break;

			// test for #RRGGBB
			case 7:
				HEX2REAL(cStr[1], cStr[2], red)
				HEX2REAL(cStr[3], cStr[4], green)
				HEX2REAL(cStr[5], cStr[6], blue)
				return GVector4(red, green, blue, 1);
				break;
			// test for #RGBA
			case 5:
				HEX1REAL(cStr[1], red)
				HEX1REAL(cStr[2], green)
				HEX1REAL(cStr[3], blue)
				HEX1REAL(cStr[4], alpha)
				return GVector4(red, green, blue, alpha);
				break;

			// test for #RGB
			case 4:
				HEX1REAL(cStr[1], red)
				HEX1REAL(cStr[2], green)
				HEX1REAL(cStr[3], blue)
				return GVector4(red, green, blue, 1);
				break;

			default:
				return GVector4(0, 0, 0, 1);
		}
	}
	// try with SVG keyword
	else {
		GNamedSVGcolor col;
		std::strncpy(col.Name, (const GChar8 *)cStr, 21);
		GNamedSVGcolor* foundCol = (GNamedSVGcolor *)std::bsearch(&col, SVGColors, 147, sizeof(GNamedSVGcolor), CompareSVGColor);
		if (foundCol == NULL)
			return GVector4(0, 0, 0, 1);
		else
			return foundCol->RGBA;

	}
	#undef HEX1REAL
	#undef HEX2REAL
}

GError GOpenGLBoard::DoScreenShot(GPixelMap& Output, const GVectBase<GUInt32, 2>& P0, const GVectBase<GUInt32, 2>& P1) const {

	GUInt32 w = P1[G_X] - P0[G_X] + 1;
	GUInt32 h = P1[G_Y] - P0[G_Y] + 1;

	GError err = Output.Create((GInt32)w, (GInt32)h, G_A8R8G8B8);
	if (err == G_NO_ERROR) {
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);

		glReadPixels((GLint)P0[G_X], (GLint)P0[G_Y], (GLsizei)w, (GLsizei)h, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLvoid *)Output.Pixels()); 
    	err = Output.Flip(G_FALSE, G_TRUE);
	}
	return err;
}

};	// end namespace Amanith
