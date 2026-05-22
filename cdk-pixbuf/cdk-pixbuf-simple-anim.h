/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* CdkPixbuf library - Simple frame-based animations
 *
 * Copyright (C) 2004 Dom Lachowicz
 *
 * Authors: Dom Lachowicz <cinamod@hotmail.com>
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

#ifndef CDK_PIXBUF_SIMPLE_ANIM_H
#define CDK_PIXBUF_SIMPLE_ANIM_H

#if defined(CDK_PIXBUF_DISABLE_SINGLE_INCLUDES) && !defined (CDK_PIXBUF_H_INSIDE) && !defined (CDK_PIXBUF_COMPILATION)
#error "Only <cdk-pixbuf/cdk-pixbuf.h> can be included directly."
#endif

#include <cdk-pixbuf/cdk-pixbuf-animation.h>

G_BEGIN_DECLS

/**
 * CdkPixbufSimpleAnim:
 * 
 * An opaque struct representing a simple animation.
 */
typedef struct _CdkPixbufSimpleAnim CdkPixbufSimpleAnim;
typedef struct _CdkPixbufSimpleAnimClass CdkPixbufSimpleAnimClass;

#define CDK_TYPE_PIXBUF_SIMPLE_ANIM              (cdk_pixbuf_simple_anim_get_type ())
#define CDK_PIXBUF_SIMPLE_ANIM(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), CDK_TYPE_PIXBUF_SIMPLE_ANIM, CdkPixbufSimpleAnim))
#define CDK_IS_PIXBUF_SIMPLE_ANIM(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), CDK_TYPE_PIXBUF_SIMPLE_ANIM))

#define CDK_PIXBUF_SIMPLE_ANIM_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), CDK_TYPE_PIXBUF_SIMPLE_ANIM, CdkPixbufSimpleAnimClass))
#define CDK_IS_PIXBUF_SIMPLE_ANIM_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CDK_TYPE_PIXBUF_SIMPLE_ANIM))
#define CDK_PIXBUF_SIMPLE_ANIM_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), CDK_TYPE_PIXBUF_SIMPLE_ANIM, CdkPixbufSimpleAnimClass))

CDK_PIXBUF_AVAILABLE_IN_ALL
GType cdk_pixbuf_simple_anim_get_type (void) G_GNUC_CONST;
CDK_PIXBUF_AVAILABLE_IN_ALL
GType cdk_pixbuf_simple_anim_iter_get_type (void) G_GNUC_CONST;

CDK_PIXBUF_DEPRECATED_IN_2_44
CdkPixbufSimpleAnim *cdk_pixbuf_simple_anim_new           (gint   width, 
                                                           gint   height,
                                                           gfloat rate);

CDK_PIXBUF_DEPRECATED_IN_2_44
void                 cdk_pixbuf_simple_anim_add_frame     (CdkPixbufSimpleAnim *animation,
                                                           CdkPixbuf           *pixbuf);
CDK_PIXBUF_DEPRECATED_IN_2_44
void                 cdk_pixbuf_simple_anim_set_loop      (CdkPixbufSimpleAnim *animation,
                                                           gboolean             loop);
CDK_PIXBUF_DEPRECATED_IN_2_44
gboolean             cdk_pixbuf_simple_anim_get_loop      (CdkPixbufSimpleAnim *animation);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (CdkPixbufSimpleAnim, g_object_unref)

G_END_DECLS

#endif  /* CDK_PIXBUF_SIMPLE_ANIM_H */
