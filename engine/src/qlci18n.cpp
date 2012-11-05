/*
  Q Light Controller
  qlci18n.cpp

  Copyright (C) Heikki Junnila

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
#ifdef __APPLE__
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
