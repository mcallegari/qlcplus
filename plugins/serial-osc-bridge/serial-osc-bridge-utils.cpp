/*
  Q Light Controller Plus

  serial-osc-bridge-utils.cpp

  Copyright (c) House Gordon Software Company LTD

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
#include <QHostAddress>
#include <QLocale>
#include <QFileInfo>
#include <QRegularExpression>

#include "serial-osc-bridge-utils.h"

QString int_to_hex(unsigned int value, int width)
{
  return QString("%1").arg(value, width, 16, QLatin1Char( '0' ));
}

QString int_to_str(unsigned int value)
{
  return QString("%1").arg(value);
}

QString int_to_str_group_sep(unsigned int value)
{
  QLocale locale = QLocale::system();
  QString valueText = locale.toString(value);
  return valueText;
}

QString int_to_data_human_sizes(unsigned int value)
{
  QLocale locale = QLocale::system();
  QString valueText = locale.formattedDataSize(value);
  return valueText;
}


bool valid_host_address(const QString& s)
{
  QHostAddress newHostAddress(s);
  return !s.isEmpty() && !newHostAddress.isNull();
}

bool valid_baudrate(int i)
{
  switch (i)
    {
    case 1200:
    case 2400:
    case 4800:
    case 9600:
    case 19200:
    case 38400:
    case 57600:
    case 115200:
      return true;
    default:
      return false;
    }
}


bool valid_filename(QString s)
{
    qDebug() << "=== Checking filename validity for " << s;

    // Allow very limited subset of characters to avoid any funkiness.
    // This is good enough for Linux/MacOS, but won't work for Windows...
    // Also check that all components (except the last part)
    // are part of an existing directory.
    if (s.isEmpty()) {
      qDebug() << "=== FALSE: filename is empty";
      return false;
    }

    QFileInfo qfi(s);

    if (!qfi.isAbsolute()) {
      qDebug() << "=== FALSE: filename is NOT absolute";
      return false;
    }

    QString bn = qfi.fileName();
    qDebug() << "=== basename: " << bn;
    bool bad_bn = bn.contains(QRegularExpression(QStringLiteral("[^-0-9A-Za-z_.]")));
    if (bad_bn) {
      qDebug() << "=== FALSE: basename " << bn << " contains forbidden characters";
      return false;
    }

    // Remove the basename, then ensure the rest of the path exists.
    QString p(s);
    p.chop(bn.size());

    //Create a QFileInfo for the parent directory.
    //and convert to canonical - if the path doesn't exist, QT's "canonicalFilePath"
    //will return empty string.
    QFileInfo dir(p);
    QString dirname = dir.canonicalFilePath();
    qDebug() << "=== Canonize: before = " << p << " after = " << dirname;
    if (dirname.isEmpty()) {
      qDebug() << "=== FALSE: parent directory " << p << " does not exist";
      return false;
    }

    qDebug() << "=== TRUE" ;
    return true;
}
