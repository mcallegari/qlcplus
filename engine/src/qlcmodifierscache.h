/*
  Q Light Controller Plus
  qlcmodifierscache.h

  Copyright (c) Massimo Callegari

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

#ifndef QLCMODIFIERSCACHE_H
#define QLCMODIFIERSCACHE_H

#include <QHash>
#include <QDir>

class ChannelModifier;

/** @addtogroup engine Engine
 * @{
 */

class QLCModifiersCache
{
public:
    QLCModifiersCache();

    /**
     * Add a channel modifier to the modifiers map.
     *
     * @param fixtureDef The fixture definition to add
     * @return true, if $modifier was added, otherwise false
     */
    bool addModifier(ChannelModifier* modifier);

    /**
     * Return a list of strings containing the cached modifiers
     * template names.
     */
    QList<QString> templateNames();

    /**
     * Get a modifier instance by name
     * @param name The modifier name
     * @return a pointer to the requested modifier or NULL if not found
     */
    ChannelModifier* modifier(QString name);

    /**
     * Get the default system channels modifiers directory that contains
     * installed modifiers templates. The location varies greatly between
     * platforms.
     *
     * @return System channels modifiers templates directory
     */
    static QDir systemTemplateDirectory();

    /**
     * Get the user's own default channels modifiers directory that is used to
     * save custom modifiers templates. The location varies greatly between
     * platforms.
     *
     * @return User channels modifiers templates directory
     */
    static QDir userTemplateDirectory();

    /**
     * Load channel modifiers templates from the given path. Ignores duplicates.
     * Returns true even if $dir doesn't contain any template,
     * if it is still accessible (and exists).
     *
     * @param dir The directory to load templates from.
     * @return true, if the path could be accessed, otherwise false.
     */
    bool load(const QDir& dir, bool systemTemplates = false);

private:
    QHash <QString, ChannelModifier*> m_modifiers;
};

/** @} */

#endif // QLCMODIFIERSCACHE_H
