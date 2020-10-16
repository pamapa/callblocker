if get_option('web-interface') and get_option('web-interface-build')
  message('build web-interface...')

  path_src_web = join_paths(meson.source_root(), 'src-web')

  # patch version in app.js file
  app_js = join_paths(path_src_web, 'src/app.js')
  message('set version @0@ in @1@'.format(meson.project_version(), app_js))
  run_command('sh', '-c',
              'sed -i \'s/appVersion = "v.*"/appVersion = "v@0@"/g\' @1@'.format(meson.project_version(), app_js))

  # build
  res = run_command('sh', '-c',
    'cd @0@ && ./prepare.sh'.format(path_src_web),
  )
  if res.returncode() != 0
    error('@0@ prepare.sh failed with @1@'.format(path_src_web, res.stderr()))
  endif
endif
