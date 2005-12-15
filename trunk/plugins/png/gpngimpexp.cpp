/****************************************************************************
** $file: amanith/plugins/png/gpngimpext.cpp   0.2.0.0   edited Dec, 12 2005
**
** 2D Pixelmap PNG import/export plugin implementation.
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

#ifdef _PNG_PLUGIN

#include "gpngimpexp.h"
#include "amanith/support/gutilities.h"
#include <cstdlib>
#include <cstring>
//#include <csetjmp>

G_EXTERN_C {
	#include "../../3rdpart/libpng/png.h"
};

/****************************************************************************
 *
 * Follows is code extracted from the simage library.  Original Authors:
 *
 *      Systems in Motion,
 *      <URL:http://www.sim.no>
 *
 *      Peder Blekken <pederb@sim.no>
 *      Morten Eriksen <mortene@sim.no>
 *      Marius Bugge Monsen <mariusbu@sim.no>
 *
 * The original COPYING notice
 *
 *      All files in this library are public domain, except simage_rgb.cpp which is
 *      Copyright (c) Mark J Kilgard <mjk@nvidia.com>. I will contact Mark
 *      very soon to hear if this source also can become public domain.
 *
 *      Please send patches for bugs and new features to: <pederb@sim.no>.
 *
 *      Peder Blekken
 *
  **********************************************************************/

// my setjmp buffer
static jmp_buf setjmp_buffer;

// called my libpng
static void warn_callback(png_structp /*ps*/, png_const_charp pc) {
	if (pc)
		G_DEBUG(pc);
}

static void err_callback(png_structp /*ps*/, png_const_charp pc) {
	/* FIXME: store error message? */
	longjmp(setjmp_buffer, 1);
	if (pc)
		G_DEBUG(pc);
}

int simage_png_identify(const char * /*ptr*/, const unsigned char *header, int headerlen) {

  static unsigned char pngcmp[] = {0x89, 'P', 'N', 'G', 0xd, 0xa, 0x1a, 0xa};
  if (headerlen < 8)
	  return 0;
  if (std::memcmp((const void*)header, (const void*)pngcmp, 8) == 0)
	  return 1;
  return 0;
}

namespace Amanith {

// *********************************************************************
//                                GPngImpExp
// *********************************************************************

// modified simage_jpeg_load function
GError GPngImpExp::RawPngLoad(const GChar8 *FileName, GInt32& Width, GInt32& Height,
							  GPixelFormat& PixelFormat, GUChar8 **OutBuffer,
							  GInt32& NumPaletteEntries, GUInt32 **PaletteBuffer,
							  const GBool ExpandPalette) {

#if defined(_PNG_READ)

	png_uint_32 width, height;
	GInt32 bit_depth, color_type, interlace_type, number_passes;
    GInt32 bytes_per_row, channels, pass, y;
	png_structp png_ptr;
	png_infop info_ptr;
	GInt32 num_palette;
    png_colorp palette;
	GUChar8 *buffer;
	unsigned char *dummytab[1];
	GPixelFormat resFormat;

	if (!OutBuffer)
		return G_INVALID_PARAMETER;

#if defined(G_OS_WIN) && _MSC_VER >= 1400
	std::FILE *infile = NULL;
	errno_t openErr = fopen_s(&infile, FileName, "rb");
	if (!infile || openErr)
		return G_READ_ERROR;
#else
	std::FILE *infile;               /* source file */
	if ((infile = std::fopen(FileName, "rb")) == NULL)
		return G_READ_ERROR;
#endif
	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also supply the
	* the compiler header file version, so that we know if the application
	* was compiled with a compatible version of the library.  REQUIRED
	*/
	/*png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		(void *)user_error_ptr, user_error_fn, user_warning_fn);*/

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, err_callback, warn_callback);
	if (png_ptr == NULL) {
		std::fclose(infile);
		return G_MEMORY_ERROR;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		std::fclose(infile);
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return G_MEMORY_ERROR;
	}


    // used to be before setjump above, but have moved to after to avoid compile warnings.
    buffer = NULL;
	num_palette = 0;
	color_type = 0;
	bytes_per_row = 0;

	/* Set error handling if you are using the setjmp/longjmp method (this is
	* the normal method of doing things with libpng).  REQUIRED unless you
	* set up your own error handlers in the png_create_read_struct() earlier.
	*/
	if (setjmp(setjmp_buffer)) {
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		std::fclose(infile);
		/* If we get here, we had a problem reading the file */
		return G_READ_ERROR;
	}

	GUInt32 *newPalette = NULL;

