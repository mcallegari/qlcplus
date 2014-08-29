/*
  Q Light Controller
  cuestack.cpp

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
    , m_fader(NULL)
    , m_elapsed(0)
    , m_previous(false)
    , m_next(false)
{
    qDebug() << Q_FUNC_INFO << (void*) this;
    Q_ASSERT(doc != NULL);
}

CueStack::~CueStack()
{
    qDebug() << Q_FUNC_INFO << (void*) this;
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

    m_mutex.lock();
    m_cues.append(cue);
    int index = m_cues.size() - 1;
    m_mutex.unlock();

    emit added(index);
}

void CueStack::insertCue(int index, const Cue& cue)
{
    qDebug() << Q_FUNC_INFO;

    m_mutex.lock();
    if (index >= 0 && index < m_cues.size())
    {
        m_cues.insert(index, cue);
        emit added(index);

        if (m_currentIndex >= index)
        {
            m_currentIndex++;
            emit currentCueChanged(m_currentIndex);
        }

        m_mutex.unlock();
    }
    else
    {
        m_mutex.unlock();
        appendCue(cue);
    }
}

void CueStack::replaceCue(int index, const Cue& cue)
{
    qDebug() << Q_FUNC_INFO;

    m_mutex.lock();
    if (index >= 0 && index < m_cues.size())
    {
        m_cues[index] = cue;
        m_mutex.unlock();
        emit changed(index);
    }
    else
    {
        m_mutex.unlock();
        appendCue(cue);
    }
}

void CueStack::removeCue(int index)
{
    qDebug() << Q_FUNC_INFO;

    m_mutex.lock();
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
    m_mutex.unlock();
}

void CueStack::removeCues(const QList <int>& indexes)
{
    qDebug() << Q_FUNC_INFO;

    // Sort the list so that the items can be removed in reverse order.
    // This way, the indices are always correct.
    QList <int> indexList = indexes;
    qSort(indexList.begin(), indexList.end());

    m_mutex.lock();
    QListIterator <int> it(indexList);
    it.toBack();
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
    m_mutex.unlock();
}

QList <Cue> CueStack::cues() const
{
    return m_cues;
}

void CueStack::setCurrentIndex(int index)
{
    qDebug() << Q_FUNC_INFO;

    m_mutex.lock();
    m_currentIndex = CLAMP(index, -1, m_cues.size() - 1);
    m_mutex.unlock();
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

uint CueStack::loadXMLID(const QDomElement& root)
{
    qDebug() << Q_FUNC_INFO;

    if (root.tagName() != KXMLQLCCueStack)
    {
        qWarning() << Q_FUNC_INFO << "CueStack node not found";
        return UINT_MAX;
    }

    if (root.attribute(KXMLQLCCueStackID).isEmpty() == false)
        return root.attribute(KXMLQLCCueStackID).toUInt();
    else
        return UINT_MAX;
}

bool CueStack::loadXML(const QDomElement& root)
{
    qDebug() << Q_FUNC_INFO;

    m_cues.clear();

    if (root.tagName() != KXMLQLCCueStack)
    {
        qWarning() << Q_FUNC_INFO << "CueStack node not found";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCCue)
        {
            Cue cue;
            if (cue.loadXML(tag) == true)
                appendCue(cue);
        }
        else if (tag.tagName() == KXMLQLCCueStackSpeed)
        {
            setFadeInSpeed(tag.attribute(KXMLQLCCueStackSpeedFadeIn).toUInt());
            setFadeOutSpeed(tag.attribute(KXMLQLCCueStackSpeedFadeOut).toUInt());
            setDuration(tag.attribute(KXMLQLCCueStackSpeedDuration).toUInt());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unrecognized CueStack tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool CueStack::saveXML(QDomDocument* doc, QDomElement* wksp_root, uint id) const
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    QDomElement root = doc->createElement(KXMLQLCCueStack);
    root.setAttribute(KXMLQLCCueStackID, id);
    wksp_root->appendChild(root);

    QDomElement speed = doc->createElement(KXMLQLCCueStackSpeed);
    speed.setAttribute(KXMLQLCCueStackSpeedFadeIn, fadeInSpeed());
    speed.setAttribute(KXMLQLCCueStackSpeedFadeOut, fadeOutSpeed());
    speed.setAttribute(KXMLQLCCueStackSpeedDuration, duration());
    root.appendChild(speed);

    foreach (Cue cue, cues())
        cue.saveXML(doc, &root);

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
    if (m_fader != NULL)
        m_fader->adjustIntensity(fraction);
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
    if (m_flashing != enable && m_cues.size() > 0)
    {
        m_flashing = enable;
        if (m_flashing == true)
            doc()->masterTimer()->registerDMXSource(this, "CueStack");
        else
            doc()->masterTimer()->unregisterDMXSource(this);
    }
}

bool CueStack::isFlashing() const
{
    return m_flashing;
}

void CueStack::writeDMX(MasterTimer* timer, QList<Universe*> ua)
{
    Q_UNUSED(timer);
    if (isFlashing() == true && m_cues.size() > 0)
    {
        QHashIterator <uint,uchar> it(m_cues.first().values());
        while (it.hasNext() == true)
        {
            it.next();
            FadeChannel fc;
            fc.setChannel(it.key());
            fc.setTarget(it.value());
            int uni = qFloor(fc.channel() / 512);
            if (uni < ua.size())
                ua[uni]->write(fc.channel() - (uni * 512), fc.target());
        }
    }
}

/****************************************************************************
 * Writing
 ****************************************************************************/

