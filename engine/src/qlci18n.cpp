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
#include <QDir>
#include <QLibraryInfo>

#include "qlcconfig.h"
#include "qlcfile.h"
#include "qlci18n.h"

QString QLCi18n::s_defaultLocale = QString();
QString QLCi18n::s_translationFilePath = QString();

void QLCi18n::init()
{
    // Set the default translation file path before parsing args
    QLCi18n::setTranslationFilePath(QLCFile::systemDirectory(TRANSLATIONDIR).absolutePath());
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
    const QString lc = defaultLocale().isEmpty() ? QLocale::system().name() : defaultLocale();
    const QString file(QString("%1_%2").arg(component).arg(lc));
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

bool QLCi18n::loadQtTranslation(const QString& component)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const QString qtTranslationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#else
    const QString qtTranslationsPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#endif

    const QString lc = defaultLocale().isEmpty() ? QLocale::system().name() : defaultLocale();
    const QString file(QString("%1_%2").arg(component).arg(lc));
    QTranslator* qtTranslator = new QTranslator(QCoreApplication::instance());
    if (qtTranslator->load(file, qtTranslationsPath))
        return QCoreApplication::installTranslator(qtTranslator);
    else
        return false;
}
