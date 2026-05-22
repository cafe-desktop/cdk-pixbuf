/* GdkPixbuf library - GdkPixbuf data structure
 *
 * Copyright (C) 2003 The Free Software Foundation
 *
 * Authors: Mark Crichton <crichton@gimp.org>
 *          Miguel de Icaza <miguel@gnu.org>
 *          Federico Mena-Quintero <federico@gimp.org>
 *          Havoc Pennington <hp@redhat.com>
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

#ifndef CDK_PIXBUF_CORE_H
#define CDK_PIXBUF_CORE_H

#if defined(CDK_PIXBUF_DISABLE_SINGLE_INCLUDES) && !defined (CDK_PIXBUF_H_INSIDE) && !defined (CDK_PIXBUF_COMPILATION)
#error "Only <cdk-pixbuf/cdk-pixbuf.h> can be included directly."
#endif

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <cdk-pixbuf/cdk-pixbuf-macros.h>

G_BEGIN_DECLS

/**
 * GdkPixbufAlphaMode:
 * @CDK_PIXBUF_ALPHA_BILEVEL: A bilevel clipping mask (black and white)
 *  will be created and used to draw the image.  Pixels below 0.5 opacity
 *  will be considered fully transparent, and all others will be
 *  considered fully opaque.
 * @CDK_PIXBUF_ALPHA_FULL: For now falls back to #CDK_PIXBUF_ALPHA_BILEVEL.
 *  In the future it will do full alpha compositing.
 *
 * Control the alpha channel for drawables.
 *
 * These values can be passed to cdk_pixbuf_xlib_render_to_drawable_alpha()
 * in cdk-pixbuf-xlib to control how the alpha channel of an image should
 * be handled.
 *
 * This function can create a bilevel clipping mask (black and white) and use
 * it while painting the image.
 *
 * In the future, when the X Window System gets an alpha channel extension,
 * it will be possible to do full alpha compositing onto arbitrary drawables.
 * For now both cases fall back to a bilevel clipping mask.
 *
 * Deprecated: 2.42: There is no user of GdkPixbufAlphaMode in GdkPixbuf,
 *   and the Xlib utility functions have been split out to their own
 *   library, cdk-pixbuf-xlib
 */
typedef enum
{
        CDK_PIXBUF_ALPHA_BILEVEL,
        CDK_PIXBUF_ALPHA_FULL
} GdkPixbufAlphaMode;

/**
 * GdkColorspace:
 * @CDK_COLORSPACE_RGB: Indicates a red/green/blue additive color space.
 * 
 * This enumeration defines the color spaces that are supported by
 * the cdk-pixbuf library.
 *
 * Currently only RGB is supported.
 */
/* Note that these values are encoded in inline pixbufs
 * as ints, so don't reorder them
 */
typedef enum {
	CDK_COLORSPACE_RGB
} GdkColorspace;

/* All of these are opaque structures */

typedef struct _GdkPixbuf GdkPixbuf;

#define CDK_TYPE_PIXBUF              (cdk_pixbuf_get_type ())
#define CDK_PIXBUF(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), CDK_TYPE_PIXBUF, GdkPixbuf))
#define CDK_IS_PIXBUF(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), CDK_TYPE_PIXBUF))


/**
 * GdkPixbufDestroyNotify:
 * @pixels: (array) (element-type guint8): The pixel array of the pixbuf
 *   that is being finalized.
 * @data: (closure): User closure data.
 * 
 * A function of this type is responsible for freeing the pixel array
 * of a pixbuf.
 *
 * The cdk_pixbuf_new_from_data() function lets you pass in a pre-allocated
 * pixel array so that a pixbuf can be created from it; in this case you
 * will need to pass in a function of type `GdkPixbufDestroyNotify` so that
 * the pixel data can be freed when the pixbuf is finalized.
 */
typedef void (* GdkPixbufDestroyNotify) (guchar *pixels, gpointer data);

/**
 * CDK_PIXBUF_ERROR:
 * 
 * Error domain used for pixbuf operations.
 *
 * Indicates that the error code will be in the `GdkPixbufError` enumeration.
 *
 * See the `GError` for information on error domains and error codes.
 */
#define CDK_PIXBUF_ERROR cdk_pixbuf_error_quark ()

