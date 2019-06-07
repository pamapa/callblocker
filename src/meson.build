libcallblockerd_sources = [
  'Logger.cpp',
  'Notify.cpp',
  'TimerUtil.cpp',
  'Utils.cpp',
  'FileList.cpp',
  'FileListsNotified.cpp',
  'FileListsCache.cpp',
  'Settings.cpp',
  'Block.cpp',
  'Phone.cpp',
  'SipPhone.cpp',
  'SipAccount.cpp',
  'AnalogPhone.cpp',
  'Modem.cpp'
]

libcallblockerd = shared_library('libcallblockerd',
                                 libcallblockerd_sources,
                                 dependencies: [dep_json, dep_pj, dep_pjsip, dep_pjsua, dep_phonenumber])

callblockerd = executable('callblockerd', 'Main.cpp',
                          link_with : libcallblockerd,
                          install: true)
