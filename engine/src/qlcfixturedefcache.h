/*
  Q Light Controller
  qlcfixturedefcache.h

  Copyright (c) Heikki Junnila

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

#ifndef QLCFIXTUREDEFCACHE_H
#define QLCFIXTUREDEFCACHE_H

#include <QStringList>
#include <QString>
#include <QMap>
#include <QDir>

class QXmlStreamReader;
class QLCFixtureDef;

/** @addtogroup engine Engine
 * @{
 */

/**
 * QLCFixtureDefCache is a cache of fixture definitions that are currently
 * available to the application. Application can get a list of available
 * manufacturer names with QLCFixturedefCache::manufacturers() and subsequently
 * all models for a particular manufacturer with QLCFixtureDefCache::models().
 *
 * The internal structure is a two-tier map (m_models), with the first tier
 * containing manufacturer names as the keys for the first map. The value of
 * each key is another map (the second-tier) whose keys are model names. The
 * value for each model name entry in the second-tier map is the actual
 * QLCFixtureDef instance.
 *
 * Multiple manufacturer & model combinations are discarded.
 *
 * Because this component is meant to be used only on the application side,
 * the returned fixture definitions are const, preventing any modifications to
 * the definitions. Modifying the definitions would also screw up the mapping
 * since they are made only during addFixtureDef() based on the definitions'
 * manufacturer() & model() data.
 */
class QLCFixtureDefCache
{
public:
    /**
     * Create a new fixture definition cache instance.
     */
    QLCFixtureDefCache();

    /**
     * Destroy a fixture definition cache instance.
     */
    ~QLCFixtureDefCache();

    /**
     * Get a fixture definition by its manufacturer and model. Only
     * const methods can be accessed for returned fixture definitions.
     *
     * @param manufacturer The fixture definition's manufacturer
     * @param model The fixture definition's model
     * @return A matching fixture definition or NULL if not found
     */
    QLCFixtureDef* fixtureDef(const QString& manufacturer,
                              const QString& model) const;

    /**
     * Get a list of available manufacturer names.
     */
    QStringList manufacturers() const;

    /**
     * Get a list of available model names for the given manufacturer.
     */
    QStringList models(const QString& manufacturer) const;

    /** Get a complete map of the available fixtures as:
      * manufacturer, <model, isUser>
      */
    QMap<QString, QMap<QString, bool> > fixtureCache() const;

    /**
     * Add a fixture definition to the model map.
     *
     * @param fixtureDef The fixture definition to add
     * @return true, if $fixtureDef was added, otherwise false
     */
    bool addFixtureDef(QLCFixtureDef *fixtureDef);

    /**
     * Store a fixture in the fixtures user data folder
     * if a fixture with the same name already exists, it
     * will be overwritten
     *
     * @param filename the target fixture file name
     * @param data the content of a fixture XML data
     * @return
     */
    bool storeFixtureDef(QString filename, QString data);

    /**
     * Realod from file a definition with the provided reference
     *
     * @param fixtureDef The fixture definition to remove
     * @return true, if $fixtureDef was found and removed, otherwise false
     */
    bool reloadFixtureDef(QLCFixtureDef *fixtureDef);

    /**
     * Load fixture definitions from the given path. Ignores duplicates.
     * Returns true even if $fixturePath doesn't contain any fixtures,
     * if it is still accessible (and exists).
     *
     * @param dir The directory to load definitions from.
     * @return true, if the path could be accessed, otherwise false.
     */
    bool load(const QDir& dir);

    /**
     * Load all the fixture information found for the given manufacturer.
     *
     * @param doc reference to the XML loader
     * @param manufacturer used to elapse the fixture file name relative path
     * @return the number of fixtures found
     */
    int loadMapManufacturer(QXmlStreamReader *doc, QString manufacturer);

    /**
     * Load a map of hardcoded fixture definitions that represent
     * the minimum information to cache a fixture when it is required
     *
     * @param dir The directory to load definitions from.
     * @return true, if the path could be accessed, otherwise false.
     */
    bool loadMap(const QDir& dir);

    /**
     * Cleans the contents of the fixture definition cache, deleting
     * all fixture definitions.
     */
    void clear();

    /**
     * Get the default system fixture definition directory that contains
     * installed fixture definitions. The location varies greatly between
     * platforms.
     *
     * @return System fixture definition directory
     */
    static QDir systemDefinitionDirectory();

    /**
     * Get the user's own default fixture definition directory that is used to
     * save custom fixture definitions. The location varies greatly between
     * platforms.
     *
     * @return User fixture definition directory
     */
    static QDir userDefinitionDirectory();

    /** Load a QLC native fixture definition from the file specified in $path */
    bool loadQXF(const QString& path, bool isUser = false);

    /** Load an Avolites D4 fixture definition from the file specified in $path */
    bool loadD4(const QString& path);

private:
    QString m_mapAbsolutePath;
    QList <QLCFixtureDef*> m_defs;
};

/** @} */

#endif
