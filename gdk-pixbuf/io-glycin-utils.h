#pragma once

#include "cdk-pixbuf-io.h"

void glycin_fill_vtable (CdkPixbufModule *module);

gboolean glycin_image_save (const char         *mimetype,
                            FILE               *f,
                            CdkPixbufSaveFunc   save_func,
                            gpointer            user_data,
                            CdkPixbuf          *pixbuf,
                            char              **keys,
                            char              **values,
                            GBytes             *icc_profile,
                            int                 quality,
                            int                 compression,
                            GError            **error);
