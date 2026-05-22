/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* CdkPixbuf library - GDIP loader declarations
 *
 * Copyright (C) 1999 The Free Software Foundation
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

#ifndef CDK_PIXBUF_GDIP_H
#define CDK_PIXBUF_GDIP_H

#include <cdk-pixbuf/cdk-pixbuf-animation.h>

typedef struct _CdkPixbufGdipAnim CdkPixbufGdipAnim;
typedef struct _CdkPixbufGdipAnimClass CdkPixbufGdipAnimClass;
typedef struct _CdkPixbufFrame CdkPixbufFrame;

#define CDK_TYPE_PIXBUF_GDIP_ANIM              (cdk_pixbuf_gdip_anim_get_type ())
#define CDK_PIXBUF_GDIP_ANIM(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), CDK_TYPE_PIXBUF_GDIP_ANIM, CdkPixbufGdipAnim))
#define CDK_IS_PIXBUF_GDIP_ANIM(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), CDK_TYPE_PIXBUF_GDIP_ANIM))

#define CDK_PIXBUF_GDIP_ANIM_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), CDK_TYPE_PIXBUF_GDIP_ANIM, CdkPixbufGdipAnimClass))
#define CDK_IS_PIXBUF_GDIP_ANIM_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CDK_TYPE_PIXBUF_GDIP_ANIM))
#define CDK_PIXBUF_GDIP_ANIM_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), CDK_TYPE_PIXBUF_GDIP_ANIM, CdkPixbufGdipAnimClass))

/* Private part of the CdkPixbufGdipAnim structure */
struct _CdkPixbufGdipAnim {
        CdkPixbufAnimation parent_instance;

        /* Number of frames */
        int n_frames;

        /* Total length of animation */
        int total_time;
        
	/* List of CdkPixbufFrame structures */
        GList *frames;

	/* bounding box size */
	int width, height;
        
        int loop;
        gboolean loading;
};

struct _CdkPixbufGdipAnimClass {
        CdkPixbufAnimationClass parent_class;
        
};

GType cdk_pixbuf_gdip_anim_get_type (void) G_GNUC_CONST;

typedef struct _CdkPixbufGdipAnimIter CdkPixbufGdipAnimIter;
typedef struct _CdkPixbufGdipAnimIterClass CdkPixbufGdipAnimIterClass;


#define CDK_TYPE_PIXBUF_GDIP_ANIM_ITER              (cdk_pixbuf_gdip_anim_iter_get_type ())
#define CDK_PIXBUF_GDIP_ANIM_ITER(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), CDK_TYPE_PIXBUF_GDIP_ANIM_ITER, CdkPixbufGdipAnimIter))
#define CDK_IS_PIXBUF_GDIP_ANIM_ITER(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), CDK_TYPE_PIXBUF_GDIP_ANIM_ITER))

#define CDK_PIXBUF_GDIP_ANIM_ITER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), CDK_TYPE_PIXBUF_GDIP_ANIM_ITER, CdkPixbufGdipAnimIterClass))
#define CDK_IS_PIXBUF_GDIP_ANIM_ITER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CDK_TYPE_PIXBUF_GDIP_ANIM_ITER))
#define CDK_PIXBUF_GDIP_ANIM_ITER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), CDK_TYPE_PIXBUF_GDIP_ANIM_ITER, CdkPixbufGdipAnimIterClass))

struct _CdkPixbufGdipAnimIter {
        CdkPixbufAnimationIter parent_instance;
        
        CdkPixbufGdipAnim   *gdip_anim;

        GTimeVal            start_time;
        GTimeVal            current_time;

        /* Time in milliseconds into this run of the animation */
        gint                position;
        
        GList              *current_frame;
        
        gint                first_loop_slowness;
};

struct _CdkPixbufGdipAnimIterClass {
        CdkPixbufAnimationIterClass parent_class;

};

GType cdk_pixbuf_gdip_anim_iter_get_type (void) G_GNUC_CONST;

struct _CdkPixbufFrame {
	/* The pixbuf with this frame's image data */
	CdkPixbuf *pixbuf;

	/* Frame duration in ms */
	int delay_time;

        /* Sum of preceding delay times */
        int elapsed;        
};

#endif
