gidocgen_dep = dependency('gi-docgen',
  version: '>= 2021.1',
  required: get_option('documentation'),
)

toml_conf = configuration_data()
toml_conf.set('VERSION', meson.project_version())
pixbuf_toml = configure_file(input: 'cdk-pixbuf.toml.in', output: 'cdk-pixbuf.toml', configuration: toml_conf)
pixdata_toml = configure_file(input: 'cdk-pixdata.toml.in', output: 'cdk-pixdata.toml', configuration: toml_conf)

gidocgen = find_program('gi-docgen', required: get_option('documentation'))

expand_content_md_files = [
  'scaling-compositing.md',
]

build_docs = get_option('documentation')

if build_docs and not build_gir
  error('API reference requires introspection.')
endif

if build_docs
  custom_target('cdk-pixbuf-doc',
    input: [ pixbuf_toml, cdkpixbuf_gir[0] ],
    output: 'cdk-pixbuf',
    command: [
      gidocgen,
      'generate',
      '--quiet',
      '--add-include-path', meson.current_build_dir() / '../cdk-pixbuf',
      '--config=@INPUT0@',
      '--output-dir=@OUTPUT@',
      '--no-namespace-dir',
      '--content-dir', meson.current_source_dir(),
      '@INPUT1@',
    ],
    depend_files: [ expand_content_md_files ],
    build_by_default: true,
    install: true,
    install_dir: cdk_pixbuf_docsdir,
  )

  custom_target('cdk-pixdata-doc',
    input: [ pixdata_toml, cdkpixdata_gir[0] ],
    output: 'cdk-pixdata',
    command: [
      gidocgen,
      'generate',
      '--quiet',
      '--add-include-path', meson.current_build_dir() / '../cdk-pixbuf',
      '--config=@INPUT0@',
      '--output-dir=@OUTPUT@',
      '--no-namespace-dir',
      '--content-dir', meson.current_source_dir(),
      '@INPUT1@',
    ],
    build_by_default: true,
    install: true,
    install_dir: cdk_pixbuf_docsdir,
  )
endif

rst2man = find_program('rst2man', 'rst2man.py', required: false)
if get_option('man') and not rst2man.found()
  error('No rst2man found, but man pages were explicitly enabled')
endif

if get_option('man') and rst2man.found()
  rst_files = [
    [ 'cdk-pixbuf-csource', ],
    [ 'cdk-pixbuf-query-loaders', ],
  ]

  rst2man_flags = [
    '--syntax-highlight=none',
  ]

  foreach rst: rst_files
    man_name = rst[0]
    man_section = rst.get(1, '1')

    custom_target('man-@0@'.format(man_name),
      input: '@0@.rst'.format(man_name),
      output: '@0@.@1@'.format(man_name, man_section),
      command: [
        rst2man,
        rst2man_flags,
        '@INPUT@',
      ],
      capture: true,
      install: true,
      install_dir: get_option('mandir') / 'man@0@'.format(man_section),
    )
  endforeach
endif