bool CueStack::isStarted() const
{
    if (m_fader != NULL)
        return true;
    else
        return false;
}

void CueStack::preRun()
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(m_fader == NULL);
    m_fader = new GenericFader(doc());
    m_fader->adjustIntensity(intensity());
    m_elapsed = 0;
    emit started();
}

void CueStack::write(QList<Universe*> ua)
{
    Q_ASSERT(m_fader != NULL);

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
    m_fader->write(ua);

    m_elapsed += MasterTimer::tick();
}

void CueStack::postRun(MasterTimer* timer)
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(timer != NULL);
    Q_ASSERT(m_fader != NULL);

    // Bounce all intensity channels to MasterTimer's fader for zeroing
    QHashIterator <FadeChannel,FadeChannel> it(m_fader->channels());
    while (it.hasNext() == true)
    {
        it.next();
        FadeChannel fc = it.value();

        if (fc.group(doc()) == QLCChannel::Intensity)
        {
            fc.setStart(fc.current(intensity()));
            fc.setTarget(0);
            fc.setElapsed(0);
            fc.setReady(false);
            fc.setFadeTime(fadeOutSpeed());
            timer->fader()->add(fc);
        }
    }

    m_currentIndex = -1;
    delete m_fader;
    m_fader = NULL;

    emit currentCueChanged(m_currentIndex);
    emit stopped();
}

int CueStack::previous()
{
    qDebug() << Q_FUNC_INFO;

    if (m_cues.size() == 0)
        return -1;

    m_mutex.lock();
    m_currentIndex--;
    if (m_currentIndex < 0)
        m_currentIndex = m_cues.size() - 1;
    m_mutex.unlock();

    return m_currentIndex;
}

int CueStack::next()
{
    qDebug() << Q_FUNC_INFO;

    if (m_cues.size() == 0)
        return -1;

    m_mutex.lock();
    m_currentIndex++;
    if (m_currentIndex >= m_cues.size())
        m_currentIndex = 0;
    m_mutex.unlock();

    return m_currentIndex;
}

void CueStack::switchCue(int from, int to, const QList<Universe *> ua)
{
    qDebug() << Q_FUNC_INFO;

    Cue newCue;
    Cue oldCue;
    m_mutex.lock();
    if (to >= 0 && to < m_cues.size())
        newCue = m_cues[to];
    if (from >= 0 && from < m_cues.size())
        oldCue = m_cues[from];
    m_mutex.unlock();

    // Fade out the HTP channels of the previous cue
    QHashIterator <uint,uchar> oldit(oldCue.values());
    while (oldit.hasNext() == true)
    {
        oldit.next();

        FadeChannel fc;
        fc.setFixture(doc(), Fixture::invalidId());
        fc.setChannel(oldit.key());

        if (fc.group(doc()) == QLCChannel::Intensity)
        {
            fc.setElapsed(0);
            fc.setReady(false);
            fc.setTarget(0);
            fc.setFadeTime(oldCue.fadeOutSpeed());
            insertStartValue(fc, ua);
            m_fader->add(fc);
        }
    }

    // Fade in all channels of the new cue
    QHashIterator <uint,uchar> newit(newCue.values());
    while (newit.hasNext() == true)
    {
        newit.next();
        FadeChannel fc;

        fc.setFixture(doc(), Fixture::invalidId());
        fc.setChannel(newit.key());
        fc.setTarget(newit.value());
        fc.setElapsed(0);
        fc.setReady(false);
        fc.setFadeTime(newCue.fadeInSpeed());
        insertStartValue(fc, ua);
        m_fader->add(fc);
    }
}

void CueStack::insertStartValue(FadeChannel& fc, const QList<Universe *> ua)
{
    qDebug() << Q_FUNC_INFO;
    const QHash <FadeChannel,FadeChannel>& channels(m_fader->channels());
    if (channels.contains(fc) == true)
    {
        // GenericFader contains the channel so grab its current
        // value as the new starting value to get a smoother fade
        FadeChannel existing = channels[fc];
        fc.setStart(existing.current());
        fc.setCurrent(fc.start());
    }
    else
    {
        // GenericFader didn't have the channel. Grab the starting value from UniverseArray.
        quint32 uni = fc.universe();
        if (uni != Universe::invalid() && uni < (quint32)ua.count())
        {
            if (fc.group(doc()) != QLCChannel::Intensity)
                fc.setStart(ua[uni]->preGMValue(fc.address()));
            else
                fc.setStart(0); // HTP channels must start at zero
        }
        fc.setCurrent(fc.start());
    }
}
