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

#include <gio/gio.h>
#include <android/imagedecoder.h>
#include <android/bitmap.h>
#include <android/data_space.h>

#include "io-android-utils.h"

/* {{{ AImageDecoder utilities */

static gboolean
cdk_pixbuf__android_check_error (gint rc, GError **error)
{
  if (rc == ANDROID_IMAGE_DECODER_SUCCESS)
    return FALSE;
  g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
               "Android Image Decoder Error: %s", AImageDecoder_resultToString (rc));
  return TRUE;
}

static void
cdk_pixbuf__android_image_decoder_initial_setup (AImageDecoder *decoder)
{
  AImageDecoder_setAndroidBitmapFormat (decoder, ANDROID_BITMAP_FORMAT_RGBA_8888);
  AImageDecoder_setUnpremultipliedRequired (decoder, TRUE);
}

static CdkPixbuf *
cdk_pixbuf__android_allocate_pixbuf (gint32 width, gint32 height, GError **error)
{
  CdkPixbuf *pixbuf;
  g_return_val_if_fail (width >= 0 && height >= 0, NULL);

  pixbuf = cdk_pixbuf_new (CDK_COLORSPACE_RGB, TRUE, 8, width, height);
  if (!pixbuf)
    {
      g_set_error_literal (error,
                           CDK_PIXBUF_ERROR,
                           CDK_PIXBUF_ERROR_INSUFFICIENT_MEMORY,
                           "Insufficient memory to allocate image");
      return NULL;
    }
  return pixbuf;
}

static gboolean
cdk_pixbuf__android_image_decoder_to_pixbuf (AImageDecoder *decoder,
                                             /*gint32 width, gint32 height,*/
                                             CdkPixbuf *pixbuf,
                                             GError **error)
{
  guint len;
  guchar *data;
  gint rc;

  data = cdk_pixbuf_get_pixels_with_length (pixbuf, &len);
  g_assert (len == cdk_pixbuf_get_height (pixbuf) * AImageDecoder_getMinimumStride (decoder));

  rc = AImageDecoder_decodeImage(decoder, data, cdk_pixbuf_get_rowstride (pixbuf), len);
  return !cdk_pixbuf__android_check_error (rc, error);
}

/* }}} */
/* {{{ Animations */

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static gint64
cdk_pixbuf__android_anomation_iter_timeval_to_usec (const GTimeVal *timeval)
{
  return timeval->tv_sec * G_USEC_PER_SEC +
         timeval->tv_usec;
}
G_GNUC_END_IGNORE_DEPRECATIONS

typedef struct {
  CdkPixbuf *pixbuf;
  gint64 delay; // usec
} CdkPixbufAndroidFrame;

static void
cdk_pixbuf__android_frame_clear (CdkPixbufAndroidFrame *self)
{
  g_object_unref (self->pixbuf);
}

// ugly hack to work arround the fact that cdk-pixbuf doesn't expose autoptr
#undef _GLIB_DEFINE_AUTOPTR_CHAINUP
#define _GLIB_DEFINE_AUTOPTR_CHAINUP(ModuleObjName, ParentName)

#define CDK_PIXBUF_TYPE_ANDROID_ANIMATION (cdk_pixbuf__android_animation_get_type ())
#define CDK_PIXBUF_TYPE_ANDROID_ANIMATION_ITER (cdk_pixbuf__android_animation_iter_get_type ())
G_DECLARE_FINAL_TYPE (CdkPixbufAndroidAnimation, cdk_pixbuf__android_animation, CDK_PIXBUF, ANDROID_ANIMATION, CdkPixbufAnimation)
G_DECLARE_FINAL_TYPE (CdkPixbufAndroidAnimationIter, cdk_pixbuf__android_animation_iter, CDK_PIXBUF, ANDROID_ANIMATION_ITER, CdkPixbufAnimationIter)

struct _CdkPixbufAndroidAnimation
{
  CdkPixbufAnimation parent_instance;
  GBytes *imgdata;
  gint32 width,height;

  GMutex lock;
  AImageDecoder *decoder;
  GArray *decoded;
};

struct _CdkPixbufAndroidAnimationIter {
  CdkPixbufAnimationIter parent_instance;

  CdkPixbufAndroidAnimation *animation;
  guint idx;
  gint32 repeat_count;
  gint64 timestate;
  gboolean on_last_frame;
};

