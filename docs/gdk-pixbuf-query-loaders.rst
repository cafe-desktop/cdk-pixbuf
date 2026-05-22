.. _cdk-pixbuf-query-loaders(1):

========================
cdk-pixbuf-query-loaders
========================

-------------------------------------
GdkPixbuf loader registration utility
-------------------------------------

SYNOPSIS
--------
|   **cdk-pixbuf-query-loaders** [OPTIONS...] [MODULES..]

DESCRIPTION
-----------

``cdk-pixbuf-query-loaders`` collects information about loadable modules for
``cdk-pixbuf`` and writes it to the default cache file location, or to
``stdout``.

If called without arguments, it looks for modules in the ``cdk-pixbuf`` loader
directory.

If called with arguments, it looks for the specified modules. The arguments may
be absolute or relative paths.

Normally, the output of ``cdk-pixbuf-query-loaders`` is written to
``$libdir/cdk-pixbuf-2.0/2.10.0/loaders.cache``, where ``cdk-pixbuf`` looks for
it by default. If it is written to some other location, the environment variable
``GDK_PIXBUF_MODULE_FILE`` can be set to point ``cdk-pixbuf`` at the file.

OPTIONS
-------

``--update-cache``

  Writes the output to the default cache location instead of the standard
  output.

ENVIRONMENT
-----------

The environment variable ``GDK_PIXBUF_MODULEDIR`` can be used to specify a
different loader directory.

The default ``cdk-pixbuf`` loader directory is
``LIBDIR/cdk-pixbuf-2.0/VERSION/loaders``, where:

- ``LIBDIR`` is the libdir used when building ``cdk-pixbuf``
- ``VERSION`` is the ABI version for ``cdk-pixbuf`` loaders
