/*
  Q Light Controller
  qlci18n.h

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

#ifndef QLCI18N_H
#define QLCI18N_H

class QCoreApplication;
class QString;

/** @addtogroup engine Engine
 * @{
 */

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
     * example "qlcplus_fi_FI.qm" will be loaded when locale() == "fi_FI" and
     * $component == "qlcplus".
     *
     * @param component The name of the component whose translation to load
     * @return true if translation was loaded successfully, otherwise false
     */
    static bool loadTranslation(const QString& component);

private:
    static QString s_defaultLocale;
    static QString s_translationFilePath;
};

/** @} */

#endif
