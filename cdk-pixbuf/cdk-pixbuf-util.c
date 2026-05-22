/* CdkPixbuf library - Utilities and miscellaneous convenience functions
 *
 * Copyright (C) 1999 The Free Software Foundation
 *
 * Authors: Federico Mena-Quintero <federico@gimp.org>
 *          Cody Russell  <bratsche@gnome.org>
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

#include "config.h"
#include <string.h>
#include <libintl.h>

#include "cdk-pixbuf-transform.h"
#include "cdk-pixbuf-private.h"

/**
 * cdk_pixbuf_add_alpha:
 * @pixbuf: A #CdkPixbuf.
 * @substitute_color: Whether to set a color to zero opacity.
 * @r: Red value to substitute.
 * @g: Green value to substitute.
 * @b: Blue value to substitute.
 *
 * Takes an existing pixbuf and adds an alpha channel to it.
 *
 * If the existing pixbuf already had an alpha channel, the channel
 * values are copied from the original; otherwise, the alpha channel
 * is initialized to 255 (full opacity).
 * 
 * If `substitute_color` is `TRUE`, then the color specified by the
 * (`r`, `g`, `b`) arguments will be assigned zero opacity. That is,
 * if you pass `(255, 255, 255)` for the substitute color, all white
 * pixels will become fully transparent.
 *
 * If `substitute_color` is `FALSE`, then the (`r`, `g`, `b`) arguments
 * will be ignored.
 *
 * Returns: (transfer full) (nullable): A newly-created pixbuf
 **/
CdkPixbuf *
cdk_pixbuf_add_alpha (const CdkPixbuf *pixbuf,
                      gboolean substitute_color,
                      guchar r,
                      guchar g,
                      guchar b)
{
	CdkPixbuf *new_pixbuf;
	int x, y;
	const guint8 *src_pixels;
	guint8 *ret_pixels;
	const guchar *src;
	guchar *dest;

	g_return_val_if_fail (CDK_IS_PIXBUF (pixbuf), NULL);
	g_return_val_if_fail (pixbuf->colorspace == CDK_COLORSPACE_RGB, NULL);
	g_return_val_if_fail (pixbuf->n_channels == 3 || pixbuf->n_channels == 4, NULL);
	g_return_val_if_fail (pixbuf->bits_per_sample == 8, NULL);

	src_pixels = cdk_pixbuf_read_pixels (pixbuf);

	if (pixbuf->has_alpha) {
		new_pixbuf = cdk_pixbuf_copy (pixbuf);
		if (!new_pixbuf)
			return NULL;

                if (!substitute_color)
                        return new_pixbuf;
	} else {
                new_pixbuf = cdk_pixbuf_new (CDK_COLORSPACE_RGB, TRUE, 8, pixbuf->width, pixbuf->height);
        }

	if (!new_pixbuf)
		return NULL;

	ret_pixels = cdk_pixbuf_get_pixels (new_pixbuf);

	for (y = 0; y < pixbuf->height; y++, src_pixels += pixbuf->rowstride, ret_pixels += new_pixbuf->rowstride) {
		guchar tr, tg, tb;

                src = src_pixels;
                dest = ret_pixels;

                if (pixbuf->has_alpha) {
                        /* Just subst color, we already copied everything else */
                        for (x = 0; x < pixbuf->width; x++) {
                                if (src[0] == r && src[1] == g && src[2] == b)
                                        dest[3] = 0;
                                src += 4;
                                dest += 4;
                        }
                } else {
                        for (x = 0; x < pixbuf->width; x++) {
                                tr = *dest++ = *src++;
                                tg = *dest++ = *src++;
                                tb = *dest++ = *src++;

                                if (substitute_color && tr == r && tg == g && tb == b)
                                        *dest++ = 0;
                                else
                                        *dest++ = 255;
                        }
		}
	}

	return new_pixbuf;
}

/**
 * cdk_pixbuf_copy_area:
 * @src_pixbuf: Source pixbuf.
 * @src_x: Source X coordinate within @src_pixbuf.
 * @src_y: Source Y coordinate within @src_pixbuf.
 * @width: Width of the area to copy.
 * @height: Height of the area to copy.
 * @dest_pixbuf: Destination pixbuf.
 * @dest_x: X coordinate within @dest_pixbuf.
 * @dest_y: Y coordinate within @dest_pixbuf.
 *
 * Copies a rectangular area from `src_pixbuf` to `dest_pixbuf`.
 *
 * Conversion of pixbuf formats is done automatically.
 *
 * If the source rectangle overlaps the destination rectangle on the
 * same pixbuf, it will be overwritten during the copy operation.
 * Therefore, you can not use this function to scroll a pixbuf.
 **/
