/*
  Q Light Controller
  palettegenerator.cpp

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

#include <QString>

#include "qlccapability.h"
#include "qlcchannel.h"

#include "palettegenerator.h"
#include "qlcfixturedef.h"
#include "chaserstep.h"
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
{
    if (m_fixtures.count() > 0)
    {
        m_name = typetoString(type);
        m_model = m_fixtures.at(0)->fixtureDef()->model();
        if (type != Undefined)
            createFunctions(type, subType);
    }
}

PaletteGenerator::~PaletteGenerator()
{
    m_scenes.clear();
    m_chasers.clear();
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
        case PrimaryColors: return tr("Primary colors"); break;
        case SixteenColors: return tr("16 Colors"); break;
        case Shutter: return tr("Shutter macros");
        case Gobos: return tr("Gobo macros");
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
        evenScene->setName(name + " - " + m_model + tr(" - Even"));
        oddScene->setName(name + " - " + m_model + tr(" - Odd"));
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

        scene->setName(name + " " + m_colNames.at(i) + " - " + m_model);
        m_scenes.append(scene);
        if (subType == OddEven)
        {
            evenScene->setName(name + " " + m_colNames.at(i) + " - " + m_model + tr(" - Even"));
            oddScene->setName(name + " " + m_colNames.at(i) + " - " + m_model + tr(" - Odd"));
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

    for (int cIdx = 0; cIdx < channel->capabilities().count(); cIdx++)
    {
        Scene *scene = new Scene(m_doc);
        Scene *evenScene = NULL;
        Scene *oddScene = NULL;
        bool even = false;
        QLCCapability *cap = channel->capabilities().at(cIdx);
        uchar value = cap->middle();
        QString name = cap->name();

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
    chaser->setName(name + " " + tr("chaser") + " - " + m_model);

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

    if (type == PrimaryColors)
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
    else if (type == SixteenColors)
    {
        if (m_redList.size() > 0 && m_greenList.size() == m_redList.size() && m_blueList.size() ==  m_redList.size())
            createRGBCMYScene(m_redList, m_greenList, m_blueList, tr("Scene"), true, subType);
        else if (m_cyanList.size() > 0 && m_magentaList.size() == m_cyanList.size() && m_yellowList.size() ==  m_cyanList.size())
            createRGBCMYScene(m_cyanList, m_magentaList, m_yellowList, tr("Scene"), false, subType);
        createChaser(typetoString(type));
    }
    else if (type == Gobos)
    {
        createCapabilityScene(m_goboList, subType);
        createChaser(typetoString(type));
    }
    else if (type == Shutter)
    {
        createCapabilityScene(m_shutterList, subType);
        createChaser(typetoString(type));
    }
}
