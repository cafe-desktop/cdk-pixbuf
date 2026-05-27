/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* CdkPixbuf library - SVG image loader
 *
 * Copyright (C) 2002 Matthias Clasen
 * Copyright (C) 2002-2004 Dom Lachowicz
 *
 * Authors: Matthias Clasen <maclas@gmx.de>
 *          Dom Lachowicz <cinamod@hotmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define RSVG_DISABLE_DEPRECATION_WARNINGS

#include <stdlib.h>

#include <librsvg/rsvg.h>
#include <cdk-pixbuf/cdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct {
        RsvgHandle                 *handle;

        CdkPixbufModuleUpdatedFunc  updated_func;
        CdkPixbufModulePreparedFunc prepared_func;
        CdkPixbufModuleSizeFunc     size_func;

        gboolean                    first_write;

        gpointer                    user_data;
} SvgContext;

G_MODULE_EXPORT void fill_vtable (CdkPixbufModule *module);
G_MODULE_EXPORT void fill_info (CdkPixbufFormat *info);

enum {
        ERROR_WRITING = 1,
        ERROR_DISPLAYING_IMAGE
} RsvgLoaderErrorReasons;

static void
rsvg_propagate_error (GError ** err,
                      const char * reason,
                      gint code)
{
        if (err) {
                *err = NULL;
                g_set_error (err, rsvg_error_quark (), code, "%s", reason);
        }
}

static gpointer
cdk_pixbuf__svg_image_begin_load (CdkPixbufModuleSizeFunc size_func,
                                  CdkPixbufModulePreparedFunc prepared_func,
                                  CdkPixbufModuleUpdatedFunc  updated_func,
                                  gpointer user_data,
                                  GError **error)
{
        SvgContext *context    = g_new0 (SvgContext, 1);

        if (error)
                *error = NULL;

        context->first_write   = TRUE;
        context->size_func     = size_func;

        context->prepared_func = prepared_func;
        context->updated_func  = updated_func;
        context->user_data     = user_data;

        return context;
}

static void
emit_updated (SvgContext *context, CdkPixbuf *pixbuf)
{
        if (context->updated_func != NULL)
                (* context->updated_func) (pixbuf,
                                           0, 0,
                                           cdk_pixbuf_get_width (pixbuf),
                                           cdk_pixbuf_get_height (pixbuf),
                                           context->user_data);
}

static void
emit_prepared (SvgContext *context, CdkPixbuf *pixbuf)
{
        if (context->prepared_func != NULL)
                (* context->prepared_func) (pixbuf, NULL, context->user_data);
}

static gboolean
cdk_pixbuf__svg_image_load_increment (gpointer data,
				      const guchar *buf, guint size,
				      GError **error)
{
        SvgContext *context = (SvgContext *)data;

        if (error)
                *error = NULL;

        if (context->first_write == TRUE) {
                context->first_write = FALSE;

                context->handle = rsvg_handle_new ();

                if (!context->handle) {
                        rsvg_propagate_error (error, "Error displaying image", ERROR_DISPLAYING_IMAGE);
                        return FALSE;
                }

                rsvg_handle_set_size_callback (context->handle, context->size_func, context->user_data, NULL);
        }

        if (!context->handle) {
                rsvg_propagate_error (error, "Error displaying image", ERROR_DISPLAYING_IMAGE);
                return FALSE;
        }

        if (!rsvg_handle_write (context->handle, buf, size, error)) {
                g_clear_error(error);
                rsvg_propagate_error (error, "Error writing", ERROR_WRITING);
                return FALSE;
        }

        return TRUE;
}

static gboolean
cdk_pixbuf__svg_image_stop_load (gpointer data, GError **error)
{
        SvgContext *context = (SvgContext *)data;
        CdkPixbuf *cdk_pixbuf;
        GdkPixbuf *gdk_pixbuf;
        gboolean result = TRUE;

        if (error)
                *error = NULL;

        if (!context->handle) {
                rsvg_propagate_error (error, "Error displaying image", ERROR_DISPLAYING_IMAGE);
                return FALSE;
        }

        if (!rsvg_handle_close (context->handle, error)) {
                g_object_unref (context->handle);
                g_free (context);
                return FALSE;
        }

        gdk_pixbuf = rsvg_handle_get_pixbuf (context->handle);

        if (gdk_pixbuf != NULL) {
                int width, height, rowstride, bytes_per_row;
                guchar *src_pixels, *dest_pixels;

                width      = gdk_pixbuf_get_width (gdk_pixbuf);
                height     = gdk_pixbuf_get_height (gdk_pixbuf);
                rowstride  = gdk_pixbuf_get_rowstride (gdk_pixbuf);
                src_pixels = gdk_pixbuf_get_pixels (gdk_pixbuf);

                cdk_pixbuf = cdk_pixbuf_new (CDK_COLORSPACE_RGB,
                                             gdk_pixbuf_get_has_alpha (gdk_pixbuf),
                                             gdk_pixbuf_get_bits_per_sample (gdk_pixbuf),
                                             width, height);

                if (cdk_pixbuf != NULL) {
                        int dest_rowstride = cdk_pixbuf_get_rowstride (cdk_pixbuf);
                        bytes_per_row = width * gdk_pixbuf_get_n_channels (gdk_pixbuf);
                        dest_pixels = cdk_pixbuf_get_pixels (cdk_pixbuf);

                        for (int y = 0; y < height; y++) {
                                memcpy (dest_pixels + y * dest_rowstride,
                                        src_pixels + y * rowstride,
                                        bytes_per_row);
                        }

                        emit_prepared (context, cdk_pixbuf);
                        emit_updated (context, cdk_pixbuf);
                        g_object_unref (cdk_pixbuf);
                } else {
                        rsvg_propagate_error (error, "Error displaying image", ERROR_DISPLAYING_IMAGE);
                        result = FALSE;
                }

                g_object_unref (gdk_pixbuf);
        }
        else {
                rsvg_propagate_error (error, "Error displaying image", ERROR_DISPLAYING_IMAGE);
                result = FALSE;
        }

        g_object_unref (context->handle);
        g_free (context);

        return result;
}

void
fill_vtable (CdkPixbufModule *module)
{
        module->begin_load     = cdk_pixbuf__svg_image_begin_load;
        module->stop_load      = cdk_pixbuf__svg_image_stop_load;
        module->load_increment = cdk_pixbuf__svg_image_load_increment;
}

void
fill_info (CdkPixbufFormat *info)
{
        static const CdkPixbufModulePattern signature[] = {
                {  " <svg",  "*    ", 100 },
                {  " <!DOCTYPE svg",  "*             ", 100 },
                { NULL, NULL, 0 }
        };

        static const gchar *mime_types[] = { /* yes folks, i actually have run into all of these in the wild... */
                "image/svg+xml",
                "image/svg",
                "image/svg-xml",
                "image/vnd.adobe.svg+xml",
                "text/xml-svg",
                "image/svg+xml-compressed",
                NULL
        };
        static const gchar *extensions[] = {
                "svg",
                "svgz",
                "svg.gz",
                NULL
        };

        info->name        = "svg";
        info->signature   = (CdkPixbufModulePattern *) signature;
        info->description = "Scalable Vector Graphics";
        info->mime_types  = (gchar **) mime_types;
        info->extensions  = (gchar **) extensions;
        info->flags       = CDK_PIXBUF_FORMAT_SCALABLE | CDK_PIXBUF_FORMAT_THREADSAFE;
        info->license     = "LGPL";
}