G_DEFINE_FINAL_TYPE (CdkPixbufAndroidAnimation, cdk_pixbuf__android_animation, CDK_TYPE_PIXBUF_ANIMATION)
G_DEFINE_FINAL_TYPE (CdkPixbufAndroidAnimationIter, cdk_pixbuf__android_animation_iter, CDK_TYPE_PIXBUF_ANIMATION_ITER)

static void
cdk_pixbuf__android_animation_finalize (GObject *object)
{
  CdkPixbufAndroidAnimation *self = (CdkPixbufAndroidAnimation *)object;
  AImageDecoder_delete (self->decoder);
  if (self->imgdata)
    g_bytes_unref (self->imgdata);
  g_array_unref (self->decoded);
  g_mutex_clear (&self->lock);
  G_OBJECT_CLASS (cdk_pixbuf__android_animation_parent_class)->finalize (object);
}

static gboolean
cdk_pixbuf__android_animation_is_static_image (G_GNUC_UNUSED CdkPixbufAnimation *animation)
{
  // An animation is never a static image
  return FALSE;
}

static CdkPixbuf *
cdk_pixbuf__android_animation_get_static_image (CdkPixbufAnimation *animation)
{
  CdkPixbuf *ret;
  CdkPixbufAndroidAnimation *self = (CdkPixbufAndroidAnimation *)animation;

  g_mutex_lock (&self->lock);
  if (self->decoded->len >= 1)
    ret = g_array_index (self->decoded, CdkPixbufAndroidFrame, 0).pixbuf;
  else
    ret = NULL;
  g_mutex_unlock (&self->lock);
  return ret;
}

static void
cdk_pixbuf__android_animation_get_size (CdkPixbufAnimation *animation, int *width, int *height)
{
  CdkPixbufAndroidAnimation *self = (CdkPixbufAndroidAnimation *)animation;
  if (width)
    *width = self->width;
  if (height)
    *height = self->height;
}

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static CdkPixbufAnimationIter *
cdk_pixbuf__android_animation_get_iter (CdkPixbufAnimation* animation, const GTimeVal* start_time)
{
  CdkPixbufAndroidAnimationIter *iter;
  CdkPixbufAndroidAnimation *self = (CdkPixbufAndroidAnimation *)animation;
  g_mutex_lock (&self->lock);
  if (self->decoded->len > 0)
    {
      iter = g_object_new (CDK_PIXBUF_TYPE_ANDROID_ANIMATION_ITER, NULL);
      iter->animation = g_object_ref (self);
      iter->idx = 0;
      iter->repeat_count = AImageDecoder_getRepeatCount (self->decoder);
      iter->timestate = cdk_pixbuf__android_anomation_iter_timeval_to_usec (start_time);
      iter->on_last_frame = FALSE;
    }
  else
    iter = NULL;
  g_mutex_unlock (&self->lock);
  return (CdkPixbufAnimationIter *)iter;
}
G_GNUC_END_IGNORE_DEPRECATIONS

static void
cdk_pixbuf__android_animation_class_init (CdkPixbufAndroidAnimationClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;
  CdkPixbufAnimationClass *animation_class = (CdkPixbufAnimationClass *)klass;

  object_class->finalize = cdk_pixbuf__android_animation_finalize;
  animation_class->is_static_image = cdk_pixbuf__android_animation_is_static_image;
  animation_class->get_static_image = cdk_pixbuf__android_animation_get_static_image;
  animation_class->get_size = cdk_pixbuf__android_animation_get_size;
  animation_class->get_iter = cdk_pixbuf__android_animation_get_iter;
}

static void
cdk_pixbuf__android_animation_init (CdkPixbufAndroidAnimation *self)
{
  self->imgdata = NULL;
  self->width = -1;
  self->height = -1;
  g_mutex_init (&self->lock);
  self->decoder = NULL;
  self->decoded = g_array_new (FALSE, FALSE, sizeof (CdkPixbufAndroidFrame));
  g_array_set_clear_func (self->decoded, (GDestroyNotify)cdk_pixbuf__android_frame_clear);
}

static void
cdk_pixbuf__android_animation_iter_finalize (GObject *object)
{
  CdkPixbufAndroidAnimationIter *self = (CdkPixbufAndroidAnimationIter *)object;
  g_object_unref (self->animation);
  G_OBJECT_CLASS (cdk_pixbuf__android_animation_iter_parent_class)->finalize (object);
}

