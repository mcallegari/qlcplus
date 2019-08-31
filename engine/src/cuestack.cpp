/*
  Q Light Controller Plus
  cuestack.cpp

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
#include <qmath.h>
#include <QDebug>
#include <QHash>

#include "genericfader.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "qlcmacros.h"
#include "cuestack.h"
#include "universe.h"
#include "cue.h"
#include "doc.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

CueStack::CueStack(Doc* doc)
    : QObject(doc)
    , m_fadeInSpeed(0)
    , m_fadeOutSpeed(0)
    , m_duration(UINT_MAX)
    , m_running(false)
    , m_intensity(1.0)
    , m_currentIndex(-1)
    , m_flashing(false)
    , m_elapsed(0)
    , m_previous(false)
    , m_next(false)
{
    //qDebug() << Q_FUNC_INFO << (void*) this;
    Q_ASSERT(doc != NULL);
}

CueStack::~CueStack()
{
    //qDebug() << Q_FUNC_INFO << (void*) this;
    Q_ASSERT(isStarted() == false);
    Q_ASSERT(isFlashing() == false);
    m_cues.clear(); // Crashes without this, WTF?!
}

Doc* CueStack::doc() const
{
    return qobject_cast<Doc*> (parent());
}

/****************************************************************************
 * Name
 ****************************************************************************/

void CueStack::setName(const QString& name, int index)
{
    if (index < 0)
        m_name = name;
    else
        m_cues[index].setName(name);
    emit changed(index);
}

QString CueStack::name(int index) const
{
    if (index < 0)
        return m_name;
    else
        return m_cues[index].name();
}

/****************************************************************************
 * Speed
 ****************************************************************************/

void CueStack::setFadeInSpeed(uint ms, int index)
{
    if (index < 0)
        m_fadeInSpeed = ms;
    else
        m_cues[index].setFadeInSpeed(ms);
    emit changed(index);
}

uint CueStack::fadeInSpeed(int index) const
{
    if (index < 0)
        return m_fadeInSpeed;
    else
        return m_cues[index].fadeInSpeed();
}

void CueStack::setFadeOutSpeed(uint ms, int index)
{
    if (index < 0)
        m_fadeOutSpeed = ms;
    else
        m_cues[index].setFadeOutSpeed(ms);
    emit changed(index);
}

uint CueStack::fadeOutSpeed(int index) const
{
    if (index < 0)
        return m_fadeOutSpeed;
    else
        return m_cues[index].fadeOutSpeed();
}

void CueStack::setDuration(uint ms, int index)
{
    if (index < 0)
        m_duration = ms;
    else
        m_cues[index].setDuration(ms);
    emit changed(index);
}

uint CueStack::duration(int index) const
{
    if (index < 0)
        return m_duration;
    else
        return m_cues[index].duration();
}

/****************************************************************************
 * Cues
 ****************************************************************************/

void CueStack::appendCue(const Cue& cue)
{
    qDebug() << Q_FUNC_INFO;

    int index = 0;
    {
        QMutexLocker locker(&m_mutex);
        m_cues.append(cue);
        index = m_cues.size() - 1;
    }

    emit added(index);
}

void CueStack::insertCue(int index, const Cue& cue)
{
    qDebug() << Q_FUNC_INFO;

    bool cueAdded = false;

    {
        QMutexLocker locker(&m_mutex);

        if (index >= 0 && index < m_cues.size())
        {
            m_cues.insert(index, cue);
            cueAdded = true;
            emit added(index);

            if (m_currentIndex >= index)
            {
                m_currentIndex++;
                emit currentCueChanged(m_currentIndex);
            }
        }
    }

    if (!cueAdded)
        appendCue(cue);
}

void CueStack::replaceCue(int index, const Cue& cue)
{
    qDebug() << Q_FUNC_INFO;

    bool cueChanged = false;
    {
        QMutexLocker locker(&m_mutex);

        if (index >= 0 && index < m_cues.size())
        {
           m_cues[index] = cue;
           cueChanged = true;
        }
    }

    if (cueChanged)
    {
        emit changed(index);
    }
    else
    {
        appendCue(cue);
    }
}

