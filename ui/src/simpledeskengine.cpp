/*
  Q Light Controller
  simpledeskengine.cpp

  Copyright (c) Heikki Junnila

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

#include <QDomDocument>
#include <QDomElement>
#include <QVariant>
#include <QDebug>
#include <QMutexLocker>

#include "simpledeskengine.h"
#include "mastertimer.h"
#include "fadechannel.h"
#include "outputmap.h"
#include "cuestack.h"
#include "universe.h"
#include "doc.h"

#define PROP_ID "id"

/****************************************************************************
 * Initialization
 ****************************************************************************/

SimpleDeskEngine::SimpleDeskEngine(Doc* doc)
    : QObject(doc)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(doc != NULL);
    doc->masterTimer()->registerDMXSource(this);
}

SimpleDeskEngine::~SimpleDeskEngine()
{
    qDebug() << Q_FUNC_INFO;

    clearContents();
    doc()->masterTimer()->unregisterDMXSource(this);
}

Doc* SimpleDeskEngine::doc() const
{
    return qobject_cast<Doc*> (parent());
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
    m_resetValues.clear();
}

/****************************************************************************
 * Universe Values
 ****************************************************************************/

void SimpleDeskEngine::setValue(uint channel, uchar value)
{
    QLCChannel::Group group = getGroupForChannel(channel);

    QMutexLocker locker(&m_mutex);
    if (value == 0 && group == QLCChannel::Intensity)
        m_values.remove(channel);
    else
        m_values[channel] = value;

    m_resetValues.remove(channel);
}

uchar SimpleDeskEngine::value(uint channel) const
{
    QMutexLocker locker(&m_mutex);
    if (m_values.contains(channel) == true)
        return m_values[channel];
    else
        return 0;
}

void SimpleDeskEngine::setCue(const Cue& cue)
{
    qDebug() << Q_FUNC_INFO;

    QMutexLocker locker(&m_mutex);
    m_values = cue.values();

    foreach (uint channel, m_values.keys())
        m_resetValues.remove(channel);
}

Cue SimpleDeskEngine::cue() const
{
    QMutexLocker locker(&m_mutex);
    return Cue(m_values);
}

void SimpleDeskEngine::resetUniverse(int universe)
{
    qDebug() << Q_FUNC_INFO;

    QMutexLocker locker(&m_mutex);
    for (int i = 0; i < 512; i++)
    {
        m_values.remove(i);
	if (getGroupForChannel(i) != QLCChannel::Intensity)
             m_resetValues[(universe * 512) + i] = 0;
    }
}

QLCChannel::Group SimpleDeskEngine::getGroupForChannel(uint channel) const
{
    // Use FadeChannel's reverse-lookup to dig up the channel's group
    FadeChannel fc;
    fc.setChannel(channel);
    return fc.group(doc());
}

/****************************************************************************
 * Cue Stacks
 ****************************************************************************/

CueStack* SimpleDeskEngine::cueStack(uint stack)
{
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

    CueStack* cs = new CueStack(doc());
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

bool SimpleDeskEngine::loadXML(const QDomElement& root)
{
    qDebug() << Q_FUNC_INFO;
    if (root.tagName() != KXMLQLCSimpleDeskEngine)
    {
        qWarning() << Q_FUNC_INFO << "Simple Desk Engine node not found";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCCueStack)
        {
            uint id = CueStack::loadXMLID(tag);
            if (id != UINT_MAX)
            {
                CueStack* cs = cueStack(id);
                cs->loadXML(tag);
            }
            else
            {
                qWarning() << Q_FUNC_INFO << "Missing CueStack ID!";
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unrecognized Simple Desk Engine tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool SimpleDeskEngine::saveXML(QDomDocument* doc, QDomElement* wksp_root) const
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    QDomElement root = doc->createElement(KXMLQLCSimpleDeskEngine);
    wksp_root->appendChild(root);

    QHashIterator <uint,CueStack*> it(m_cueStacks);
    while (it.hasNext() == true)
    {
        it.next();

        // Save CueStack only if it contains something
        const CueStack* cs = it.value();
        if (cs->cues().size() > 0)
            cs->saveXML(doc, &root, it.key());
    }

    return true;
}

/****************************************************************************
 * DMXSource
 ****************************************************************************/

void SimpleDeskEngine::writeDMX(MasterTimer* timer, QList<Universe *> ua)
{
    QMutexLocker locker(&m_mutex);

    writeValuesHash(m_resetValues, ua);
    m_resetValues.clear();
    writeValuesHash(m_values, ua);

    foreach (CueStack* cueStack, m_cueStacks)
    {
        if (cueStack == NULL)
            continue;

        if (cueStack->isRunning() == true)
        {
            if (cueStack->isStarted() == false)
                cueStack->preRun();

            cueStack->write(ua);
        }
        else
        {
            if (cueStack->isStarted() == true)
                cueStack->postRun(timer);
        }
    }
}

void SimpleDeskEngine::writeValuesHash(QHash<uint, uchar> & hash, QList<Universe *> ua)
{
    QHashIterator <uint,uchar> it(hash);
    while (it.hasNext() == true)
    {
        it.next();

        Fixture* fxi = doc()->fixture(doc()->fixtureForAddress(it.key()));
        if (fxi == NULL || fxi->isDimmer() == true)
        {
            int uni = it.key() >> 9;
            int address = it.key() & 0x01FF;
            ua[uni]->write(address, it.value());
        }
        else
        {
            //uint ch = it.key() - fxi->universeAddress();
            int uni = fxi->universe();
            /*
            QLCChannel::Group grp = QLCChannel::NoGroup;
            if (ch < fxi->channels())
                grp = fxi->channel(ch)->group();
            else
                grp = QLCChannel::Intensity;
            */
            ua[uni]->write(fxi->address(), it.value());
        }
    }
}

