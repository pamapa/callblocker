/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2015-2015 Patrick Ammann <pammann@gmx.net>

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

#include <string>
#include <stdio.h>
#include <syslog.h>


#define USE_SYSLOG                  1


static unsigned int s_logLevel = LOG_INFO;


void Logger::start() {
#if USE_SYSLOG
  openlog("callblockerd", 0, LOG_USER);
#endif
}

void Logger::stop() {
#if USE_SYSLOG
  closelog();
#endif
}

void Logger::setLogLevel(std::string level) {
  if (level == "debug") s_logLevel = LOG_DEBUG;
  else if (level == "info") s_logLevel = LOG_INFO;
  else if (level == "notice") s_logLevel = LOG_NOTICE;
  else if (level == "warn") s_logLevel = LOG_WARNING;
  else if (level == "error")s_logLevel = LOG_ERR;
  else {
    s_logLevel = LOG_INFO;
    Logger::warn("invalid settings file: unknown log_level %s", level.c_str());
  }
}

void Logger::error(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  message(LOG_ERR, format, ap);
  va_end(ap);
}

void Logger::warn(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  message(LOG_WARNING, format, ap);
  va_end(ap);
}

void Logger::notice(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  message(LOG_NOTICE, format, ap);
  va_end(ap);
}

void Logger::info(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  message(LOG_INFO, format, ap);
  va_end(ap);
}

void Logger::debug(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  message(LOG_DEBUG, format, ap);
  va_end(ap);
}

void Logger::message(int priority, const char* format, va_list ap) {
  if (priority > s_logLevel) return;

  va_list ap2;
  va_copy(ap2, ap);
#if USE_SYSLOG
  vsyslog(priority, format, ap);
#else
  FILE* stream = stdout;
  std::string fmt;
  switch (priority) {
    default:
    case LOG_ERR:     stream = stderr; fmt = "Error: ";  break;
    case LOG_WARNING: stream = stderr; fmt = "Warn: ";   break;
    case LOG_NOTICE:  stream = stdout; fmt = "Notice: "; break;
    case LOG_INFO:    stream = stdout; fmt = "Info: ";   break;
    case LOG_DEBUG:   stream = stdout; fmt = "Debug: ";  break;
  }
  fmt += format;
  fmt += "\n";
  (void)vfprintf(stream, fmt.c_str(), ap2);
  (void)fflush(stream);
#endif
  va_end(ap2);
}

