bin = executable('cdk-pixbuf-thumbnailer',
           [ 'cdk-pixbuf-thumbnailer.c', 'gnome-thumbnailer-skeleton.c' ],
           c_args: common_cflags + [
             '-DTHUMBNAILER_RETURNS_PIXBUF',
             '-DTHUMBNAILER_USAGE="Thumbnail images"',
           ],
           dependencies: cdk_pixbuf_deps + [ cdkpixbuf_dep ],
           install: true)
meson.override_find_program('cdk-pixbuf-thumbnailer', bin)

cdk_pixbuf_print_mime_types = executable('cdk-pixbuf-print-mime-types',
                                         'cdk-pixbuf-print-mime-types.c',
                                         c_args: common_cflags,
                                         dependencies: cdk_pixbuf_deps + [ cdkpixbuf_dep ])

custom_target('thumbnailer',
              input: 'cdk-pixbuf-thumbnailer.thumbnailer.in',
              output: 'cdk-pixbuf-thumbnailer.thumbnailer',
              command: [
                gen_thumbnailer,
                '--printer', cdk_pixbuf_print_mime_types,
                '--pixdata', cdk_pixbuf_pixdata,
                '--loaders', loaders_cache,
                '--bindir', cdk_pixbuf_bindir,
                '@INPUT@',
                '@OUTPUT@',
              ],
              install: true,
              install_dir: join_paths(cdk_pixbuf_datadir, 'thumbnailers'))
