/* -*- Mode: C; c-basic-offset: 2; -*- */
/* GdkPixbuf library - test loaders
 *
 * Copyright (C) 2001 Søren Sandmann (sandmann@daimi.au.dk)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "cdk-pixbuf/cdk-pixbuf.h"
#include <stdio.h>
#include <stdlib.h>

static gboolean
load_and_save (const char *filename, GError **error)
{
  GdkPixbuf *pixbuf = NULL;
  GdkPixbufLoader *loader;
  guchar *contents;
  char *new_filename = NULL;
  gsize size;
  gboolean ret = TRUE;

  if (!g_file_get_contents (filename, (char **) &contents, &size, error))
    return FALSE;

  loader = cdk_pixbuf_loader_new ();
  if (!cdk_pixbuf_loader_write (loader, contents, size, error))
    {
      ret = FALSE;
      goto out;
    }

  if (!cdk_pixbuf_loader_close (loader, error))
    {
      if (!g_error_matches (*error, CDK_PIXBUF_ERROR, CDK_PIXBUF_ERROR_INCOMPLETE_ANIMATION))
        {
          ret = FALSE;
          goto out;
        }
      g_clear_error (error);
    }

  pixbuf = cdk_pixbuf_loader_get_pixbuf (loader);
  g_assert (pixbuf);
  g_object_ref (pixbuf);

  g_object_unref (loader);

  new_filename = g_strdup_printf ("%s.ref.png", filename);
  ret = cdk_pixbuf_save (pixbuf, new_filename, "png", error, NULL);

out:
  g_free (contents);
  g_free (new_filename);
  g_clear_object (&pixbuf);

  return ret;
}

static void
usage (void)
{
  g_print ("usage: pixbuf-save-ref <files>\n");
  exit (EXIT_FAILURE);
}

int
main (int argc, char **argv)
{
  int i;

  g_log_set_always_fatal (G_LOG_LEVEL_WARNING | G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL);

  if (argc == 1)
    usage();

  for (i = 1; i < argc; ++i)
    {
      GError *err = NULL;

      g_print ("%s\t\t", argv[i]);
      fflush (stdout);
      if (!load_and_save (argv[i], &err))
        {
          fprintf (stderr, "%s: error: %s\n", argv[i], err->message);
          g_clear_error (&err);
        }
      else
        {
          g_print ("success\n");
        }
    }

  return 0;
}
