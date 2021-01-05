/*
  Q Light Controller Plus
  simpledesk.cpp

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

#include <QQmlContext>

#include "simpledesk.h"
#include "keypadparser.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "scenevalue.h"
#include "listmodel.h"
#include "tardis.h"
#include "doc.h"

#define UserRoleClassReference  (Qt::UserRole + 1)
#define UserRoleChannelIndex    (Qt::UserRole + 2)
#define UserRoleChannelValue    (Qt::UserRole + 3)
#define UserRoleChannelStatus   (Qt::UserRole + 4)
#define UserRoleChannelOverride (Qt::UserRole + 5)

#define MAX_KEYPAD_HISTORY      10

SimpleDesk::SimpleDesk(QQuickView *view, Doc *doc, QObject *parent)
    : PreviewContext(view, doc, "SDESK", parent)
{
    Q_ASSERT(m_doc != nullptr);

    setContextResource("qrc:/SimpleDesk.qml");
    setContextTitle(tr("Simple Desk"));

    view->rootContext()->setContextProperty("simpleDesk", this);

    m_doc->masterTimer()->registerDMXSource(this);

    m_channelList = new ListModel(this);
    QStringList listRoles;
    listRoles << "cRef" << "chIndex" << "chValue" << "chDisplay" << "isOverride";
    m_channelList->setRoleNames(listRoles);

    // initialize channel value data
    for (quint32 i = 0; i < m_doc->inputOutputMap()->universesCount(); i++)
        m_prevUniverseValues[i].fill(0, 512);

    updateChannelList();

    connect(m_doc, SIGNAL(loaded()), this, SLOT(updateChannelList()));
    connect(m_doc, SIGNAL(fixtureAdded(quint32)), this, SLOT(updateChannelList()));
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(updateChannelList()));
    connect(m_doc->inputOutputMap(), SIGNAL(universeWritten(quint32,QByteArray)),
            this, SLOT(slotUniverseWritten(quint32,QByteArray)));
}

SimpleDesk::~SimpleDesk()
{
    m_doc->masterTimer()->unregisterDMXSource(this);
}

void SimpleDesk::setUniverseFilter(quint32 universeFilter)
{
    PreviewContext::setUniverseFilter(universeFilter);
    updateChannelList();
    emit fixtureListChanged();
}

QVariant SimpleDesk::channelList() const
{
    return QVariant::fromValue(m_channelList);
}

QVariantList SimpleDesk::fixtureList() const
{
    QVariantList list;

    foreach (Fixture *fxi, m_doc->fixtures())
    {
        if (fxi->universe() != m_universeFilter)
            continue;

        list.append(QVariant::fromValue(fxi));
    }

    return list;
}

void SimpleDesk::updateChannelList()
{
    QVariantList chList;
    quint32 start = (m_universeFilter * 512);
    quint32 prevID = Fixture::invalidId();
    int status = None;

    m_channelList->clear();

    QByteArray currUni = m_prevUniverseValues.value(m_universeFilter);

    for (int i = 0; i < currUni.length(); i++)
    {
        quint32 chIndex = 0;
        quint32 chValue = currUni.at(i);
        bool override = false;

        Fixture *fxi = m_doc->fixture(m_doc->fixtureForAddress(start + i));
        if (fxi != nullptr)
        {
            if (fxi->id() != prevID)
            {
                status = (status == Odd) ? Even : Odd;
                prevID = fxi->id();
            }
            chIndex = i - fxi->address();
            if (hasChannel(i))
            {
                chValue = value(i);
                override = true;
            }
            else
            {
                chValue = fxi->channelValueAt(chIndex);
            }
        }
        else
        {
            if (hasChannel(i))
            {
                override = true;
                chValue = value(i);
            }
        }

        QVariantMap chMap;
        chMap.insert("cRef", QVariant::fromValue(fxi));
        chMap.insert("chIndex", chIndex);
        chMap.insert("chValue", chValue);
        chMap.insert("chDisplay", status);
        chMap.insert("isOverride", override);

        m_channelList->addDataMap(chMap);
    }

    emit channelListChanged();
}

QVariant SimpleDesk::universesListModel() const
{
    QVariantList universesList;

    for (Universe *uni : m_doc->inputOutputMap()->universes())
    {
        QVariantMap uniMap;
        uniMap.insert("mLabel", uni->name());
        uniMap.insert("mValue", uni->id());
        universesList.append(uniMap);
    }

    return QVariant::fromValue(universesList);
}

/************************************************************************
 * Universe Values
 ************************************************************************/

void SimpleDesk::setValue(quint32 fixtureID, uint channel, uchar value)
{
    QMutexLocker locker(&m_mutex);
    quint32 start = (m_universeFilter * 512);
    QVariant currentVal, newVal;
    SceneValue currScv;

    if (m_values.contains(start + channel))
    {
        //currScv.fxi = fixtureID;
        currScv.channel = channel;
        currScv.value = m_values[start + channel];
    }
    currentVal.setValue(currScv);

    m_values[start + channel] = value;

    QModelIndex mIndex = m_channelList->index(int(channel), 0, QModelIndex());
    m_channelList->setData(mIndex, value, UserRoleChannelValue);
    m_channelList->setData(mIndex, true, UserRoleChannelOverride);

    newVal.setValue(SceneValue(Fixture::invalidId(), channel, value));
    Tardis::instance()->enqueueAction(Tardis::SimpleDeskSetChannel, 0, currentVal, newVal);

    if (fixtureID != Fixture::invalidId())
    {
        Fixture *fixture = m_doc->fixture(fixtureID);
        quint32 relCh = channel - fixture->address();
        emit channelValueChanged(fixtureID, relCh, value);
    }

    setChanged(true);
}

