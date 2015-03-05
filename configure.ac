dnl Process this file with autoconf to produce a configure script.
dnl
dnl callblocker - blocking unwanted calls from your home phone
dnl Copyright (C) 2015-2015 Patrick Ammann <pammann@gmx.net>
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
AC_INIT([callblocker], [0.1], [pammann@gmx.net])

AM_INIT_AUTOMAKE()

dnl Checks for programs.
AC_PROG_CXX

dnl Check for Libraries.
AC_CHECK_HEADERS([pjlib.h])
AC_CHECK_HEADERS([pjsua-lib/pjsua.h])
AC_CHECK_LIB([pj], pj_init)
AC_CHECK_LIB([pjsip], pjsip_parse_uri)
AC_CHECK_LIB([pjsua], pjsua_init)
AC_CHECK_HEADERS([json-c/json.h])
AC_CHECK_LIB([json-c], json_tokener_parse)

AC_CONFIG_FILES([Makefile src/callblockerd/Makefile])
AC_OUTPUT