/**
 * GdkPixbufError:
 * @CDK_PIXBUF_ERROR_CORRUPT_IMAGE: An image file was broken somehow.
 * @CDK_PIXBUF_ERROR_INSUFFICIENT_MEMORY: Not enough memory.
 * @CDK_PIXBUF_ERROR_BAD_OPTION: A bad option was passed to a pixbuf save module.
 * @CDK_PIXBUF_ERROR_UNKNOWN_TYPE: Unknown image type.
 * @CDK_PIXBUF_ERROR_UNSUPPORTED_OPERATION: Don't know how to perform the
 *  given operation on the type of image at hand.
 * @CDK_PIXBUF_ERROR_FAILED: Generic failure code, something went wrong.
 * @CDK_PIXBUF_ERROR_INCOMPLETE_ANIMATION: Only part of the animation was loaded.
 * 
 * An error code in the `CDK_PIXBUF_ERROR` domain.
 *
 * Many cdk-pixbuf operations can cause errors in this domain, or in
 * the `G_FILE_ERROR` domain.
 */
typedef enum {
        /* image data hosed */
        CDK_PIXBUF_ERROR_CORRUPT_IMAGE,
        /* no mem to load image */
        CDK_PIXBUF_ERROR_INSUFFICIENT_MEMORY,
        /* bad option passed to save routine */
        CDK_PIXBUF_ERROR_BAD_OPTION,
        /* unsupported image type (sort of an ENOSYS) */
        CDK_PIXBUF_ERROR_UNKNOWN_TYPE,
        /* unsupported operation (load, save) for image type */
        CDK_PIXBUF_ERROR_UNSUPPORTED_OPERATION,
        CDK_PIXBUF_ERROR_FAILED,
        CDK_PIXBUF_ERROR_INCOMPLETE_ANIMATION
} GdkPixbufError;

CDK_PIXBUF_AVAILABLE_IN_ALL
GQuark cdk_pixbuf_error_quark (void);



CDK_PIXBUF_AVAILABLE_IN_ALL
GType cdk_pixbuf_get_type (void) G_GNUC_CONST;

/* Reference counting */

CDK_PIXBUF_DEPRECATED_IN_2_0_FOR(g_object_ref)
GdkPixbuf *cdk_pixbuf_ref      (GdkPixbuf *pixbuf);
CDK_PIXBUF_DEPRECATED_IN_2_0_FOR(g_object_unref)
void       cdk_pixbuf_unref    (GdkPixbuf *pixbuf);

/* GdkPixbuf accessors */

CDK_PIXBUF_AVAILABLE_IN_ALL
GdkColorspace cdk_pixbuf_get_colorspace      (const GdkPixbuf *pixbuf);
CDK_PIXBUF_AVAILABLE_IN_ALL
int           cdk_pixbuf_get_n_channels      (const GdkPixbuf *pixbuf);
CDK_PIXBUF_AVAILABLE_IN_ALL
gboolean      cdk_pixbuf_get_has_alpha       (const GdkPixbuf *pixbuf);
CDK_PIXBUF_AVAILABLE_IN_ALL
int           cdk_pixbuf_get_bits_per_sample (const GdkPixbuf *pixbuf);
CDK_PIXBUF_AVAILABLE_IN_ALL
guchar       *cdk_pixbuf_get_pixels          (const GdkPixbuf *pixbuf);
CDK_PIXBUF_AVAILABLE_IN_ALL
int           cdk_pixbuf_get_width           (const GdkPixbuf *pixbuf);
CDK_PIXBUF_AVAILABLE_IN_ALL
int           cdk_pixbuf_get_height          (const GdkPixbuf *pixbuf);
CDK_PIXBUF_AVAILABLE_IN_ALL
int           cdk_pixbuf_get_rowstride       (const GdkPixbuf *pixbuf);
CDK_PIXBUF_AVAILABLE_IN_2_26
gsize         cdk_pixbuf_get_byte_length     (const GdkPixbuf *pixbuf);

CDK_PIXBUF_AVAILABLE_IN_2_26
guchar       *cdk_pixbuf_get_pixels_with_length (const GdkPixbuf *pixbuf,
                                                 guint           *length);

CDK_PIXBUF_AVAILABLE_IN_2_32
const guint8* cdk_pixbuf_read_pixels         (const GdkPixbuf  *pixbuf);
CDK_PIXBUF_AVAILABLE_IN_2_32
GBytes *      cdk_pixbuf_read_pixel_bytes    (const GdkPixbuf  *pixbuf);



/* Create a blank pixbuf with an optimal rowstride and a new buffer */

