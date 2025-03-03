project(
  'callblocker', 'cpp',
  version: '0.15.6',
  license: 'GPL-2.0',
  meson_version: '>= 0.50.0'
)

# grab necessary paths
path_prefix = get_option('prefix')
path_sysconfdir = join_paths(path_prefix, get_option('sysconfdir'))
path_datadir = join_paths(path_prefix, get_option('datadir'))
path_localstatedir = join_paths(path_prefix, get_option('localstatedir'))

add_global_arguments('-DPACKAGE_NAME="@0@"'.format(meson.project_name()), language: 'cpp')
add_global_arguments('-DVERSION="@0@"'.format(meson.project_version()), language: 'cpp')
add_global_arguments('-DSYSCONFDIR="@0@"'.format(path_sysconfdir), language: 'cpp')
add_global_arguments('-DDATADIR="@0@"'.format(path_datadir), language: 'cpp')

cpp = meson.get_compiler('cpp')
dep_json = dependency('json-c')
dep_phonenumber = cpp.find_library('phonenumber')
deps_for_pjproject = [
  cpp.find_library('pthread'),
  cpp.find_library('uuid'),
  dependency('openssl')
]

add_global_arguments('-DPJ_AUTOCONF=1', language : 'cpp')
if dep_phonenumber.found()
  add_global_arguments('-DHAVE_LIBPHONENUMBER=1', language : 'cpp')
endif

subdir('third-party/pjproject')
subdir('src')
subdir('web')
subdir('usr/share')
subdir('usr/var/www')
subdir('etc/callblocker')
subdir('etc/systemd/system')
subdir('tests/callblockerd')
