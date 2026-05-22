/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* CdkPixbuf library - Simple transformations of animations
 *
 * Copyright (C) Red Hat, Inc
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
 *
 */

#include <glib.h>

#include "cdk-pixbuf.h"
#include "cdk-pixbuf-io.h"
#include "cdk-pixbuf-scaled-anim.h"

G_GNUC_BEGIN_IGNORE_DEPRECATIONS

struct _CdkPixbufScaledAnimClass
{
        CdkPixbufAnimationClass parent_class;
};

struct _CdkPixbufScaledAnim
{
 	CdkPixbufAnimation parent_instance;

	CdkPixbufAnimation *anim;
	gdouble xscale;
	gdouble yscale;
	gdouble tscale;

	CdkPixbuf *current;
};

struct _CdkPixbufScaledAnimIterClass
{
        CdkPixbufAnimationClass parent_class;
};

struct _CdkPixbufScaledAnimIter
{
 	CdkPixbufAnimationIter parent_instance;

	CdkPixbufScaledAnim *scaled;
        CdkPixbufAnimationIter *iter;
};

typedef struct _CdkPixbufScaledAnimIter CdkPixbufScaledAnimIter;
typedef struct _CdkPixbufScaledAnimIterClass CdkPixbufScaledAnimIterClass;

CdkPixbufScaledAnim *
_cdk_pixbuf_scaled_anim_new (CdkPixbufAnimation *anim,
                             gdouble             xscale,
                             gdouble             yscale,
                             gdouble             tscale)
{
	CdkPixbufScaledAnim *scaled;

	scaled = g_object_new (CDK_TYPE_PIXBUF_SCALED_ANIM, NULL);

	scaled->anim = g_object_ref (anim);
	scaled->xscale = xscale;
	scaled->yscale = yscale;
	scaled->tscale = tscale;

	return scaled;
}

G_DEFINE_TYPE (CdkPixbufScaledAnim, cdk_pixbuf_scaled_anim, CDK_TYPE_PIXBUF_ANIMATION);

static void
cdk_pixbuf_scaled_anim_init (CdkPixbufScaledAnim *scaled)
{
	scaled->xscale = 1.0;
	scaled->yscale = 1.0;
	scaled->tscale = 1.0;
}

static void
cdk_pixbuf_scaled_anim_finalize (GObject *object)
{
	CdkPixbufScaledAnim *scaled = (CdkPixbufScaledAnim *)object;

	if (scaled->anim) {
		g_object_unref (scaled->anim);
		scaled->anim = NULL;
	}

	if (scaled->current) {
		g_object_unref (scaled->current);
		scaled->current = NULL;
	}

	G_OBJECT_CLASS (cdk_pixbuf_scaled_anim_parent_class)->finalize (object);
}

static gboolean
is_static_image (CdkPixbufAnimation *anim)
{
	CdkPixbufScaledAnim *scaled = (CdkPixbufScaledAnim *)anim;

	return cdk_pixbuf_animation_is_static_image (scaled->anim);
}	

static CdkPixbuf *
get_scaled_pixbuf (CdkPixbufScaledAnim *scaled, 
                   CdkPixbuf           *pixbuf)
{
	GQuark  quark;
	gchar **options;

	if (scaled->current) 
		g_object_unref (scaled->current);

	/* Preserve the options associated with the original pixbuf 
	   (if present), mostly so that client programs can use the
	   "orientation" option (if present) to rotate the image 
	   appropriately. cdk_pixbuf_scale_simple (and most other
           cdk transform operations) does not preserve the attached
           options when returning a new pixbuf. */

	quark = g_quark_from_static_string ("cdk_pixbuf_options");
	options = g_object_get_qdata (G_OBJECT (pixbuf), quark);

	/* Get a new scaled pixbuf */
	scaled->current  = cdk_pixbuf_scale_simple (pixbuf, 
                        MAX((int) ((gdouble) cdk_pixbuf_get_width (pixbuf) * scaled->xscale + .5), 1),
                        MAX((int) ((gdouble) cdk_pixbuf_get_height (pixbuf) * scaled->yscale + .5), 1),
			CDK_INTERP_BILINEAR);

	/* Copy the original pixbuf options to the scaled pixbuf */
        if (options && scaled->current)
	          g_object_set_qdata_full (G_OBJECT (scaled->current), quark, 
                                           g_strdupv (options), (GDestroyNotify) g_strfreev);

	return scaled->current;
}

static CdkPixbuf *
get_static_image (CdkPixbufAnimation *anim)
{
	CdkPixbufScaledAnim *scaled = (CdkPixbufScaledAnim *)anim;
	CdkPixbuf *pixbuf;
	
	pixbuf = cdk_pixbuf_animation_get_static_image (scaled->anim);
	return get_scaled_pixbuf (scaled, pixbuf);
}

