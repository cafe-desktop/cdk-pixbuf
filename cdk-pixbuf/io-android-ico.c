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


#define MODULE_ENTRY(function) void _cdk_pixbuf__android_ico_ ## function

MODULE_ENTRY (fill_vtable) (CdkPixbufModule *module)
{
  cdk_pixbuf__android_fill_vtable (module);
}

MODULE_ENTRY (fill_info) (CdkPixbufFormat *info)
{
  static const CdkPixbufModulePattern signature[] = {
    { "  \x1   ", "zz znz", 100 },
    { "  \x2   ", "zz znz", 100 },
    { NULL, NULL, 0 }
  };
  static const gchar *mime_types[] = {
    "image/x-icon",
    "image/x-ico",
    "image/x-win-bitmap",
    "image/vnd.microsoft.icon",
    "application/ico",
    "image/ico",
    "image/icon",
    "text/ico",
    NULL
  };
  static const gchar *extensions[] = {
    "ico",
    "cur",
    NULL
  };

  info->name = "ico";
  info->signature = (CdkPixbufModulePattern *) signature;
  info->description = "ICO";
  info->mime_types = (gchar **) mime_types;
  info->extensions = (gchar **) extensions;
  info->flags = CDK_PIXBUF_FORMAT_THREADSAFE;
  info->license = "Apache-2.0";
}