static gint
cdk_pixbuf__android_animation_iter_get_delay_time (CdkPixbufAnimationIter *iter)
{
  CdkPixbufAndroidAnimationIter *self;
  gint delay_time;
  self = (CdkPixbufAndroidAnimationIter *)iter;
  if (self->on_last_frame)
    return -1;
  g_mutex_lock (&self->animation->lock);
  delay_time = g_array_index (self->animation->decoded, CdkPixbufAndroidFrame, self->idx).delay / 1000 /* usec to millis */;
  g_mutex_unlock (&self->animation->lock);
  return delay_time;
}

static CdkPixbuf *
cdk_pixbuf__android_animation_iter_get_pixbuf (CdkPixbufAnimationIter *iter)
{
  CdkPixbufAndroidAnimationIter *self;
  CdkPixbuf *ret;
  self = (CdkPixbufAndroidAnimationIter *)iter;
  g_mutex_lock (&self->animation->lock);
  ret = g_array_index (self->animation->decoded, CdkPixbufAndroidFrame, self->idx).pixbuf;
  g_mutex_unlock (&self->animation->lock);
  return ret;
}

static gboolean
cdk_pixbuf__android_animation_iter_on_currently_loading_frame (G_GNUC_UNUSED CdkPixbufAnimationIter *iter)
{
  return FALSE;
}

static gint64
cdk_pixbuf__android_get_delay_for_current_frame (AImageDecoder *decoder)
{
  gint64 duration;
  AImageDecoderFrameInfo *frameInfo = AImageDecoderFrameInfo_create();
  AImageDecoder_getFrameInfo (decoder, frameInfo);
  duration = AImageDecoderFrameInfo_getDuration(frameInfo);
  AImageDecoderFrameInfo_delete(frameInfo);
  if (duration < 0)
    return duration;
  return duration / 1000 /* nanos to usec */;
}

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static gboolean
cdk_pixbuf__android_animation_iter_advance (CdkPixbufAnimationIter *iter, const GTimeVal *current_time)
{
  CdkPixbufAndroidAnimationIter *self;
  gint64 new_time;
  gboolean retval;
  self = (CdkPixbufAndroidAnimationIter *)iter;
  if (self->on_last_frame)
    return FALSE;

  new_time = cdk_pixbuf__android_anomation_iter_timeval_to_usec (current_time);
  g_mutex_lock (&self->animation->lock);
  retval = FALSE;
  while ((self->timestate + g_array_index (self->animation->decoded, CdkPixbufAndroidFrame, self->idx).delay) < new_time)
    {
      if (self->animation->decoded->len <= self->idx+1) // currently on last decoded frame
        {
          gint rc;
          GError *error;
          CdkPixbuf *pixbuf;
          CdkPixbufAndroidFrame frame;
          rc = AImageDecoder_advanceFrame (self->animation->decoder);
          if (rc == ANDROID_IMAGE_DECODER_FINISHED)
            {
              if (self->repeat_count == ANDROID_IMAGE_DECODER_INFINITE)
                {
                  self->timestate += g_array_index (self->animation->decoded, CdkPixbufAndroidFrame, self->idx).delay;
                  self->idx = 0;
                  retval = TRUE;
                }
              else
                {
                  if (self->repeat_count > 0)
                    {
                      self->timestate += g_array_index (self->animation->decoded, CdkPixbufAndroidFrame, self->idx).delay;
                      self->idx = 0;
                      retval = TRUE;
                      --self->repeat_count;
                    }
                  else
                    self->on_last_frame = TRUE;
                }
              break;
            }
          else if (rc != ANDROID_IMAGE_DECODER_SUCCESS)
            {
              g_critical ("Android animation decoder failure: %s", AImageDecoder_resultToString(rc));
              break;
            }
          error = NULL;
          pixbuf = cdk_pixbuf__android_allocate_pixbuf (self->animation->width, self->animation->height, &error);
          if (!pixbuf)
            {
              g_critical ("Failed to allocate android pixbuf: %s", error->message);
              g_error_free (error);
              break;
            }
          if (!cdk_pixbuf__android_image_decoder_to_pixbuf (self->animation->decoder, pixbuf, &error))
            {
              g_critical ("Android animation decoder failure: %s", error->message);
              g_error_free (error);
              break;
            }

          frame.pixbuf = pixbuf;
          frame.delay = cdk_pixbuf__android_get_delay_for_current_frame (self->animation->decoder);
          g_array_append_val (self->animation->decoded, frame);
        }
      self->timestate += g_array_index (self->animation->decoded, CdkPixbufAndroidFrame, self->idx).delay;
      self->idx++;
      retval = TRUE;
    }
  g_mutex_unlock (&self->animation->lock);
  return retval;
}
G_GNUC_END_IGNORE_DEPRECATIONS

