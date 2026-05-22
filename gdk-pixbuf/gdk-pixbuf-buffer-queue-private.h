/* CdkPixbuf library
 * Copyright (C) 2003-2006 David Schleef <ds@schleef.org>
 *		 2005-2006 Eric Anholt <eric@anholt.net>
 *		 2006-2007 Benjamin Otte <otte@gnome.org>
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

#ifndef __CDK_PIXBUF_BUFFER_QUEUE_H__
#define __CDK_PIXBUF_BUFFER_QUEUE_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _CdkPixbufBufferQueue CdkPixbufBufferQueue;

CdkPixbufBufferQueue *  cdk_pixbuf_buffer_queue_new             (void);

CdkPixbufBufferQueue *  cdk_pixbuf_buffer_queue_ref             (CdkPixbufBufferQueue   *queue);
void                    cdk_pixbuf_buffer_queue_unref           (CdkPixbufBufferQueue   *queue);

gsize                   cdk_pixbuf_buffer_queue_get_size        (CdkPixbufBufferQueue   *queue);
gsize                   cdk_pixbuf_buffer_queue_get_offset      (CdkPixbufBufferQueue   *queue);

void                    cdk_pixbuf_buffer_queue_flush           (CdkPixbufBufferQueue   *queue,
                                                                 gsize                   n_bytes);
void                    cdk_pixbuf_buffer_queue_clear           (CdkPixbufBufferQueue   *queue);
void                    cdk_pixbuf_buffer_queue_push            (CdkPixbufBufferQueue   *queue,
                                                                 GBytes                 *buffer);
GBytes *                cdk_pixbuf_buffer_queue_pull            (CdkPixbufBufferQueue   *queue,
                                                                 gsize                   length);
GBytes *                cdk_pixbuf_buffer_queue_pull_buffer     (CdkPixbufBufferQueue   *queue);
GBytes *                cdk_pixbuf_buffer_queue_peek            (CdkPixbufBufferQueue   *queue,
                                                                 gsize                   length);
GBytes *                cdk_pixbuf_buffer_queue_peek_buffer     (CdkPixbufBufferQueue   *queue);

G_END_DECLS
#endif

