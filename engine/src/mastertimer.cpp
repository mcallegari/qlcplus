/*
  Q Light Controller Plus
  mastertimer.cpp

  Copyright (C) Heikki Junnila
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

#include <QDebug>
#include <QSettings>
#include <QElapsedTimer>
#include <QMutexLocker>

#if defined(WIN32) || defined(Q_OS_WIN)
#   include "mastertimer-win32.h"
#else
#   include <unistd.h>
#   include "mastertimer-unix.h"
#endif

#include "inputoutputmap.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "dmxsource.h"
#include "qlcmacros.h"
#include "function.h"
#include "universe.h"
#include "doc.h"

#define MASTERTIMER_FREQUENCY "mastertimer/frequency"
#define LATE_TO_BEAT_THRESHOLD 25

/** The timer tick frequency in Hertz */
uint MasterTimer::s_frequency = 50;
uint MasterTimer::s_tick = 20;

//#define DEBUG_MASTERTIMER

#ifdef DEBUG_MASTERTIMER
quint64 ticksCount = 0;
#endif

/*****************************************************************************
 * Initialization
 *****************************************************************************/

MasterTimer::MasterTimer(Doc* doc)
    : QObject(doc)
    , d_ptr(new MasterTimerPrivate(this))
    , m_stopAllFunctions(false)
    , m_dmxSourceListMutex(QMutex::Recursive)
    , m_fader(new GenericFader(doc))
    , m_beatSourceType(None)
    , m_currentBPM(120)
    , m_beatTimeDuration(500)
    , m_beatRequested(false)
    , m_beatTimer(new QElapsedTimer())
    , m_lastBeatOffset(0)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(d_ptr != NULL);

    QSettings settings;
    QVariant var = settings.value(MASTERTIMER_FREQUENCY);
    if (var.isValid() == true)
        s_frequency = var.toUInt();

    s_tick = uint(double(1000) / double(s_frequency));
}

MasterTimer::~MasterTimer()
{
    if (d_ptr->isRunning() == true)
        stop();

    delete d_ptr;
    d_ptr = NULL;

    delete m_beatTimer;
}

void MasterTimer::start()
{
    Q_ASSERT(d_ptr != NULL);
    d_ptr->start();
}

void MasterTimer::stop()
{
    Q_ASSERT(d_ptr != NULL);
    stopAllFunctions();
    d_ptr->stop();
}

void MasterTimer::timerTick()
{
    Doc* doc = qobject_cast<Doc*> (parent());
    Q_ASSERT(doc != NULL);

#ifdef DEBUG_MASTERTIMER
    qDebug() << "[MasterTimer] *********** tick:" << ticksCount++ << "**********";
#endif

    switch (m_beatSourceType)
    {
        case Internal:
        {
            int elapsedTime = qRound((double)m_beatTimer->nsecsElapsed() / 1000000) + m_lastBeatOffset;
            //qDebug() << "Elapsed beat:" << elapsedTime;
            if (elapsedTime >= m_beatTimeDuration)
            {
                // it's time to fire a beat
                m_beatRequested = true;

                // restart the time for the next beat, starting at a delta
                // milliseconds, otherwise it will generate an unpleasant drift
                //qDebug() << "Elapsed:" << elapsedTime << ", delta:" << elapsedTime - m_beatTimeDuration;
                m_lastBeatOffset = elapsedTime - m_beatTimeDuration;
                m_beatTimer->restart();

                // inform the listening classes that a beat is happening
                emit beat();
            }
        }
        break;
        case External:
        break;

        case None:
        default:
            m_beatRequested = false;
        break;
    }

    QList<Universe *> universes = doc->inputOutputMap()->claimUniverses();
    for (int i = 0 ; i < universes.count(); i++)
    {
        universes[i]->flushInput();
        universes[i]->zeroIntensityChannels();
        universes[i]->zeroRelativeValues();
    }

    timerTickFunctions(universes);
    timerTickDMXSources(universes);
    timerTickFader(universes);

    doc->inputOutputMap()->releaseUniverses();
    doc->inputOutputMap()->dumpUniverses();

    m_beatRequested = false;
}

uint MasterTimer::frequency()
{
    return s_frequency;
}

uint MasterTimer::tick()
{
    return s_tick;
}

/*****************************************************************************
 * Functions
 *****************************************************************************/

void MasterTimer::startFunction(Function* function)
{
    if (function == NULL)
        return;

    QMutexLocker locker(&m_functionListMutex);
    if (m_startQueue.contains(function) == false)
        m_startQueue.append(function);
}

void MasterTimer::stopAllFunctions()
{
    m_stopAllFunctions = true;

    /* Wait until all functions have been stopped */
    while (runningFunctions() > 0)
    {
#if defined(WIN32) || defined(Q_OS_WIN)
        Sleep(10);
#else
        usleep(10000);
#endif
    }

    // WARNING: the following brackets are fundamental for
    // the scope of this piece of code !!
    {
        /* Remove all generic fader's channels */
        QMutexLocker faderLocker(&m_faderMutex);
        fader()->removeAll();
    }

    m_stopAllFunctions = false;
}

