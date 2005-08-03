/****************************************************************************
** $file: amanith/2d/gpixelmap.h   0.1.0.0   edited Jun 30 08:00
**
** 2D Pixelmap definition.
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

#ifndef GPIXELMAP_H
#define GPIXELMAP_H

/*!
	\file gpixelmap.h
	\brief Header for GPixelMap class.
*/

#include "amanith/gelement.h"

namespace Amanith {

	// *********************************************************************
	//                            GPixelMap
	// *********************************************************************

	//! GPixelMap static class descriptor.
	static const GClassID G_PIXELMAP_CLASSID = GClassID("GPixelMap", 0xDA2A436E, 0x92E44BA3, 0xB1D35233, 0x6E81A5CF);

	//! Supported pixel formats
	enum GPixelFormat {
		//! 8 bits grayscale pixel format
		G_GRAYSCALE,
		//! 8 bits RGB paltetted pixel format
		G_RGB_PALETTE,
		//! 24 bits RGB pixel format (8 bits per channel)
		G_R8G8B8,
		//! 32 bits ARGB pixel format (8 bits per channel)
		G_A8R8G8B8,
		//! 16 bits ARGB pixel format (1 bit for alpha, 5 bits for R, G and B channels)
		G_A1R5G5B5,
		//! 16 bits RGB pixel format (5 bits for red, 6 bits for green, and 5 bits for blue)
		G_R5G6B5
	};

	//! Available filters for bitmap resize operation
	enum GResizeOp {
		//! Box filter
		G_RESIZE_BOX,
		//! Triangle filter
		G_RESIZE_TRIANGLE,
		//! 3rd order (quadratic) b-spline filter
		G_RESIZE_QUADRATIC,
		//! 4th order (cubic) b-spline filter
		G_RESIZE_CUBIC,
		//! Catmull-Rom spline, Overhauser spline filter
		G_RESIZE_CATMULLROM,
		//! Gaussian filter
		G_RESIZE_GAUSSIAN,
		//! Sinc, perfect low-pass filter
		G_RESIZE_SINC,
		//! Bessel filter
		G_RESIZE_BESSEL,
		//! Bell filter
		G_RESIZE_BELL,
		//! Hanning filter
		G_RESIZE_HANNING,
		//! Hamming filter
		G_RESIZE_HAMMING,
		//! Black-man filter
		G_RESIZE_BLACKMAN,
		//! Normal distribution filter (Normal(x) = Gaussian(x/2) / 2)
		G_RESIZE_NORMAL
	};

	/*!
		\class GPixelMap
		\brief A 2D cross-platform bitmap class.

		The GPixelMap class provides a hardware-independent pixmap representation with direct access to the pixel data.
		An image has the parameters width, height and color depth (bits per pixel), a color table and the actual pixels.
		GPixelMap supports 8-bpp (grayscale and paletted formats), 16-bpp, 24 and 32 bpp image data.
		Here are details of internal formats:

		- 8-bpp images: they can be grayscale (G_GRAYSCALE) or paletted (G_RGB_PALETTE). In both cases
		each pixel is 1 byte. In grayscale bitmpas, the	pixel value represent a gray level (form 0 to 255). For paletted
		bitmaps, the pixel value is a color table index (from 0 to 255). Each palette entry is maintained in memory as a
		single (4 bytes long) integer, where the most significant byte is an optional alpha value, then
		come (going down to the least significant byte) 8bit red value, 8bit green value and finally 8bit blue value.\n\n
		- 16-bpp images: they can be with (G_A1R5G5B5) and without (G_R5G6B5) alpha channel. In the first case the most significant
		bit is an alpha value, and then come (from most to least significants bits positions) 5bits red, 5bits green and
		5 bits blue values.\n\n
		- 24/32-bpp images: they can be with (G_A8R8G8B8) and without (G_R8G8B8) alpha channel. In the first case the
		most significant byte is an 8bits alpha value, and then come (from most to least significant bytes) 8bits red, 8bits
		green and 8 bits blue values. Internally each pixel is always maintained as a (4 bytes long) integer.
	*/
	class G_EXPORT GPixelMap : public GElement {

	private:
		//! The pointer to the first pixel.
		GUChar8 *gPixels;
		//! The pointer to the first palette entry. It can be NULL (for full color images, for examples).
		GUInt32 *gPalette;
		//! The current pixel format.
		GPixelFormat gPixelFormat;
		//! The width of this image.
		GUInt32 gWidth;
		//! The height of this image.
		GUInt32 gHeight;

