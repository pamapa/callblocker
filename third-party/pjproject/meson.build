# similar as here: https://github.com/endlessm/systemd/blob/master/meson.build
# on armhf: 'armv7l-unknown-linux-gnueabihf' or 'armv6l-unknown-linux-gnueabihf'
# on amd64: 'x86_64-unknown-linux-gnu'
dpkg_architecture = find_program('dpkg-architecture')
deb_host_gnu_system = run_command(dpkg_architecture, '-qDEB_HOST_GNU_SYSTEM').stdout().strip()
pj_target_name = host_machine.cpu() + '-unknown-' + deb_host_gnu_system

if get_option('pjproject-build')
  message('pjproject: download and build locally...')

  env_pj = environment()
  if meson.is_cross_build()
    message('pjproject: cross build for @0@'.format(pj_target_name))
    if host_machine.cpu_family() == 'arm'
      env_pj.set('PJPROJECT_CONFIGURE_OPTS', '--host=arm-linux-gnueabihf --target=@0@'.format(pj_target_name))
      env_pj.set('CROSS_COMPILE', 'arm-linux-gnueabihf-')
    else
      error('cross build to this cpu family not supported')
    endif
  endif

  res = run_command('sh', '-c',
    'cd @0@/third-party/pjproject && ./prepare.sh'.format(meson.source_root()),
    env: env_pj
  )
  if res.returncode() != 0
    error('third-party/pjproject prepare.sh failed with @0@'.format(res.stderr()))
  endif
endif

unpack_dir_pj = 'pjproject-2.11.1'
inc_pj = include_directories(
  '@0@/pjlib/include'.format(unpack_dir_pj),
  '@0@/pjlib-util/include'.format(unpack_dir_pj),
  '@0@/pjsip/include'.format(unpack_dir_pj),
  '@0@/pjmedia/include'.format(unpack_dir_pj),
  '@0@/pjnath/include'.format(unpack_dir_pj)
)

pj_path = join_paths(meson.source_root(), 'third-party/pjproject', unpack_dir_pj)

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
    '@0@/third_party/lib/libsrtp-@1@.a'.format(pj_path, pj_target_name),
    '@0@/third_party/lib/libresample-@1@.a'.format(pj_path, pj_target_name)
  ]
)
