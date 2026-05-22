/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* CdkPixbuf library - Progressive loader object
 *
 * Copyright (C) 1999 The Free Software Foundation
 *
 * Authors: Mark Crichton <crichton@gimp.org>
 *          Miguel de Icaza <miguel@gnu.org>
 *          Federico Mena-Quintero <federico@gimp.org>
 *          Jonathan Blandford <jrb@redhat.com>
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

#include "cdk-pixbuf-private.h"
#include "cdk-pixbuf-animation.h"
#include "cdk-pixbuf-scaled-anim.h"
#include "cdk-pixbuf-loader.h"
#include "cdk-pixbuf-marshal.h"

/**
 * CdkPixbufLoader:
 *
 * Incremental image loader.
 * 
 * `CdkPixbufLoader` provides a way for applications to drive the
 * process of loading an image, by letting them send the image data
 * directly to the loader instead of having the loader read the data
 * from a file. Applications can use this functionality instead of
 * `cdk_pixbuf_new_from_file()` or `cdk_pixbuf_animation_new_from_file()`
 * when they need to parse image data in small chunks. For example,
 * it should be used when reading an image from a (potentially) slow
 * network connection, or when loading an extremely large file.
 *
 * To use `CdkPixbufLoader` to load an image, create a new instance,
 * and call [method@CdkPixbuf.PixbufLoader.write] to send the data
 * to it. When done, [method@CdkPixbuf.PixbufLoader.close] should be
 * called to end the stream and finalize everything.
 *
 * The loader will emit three important signals throughout the process:
 *
 *  - [signal@CdkPixbuf.PixbufLoader::size-prepared] will be emitted as
 *    soon as the image has enough information to determine the size of
 *    the image to be used. If you want to scale the image while loading
 *    it, you can call [method@CdkPixbuf.PixbufLoader.set_size] in
 *    response to this signal.
 *  - [signal@CdkPixbuf.PixbufLoader::area-prepared] will be emitted as
 *    soon as the pixbuf of the desired has been allocated. You can obtain
 *    the `CdkPixbuf` instance by calling [method@CdkPixbuf.PixbufLoader.get_pixbuf].
 *    If you want to use it, simply acquire a reference to it. You can
 *    also call `cdk_pixbuf_loader_get_pixbuf()` later to get the same
 *    pixbuf.
 *  - [signal@CdkPixbuf.PixbufLoader::area-updated] will be emitted every
 *    time a region is updated. This way you can update a partially
 *    completed image. Note that you do not know anything about the
 *    completeness of an image from the updated area. For example, in an
 *    interlaced image you will need to make several passes before the
 *    image is done loading.
 * 
 * ## Loading an animation
 *
 * Loading an animation is almost as easy as loading an image. Once the
 * first [signal@CdkPixbuf.PixbufLoader::area-prepared] signal has been
 * emitted, you can call [method@CdkPixbuf.PixbufLoader.get_animation] to
 * get the [class@CdkPixbuf.PixbufAnimation] instance, and then call
 * and [method@CdkPixbuf.PixbufAnimation.get_iter] to get a
 * [class@CdkPixbuf.PixbufAnimationIter] to retrieve the pixbuf for the
 * desired time stamp.
 */


enum {
        SIZE_PREPARED,
        AREA_PREPARED,
        AREA_UPDATED,
        CLOSED,
        LAST_SIGNAL
};


static void cdk_pixbuf_loader_finalize (GObject *loader);

static guint    pixbuf_loader_signals[LAST_SIGNAL] = { 0 };

/* Internal data */

typedef struct
{
        CdkPixbufAnimation *animation;
        gboolean closed;
        guchar header_buf[SNIFF_BUFFER_SIZE];
        gint header_buf_offset;
        CdkPixbufModule *image_module;
        gpointer context;
        gint original_width;
        gint original_height;
        gint width;
        gint height;
        gboolean size_fixed;
        gboolean needs_scale;
	gchar *filename;
} CdkPixbufLoaderPrivate;

G_DEFINE_TYPE (CdkPixbufLoader, cdk_pixbuf_loader, G_TYPE_OBJECT)


