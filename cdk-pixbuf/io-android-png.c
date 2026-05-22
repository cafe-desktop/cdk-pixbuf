/* CdkPixbuf library - Android image loader
 *
 * Copyright (C) 2025  Florian Leander Singer <sp1rit@disroot.org>
 *
 * Authors: Florian Leander Singer <sp1rit@disroot.org>
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

#include "io-android-utils.h"


#define MODULE_ENTRY(function) void _cdk_pixbuf__android_png_ ## function

static gboolean
cdk_pixbuf__android_save_image_png (CdkPixbufSaveFunc save_func, gpointer user_data,
                                    CdkPixbuf *pixbuf,
                                    gchar **option_keys, gchar **option_values,
                                    GError **error)
{
  return cdk_pixbuf__android_save_image (save_func, user_data,
                                         pixbuf, ANDROID_BITMAP_COMPRESS_FORMAT_PNG,
                                         error);
}

MODULE_ENTRY (fill_vtable) (CdkPixbufModule *module)
{
  cdk_pixbuf__android_fill_vtable (module);

  module->save_to_callback = cdk_pixbuf__android_save_image_png;
}

MODULE_ENTRY (fill_info) (CdkPixbufFormat *info)
{
  static const CdkPixbufModulePattern signature[] = {
    { "\x89PNG\r\n\x1a\x0a", NULL, 100 },
    { NULL, NULL, 0 }
  };
  static const gchar *mime_types[] = {
    "image/png",
    NULL
  };
  static const gchar *extensions[] = {
    "png",
    NULL
  };

  info->name = "png";
  info->signature = (CdkPixbufModulePattern *) signature;
  info->description = "PNG";
  info->mime_types = (gchar **) mime_types;
  info->extensions = (gchar **) extensions;
  info->flags = CDK_PIXBUF_FORMAT_WRITABLE | CDK_PIXBUF_FORMAT_THREADSAFE;
  info->license = "Apache-2.0";
}
