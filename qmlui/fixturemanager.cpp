/*
  Q Light Controller Plus
  fixturemanager.cpp

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

#include <QQuickItem>
#include <QDebug>

#include "fixturemanager.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "fixture.h"
#include "doc.h"

FixtureManager::FixtureManager(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
{
    Q_ASSERT(m_doc != NULL);

    connect(m_doc, SIGNAL(loaded()),
            this, SIGNAL(docLoaded()));
}

quint32 FixtureManager::invalidFixture()
{
    return Fixture::invalidId();
}

quint32 FixtureManager::fixtureForAddress(quint32 index)
{
    return m_doc->fixtureForAddress(index);
}

bool FixtureManager::addFixture(QString manuf, QString model, QString mode, QString name,
                                int uniIdx, int address, int channels, int quantity, quint32 gap,
                                qreal xPos, qreal yPos)
{
    qDebug() << Q_FUNC_INFO << manuf << model << quantity;

    QLCFixtureDef *fxiDef = m_doc->fixtureDefCache()->fixtureDef(manuf, model);
    Q_ASSERT(fxiDef != NULL);

    QLCFixtureMode *fxiMode = fxiDef->mode(mode);
    Q_ASSERT(fxiMode != NULL);

    for (int i = 0; i < quantity; i++)
    {
        Fixture *fxi = new Fixture(m_doc);

        /* If we're adding more than one fixture,
           append a number to the end of the name */
        if (quantity > 1)
            fxi->setName(QString("%1 #%2").arg(name).arg(i + 1));
        else
            fxi->setName(name);
        fxi->setAddress(address + (i * channels) + (i * gap));
        fxi->setUniverse(uniIdx);
        fxi->setFixtureDefinition(fxiDef, fxiMode);

        m_doc->addFixture(fxi);
        emit newFixtureCreated(fxi->id(), xPos, yPos);
    }
    m_fixtureList.clear();
    m_fixtureList = m_doc->fixtures();
    emit fixturesCountChanged();

    return true;
}

QString FixtureManager::channelIcon(quint32 fxID, quint32 chIdx)
{
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return QString();

    const QLCChannel *channel = fixture->channel(chIdx);
    if (channel == NULL)
        return QString();

    QString chIcon = channel->getIconNameFromGroup(channel->group());
    if (chIcon.startsWith("#"))
    {
        if (chIcon == "#FF0000") return "qrc:/red.svg";
        else if (chIcon == "#00FF00") return "qrc:/green.svg";
        else if (chIcon == "#0000FF") return "qrc:/blue.svg";
        else if (chIcon == "#00FFFF") return "qrc:/cyan.svg";
        else if (chIcon == "#FF00FF") return "qrc:/magenta.svg";
        else if (chIcon == "#FFFF00") return "qrc:/yellow.svg";
        else if (chIcon == "#FF7E00") return "qrc:/amber.svg";
        else if (chIcon == "#FFFFFF") return "qrc:/white.svg";
        else if (chIcon == "#9400D3") return "qrc:/uv.svg";
    }
    else
        chIcon.replace(".png", ".svg");

    return "qrc" + chIcon;
}

void FixtureManager::setIntensityValue(quint8 value)
{
    emit channelTypeValueChanged(QLCChannel::Intensity, value);
}

void FixtureManager::setColorValue(quint8 red, quint8 green, quint8 blue, quint8 white)
{
    emit channelTypeValueChanged(QLCChannel::Red, red);
    emit channelTypeValueChanged(QLCChannel::Green, green);
    emit channelTypeValueChanged(QLCChannel::Blue, blue);
    emit channelTypeValueChanged(QLCChannel::White, white);
}

QMultiHash<int, SceneValue> FixtureManager::setFixtureCapabilities(quint32 fxID, bool enable)
{
    int capDelta = 1;
    bool hasDimmer = false, hasColor = false, hasPosition = false;

    QMultiHash<int, SceneValue> channelsMap;

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return channelsMap;

    if (enable == false)
        capDelta = -1;

    for (quint32 ch = 0; ch < fixture->channels(); ch++)
    {
        const QLCChannel* channel(fixture->channel(ch));
        if(channel == NULL)
            continue;

        int chType = channel->group();

        switch (channel->group())
        {
            case QLCChannel::Intensity:
            {
                QLCChannel::PrimaryColour col = channel->colour();
                switch (col)
                {
                    case QLCChannel::NoColour:
                        hasDimmer = true;
                    break;
                    case QLCChannel::Red:
                    case QLCChannel::Green:
                    case QLCChannel::Blue:
                    case QLCChannel::Cyan:
                    case QLCChannel::Magenta:
                    case QLCChannel::Yellow:
                    case QLCChannel::White:
                        hasColor = true;
                    break;
                    default: break;
                }
                chType = col;
            }
            break;
            case QLCChannel::Pan:
            case QLCChannel::Tilt:
                hasPosition = true;
            break;
            default:
            break;
        }
        if (hasDimmer)
        {
            QQuickItem *dimmerCapItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("capIntensity"));
            dimmerCapItem->setProperty("counter", dimmerCapItem->property("counter").toInt() + capDelta);
        }
        if (hasColor)
        {
            QQuickItem *colorCapItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("capColor"));
            colorCapItem->setProperty("counter", colorCapItem->property("counter").toInt() + capDelta);
        }
        if (hasPosition)
        {
            QQuickItem *positionCapItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("capPosition"));
            positionCapItem->setProperty("counter", positionCapItem->property("counter").toInt() + capDelta);
        }

        channelsMap.insert(chType, SceneValue(fxID, ch));
    }
    return channelsMap;
}

int FixtureManager::fixturesCount()
{
    return m_doc->fixtures().count();
}

QQmlListProperty<Fixture> FixtureManager::fixtures()
{
    m_fixtureList.clear();
    m_fixtureList = m_doc->fixtures();
    return QQmlListProperty<Fixture>(this, m_fixtureList);
}


