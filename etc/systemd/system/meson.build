# else: debhelper/dh_systemd takes care (see debian/callblocker.service)
if get_option('install-custom')
  install_data(
    ['callblockerd.service'],
    install_dir: join_paths(path_sysconfdir, 'systemd', 'system')
  )
endif