void MasterTimer::fadeAndStopAll(int timeout)
{
    if (timeout == 0)
        return;

    Doc* doc = qobject_cast<Doc*> (parent());
    Q_ASSERT(doc != NULL);

    QList<FadeChannel> fcList;

    QList<Universe *> universes = doc->inputOutputMap()->claimUniverses();
    for (int i = 0; i < universes.count(); i++)
    {
        QHashIterator <int,uchar> it(universes[i]->intensityChannels());
        while (it.hasNext() == true)
        {
            it.next();

            Fixture* fxi = doc->fixture(doc->fixtureForAddress(it.key()));
            if (fxi != NULL)
            {
                uint ch = it.key() - fxi->universeAddress();
                if (fxi->channelCanFade(ch))
                {
                    FadeChannel fc(doc, fxi->id(), ch);
                    fc.setStart(it.value());
                    fc.setTarget(0);
                    fc.setFadeTime(timeout);
                    fcList.append(fc);
                }
            }
        }
    }
    doc->inputOutputMap()->releaseUniverses();

    // Stop all functions first
    stopAllFunctions();

    // Instruct mastertimer to do a fade out of all
    // the intensity channels that can fade
    QMutexLocker faderLocker(&m_faderMutex);
    foreach(FadeChannel fade, fcList)
        fader()->add(fade);
}

int MasterTimer::runningFunctions() const
{
    return m_functionList.size();
}

void MasterTimer::timerTickFunctions(QList<Universe *> universes)
{
    // List of m_functionList indices that should be removed at the end of this
    // function. The functions at the indices have been stopped.
    QList<int> removeList;

    bool functionListHasChanged = false;
    bool stoppedAFunction = true;
    bool firstIteration = true;

    while (stoppedAFunction)
    {
        stoppedAFunction = false;
        removeList.clear();

        for (int i = 0; i < m_functionList.size(); i++)
        {
            Function* function = m_functionList.at(i);

            if (function != NULL)
            {
                /* Run the function unless it's supposed to be stopped */
                if (function->stopped() == false && m_stopAllFunctions == false)
                {
                    if (firstIteration)
                        function->write(this, universes);
                }
                else
                {
                    // Clear function's parentList
                    if (m_stopAllFunctions)
                        function->stop(FunctionParent::master());
                    /* Function should be stopped instead */
                    function->postRun(this, universes);
                    //qDebug() << "[MasterTimer] Add function (ID: " << function->id() << ") to remove list ";
                    removeList << i; // Don't remove the item from the list just yet.
                    functionListHasChanged = true;
                    stoppedAFunction = true;
                }
            }
        }

        // Remove functions that need to be removed AFTER all functions have been run
        // for this round. This is done separately to prevent a case when a function
        // is first removed and then another is added (chaser, for example), keeping the
        // list's size the same, thus preventing the last added function from being run
        // on this round. The indices in removeList are automatically sorted because the
        // list is iterated with an int above from 0 to size, so iterating the removeList
        // backwards here will always remove the correct indices.
        QListIterator <int> it(removeList);
        it.toBack();
        while (it.hasPrevious() == true)
            m_functionList.removeAt(it.previous());

        firstIteration = false;
    }

    {
        QMutexLocker locker(&m_functionListMutex);
        while (m_startQueue.size() > 0)
        {
            QList<Function*> startQueue(m_startQueue);
            m_startQueue.clear();
            locker.unlock();

            foreach (Function* f, startQueue)
            {
                if (m_functionList.contains(f))
                {
                    f->postRun(this, universes);
                }
                else
                {
                    m_functionList.append(f);
                    functionListHasChanged = true;
                }
                f->preRun(this);
                f->write(this, universes);
                emit functionStarted(f->id());
            }

            locker.relock();
        }
    }

    if (functionListHasChanged)
        emit functionListChanged();
}

/****************************************************************************
 * DMX Sources
 ****************************************************************************/

void MasterTimer::registerDMXSource(DMXSource* source)
{
    Q_ASSERT(source != NULL);

    QMutexLocker lock(&m_dmxSourceListMutex);
    if (m_dmxSourceList.contains(source) == false)
    {
        int insertPos = 0;

        for (int i = m_dmxSourceList.count() - 1; i >= 0; i--)
        {
            DMXSource *src = m_dmxSourceList.at(i);
            if (src->priority() <= source->priority())
            {
                insertPos = i + 1;
                break;
            }
        }

        m_dmxSourceList.insert(insertPos, source);
        qDebug() << "DMX source with priority" <<  source->priority() << "registered at pos" << insertPos;
    }
}