	protected:
		//! Cloning function, copies (physically) a Source bitmap curve into this image.
		GError BaseClone(const GElement& Source);
		//! Returns how many bytes are needed to store pixels, specifying a bitmap dimensions and pixel format
		static GInt32 NeededBytes(const GInt32 _Width, const GInt32 _Height, const GPixelFormat PixelFormat);
		//! Returns how many bytes are needed to store a color palette, specifying the pixel format
		static GInt32 PaletteSize(const GPixelFormat PixelFormat);
		//! Do a reference copy, "moving" only pixels and palette pointers from Source parameter.
		void ReferenceMove(GPixelMap& Source);
		//! Free the memory occupied by pixels and palette, then reset internal dimensions to 0.
		void Reset();
		//! Build a brightness LUT (lookup table), used by edge preserving smooth filter
		static void BuildFiltersBrightnessLUT(GUChar8 *LUT, const GUInt32 Threshold, const GBool SixDeg);
		//! Trace contours, using the fast schema
		static GError TraceContoursFast(GUChar8 *in, GInt32 *r, GUChar8 *mid, GUChar8 *bp,
										GInt32 max_no, GInt32 x_size, GInt32 y_size);
		//! Trace contours, using the accurate schema
		static GError TraceContoursAccurate(GUChar8 *in, GInt32 *r, GUChar8 *mid, GUChar8 *bp,
											GInt32 max_no, GInt32 x_size, GInt32 y_size);
		/*!
			Do a thinning operation. Some basic informations about thinning can be found
			here: http://www.mip.sdu.dk/ipl98/how_to_use_ipl98/tutorial/node52.html
		*/
		static GError ThinContours(GInt32 *r, GUChar8 *mid, GInt32 x_size, GInt32 y_size);

		/*!
			Pick a pixels column from image.

			\param Index the column index, must be valid.
			\param StartY the y coordinate where to begin pixel copy
			\param EndY the y coordinate where to end pixel copy
			\param Pixels the output buffer where to put pixels
		*/
		void ExtractColumn(const GUInt32 Index, const GUInt32 StartY, const GUInt32 EndY, GDynArray<GUChar8>& Pixels) const;
		/*!
			Pick a pixels row form image.

			\param Index the row index, must be valid.
			\param StartX the x coordinate where to begin pixel copy
			\param EndX the x coordinate where to end pixel copy
			\param Pixels the output buffer where to put pixels
		*/
		void ExtractRow(const GUInt32 Index, const GUInt32 StartX, const GUInt32 EndX, GDynArray<GUChar8>& Pixels) const;

		/*!
			Resize a full color pixmap, using digital filters.

			\param NewWidth the new width, must be positive.
			\param NewHeight the new height, must be positive.
			\param FullColorSource the source image.
			\param ResizedPixelMap the destination (resized) image.
			\param ResizeOp the operation (filter) used during resizing
			\return G_NO_ERROR is operation succeeds, an error code otherwise.
		*/
		static GError ResizeRGB(const GUInt32 NewWidth, const GUInt32 NewHeight, const GPixelMap& FullColorSource,
								GPixelMap& ResizedPixelMap,	const GResizeOp ResizeOp);

		/*!
			Resize a grayscale pixmap, using digital filters.

			\param NewWidth the new width, must be positive.
			\param NewHeight the new height, must be positive.
			\param GraySource the source image.
			\param ResizedPixelMap the destination (resized) image.
			\param ResizeOp the operation (filter) used during resizing
			\return G_NO_ERROR is operation succeeds, an error code otherwise.
		*/
		static GError ResizeGray(const GUInt32 NewWidth, const GUInt32 NewHeight, const GPixelMap& GraySource,
								 GPixelMap& ResizedPixelMap, const GResizeOp ResizeOp);

