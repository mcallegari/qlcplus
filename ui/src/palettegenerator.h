/*
  Q Light Controller Plus
  palettegenerator.h

  Copyright (C) Massimo Callegari

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

#ifndef PALETTEGENERATOR_H
#define PALETTEGENERATOR_H

#include <QObject>
#include <QString>
#include <QList>
#include <QHash>

class FixtureGroup;
class RGBMatrix;
class Fixture;
class Chaser;
class Scene;
class Doc;

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
    void createColorScene(QHash<quint32, quint32> chMap, QString name, PaletteSubType subType);

    void createRGBCMYScene(QHash<quint32, quint32> rcMap,
                            QHash<quint32, quint32> gmMap,
                            QHash<quint32, quint32> byMap,
                            QString name, bool rgb, PaletteSubType subType);

    void createCapabilityScene(QHash<quint32, quint32> chMap, PaletteSubType subType);

    void createRGBMatrices(QHash<quint32, quint32> rgbMap);

    void createChaser(QString name);

    /**
     * This is the heart of PaletteGenerator. It creates a set of functions
     * based on the fixture list provided, the palette type and the palette subtype.
     * At the end of this function, the PaletteGenerator instance will be filled
     * with Scenes and Chaser ready to be added to a QLC+ project
     *
     */
    void createFunctions(PaletteType type, PaletteSubType subType);

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

#endif

