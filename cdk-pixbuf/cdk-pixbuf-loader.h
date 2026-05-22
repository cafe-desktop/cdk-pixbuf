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

#ifndef CDK_PIXBUF_LOADER_H
#define CDK_PIXBUF_LOADER_H

#if defined(CDK_PIXBUF_DISABLE_SINGLE_INCLUDES) && !defined (CDK_PIXBUF_H_INSIDE) && !defined (CDK_PIXBUF_COMPILATION)
#error "Only <cdk-pixbuf/cdk-pixbuf.h> can be included directly."
#endif

#include <glib.h>
#include <glib-object.h>
#include <cdk-pixbuf/cdk-pixbuf-core.h>
#include <cdk-pixbuf/cdk-pixbuf-animation.h>
#include <cdk-pixbuf/cdk-pixbuf-io.h>

G_BEGIN_DECLS

#define CDK_TYPE_PIXBUF_LOADER		   (cdk_pixbuf_loader_get_type ())
#define CDK_PIXBUF_LOADER(obj)		   (G_TYPE_CHECK_INSTANCE_CAST ((obj), CDK_TYPE_PIXBUF_LOADER, CdkPixbufLoader))
#define CDK_PIXBUF_LOADER_CLASS(klass)	   (G_TYPE_CHECK_CLASS_CAST ((klass), CDK_TYPE_PIXBUF_LOADER, CdkPixbufLoaderClass))
#define CDK_IS_PIXBUF_LOADER(obj)	   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CDK_TYPE_PIXBUF_LOADER))
#define CDK_IS_PIXBUF_LOADER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CDK_TYPE_PIXBUF_LOADER))
#define CDK_PIXBUF_LOADER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CDK_TYPE_PIXBUF_LOADER, CdkPixbufLoaderClass))

typedef struct _CdkPixbufLoader CdkPixbufLoader;
struct _CdkPixbufLoader
{
  /*< private >*/
  GObject parent_instance;
  
  gpointer priv;
};

typedef struct _CdkPixbufLoaderClass CdkPixbufLoaderClass;
struct _CdkPixbufLoaderClass
{
  GObjectClass parent_class;

  void (*size_prepared)      (CdkPixbufLoader *loader, 
			      int              width,
			      int              height);

  void (*area_prepared)      (CdkPixbufLoader *loader);

  /* Last known frame needs a redraw for x, y, width, height */
  void (*area_updated)       (CdkPixbufLoader *loader,
                              int              x,
                              int              y,
			      int              width,
			      int              height);

  void (*closed)             (CdkPixbufLoader *loader);
};

CDK_PIXBUF_AVAILABLE_IN_ALL
GType                cdk_pixbuf_loader_get_type      (void) G_GNUC_CONST;
CDK_PIXBUF_AVAILABLE_IN_ALL
CdkPixbufLoader *    cdk_pixbuf_loader_new           (void);
CDK_PIXBUF_AVAILABLE_IN_ALL
CdkPixbufLoader *    cdk_pixbuf_loader_new_with_type (const char *image_type,
                                                      GError    **error);
CDK_PIXBUF_AVAILABLE_IN_2_4
CdkPixbufLoader *    cdk_pixbuf_loader_new_with_mime_type (const char *mime_type,
							   GError    **error);
CDK_PIXBUF_AVAILABLE_IN_2_2
void                 cdk_pixbuf_loader_set_size (CdkPixbufLoader  *loader,
                                                 int               width,
						 int               height);
CDK_PIXBUF_AVAILABLE_IN_ALL
gboolean             cdk_pixbuf_loader_write         (CdkPixbufLoader *loader,
						      const guchar    *buf,
						      gsize            count,
                                                      GError         **error);
CDK_PIXBUF_AVAILABLE_IN_2_30
gboolean             cdk_pixbuf_loader_write_bytes   (CdkPixbufLoader *loader,
                                                      GBytes          *buffer,
                                                      GError         **error);
CDK_PIXBUF_AVAILABLE_IN_ALL
CdkPixbuf *          cdk_pixbuf_loader_get_pixbuf    (CdkPixbufLoader *loader);
CDK_PIXBUF_DEPRECATED_IN_2_44
CdkPixbufAnimation * cdk_pixbuf_loader_get_animation (CdkPixbufLoader *loader);
CDK_PIXBUF_AVAILABLE_IN_ALL
gboolean             cdk_pixbuf_loader_close         (CdkPixbufLoader *loader,
                                                      GError         **error);
CDK_PIXBUF_AVAILABLE_IN_2_2
CdkPixbufFormat     *cdk_pixbuf_loader_get_format    (CdkPixbufLoader *loader);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(CdkPixbufLoader, g_object_unref)

G_END_DECLS

#endif