		/*!
			Apply a top-bottom hat filter to a grayscale image.

			The implementation uses two filter versions (regulated by Strong parameter). Their respective matrices are:

			- Soft version
				\f[
				bottom = \left( \begin{array}{ccc}
				1 & 0 & 1 \\
				0 & -4 & 0 \\
				1 & 0 & 1 \end{array} \right)
				top = \left( \begin{array}{ccc}
				-1 & 0 & -1 \\
				0 & 4 & 0 \\
				-1 & 0 & -1 \end{array} \right)
				\f]
			- Strong version
				\f[
				bottom = \left( \begin{array}{ccc}
				1 & 1 & 1 \\
				1 & -8 & 1 \\
				1 & 1 & 1 \end{array} \right)
				top = \left( \begin{array}{ccc}
				-1 & -1 & -1 \\
				-1 & 8 & -1 \\
				-1 & -1 & -1 \end{array} \right)
				\f]

			\param Divisor the divisor used, must be positive. The more the value is, the less is edge
			enhancement.
			\param Strong if G_TRUE the strong filter version is used, else the soft one.
		*/
		GError EdgeEnhanceMono(const GInt32 Divisor, const GBool Strong);
		/*!
			Apply an edge-preservin smooth filter to a grayscale image.

			\param DistThres distance threshold, must be positive
			\param BrighThres brightness threshold, must be between 0 and 255; it determines the maximum difference in
			gray levels between two pixels which allows them to be considered part of the same "region" in the image.
			\param Fast if G_TRUE the fast schema (1 radius mask) is used, else an accurate mask in accord to DistThres
			parameter is used.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note this filter is based on Susan, and can be applied just to grayscale images.
		*/
		GError EdgePreservingSmoothMono(const GInt32 DistThres, const GInt32 BrighThres, const GBool Fast);
		/*!
			Trace contours of a grayscale image.

			\param BrighThres brightness threshold, must be between 0 and 255; it determines the maximum difference in
			gray levels between two pixels which allows them to be considered part of the same "region" in the image.
			\param Thinning if G_TRUE, after tracing contours are also thinned.
			\param Fast if G_TRUE the fast schema is used, else an accurate mask (slow but accurate schema) is used.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note this filter is based on Susan, and can be applied just to grayscale images.
		*/
		GError TraceContoursMono(const GInt32 BrighThres, const GBool Thinning, const GBool Fast);

