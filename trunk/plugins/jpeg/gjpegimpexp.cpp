/****************************************************************************
** $file: amanith/plugins/jpeg/gjpegimpext.cpp   0.1.0.0   edited Jun 30 08:00
**
** 2D Pixelmap JPEG import/export plugin implementation.
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

#ifdef _JPEG_PLUGIN

#include "gjpegimpexp.h"
#include "amanith/support/gutilities.h"
#include "amanith/2d/gpixelmap.h"
#include <cstdlib>
#include <cstring>
#include <csetjmp>

G_EXTERN_C {
	#include "../../3rdpart/libjpeg/jpeglib.h"
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
struct my_error_mgr {
  struct jpeg_error_mgr pub;    /* "public" fields */
  jmp_buf setjmp_buffer;        /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

static void my_error_exit (j_common_ptr cinfo) {
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    my_error_ptr myerr = (my_error_ptr) cinfo->err;

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    /*(*cinfo->err->output_message) (cinfo);*/

    /* FIXME: get error messahe from jpeglib */

    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
}


int simage_jpeg_identify(const char *, const unsigned char *header, int headerlen)
{
    static unsigned char jpgcmp[] = {'J', 'F', 'I', 'F'};
    if (headerlen < 4) return 0;
	if (std::memcmp((const void*)&header[6], (const void*)jpgcmp, 4) == 0) return 1;
    return 0;
}


namespace Amanith {

// *********************************************************************
//                                GJpegImpExp
// *********************************************************************

// modified simage_jpeg_load function
GError GJpegImpExp::RawJpegLoad(const GChar8 *FileName, GInt32& Width, GInt32& Height,
								GInt32& NumComponents, GUChar8 **OutBuffer) {

#if defined(_JPEG_READ)

    GInt32 width;
    GInt32 height;
    GUChar8 *currPtr;
    GInt32 numChannels;

	if (!OutBuffer)
		return G_INVALID_PARAMETER;

    /* This struct contains the JPEG decompression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     */
    struct jpeg_decompress_struct cinfo;
    /* We use our private extension JPEG error handler.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */
    struct my_error_mgr jerr;
    /* More stuff */
	std::FILE *infile;               /* source file */
    JSAMPARRAY rowbuffer;        /* Output row buffer */
    GInt32 row_stride;              /* physical row width in output buffer */

    /* In this example we want to open the input file before doing anything else,
     * so that the setjmp() error recovery below can assume the file is open.
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to read binary files.
     */

	if ((infile = std::fopen(FileName, "rb")) == NULL)
		return G_READ_ERROR;

    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer)) {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object, close the input file, and return.
         */
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return G_UNKNOWN_ERROR;
    }

    // used to be before setjump above, but have moved to after to avoid compile warnings.
    GUChar8 *buffer = NULL;

    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */

    jpeg_stdio_src(&cinfo, infile);

    /* Step 3: read file parameters with jpeg_read_header() */

    (void) jpeg_read_header(&cinfo, TRUE);
    /* We can ignore the return value from jpeg_read_header since
     *   (a) suspension is not possible with the stdio data source, and
     *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
     * See libjpeg.doc for more info.
     */

    /* Step 4: set parameters for decompression */
    /* In this example, we don't need to change any of the defaults set by
     * jpeg_read_header(), so we do nothing here.
     */

    /* Step 5: Start decompressor */
    if (cinfo.jpeg_color_space == JCS_GRAYSCALE) {
        numChannels = 1;
        cinfo.out_color_space = JCS_GRAYSCALE;
    }
	else { /* use rgb wih alpha channel to pad*/
        numChannels = 3;
        cinfo.out_color_space = JCS_RGB;
    }

    (void) jpeg_start_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* We may need to do some setup of our own at this point before reading
     * the data.  After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     * In this example, we need to make an output work buffer of the right size.
     */
    /* JSAMPLEs per row in output buffer */
    row_stride = cinfo.output_width * cinfo.output_components;
    /* Make a one-row-high sample array that will go away when done with image */
    rowbuffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
    width = cinfo.output_width;
    height = cinfo.output_height;
	// we allocate a buffer for entire image
    buffer = currPtr = new GUChar8[width * height * cinfo.output_components];

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    /* Here we use the library's state variable cinfo.output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     */

    /* flip image upside down */
    if (buffer) {
        //currPtr = buffer + row_stride * (cinfo.output_height - 1);
		currPtr = buffer;
        while (cinfo.output_scanline < cinfo.output_height) {
            /* jpeg_read_scanlines expects an array of pointers to scanlines.
             * Here the array is only one element long, but you could ask for
             * more than one scanline at a time if that's more convenient.
             */
            (void) jpeg_read_scanlines(&cinfo, rowbuffer, 1);
			std::memcpy((void *)currPtr, (void *)rowbuffer[0], row_stride);
            /* Assume put_scanline_someplace wants a pointer and sample count. */
			//currPtr -= row_stride;
			currPtr += row_stride;
        }
    }
    /* Step 7: Finish decompression */
    (void) jpeg_finish_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* Step 8: Release JPEG decompression object */
    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);

    /* After finish_decompress, we can close the input file.
     * Here we postpone it until after no more JPEG errors are possible,
     * so as to simplify the setjmp error logic above.  (Actually, I don't
     * think that jpeg_destroy can do an error exit, but why assume anything...)
     */
	std::fclose(infile);

    /* At this point you may want to check to see whether any corrupt-data
     * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
     */

    /* And we're done! */
    if (buffer) {
        Width = width;
        Height = height;
        NumComponents = numChannels;
		*OutBuffer = buffer;
		return G_NO_ERROR;
    }
    else
        return G_MEMORY_ERROR;
