/*
  Q Light Controller
  palettegenerator.cpp

  Copyright (C) Heikki Junnila

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

#include <QString>

#include "qlccapability.h"
#include "qlcchannel.h"

#include "palettegenerator.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"

PaletteGenerator::PaletteGenerator(Doc* doc, const QList <Fixture*>& fxiList)
        : m_doc(doc)
        , m_fixtures(fxiList)
{
}

PaletteGenerator::~PaletteGenerator()
{
    purgeScenes();
}

void PaletteGenerator::createColours()
{
    purgeScenes();

    /* Create functions for each selected fixture */
    QListIterator <Fixture*> fxiit(m_fixtures);
    while (fxiit.hasNext() == true)
    {
        Fixture* fxi(fxiit.next());
        Q_ASSERT(fxi != NULL);

        createGroupScenes(fxi, QLCChannel::Colour);
    }

    addScenesToDoc();
}

void PaletteGenerator::createGobos()
{
    purgeScenes();

    /* Create functions for each selected fixture */
    QListIterator <Fixture*> fxiit(m_fixtures);
    while (fxiit.hasNext() == true)
    {
        Fixture* fxi(fxiit.next());
        Q_ASSERT(fxi != NULL);

        createGroupScenes(fxi, QLCChannel::Gobo);
    }

    addScenesToDoc();
}

void PaletteGenerator::createShutters()
{
    purgeScenes();

    /* Create functions for each selected fixture */
    QListIterator <Fixture*> fxiit(m_fixtures);
    while (fxiit.hasNext() == true)
    {
        Fixture* fxi(fxiit.next());
        Q_ASSERT(fxi != NULL);

        createGroupScenes(fxi, QLCChannel::Shutter);
    }

    addScenesToDoc();
}

void PaletteGenerator::createGroupScenes(const Fixture* fxi,
                                         QLCChannel::Group group)
{
    Q_ASSERT(fxi != NULL);

    QList <quint32> channels = findChannels(fxi, group);
    for (int i = 0; i < channels.size(); i++)
    {
        quint32 ch = channels.at(i);
        const QLCChannel* channel = fxi->channel(ch);
        Q_ASSERT(channel != NULL);

        QListIterator <QLCCapability*> capit(channel->capabilities());
        while (capit.hasNext() == true)
        {
            const QLCCapability* cap = capit.next();
            Q_ASSERT(cap != NULL);

            QString name;
            if (channels.size() > 1)
            {
                // There's more than one channel (e.g. two gobo
                // gobo wheels, several distinct color wheels)
                name = QString("%1 (%2) - %3")
                               .arg(QLCChannel::groupToString(group))
                               .arg(i + 1).arg(cap->name());
            }
            else
            {
                // There's only one channel of the group
                // in the fixture (e.g. one gobo wheel)
                name = QString("%1 - %2")
                               .arg(QLCChannel::groupToString(group))
                               .arg(cap->name());
            }

            Scene* scene = NULL;
            if (m_scenes.contains(name) == true)
            {
                // Append this fixture's values to a scene with the
                // same name, that contains other similar values.
                scene = m_scenes[name];
            }
            else
            {
                scene = new Scene(m_doc);
                scene->setName(name);
                m_scenes[name] = scene;
            }

            scene->setValue(fxi->id(), ch, cap->middle());
            //scene->setBus(Bus::defaultPalette());
        }
    }
}

QList <quint32> PaletteGenerator::findChannels(const Fixture* fixture,
                                               QLCChannel::Group group)
{
    QList <quint32> channels;

    Q_ASSERT(fixture != NULL);
    for (quint32 ch = 0; ch < fixture->channels(); ch++)
    {
        const QLCChannel* channel(fixture->channel(ch));
        Q_ASSERT(channel != NULL);
        if (channel->group() == group && channel->capabilities().size() > 1)
            channels << ch;
    }

    return channels;
}

void PaletteGenerator::purgeScenes()
{
    QHashIterator <QString,Scene*> it(m_scenes);
    while (it.hasNext() == true)
    {
        it.next();
        if (m_doc->function(it.value()->id()) == NULL)
            delete it.value();
    }
    m_scenes.clear();
}

void PaletteGenerator::addScenesToDoc()
{
    QHashIterator <QString,Scene*> it(m_scenes);
    while (it.hasNext() == true)
    {
        it.next();
        if (m_doc->addFunction(it.value()) == false)
            break;
    }
}
