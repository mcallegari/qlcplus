/*
  Q Light Controller
  qlci18n.cpp

  Copyright (C) Heikki Junnila

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

#include <QCoreApplication>
#include <QTranslator>
#include <QLocale>
#include <QString>
#include <QDebug>

#include "qlcconfig.h"
#include "qlci18n.h"

QString QLCi18n::s_defaultLocale = QString();
QString QLCi18n::s_translationFilePath = QString();

void QLCi18n::init()
{
#if defined(__APPLE__) || defined(Q_OS_MAC)
    // Set the default translation file path before parsing args
    QLCi18n::setTranslationFilePath(QString("%1/../%2")
                                    .arg(QCoreApplication::applicationDirPath())
                                    .arg(TRANSLATIONDIR));
#else
    // Set the default translation file path before parsing args
    QLCi18n::setTranslationFilePath(TRANSLATIONDIR);
#endif
}

void QLCi18n::setDefaultLocale(const QString& locale)
{
    s_defaultLocale = locale;
}

QString QLCi18n::defaultLocale()
{
    return s_defaultLocale;
}

void QLCi18n::setTranslationFilePath(const QString& path)
{
    s_translationFilePath = path;
}

QString QLCi18n::translationFilePath()
{
    return s_translationFilePath;
}

bool QLCi18n::loadTranslation(const QString& component)
{
    QString lc;

    if (defaultLocale().isEmpty() == true)
        lc = QLocale::system().name();
    else
        lc = defaultLocale();

    QString file(QString("%1_%2").arg(component).arg(lc));
    QTranslator* translator = new QTranslator(QCoreApplication::instance());
    if (translator->load(file, translationFilePath()) == true)
    {
        QCoreApplication::installTranslator(translator);
        return true;
    }
    else
    {
        return false;
    }
}
