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
    , m_manufacturer(QString())
    , m_model(QString())
    , m_definition(NULL)
    , m_mode(NULL)
{
    Q_ASSERT(m_doc != NULL);
    Q_ASSERT(m_view != NULL);

/*
    QObject *loaderObj = view->rootObject()->findChild<QObject *>("editorLoader");

    if (loaderObj != NULL)
        connect(loaderObj, SIGNAL(loaded()), this, SLOT(slotUiEditorLoaded()));
    else
        qDebug () << "Cannot find the editor loader object !";
*/
}

QStringList FixtureBrowser::manufacturers()
{
    QStringList mfList = m_doc->fixtureDefCache()->manufacturers();
    mfList.sort();
    return mfList;
}

void FixtureBrowser::setModel(QString model)
{
    m_model = model;
    m_definition = m_doc->fixtureDefCache()->fixtureDef(m_manufacturer, m_model);
    if (m_definition != NULL)
    {
        QList<QLCFixtureMode *> modesList = m_definition->modes();
        if (modesList.count() > 0)
        {
            m_mode = modesList.first();
            emit modeChannelsChanged();
        }
    }
}

QStringList FixtureBrowser::models()
{
    qDebug() << "Fixtures list for" << m_manufacturer;
    QStringList fxList = m_doc->fixtureDefCache()->models(m_manufacturer);
    fxList.sort();
    return fxList;
}

QString FixtureBrowser::mode() const
{
    if (m_mode != NULL)
        return m_mode->name();
    return QString();
}

void FixtureBrowser::setMode(QString name)
{
    if (m_definition != NULL)
    {
        m_mode = m_definition->mode(name);
        emit modeChannelsChanged();
    }
}

QStringList FixtureBrowser::modes()
{
    QStringList modes;

    if (m_definition != NULL)
    {
        QList<QLCFixtureMode *> modesList = m_definition->modes();
        foreach(QLCFixtureMode *mode, modesList)
            modes.append(mode->name());
    }
    return modes;
}

int FixtureBrowser::modeChannels()
{
    if (m_mode != NULL)
    {
        return m_mode->channels().count();
    }
    return 0;
}