static void
cdk_pixbuf_loader_class_init (CdkPixbufLoaderClass *class)
{
        GObjectClass *object_class;
  
        object_class = (GObjectClass *) class;
  
        object_class->finalize = cdk_pixbuf_loader_finalize;

        /**
         * CdkPixbufLoader::size-prepared:
         * @loader: the object which received the signal.
         * @width: the original width of the image
         * @height: the original height of the image
         *
         * This signal is emitted when the pixbuf loader has been fed the
         * initial amount of data that is required to figure out the size
         * of the image that it will create.
         *
         * Applications can call cdk_pixbuf_loader_set_size() in response
         * to this signal to set the desired size to which the image
         * should be scaled.
         */
        pixbuf_loader_signals[SIZE_PREPARED] =
                g_signal_new ("size-prepared",
                              G_TYPE_FROM_CLASS (object_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (CdkPixbufLoaderClass, size_prepared),
                              NULL, NULL,
                              _cdk_pixbuf_marshal_VOID__INT_INT,
                              G_TYPE_NONE, 2, 
                              G_TYPE_INT,
                              G_TYPE_INT);
  
        /**
         * CdkPixbufLoader::area-prepared:
         * @loader: the object which received the signal.
         *
         * This signal is emitted when the pixbuf loader has allocated the 
         * pixbuf in the desired size.
         *
         * After this signal is emitted, applications can call
         * cdk_pixbuf_loader_get_pixbuf() to fetch the partially-loaded
         * pixbuf.
         */
        pixbuf_loader_signals[AREA_PREPARED] =
                g_signal_new ("area-prepared",
                              G_TYPE_FROM_CLASS (object_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (CdkPixbufLoaderClass, area_prepared),
                              NULL, NULL,
                              _cdk_pixbuf_marshal_VOID__VOID,
                              G_TYPE_NONE, 0);

        /**
         * CdkPixbufLoader::area-updated:
         * @loader: the object which received the signal.
         * @x: X offset of upper-left corner of the updated area.
         * @y: Y offset of upper-left corner of the updated area.
         * @width: Width of updated area.
         * @height: Height of updated area.
         *
         * This signal is emitted when a significant area of the image being
         * loaded has been updated.
         *
         * Normally it means that a complete scanline has been read in, but
         * it could be a different area as well.
         *
         * Applications can use this signal to know when to repaint
         * areas of an image that is being loaded.
         */
        pixbuf_loader_signals[AREA_UPDATED] =
                g_signal_new ("area-updated",
                              G_TYPE_FROM_CLASS (object_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (CdkPixbufLoaderClass, area_updated),
                              NULL, NULL,
                              _cdk_pixbuf_marshal_VOID__INT_INT_INT_INT,
                              G_TYPE_NONE, 4,
                              G_TYPE_INT,
                              G_TYPE_INT,
                              G_TYPE_INT,
                              G_TYPE_INT);
  
        /**
         * CdkPixbufLoader::closed:
         * @loader: the object which received the signal.
         *
         * This signal is emitted when cdk_pixbuf_loader_close() is called.
         *
         * It can be used by different parts of an application to receive
         * notification when an image loader is closed by the code that
         * drives it.
         */
        pixbuf_loader_signals[CLOSED] =
                g_signal_new ("closed",
                              G_TYPE_FROM_CLASS (object_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (CdkPixbufLoaderClass, closed),
                              NULL, NULL,
                              _cdk_pixbuf_marshal_VOID__VOID,
                              G_TYPE_NONE, 0);
}

static void
cdk_pixbuf_loader_init (CdkPixbufLoader *loader)
{
        CdkPixbufLoaderPrivate *priv;
  
        priv = g_new0 (CdkPixbufLoaderPrivate, 1);
        priv->original_width = -1;
        priv->original_height = -1;
        priv->width = -1;
        priv->height = -1;

        loader->priv = priv;
}

static void
cdk_pixbuf_loader_finalize (GObject *object)
{
        CdkPixbufLoader *loader;
        CdkPixbufLoaderPrivate *priv = NULL;
  
        loader = CDK_PIXBUF_LOADER (object);
        priv = loader->priv;

        if (!priv->closed) {
                g_warning ("CdkPixbufLoader finalized without calling cdk_pixbuf_loader_close() - this is not allowed. You must explicitly end the data stream to the loader before dropping the last reference.");
        }
        if (priv->animation)
                g_object_unref (priv->animation);
  
	g_free (priv->filename);

        g_free (priv);
  
        G_OBJECT_CLASS (cdk_pixbuf_loader_parent_class)->finalize (object);
}

/**
 * cdk_pixbuf_loader_set_size:
 * @loader: A pixbuf loader.
 * @width: The desired width of the image being loaded.
 * @height: The desired height of the image being loaded.
 *
 * Causes the image to be scaled while it is loaded.
 *
 * The desired image size can be determined relative to the original
 * size of the image by calling cdk_pixbuf_loader_set_size() from a
 * signal handler for the ::size-prepared signal.
 *
 * Attempts to set the desired image size  are ignored after the 
 * emission of the ::size-prepared signal.
 *
 * Since: 2.2
 */
void 
cdk_pixbuf_loader_set_size (CdkPixbufLoader *loader,
			    gint             width,
			    gint             height)
{
        CdkPixbufLoaderPrivate *priv;

        g_return_if_fail (CDK_IS_PIXBUF_LOADER (loader));
        g_return_if_fail (width >= 0 && height >= 0);

        priv = CDK_PIXBUF_LOADER (loader)->priv;

        if (!priv->size_fixed) 
                {
                        priv->width = width;
                        priv->height = height;
                }
}

static void
cdk_pixbuf_loader_size_func (gint *width, gint *height, gpointer loader)
{
        CdkPixbufLoaderPrivate *priv = CDK_PIXBUF_LOADER (loader)->priv;

        priv->original_width = *width;
        priv->original_height = *height;

        /* allow calling cdk_pixbuf_loader_set_size() before the signal */
        if (priv->width == -1 && priv->height == -1) 
                {
                        priv->width = *width;
                        priv->height = *height;
                }

        g_signal_emit (loader, pixbuf_loader_signals[SIZE_PREPARED], 0, *width, *height);
        priv->size_fixed = TRUE;

        *width = priv->width;
        *height = priv->height;
}

static void
cdk_pixbuf_loader_prepare (CdkPixbuf          *pixbuf,
                           CdkPixbufAnimation *anim,
			   gpointer            loader)
{
        CdkPixbufLoaderPrivate *priv = CDK_PIXBUF_LOADER (loader)->priv;
        gint width, height;
        g_return_if_fail (pixbuf != NULL);

G_GNUC_BEGIN_IGNORE_DEPRECATIONS

        width = anim ? cdk_pixbuf_animation_get_width (anim) :
                cdk_pixbuf_get_width (pixbuf);
        height = anim ? cdk_pixbuf_animation_get_height (anim) :
                cdk_pixbuf_get_height (pixbuf);

G_GNUC_END_IGNORE_DEPRECATIONS

        if (!priv->size_fixed) 
                {
			gint w = width;
			gint h = height;
                        /* Defend against lazy loaders which don't call size_func */
                        cdk_pixbuf_loader_size_func (&w, &h, loader);
                }

        priv->needs_scale = FALSE;
        if (priv->width > 0 && priv->height > 0 &&
            (priv->width != width || priv->height != height))
                priv->needs_scale = TRUE;

        if (anim)
                g_object_ref (anim);
        else {
                if (priv->original_width > 0) {
                        char *original_width_str = NULL;

                        original_width_str = g_strdup_printf ("%d", priv->original_width);
                        cdk_pixbuf_set_option (pixbuf, "original-width", original_width_str);
                        g_free (original_width_str);
                }

                if (priv->original_height > 0) {
                        char *original_height_str = NULL;

                        original_height_str = g_strdup_printf ("%d", priv->original_height);
                        cdk_pixbuf_set_option (pixbuf, "original-height", original_height_str);
                        g_free (original_height_str);
                }

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
                anim = cdk_pixbuf_non_anim_new (pixbuf);
G_GNUC_END_IGNORE_DEPRECATIONS
        }

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	if (priv->needs_scale && width != 0 && height != 0) {
		priv->animation  = CDK_PIXBUF_ANIMATION (_cdk_pixbuf_scaled_anim_new (anim,
                                         (double) priv->width / width,
                                         (double) priv->height / height,
					  1.0));
			g_object_unref (anim);
	}
	else
        	priv->animation = anim;
G_GNUC_END_IGNORE_DEPRECATIONS
  
        if (!priv->needs_scale)
                g_signal_emit (loader, pixbuf_loader_signals[AREA_PREPARED], 0);
}

static void
cdk_pixbuf_loader_update (CdkPixbuf *pixbuf,
			  gint       x,
			  gint       y,
			  gint       width,
			  gint       height,
			  gpointer   loader)
{
        CdkPixbufLoaderPrivate *priv = CDK_PIXBUF_LOADER (loader)->priv;
  
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        if (!priv->needs_scale)
                g_signal_emit (loader,
                               pixbuf_loader_signals[AREA_UPDATED],
                               0,
                               x, y,
                               /* sanity check in here.  Defend against an errant loader */
                               MIN (width, cdk_pixbuf_animation_get_width (priv->animation)),
                               MIN (height, cdk_pixbuf_animation_get_height (priv->animation)));
G_GNUC_END_IGNORE_DEPRECATIONS
}

/* Defense against broken loaders; DO NOT take this as a GError example! */
static void
cdk_pixbuf_loader_ensure_error (CdkPixbufLoader *loader,
                                GError         **error)
{ 
        CdkPixbufLoaderPrivate *priv = loader->priv;

        if (error == NULL || *error != NULL)
                return;

        g_warning ("Bug! loader '%s' didn't set an error on failure",
                   priv->image_module->module_name);
        g_set_error (error,
                     CDK_PIXBUF_ERROR,
                     CDK_PIXBUF_ERROR_FAILED,
                     _("Internal error: Image loader module “%s” failed to"
                       " complete an operation, but didn’t give a reason for"
                       " the failure"),
                     priv->image_module->module_name);
}

static gint
cdk_pixbuf_loader_load_module (CdkPixbufLoader *loader,
                               const char      *image_type,
                               GError         **error)
{
        CdkPixbufLoaderPrivate *priv = loader->priv;

        if (image_type)
                {
                        priv->image_module = _cdk_pixbuf_get_named_module (image_type,
                                                                           error);
                }
        else
                {
                        priv->image_module = _cdk_pixbuf_get_module (priv->header_buf,
                                                                     priv->header_buf_offset,
                                                                     priv->filename,
                                                                     error);
                }
  
        if (priv->image_module == NULL)
                return 0;
  
        if (!_cdk_pixbuf_load_module (priv->image_module, error))
                return 0;
  
        if (priv->image_module->module == NULL)
                return 0;
  
        if ((priv->image_module->begin_load == NULL) ||
            (priv->image_module->stop_load == NULL) ||
            (priv->image_module->load_increment == NULL))
                {
                        g_set_error (error,
                                     CDK_PIXBUF_ERROR,
                                     CDK_PIXBUF_ERROR_UNSUPPORTED_OPERATION,
                                     _("Incremental loading of image type “%s” is not supported"),
                                     priv->image_module->module_name);

                        return 0;
                }

        priv->context = priv->image_module->begin_load (cdk_pixbuf_loader_size_func,
                                                        cdk_pixbuf_loader_prepare,
                                                        cdk_pixbuf_loader_update,
                                                        loader,
                                                        error);
  
        if (priv->context == NULL)
                {
                        cdk_pixbuf_loader_ensure_error (loader, error);
                        return 0;
                }
  
        if (priv->header_buf_offset
            && priv->image_module->load_increment (priv->context, priv->header_buf, priv->header_buf_offset, error))
                return priv->header_buf_offset;
  
        return 0;
}

static int
cdk_pixbuf_loader_eat_header_write (CdkPixbufLoader *loader,
				    const guchar    *buf,
				    gsize            count,
                                    GError         **error)
{
        gint n_bytes;
        CdkPixbufLoaderPrivate *priv = loader->priv;
  
        n_bytes = MIN(SNIFF_BUFFER_SIZE - priv->header_buf_offset, count);
        memcpy (priv->header_buf + priv->header_buf_offset, buf, n_bytes);
  
        priv->header_buf_offset += n_bytes;
  
        if (priv->header_buf_offset >= SNIFF_BUFFER_SIZE)
                {
                        if (cdk_pixbuf_loader_load_module (loader, NULL, error) == 0)
                                return 0;
                }
  
        return n_bytes;
}

/**
 * cdk_pixbuf_loader_write:
 * @loader: A pixbuf loader.
 * @buf: (array length=count): Pointer to image data.
 * @count: Length of the @buf buffer in bytes.
 * @error: return location for errors
 *
 * Parses the next `count` bytes in the given image buffer.
 *
 * Return value: `TRUE` if the write was successful, or
 *   `FALSE` if the loader cannot parse the buffer
 **/
gboolean
cdk_pixbuf_loader_write (CdkPixbufLoader *loader,
			 const guchar    *buf,
			 gsize            count,
                         GError         **error)
{
        CdkPixbufLoaderPrivate *priv;
  
        g_return_val_if_fail (CDK_IS_PIXBUF_LOADER (loader), FALSE);
  
        g_return_val_if_fail (buf != NULL, FALSE);
        g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  
        priv = loader->priv;

        /* we expect it's not to be closed */
        g_return_val_if_fail (priv->closed == FALSE, FALSE);
  
        if (count > 0 && priv->image_module == NULL)
                {
                        gint eaten;
      
                        eaten = cdk_pixbuf_loader_eat_header_write (loader, buf, count, error);
                        if (eaten <= 0)
                               goto fail; 
      
                        count -= eaten;
                        buf += eaten;
                }
  
        /* By this point, we expect the image_module to have been loaded. */
        g_assert (count == 0 || priv->image_module != NULL);

        if (count > 0 && priv->image_module->load_increment != NULL)
                {
                        if (!priv->image_module->load_increment (priv->context, buf, count,
                                                                 error))
				goto fail;
                }
      
        return TRUE;

 fail:
        cdk_pixbuf_loader_ensure_error (loader, error);
        cdk_pixbuf_loader_close (loader, NULL);

        return FALSE;
}

/**
 * cdk_pixbuf_loader_write_bytes:
 * @loader: A pixbuf loader.
 * @buffer: The image data as a `GBytes` buffer.
 * @error: return location for errors
 *
 * Parses the next contents of the given image buffer.
 *
 * Return value: `TRUE` if the write was successful, or `FALSE` if
 *   the loader cannot parse the buffer
 *
 * Since: 2.30
 */
gboolean
cdk_pixbuf_loader_write_bytes (CdkPixbufLoader *loader,
                               GBytes          *buffer,
                               GError         **error)
{
        g_return_val_if_fail (CDK_IS_PIXBUF_LOADER (loader), FALSE);

        g_return_val_if_fail (buffer != NULL, FALSE);
        g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

        return cdk_pixbuf_loader_write (loader,
                                        g_bytes_get_data (buffer, NULL),
                                        g_bytes_get_size (buffer),
                                        error);
}

/**
 * cdk_pixbuf_loader_new:
 *
 * Creates a new pixbuf loader object.
 *
 * Return value: A newly-created pixbuf loader.
 **/
CdkPixbufLoader *
cdk_pixbuf_loader_new (void)
{
        return g_object_new (CDK_TYPE_PIXBUF_LOADER, NULL);
}

/**
 * cdk_pixbuf_loader_new_with_type:
 * @image_type: name of the image format to be loaded with the image
 * @error: (allow-none): return location for an allocated #GError, or `NULL` to ignore errors
 *
 * Creates a new pixbuf loader object that always attempts to parse
 * image data as if it were an image of type @image_type, instead of
 * identifying the type automatically.
 *
 * This function is useful if you want an error if the image isn't the
 * expected type; for loading image formats that can't be reliably
 * identified by looking at the data; or if the user manually forces
 * a specific type.
 *
 * The list of supported image formats depends on what image loaders
 * are installed, but typically "png", "jpeg", "gif", "tiff" and 
 * "xpm" are among the supported formats. To obtain the full list of
 * supported image formats, call cdk_pixbuf_format_get_name() on each 
 * of the #CdkPixbufFormat structs returned by cdk_pixbuf_get_formats().
 *
 * Return value: A newly-created pixbuf loader.
 **/
CdkPixbufLoader *
cdk_pixbuf_loader_new_with_type (const char *image_type,
                                 GError    **error)
{
        CdkPixbufLoader *retval;
        GError *tmp;
        g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  
        retval = g_object_new (CDK_TYPE_PIXBUF_LOADER, NULL);

        tmp = NULL;
        cdk_pixbuf_loader_load_module (retval, image_type, &tmp);
        if (tmp != NULL)
                {
                        g_propagate_error (error, tmp);
                        cdk_pixbuf_loader_close (retval, NULL);
                        g_object_unref (retval);
                        return NULL;
                }

        return retval;
}

/**
 * cdk_pixbuf_loader_new_with_mime_type:
 * @mime_type: the mime type to be loaded 
 * @error: (allow-none): return location for an allocated #GError, or `NULL` to ignore errors
 *
 * Creates a new pixbuf loader object that always attempts to parse
 * image data as if it were an image of MIME type @mime_type, instead of
 * identifying the type automatically.
 *
 * This function is useful if you want an error if the image isn't the
 * expected MIME type; for loading image formats that can't be reliably
 * identified by looking at the data; or if the user manually forces a
 * specific MIME type.
 *
 * The list of supported mime types depends on what image loaders
 * are installed, but typically "image/png", "image/jpeg", "image/gif", 
 * "image/tiff" and "image/x-xpixmap" are among the supported mime types. 
 * To obtain the full list of supported mime types, call 
 * cdk_pixbuf_format_get_mime_types() on each of the #CdkPixbufFormat 
 * structs returned by cdk_pixbuf_get_formats().
 *
 * Return value: A newly-created pixbuf loader.
 *
 * Since: 2.4
 **/
CdkPixbufLoader *
cdk_pixbuf_loader_new_with_mime_type (const char *mime_type,
                                      GError    **error)
{
        const char * image_type = NULL;
        char ** mimes;

        CdkPixbufLoader *retval;
        GError *tmp;
  
        GSList * formats;
        CdkPixbufFormat *info;
        int i, j, length;

        formats = cdk_pixbuf_get_formats ();
        length = g_slist_length (formats);

        for (i = 0; i < length && image_type == NULL; i++) {
                info = (CdkPixbufFormat *)g_slist_nth_data (formats, i);
                mimes = info->mime_types;
                
                for (j = 0; mimes[j] != NULL; j++)
                        if (g_ascii_strcasecmp (mimes[j], mime_type) == 0) {
                                image_type = info->name;
                                break;
                        }
        }

        g_slist_free (formats);

        retval = g_object_new (CDK_TYPE_PIXBUF_LOADER, NULL);

        tmp = NULL;
        cdk_pixbuf_loader_load_module (retval, image_type, &tmp);
        if (tmp != NULL)
                {
                        g_propagate_error (error, tmp);
                        cdk_pixbuf_loader_close (retval, NULL);
                        g_object_unref (retval);
                        return NULL;
                }

        return retval;
}

CdkPixbufLoader *
_cdk_pixbuf_loader_new_with_filename (const char *filename)
{
	CdkPixbufLoader *retval;
        CdkPixbufLoaderPrivate *priv;

        retval = g_object_new (CDK_TYPE_PIXBUF_LOADER, NULL);
	priv = retval->priv;
	priv->filename = g_strdup (filename);

	return retval;
}

/**
 * cdk_pixbuf_loader_get_pixbuf:
 * @loader: A pixbuf loader.
 *
 * Queries the #CdkPixbuf that a pixbuf loader is currently creating.
 *
 * In general it only makes sense to call this function after the
 * [signal@CdkPixbuf.PixbufLoader::area-prepared] signal has been
 * emitted by the loader; this means that enough data has been read
 * to know the size of the image that will be allocated.
 *
 * If the loader has not received enough data via cdk_pixbuf_loader_write(),
 * then this function returns `NULL`.
 *
 * The returned pixbuf will be the same in all future calls to the loader,
 * so if you want to keep using it, you should acquire a reference to it.
 *
 * Additionally, if the loader is an animation, it will return the "static
 * image" of the animation (see cdk_pixbuf_animation_get_static_image()).
 * 
 * Return value: (transfer none) (nullable): The pixbuf that the loader is
 *   creating
 **/
CdkPixbuf *
cdk_pixbuf_loader_get_pixbuf (CdkPixbufLoader *loader)
{
        CdkPixbufLoaderPrivate *priv;
  
        g_return_val_if_fail (CDK_IS_PIXBUF_LOADER (loader), NULL);
  
        priv = loader->priv;

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        if (priv->animation)
                return cdk_pixbuf_animation_get_static_image (priv->animation);
        else
                return NULL;
G_GNUC_END_IGNORE_DEPRECATIONS
}

/**
 * cdk_pixbuf_loader_get_animation:
 * @loader: A pixbuf loader
 *
 * Queries the #CdkPixbufAnimation that a pixbuf loader is currently creating.
 *
 * In general it only makes sense to call this function after the
 * [signal@CdkPixbuf.PixbufLoader::area-prepared] signal has been emitted by
 * the loader.
 *
 * If the loader doesn't have enough bytes yet, and hasn't emitted the `area-prepared`
 * signal, this function will return `NULL`.
 *
 * Return value: (transfer none) (nullable): The animation that the loader is
 *   currently loading
 */
CdkPixbufAnimation *
cdk_pixbuf_loader_get_animation (CdkPixbufLoader *loader)
{
        CdkPixbufLoaderPrivate *priv;
  
        g_return_val_if_fail (CDK_IS_PIXBUF_LOADER (loader), NULL);
  
        priv = loader->priv;
  
        return priv->animation;
}

/**
 * cdk_pixbuf_loader_close:
 * @loader: A pixbuf loader.
 * @error: (allow-none): return location for a #GError, or `NULL` to ignore errors
 *
 * Informs a pixbuf loader that no further writes with
 * cdk_pixbuf_loader_write() will occur, so that it can free its
 * internal loading structures.
 *
 * This function also tries to parse any data that hasn't yet been parsed;
 * if the remaining data is partial or corrupt, an error will be returned.
 *
 * If `FALSE` is returned, `error` will be set to an error from the
 * `CDK_PIXBUF_ERROR` or `G_FILE_ERROR` domains.
 *
 * If you're just cancelling a load rather than expecting it to be finished,
 * passing `NULL` for `error` to ignore it is reasonable.
 *
 * Remember that this function does not release a reference on the loader, so
 * you will need to explicitly release any reference you hold.
 *
 * Returns: `TRUE` if all image data written so far was successfully
 *   passed out via the update_area signal
 */
gboolean
cdk_pixbuf_loader_close (CdkPixbufLoader *loader,
                         GError         **error)
{
        CdkPixbufLoaderPrivate *priv;
        gboolean retval = TRUE;
  
        g_return_val_if_fail (CDK_IS_PIXBUF_LOADER (loader), TRUE);
        g_return_val_if_fail (error == NULL || *error == NULL, TRUE);
  
        priv = loader->priv;
  
        if (priv->closed)
                return TRUE;
  
        /* We have less than SNIFF_BUFFER_SIZE bytes in the image.  
         * Flush it, and keep going. 
         */
        if (priv->image_module == NULL)
                {
                        GError *tmp = NULL;
                        cdk_pixbuf_loader_load_module (loader, NULL, &tmp);
                        if (tmp != NULL)
                                {
                                        g_propagate_error (error, tmp);
                                        retval = FALSE;
                                }
                }  

        if (priv->image_module && priv->image_module->stop_load && priv->context) 
                {
                        GError *tmp = NULL;
                        if (!priv->image_module->stop_load (priv->context, &tmp) || tmp)
                                {
					/* don't call cdk_pixbuf_loader_ensure_error()
 					 * here, since we might not get an error in the
 					 * cdk_pixbuf_get_file_info() case
 					 */
					if (tmp) {
						if (error && *error == NULL)
							g_propagate_error (error, tmp);
						else
							g_error_free (tmp);
					}
                                        retval = FALSE;
                                }
                }
  
        priv->closed = TRUE;

        if (priv->needs_scale) 
                {

                        g_signal_emit (loader, pixbuf_loader_signals[AREA_PREPARED], 0);
                        g_signal_emit (loader, pixbuf_loader_signals[AREA_UPDATED], 0, 
                                       0, 0, priv->width, priv->height);
                }

        
        g_signal_emit (loader, pixbuf_loader_signals[CLOSED], 0);

        return retval;
}

/**
 * cdk_pixbuf_loader_get_format:
 * @loader: A pixbuf loader.
 *
 * Obtains the available information about the format of the 
 * currently loading image file.
 *
 * Returns: (nullable) (transfer none): A #CdkPixbufFormat
 * 
 * Since: 2.2
 */
CdkPixbufFormat *
cdk_pixbuf_loader_get_format (CdkPixbufLoader *loader)
{
        CdkPixbufLoaderPrivate *priv;
  
        g_return_val_if_fail (CDK_IS_PIXBUF_LOADER (loader), NULL);
  
        priv = loader->priv;

        if (priv->image_module)
                return _cdk_pixbuf_get_format (priv->image_module);
        else
                return NULL;
}