static void
cdk_pixbuf__android_animation_iter_class_init (CdkPixbufAndroidAnimationIterClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;
  CdkPixbufAnimationIterClass *iter_class = (CdkPixbufAnimationIterClass *)klass;

  object_class->finalize = cdk_pixbuf__android_animation_iter_finalize;
  iter_class->get_delay_time = cdk_pixbuf__android_animation_iter_get_delay_time;
  iter_class->get_pixbuf = cdk_pixbuf__android_animation_iter_get_pixbuf;
  iter_class->on_currently_loading_frame = cdk_pixbuf__android_animation_iter_on_currently_loading_frame;
  iter_class->advance = cdk_pixbuf__android_animation_iter_advance;
}

static void
cdk_pixbuf__android_animation_iter_init (CdkPixbufAndroidAnimationIter *self)
{
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  GTimeVal current;
  g_get_current_time (&current);
  self->timestate = cdk_pixbuf__android_anomation_iter_timeval_to_usec (&current);
G_GNUC_END_IGNORE_DEPRECATIONS

  self->animation = NULL;
  self->idx = 0;
  self->repeat_count = -1;
  self->on_last_frame = FALSE;
}

static CdkPixbufAndroidAnimation *
cdk_pixbuf__android_get_animation (GBytes *imgdata /* takes ownership */,
                                   AImageDecoder *decoder /* takes ownership */,
                                   gint32 width, gint32 height)
{
  CdkPixbufAndroidAnimation *self;
  g_assert (AImageDecoder_isAnimated (decoder)); // may only be called for animated images

  self = g_object_new (CDK_PIXBUF_TYPE_ANDROID_ANIMATION, NULL);
  self->imgdata = imgdata;
  self->width = width;
  self->height = height;
  self->decoder = decoder;

  return self;
}

/* }}} */
/* {{{ Module implementation */

static CdkPixbuf *
cdk_pixbuf__android_image_load (FILE *f, GError **error)
{
  AImageDecoder *decoder;
  gint rc;
  const AImageDecoderHeaderInfo *header;
  gint32 width,height;
  CdkPixbuf *ret;

  rc = AImageDecoder_createFromFd (fileno (f), &decoder);
  if (cdk_pixbuf__android_check_error (rc, error))
    return NULL;
  cdk_pixbuf__android_image_decoder_initial_setup (decoder);
  header = AImageDecoder_getHeaderInfo (decoder);
  width = AImageDecoderHeaderInfo_getWidth (header);
  height = AImageDecoderHeaderInfo_getHeight (header);
  ret = cdk_pixbuf__android_allocate_pixbuf (width, height, error);
  if (!ret)
    {
      AImageDecoder_delete (decoder);
      return NULL;
    }
  if (!cdk_pixbuf__android_image_decoder_to_pixbuf (decoder, ret, error))
    {
      g_object_unref (ret);
      AImageDecoder_delete (decoder);
      return NULL;
    }
  AImageDecoder_delete (decoder);
  return ret;
}

typedef struct
{
  CdkPixbufModuleSizeFunc     size_func;
  CdkPixbufModulePreparedFunc prepared_func;
  CdkPixbufModuleUpdatedFunc  updated_func;
  gpointer                    user_data;

  GByteArray *buffer;
} CdkPixbufAndroidLoaderContext;

static gpointer
cdk_pixbuf__android_image_begin_load (CdkPixbufModuleSizeFunc       size_func,
                                      CdkPixbufModulePreparedFunc   prepared_func,
                                      CdkPixbufModuleUpdatedFunc    updated_func,
                                      gpointer                      user_data,
                                      GError                      **error)
{
  CdkPixbufAndroidLoaderContext *context = g_new (CdkPixbufAndroidLoaderContext, 1);
  context->buffer = g_byte_array_new ();
  context->size_func     = size_func;
  context->prepared_func = prepared_func;
  context->updated_func  = updated_func;
  context->user_data     = user_data;
  return context;
}

