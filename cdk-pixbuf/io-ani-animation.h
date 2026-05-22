/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* CdkPixbuf library - ANI loader declarations
 *
 * Copyright (C) 2002 The Free Software Foundation
 *
 * Author: Matthias Clasen <maclas@gmx.de>
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

#ifndef CDK_PIXBUF_ANI_ANIMATION_H
#define CDK_PIXBUF_ANI_ANIMATION_H

#include <glib/gi18n-lib.h>
#include "cdk-pixbuf-animation.h"

typedef struct _CdkPixbufAniAnim CdkPixbufAniAnim;
typedef struct _CdkPixbufAniAnimClass CdkPixbufAniAnimClass;

#define CDK_TYPE_PIXBUF_ANI_ANIM              (cdk_pixbuf_ani_anim_get_type ())
#define CDK_PIXBUF_ANI_ANIM(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), CDK_TYPE_PIXBUF_ANI_ANIM, CdkPixbufAniAnim))
#define CDK_IS_PIXBUF_ANI_ANIM(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), CDK_TYPE_PIXBUF_ANI_ANIM))

#define CDK_PIXBUF_ANI_ANIM_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), CDK_TYPE_PIXBUF_ANI_ANIM, CdkPixbufAniAnimClass))
#define CDK_IS_PIXBUF_ANI_ANIM_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CDK_TYPE_PIXBUF_ANI_ANIM))
#define CDK_PIXBUF_ANI_ANIM_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), CDK_TYPE_PIXBUF_ANI_ANIM, CdkPixbufAniAnimClass))

/* Private part of the CdkPixbufAniAnim structure */
struct _CdkPixbufAniAnim {
        CdkPixbufAnimation parent_instance;

        /* Total length of animation */
        int total_time;
        
        /* Number of frames */
        int n_frames;
        
        /* Number of pixbufs */
        int n_pixbufs;
        
        CdkPixbuf **pixbufs;
        
        /* Maps frame number to pixbuf */
        int *sequence;
        
        /* The duration of each frame, in milliseconds */
	int *delay;
        
        /* bounding box size */
	int width, height;
};

struct _CdkPixbufAniAnimClass {
        CdkPixbufAnimationClass parent_class;
        
};

GType cdk_pixbuf_ani_anim_get_type (void) G_GNUC_CONST;



typedef struct _CdkPixbufAniAnimIter CdkPixbufAniAnimIter;
typedef struct _CdkPixbufAniAnimIterClass CdkPixbufAniAnimIterClass;


#define CDK_TYPE_PIXBUF_ANI_ANIM_ITER              (cdk_pixbuf_ani_anim_iter_get_type ())
#define CDK_PIXBUF_ANI_ANIM_ITER(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), CDK_TYPE_PIXBUF_ANI_ANIM_ITER, CdkPixbufAniAnimIter))
#define CDK_IS_PIXBUF_ANI_ANIM_ITER(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), CDK_TYPE_PIXBUF_ANI_ANIM_ITER))

#define CDK_PIXBUF_ANI_ANIM_ITER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), CDK_TYPE_PIXBUF_ANI_ANIM_ITER, CdkPixbufAniAnimIterClass))
#define CDK_IS_PIXBUF_ANI_ANIM_ITER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CDK_TYPE_PIXBUF_ANI_ANIM_ITER))
#define CDK_PIXBUF_ANI_ANIM_ITER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), CDK_TYPE_PIXBUF_ANI_ANIM_ITER, CdkPixbufAniAnimIterClass))

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
struct _CdkPixbufAniAnimIter {
        CdkPixbufAnimationIter parent_instance;
        
        CdkPixbufAniAnim   *ani_anim;

        GTimeVal            start_time;
        GTimeVal            current_time;

        /* Time in milliseconds into this run of the animation */
        gint                position;

        /* Index of the current frame */
        gint                current_frame;

        /* Time in milliseconds from the start of the animation till the
           begin of the current frame */
        gint                elapsed;
};
G_GNUC_END_IGNORE_DEPRECATIONS

struct _CdkPixbufAniAnimIterClass {
        CdkPixbufAnimationIterClass parent_class;

};

GType cdk_pixbuf_ani_anim_iter_get_type (void) G_GNUC_CONST;

#endif