	public:
		//! Default constructor.
		GPixelMap();
		//! Constructor with owner (kernel) parameter.
		GPixelMap(const GElement* Owner);
		//! Destructor, free memory allocated for pixels and palette.
		~GPixelMap();
		//! Bitmap width, expressed in pixel.
		inline GInt32 Width() const {
			return gWidth;
		}
		//! Bitmap height, expressed in pixel.
		inline GInt32 Height() const {
			return gHeight;
		}
		//! Get current pixel format.
		inline GPixelFormat PixelFormat() const {
			return gPixelFormat;
		}
		//! Number of pixels, equal to Width() * Height().
		GInt32 PixelsCount() const;
		//! Number of palette entries. If image is not paletted, zero will be returned.
		inline GInt32 PaletteEntriesCount() const {
			return (this->PaletteSize() / sizeof(GUInt32));
		}
		//!	Number of bits used to encode a single pixel, also called bits per pixel (bpp) or bit planes of an image.
		GInt32 BitsPerPixel() const;
		//! number of bytes used to encode a single pixel.
		GInt32 BytesPerPixel() const;
		//! Number of bytes per scanline (a scanline is a row of pixels).
		GInt32 BytesPerLine() const;
		//! Number of bytes allocated by the image data (pixels).
		GInt32 Size() const;
		//! Number of bytes allocated by the colors palette.
		GInt32 PaletteSize() const;
		//! returns if the image has an alpha channel.
		GBool HasAlphaChannel() const;
		//! Returns if the image is paletted (it has an associated palette).
		GBool IsPaletted() const;
		//! Returns if image is gray scale.
		GBool IsGrayScale() const;
		//! Returns if image is high color (15bits or 16bits).
		GBool IsHighColor() const;
		//! Returns if image is truecolor (24 or 32 bits).
		GBool IsTrueColor() const;
		/*!
			Clear (fill) the image with a specified color.

			\param Index_Or_A8R8G8B8 the color used for filling.\n
			If the image is an 8 bit image, this value represents an index (between 0 and 255). If the image is an
			high-color, this value represent a full 32bit ARGB color, that will be automatically converted into the
			appropriate 15/16 bit value. If the image is a full 24/32 bits image, this value is the direct value that
			will be written to all pixels.
		*/
		void Clear(const GUInt32& Index_Or_A8R8G8B8);
		/*!
			Get a pixel.

			\param X the x coordinate of wanted pixel, must be valid.
			\param Y the y coordinate of wanted pixel, must be valid.
			\param Index_Or_A8R8G8B8 the color of specified pixel.\n
			If the image is an 8 bit image, this value represents an index (between 0 and 255). If the image is an
			high-color image, this value represents a full 32bit ARGB color, expanded from internal 15/16 bits value. If the
			image is a full 24/32 bits image, this value is the direct value taken from pixels array.
			\return G_NO_ERROR is operation succeeds, an error code otherwise.
			\note the pixel coordinates must be inside permitted range, else an G_OUT_OF_RANGE error code will be
			returned.
		*/
		GError Pixel(const GUInt32 X, const GUInt32 Y, GUInt32& Index_Or_A8R8G8B8) const;
		/*!
			Set a pixel color.

			\param X the x coordinate of wanted pixel, must be valid.
			\param Y the y coordinate of wanted pixel, must be valid.
			\param Index_Or_A8R8G8B8 the new color for specified pixel.\n
			If the image is an 8 bit image, this value represents an index (between 0 and 255). If the image is an
			high-color image, this value represents a full 32bit ARGB color, that will be automatically converted into
			the appropriate 15/16 bit value 15/16 bits value. If the image is a full 24/32 bits image, this value will be
			written directly into pixels array.
			\return G_NO_ERROR is operation succeeds, an error code otherwise.
			\note the pixel coordinates must be inside permitted range, else an G_OUT_OF_RANGE error code will be
			returned.
		*/
		GError SetPixel(const GUInt32 X, const GUInt32 Y, const GUInt32 Index_Or_A8R8G8B8);
		/*!
			Swap (reverse) color channels from ARGB to ABGR.

			\param ReverseAlphaToo if G_TRUE also the alpha channel (if present) is reversed. In this case format is
			converted from ARGB to BGRA.
			\return G_NO_ERROR is operation succeeds, an error code otherwise.
			\note this operation can be done only on high-color and full color images. Grayscale and paletted images
			are not supported (G_INVALID_OPERATION will be returned in these cases).
		*/
		GError ReverseChannels(const GBool ReverseAlphaToo = G_FALSE);
		/*!
			Swap (reverse) color channels from ARGB to ABGR, and put result into specified output image.

			\param ReversedImage the output (reversed) image.
			\param ReverseAlphaToo if G_TRUE also the alpha channel (if present) is reversed. In this case format is
			converted from ARGB to BGRA.
			\return G_NO_ERROR is operation succeeds, an error code otherwise.
			\note this operation can be done only on high-color and full color images. Grayscale and paletted images
			are not supported (G_INVALID_OPERATION will be returned in these cases).
		*/
		GError ReverseChannels(GPixelMap& ReversedImage, const GBool ReverseAlphaToo = G_FALSE) const;
		/*!
			Flip the image.

			\param Horizontal if G_TRUE an horizontal flip is done.
			\param Vertical if G_TRUE a vertical flip is done.
			\return G_NO_ERROR is operation succeeds, an error code otherwise.
			\note flipping order is not relevant, in fact HorzFlip(VertFlip(image)) is equal to VertFlip(HorzFlip(image)).

		*/
		GError Flip(const GBool Horizontal, const GBool Vertical);
		/*!
			Negative image, it negates every color channel.

			\return G_NO_ERROR is operation succeeds, an error code otherwise.
			\note paletted images are not supported.
		*/
		GError Negative();
		/*!
			Negative image, it negates every color channel, and put result into specified output image

			\param NegativePixelMap the output (negated) image.
			\return G_NO_ERROR is operation succeeds, an error code otherwise.
			\note paletted images are not supported.
		*/
		GError Negative(GPixelMap& NegativePixelMap) const;
		/*!
			Resize the bitmap, using the specified "smoothing" filter.

			\param NewWidth the new width of the image, must be positive.
			\param NewHeight the new height of the image, must be positive.
			\param ResizeOp filter used during rescaling.
			\return G_NO_ERROR is operation succeeds, an error code otherwise.
			\note resize is possible only on grayscale, high color and full color images. If this function is called by
			a paletted image, a G_INVALID_OPERATION error code is returned.
		*/
		GError Resize(const GUInt32 NewWidth, const GUInt32 NewHeight, const GResizeOp ResizeOp = G_RESIZE_QUADRATIC);

		/*!
			Resize the bitmap, using the specified "smoothing" filter, and put the result into specified output pixelmap.

			\param NewWidth the new width of the image, must be positive.
			\param NewHeight the new height of the image, must be positive.
			\param ResizedPixelMap the resized bitmap, used as output.
			\param ResizeOp filter used during rescaling.
			\return G_NO_ERROR is operation succeeds, an error code otherwise.
			\note resize is possible only on grayscale, high color and full color images. If this function is called by
			a paletted image, a G_INVALID_OPERATION error code is returned.
		*/
		GError Resize(const GUInt32 NewWidth, const GUInt32 NewHeight, GPixelMap& ResizedPixelMap,
					  const GResizeOp ResizeOp = G_RESIZE_QUADRATIC);