	/* Set up the input control if you are using standard C streams */
	png_init_io(png_ptr, infile);

	/* The call to png_read_info() gives us all of the information from the
	* PNG file before the first IDAT (image data chunk).  REQUIRED
	*/
	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
				&interlace_type, NULL, NULL);


	/**** Set up the data transformations you want.  Note that these are all
	**** optional.  Only call them if you want/need them.  Many of the
	**** transformations only work on specific types of images, and many
	**** are mutually exclusive.
	****/

	/* tell libpng to strip 16 bit/color files down to 8 bits/color */
	png_set_strip_16(png_ptr);

	switch (color_type) {

		case PNG_COLOR_TYPE_GRAY:
			/* expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
			if (bit_depth < 8)
				png_set_expand(png_ptr);
			resFormat = G_GRAYSCALE;
			break;

		case PNG_COLOR_TYPE_GRAY_ALPHA:
			/* expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
			if (bit_depth < 8)
				png_set_expand(png_ptr);
			/* expand paletted or RGB images with transparency to full alpha channels
			* so the data will be available as RGBA quartets */
			if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
				png_set_expand(png_ptr);
			resFormat = G_A8R8G8B8;
			break;

		case PNG_COLOR_TYPE_PALETTE:

			// expand palette to full RGB?
			if (ExpandPalette) {
				png_set_expand(png_ptr);
				/* Add filler (or alpha) byte (before/after each RGB triplet) */
				png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
				resFormat = G_R8G8B8;
			}
			else {
				// read palette
				png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
				/* extract multiple pixels with bit depths of 1, 2, and 4 from a single
				* byte into separate bytes (useful for paletted and grayscale images).
				*/
				png_set_packing(png_ptr);
				// allocate palette
				GInt32 i, j;
				GUInt32 k;

				if (num_palette > 256)
					j = 256;
				else
					j = num_palette;
				newPalette = new GUInt32[256];
				// copy palette
				for (i = 0; i < j; i++) {
					k = ((GUInt32)palette[i].blue) | (((GUInt32)palette[i].green) << 8) |
						(((GUInt32)palette[i].red) << 16);
					newPalette[i] = k;
				}
				for (i = j; i < 256; i++)
					newPalette[i] = 0;
				resFormat = G_RGB_PALETTE;
			}
			break;

		case PNG_COLOR_TYPE_RGB:
			/* Add filler (or alpha) byte (before/after each RGB triplet) */
			png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
			resFormat = G_R8G8B8;
			break;

		case PNG_COLOR_TYPE_RGB_ALPHA:
			/* expand paletted or RGB images with transparency to full alpha channels
			* so the data will be available as RGBA quartets */
			if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
				png_set_expand(png_ptr);
			resFormat = G_A8R8G8B8;
			break;

		default:
			// just to avoid warnings...
			resFormat = G_GRAYSCALE;
	}

	png_set_bgr(png_ptr);

	/* Turn on interlace handling.  REQUIRED if you are not using
	* png_read_image().  To see how to handle interlacing passes,
	* see the png_read_row() method below.
	*/
	png_read_update_info(png_ptr, info_ptr);

	number_passes = png_set_interlace_handling(png_ptr);
	channels = png_get_channels(png_ptr, info_ptr);

	/* allocate the memory to hold the image using the fields of info_ptr. */
	bytes_per_row = png_get_rowbytes(png_ptr, info_ptr);
	buffer = new GUChar8[(bytes_per_row * height)];

	if (buffer) {
		for (pass = 0; pass < number_passes; pass++) {
			for (y = 0; (GUInt32)y < height; y++) {
				/* flips image upside down */
				//dummytab[0] = (unsigned char *)&buffer[bytes_per_row * (height - 1 - y)];
				dummytab[0] = (unsigned char *)&buffer[bytes_per_row * y];
				png_read_rows(png_ptr, dummytab, NULL, 1);
			}
		}
		/* read rest of file, and get additional chunks in info_ptr - REQUIRED */
		png_read_end(png_ptr, info_ptr);
	}
	  
	/* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	/* close the file */
	std::fclose(infile);

    /* And we're done! */
    if (buffer) {
        Width = width;
        Height = height;
        PixelFormat = resFormat;
		*OutBuffer = buffer;
		NumPaletteEntries = num_palette;
		if (PaletteBuffer)
			*PaletteBuffer = newPalette;
		return G_NO_ERROR;
    }
    else
        return G_MEMORY_ERROR;
#else
	return G_MISSED_FEATURE;
#endif
}

