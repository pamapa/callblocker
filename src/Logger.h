/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2015-2020 Patrick Ammann <pammann@gmx.net>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>
#include <string>

enum class LogLevel {
    ERROR = 3,
    WARN,
    NOTICE,
    INFO,
    DEBUG,
};

class Logger {
public:
    static void start(bool useSyslog);
    static void stop();
    static LogLevel setLogLevel(LogLevel logLevel);

    static void error(const char* format, ...);
    static void warn(const char* format, ...);
    static void notice(const char* format, ...);
    static void info(const char* format, ...);
    static void debug(const char* format, ...);

private:
    static bool s_useSyslog;
    static LogLevel s_logLevel;

    static void message(LogLevel level, const char* format, va_list ap);
};

#endif