		/*!
			Resize (enlarge or reduce) the bitmap canvas.
			The Top and Bottom values control the vertical placement; the Left and Right values control the
			horizontal placement

			\param Top how many pixels we wanna add/subtract to/from top border.
			\param Bottom how many pixels we wanna add/subtract to/from bottom border.
			\param Right how many pixels we wanna add/subtract to/from right border.
			\param Left how many pixels we wanna add/subtract to/from left border.
			\param Index_Or_A8R8G8B8 the pixel value used to fill extra added pixel on borders.\n
			If the image is an 8 bit image, this value represents an index (between 0 and 255). If the image is an
			high-color image, this value represents a full 32bit ARGB color, that will be automatically converted into
			the appropriate 15/16 bit value 15/16 bits value. If the image is a full 24/32 bits image, this value will be
			written directly into pixels array.
			\return G_NO_ERROR is operation succeeds, an error code otherwise.
			\note if specified borders would result in a 0-width or o-height new picture, an G_INVALID_PARAMETER error
			code is returned.
		*/
		GError ResizeCanvas(const GInt32 Top, const GInt32 Bottom, const GInt32 Left, const GInt32 Right,
							const GUInt32 Index_Or_A8R8G8B8 = 0);

		/*!
			Resize (enlarge or reduce) the bitmap canvas, and put the result into specified output image.
			The Top and Bottom values control the vertical placement; the Left and Right values control the
			horizontal placement

			\param Top how many pixels we wanna add/subtract to/from top border.
			\param Bottom how many pixels we wanna add/subtract to/from bottom border.
			\param Right how many pixels we wanna add/subtract to/from right border.
			\param Left how many pixels we wanna add/subtract to/from left border.
			\param ResizedPixelMap the canvas-resized bitmap, used as output.
			\param Index_Or_A8R8G8B8 the pixel value used to fill extra added pixel on borders.\n
			If the image is an 8 bit image, this value represents an index (between 0 and 255). If the image is an
			high-color image, this value represents a full 32bit ARGB color, that will be automatically converted into
			the appropriate 15/16 bit value 15/16 bits value. If the image is a full 24/32 bits image, this value will be
			written directly into pixels array.
			\return G_NO_ERROR is operation succeeds, an error code otherwise.
			\note if specified borders would result in a 0-width or o-height new picture, an G_INVALID_PARAMETER error
			code is returned.
		*/
		GError ResizeCanvas(const GInt32 Top, const GInt32 Bottom, const GInt32 Left, const GInt32 Right,
							GPixelMap& ResizedPixelMap,	const GUInt32 Index_Or_A8R8G8B8 = 0) const;

		/*!
			Resize (enlarge or reduce) the bitmap canvas, doing pixel mirroring for enlarged borders.
			This method can be useful to build tiled pictures.

			\param Top how many pixels we wanna add/subtract to/from top border.
			\param Bottom how many pixels we wanna add/subtract to/from bottom border.
			\param Right how many pixels we wanna add/subtract to/from right border.
			\param Left how many pixels we wanna add/subtract to/from left border.
			\return G_NO_ERROR is operation succeeds, an error code otherwise.
			\note if specified borders would result in a 0-width or o-height new picture, an G_INVALID_PARAMETER error
			code is returned.
		*/
		GError ResizeCanvasMirror(const GInt32 Top, const GInt32 Bottom, const GInt32 Left, const GInt32 Right);

		/*!
			Resize (enlarge or reduce) the bitmap canvas, doing pixel mirroring for enlarged borders. Output
			is then put into specified destination image.

			This method can be useful to build tiled pictures.

			\param Top how many pixels we wanna add/subtract to/from top border.
			\param Bottom how many pixels we wanna add/subtract to/from bottom border.
			\param Right how many pixels we wanna add/subtract to/from right border.
			\param Left how many pixels we wanna add/subtract to/from left border.
			\param ResizedPixelMap the canvas-resized bitmap, used as output.
			\return G_NO_ERROR is operation succeeds, an error code otherwise.
			\note if specified borders would result in a 0-width or o-height new picture, an G_INVALID_PARAMETER error
			code is returned.
		*/
		GError ResizeCanvasMirror(const GInt32 Top, const GInt32 Bottom, const GInt32 Left, const GInt32 Right,
								  GPixelMap& ResizedPixelMap);

