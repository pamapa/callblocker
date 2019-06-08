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

libcallblockerd = static_library(
  'callblockerd',
  libcallblockerd_sources,
  dependencies: [dep_json, dep_phonenumber, dep_pjproject]
)

callblockerd = executable(
  'callblockerd', 'Main.cpp',
  link_with : libcallblockerd,
  dependencies: [dep_json, dep_phonenumber, dep_pjproject],
  install: true
)
