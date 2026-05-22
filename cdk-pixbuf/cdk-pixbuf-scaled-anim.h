/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* CdkPixbuf library - Simple transformations of animations
 *
 * Copyright (C) 2007 Red Hat, Inc
 *
 * Authors: Matthias Clasen <mclasen@redhat.com>
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

#ifndef CDK_PIXBUF_SCALED_ANIM_H
#define CDK_PIXBUF_SCALED_ANIM_H

#include <cdk-pixbuf/cdk-pixbuf-animation.h>

G_BEGIN_DECLS

#define CDK_TYPE_PIXBUF_SCALED_ANIM              (cdk_pixbuf_scaled_anim_get_type ())
#define CDK_TYPE_PIXBUF_SCALED_ANIM_ITER         (cdk_pixbuf_scaled_anim_iter_get_type ())

typedef struct _CdkPixbufScaledAnim CdkPixbufScaledAnim;
typedef struct _CdkPixbufScaledAnimClass CdkPixbufScaledAnimClass;

GType cdk_pixbuf_scaled_anim_get_type (void) G_GNUC_CONST;
GType cdk_pixbuf_scaled_anim_iter_get_type (void) G_GNUC_CONST;

CdkPixbufScaledAnim *_cdk_pixbuf_scaled_anim_new (CdkPixbufAnimation *anim,
                                                  gdouble             xscale, 
                                                  gdouble             yscale,
                                                  gdouble             tscale);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (CdkPixbufScaledAnim, g_object_unref)

G_END_DECLS

#endif  /* CDK_PIXBUF_SCALED_ANIM_H */