		/*!
			Apply an edge-preserving smooth filter (based on Susan).

			\param DistThres distance threshold, must be positive.
			\param BrighThres brightness threshold, must be between 0 and 255; it determines the maximum difference in
			gray levels between two pixels which allows them to be considered part of the same "region" in the image.
			\param Fast if G_TRUE the fast schema (1 radius mask) is used, else an accurate mask in accord to DistThres
			parameter is used.
			\return G_NO_ERROR is operation succeeds, an error code otherwise.
			\note this filter cannot be applied on paletted image, in this case an G_INVALID_OPERATION error code
			is returned.
		*/
		GError EdgePreservingSmooth(const GInt32 DistThres = 4, const GInt32 BrighThres = 20,
									const GBool Fast = G_FALSE);

		/*!
			Trace image contours, using an edge detector filter (based on Susan) .

			\param BrighThres brightness threshold, must be between 0 and 255; it determines the maximum difference in
			gray levels between two pixels which allows them to be considered part of the same "region" in the image.
			\param Thinning if G_TRUE, after tracing contours are also thinned.
			\param Fast if G_TRUE the fast schema is used, else an accurate mask (slow but accurate schema) is used.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note this filter cannot be applied on paletted image, in this case an G_INVALID_OPERATION error code
			is returned.
		*/
		GError TraceContours(const GInt32 BrighThres = 20, const GBool Thinning = G_TRUE, const GBool Fast = G_FALSE);

		/*!
			Apply a top-bottom hat filter to enhance edges.

			The implementation uses two filter versions (regulated by Strong parameter). Their respective matrices are:

			- Soft version
				\f[
				bottom = \left( \begin{array}{ccc}
				1 & 0 & 1 \\
				0 & -4 & 0 \\
				1 & 0 & 1 \end{array} \right)
				top = \left( \begin{array}{ccc}
				-1 & 0 & -1 \\
				0 & 4 & 0 \\
				-1 & 0 & -1 \end{array} \right)
				\f]
			- Strong version
				\f[
				bottom = \left( \begin{array}{ccc}
				1 & 1 & 1 \\
				1 & -8 & 1 \\
				1 & 1 & 1 \end{array} \right)
				top = \left( \begin{array}{ccc}
				-1 & -1 & -1 \\
				-1 & 8 & -1 \\
				-1 & -1 & -1 \end{array} \right)
				\f]

			\param Divisor the divisor used, must be positive. The more the value is, the less is edge enhancement.
			\param Strong if G_TRUE the strong filter version is used, else the soft one.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note this filter cannot be applied on paletted image, in this case an G_INVALID_OPERATION error code
			is returned.
		*/
		GError EdgeEnhance(const GInt32 Divisor = 1, const GBool Strong = G_FALSE);

