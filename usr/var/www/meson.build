if get_option('web-interface')
  message('build web-interface...')

  # fetch js/vendor files
  path_js = join_paths(meson.source_root(), 'usr/var/www/callblocker/js')
  res = run_command('sh', '-c',
                    'cd @0@/usr/var/www/callblocker/js && npm --prefix ./vendor install'.format(meson.source_root())
  )
  if res.returncode() == 0
    message('npm install...')
  else
    error('npm failed with @0@'.format(res.stderr()))
  endif

  # patch version in js file
  app_js = join_paths(path_js, 'app.js')
  message('set version @0@ in @1@'.format(meson.project_version(), app_js))
  run_command('sh', '-c',
              'sed -i \'s/content: "v.*"/content: "v@0@"/g\' @1@'.format(meson.project_version(), app_js))

  install_subdir(
    'callblocker',
    exclude_files: ['package.json', 'package-lock.json'],
    install_dir: join_paths(path_localstatedir, 'www')
  )
endif
