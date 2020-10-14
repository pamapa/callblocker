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

#include "Logger.h" // API

#include <stdio.h>
#include <string>
#include <syslog.h>

bool Logger::s_useSyslog = false;
LogLevel Logger::s_logLevel = LogLevel::INFO;

void Logger::start(bool useSyslog)
{
    s_useSyslog = useSyslog;
    s_logLevel = LogLevel::INFO;
    if (s_useSyslog) {
        openlog("callblockerd", 0, LOG_USER);
    }
}

void Logger::stop()
{
    if (s_useSyslog) {
        closelog();
    }
}

LogLevel Logger::setLogLevel(LogLevel logLevel)
{
    LogLevel oldLogLevel = s_logLevel;
    s_logLevel = logLevel;
    return oldLogLevel;
}

void Logger::error(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    message(LogLevel::ERROR, format, ap);
    va_end(ap);
}

void Logger::warn(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    message(LogLevel::WARN, format, ap);
    va_end(ap);
}

void Logger::notice(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    message(LogLevel::NOTICE, format, ap);
    va_end(ap);
}

void Logger::info(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    message(LogLevel::INFO, format, ap);
    va_end(ap);
}

void Logger::debug(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    message(LogLevel::DEBUG, format, ap);
    va_end(ap);
}

void Logger::message(LogLevel level, const char* format, va_list ap)
{
    if (level > s_logLevel)
        return;

    va_list ap2;
    va_copy(ap2, ap);
    if (s_useSyslog) {
        int priority;
        switch (level) {
        default:
        case LogLevel::ERROR:
            priority = LOG_ERR;
            break;
        case LogLevel::WARN:
            priority = LOG_WARNING;
            break;
        case LogLevel::NOTICE:
            priority = LOG_NOTICE;
            break;
        case LogLevel::INFO:
            priority = LOG_INFO;
            break;
        case LogLevel::DEBUG:
            priority = LOG_DEBUG;
            break;
        }
        vsyslog(priority, format, ap);
    } else {
        FILE* stream;
        std::string fmt;
        switch (level) {
        default:
        case LogLevel::ERROR:
            stream = stderr;
            fmt = "ERROR:  ";
            break;
        case LogLevel::WARN:
            stream = stderr;
            fmt = "WARN:   ";
            break;
        case LogLevel::NOTICE:
            stream = stdout;
            fmt = "NOTICE: ";
            break;
        case LogLevel::INFO:
            stream = stdout;
            fmt = "INFO:   ";
            break;
        case LogLevel::DEBUG:
            stream = stdout;
            fmt = "DEBUG:  ";
            break;
        }
        fmt += format;
        fmt += "\n";
        (void)vfprintf(stream, fmt.c_str(), ap2);
        (void)fflush(stream);
    }
    va_end(ap2);
}
