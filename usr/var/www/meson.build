run_command('sh', '-c',
  'cd @0@/usr/var/www/callblocker/js && npm --prefix ./vendor install'.format(meson.source_root())
)

install_subdir(
  'callblocker',
  exclude_files: ['package.json', 'Makefile', 'Makefile.am', 'Makefile.in'],
  install_dir: join_paths(path_localstatedir, 'www')
)