// modified simage_jpeg_save function
GError GPngImpExp::RawPngSave(const GChar8 *FileName, const GInt32 Width, const GInt32 Height,
							  const GPixelFormat PixelFormat, const GUChar8 *Buffer,
							  const GInt32 NumPaletteEntries, const GUInt32 *PaletteBuffer,
							  const GBool Interlaced) {

#if defined(_PNG_WRITE)

	std::FILE *fp = NULL;
	png_structp png_ptr;
	png_infop info_ptr;
	png_text text_ptr[3];

	// open the file
#if defined(G_OS_WIN) && _MSC_VER >= 1400
	errno_t openErr = fopen_s(&fp, FileName, "wb");
	if (!fp || openErr)
		return G_WRITE_ERROR;
#else
	fp = std::fopen(FileName, "wb");
	if (fp == NULL)
		return G_WRITE_ERROR;
#endif

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also check that
	* the library version is compatible with the one used at compile time,
	* in case we are using dynamically linked libraries.  REQUIRED. */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png_ptr == NULL) {
		std::fclose(fp);
		return G_WRITE_ERROR;
	}

	// Allocate/initialize the image information data. REQUIRED.
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		std::fclose(fp);
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return G_MEMORY_ERROR;
	}

	/* Set error handling.  REQUIRED if you aren't supplying your own
	* error handling functions in the png_create_write_struct() call.
	*/
	if (setjmp(png_ptr->jmpbuf)) {
		/* If we get here, we had a problem reading the file */
		std::fclose(fp);
		png_destroy_write_struct(&png_ptr, (png_infopp)info_ptr);
		return G_MEMORY_ERROR;
	}

	/*  we're not using png_init_io(), as we don't want to pass a FILE*
		into libpng, in case it's an MSWindows DLL with a different CRT
		(C run-time library) */
	//png_set_write_fn(png_ptr, (void *)fp, (png_rw_ptr)user_write_cb, (png_flush_ptr)user_flush_cb);

	png_init_io(png_ptr, fp);

	int colortype = 0;
	int y, bytesperrow = 0;
  
	/* Set the image information here.  Width and height are up to 2^31,
	* bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
	* the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
	* PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
	* or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
	* PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
	* currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED	*/

	switch (PixelFormat) {
		case G_A1R5G5B5:
		case G_R5G6B5:
			std::fclose(fp);
			png_destroy_write_struct(&png_ptr, (png_infopp)info_ptr);
			return G_INVALID_PARAMETER;
			break;

		case G_GRAYSCALE:
			colortype = PNG_COLOR_TYPE_GRAY;
			bytesperrow = Width;
			break;

		case G_R8G8B8:
			colortype = PNG_COLOR_TYPE_RGB;
			bytesperrow = Width * 4;
			break;

		case G_A8R8G8B8:
			colortype = PNG_COLOR_TYPE_RGB_ALPHA;
			bytesperrow = Width * 4;
			break;

		case G_RGB_PALETTE:
			colortype = PNG_COLOR_TYPE_PALETTE;
			bytesperrow = Width;
			break;
	}

	if (Interlaced)
		png_set_IHDR(png_ptr, info_ptr, Width, Height, 8, colortype,
					 PNG_INTERLACE_ADAM7, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	else
		png_set_IHDR(png_ptr, info_ptr, Width, Height, 8, colortype,
					 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	/* Optional gamma chunk is strongly suggested if you have any guess
	* as to the correct gamma of the image. */
	/* png_set_gAMA(png_ptr, info_ptr, gamma); */

//#if defined(PNG_TEXT_SUPPORTED)
  /* Optionally write comments into the image */
	text_ptr[0].key = "Title";
	text_ptr[0].text = (char*)FileName;
	text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
	text_ptr[1].key = "Author";
	text_ptr[1].text = "Amanith (http://www.amanith.org)";
	text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
	text_ptr[2].key = "Description";
	text_ptr[2].text = "Image saved using Amanith.";
	text_ptr[2].compression = PNG_TEXT_COMPRESSION_zTXt;
	png_set_text(png_ptr, info_ptr, text_ptr, 3);
//#endif /* PNG_TEXT_SUPPORTED */

	/* other optional chunks like cHRM, bKGD, tRNS, tIME, oFFs, pHYs, */

	png_color tmpPalette[256];
	for (GInt32 i = 0; i < NumPaletteEntries; ++i) {
		tmpPalette[i].red = (png_byte)((PaletteBuffer[i] >> 16) & 255);
		tmpPalette[i].green = (png_byte)((PaletteBuffer[i] >> 8) & 255);
		tmpPalette[i].blue = (png_byte)(PaletteBuffer[i] & 255);
	}
	for (GInt32 i = NumPaletteEntries; i < 256; ++i)
		tmpPalette[i].red = tmpPalette[i].green = tmpPalette[i].blue = 0;

	if (PaletteBuffer)
		png_set_PLTE(png_ptr, info_ptr, tmpPalette, NumPaletteEntries);

	/* Write the file header information.  REQUIRED */
	png_write_info(png_ptr, info_ptr);

	/* Once we write out the header, the compression type on the text
	* chunks gets changed to PNG_TEXT_COMPRESSION_NONE_WR or
	* PNG_TEXT_COMPRESSION_zTXt_WR, so it doesn't get written out again
	* at the end.
	*/

	/* set up the transformations you want.  Note that these are
	* all optional.  Only call them if you want them. */

	/* invert monochrome pixels */
	/* png_set_invert(png_ptr); */

	/* Shift the pixels up to a legal bit depth and fill in
	* as appropriate to correctly scale the image */
	/* png_set_shift(png_ptr, &sig_bit);*/

	/* pack pixels into bytes */
	/* png_set_packing(png_ptr); */

	/* swap location of alpha bytes from ARGB to RGBA */
	//png_set_swap_alpha(png_ptr);

	/* Get rid of filler (OR ALPHA) bytes, pack XRGB/RGBX/ARGB/RGBA into
	* RGB (4 channels -> 3 channels). The second parameter is not used. */
	//png_set_filler(png_ptr, 255, PNG_FILLER_AFTER);

	/* flip BGR pixels to RGB */
	if (PixelFormat == G_A8R8G8B8 || PixelFormat == G_R8G8B8)
		png_set_bgr(png_ptr);

	if (PixelFormat == G_R8G8B8)
		png_set_filler(png_ptr, 255, PNG_FILLER_AFTER);

	/* swap bytes of 16-bit files to most significant byte first */
	/* png_set_swap(png_ptr); */

	/* swap bits of 1, 2, 4 bit packed pixel formats */
	/* png_set_packswap(png_ptr); */


	/* The easiest way to write the image (you may have a different memory
	* layout, however, so choose what fits your needs best).  You need to
	* use the first method if you aren't handling interlacing yourself.
	*/

	/* If you are only writing one row at a time, this works */
  
	//bytesperrow = width * numcomponents;

	for (y = 0; y < Height; y++)
		png_write_row(png_ptr, (png_bytep)Buffer + bytesperrow * y);
  
	/* You can write optional chunks like tEXt, zTXt, and tIME at the end
	* as well.
	*/
  
	/* It is REQUIRED to call this to finish writing the rest of the file */
	png_write_end(png_ptr, info_ptr);

	/* if you allocated any text comments, free them here */

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	/* close the file */
	std::fclose(fp);

	/* that's it */
	return G_NO_ERROR;
#else
	// just to avoid warnings
	if (FileName && Width && Height && PixelFormat && Buffer && NumPaletteEntries && PaletteBuffer && Interlaced)
		return G_MISSED_FEATURE;
	return G_MISSED_FEATURE;
#endif
}


// constructor
GPngImpExp::GPngImpExp() : GImpExp() {
	AddPngFeatures();
}

// constructor
GPngImpExp::GPngImpExp(const GElement* Owner) : GImpExp(Owner) {
	AddPngFeatures();
}

// destructor
GPngImpExp::~GPngImpExp() {
}

void GPngImpExp::AddPngFeatures() {

#if defined(_PNG_READ) && defined(_PNG_WRITE)
	AddEntry(GImpExpFeature(G_PIXELMAP_CLASSID, "Png", "png", 1, 0, 0, 0, G_IMPEXP_READWRITE));
#elif defined(_PNG_READ)
	AddEntry(GImpExpFeature(G_PIXELMAP_CLASSID, "Png", "png", 1, 0, 0, 0, G_IMPEXP_READ));
#else
	AddEntry(GImpExpFeature(G_PIXELMAP_CLASSID, "Png", "png", 1, 0, 0, 0, G_IMPEXP_WRITE));
#endif
}


GError GPngImpExp::ReadPng(const GChar8 *FullFileName, GElement& Element, const GBool ExpandPalette) {

	GPixelMap& image = (GPixelMap&)Element;
	GPixelFormat pixFormat;
	GInt32 width = -1, height = -1, numPalEntries = -1, bufferSize;
	GUChar8 *pixelBuffer = NULL, *p = NULL;
	GUInt32 *paletteBuffer = NULL;
	GUInt32 *pal = NULL;
	GError err;

	err = GPngImpExp::RawPngLoad(FullFileName, width, height, pixFormat, &pixelBuffer,
								numPalEntries, &paletteBuffer, ExpandPalette);
	if (err == G_NO_ERROR) {

		// reset image dimensions and pixel format
		err = image.Reset(width, height, pixFormat);
		if (err != G_NO_ERROR)
			goto finalize;
		p = image.Pixels();
		G_ASSERT(p != NULL);
		// copy pixels from buffer to image
		bufferSize = image.Size();
		std::memcpy((void *)p, (void *)pixelBuffer, bufferSize);

		// copy palette
		if (image.IsPaletted()) {
			pal = image.Palette();
			G_ASSERT(pal != NULL);
			bufferSize = image.PaletteSize();
			std::memcpy((void *)pal, (void *)paletteBuffer, bufferSize);
		}
	}

finalize:
	if (pixelBuffer)
		delete [] pixelBuffer;
	if (paletteBuffer)
		delete [] paletteBuffer;

	return err;
}

GError GPngImpExp::WritePng(const GChar8 *FullFileName, const GElement& Element, const GBool Interlaced) {

	GPixelMap& image = (GPixelMap&)Element;
	GUChar8 *buf;
	GError err;

	if (image.PixelsCount() <= 0)
		return G_INVALID_PARAMETER;
	buf = (GUChar8 *)image.Pixels();

	// paletted image
	if (image.IsPaletted()) {
		err = RawPngSave(FullFileName, image.Width(), image.Height(), image.PixelFormat(), buf, 256,
						image.Palette(), Interlaced);
	}
	else
	// if image is 15/16 bit color, we must convert to full 24/32 bit
	if (image.IsHighColor()) {
		GPixelMap tmpImage;
		if (image.PixelFormat() == G_A1R5G5B5) {
			err = image.SetPixelFormat(G_A8R8G8B8, tmpImage);
			if (err == G_NO_ERROR) {
				buf = (GUChar8 *)tmpImage.Pixels();
				err = RawPngSave(FullFileName, tmpImage.Width(), tmpImage.Height(), tmpImage.PixelFormat(),
								 buf, 0, NULL, Interlaced);
			}
		}
		else {
			err = image.SetPixelFormat(G_R8G8B8, tmpImage);
			if (err == G_NO_ERROR) {
				buf = (GUChar8 *)tmpImage.Pixels();
				err = RawPngSave(FullFileName, tmpImage.Width(), tmpImage.Height(), tmpImage.PixelFormat(),
								 buf, 0, NULL, Interlaced);
			}
		}
	}
	else
		// full color image
		err = RawPngSave(FullFileName, image.Width(), image.Height(), image.PixelFormat(), buf, 0, NULL, Interlaced);
	return err;
}

GError GPngImpExp::DoRead(const GChar8 *FullFileName, GElement& Element, const GDynArray<GImpExpOption>& ParsedOptions) {

	GBool expandPalette = G_FALSE;

	// check each option
	GDynArray<GImpExpOption>::const_iterator it = ParsedOptions.begin();
	for (; it != ParsedOptions.end(); ++it) {
		if (StrUtils::SameText(it->OptionName, "expandpalette"))
			if (StrUtils::SameText(it->OptionValue, "true"))
				expandPalette = G_TRUE;
	}
	return ReadPng(FullFileName, Element, expandPalette);
}

GError GPngImpExp::DoWrite(const GChar8 *FullFileName, const GElement& Element, const GDynArray<GImpExpOption>& ParsedOptions) {

	// default options
	GBool interlacedEncoding = G_FALSE;

	// check each option
	GDynArray<GImpExpOption>::const_iterator it = ParsedOptions.begin();
	for (; it != ParsedOptions.end(); ++it) {
		if (StrUtils::SameText(it->OptionName, "encoding"))
			if (StrUtils::SameText(it->OptionValue, "interlaced"))
				interlacedEncoding = G_TRUE;
	}
	return WritePng(FullFileName, Element, interlacedEncoding);
}

// export interface functions
#ifdef _PNG_PLUGIN_EXTERNAL
G_EXTERN_C G_PLUGIN_EXPORT GUInt32 ProxiesCount() {
	return 1;
}
G_EXTERN_C G_PLUGIN_EXPORT const GElementProxy* ProxyInstance(const GUInt32 Index) {
	if (Index == 0)
		return &G_PNGIMPEXP_PROXY;
	return NULL;
}
#endif

}

#endif