static void
get_size (CdkPixbufAnimation *anim,
	  int                *width,
	  int 		     *height)
{
	CdkPixbufScaledAnim *scaled = (CdkPixbufScaledAnim *)anim;

        CDK_PIXBUF_ANIMATION_GET_CLASS (scaled->anim)->get_size (scaled->anim, width, height);
	if (width) 
		*width = (int)(*width * scaled->xscale + .5);
	if (height)
		*height = (int)(*height * scaled->yscale + .5);
}

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static CdkPixbufAnimationIter *
get_iter (CdkPixbufAnimation *anim,
          const GTimeVal     *start_time)
{
	CdkPixbufScaledAnim *scaled = (CdkPixbufScaledAnim *)anim;
	CdkPixbufScaledAnimIter *iter;

	iter = g_object_new (CDK_TYPE_PIXBUF_SCALED_ANIM_ITER, NULL);

	iter->scaled = g_object_ref (scaled);
	iter->iter = cdk_pixbuf_animation_get_iter (scaled->anim, start_time);
	
	return (CdkPixbufAnimationIter*)iter;
}
G_GNUC_END_IGNORE_DEPRECATIONS

static void
cdk_pixbuf_scaled_anim_class_init (CdkPixbufScaledAnimClass *klass)
{
        GObjectClass *object_class;
        CdkPixbufAnimationClass *anim_class;

        object_class = G_OBJECT_CLASS (klass);
        anim_class = CDK_PIXBUF_ANIMATION_CLASS (klass);
        
        object_class->finalize = cdk_pixbuf_scaled_anim_finalize;
        
        anim_class->is_static_image = is_static_image;
        anim_class->get_static_image = get_static_image;
        anim_class->get_size = get_size;
        anim_class->get_iter = get_iter;
}


G_DEFINE_TYPE (CdkPixbufScaledAnimIter, cdk_pixbuf_scaled_anim_iter, CDK_TYPE_PIXBUF_ANIMATION_ITER);

static void
cdk_pixbuf_scaled_anim_iter_init (CdkPixbufScaledAnimIter *iter)
{
}

static int
get_delay_time (CdkPixbufAnimationIter *iter)
{
	CdkPixbufScaledAnimIter *scaled = (CdkPixbufScaledAnimIter *)iter;
	int delay;

	delay = cdk_pixbuf_animation_iter_get_delay_time (scaled->iter);
	delay = (int)(delay * scaled->scaled->tscale);

	return delay;
}

static CdkPixbuf *
get_pixbuf (CdkPixbufAnimationIter *iter)
{
	CdkPixbufScaledAnimIter *scaled = (CdkPixbufScaledAnimIter *)iter;
	CdkPixbuf *pixbuf;

	pixbuf = cdk_pixbuf_animation_iter_get_pixbuf (scaled->iter);
	return get_scaled_pixbuf (scaled->scaled, pixbuf);
}

static gboolean 
on_currently_loading_frame (CdkPixbufAnimationIter *iter)
{
	CdkPixbufScaledAnimIter *scaled = (CdkPixbufScaledAnimIter *)iter;

	return cdk_pixbuf_animation_iter_on_currently_loading_frame (scaled->iter);
}

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static gboolean
advance (CdkPixbufAnimationIter *iter,
	 const GTimeVal         *current_time)
{
	CdkPixbufScaledAnimIter *scaled = (CdkPixbufScaledAnimIter *)iter;

	return cdk_pixbuf_animation_iter_advance (scaled->iter, current_time);
}
G_GNUC_END_IGNORE_DEPRECATIONS

static void
cdk_pixbuf_scaled_anim_iter_finalize (GObject *object)
{
        CdkPixbufScaledAnimIter *iter = (CdkPixbufScaledAnimIter *)object;
        
	g_object_unref (iter->iter);
   	g_object_unref (iter->scaled);

	G_OBJECT_CLASS (cdk_pixbuf_scaled_anim_iter_parent_class)->finalize (object);
}

static void
cdk_pixbuf_scaled_anim_iter_class_init (CdkPixbufScaledAnimIterClass *klass)
{
        GObjectClass *object_class;
        CdkPixbufAnimationIterClass *anim_iter_class;

        object_class = G_OBJECT_CLASS (klass);
        anim_iter_class = CDK_PIXBUF_ANIMATION_ITER_CLASS (klass);
        
        object_class->finalize = cdk_pixbuf_scaled_anim_iter_finalize;
        
        anim_iter_class->get_delay_time = get_delay_time;
        anim_iter_class->get_pixbuf = get_pixbuf;
        anim_iter_class->on_currently_loading_frame = on_currently_loading_frame;
        anim_iter_class->advance = advance;
}
