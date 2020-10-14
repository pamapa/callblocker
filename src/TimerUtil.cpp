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

#include "TimerUtil.h" // API

#include <time.h>

TimerUtil::TimerUtil()
{
    m_active = false;
    timerclear(&elapseTime);
}

void TimerUtil::restart(time_t elapseSec)
{
    struct timeval add;
    timerclear(&add);
    add.tv_sec = elapseSec;
    getCurrent(&elapseTime);
    timeradd(&elapseTime, &add, &elapseTime);
    m_active = true;
}

void TimerUtil::stop(void)
{
    m_active = false;
    timerclear(&elapseTime);
}

bool TimerUtil::isActive()
{
    return m_active;
}

bool TimerUtil::hasElapsed()
{
    struct timeval now;
    getCurrent(&now);
    return timercmp(&elapseTime, &now, <=) ? true : false;
}

void TimerUtil::getCurrent(struct timeval* res)
{
    struct timespec tp;
    (void)clock_gettime(CLOCK_MONOTONIC, &tp);
    res->tv_sec = tp.tv_sec;
    res->tv_usec = tp.tv_nsec / 1000;
}
