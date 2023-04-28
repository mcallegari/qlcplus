/*
  Q Light Controller Plus
  palettegenerator.h

  Copyright (C) Massimo Callegari

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

#ifndef PALETTEGENERATOR_H
#define PALETTEGENERATOR_H

#include <QObject>
#include <QString>
#include <QList>
#include <QHash>

#include "scenevalue.h"

class FixtureGroup;
class RGBMatrix;
class Fixture;
class Chaser;
class Scene;
class Doc;

/** @addtogroup ui_functions
 * @{
 */

/**
 * This class can be used on intelligent lights (scanners, moving heads..)
 * to automatically generate functions for their capabilities (color, gobo..)
 * for easy and quick initial setup.
 */
class PaletteGenerator: public QObject
{
    Q_OBJECT

public:
    enum PaletteType
    {
        Undefined = 0,
        PrimaryColors,
        SixteenColors,
        Shutter,
        Gobos,
        ColourMacro,
        Animation
    };

    enum PaletteSubType
    {
        None = 0,
        All,
        OddEven
    };

    /**
     * Create a new PaletteGenerator instance.
     *
     * @param doc The Doc object that takes all generated functions
     * @param fxiList List of fixtures to create functions for
     */
    PaletteGenerator(Doc* doc, const QList <Fixture*>& fxList,
                     PaletteType type = Undefined, PaletteSubType subType = None);

    /** Destructor */
    ~PaletteGenerator();

    /********************************************************************
     * Name
     ********************************************************************/

    /** Set a human readable (and translated) name for this palette */
    void setName(QString name);

    /** Get the palette name */
    QString name();

    /** Get the palette full name, including the fixture model */
    QString fullName();

    /** Get the palette fixture model */
    QString model();

    /********************************************************************
     * Type
     ********************************************************************/

    PaletteType type();

    PaletteSubType subType();

    static QString typetoString(PaletteType type);

    /********************************************************************
     * Contents
     ********************************************************************/
    static QStringList getCapabilities(const Fixture* fixture);

    QList<Scene *> scenes();
    QList<Chaser *> chasers();
    QList<RGBMatrix *> matrices();

    void addToDoc();

private:
    void createColorScene(QList<SceneValue> chMap, QString name, PaletteSubType subType);

    void createRGBCMYScene(QList<SceneValue> rcMap,
                           QList<SceneValue> gmMap,
                           QList<SceneValue> byMap,
                           QString name, bool rgb, PaletteSubType subType);

    void createCapabilityScene(QHash<quint32, quint32> chMap, PaletteSubType subType);

    void createRGBMatrices(QList<SceneValue> rgbMap);

    void createChaser(QString name);

    /**
     * This is the heart of PaletteGenerator. It creates a set of functions
     * based on the fixture list provided, the palette type and the palette subtype.
     * At the end of this function, the PaletteGenerator instance will be filled
     * with Scenes and Chaser ready to be added to a QLC+ project
     *
     */
    void createFunctions(PaletteType type, PaletteSubType subType);

    QString getNamePrefix(QString name);
    QString getNamePrefix(QString type, QString name);

private:
    Doc* m_doc;
    QString m_name;
    PaletteType m_type;
    PaletteSubType m_subType;
    QList <Fixture*> m_fixtures;
    FixtureGroup *m_fixtureGroup;
    QString m_model;
    QList <Scene*> m_scenes;
    QList <Chaser*> m_chasers;
    QList <RGBMatrix*> m_matrices;
};

/** @} */

#endif
