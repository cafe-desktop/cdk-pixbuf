/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* CdkPixbuf library - GIF loader declarations
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

#ifndef CDK_PIXBUF_GIF_H
#define CDK_PIXBUF_GIF_H

#include "cdk-pixbuf-animation.h"

typedef enum {
        /* Keep this frame and composite next frame over it */
        /* (GIF disposal method 1) */
	CDK_PIXBUF_FRAME_RETAIN,
        /* Revert to background color before compositing next frame */
        /* (GIF disposal method 2) */
	CDK_PIXBUF_FRAME_DISPOSE,
        /* Revert to previously-displayed composite image after
         * displaying this frame
         */
        /* (GIF disposal method 3) */
	CDK_PIXBUF_FRAME_REVERT
} CdkPixbufFrameAction;



typedef struct _CdkPixbufGifAnim CdkPixbufGifAnim;
typedef struct _CdkPixbufGifAnimClass CdkPixbufGifAnimClass;
typedef struct _CdkPixbufFrame CdkPixbufFrame;

#define CDK_TYPE_PIXBUF_GIF_ANIM              (cdk_pixbuf_gif_anim_get_type ())
#define CDK_PIXBUF_GIF_ANIM(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), CDK_TYPE_PIXBUF_GIF_ANIM, CdkPixbufGifAnim))
#define CDK_IS_PIXBUF_GIF_ANIM(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), CDK_TYPE_PIXBUF_GIF_ANIM))

#define CDK_PIXBUF_GIF_ANIM_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), CDK_TYPE_PIXBUF_GIF_ANIM, CdkPixbufGifAnimClass))
#define CDK_IS_PIXBUF_GIF_ANIM_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CDK_TYPE_PIXBUF_GIF_ANIM))
#define CDK_PIXBUF_GIF_ANIM_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), CDK_TYPE_PIXBUF_GIF_ANIM, CdkPixbufGifAnimClass))

/* Private part of the CdkPixbufGifAnim structure */
struct _CdkPixbufGifAnim {
        CdkPixbufAnimation parent_instance;

        /* Total length of animation */
        int total_time;
        
        /* Color map */
        guchar color_map[256 * 3];

	/* List of CdkPixbufFrame structures */
        GList *frames;

	/* bounding box size */
	int width, height;

        int loop;

        /* Last rendered frames */
	CdkPixbuf *last_frame_data;
	CdkPixbufFrame *last_frame;
	CdkPixbuf *last_frame_revert_data;
};

struct _CdkPixbufGifAnimClass {
        CdkPixbufAnimationClass parent_class;
        
};

GType cdk_pixbuf_gif_anim_get_type (void) G_GNUC_CONST;



typedef struct _CdkPixbufGifAnimIter CdkPixbufGifAnimIter;
typedef struct _CdkPixbufGifAnimIterClass CdkPixbufGifAnimIterClass;


#define CDK_TYPE_PIXBUF_GIF_ANIM_ITER              (cdk_pixbuf_gif_anim_iter_get_type ())
#define CDK_PIXBUF_GIF_ANIM_ITER(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), CDK_TYPE_PIXBUF_GIF_ANIM_ITER, CdkPixbufGifAnimIter))
#define CDK_IS_PIXBUF_GIF_ANIM_ITER(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), CDK_TYPE_PIXBUF_GIF_ANIM_ITER))

#define CDK_PIXBUF_GIF_ANIM_ITER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), CDK_TYPE_PIXBUF_GIF_ANIM_ITER, CdkPixbufGifAnimIterClass))
#define CDK_IS_PIXBUF_GIF_ANIM_ITER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CDK_TYPE_PIXBUF_GIF_ANIM_ITER))
#define CDK_PIXBUF_GIF_ANIM_ITER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), CDK_TYPE_PIXBUF_GIF_ANIM_ITER, CdkPixbufGifAnimIterClass))

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
struct _CdkPixbufGifAnimIter {
        CdkPixbufAnimationIter parent_instance;
        
        CdkPixbufGifAnim   *gif_anim;

        GTimeVal            start_time;
        GTimeVal            current_time;

        /* Time in milliseconds into this run of the animation */
        gint                position;
        
        GList              *current_frame;
        
        gint                first_loop_slowness;
};
G_GNUC_END_IGNORE_DEPRECATIONS

struct _CdkPixbufGifAnimIterClass {
        CdkPixbufAnimationIterClass parent_class;

};

GType cdk_pixbuf_gif_anim_iter_get_type (void) G_GNUC_CONST;



struct _CdkPixbufFrame {
	/* Compressed frame data */
	GByteArray *lzw_data;
	guint8 lzw_code_size;

        /* Position of frame data in image */
        int x_offset;
	int y_offset;
	guint16 width;
	guint16 height;

	/* Layout of pixels */
	gboolean interlace;

	/* Color map */
	gboolean color_map_allocated;
	guchar *color_map;

	/* Transparency */
	int transparent_index;

	/* Frame duration in ms */
	int delay_time;

        /* Sum of preceding delay times */
        int elapsed;
        
        /* Overlay mode */
	CdkPixbufFrameAction action;
};

#endif