void CueStack::removeCue(int index)
{
    qDebug() << Q_FUNC_INFO;

    QMutexLocker locker(&m_mutex);
    if (index >= 0 && index < m_cues.size())
    {
        m_cues.removeAt(index);
        emit removed(index);

        if (index < m_currentIndex)
        {
            m_currentIndex--;
            emit currentCueChanged(m_currentIndex);
        }
    }
}

void CueStack::removeCues(const QList <int>& indexes)
{
    qDebug() << Q_FUNC_INFO;

    // Sort the list so that the items can be removed in reverse order.
    // This way, the indices are always correct.
    QList <int> indexList = indexes;
    std::sort(indexList.begin(), indexList.end());

    QListIterator <int> it(indexList);
    it.toBack();

    QMutexLocker locker(&m_mutex);

    while (it.hasPrevious() == true)
    {
        int index(it.previous());
        if (index >= 0 && index < m_cues.size())
        {
            m_cues.removeAt(index);
            emit removed(index);

            if (index < m_currentIndex)
            {
                m_currentIndex--;
                emit currentCueChanged(m_currentIndex);
            }
        }
    }
}

QList <Cue> CueStack::cues() const
{
    return m_cues;
}

void CueStack::setCurrentIndex(int index)
{
    qDebug() << Q_FUNC_INFO;

    QMutexLocker locker(&m_mutex);
    m_currentIndex = CLAMP(index, -1, m_cues.size() - 1);
}

int CueStack::currentIndex() const
{
    return m_currentIndex;
}

void CueStack::previousCue()
{
    qDebug() << Q_FUNC_INFO;
    m_previous = true;
    if (isRunning() == false)
        start();
}

void CueStack::nextCue()
{
    qDebug() << Q_FUNC_INFO;
    m_next = true;
    if (isRunning() == false)
        start();
}

/****************************************************************************
 * Save & Load
 ****************************************************************************/

uint CueStack::loadXMLID(QXmlStreamReader &root)
{
    qDebug() << Q_FUNC_INFO;

    if (root.name() != KXMLQLCCueStack)
    {
        qWarning() << Q_FUNC_INFO << "CueStack node not found";
        return UINT_MAX;
    }

    QXmlStreamAttributes attrs = root.attributes();

    if (attrs.hasAttribute(KXMLQLCCueStackID) == true)
        return attrs.value(KXMLQLCCueStackID).toString().toUInt();
    else
        return UINT_MAX;
}

bool CueStack::loadXML(QXmlStreamReader &root)
{
    qDebug() << Q_FUNC_INFO;

    m_cues.clear();

    if (root.name() != KXMLQLCCueStack)
    {
        qWarning() << Q_FUNC_INFO << "CueStack node not found";
        return false;
    }

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCCue)
        {
            Cue cue;
            if (cue.loadXML(root) == true)
                appendCue(cue);
        }
        else if (root.name() == KXMLQLCCueStackSpeed)
        {
            setFadeInSpeed(root.attributes().value(KXMLQLCCueStackSpeedFadeIn).toString().toUInt());
            setFadeOutSpeed(root.attributes().value(KXMLQLCCueStackSpeedFadeOut).toString().toUInt());
            setDuration(root.attributes().value(KXMLQLCCueStackSpeedDuration).toString().toUInt());
            root.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unrecognized CueStack tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool CueStack::saveXML(QXmlStreamWriter *doc, uint id) const
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCCueStack);
    doc->writeAttribute(KXMLQLCCueStackID, QString::number(id));

    doc->writeStartElement(KXMLQLCCueStackSpeed);
    doc->writeAttribute(KXMLQLCCueStackSpeedFadeIn, QString::number(fadeInSpeed()));
    doc->writeAttribute(KXMLQLCCueStackSpeedFadeOut, QString::number(fadeOutSpeed()));
    doc->writeAttribute(KXMLQLCCueStackSpeedDuration, QString::number(duration()));
    doc->writeEndElement();

    foreach (Cue cue, cues())
        cue.saveXML(doc);

    /* End the <CueStack> tag */
    doc->writeEndElement();

    return true;
}

