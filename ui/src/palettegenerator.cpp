/*
  Q Light Controller Plus
  palettegenerator.cpp

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

#include <QString>
#include <QDebug>

#include "qlccapability.h"
#include "qlcfixturedef.h"
#include "qlcchannel.h"

#include "palettegenerator.h"
#include "fixturegroup.h"
#include "chaserstep.h"
#include "rgbmatrix.h"
#include "rgbscriptscache.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

PaletteGenerator::PaletteGenerator(Doc* doc, const QList <Fixture*>& fxList,
                                   PaletteType type, PaletteSubType subType)
        : m_doc(doc)
        , m_name(QString())
        , m_type(type)
        , m_subType(subType)
        , m_fixtures(fxList)
        , m_fixtureGroup(NULL)
{
    if (m_fixtures.count() > 0)
    {
        m_name = typetoString(type);
        if (m_fixtures.at(0)->fixtureDef() != NULL)
            m_model = m_fixtures.at(0)->fixtureDef()->model();
        if (type != Undefined)
            createFunctions(type, subType);
    }
}

PaletteGenerator::~PaletteGenerator()
{
    m_fixtures.clear();
    m_scenes.clear();
    m_chasers.clear();
    m_matrices.clear();
}

void PaletteGenerator::setName(QString name)
{
    m_name = name;
}

QString PaletteGenerator::name()
{
    return m_name;
}

QString PaletteGenerator::fullName()
{
    return m_name + " - " + m_model;
}

QString PaletteGenerator::model()
{
    return m_model;
}

PaletteGenerator::PaletteType PaletteGenerator::type()
{
    return m_type;
}

PaletteGenerator::PaletteSubType PaletteGenerator::subType()
{
    return m_subType;
}

QString PaletteGenerator::typetoString(PaletteGenerator::PaletteType type)
{
    switch(type)
    {
        case PrimaryColors: return tr("Primary colours"); break;
        case SixteenColors: return tr("16 Colours"); break;
        case Shutter: return tr("Shutter macros");
        case Gobos: return tr("Gobo macros");
        case ColourMacro: return tr("Colour macros");
        case Animation: return tr("Animations");
        case Undefined:
        default:
            return tr("Unknown");
        break;
    }
}

QStringList PaletteGenerator::getCapabilities(const Fixture *fixture)
{
    QStringList caps;
    bool hasPan = false, hasTilt = false;
    bool hasRed = false, hasGreen = false, hasBlue = false;
    bool hasCyan = false, hasMagenta = false, hasYellow = false;
    bool hasWhite = false;

    Q_ASSERT(fixture != NULL);
    for (quint32 ch = 0; ch < fixture->channels(); ch++)
    {
        const QLCChannel* channel(fixture->channel(ch));
        Q_ASSERT(channel != NULL);

        switch (channel->group())
        {
            case QLCChannel::Colour:
            case QLCChannel::Gobo:
            case QLCChannel::Shutter:
            {
                if (channel->capabilities().size() > 1)
                {
                    QString cap = QLCChannel::groupToString(channel->group());
                    if (!caps.contains(cap))
                        caps.append(cap);
                }
            }
            break;
            case QLCChannel::Pan:
                hasPan = true;
            break;
            case QLCChannel::Tilt:
                hasTilt = true;
            break;
            case QLCChannel::Intensity:
            {
                QLCChannel::PrimaryColour col = channel->colour();
                switch (col)
                {
                    case QLCChannel::Red: hasRed = true; break;
                    case QLCChannel::Green: hasGreen = true; break;
                    case QLCChannel::Blue: hasBlue = true; break;
                    case QLCChannel::Cyan: hasCyan = true; break;
                    case QLCChannel::Magenta: hasMagenta = true; break;
                    case QLCChannel::Yellow: hasYellow = true; break;
                    case QLCChannel::White: hasWhite = true; break;
                    default: break;
                }
            }
            break;
            default:
            break;
        }
    }

    if (hasPan && hasTilt)
        caps.append(KQLCChannelMovement);

    if (hasRed && hasGreen && hasBlue)
        caps.append(KQLCChannelRGB);

    if (hasCyan && hasMagenta && hasYellow)
        caps.append(KQLCChannelCMY);

    if (hasWhite)
        caps.append(KQLCChannelWhite);

    return caps;
}

QList<Scene *> PaletteGenerator::scenes()
{
    return m_scenes;
}

QList<Chaser *> PaletteGenerator::chasers()
{
    return m_chasers;
}

QList<RGBMatrix *> PaletteGenerator::matrices()
{
    return m_matrices;
}

void PaletteGenerator::addToDoc()
{
    foreach(Scene *scene, m_scenes)
        m_doc->addFunction(scene);

    foreach(Chaser *chaser, m_chasers)
    {
        foreach(Scene *scene, m_scenes)
        {
            qDebug() << "Add chaser step:" << scene->id();
            chaser->addStep(ChaserStep(scene->id()));
        }
        m_doc->addFunction(chaser);
    }

    if (m_fixtureGroup != NULL)
        m_doc->addFixtureGroup(m_fixtureGroup);

    foreach(RGBMatrix *matrix, m_matrices)
    {
        matrix->setFixtureGroup(m_fixtureGroup->id());
        m_doc->addFunction(matrix);
    }
}

void PaletteGenerator::createColorScene(QHash<quint32, quint32> chMap, QString name, PaletteSubType subType)
{
    if (chMap.size() == 0)
        return;

    Scene *scene = new Scene(m_doc);
    Scene *evenScene = NULL;
    Scene *oddScene = NULL;
    bool even = false;

    if (subType == OddEven)
    {
        evenScene = new Scene(m_doc);
        oddScene = new Scene(m_doc);
    }

    QHashIterator <quint32, quint32> it(chMap);
    while (it.hasNext() == true)
    {
        it.next();
        scene->setValue(it.key(), it.value(), 255);
        if (subType == OddEven)
        {
            if (even)
                evenScene->setValue(it.key(), it.value(), 255);
            else
                oddScene->setValue(it.key(), it.value(), 255);
            even = !even;
        }
    }
    scene->setName(name + " - " + m_model);
    m_scenes.append(scene);
    if (subType == OddEven)
    {
        evenScene->setName(tr("%1 - %2 (Even)").arg(name).arg(m_model));
        oddScene->setName(tr("%1 - %2 (Odd)").arg(name).arg(m_model));
        m_scenes.append(evenScene);
        m_scenes.append(oddScene);
    }
}

void PaletteGenerator::createRGBCMYScene(QHash<quint32, quint32> rcMap,
                                         QHash<quint32, quint32> gmMap,
                                         QHash<quint32, quint32> byMap,
                                         QString name, bool rgb,
                                         PaletteGenerator::PaletteSubType subType)
{
    if (rcMap.size() == 0 || gmMap.size() == 0 || byMap.size() == 0)
        return;

    bool even = false;
    QList <QColor> m_colList;
    QList <QString> m_colNames;

    m_colList << Qt::black << Qt::darkBlue << Qt::blue << Qt::darkGreen <<
                 Qt::darkCyan << Qt::green << Qt::cyan << Qt::darkRed <<
                 Qt::darkMagenta << Qt::darkYellow << Qt::darkGray << Qt::lightGray <<
                 Qt::red << Qt::magenta << Qt::yellow << Qt::white;

    m_colNames << tr("Black") << tr("Dark Blue") << tr("Blue") << tr("Dark Green")
               << tr("Dark Cyan") << tr("Green") << tr("Cyan") << tr("Dark Red")
               << tr("Dark Magenta") << tr("Dark Yellow") << tr("Dark Gray") << tr("Light Gray")
               << tr("Red") << tr("Magenta") << tr("Yellow") << tr("White");

    for (int i = 0; i < m_colList.count(); i++)
    {
        QColor col = m_colList.at(i);
        uchar rc = col.red();
        uchar gm = col.green();
        uchar by = col.blue();
        if (rgb == false)
        {
            rc = col.cyan();
            gm = col.magenta();
            by = col.yellow();
        }

        Scene *scene = new Scene(m_doc);
        Scene *evenScene = NULL;
        Scene *oddScene = NULL;

        if (subType == OddEven)
        {
            evenScene = new Scene(m_doc);
            oddScene = new Scene(m_doc);
        }

        QHashIterator <quint32, quint32> it(rcMap);
        while (it.hasNext() == true)
        {
            it.next();
            scene->setValue(it.key(), it.value(), rc);
            scene->setValue(it.key(), gmMap[it.key()], gm);
            scene->setValue(it.key(), byMap[it.key()], by);

            if (subType == OddEven)
            {
                if (even)
                {
                    evenScene->setValue(it.key(), it.value(), rc);
                    evenScene->setValue(it.key(), gmMap[it.key()], gm);
                    evenScene->setValue(it.key(), byMap[it.key()], by);
                }
                else
                {
                    oddScene->setValue(it.key(), it.value(), rc);
                    oddScene->setValue(it.key(), gmMap[it.key()], gm);
                    oddScene->setValue(it.key(), byMap[it.key()], by);
                }
                even = !even;
            }
        }
        qDebug() << "color name:" << m_colNames.at(i) << "i:" << i << "count:" << m_colNames.count();

        scene->setName(tr("%1 %2 - %3").arg(name).arg(m_colNames.at(i)).arg(m_model));
        m_scenes.append(scene);
        if (subType == OddEven)
        {
            evenScene->setName(tr("%1 %2 - %3 (Even)").arg(name).arg(m_colNames.at(i)).arg(m_model));
            oddScene->setName(tr("%1 %2 - %3 (Odd)").arg(name).arg(m_colNames.at(i)).arg(m_model));
            m_scenes.append(evenScene);
            m_scenes.append(oddScene);
        }
    }
}

void PaletteGenerator::createCapabilityScene(QHash<quint32, quint32> chMap,
                                             PaletteGenerator::PaletteSubType subType)
{
    if (chMap.size() == 0)
        return;

    Fixture *fxi = m_fixtures.at(0);
    Q_ASSERT(fxi != NULL);
    QHashIterator <quint32, quint32> it(chMap);

    quint32 ch = it.next().value();
    const QLCChannel* channel = fxi->channel(ch);
    QStringList tmpCapList;

    for (int cIdx = 0; cIdx < channel->capabilities().count(); cIdx++)
    {
        Scene *scene = new Scene(m_doc);
        Scene *evenScene = NULL;
        Scene *oddScene = NULL;
        bool even = false;
        QLCCapability *cap = channel->capabilities().at(cIdx);
        uchar value = cap->middle();
        QString name = cap->name();

        // Do not add the same capability twice
        if (tmpCapList.contains(name))
            continue;

        tmpCapList.append(name);

        if (subType == OddEven)
        {
            evenScene = new Scene(m_doc);
            oddScene = new Scene(m_doc);
        }

        QHashIterator <quint32, quint32> it(chMap);
        while (it.hasNext() == true)
        {
            it.next();
            scene->setValue(it.key(), it.value(), value);
            if (subType == OddEven)
            {
                if (even)
                    evenScene->setValue(it.key(), it.value(), value);
                else
                    oddScene->setValue(it.key(), it.value(), value);
                even = !even;
            }
        }

        scene->setName(name + " - " + m_model);
        m_scenes.append(scene);
        if (subType == OddEven)
        {
            evenScene->setName(name + " - " + m_model + tr(" - Even"));
            oddScene->setName(name + " - " + m_model + tr(" - Odd"));
            m_scenes.append(evenScene);
            m_scenes.append(oddScene);
        }
    }
}

void PaletteGenerator::createRGBMatrices(QHash<quint32, quint32> rgbMap)
{
    m_fixtureGroup = new FixtureGroup(m_doc);
    m_fixtureGroup->setSize(QSize(rgbMap.size(), 1));

    QHashIterator <quint32, quint32> it(rgbMap);
    while (it.hasNext() == true)
    {
        it.next();
        m_fixtureGroup->assignFixture(it.key());
        m_fixtureGroup->setName(m_model + tr(" - RGB Group"));
    }
    QStringList algoList = m_doc->rgbScriptsCache()->names();
    foreach(QString algo, algoList)
    {
        RGBMatrix *matrix = new RGBMatrix(m_doc);
        matrix->setName(tr("Animation %1").arg(algo) + " - " + m_model);
        //matrix->setFixtureGroup();
        matrix->setAlgorithm(RGBAlgorithm::algorithm(m_doc, algo));
        m_matrices.append(matrix);
    }
}

void PaletteGenerator::createChaser(QString name)
{
    if (m_scenes.count() == 0)
        return;

    Chaser *chaser = new Chaser(m_doc);
    chaser->setFadeInMode(Chaser::Common);
    chaser->setFadeInSpeed(3000);
    chaser->setFadeOutMode(Chaser::Common);
    chaser->setFadeOutSpeed(0);
    chaser->setDurationMode(Chaser::Common);
    chaser->setDuration(10000);
    chaser->setName(tr("%1 chaser - %2").arg(name).arg(m_model));

    // that's all here. I need to add an empty Chaser cause
    // scene's IDs have not been assigned yet
    m_chasers.append(chaser);
}

void PaletteGenerator::createFunctions(PaletteGenerator::PaletteType type,
                                       PaletteGenerator::PaletteSubType subType)
{
    if (m_fixtures.count() == 0)
        return;

    // the following QHash will hold the fixture/channel map of each fixture
    // in m_fixtures depending on type and subtype
    QHash<quint32, quint32> m_panList;
    QHash<quint32, quint32> m_tiltList;
    QHash<quint32, quint32> m_redList;
    QHash<quint32, quint32> m_greenList;
    QHash<quint32, quint32> m_blueList;
    QHash<quint32, quint32> m_cyanList;
    QHash<quint32, quint32> m_magentaList;
    QHash<quint32, quint32> m_yellowList;
    QHash<quint32, quint32> m_whiteList;
    QHash<quint32, quint32> m_goboList;
    QHash<quint32, quint32> m_shutterList;
    QHash<quint32, quint32> m_colorMacroList;

    for(int i = 0; i < m_fixtures.count(); i++)
    {
        Fixture *fixture = m_fixtures.at(i);
        Q_ASSERT(fixture != NULL);
        quint32 fxID = fixture->id();

        for (quint32 ch = 0; ch < fixture->channels(); ch++)
        {
            const QLCChannel* channel(fixture->channel(ch));
            Q_ASSERT(channel != NULL);

            switch (channel->group())
            {
                case QLCChannel::Pan: m_panList[fxID] = ch; break;
                case QLCChannel::Tilt: m_tiltList[fxID] = ch; break;
                case QLCChannel::Gobo: m_goboList[fxID] = ch; break;
                case QLCChannel::Shutter: m_shutterList[fxID] = ch; break;
                case QLCChannel::Colour: m_colorMacroList[fxID] = ch; break;
                case QLCChannel::Intensity:
                {
                    QLCChannel::PrimaryColour col = channel->colour();
                    switch (col)
                    {
                        case QLCChannel::Red: m_redList[fxID] = ch; break;
                        case QLCChannel::Green: m_greenList[fxID] = ch; break;
                        case QLCChannel::Blue: m_blueList[fxID] = ch; break;
                        case QLCChannel::Cyan: m_cyanList[fxID] = ch; break;
                        case QLCChannel::Magenta: m_magentaList[fxID] = ch; break;
                        case QLCChannel::Yellow: m_yellowList[fxID] = ch; break;
                        case QLCChannel::White: m_whiteList[fxID] = ch; break;
                        default: break;
                    }
                }
                break;
                default:
                break;
            }
        }
    }

    switch (type)
    {
        case PrimaryColors:
        {
            createColorScene(m_redList, tr("Red scene"), subType);
            createColorScene(m_greenList, tr("Green scene"), subType);
            createColorScene(m_blueList, tr("Blue scene"), subType);
            createColorScene(m_cyanList, tr("Cyan scene"), subType);
            createColorScene(m_magentaList, tr("Magenta scene"), subType);
            createColorScene(m_yellowList, tr("Yellow scene"), subType);
            createColorScene(m_whiteList, tr("White scene"), subType);
            createChaser(typetoString(type));
        }
        break;
        case SixteenColors:
        {
            if (m_redList.size() > 0 && m_greenList.size() == m_redList.size() && m_blueList.size() ==  m_redList.size())
                createRGBCMYScene(m_redList, m_greenList, m_blueList, tr("Scene"), true, subType);
            else if (m_cyanList.size() > 0 && m_magentaList.size() == m_cyanList.size() && m_yellowList.size() ==  m_cyanList.size())
                createRGBCMYScene(m_cyanList, m_magentaList, m_yellowList, tr("Scene"), false, subType);
            createChaser(typetoString(type));
        }
        break;
        case Animation:
        {
            if (m_redList.size() > 1 && m_greenList.size() == m_redList.size() && m_blueList.size() ==  m_redList.size())
                createRGBMatrices(m_redList);
        }
        break;
        case Gobos:
        {
            createCapabilityScene(m_goboList, subType);
            createChaser(typetoString(type));
        }
        break;
        case Shutter:
        {
            createCapabilityScene(m_shutterList, subType);
            createChaser(typetoString(type));
        }
        break;
        case ColourMacro:
        {
            createCapabilityScene(m_colorMacroList, subType);
            createChaser(typetoString(type));
        }
        break;
        case Undefined:
        default:
        break;

    }

}