CDK_PIXBUF_AVAILABLE_IN_ALL
GdkPixbuf *cdk_pixbuf_new (GdkColorspace colorspace, gboolean has_alpha, int bits_per_sample,
			   int width, int height);

CDK_PIXBUF_AVAILABLE_IN_2_36
gint cdk_pixbuf_calculate_rowstride (GdkColorspace colorspace,
				     gboolean      has_alpha,
				     int           bits_per_sample,
				     int           width,
				     int           height);

/* Copy a pixbuf */
CDK_PIXBUF_AVAILABLE_IN_ALL
GdkPixbuf *cdk_pixbuf_copy (const GdkPixbuf *pixbuf);

/* Create a pixbuf which points to the pixels of another pixbuf */
CDK_PIXBUF_AVAILABLE_IN_ALL
GdkPixbuf *cdk_pixbuf_new_subpixbuf (GdkPixbuf *src_pixbuf,
                                     int        src_x,
                                     int        src_y,
                                     int        width,
                                     int        height);

/* Simple loading */

#ifdef G_OS_WIN32
/* In previous versions these _utf8 variants where exported and linked to
 * by default. Export them here for ABI (and gi API) compat.
 */

CDK_PIXBUF_AVAILABLE_IN_ALL
GdkPixbuf *cdk_pixbuf_new_from_file_utf8 (const char *filename,
                                          GError    **error);
CDK_PIXBUF_AVAILABLE_IN_2_4
GdkPixbuf *cdk_pixbuf_new_from_file_at_size_utf8 (const char *filename,
                                                  int         width,
                                                  int         height,
                                                  GError    **error);
CDK_PIXBUF_AVAILABLE_IN_2_6
GdkPixbuf *cdk_pixbuf_new_from_file_at_scale_utf8 (const char *filename,
                                                   int         width,
                                                   int         height,
                                                   gboolean    preserve_aspect_ratio,
                                                   GError    **error);
#endif

CDK_PIXBUF_AVAILABLE_IN_ALL
GdkPixbuf *cdk_pixbuf_new_from_file (const char *filename,
                                     GError    **error);
CDK_PIXBUF_AVAILABLE_IN_2_4
GdkPixbuf *cdk_pixbuf_new_from_file_at_size (const char *filename,
					     int         width, 
					     int         height,
					     GError    **error);
CDK_PIXBUF_AVAILABLE_IN_2_6
GdkPixbuf *cdk_pixbuf_new_from_file_at_scale (const char *filename,
					      int         width, 
					      int         height,
					      gboolean    preserve_aspect_ratio,
					      GError    **error);
CDK_PIXBUF_AVAILABLE_IN_2_26
GdkPixbuf *cdk_pixbuf_new_from_resource (const char *resource_path,
					 GError    **error);
CDK_PIXBUF_AVAILABLE_IN_2_26
GdkPixbuf *cdk_pixbuf_new_from_resource_at_scale (const char *resource_path,
						  int         width,
						  int         height,
						  gboolean    preserve_aspect_ratio,
						  GError    **error);

CDK_PIXBUF_AVAILABLE_IN_ALL
GdkPixbuf *cdk_pixbuf_new_from_data (const guchar *data,
				     GdkColorspace colorspace,
				     gboolean has_alpha,
				     int bits_per_sample,
				     int width, int height,
				     int rowstride,
				     GdkPixbufDestroyNotify destroy_fn,
				     gpointer destroy_fn_data);

CDK_PIXBUF_AVAILABLE_IN_2_32
GdkPixbuf *cdk_pixbuf_new_from_bytes (GBytes *data,
				      GdkColorspace colorspace,
				      gboolean has_alpha,
				      int bits_per_sample,
				      int width, int height,
				      int rowstride);

CDK_PIXBUF_DEPRECATED_IN_2_44
GdkPixbuf *cdk_pixbuf_new_from_xpm_data (const char **data);

CDK_PIXBUF_DEPRECATED_IN_2_32
GdkPixbuf* cdk_pixbuf_new_from_inline	(gint          data_length,
					 const guint8 *data,
					 gboolean      copy_pixels,
					 GError      **error);
/* Mutations */
CDK_PIXBUF_AVAILABLE_IN_ALL
void       cdk_pixbuf_fill              (GdkPixbuf    *pixbuf,
                                         guint32       pixel);

/* Saving */

