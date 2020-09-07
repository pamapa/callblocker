if get_option('web-interface')
  if get_option('web-interface-build')
    message('build web-interface...')
    path_package_json = join_paths(meson.source_root(), 'usr/var/www/callblocker')
    # fetch js files
    res = run_command('sh', '-c',
                      'cd @0@ && npm install'.format(path_package_json)
    )
    if res.returncode() != 0
      error('npm install failed with @0@'.format(res.stderr()))
    endif
    # patch version in js file
    app_js = join_paths(path_package_json, 'src/app.js')
    message('set version @0@ in @1@'.format(meson.project_version(), app_js))
    run_command('sh', '-c',
                'sed -i \'s/appVersion = "v.*"/appVersion = "v@0@"/g\' @1@'.format(meson.project_version(), app_js))
    # build bundle
    res = run_command('sh', '-c',
                      'cd @0@ && npm run build'.format(path_package_json)
    )
    if res.returncode() != 0
      error('npm run build failed with @0@'.format(res.stderr()))
    endif
endif

  install_subdir(
    'callblocker',
    exclude_files: ['package.json', 'package-lock.json', 'webpack.config.js'],
    exclude_directories: ['node_modules', 'src'],
    install_dir: join_paths(path_localstatedir, 'www')
  )
endif