void MasterTimer::unregisterDMXSource(DMXSource* source)
{
    Q_ASSERT(source != NULL);

    QMutexLocker lock(&m_dmxSourceListMutex);
    m_dmxSourceList.removeAll(source);
}

void MasterTimer::requestNewPriority(DMXSource *source)
{
    Q_ASSERT(source != NULL);
    QMutexLocker lock(&m_dmxSourceListMutex);
    if (m_dmxSourceList.contains(source) == true)
    {
       int pos = m_dmxSourceList.indexOf(source);
       int newPos = 0;

       for (int i = m_dmxSourceList.count() - 1; i >= 0; i--)
       {
           DMXSource *src = m_dmxSourceList.at(i);
           if (src->priority() <= source->priority())
           {
               newPos = i;
               break;
           }
       }

       m_dmxSourceList.move(pos, newPos);
       qDebug() << "DMX source moved from" << pos << "to" << m_dmxSourceList.indexOf(source) << ". Count:" << m_dmxSourceList.count();
    }

}

void MasterTimer::timerTickDMXSources(QList<Universe *> universes)
{
    /* Lock before accessing the DMX sources list. */
    QMutexLocker lock(&m_dmxSourceListMutex);

    foreach (DMXSource* source, m_dmxSourceList)
    {
        Q_ASSERT(source != NULL);

#ifdef DEBUG_MASTERTIMER
        qDebug() << "[MasterTimer] ticking DMX source" << i;
#endif

        /* Get DMX data from the source */
        source->writeDMX(this, universes);
    }
}

/****************************************************************************
 * Generic Fader
 ****************************************************************************/

GenericFader* MasterTimer::fader() const
{
    return m_fader;
}

void MasterTimer::faderAdd(const FadeChannel& ch)
{
    QMutexLocker faderLocker(&m_faderMutex);

    fader()->add(ch);
}

void MasterTimer::faderForceAdd(const FadeChannel& ch)
{
    QMutexLocker faderLocker(&m_faderMutex);

    fader()->forceAdd(ch);
}

QHash<FadeChannel,FadeChannel> MasterTimer::faderChannels() const
{
    QMutexLocker faderLocker(const_cast<QMutex*>(&m_faderMutex));

    return fader()->channels();
}

QHash<FadeChannel,FadeChannel> const& MasterTimer::faderChannelsRef() const
{
    return fader()->channels();
}

QMutex* MasterTimer::faderMutex() const
{
    return const_cast<QMutex*>(&m_faderMutex);
}

void MasterTimer::timerTickFader(QList<Universe *> universes)
{
    QMutexLocker faderLocker(&m_faderMutex);

#ifdef DEBUG_MASTERTIMER
        qDebug() << "[MasterTimer] ticking fader (channels:" << fader()->channels().count() << ")";
#endif

        fader()->write(universes);
}

/*************************************************************************
 * Beats generation
 *************************************************************************/

void MasterTimer::setBeatSourceType(MasterTimer::BeatsSourceType type)
{
    if (type == m_beatSourceType)
        return;

    // alright, this causes a time drift of maximum 1ms per beat
    // but at the moment I am not looking for a better solution
    m_beatTimeDuration = 60000 / m_currentBPM;
    m_beatTimer->restart();

    m_beatSourceType = type;
}

MasterTimer::BeatsSourceType MasterTimer::beatSourceType() const
{
    return m_beatSourceType;
}

void MasterTimer::requestBpmNumber(int bpm)
{
    if (bpm == m_currentBPM)
        return;

    m_currentBPM = bpm;
    m_beatTimeDuration = 60000 / m_currentBPM;
    m_beatTimer->restart();

    emit bpmNumberChanged(bpm);
}

int MasterTimer::bpmNumber() const
{
    return m_currentBPM;
}

int MasterTimer::beatTimeDuration() const
{
    return m_beatTimeDuration;
}

int MasterTimer::timeToNextBeat() const
{
    return m_beatTimeDuration - m_beatTimer->elapsed();
}

int MasterTimer::nextBeatTimeOffset() const
{
    // get the time offset to the next beat
    int toNext = timeToNextBeat();
    // get the percentage of beat time passed
    int beatPercentage = (100 * toNext) / m_beatTimeDuration;

    // if a Function has been started within the first LATE_TO_BEAT_THRESHOLD %
    // of a beat, then it means it is "late" but there's
    // no need to wait a whole beat
    if (beatPercentage <= LATE_TO_BEAT_THRESHOLD)
        return toNext;

    // otherwise we're running early, so we should wait the
    // whole remaining time
    return -toNext;
}

bool MasterTimer::isBeat() const
{
    return m_beatRequested;
}

void MasterTimer::requestBeat()
{
    // forceful request of a beat, processed at
    // the next timerTick call
    m_beatRequested = true;
}