#ifndef __GTK_DOC_IGNORE__
#ifdef G_OS_WIN32
/* DLL ABI stability hack. */
#define cdk_pixbuf_save cdk_pixbuf_save_utf8
#endif
#endif

CDK_PIXBUF_AVAILABLE_IN_ALL
gboolean cdk_pixbuf_save           (GdkPixbuf  *pixbuf, 
                                    const char *filename, 
                                    const char *type, 
                                    GError    **error,
                                    ...) G_GNUC_NULL_TERMINATED;

CDK_PIXBUF_AVAILABLE_IN_ALL
gboolean cdk_pixbuf_savev          (GdkPixbuf  *pixbuf, 
                                    const char *filename, 
                                    const char *type,
                                    char      **option_keys,
                                    char      **option_values,
                                    GError    **error);

#ifdef G_OS_WIN32
CDK_PIXBUF_AVAILABLE_IN_ALL
gboolean cdk_pixbuf_savev_utf8     (GdkPixbuf  *pixbuf,
                                    const char *filename,
                                    const char *type,
                                    char      **option_keys,
                                    char      **option_values,
                                    GError    **error);
#endif

/* Saving to a callback function */


/**
 * GdkPixbufSaveFunc:
 * @buf: (array length=count) (element-type guint8): bytes to be written.
 * @count: number of bytes in @buf. 
 * @error: (out): A location to return an error.
 * @data: (closure): user data passed to cdk_pixbuf_save_to_callback(). 
 * 
 * Save functions used by [method@GdkPixbuf.Pixbuf.save_to_callback].
 *
 * This function is called once for each block of bytes that is "written"
 * by `cdk_pixbuf_save_to_callback()`.
 *
 * If successful it should return `TRUE`; if an error occurs it should set
 * `error` and return `FALSE`, in which case `cdk_pixbuf_save_to_callback()`
 * will fail with the same error.
 *
 * Returns: `TRUE` if successful, `FALSE` otherwise
 * 
 * Since: 2.4
 */

typedef gboolean (*GdkPixbufSaveFunc)   (const gchar *buf,
					 gsize count,
					 GError **error,
					 gpointer data);

CDK_PIXBUF_AVAILABLE_IN_2_4
gboolean cdk_pixbuf_save_to_callback    (GdkPixbuf  *pixbuf,
					 GdkPixbufSaveFunc save_func,
					 gpointer user_data,
					 const char *type, 
					 GError    **error,
					 ...) G_GNUC_NULL_TERMINATED;

CDK_PIXBUF_AVAILABLE_IN_2_4
gboolean cdk_pixbuf_save_to_callbackv   (GdkPixbuf  *pixbuf, 
					 GdkPixbufSaveFunc save_func,
					 gpointer user_data,
					 const char *type,
					 char      **option_keys,
					 char      **option_values,
					 GError    **error);

/* Saving into a newly allocated char array */

CDK_PIXBUF_AVAILABLE_IN_2_4
gboolean cdk_pixbuf_save_to_buffer      (GdkPixbuf  *pixbuf,
					 gchar     **buffer,
					 gsize      *buffer_size,
					 const char *type, 
					 GError    **error,
					 ...) G_GNUC_NULL_TERMINATED;

CDK_PIXBUF_AVAILABLE_IN_2_4
gboolean cdk_pixbuf_save_to_bufferv     (GdkPixbuf  *pixbuf,
					 gchar     **buffer,
					 gsize      *buffer_size,
					 const char *type, 
					 char      **option_keys,
					 char      **option_values,
					 GError    **error);

CDK_PIXBUF_AVAILABLE_IN_2_14
GdkPixbuf *cdk_pixbuf_new_from_stream   (GInputStream   *stream,
					 GCancellable   *cancellable,
                                         GError        **error);

CDK_PIXBUF_AVAILABLE_IN_ALL
void cdk_pixbuf_new_from_stream_async (GInputStream        *stream,
				       GCancellable        *cancellable,
				       GAsyncReadyCallback  callback,
				       gpointer             user_data);

CDK_PIXBUF_AVAILABLE_IN_ALL
GdkPixbuf *cdk_pixbuf_new_from_stream_finish (GAsyncResult  *async_result,
					      GError       **error);

CDK_PIXBUF_AVAILABLE_IN_2_14
GdkPixbuf *cdk_pixbuf_new_from_stream_at_scale   (GInputStream   *stream,
                                                  gint            width,
                                                  gint            height,
                                                  gboolean        preserve_aspect_ratio,
						  GCancellable   *cancellable,
                                                  GError        **error);

