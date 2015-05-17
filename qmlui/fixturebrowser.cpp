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
    qDebug() << "Fixtures list for" << manufacturer;
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

int FixtureBrowser::availableChannel(int uniIdx, int channels, int requested)
{
    qDebug() << "[FixtureBrowser] uniIdx:" << uniIdx << ", channels:" << channels << ", requested:" << requested;
    bool isAvailable = true;
    quint32 absAddress = (requested & 0x01FF) | (uniIdx << 9);
    for (int i = 0; i < channels; i++)
    {
        if(m_doc->fixtureForAddress(absAddress + i) != Fixture::invalidId())
        {
            isAvailable = false;
            break;
        }
    }
    if (isAvailable == true)
    {
        qDebug() << "Requested channel is available:" << requested;
        return requested;
    }
    else
    {
        qDebug() << "Requested channel not available";
        int validAddr = 0;
        int freeCounter = 0;
        absAddress = uniIdx << 9;
        for (int i = 0; i < 512; i++)
        {
            if(m_doc->fixtureForAddress(absAddress + i) != Fixture::invalidId())
            {
                freeCounter = 0;
                validAddr = i + 1;
            }
            else
                freeCounter++;

            if (freeCounter == channels)
            {
                qDebug() << "--> Returning " << validAddr;
                return validAddr;
            }
        }
    }
    qDebug() << "Returning 0 !!!";
    return 0;
}