void
cdk_pixbuf_copy_area (const CdkPixbuf *src_pixbuf,
		      int src_x, int src_y,
		      int width, int height,
		      CdkPixbuf *dest_pixbuf,
		      int dest_x, int dest_y)
{
	g_return_if_fail (src_pixbuf != NULL);
	g_return_if_fail (dest_pixbuf != NULL);

	g_return_if_fail (src_x >= 0 && src_x + width <= src_pixbuf->width);
	g_return_if_fail (src_y >= 0 && src_y + height <= src_pixbuf->height);

	g_return_if_fail (dest_x >= 0 && dest_x + width <= dest_pixbuf->width);
	g_return_if_fail (dest_y >= 0 && dest_y + height <= dest_pixbuf->height);

        g_return_if_fail (!(cdk_pixbuf_get_has_alpha (src_pixbuf) && !cdk_pixbuf_get_has_alpha (dest_pixbuf)));
        
	/* This will perform format conversions automatically */

	cdk_pixbuf_scale (src_pixbuf,
			  dest_pixbuf,
			  dest_x, dest_y,
			  width, height,
			  (double) (dest_x - src_x),
			  (double) (dest_y - src_y),
			  1.0, 1.0,
			  CDK_INTERP_NEAREST);
}



/**
 * cdk_pixbuf_saturate_and_pixelate:
 * @src: source image
 * @dest: place to write modified version of @src
 * @saturation: saturation factor
 * @pixelate: whether to pixelate
 *
 * Modifies saturation and optionally pixelates `src`, placing the result in
 * `dest`.
 *
 * The `src` and `dest` pixbufs must have the same image format, size, and
 * rowstride.
 *
 * The `src` and `dest` arguments may be the same pixbuf with no ill effects.
 *
 * If `saturation` is 1.0 then saturation is not changed. If it's less than 1.0,
 * saturation is reduced (the image turns toward grayscale); if greater than
 * 1.0, saturation is increased (the image gets more vivid colors).
 *
 * If `pixelate` is `TRUE`, then pixels are faded in a checkerboard pattern to
 * create a pixelated image.
 * 
 **/
void
cdk_pixbuf_saturate_and_pixelate (const CdkPixbuf *src,
                                  CdkPixbuf *dest,
                                  gfloat saturation,
                                  gboolean pixelate)
{
        /* NOTE that src and dest MAY be the same pixbuf! */
  
        g_return_if_fail (CDK_IS_PIXBUF (src));
        g_return_if_fail (CDK_IS_PIXBUF (dest));
        g_return_if_fail (cdk_pixbuf_get_height (src) == cdk_pixbuf_get_height (dest));
        g_return_if_fail (cdk_pixbuf_get_width (src) == cdk_pixbuf_get_width (dest));
        g_return_if_fail (cdk_pixbuf_get_has_alpha (src) == cdk_pixbuf_get_has_alpha (dest));
        g_return_if_fail (cdk_pixbuf_get_colorspace (src) == cdk_pixbuf_get_colorspace (dest));
  
        if (saturation == 1.0 && !pixelate) {
                if (dest != src)
                        cdk_pixbuf_copy_area (src, 0, 0, 
                                              cdk_pixbuf_get_width (src),
                                              cdk_pixbuf_get_height (src),
                                              dest, 0, 0);
        } else {
                int i, j, t;
                int width, height, has_alpha, src_rowstride, dest_rowstride, bytes_per_pixel;
		const guchar *src_line;
		guchar *dest_line;
                const guchar *src_pixel;
		guchar *dest_pixel;
                guchar intensity;

                has_alpha = cdk_pixbuf_get_has_alpha (src);
		bytes_per_pixel = has_alpha ? 4 : 3;
                width = cdk_pixbuf_get_width (src);
                height = cdk_pixbuf_get_height (src);
                src_rowstride = cdk_pixbuf_get_rowstride (src);
                dest_rowstride = cdk_pixbuf_get_rowstride (dest);
                
                dest_line = cdk_pixbuf_get_pixels (dest);
                src_line = cdk_pixbuf_read_pixels (src);
		
#define DARK_FACTOR 0.7
#define INTENSITY(r, g, b) ((r) * 0.30 + (g) * 0.59 + (b) * 0.11)
#define CLAMP_UCHAR(v) (t = (v), CLAMP (t, 0, 255))
#define SATURATE(v) ((1.0 - saturation) * intensity + saturation * (v))

		for (i = 0 ; i < height ; i++) {
			src_pixel = src_line;
			src_line += src_rowstride;
			dest_pixel = dest_line;
			dest_line += dest_rowstride;

			for (j = 0 ; j < width ; j++) {
                                intensity = INTENSITY (src_pixel[0], src_pixel[1], src_pixel[2]);
                                if (pixelate && (i + j) % 2 == 0) {
                                        dest_pixel[0] = intensity / 2 + 127;
                                        dest_pixel[1] = intensity / 2 + 127;
                                        dest_pixel[2] = intensity / 2 + 127;
                                } else if (pixelate) {
                                        dest_pixel[0] = CLAMP_UCHAR ((SATURATE (src_pixel[0])) * DARK_FACTOR);
					dest_pixel[1] = CLAMP_UCHAR ((SATURATE (src_pixel[1])) * DARK_FACTOR);
                                        dest_pixel[2] = CLAMP_UCHAR ((SATURATE (src_pixel[2])) * DARK_FACTOR);
                                } else {
                                        dest_pixel[0] = CLAMP_UCHAR (SATURATE (src_pixel[0]));
                                        dest_pixel[1] = CLAMP_UCHAR (SATURATE (src_pixel[1]));
                                        dest_pixel[2] = CLAMP_UCHAR (SATURATE (src_pixel[2]));
                                }
				
                                if (has_alpha)
                                        dest_pixel[3] = src_pixel[3];

				src_pixel += bytes_per_pixel;
				dest_pixel += bytes_per_pixel;
			}
                }
        }
}