static gboolean
cdk_pixbuf__android_image_load_increment (gpointer       data,
                                          const guchar  *buf,
                                          guint          size,
                                          GError       **error)
{
  CdkPixbufAndroidLoaderContext *context = (CdkPixbufAndroidLoaderContext *)data;
  g_byte_array_append (context->buffer, buf, size);
  return TRUE;
}

static gboolean
cdk_pixbuf__android_image_stop_load (gpointer data, GError **error)
{
  AImageDecoder *decoder;
  gint rc;
  const AImageDecoderHeaderInfo *header;
  gint32 width,height,set_width,set_height;
  CdkPixbuf *pixbuf;
  CdkPixbufAndroidLoaderContext *context = (CdkPixbufAndroidLoaderContext *)data;
  GBytes *imgdata = g_byte_array_free_to_bytes (context->buffer);

  rc = AImageDecoder_createFromBuffer (context->buffer->data, context->buffer->len, &decoder);
  if (cdk_pixbuf__android_check_error (rc, error))
    {
      g_bytes_unref (imgdata);
      g_free (context);
      return FALSE;
    }

  cdk_pixbuf__android_image_decoder_initial_setup (decoder);
  header = AImageDecoder_getHeaderInfo (decoder);
  width = AImageDecoderHeaderInfo_getWidth (header);
  height = AImageDecoderHeaderInfo_getHeight (header);
  set_width = width;
  set_height = height;
  (* context->size_func) (&set_width, &set_height, context->user_data);
  // Let someone else (ideally GL) deal with upscaling
  if (set_width <= width && set_height <= height && (width != set_width || height != set_height))
    {
      width = set_width;
      height = set_height;
      rc = AImageDecoder_setTargetSize (decoder, width, height);
      if (cdk_pixbuf__android_check_error (rc, error))
        {
          AImageDecoder_delete (decoder);
          g_bytes_unref (imgdata);
          g_free (context);
          return FALSE;
        }
    }

  pixbuf = cdk_pixbuf__android_allocate_pixbuf (width, height, error);
  if (!pixbuf)
    {
      AImageDecoder_delete (decoder);
      g_bytes_unref (imgdata);
      g_free (context);
      return FALSE;
    }

  if (AImageDecoder_isAnimated (decoder))
    {
      CdkPixbufAndroidAnimation *animation;
      CdkPixbufAndroidFrame frame;
      animation = cdk_pixbuf__android_get_animation (imgdata, decoder, width, height);
      g_mutex_lock (&animation->lock);
      frame.pixbuf = pixbuf;
      frame.delay = cdk_pixbuf__android_get_delay_for_current_frame (animation->decoder);
      g_array_append_val (animation->decoded, frame);
      g_mutex_unlock (&animation->lock);

      (* context->prepared_func) (pixbuf, (CdkPixbufAnimation *)animation, context->user_data);
      if (!cdk_pixbuf__android_image_decoder_to_pixbuf (animation->decoder, pixbuf, error))
        {
          g_object_unref (animation);
          g_free (context);
          return FALSE;
        }
      (* context->updated_func) (pixbuf, 0, 0, width, height, context->user_data);

      g_object_unref (animation);
      g_free (context);
      return TRUE;
    }
  else
    {
      (* context->prepared_func) (pixbuf, NULL, context->user_data);
      if (!cdk_pixbuf__android_image_decoder_to_pixbuf (decoder, pixbuf, error))
        {
          AImageDecoder_delete (decoder);
          g_bytes_unref (imgdata);
          g_object_unref (pixbuf);
          g_free (context);
          return FALSE;
        }
      (* context->updated_func) (pixbuf, 0, 0, width, height, context->user_data);
      AImageDecoder_delete (decoder);
      g_bytes_unref (imgdata);
      g_object_unref (pixbuf);
      g_free (context);
      return TRUE;
    }
}

