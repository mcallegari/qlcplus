/*
  Q Light Controller Plus
  simpledeskengine.cpp

  Copyright (c) Heikki Junnila
                Massimo Callegari

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QMutexLocker>
#include <QVariant>
#include <QDebug>

#include "simpledeskengine.h"
#include "genericfader.h"
#include "mastertimer.h"
#include "fadechannel.h"
#include "cuestack.h"
#include "universe.h"
#include "doc.h"

#define PROP_ID "id"

/****************************************************************************
 * Initialization
 ****************************************************************************/

SimpleDeskEngine::SimpleDeskEngine(Doc* doc)
    : QObject(doc)
    , m_doc(doc)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(doc != NULL);
    doc->masterTimer()->registerDMXSource(this);
}

SimpleDeskEngine::~SimpleDeskEngine()
{
    qDebug() << Q_FUNC_INFO;

    clearContents();
    m_doc->masterTimer()->unregisterDMXSource(this);
}

void SimpleDeskEngine::clearContents()
{
    qDebug() << Q_FUNC_INFO;

    // Stop all cuestacks and wait for each of them to stop
    foreach (CueStack* cs, m_cueStacks.values())
    {
        cs->stop();
        while (cs->isStarted() == true) { /* NOP */ }
    }

    QMutexLocker locker(&m_mutex);
    foreach (CueStack* cs, m_cueStacks.values())
        delete cs;
    m_cueStacks.clear();
    m_values.clear();
}

/****************************************************************************
 * Universe Values
 ****************************************************************************/

void SimpleDeskEngine::setValue(uint channel, uchar value)
{
    //qDebug() << Q_FUNC_INFO << "channel:" << channel << ", value:" << value;

    QMutexLocker locker(&m_mutex);
    m_values[channel] = value;
    setChanged(true);
}

uchar SimpleDeskEngine::value(uint channel) const
{
    QMutexLocker locker(&m_mutex);
    if (m_values.contains(channel) == true)
        return m_values[channel];
    else
        return 0;
}

bool SimpleDeskEngine::hasChannel(uint channel)
{
    QMutexLocker locker(&m_mutex);
    return m_values.contains(channel);
}

void SimpleDeskEngine::setCue(const Cue& cue)
{
    qDebug() << Q_FUNC_INFO;

    QMutexLocker locker(&m_mutex);
    m_values = cue.values();
    setChanged(true);
}

Cue SimpleDeskEngine::cue() const
{
    QMutexLocker locker(&m_mutex);
    return Cue(m_values);
}

void SimpleDeskEngine::resetUniverse(int universe)
{
    qDebug() << Q_FUNC_INFO;

    // remove values previously set on universe
    QMutexLocker locker(&m_mutex);
    QHashIterator <uint,uchar> it(m_values);
    while (it.hasNext() == true)
    {
        it.next();
        int uni = it.key() >> 9;
        if (uni == universe)
            m_values.remove(it.key());
    }

    // add command to queue. Will be taken care of at the next writeDMX call
    m_commandQueue.append(QPair<int,quint32>(ResetUniverse, universe));
    setChanged(true);
}

void SimpleDeskEngine::resetChannel(uint channel)
{
    QMutexLocker locker(&m_mutex);
    if (m_values.contains(channel))
        m_values.remove(channel);

    // add command to queue. Will be taken care of at the next writeDMX call
    m_commandQueue.append(QPair<int,quint32>(ResetChannel, channel));
    setChanged(true);
}

/****************************************************************************
 * Cue Stacks
 ****************************************************************************/

CueStack* SimpleDeskEngine::cueStack(uint stack)
{
    QMutexLocker locker(&m_mutex);

    if (m_cueStacks.contains(stack) == false)
    {
        m_cueStacks[stack] = createCueStack();
        m_cueStacks[stack]->setProperty(PROP_ID, stack);
    }

    return m_cueStacks[stack];
}

CueStack* SimpleDeskEngine::createCueStack()
{
    qDebug() << Q_FUNC_INFO;

    CueStack* cs = new CueStack(m_doc);
    Q_ASSERT(cs != NULL);
    connect(cs, SIGNAL(currentCueChanged(int)), this, SLOT(slotCurrentCueChanged(int)));
    connect(cs, SIGNAL(started()), this, SLOT(slotCueStackStarted()));
    connect(cs, SIGNAL(stopped()), this, SLOT(slotCueStackStopped()));
    return cs;
}

void SimpleDeskEngine::slotCurrentCueChanged(int index)
{
    qDebug() << Q_FUNC_INFO;

    if (sender() == NULL)
        return;

    uint stack = sender()->property(PROP_ID).toUInt();
    emit currentCueChanged(stack, index);
}

void SimpleDeskEngine::slotCueStackStarted()
{
    qDebug() << Q_FUNC_INFO;

    if (sender() == NULL)
        return;

    uint stack = sender()->property(PROP_ID).toUInt();
    emit cueStackStarted(stack);
}

void SimpleDeskEngine::slotCueStackStopped()
{
    qDebug() << Q_FUNC_INFO;

    if (sender() == NULL)
        return;

    uint stack = sender()->property(PROP_ID).toUInt();
    emit cueStackStopped(stack);
}

/************************************************************************
 * Save & Load
 ************************************************************************/

