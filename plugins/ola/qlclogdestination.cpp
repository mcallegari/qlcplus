/*
  Q Light Controller
  qlclogdestination.cpp

  Copyright (c) Heikki Junnila
                Simon Newton

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QDebug>
#include <ola/StringUtils.h>

#include "qlclogdestination.h"

namespace ola {

using std::string;

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
