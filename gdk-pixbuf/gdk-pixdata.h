/* GdkPixbuf library - GdkPixdata - functions for inlined pixbuf handling
 * Copyright (C) 1999, 2001 Tim Janik
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __CDK_PIXDATA_H__
#define __CDK_PIXDATA_H__

#include        <cdk-pixbuf/cdk-pixbuf.h>

G_BEGIN_DECLS

/**
 * CDK_PIXBUF_MAGIC_NUMBER:
 *
 * Magic number for #GdkPixdata structures.
 **/
#define CDK_PIXBUF_MAGIC_NUMBER (0x47646b50)    /* 'GdkP' */

/**
 * GdkPixdataType:
 * @CDK_PIXDATA_COLOR_TYPE_RGB:  each pixel has red, green and blue samples.
 * @CDK_PIXDATA_COLOR_TYPE_RGBA: each pixel has red, green and blue samples 
 *    and an alpha value.
 * @CDK_PIXDATA_COLOR_TYPE_MASK: mask for the colortype flags of the enum.
 * @CDK_PIXDATA_SAMPLE_WIDTH_8: each sample has 8 bits.
 * @CDK_PIXDATA_SAMPLE_WIDTH_MASK: mask for the sample width flags of the enum.
 * @CDK_PIXDATA_ENCODING_RAW: the pixel data is in raw form. 
 * @CDK_PIXDATA_ENCODING_RLE: the pixel data is run-length encoded. Runs may 
 *    be up to 127 bytes long; their length is stored in a single byte 
 *    preceding the pixel data for the run. If a run is constant, its length
 *    byte has the high bit set and the pixel data consists of a single pixel
 *    which must be repeated. 
 * @CDK_PIXDATA_ENCODING_MASK: mask for the encoding flags of the enum.
 *
 * An enumeration containing three sets of flags for a #GdkPixdata struct: 
 * one for the used colorspace, one for the width of the samples and one 
 * for the encoding of the pixel data.
 *
 * Deprecated: 2.32
 **/
typedef enum
{
  /* colorspace + alpha */
  CDK_PIXDATA_COLOR_TYPE_RGB    = 0x01,
  CDK_PIXDATA_COLOR_TYPE_RGBA   = 0x02,
  CDK_PIXDATA_COLOR_TYPE_MASK   = 0xff,
  /* width, support 8bits only currently */
  CDK_PIXDATA_SAMPLE_WIDTH_8    = 0x01 << 16,
  CDK_PIXDATA_SAMPLE_WIDTH_MASK = 0x0f << 16,
  /* encoding */
  CDK_PIXDATA_ENCODING_RAW      = 0x01 << 24,
  CDK_PIXDATA_ENCODING_RLE      = 0x02 << 24,
  CDK_PIXDATA_ENCODING_MASK     = 0x0f << 24
} GdkPixdataType;

typedef struct _GdkPixdata GdkPixdata;
struct _GdkPixdata
{
  guint32 magic;        /* CDK_PIXBUF_MAGIC_NUMBER */
  gint32  length;       /* <1 to disable length checks, otherwise:
			 * CDK_PIXDATA_HEADER_LENGTH + pixel_data length
			 */
  guint32 pixdata_type; /* GdkPixdataType */
  guint32 rowstride;
  guint32 width;
  guint32 height;
  guint8 *pixel_data;
};

/**
 * CDK_PIXDATA_HEADER_LENGTH:
 *
 * The length of a #GdkPixdata structure without the @pixel_data pointer.
 *
 * Deprecated: 2.32
 **/
#define	CDK_PIXDATA_HEADER_LENGTH	(4 + 4 + 4 + 4 + 4 + 4)

/* the returned stream is plain htonl of GdkPixdata members + pixel_data */
CDK_PIXBUF_DEPRECATED_IN_2_32
guint8*		cdk_pixdata_serialize	(const GdkPixdata	*pixdata,
					 guint			*stream_length_p);
