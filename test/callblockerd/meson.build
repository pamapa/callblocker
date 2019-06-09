unit_tests_sources = [
  'Main.cpp',
  'Test_Utils.cpp',
  'Test_Block.cpp',
  'Test_FileListsCache.cpp',
  'Test_FileListsNotified.cpp'
]

incdir = include_directories(
  '../../src'
)
unit_tests = executable(
  'unit_tests',
  unit_tests_sources,
  include_directories: incdir,
  link_with : libcallblockerd
)
test(
  'unit_tests',
  unit_tests,
  args: [meson.source_root()]
)

