/*
  Q Light Controller
  qlci18n.h

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

#ifndef QLCI18N_H
#define QLCI18N_H

class QCoreApplication;
class QString;

class QLCi18n
{
public:
    /** Initialize QLCi18n */
    static void init();

    /** Set the default locale to use when loading translations */
    static void setDefaultLocale(const QString& locale);

    /** Get the default locale used when loading translations */
    static QString defaultLocale();

    /** Set the folder path to load translations from */
    static void setTranslationFilePath(const QString& path);

    /** Get the folder path where translation are loaded from */
    static QString translationFilePath();

    /**
     * Load translation for a component. The translation file that this method
     * attempts to load takes the following form: "<component>_<locale>.qm". For
     * example "qlc_fi_FI.qm" will be loaded when locale() == "fi_FI" and
     * $component == "qlc".
     *
     * @param component The name of the component whose translation to load
     * @return true if translation was loaded successfully, otherwise false
     */
    static bool loadTranslation(const QString& component);

private:
    static QString s_defaultLocale;
    static QString s_translationFilePath;
};

#endif