#else
	return G_MISSED_FEATURE;
#endif
}

// modified simage_jpeg_save function
GError GJpegImpExp::RawJpegSave(const GChar8 *FileName, const GInt32 Width, const GInt32 Height,
								const GInt32 NumComponents,	const GUChar8 *Buffer,
								const GInt32 Quality, const GBool ProgressiveEncoding) {

#if defined(_JPEG_WRITE)

    /* This struct contains the JPEG compression parameters and pointers to
    * working space (which is allocated as needed by the JPEG library).
    * It is possible to have several such structures, representing multiple
    * compression/decompression processes, in existence at once.  We refer
    * to any one struct (and its associated working data) as a "JPEG object".
    */
    struct jpeg_compress_struct cinfo;
    /* This struct represents a JPEG error handler.  It is declared separately
    * because applications often want to supply a specialized error handler
    * (see the second half of this file for an example).  But here we just
    * take the easy way out and use the standard error handler, which will
    * print a message on stderr and call exit() if compression fails.
    * Note that this struct must live as long as the main JPEG parameter
    * struct, to avoid dangling-pointer problems.
    */
    struct jpeg_error_mgr jerr;
    /* More stuff */
    JSAMPROW row_pointer[1];    /* pointer to JSAMPLE row[s] */
    GInt32 row_stride;        /* physical row width in image buffer */

    /* Step 1: allocate and initialize JPEG compression object */

    /* We have to set up the error handler first, in case the initialization
    * step fails.  (Unlikely, but it could happen if you are out of memory.)
    * This routine fills in the contents of struct jerr, and returns jerr's
    * address which we place into the link field in cinfo.
    */
    cinfo.err = jpeg_std_error(&jerr);
    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    /* Step 2 & 3: specify data destination (eg, a file) and set parameters for compression*/
    /* Note: steps 2 and 3 can be done in either order. For our comfort we choose to do first step 3
	* and then step 2
	*/

    /* Step 3: set parameters for compression */
    /* First we supply a description of the input image.
    * Four fields of the cinfo struct must be filled in:
    */
    cinfo.image_width = Width;						/* image width and height, in pixels */
    cinfo.image_height = Height;
	if (NumComponents == 3) {
		cinfo.input_components = 3;					/* # of color components per pixel */
		cinfo.in_color_space = JCS_RGB;				/* colorspace of input image */
	}
	else
	if (NumComponents == 1) {
		cinfo.input_components = 1;					/* # of color components per pixel */
		cinfo.in_color_space = JCS_GRAYSCALE;		/* colorspace of input image */
	}
	else
		return G_INVALID_PARAMETER;

	/* Step 2: specify data destination
    * Here we use the library-supplied code to send compressed data to a
    * stdio stream.  You can also write your own code to do something else.
    * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
    * requires it in order to write binary files.
    */
	std::FILE *outfile = std::fopen(FileName, "wb");
	if (!outfile)
        return G_WRITE_ERROR;
    jpeg_stdio_dest(&cinfo, outfile);


    /* Now use the library's routine to set default compression parameters.
    * (You must set at least cinfo.in_color_space before calling this,
    * since the defaults depend on the source color space.)
    */
    jpeg_set_defaults(&cinfo);
    /* Now you can set any non-default parameters you wish to.
    * Here we just illustrate the use of quality (quantization table) scaling:
    */
    jpeg_set_quality(&cinfo, Quality, TRUE /* limit to baseline-JPEG values */);

	if (ProgressiveEncoding)
		jpeg_simple_progression(&cinfo);

    /* Step 4: Start compressor */
    /* TRUE ensures that we will write a complete interchange-JPEG file.
    * Pass TRUE unless you are very sure of what you're doing.
    */
    jpeg_start_compress(&cinfo, TRUE);

    /* Step 5: while (scan lines remain to be written) */
    /*           jpeg_write_scanlines(...); */
    /* Here we use the library's state variable cinfo.next_scanline as the
    * loop counter, so that we don't have to keep track ourselves.
    * To keep things simple, we pass one scanline per call; you can pass
    * more if you wish, though.
    */
    row_stride = Width * NumComponents;    /* JSAMPLEs per row in image_buffer */

    while (cinfo.next_scanline < cinfo.image_height) {
        /* jpeg_write_scanlines expects an array of pointers to scanlines.
        * Here the array is only one element long, but you could pass
        * more than one scanline at a time if that's more convenient.
        */
        row_pointer[0] = (JSAMPROW)&Buffer[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    /* Step 6: Finish compression */
    jpeg_finish_compress(&cinfo);
    /* After finish_compress, we can close the output file. */
	std::fclose(outfile);
    /* Step 7: release JPEG compression object */
    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_compress(&cinfo);
    /* And we're done! */
    return G_NO_ERROR;
#else
	return G_MISSED_FEATURE;
#endif
}


// constructor
GJpegImpExp::GJpegImpExp() : GImpExp() {
	AddJpegFeatures();
}

// constructor
GJpegImpExp::GJpegImpExp(const GElement* Owner) : GImpExp(Owner) {
	AddJpegFeatures();
}

// destructor
GJpegImpExp::~GJpegImpExp() {
}

void GJpegImpExp::AddJpegFeatures() {

#if defined(_JPEG_READ) && defined(_JPEG_WRITE)
	AddEntry(GImpExpFeature(G_PIXELMAP_CLASSID, "Jpeg", "jpg;jpeg", 1, 0, 0, 0, G_IMPEXP_READWRITE));
#elif defined(_JPEG_READ)
	AddEntry(GImpExpFeature(G_PIXELMAP_CLASSID, "Jpeg", "jpg;jpeg", 1, 0, 0, 0, G_IMPEXP_READ));
#else
	AddEntry(GImpExpFeature(G_PIXELMAP_CLASSID, "Jpeg", "jpg;jpeg", 1, 0, 0, 0, G_IMPEXP_WRITE));
#endif
}


GError GJpegImpExp::ReadJpeg(const GChar8 *FullFileName, GElement& Element) {

	GPixelMap& image = (GPixelMap&)Element;
	GInt32 width = 0, height = 0, numComponents = 0, bufferSize;
	GInt32 i, j, srcOfs, dstOfs;
	GUChar8 *pixelBuffer = NULL, *p = NULL;
	GError err;

	err = RawJpegLoad(FullFileName, width, height, numComponents, &pixelBuffer);
	if (err == G_NO_ERROR) {
		// image is greyscale
		if (numComponents == 1) {
			err = image.Reset(width, height, G_GRAYSCALE);
			if (err != G_NO_ERROR)
				goto finalize;
			// check for classid consistency
			p = image.Pixels();
			G_ASSERT(p != NULL);
			// copy pixels from buffer to image
			bufferSize = width * height;
			std::memcpy((void *)p, (void *)pixelBuffer, bufferSize);
		}
		// image is full RGB 24 bit
		else {
			err = image.Reset(width, height, G_R8G8B8);
			if (err != G_NO_ERROR)
				goto finalize;
			// check for classid consistency
			p = image.Pixels();
			G_ASSERT(p != NULL);
			// convert from RGB to RGBA
			j = width * height;
			srcOfs = dstOfs = 0;
			for (i = 0; i < j; i++) {
				GUChar8 R = pixelBuffer[dstOfs++];
				GUChar8 G = pixelBuffer[dstOfs++];
				GUChar8 B = pixelBuffer[dstOfs++];
				p[srcOfs++] = B;
				p[srcOfs++] = G;
				p[srcOfs++] = R;
				p[srcOfs++] = 255;
			}
		}
	}
finalize:
	if (pixelBuffer)
		delete [] pixelBuffer;
	return err;

}

GError GJpegImpExp::WriteJpeg(const GChar8 *FullFileName, const GElement& Element, const GInt32 Quality,
							  const GBool ProgressiveEncoding) {

	GPixelMap& image = (GPixelMap&)Element;

	if (image.PixelsCount() <= 0)
		return G_INVALID_PARAMETER;

	GUChar8 *buf, *tmpBuf;
	GInt32 i, j, srcOfs, dstOfs;
	GError err;

	// in this case we are already done
	if (image.IsGrayScale()) {
		buf = image.Pixels();
		G_ASSERT(buf != NULL);
		err = RawJpegSave(FullFileName, image.Width(), image.Height(), 1, buf, Quality, ProgressiveEncoding);
	}
	else {
		GPixelMap tmpImage;
		err = G_NO_ERROR;
		// check for color conversion
		if (!image.IsTrueColor()) {
			// in these cases we can expand to a full 24bit RGB image
			err = image.SetPixelFormat(G_R8G8B8, tmpImage);
			buf = tmpImage.Pixels();
		}
		else
			buf = image.Pixels();

		// if pixel format conversion is well done save image
		if (err == G_NO_ERROR) {
			G_ASSERT(buf != NULL);			
			// we have to use a 3 components RGB buffer
			tmpBuf = new GUChar8[image.Width() * image.Height() * 3];
			// convert from RGBA to RGB (physical 32bits to physical 24bits)
			j = image.Width() * image.Height();
			srcOfs = dstOfs = 0;
			for (i = 0; i < j; i++) {
				GUChar8 b = buf[srcOfs++];
				GUChar8 g = buf[srcOfs++];
				GUChar8 r = buf[srcOfs++];
				srcOfs++;
				tmpBuf[dstOfs++] = r;
				tmpBuf[dstOfs++] = g;
				tmpBuf[dstOfs++] = b;
			}
			err = RawJpegSave(FullFileName, image.Width(), image.Height(), 3, tmpBuf,
							Quality, ProgressiveEncoding);
			delete [] tmpBuf;
		}
	}
	return err;
}

GError GJpegImpExp::DoRead(const GChar8 *FullFileName, GElement& Element, const GDynArray<GImpExpOption>& ParsedOptions) {

	// just to avoid warnings
	if (ParsedOptions.size() > 0) {
	}
	return ReadJpeg(FullFileName, Element);
}

GError GJpegImpExp::DoWrite(const GChar8 *FullFileName, const GElement& Element, const GDynArray<GImpExpOption>& ParsedOptions) {

	// default options
	GInt32 quality = 100;
	GBool progrEncoding = G_FALSE;

	// check each option
	GDynArray<GImpExpOption>::const_iterator it = ParsedOptions.begin();
	for (; it != ParsedOptions.end(); ++it) {
		if (StrUtils::SameText(it->OptionName, "quality"))
				quality = StrUtils::ToInt(it->OptionValue);
		else
		if (StrUtils::SameText(it->OptionName, "encoding"))
			if (StrUtils::SameText(it->OptionValue, "progressive"))
				progrEncoding = G_TRUE;
	}
	quality = GMath::Clamp(quality, 0, 100);
	return WriteJpeg(FullFileName, Element, quality, progrEncoding);
}

// export interface functions
#ifdef _JPEG_PLUGIN_EXTERNAL
G_EXTERN_C G_PLUGIN_EXPORT GUInt32 ProxiesCount() {
	return 1;
}
G_EXTERN_C G_PLUGIN_EXPORT const GElementProxy* ProxyInstance(const GUInt32 Index) {
	if (Index == 0)
		return &G_JPEGIMPEXP_PROXY;
	return NULL;
}
#endif

}

#endif