/****************************************************************************
 * Running
 ****************************************************************************/

void CueStack::start()
{
    qDebug() << Q_FUNC_INFO;
    m_running = true;
}

void CueStack::stop()
{
    qDebug() << Q_FUNC_INFO;
    m_running = false;
}

bool CueStack::isRunning() const
{
    return m_running;
}

void CueStack::adjustIntensity(qreal fraction)
{
    m_intensity = fraction;

    foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
    {
        if (!fader.isNull())
            fader->adjustIntensity(fraction);
    }
}

qreal CueStack::intensity() const
{
    return m_intensity;
}

/****************************************************************************
 * Flashing
 ****************************************************************************/

void CueStack::setFlashing(bool enable)
{
    qDebug() << Q_FUNC_INFO;
    if (m_flashing == enable || m_cues.isEmpty())
        return;

    m_flashing = enable;
    if (m_flashing == true)
        doc()->masterTimer()->registerDMXSource(this);
}

bool CueStack::isFlashing() const
{
    return m_flashing;
}

void CueStack::writeDMX(MasterTimer *timer, QList<Universe*> ua)
{
    Q_UNUSED(timer);
    if (m_cues.isEmpty())
        return;

    if (isFlashing())
    {
        if (m_fadersMap.isEmpty())
        {
            QHashIterator <uint,uchar> it(m_cues.first().values());
            while (it.hasNext() == true)
            {
                it.next();
                FadeChannel fc(doc(), Fixture::invalidId(), it.key());
                quint32 universe = fc.universe();
                if (universe == Universe::invalid())
                    continue;

                QSharedPointer<GenericFader> fader = m_fadersMap.value(universe, QSharedPointer<GenericFader>());
                if (fader.isNull())
                {
                    fader = ua[universe]->requestFader();
                    m_fadersMap[universe] = fader;
                }

                fc.setTarget(it.value());
                fc.addFlag(FadeChannel::Flashing);
                fader->add(fc);
            }
        }
    }
    else
    {
        QMapIterator <quint32, QSharedPointer<GenericFader> > it(m_fadersMap);
        while (it.hasNext() == true)
        {
            it.next();
            quint32 universe = it.key();
            QSharedPointer<GenericFader> fader = it.value();
            if (!fader.isNull())
                ua[universe]->dismissFader(fader);
        }

        m_fadersMap.clear();
        doc()->masterTimer()->unregisterDMXSource(this);
    }
}

/****************************************************************************
 * Writing
 ****************************************************************************/

bool CueStack::isStarted() const
{
    return m_fadersMap.isEmpty() ? false : true;
}

void CueStack::preRun()
{
    qDebug() << Q_FUNC_INFO;

    m_elapsed = 0;
    emit started();
}

void CueStack::write(QList<Universe*> ua)
{
    if (m_cues.size() == 0 || isRunning() == false)
        return;

    if (m_previous == true)
    {
        // previousCue() was requested by user
        m_elapsed = 0;
        int from = m_currentIndex;
        int to = previous();
        switchCue(from, to, ua);
        m_previous = false;
        emit currentCueChanged(m_currentIndex);
    }
    else if (m_next == true)
    {
        // nextCue() was requested by user
        m_elapsed = 0;
        int from = m_currentIndex;
        int to = next();
        switchCue(from, to, ua);
        m_next = false;
        emit currentCueChanged(m_currentIndex);
    }
/*
    else if (m_elapsed >= duration())
    {
        // Duration expired
        m_elapsed = 0;
        switchCue(next(), ua);
        emit currentCueChanged(m_currentIndex);
    }
*/
    //m_fader->write(ua);

    m_elapsed += MasterTimer::tick();
}