CDK_PIXBUF_DEPRECATED_IN_2_32
gboolean	cdk_pixdata_deserialize	(GdkPixdata		*pixdata,
					 guint			 stream_length,
					 const guint8		*stream,
					 GError		       **error);
CDK_PIXBUF_DEPRECATED_IN_2_32
gpointer	cdk_pixdata_from_pixbuf	(GdkPixdata		*pixdata,
					 const GdkPixbuf	*pixbuf,
					 gboolean		 use_rle);
CDK_PIXBUF_DEPRECATED_IN_2_32
GdkPixbuf*	cdk_pixbuf_from_pixdata	(const GdkPixdata	*pixdata,
					 gboolean		 copy_pixels,
					 GError		       **error);
/** 
 * GdkPixdataDumpType:
 * @CDK_PIXDATA_DUMP_PIXDATA_STREAM: Generate pixbuf data stream (a single 
 *    string containing a serialized #GdkPixdata structure in network byte 
 *    order).
 * @CDK_PIXDATA_DUMP_PIXDATA_STRUCT: Generate #GdkPixdata structure (needs 
 *    the #GdkPixdata structure definition from cdk-pixdata.h).
 * @CDK_PIXDATA_DUMP_MACROS: Generate <function>*_ROWSTRIDE</function>,     
 *    <function>*_WIDTH</function>, <function>*_HEIGHT</function>,
 *    <function>*_BYTES_PER_PIXEL</function> and 
 *    <function>*_RLE_PIXEL_DATA</function> or <function>*_PIXEL_DATA</function>
 *    macro definitions for the image.
 * @CDK_PIXDATA_DUMP_GTYPES: Generate GLib data types instead of 
 *    standard C data types.
 * @CDK_PIXDATA_DUMP_CTYPES: Generate standard C data types instead of 
 *    GLib data types.
 * @CDK_PIXDATA_DUMP_STATIC: Generate static symbols.
 * @CDK_PIXDATA_DUMP_CONST: Generate const symbols.
 * @CDK_PIXDATA_DUMP_RLE_DECODER: Provide a <function>*_RUN_LENGTH_DECODE(image_buf, rle_data, size, bpp)</function> 
 *    macro definition  to  decode  run-length encoded image data.
 *  
 * An enumeration which is used by cdk_pixdata_to_csource() to
 * determine the form of C source to be generated. The three values
 * @CDK_PIXDATA_DUMP_PIXDATA_STREAM, @CDK_PIXDATA_DUMP_PIXDATA_STRUCT
 * and @CDK_PIXDATA_DUMP_MACROS are mutually exclusive, as are
 * @CDK_PIXBUF_DUMP_GTYPES and @CDK_PIXBUF_DUMP_CTYPES. The remaining
 * elements are optional flags that can be freely added.
 *
 * Deprecated: 2.32
 **/
typedef enum
{
  /* type of source to save */
  CDK_PIXDATA_DUMP_PIXDATA_STREAM	= 0,
  CDK_PIXDATA_DUMP_PIXDATA_STRUCT	= 1,
  CDK_PIXDATA_DUMP_MACROS		= 2,
  /* type of variables to use */
  CDK_PIXDATA_DUMP_GTYPES		= 0,
  CDK_PIXDATA_DUMP_CTYPES		= 1 << 8,
  CDK_PIXDATA_DUMP_STATIC		= 1 << 9,
  CDK_PIXDATA_DUMP_CONST		= 1 << 10,
  /* save RLE decoder macro? */
  CDK_PIXDATA_DUMP_RLE_DECODER		= 1 << 16
} GdkPixdataDumpType;
  

CDK_PIXBUF_DEPRECATED_IN_2_32
GString*	cdk_pixdata_to_csource	(GdkPixdata		*pixdata,
					 const gchar		*name,
					 GdkPixdataDumpType	 dump_type);


G_END_DECLS

#endif /* __CDK_PIXDATA_H__ */