CDK_PIXBUF_AVAILABLE_IN_ALL
void cdk_pixbuf_new_from_stream_at_scale_async (GInputStream        *stream,
						gint                 width,
						gint                 height,
						gboolean             preserve_aspect_ratio,
						GCancellable        *cancellable,
						GAsyncReadyCallback  callback,
						gpointer             user_data);

CDK_PIXBUF_AVAILABLE_IN_2_14
gboolean   cdk_pixbuf_save_to_stream    (GdkPixbuf      *pixbuf,
                                         GOutputStream  *stream,
                                         const char     *type,
					 GCancellable   *cancellable,
                                         GError        **error,
                                         ...);

CDK_PIXBUF_AVAILABLE_IN_ALL
void cdk_pixbuf_save_to_stream_async (GdkPixbuf           *pixbuf,
				      GOutputStream       *stream,
				      const gchar         *type,
				      GCancellable        *cancellable,
				      GAsyncReadyCallback  callback,
				      gpointer             user_data,
				      ...);

CDK_PIXBUF_AVAILABLE_IN_ALL
gboolean cdk_pixbuf_save_to_stream_finish (GAsyncResult  *async_result,
					   GError       **error);

CDK_PIXBUF_AVAILABLE_IN_2_36
void cdk_pixbuf_save_to_streamv_async (GdkPixbuf           *pixbuf,
                                       GOutputStream       *stream,
                                       const gchar         *type,
                                       gchar              **option_keys,
                                       gchar              **option_values,
                                       GCancellable        *cancellable,
                                       GAsyncReadyCallback  callback,
                                       gpointer             user_data);

CDK_PIXBUF_AVAILABLE_IN_2_36
gboolean cdk_pixbuf_save_to_streamv (GdkPixbuf      *pixbuf,
                                     GOutputStream  *stream,
                                     const char     *type,
                                     char          **option_keys,
                                     char          **option_values,
                                     GCancellable   *cancellable,
                                     GError        **error);

/* Adding an alpha channel */
CDK_PIXBUF_AVAILABLE_IN_ALL
GdkPixbuf *cdk_pixbuf_add_alpha (const GdkPixbuf *pixbuf, gboolean substitute_color,
				 guchar r, guchar g, guchar b);

/* Copy an area of a pixbuf onto another one */
CDK_PIXBUF_AVAILABLE_IN_ALL
void cdk_pixbuf_copy_area (const GdkPixbuf *src_pixbuf,
			   int src_x, int src_y,
			   int width, int height,
			   GdkPixbuf *dest_pixbuf,
			   int dest_x, int dest_y);

/* Brighten/darken and optionally make it pixelated-looking */
CDK_PIXBUF_AVAILABLE_IN_ALL
void cdk_pixbuf_saturate_and_pixelate (const GdkPixbuf *src,
                                       GdkPixbuf       *dest,
                                       gfloat           saturation,
                                       gboolean         pixelate);

/* Transform an image to agree with its embedded orientation option / tag */
CDK_PIXBUF_AVAILABLE_IN_2_12
GdkPixbuf *cdk_pixbuf_apply_embedded_orientation (GdkPixbuf *src);

/*  key/value pairs that can be attached by the pixbuf loader  */
CDK_PIXBUF_AVAILABLE_IN_ALL
gboolean cdk_pixbuf_set_option  (GdkPixbuf   *pixbuf,
                                 const gchar *key,
                                 const gchar *value);
CDK_PIXBUF_AVAILABLE_IN_ALL
const gchar * cdk_pixbuf_get_option (GdkPixbuf   *pixbuf,
                                              const gchar *key);
CDK_PIXBUF_AVAILABLE_IN_2_36
gboolean cdk_pixbuf_remove_option (GdkPixbuf   *pixbuf,
                                   const gchar *key);
CDK_PIXBUF_AVAILABLE_IN_2_32
GHashTable * cdk_pixbuf_get_options (GdkPixbuf   *pixbuf);
CDK_PIXBUF_AVAILABLE_IN_2_36
gboolean cdk_pixbuf_copy_options (GdkPixbuf *src_pixbuf,
                                  GdkPixbuf *dest_pixbuf);


G_DEFINE_AUTOPTR_CLEANUP_FUNC(GdkPixbuf, g_object_unref)

G_END_DECLS

#endif /* CDK_PIXBUF_CORE_H */
