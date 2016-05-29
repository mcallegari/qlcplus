/*
  Q Light Controller Plus
  fixturebrowser.cpp

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
#include <QQmlContext>

#include "fixturebrowser.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "doc.h"

FixtureBrowser::FixtureBrowser(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_view(view)
    , m_definition(NULL)
{
    Q_ASSERT(m_doc != NULL);
    Q_ASSERT(m_view != NULL);
}

QStringList FixtureBrowser::manufacturers()
{
    QStringList mfList = m_doc->fixtureDefCache()->manufacturers();
    mfList.sort();
    return mfList;
}

int FixtureBrowser::genericIndex()
{
    QStringList mfList = m_doc->fixtureDefCache()->manufacturers();
    mfList.sort();
    return mfList.indexOf("Generic");
}

QStringList FixtureBrowser::models(QString manufacturer)
{
    qDebug() << "[FixtureBrowser] Fixtures list for" << manufacturer;
    QStringList fxList = m_doc->fixtureDefCache()->models(manufacturer);
    fxList.sort();
    return fxList;
}

QStringList FixtureBrowser::modes(QString manufacturer, QString model)
{
    QStringList modesList;

    m_definition = m_doc->fixtureDefCache()->fixtureDef(manufacturer, model);

    if (m_definition != NULL)
    {
        QList<QLCFixtureMode *> fxModesList = m_definition->modes();
        foreach(QLCFixtureMode *mode, fxModesList)
            modesList.append(mode->name());
    }
    return modesList;
}

int FixtureBrowser::modeChannels(QString modeName)
{
    if (m_definition != NULL)
    {
        QLCFixtureMode *mode = m_definition->mode(modeName);
        if (mode != NULL)
            return mode->channels().count();
    }
    return 0;
}

int FixtureBrowser::availableChannel(quint32 uniIdx, int channels, int quantity, int gap, int requested)
{
    qDebug() << "[FixtureBrowser] uniIdx:" << uniIdx << ", channels:" << channels << ", requested:" << requested;
    bool isAvailable = true;
    quint32 uniFilter = uniIdx == Universe::invalid() ? 0 : uniIdx;
    quint32 absAddress = (requested & 0x01FF) | (uniFilter << 9);
    for (int n = 0; n < quantity; n++)
    {
        for (int i = 0; i < channels; i++)
        {
            if(m_doc->fixtureForAddress(absAddress + i) != Fixture::invalidId())
            {
                isAvailable = false;
                break;
            }
        }
        absAddress += channels + gap;
    }
    if (isAvailable == true)
    {
        qDebug() << "[FixtureBrowser] Requested channel is available:" << requested;
        return requested;
    }
    else
    {
        qDebug() << "[FixtureBrowser] Requested channel" << requested << "not available in universe" << uniFilter;
        int validAddr = 0;
        int freeCounter = 0;
        absAddress = uniFilter << 9;
        for (int i = 0; i < 512; i++)
        {
            if(m_doc->fixtureForAddress(absAddress + i) != Fixture::invalidId())
            {
                freeCounter = 0;
                validAddr = i + 1;
            }
            else
                freeCounter++;

            if (freeCounter == (channels * quantity) + (gap * quantity))
            {
                qDebug() << "[FixtureBrowser] Returning available address:" << validAddr;
                return validAddr;
            }
        }
    }

    return -1;
}

int FixtureBrowser::availableChannel(quint32 fixtureID, int requested)
{
    qDebug() << "[FixtureBrowser] fxID:" << fixtureID << ", requested:" << requested;
    bool isAvailable = true;

    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == NULL)
        return -1;

    quint32 channels = fixture->channels();
    quint32 absAddress = (requested & 0x01FF) | (fixture->universe() << 9);

    for (quint32 i = 0; i < channels; i++)
    {
        quint32 fxIDOnAddr = m_doc->fixtureForAddress(absAddress + i);
        if(fxIDOnAddr != Fixture::invalidId() && fxIDOnAddr != fixtureID)
        {
            isAvailable = false;
            break;
        }
    }

    if (isAvailable == true)
    {
        qDebug() << "[FixtureBrowser] Requested channel is available:" << requested;
        return requested;
    }

    return -1;
}