/**
 * cdk_pixbuf_apply_embedded_orientation:
 * @src: a pixbuf with an orientation option
 *
 * Takes an existing pixbuf and checks for the presence of an
 * associated "orientation" option.
 *
 * The orientation option may be provided by the JPEG loader (which
 * reads the exif orientation tag) or the TIFF loader (which reads
 * the TIFF orientation tag, and compensates it for the partial
 * transforms performed by libtiff).
 *
 * If an orientation option/tag is present, the appropriate transform
 * will be performed so that the pixbuf is oriented correctly.
 *
 * Return: (transfer full) (nullable): A newly-created pixbuf
 *
 * Since: 2.12
 **/
CdkPixbuf *
cdk_pixbuf_apply_embedded_orientation (CdkPixbuf *src)
{
  	const gchar *orientation_string;
	int          transform = 0;
	CdkPixbuf   *temp;
	CdkPixbuf   *dest;

	g_return_val_if_fail (CDK_IS_PIXBUF (src), NULL);

	/* Read the orientation option associated with the pixbuf */
	orientation_string = cdk_pixbuf_get_option (src, "orientation");	

	if (orientation_string) {
		/* If an orientation option was found, convert the 
		   orientation string into an integer. */
		transform = (int) g_ascii_strtoll (orientation_string, NULL, 10);
	}

	/* Apply the actual transforms, which involve rotations and flips. 
	   The meaning of orientation values 1-8 and the required transforms
	   are defined by the TIFF and EXIF (for JPEGs) standards. */
        switch (transform) {
        case 1:
                dest = src;
                g_object_ref (dest);
                break;
        case 2:
                dest = cdk_pixbuf_flip (src, TRUE);
                break;
        case 3:
                dest = cdk_pixbuf_rotate_simple (src, CDK_PIXBUF_ROTATE_UPSIDEDOWN);
                break;
        case 4:
                dest = cdk_pixbuf_flip (src, FALSE);
                break;
        case 5:
                temp = cdk_pixbuf_rotate_simple (src, CDK_PIXBUF_ROTATE_CLOCKWISE);
                dest = cdk_pixbuf_flip (temp, TRUE);
                g_object_unref (temp);
                break;
        case 6:
                dest = cdk_pixbuf_rotate_simple (src, CDK_PIXBUF_ROTATE_CLOCKWISE);
                break;
        case 7:
                temp = cdk_pixbuf_rotate_simple (src, CDK_PIXBUF_ROTATE_CLOCKWISE);
                dest = cdk_pixbuf_flip (temp, FALSE);
                g_object_unref (temp);
                break;
        case 8:
                dest = cdk_pixbuf_rotate_simple (src, CDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
                break;
        default:
		/* if no orientation tag was present */
                dest = src;
                g_object_ref (dest);
                break;
        }

        return dest;
}

#ifdef CDK_PIXBUF_RELOCATABLE

static const gchar *
get_localedir (void)
{
    gchar *temp;
    
    temp = g_build_filename (cdk_pixbuf_get_toplevel (), "share/locale", NULL);

#ifdef G_OS_WIN32
    {
      gchar *retval;
      /* The localedir is passed to bindtextdomain() which isn't
      * UTF-8-aware.
      */
      retval = g_win32_locale_filename_from_utf8 (temp);
      g_free (temp);
      return retval;
    }
#else
    return temp;
#endif
}

#undef CDK_PIXBUF_LOCALEDIR
#define CDK_PIXBUF_LOCALEDIR get_localedir ()

#endif

void
_cdk_pixbuf_init_gettext (void)
{
        static gsize gettext_initialized = FALSE;

        if (G_UNLIKELY (g_once_init_enter (&gettext_initialized))) {
                bindtextdomain (GETTEXT_PACKAGE, CDK_PIXBUF_LOCALEDIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
                bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
                g_once_init_leave (&gettext_initialized, TRUE);
        }
}

const gchar *
cdk_pixbuf_gettext (const gchar *msgid)
{
        return g_dgettext (GETTEXT_PACKAGE, msgid);
}
