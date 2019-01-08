dnl Process this file with autoconf to produce a configure script.
dnl
dnl callblocker - blocking unwanted calls from your home phone
dnl Copyright (C) 2015-2019 Patrick Ammann <pammann@gmx.net>
dnl
dnl This program is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU General Public License
dnl as published by the Free Software Foundation; either version 3
dnl of the License, or (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

AC_PREREQ(2.60)
AC_INIT([callblocker], [0.12.0], [pammann@gmx.net])
AM_INIT_AUTOMAKE()

dnl Add support for libraries
AC_PROG_RANLIB

dnl Checks for programs.
AC_PROG_CXX

dnl Compiler defines
CPPFLAGS="$CPPFLAGS -std=c++11 -DPJ_AUTOCONF=1"

dnl Check for Libraries.
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([pjlib.h])
AC_CHECK_HEADERS([pjsua-lib/pjsua.h])
AC_CHECK_LIB([pj], pj_init, , [AC_MSG_ERROR("Linking against pj failed.")])
AC_CHECK_LIB([pjsip], pjsip_parse_uri, , [AC_MSG_ERROR("Linking against pjsip failed.")])
AC_CHECK_LIB([pjmedia], pjmedia_null_port_create, , [AC_MSG_ERROR("Linking against pjmedia failed.")])
AC_CHECK_LIB([pjsua], pjsua_init, , [AC_MSG_ERROR("Linking against pjsua failed.")])
AC_CHECK_HEADERS([json-c/json.h])
AC_CHECK_LIB([json-c], json_tokener_parse, , [AC_MSG_ERROR("Linking against json-c failed.")])
AC_CHECK_HEADERS([phonenumbers/phonenumberutil.h])
AC_CHECK_LIB([phonenumber], main, , [AC_MSG_ERROR("Linking against phonenumber failed.")])
AC_LANG_POP([C++])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([etc/Makefile])
AC_CONFIG_FILES([etc/callblocker/Makefile])
AC_CONFIG_FILES([etc/callblocker/cache/Makefile])
AC_CONFIG_FILES([etc/callblocker/blacklists/Makefile])
AC_CONFIG_FILES([etc/callblocker/whitelists/Makefile])
AC_CONFIG_FILES([etc/systemd/system/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([usr/Makefile])
AC_CONFIG_FILES([usr/share/callblocker/Makefile])
AC_CONFIG_FILES([usr/var/www/callblocker/Makefile])
AC_CONFIG_FILES([usr/var/www/callblocker/js/Makefile])
AC_CONFIG_FILES([usr/var/www/callblocker/python-fcgi/Makefile])
dnl Tests
AC_CONFIG_FILES([test/callblockerd/Makefile])
AC_OUTPUT