void CueStack::postRun(MasterTimer* timer, QList<Universe *> ua)
{
    qDebug() << Q_FUNC_INFO;

    Q_UNUSED(timer);

    /* If no fade out is needed, dismiss all the requested faders.
     * Otherwise, set all the faders to fade out and let Universe dismiss them
     * when done */
    if (fadeOutSpeed() == 0)
    {
        QMapIterator <quint32, QSharedPointer<GenericFader> > it(m_fadersMap);
        while (it.hasNext() == true)
        {
            it.next();
            quint32 universe = it.key();
            QSharedPointer<GenericFader> fader = it.value();
            if (!fader.isNull())
                ua[universe]->dismissFader(fader);
        }
    }
    else
    {
        foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
        {
            if (!fader.isNull())
                fader->setFadeOut(true, fadeOutSpeed());
        }
    }

    m_fadersMap.clear();

    m_currentIndex = -1;

    emit currentCueChanged(m_currentIndex);
    emit stopped();
}

int CueStack::previous()
{
    qDebug() << Q_FUNC_INFO;

    if (m_cues.size() == 0)
        return -1;

    QMutexLocker locker(&m_mutex);

    m_currentIndex--;
    if (m_currentIndex < 0)
        m_currentIndex = m_cues.size() - 1;

    return m_currentIndex;
}

FadeChannel *CueStack::getFader(QList<Universe *> universes, quint32 universeID, quint32 fixtureID, quint32 channel)
{
    // get the universe Fader first. If doesn't exist, create it
    QSharedPointer<GenericFader> fader = m_fadersMap.value(universeID, QSharedPointer<GenericFader>());
    if (fader.isNull())
    {
        fader = universes[universeID]->requestFader();
        fader->adjustIntensity(intensity());
        m_fadersMap[universeID] = fader;
    }

    return fader->getChannelFader(doc(), universes[universeID], fixtureID, channel);
}

void CueStack::updateFaderValues(FadeChannel *fc, uchar value, uint fadeTime)
{
    fc->setStart(fc->current());
    fc->setTarget(value);
    fc->setElapsed(0);
    fc->setReady(false);
    fc->setFadeTime(fadeTime);
}

int CueStack::next()
{
    qDebug() << Q_FUNC_INFO;

    if (m_cues.size() == 0)
        return -1;

    QMutexLocker locker(&m_mutex);
    m_currentIndex++;
    if (m_currentIndex >= m_cues.size())
        m_currentIndex = 0;

    return m_currentIndex;
}

void CueStack::switchCue(int from, int to, const QList<Universe *> ua)
{
    qDebug() << Q_FUNC_INFO;

    Cue newCue;
    Cue oldCue;

    {
        QMutexLocker locker(&m_mutex);

        if (to >= 0 && to < m_cues.size())
            newCue = m_cues[to];
        if (from >= 0 && from < m_cues.size())
            oldCue = m_cues[from];
    }

    // Fade out the HTP channels of the previous cue
    QHashIterator <uint,uchar> oldit(oldCue.values());
    while (oldit.hasNext() == true)
    {
        oldit.next();
        uint absChannel = oldit.key();
        quint32 universe = (absChannel >> 9);
        FadeChannel *fc = getFader(ua, universe, Fixture::invalidId(), absChannel);

        if (fc->flags() & FadeChannel::Intensity)
            updateFaderValues(fc, 0, oldCue.fadeOutSpeed());
    }

    // Fade in all channels of the new cue
    QHashIterator <uint,uchar> newit(newCue.values());
    while (newit.hasNext() == true)
    {
        newit.next();
        uint absChannel = newit.key();
        quint32 universe = (absChannel >> 9);
        FadeChannel *fc = getFader(ua, universe, Fixture::invalidId(), absChannel);
        updateFaderValues(fc, newit.value(), newCue.fadeInSpeed());
    }
}

