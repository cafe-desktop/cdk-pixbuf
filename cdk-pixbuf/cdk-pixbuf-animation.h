/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* CdkPixbuf library - Animation support
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

#ifndef CDK_PIXBUF_ANIMATION_H
#define CDK_PIXBUF_ANIMATION_H

#if defined(CDK_PIXBUF_DISABLE_SINGLE_INCLUDES) && !defined (CDK_PIXBUF_H_INSIDE) && !defined (CDK_PIXBUF_COMPILATION)
#error "Only <cdk-pixbuf/cdk-pixbuf.h> can be included directly."
#endif

#include <glib-object.h>
#include <cdk-pixbuf/cdk-pixbuf-core.h>

G_BEGIN_DECLS

/* Animation support */

typedef struct _CdkPixbufAnimation CdkPixbufAnimation;


typedef struct _CdkPixbufAnimationIter CdkPixbufAnimationIter;

#define CDK_TYPE_PIXBUF_ANIMATION              (cdk_pixbuf_animation_get_type ())
#define CDK_PIXBUF_ANIMATION(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), CDK_TYPE_PIXBUF_ANIMATION, CdkPixbufAnimation))
#define CDK_IS_PIXBUF_ANIMATION(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), CDK_TYPE_PIXBUF_ANIMATION))

#define CDK_TYPE_PIXBUF_ANIMATION_ITER              (cdk_pixbuf_animation_iter_get_type ())
#define CDK_PIXBUF_ANIMATION_ITER(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), CDK_TYPE_PIXBUF_ANIMATION_ITER, CdkPixbufAnimationIter))
#define CDK_IS_PIXBUF_ANIMATION_ITER(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), CDK_TYPE_PIXBUF_ANIMATION_ITER))

CDK_PIXBUF_AVAILABLE_IN_ALL
GType               cdk_pixbuf_animation_get_type        (void) G_GNUC_CONST;

#ifdef G_OS_WIN32
/* API/ABI compat, see cdk-pixbuf-core.h for details */
CDK_PIXBUF_DEPRECATED_IN_2_44
CdkPixbufAnimation *cdk_pixbuf_animation_new_from_file_utf8   (const char         *filename,
                                                               GError            **error);
#endif

CDK_PIXBUF_DEPRECATED_IN_2_44
CdkPixbufAnimation *cdk_pixbuf_animation_new_from_file   (const char         *filename,
                                                          GError            **error);
CDK_PIXBUF_DEPRECATED_IN_2_44
CdkPixbufAnimation *cdk_pixbuf_animation_new_from_stream (GInputStream       *stream,
                                                          GCancellable       *cancellable,
                                                          GError            **error);
CDK_PIXBUF_DEPRECATED_IN_2_44
void                cdk_pixbuf_animation_new_from_stream_async (GInputStream *stream,
                                                          GCancellable       *cancellable,
                                                          GAsyncReadyCallback callback,
                                                          gpointer            user_data);
CDK_PIXBUF_DEPRECATED_IN_2_44
CdkPixbufAnimation *cdk_pixbuf_animation_new_from_stream_finish (GAsyncResult*async_result,
                                                          GError            **error);
CDK_PIXBUF_DEPRECATED_IN_2_44
CdkPixbufAnimation *cdk_pixbuf_animation_new_from_resource(const char        *resource_path,
                                                          GError            **error);

CDK_PIXBUF_DEPRECATED_IN_2_0_FOR(g_object_ref)
CdkPixbufAnimation *cdk_pixbuf_animation_ref             (CdkPixbufAnimation *animation);
CDK_PIXBUF_DEPRECATED_IN_2_0_FOR(g_object_unref)
void                cdk_pixbuf_animation_unref           (CdkPixbufAnimation *animation);

CDK_PIXBUF_DEPRECATED_IN_2_44
int                 cdk_pixbuf_animation_get_width       (CdkPixbufAnimation *animation);
CDK_PIXBUF_DEPRECATED_IN_2_44
int                 cdk_pixbuf_animation_get_height      (CdkPixbufAnimation *animation);
CDK_PIXBUF_DEPRECATED_IN_2_44
gboolean            cdk_pixbuf_animation_is_static_image  (CdkPixbufAnimation *animation);
CDK_PIXBUF_DEPRECATED_IN_2_44
CdkPixbuf          *cdk_pixbuf_animation_get_static_image (CdkPixbufAnimation *animation);

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
CDK_PIXBUF_DEPRECATED_IN_2_44
CdkPixbufAnimationIter *cdk_pixbuf_animation_get_iter                        (CdkPixbufAnimation     *animation,
                                                                              const GTimeVal         *start_time);
G_GNUC_END_IGNORE_DEPRECATIONS

CDK_PIXBUF_DEPRECATED_IN_2_44
GType                   cdk_pixbuf_animation_iter_get_type                   (void) G_GNUC_CONST;
CDK_PIXBUF_DEPRECATED_IN_2_44
int                     cdk_pixbuf_animation_iter_get_delay_time             (CdkPixbufAnimationIter *iter);
CDK_PIXBUF_DEPRECATED_IN_2_44
CdkPixbuf              *cdk_pixbuf_animation_iter_get_pixbuf                 (CdkPixbufAnimationIter *iter);
CDK_PIXBUF_DEPRECATED_IN_2_44
gboolean                cdk_pixbuf_animation_iter_on_currently_loading_frame (CdkPixbufAnimationIter *iter);
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
CDK_PIXBUF_DEPRECATED_IN_2_44
gboolean                cdk_pixbuf_animation_iter_advance                    (CdkPixbufAnimationIter *iter,
                                                                              const GTimeVal         *current_time);
