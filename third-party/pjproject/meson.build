# Debian will no longer package pjproject
# see https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=925263
#dep_pj = cc.find_library('libpj')
#dep_pjsip = cc.find_library('pjsip')
#dep_pjsua = cc.find_library('pjsua')

message('pjproject: download and build locally...')
run_command('sh', '-c',
  'cd @0@/third-party/pjproject && ./prepare.sh'.format(meson.source_root())
)

inc_pj = include_directories(
  'pjproject-2.8/pjlib/include',
  'pjproject-2.8/pjlib-util/include',
  'pjproject-2.8/pjsip/include',
  'pjproject-2.8/pjmedia/include',
  'pjproject-2.8/pjnath/include'
)

pj_path = join_paths(meson.source_root(), 'third-party/pjproject/pjproject-2.8')

# similar as here: https://github.com/endlessm/systemd/blob/master/meson.build
# on armhf: 'armv7l-unknown-linux-gnueabihf' or 'armv6l-unknown-linux-gnueabihf'
# on amd64: 'x86_64-unknown-linux-gnu'
dpkg_architecture = find_program('dpkg-architecture')
deb_host_gnu_system = run_command(dpkg_architecture, '-qDEB_HOST_GNU_SYSTEM').stdout().strip()
pj_target_name = host_machine.cpu_family() + '-unknown-' + deb_host_gnu_system

dep_pjproject = declare_dependency(
  include_directories: inc_pj,
  dependencies: deps_for_pjproject,
  link_args: [
    '@0@/pjlib/lib/libpj-@1@.a'.format(pj_path, pj_target_name),
    '@0@/pjsip/lib/libpjsip-simple-@1@.a'.format(pj_path, pj_target_name),
    '@0@/pjlib-util/lib/libpjlib-util-@1@.a'.format(pj_path, pj_target_name),
    '@0@/pjsip/lib/libpjsip-@1@.a'.format(pj_path, pj_target_name),
    '@0@/pjsip/lib/libpjsip-ua-@1@.a'.format(pj_path, pj_target_name),
    '@0@/pjsip/lib/libpjsua-@1@.a'.format(pj_path, pj_target_name),
    '@0@/pjmedia/lib/libpjmedia-@1@.a'.format(pj_path, pj_target_name),
    '@0@/pjmedia/lib/libpjmedia-audiodev-@1@.a'.format(pj_path, pj_target_name),
    '@0@/pjmedia/lib/libpjmedia-codec-@1@.a'.format(pj_path, pj_target_name),
    '@0@/pjnath/lib/libpjnath-@1@.a'.format(pj_path, pj_target_name),
    '@0@/third_party/lib/libsrtp-@1@.a'.format(pj_path, pj_target_name)
  ]
)