		/*!
			Convert the image to a new specified pixel format.

			Every pixel will be converted from current format to the new specified one.

			\param NewPixelFormat the new pixel format
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
		*/
		GError SetPixelFormat(const GPixelFormat NewPixelFormat);
		/*!
			Convert the image to a new specified pixel format, and put the converted image into the specified
			destination.

			Every pixel will be converted from current format to the new specified one.

			\param NewPixelFormat the new pixel format
			\param ConvertedImage the destination image.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
		*/
		GError SetPixelFormat(const GPixelFormat NewPixelFormat, GPixelMap& ConvertedImage) const;
		/*
			Reset the image.

			This method free all occupied memory (pixels and color table), then new memory is allocated to respect
			the new specified dimensions and pixel format.
		*/
		GError Reset(const GInt32 NewWidth, const GInt32 NewHeight, const GPixelFormat NewPixelFormat);
		/*!
			Split red, green, blue and alpha channels into single images.

			\param AlphaImage is specified (non-NULL), it will contain a grayscale image with alpha values extracted from
			this bitmap.
			\param RedImage is specified (non-NULL), it will contain a grayscale image with red values extracted from
			this bitmap.
			\param GreenImage is specified (non-NULL), it will contain a grayscale image with green values extracted from
			this bitmap.
			\param BlueImage is specified (non-NULL), it will contain a grayscale image with blue values extracted from
			this bitmap.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note grayscale and paletted images are not supported, in this case an G_INVALID_OPERATION error code will be
			returned. 
		*/
		GError SplitChannels(GPixelMap *AlphaImage, GPixelMap *RedImage, GPixelMap *GreenImage, GPixelMap *BlueImage) const;
		/*!
			Merge RGB and Alpha channels to build the final full color image.

			\param RedImage the pixelmap containing the red channel. Must be a non-NULL grayscale pixelmap.
			\param GreenImage the pixelmap containing the green channel. Must be a non-NULL grayscale pixelmap.
			\param BlueImage the pixelmap containing the blue channel. Must be a non-NULL grayscale pixelmap.
			\param AlphaImage the pixelmap containing the optional alpha channel. If specified must be a grayscale pixelmap.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
			\note all specified channels must have the same dimensions, else G_INVALID_PARAMETER error code is
			returned.
		*/
		GError MergeChannels(const GPixelMap& RedImage, const GPixelMap& GreenImage, const GPixelMap& BlueImage,
							 const GPixelMap *AlphaImage = NULL);
		/*!
			Get pointer to the first pixel.
			
			Here's an example that accesses every image pixels (it supposes a grayscale or paletted image, so
			every pixel is a byte):\n\n
\code
	GUChar8 *pixelsArray = Image.Pixels();
	GInt32 i, j = Image.PixelsCount();
	for (i = 0; i < j; ++i)
		< do something with pixelsArray[i] >
\endcode
		*/
		inline GUChar8* Pixels() const {
			return gPixels;
		}
		/*!
			Get pointer to the first palette entry (if present). For non paletted images a NULL value
			will be returned.

			Here's an example that accesses every palette entry (it supposes a paletted image):\n\n
\code
	GUInt32 *palette = Image.Palette();
	GInt32 i, j = Image.PaletteEntriesCount();
	for (i = 0; i < j; ++i)
		< do something with palette[i] >
\endcode
		*/
		inline GUInt32* Palette() const {
			if (!IsPaletted())
				return NULL;
			return gPalette;
		}
		/*!
			Load a pixmap from external file (using kernel plugins). This method is provided for convenience.

			Internally this method look for kernel that has created this instances. If such kernel exists, call
			kernel->Load() function passing specified parameters, else return a G_MISSING_KERNEL error code.

			\param FileName is a string containing the full file name (path + name + extension) of the file we wanna read
			from. It must be non-empty.
			\param Options a string containing a semicolon-separated options. Each option must be in the format name=value.
			\param FormatName an optional filter; it's case insensitive.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
		*/
		GError Load(const GChar8 *FileName, const GChar8 *Options = NULL, const GChar8 *FormatName = NULL);
		/*!
			Save the pixmap to external file (using kernel plugins). This method is provided for convenience.

			Internally this method look for kernel that has created this instances. If such kernel exists, call
			kernel->Load() function passing specified parameters, else return a G_MISSING_KERNEL error code.

			\param FileName is a string containing the full file name (path + name + extension) of the file we wanna write
			to. It must be non-empty.
			\param Options a string containing a semicolon-separated options. Each option must be in the format name=value.
			\param FormatName an optional filter; it's case insensitive.
			\return G_NO_ERROR if operation succeeds, an error code otherwise.
		*/
		GError Save(const GChar8 *FileName, const GChar8 *Options = NULL, const GChar8 *FormatName = NULL);
		/*!
			RGB to gray conversion.

			The method used is based on a barycentric sum, where weights are taken from 'Luma' table.

			\param Red the red value.
			\param Green the green value.
			\param Blue the blue value.
			\return the corresponding gray value.
		*/
		inline static GUInt32 RGBToGray(const GUInt32 Red, const GUInt32 Green, const GUInt32 Blue) {
			// Rec 601-1: 0.299, 0.587, 0.114
			// Rec 709 / ITU:   0.2125, 0.7154, 0.0721
			GReal fLum = (GReal)Red * (GReal)0.299 + (GReal)Green * (GReal)0.587 + (GReal)Blue * (GReal)0.114;
			return (GUInt32)fLum;
		}
		/*!
			RGB to gray conversion.

			The method used is based on a barycentric sum, where weights are taken from 'Luma' table.

			\param RGB color to be converted, containing red, green and blue values packed in 8 bits each.
			Blue occupies the least significant byte.
			\return the corresponding gray value.
		*/
		static GUInt32 RGBToGray(const GUInt32 RGB) {
			// Rec 601-1: 0.299, 0.587, 0.114
			// Rec 709 / ITU:   0.2125, 0.7154, 0.0721
			GUInt32 r = (RGB >> 16) & 0xFF;
			GUInt32 g = (RGB >> 8) & 0xFF;
			GUInt32 b = (RGB & 0xFF);
			GReal fLum = (GReal)r * (GReal)0.299 + (GReal)g * (GReal)0.587 + (GReal)b * (GReal)0.114;
			return (GUInt32)fLum;
		}
		/*!
			RGB to CMY conversion.

			\param RGB color to be converted, containing red, green and blue values packed in 8 bits each.
			Blue occupies the least significant byte.
			\return the corresponding CMY value (yellow occupies the least significant byte).
		*/
		inline static GUInt32 RGBToCMY(const GUInt32 RGB) {

			return(RGB ^ 0x00FFFFFF);
		}
		/*!
			CMY to RGB conversion.

			\param CMY color to be converted, containing cyan, magenta and yellow values packed in 8 bits each.
			Yellow occupies the least significant byte.
			\return the corresponding RGB value (blue occupies the least significant byte).
		*/
		inline static GUInt32 CMYToRGB(const GUInt32 CMY) {
			return(CMY ^ 0x00FFFFFF);
		}
		/*!
			CMY to KCMY conversion.

			The black (K) component is calculated as the Min(C, M, Y). Then output C, M and Y values are scaled
			according to the black component.

			\param CMY color to be converted, containing cyan, magenta and yellow values packed in 8 bits each.
			Yellow occupies the least significant byte.
			\return the corresponding KCMY value (yellow occupies the least significant byte, and black the most
			significant one).
		*/
		static GUInt32 CMYToKCMY(const GUInt32 CMY);
		/*!
			KCMY to CMY conversion.

			\param KCMY color to be converted, containing cyan, magenta and yellow values packed in 8 bits each.
			Yellow occupies the least significant byte and black the most significant one.
			\return the corresponding CMY value (yellow occupies the least significant byte).
		*/
		static GUInt32 KCMYToCMY(const GUInt32 KCMY);
		/*!
			RGB to KCMY conversion, provided for convenience.

			It's just a CMYToKCMY(RGBToCMY(RGB)).
		*/
		inline static GUInt32 RGBToKCMY(const GUInt32 RGB) {
			GUInt32 CMY = RGBToCMY(RGB);
			return CMYToKCMY(CMY);
		}
		/*!
			KCMY to RGB conversion, provided for convenience.

			It's just a CMYToRGB(KCMYToCMY(KCMY)).
		*/
		inline static GUInt32 KCMYToRGB(const GUInt32 KCMY) {
			GUInt32 CMY = KCMYToCMY(KCMY);
			return CMYToRGB(CMY);
		}
		/*!
			RGB to HSV conversion.

			\param RGB color to be converted, containing red, green and blue values packed in 8 bits each.
			Blue occupies the least significant byte.
			\return the corresponding HSV value (the brightness V occupies the least significant byte).
		*/
		static GUInt32 RGBToHSV(const GUInt32 RGB);
		/*!
			RGB to HSV conversion.

			\param HSV color to be converted, containing hue, saturation and brightness values packed in 8 bits each.
			Brightness occupies the least significant byte.
			\return the corresponding RGB value (blue occupies the least significant byte).
		*/
		static GUInt32 HSVToRGB(const GUInt32 HSV);
		/*!
			RGB to HSL conversion.

			\param RGB color to be converted, containing red, green and blue values packed in 8 bits each.
			Blue occupies the least significant byte.
			\return the corresponding HSL value (the luminance L occupies the least significant byte).
		*/
		static GUInt32 RGBToHSL(const GUInt32 RGB);
		/*!
			HSL to RGB conversion.

			\param HSL color to be converted, containing hue, saturation and luminance values packed in 8 bits each.
			Luminance occupies the least significant byte.
			\return the corresponding RGB value (blue occupies the least significant byte).
		*/
		static GUInt32 HSLToRGB(const GUInt32 HSL);
		//! Calculate hue distance (caring of hue wrapping)
		static GInt32 HueDistance(const GUChar8 Hue1, const GUChar8 Hue2);
		//! Get class descriptor
		const GClassID& ClassID() const {
			return G_PIXELMAP_CLASSID;
		}
		//! Get base class (father class) descriptor
		const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
	};


	// *********************************************************************
	//                            GPixelMapProxy
	// *********************************************************************

	/*!
		\class GPixelMapProxy
		\brief This class implements a GPixelMap proxy (provider).

		This proxy provides the creation of GPixelMap class instances.
	*/
	class G_EXPORT GPixelMapProxy : public GElementProxy {
	public:
		//! Creates a new GPixelMap instance
		GElement* CreateNew(const GElement* Owner = NULL) const {
			return new GPixelMap(Owner);
		}
		//! Get class descriptor of elements type "provided" by this proxy.
		const GClassID& ClassID() const {
			return G_PIXELMAP_CLASSID;
		}
		//! Get base class (father class) descriptor of elements type "provided" by this proxy.
		const GClassID& DerivedClassID() const {
			return G_ELEMENT_CLASSID;
		}
	};
	//! Static proxy for GPixelMap class.
	static const GPixelMapProxy G_PIXELMAP_PROXY;

};	// end namespace Amanith

#endif
