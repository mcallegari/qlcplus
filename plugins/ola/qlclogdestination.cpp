/*
  Q Light Controller
  qlclogdestination.cpp

  Copyright (c) Heikki Junnila
                Simon Newton

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QDebug>
#include <ola/StringUtils.h>

#include "qlclogdestination.h"

namespace ola {

const string QLCLogDestination::PREFIX = "OLA: ";

void QLCLogDestination::Write(log_level level, const string &log_line) {

    string output = PREFIX;
    output.append(log_line);
    ola::StringTrim(&output);

    switch (level)
    {
    case ola::OLA_LOG_FATAL:
        qCritical() << output.data();
        break;
    case ola::OLA_LOG_WARN:
        qWarning() << output.data();
        break;
    case ola::OLA_LOG_INFO:
    case ola::OLA_LOG_DEBUG:
        qDebug() << output.data();
        break;
    default:
        break;
    }
}

} // ola
