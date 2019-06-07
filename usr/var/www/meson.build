res = run_command('sh', '-c',
                  'cd @0@/usr/var/www/callblocker/js && npm --prefix ./vendor install'.format(meson.source_root())
)
if res.returncode() == 0
  message('npm install...')
else
  error('npm failed with @0@'.format(res.stderr()))
endif

install_subdir(
  'callblocker',
  exclude_files: ['package.json', 'Makefile', 'Makefile.am', 'Makefile.in'],
  install_dir: join_paths(path_localstatedir, 'www')
)