G_GNUC_END_IGNORE_DEPRECATIONS


#ifdef CDK_PIXBUF_ENABLE_BACKEND



/**
 * CdkPixbufAnimationClass:
 * @parent_class: the parent class
 * @is_static_image: returns whether the given animation is just a static image.
 * @get_static_image: returns a static image representing the given animation.
 * @get_size: fills @width and @height with the frame size of the animation.
 * @get_iter: returns an iterator for the given animation.
 * 
 * Modules supporting animations must derive a type from 
 * #CdkPixbufAnimation, providing suitable implementations of the 
 * virtual functions.
 *
 * Deprecated: 2.44: Use a different image loading library for animatable assets
 */
typedef struct _CdkPixbufAnimationClass CdkPixbufAnimationClass;

#define CDK_PIXBUF_ANIMATION_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), CDK_TYPE_PIXBUF_ANIMATION, CdkPixbufAnimationClass))
#define CDK_IS_PIXBUF_ANIMATION_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CDK_TYPE_PIXBUF_ANIMATION))
#define CDK_PIXBUF_ANIMATION_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), CDK_TYPE_PIXBUF_ANIMATION, CdkPixbufAnimationClass))

/* Private part of the CdkPixbufAnimation structure */
struct _CdkPixbufAnimation {
        GObject parent_instance;

};

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
struct _CdkPixbufAnimationClass {
        GObjectClass parent_class;

        /*< public >*/

        gboolean                (*is_static_image)  (CdkPixbufAnimation *animation);

        CdkPixbuf*              (*get_static_image) (CdkPixbufAnimation *animation);

        void                    (*get_size) (CdkPixbufAnimation *animation,
                                             int                 *width,
                                             int                 *height);

        CdkPixbufAnimationIter* (*get_iter) (CdkPixbufAnimation *animation,
                                             const GTimeVal     *start_time);
};
G_GNUC_END_IGNORE_DEPRECATIONS



/**
 * CdkPixbufAnimationIterClass:
 * @parent_class: the parent class
 * @get_delay_time: returns the time in milliseconds that the current frame 
 *  should be shown.
 * @get_pixbuf: returns the current frame.
 * @on_currently_loading_frame: returns whether the current frame of @iter is 
 *  being loaded.
 * @advance: advances the iterator to @current_time, possibly changing the 
 *  current frame.
 * 
 * Modules supporting animations must derive a type from 
 * #CdkPixbufAnimationIter, providing suitable implementations of the 
 * virtual functions.
 *
 * Deprecated: 2.44: Use a different image loading library for animatable assets
 */
typedef struct _CdkPixbufAnimationIterClass CdkPixbufAnimationIterClass;

#define CDK_PIXBUF_ANIMATION_ITER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), CDK_TYPE_PIXBUF_ANIMATION_ITER, CdkPixbufAnimationIterClass))
#define CDK_IS_PIXBUF_ANIMATION_ITER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CDK_TYPE_PIXBUF_ANIMATION_ITER))
#define CDK_PIXBUF_ANIMATION_ITER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), CDK_TYPE_PIXBUF_ANIMATION_ITER, CdkPixbufAnimationIterClass))

struct _CdkPixbufAnimationIter {
        GObject parent_instance;

};

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
struct _CdkPixbufAnimationIterClass {
        GObjectClass parent_class;

        /*< public >*/

        int        (*get_delay_time)   (CdkPixbufAnimationIter *iter);

        CdkPixbuf* (*get_pixbuf)       (CdkPixbufAnimationIter *iter);

        gboolean   (*on_currently_loading_frame) (CdkPixbufAnimationIter *iter);

        gboolean   (*advance)          (CdkPixbufAnimationIter *iter,
                                        const GTimeVal         *current_time);
};
G_GNUC_END_IGNORE_DEPRECATIONS

#define CDK_TYPE_PIXBUF_NON_ANIM              (cdk_pixbuf_non_anim_get_type ())
#define CDK_PIXBUF_NON_ANIM(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), CDK_TYPE_PIXBUF_NON_ANIM, CdkPixbufNonAnim))
#define CDK_IS_PIXBUF_NON_ANIM(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), CDK_TYPE_PIXBUF_NON_ANIM))


CDK_PIXBUF_AVAILABLE_IN_ALL
GType               cdk_pixbuf_non_anim_get_type (void) G_GNUC_CONST;
CDK_PIXBUF_DEPRECATED_IN_2_44
CdkPixbufAnimation* cdk_pixbuf_non_anim_new (CdkPixbuf *pixbuf);

#endif /* CDK_PIXBUF_ENABLE_BACKEND */

G_DEFINE_AUTOPTR_CLEANUP_FUNC (CdkPixbufAnimation, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (CdkPixbufAnimationIter, g_object_unref)

G_END_DECLS

#endif /* CDK_PIXBUF_ANIMATION_H */