static CdkPixbufAnimation *
cdk_pixbuf__android_image_load_animation (FILE *f, GError **error)
{
  AImageDecoder *decoder;
  gint rc;
  const AImageDecoderHeaderInfo *header;
  gint32 width,height;
  CdkPixbuf *initial_frame;
  CdkPixbufAnimation *animation;
  CdkPixbufAndroidFrame frame;

  rc = AImageDecoder_createFromFd (fileno (f), &decoder);
  if (cdk_pixbuf__android_check_error (rc, error))
    return NULL;
  cdk_pixbuf__android_image_decoder_initial_setup (decoder);
  header = AImageDecoder_getHeaderInfo (decoder);
  width = AImageDecoderHeaderInfo_getWidth (header);
  height = AImageDecoderHeaderInfo_getHeight (header);

  initial_frame = cdk_pixbuf__android_allocate_pixbuf (width, height, error);
  if (!initial_frame)
    {
      AImageDecoder_delete (decoder);
      return FALSE;
    }

  if (!cdk_pixbuf__android_image_decoder_to_pixbuf (decoder, initial_frame, error))
    {
      AImageDecoder_delete (decoder);
      g_object_unref (initial_frame);
      return FALSE;
    }

  if (!AImageDecoder_isAnimated (decoder))
    {
      animation = cdk_pixbuf_non_anim_new (initial_frame);
      AImageDecoder_delete (decoder);
      g_object_unref (initial_frame);
      return animation;
    }

  animation = (CdkPixbufAnimation *)cdk_pixbuf__android_get_animation (NULL, decoder,
                                                                       width, height);
  frame.pixbuf = initial_frame;
  frame.delay = cdk_pixbuf__android_get_delay_for_current_frame (((CdkPixbufAndroidAnimation *)animation)->decoder);
  g_array_append_val (((CdkPixbufAndroidAnimation *)animation)->decoded, frame);
  return animation;
}

void
cdk_pixbuf__android_fill_vtable (CdkPixbufModule *module)
{
  module->load = cdk_pixbuf__android_image_load;
  module->begin_load = cdk_pixbuf__android_image_begin_load;
  module->stop_load = cdk_pixbuf__android_image_stop_load;
  module->load_increment = cdk_pixbuf__android_image_load_increment;
  module->load_animation = cdk_pixbuf__android_image_load_animation;
}

/* }}} */
/* {{{ Save image functionality */

typedef struct
{
  CdkPixbufSaveFunc save_func;
  gpointer user_data;

  GError **error;
} CdkPixbufAndroidSaveImageData;

static bool
cdk_pixbuf__android_save_image_cb (gpointer user_data, const void *data, gsize size)
{
  CdkPixbufAndroidSaveImageData *save_image_data = (CdkPixbufAndroidSaveImageData *)user_data;
  return save_image_data->save_func (data, size, save_image_data->error, save_image_data->user_data);
}

gboolean
cdk_pixbuf__android_save_image (CdkPixbufSaveFunc save_func, gpointer user_data, CdkPixbuf *pixbuf, gint32 format, GError **error)
{
  AndroidBitmapInfo info;
  CdkPixbufAndroidSaveImageData data;
  gint rc;
  info = (AndroidBitmapInfo){
    .width = cdk_pixbuf_get_width (pixbuf),
    .height = cdk_pixbuf_get_height (pixbuf),
    .stride = cdk_pixbuf_get_rowstride (pixbuf),
    .format = ANDROID_BITMAP_FORMAT_RGBA_8888,
    .flags = ANDROID_BITMAP_FLAGS_ALPHA_UNPREMUL
  };
  data = (CdkPixbufAndroidSaveImageData){
    .save_func = save_func,
    .user_data = user_data,
    .error = error
  };
  rc = AndroidBitmap_compress(&info, ADATASPACE_SRGB,
                              cdk_pixbuf_get_pixels (pixbuf),
                              format, 90,
                              &data, cdk_pixbuf__android_save_image_cb);
  if (data.error && *data.error)
    return FALSE;
  if (rc == ANDROID_BITMAP_RESULT_SUCCESS)
    return TRUE;
  switch (rc)
    {
      case ANDROID_BITMAP_RESULT_BAD_PARAMETER:
        g_set_error_literal (error, G_IO_ERROR, CDK_PIXBUF_ERROR_FAILED, "Bad parameter");
        break;
      case ANDROID_BITMAP_RESULT_JNI_EXCEPTION:
        g_set_error_literal (error, G_IO_ERROR, CDK_PIXBUF_ERROR_FAILED, "JNI exception occured");
        break;
      case ANDROID_BITMAP_RESULT_ALLOCATION_FAILED:
        g_set_error_literal (error, G_IO_ERROR, CDK_PIXBUF_ERROR_INSUFFICIENT_MEMORY, "Allocation failed");
        break;
      default:
        g_set_error_literal (error, G_IO_ERROR, CDK_PIXBUF_ERROR_FAILED, "Unknown error");
    }
  return FALSE;
}

/* }}} */
/* vim:set foldmethod=marker: */