bool SimpleDeskEngine::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCSimpleDeskEngine)
    {
        qWarning() << Q_FUNC_INFO << "Simple Desk Engine node not found";
        return false;
    }

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCCueStack)
        {
            uint id = CueStack::loadXMLID(root);
            if (id != UINT_MAX)
            {
                CueStack* cs = cueStack(id);
                cs->loadXML(root);
            }
            else
            {
                qWarning() << Q_FUNC_INFO << "Missing CueStack ID!";
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unrecognized Simple Desk Engine tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool SimpleDeskEngine::saveXML(QXmlStreamWriter *doc) const
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCSimpleDeskEngine);

    QMutexLocker locker(&m_mutex);

    QHashIterator <uint,CueStack*> it(m_cueStacks);
    while (it.hasNext() == true)
    {
        it.next();

        // Save CueStack only if it contains something
        const CueStack* cs = it.value();
        if (cs->cues().size() > 0)
            cs->saveXML(doc, it.key());
    }

    /* End the <Engine> tag */
    doc->writeEndElement();

    return true;
}

/****************************************************************************
 * DMXSource
 ****************************************************************************/

FadeChannel *SimpleDeskEngine::getFader(QList<Universe *> universes, quint32 universeID,
                                        quint32 fixtureID, quint32 channel)
{
    // get the universe Fader first. If doesn't exist, create it
    QSharedPointer<GenericFader> fader = m_fadersMap.value(universeID, QSharedPointer<GenericFader>());
    if (fader.isNull())
    {
        fader = universes[universeID]->requestFader(Universe::SimpleDesk);
        m_fadersMap[universeID] = fader;
    }

    return fader->getChannelFader(m_doc, universes[universeID], fixtureID, channel);
}

void SimpleDeskEngine::writeDMX(MasterTimer *timer, QList<Universe *> ua)
{
    QMutexLocker locker(&m_mutex);

    if (m_commandQueue.isEmpty() == false)
    {
        for (int i = 0; i < m_commandQueue.count(); i++)
        {
            QPair<int,quint32> command = m_commandQueue.at(i);
            if (command.first == ResetUniverse)
            {
                quint32 universe = command.second;
                if (universe >= (quint32)ua.count())
                    continue;

                ua[universe]->reset(0, 512);

                QSharedPointer<GenericFader> fader = m_fadersMap.value(universe, QSharedPointer<GenericFader>());
                if (!fader.isNull())
                {
                    // loop through all active fadechannels and restore defualt values
                    QHashIterator<quint32, FadeChannel> it(fader->channels());
                    while (it.hasNext() == true)
                    {
                        it.next();
                        FadeChannel fc = it.value();
                        Fixture *fixture = m_doc->fixture(fc.fixture());
                        quint32 chIndex = fc.channel();

                        if (fixture != NULL)
                        {
                            const QLCChannel *ch = fixture->channel(chIndex);
                            if (ch != NULL)
                            {
                                qDebug() << "Restoring default value of fixture" << fixture->id()
                                         << "channel" << chIndex << "value" << ch->defaultValue();
                                chIndex += fixture->address();

                                if (fixture->crossUniverse() && chIndex > 511)
                                    chIndex -= 512;

                                ua[universe]->setChannelDefaultValue(chIndex, ch->defaultValue());
                            }
                        }
                    }
                    ua[universe]->dismissFader(fader);
                    m_fadersMap.remove(universe);
                }
            }
            else if (command.first == ResetChannel)
            {
                quint32 channel = command.second;
                quint32 universe = channel >> 9;
                QSharedPointer<GenericFader> fader = m_fadersMap.value(universe, QSharedPointer<GenericFader>());
                if (!fader.isNull())
                {
                    FadeChannel fc(m_doc, Fixture::invalidId(), channel);
                    Fixture *fixture = m_doc->fixture(fc.fixture());
                    quint32 chIndex = fc.channel();
                    fader->remove(&fc);

                    if (fixture->crossUniverse() && channel > 511)
                        channel -= 512;

                    ua[universe]->reset(channel & 0x01FF, 1);
                    if (fixture != NULL)
                    {
                        const QLCChannel *ch = fixture->channel(chIndex);
                        if (ch != NULL)
                        {
                            qDebug() << "Restoring default value of fixture" << fixture->id()
                                     << "channel" << chIndex << "value" << ch->defaultValue();
                            ua[universe]->setChannelDefaultValue(channel, ch->defaultValue());
                        }
                    }
                }
            }
        }
        m_commandQueue.clear();
    }

    if (hasChanged())
    {
        QHashIterator <uint,uchar> it(m_values);
        while (it.hasNext() == true)
        {
            it.next();
            int uni = it.key() >> 9;
            int address = it.key();
            uchar value = it.value();
            FadeChannel *fc = getFader(ua, uni, Fixture::invalidId(), address);
            fc->setCurrent(value);
            fc->setTarget(value);
            fc->addFlag(FadeChannel::Override);
        }
        setChanged(false);
    }

    foreach (CueStack *cueStack, m_cueStacks)
    {
        if (cueStack == NULL)
            continue;

        if (cueStack->isRunning())
        {
            if (!cueStack->isStarted())
                cueStack->preRun();

            cueStack->write(ua);
        }
        else
        {
            if (cueStack->isStarted())
                cueStack->postRun(timer, ua);
        }
    }
}
