install_data(['tpl_settings.json', 'README.md'],
             install_mode: ['rw-r--r--', 'www-data', 'www-data'],
             install_dir: join_paths(path_sysconfdir, 'callblocker'))

# create directories and install README.md
install_subdir('blacklists',
               install_mode: ['rw-r--r--', 'www-data', 'www-data'],
               install_dir: join_paths(path_sysconfdir, 'callblocker'))
install_subdir('whitelists',
               install_mode: ['rw-r--r--', 'www-data', 'www-data'],
               install_dir: join_paths(path_sysconfdir, 'callblocker'))
install_subdir('cache',
               install_mode: ['rw-r--r--', 'www-data', 'www-data'],
               install_dir: join_paths(path_sysconfdir, 'callblocker'))