uchar SimpleDesk::value(uint channel) const
{
    QMutexLocker locker(&m_mutex);
    quint32 start = (m_universeFilter * 512);
    if (m_values.contains(start + channel) == true)
        return m_values[start + channel];
    else
        return 0;
}

bool SimpleDesk::hasChannel(uint channel)
{
    QMutexLocker locker(&m_mutex);
    quint32 start = (m_universeFilter * 512);
    return m_values.contains(start + channel);
}

void SimpleDesk::resetUniverse(int universe)
{
    // remove values previously set on universe
    QMutexLocker locker(&m_mutex);
    QHashIterator <uint,uchar> it(m_values);
    while (it.hasNext() == true)
    {
        it.next();
        uint absChannel = it.key();
        int uni = absChannel >> 9;
        if (uni == universe)
        {
            m_values.remove(absChannel);

            // remove the override flag from the displayed channel
            QModelIndex mIndex = m_channelList->index(int(absChannel & 0x01FF), 0, QModelIndex());
            //uchar chValue = uchar(m_prevUniverseValues[uni].at(absChannel & 0x01FF));
            m_channelList->setData(mIndex, 0, UserRoleChannelValue);
            m_channelList->setData(mIndex, false, UserRoleChannelOverride);
        }
    }

    // add command to queue. Will be taken care of at the next writeDMX call
    m_commandQueue.append(QPair<int,quint32>(ResetUniverse, universe));
    setChanged(true);
}

void SimpleDesk::resetChannel(uint channel)
{
    QMutexLocker locker(&m_mutex);
    quint32 start = (m_universeFilter * 512);
    QVariant currentVal;
    SceneValue currScv;

    if (m_values.contains(start + channel) == false)
        return;

    currScv.channel = channel;
    currScv.value = m_values[start + channel];
    currentVal.setValue(currScv);
    m_values.remove(start + channel);

    // add command to queue. Will be taken care of at the next writeDMX call
    m_commandQueue.append(QPair<int,quint32>(ResetChannel, start + channel));

    Tardis::instance()->enqueueAction(Tardis::SimpleDeskResetChannel, 0, currentVal, currentVal);

    setChanged(true);
}

void SimpleDesk::slotUniverseWritten(quint32 idx, const QByteArray& ua)
{
    if (idx != m_universeFilter) // || isEnabled() == false)
        return;

    QByteArray currUni = m_prevUniverseValues.value(idx);

    for (int i = 0; i < ua.length(); i++)
    {
        if (ua.at(i) == currUni.at(i))
            continue;

        //qDebug() << "Channel " << i << "changed to " << QString::number(uchar(ua.at(i)));

        // update displayed channel only if it is not overridden
        if (hasChannel(i) == false)
        {
            QModelIndex mIndex = m_channelList->index(int(i), 0, QModelIndex());
            m_channelList->setData(mIndex, QVariant(uchar(ua.at(i))), UserRoleChannelValue);
        }
    }

    m_prevUniverseValues[idx].replace(0, ua.length(), ua);
}

/************************************************************************
 * Keypad
 ************************************************************************/

void SimpleDesk::sendKeypadCommand(QString command)
{
    QByteArray uniData = m_prevUniverseValues.value(m_universeFilter);
    QList<SceneValue> scvList = KeyPadParser::parseCommand(m_doc, command, uniData);

    for (SceneValue scv : scvList)
    {
        setValue(Fixture::invalidId(), scv.channel, scv.value);
        QModelIndex mIndex = m_channelList->index(int(scv.channel), 0, QModelIndex());
        m_channelList->setData(mIndex, QVariant(scv.value), UserRoleChannelValue);
    }

    m_keypadCommandHistory.prepend(command);
    if (m_keypadCommandHistory.count() > MAX_KEYPAD_HISTORY)
        m_keypadCommandHistory.removeLast();

    emit commandHistoryChanged();
}

QStringList SimpleDesk::commandHistory() const
{
    return m_keypadCommandHistory;
}

/************************************************************************
 * DMXSource
 ************************************************************************/

FadeChannel *SimpleDesk::getFader(QList<Universe *> universes, quint32 universeID, quint32 fixtureID, quint32 channel)
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

void SimpleDesk::writeDMX(MasterTimer *timer, QList<Universe *> ua)
{
    Q_UNUSED(timer)

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
                    // loop through all active fadechannels and restore default values
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
                                //qDebug() << "Restoring default value of fixture" << fixture->id()
                                //         << "channel" << chIndex << "value" << ch->defaultValue();
                                ua[universe]->setChannelDefaultValue(fixture->address() + chIndex, ch->defaultValue());
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

                    // remove the override flag from the displayed channel
                    QModelIndex mIndex = m_channelList->index(int(channel & 0x01FF), 0, QModelIndex());
                    m_channelList->setData(mIndex, false, UserRoleChannelOverride);
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
}

