/*
  Q Light Controller Plus
  rgbscriptscache.h

  Copyright (c) David Garyga

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

#ifndef RGBSCRIPTSCACHE_H
#define RGBSCRIPTSCACHE_H

#include <QMap>

class RGBScript;
class QDir;
class Doc;

/** @addtogroup engine Engine
 * @{
 */

class RGBScriptsCache
{
public:
    explicit RGBScriptsCache(Doc* doc);

    /**
     * Return a list of strings containing the cached scripts names.
     */
    QStringList names() const;

    /**
     * Get a script instance by name
     */
    RGBScript const& script(QString name) const;

    /**
     * Load RGB scripts from the given path. Ignores duplicates.
     * Returns true even if $dir doesn't contain any script,
     * if it is still accessible (and exists).
     *
     * @param dir The directory to load scripts from.
     * @return true, if the path could be accessed, otherwise false.
     */
    bool load(const QDir& dir);

    /**
     * Get the default system RGB scripts directory that contains
     * installed RGB scripts. The location varies greatly between
     * platforms.
     *
     * @return System RGB scripts directory
     */
    static QDir systemScriptsDirectory();

    /**
     * Get the user's own default RGB scripts directory that is used to
     * save custom RGB scripts. The location varies greatly between
     * platforms.
     *
     * @return User RGB scripts directory
     */
    static QDir userScriptsDirectory();

private:
    Doc* m_doc;
    QMap<QString, RGBScript*> m_scriptsMap; //! One instance of each script, filename-based map
    RGBScript* m_dummyScript; //! Dummy empty script
};

/** @} */

#endif // RGBSCRIPTSCACHE_H
