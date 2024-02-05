/*
  Q Light Controller Plus
  cuestack.h

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

#ifndef CUESTACK_H
#define CUESTACK_H

#include <QObject>
#include <QMutex>
#include <QList>
#include <QMap>

#include "dmxsource.h"
#include "cue.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class UniverseArray;
class GenericFader;
class MasterTimer;
class FadeChannel;
class Doc;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCCueStack              QString("CueStack")
#define KXMLQLCCueStackID            QString("ID")
#define KXMLQLCCueStackSpeed         QString("Speed")
#define KXMLQLCCueStackSpeedFadeIn   QString("FadeIn")
#define KXMLQLCCueStackSpeedFadeOut  QString("FadeOut")
#define KXMLQLCCueStackSpeedDuration QString("Duration")

class CueStack : public QObject, public DMXSource
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    CueStack(Doc* doc);
    ~CueStack();

private:
    Doc* doc() const;

    /************************************************************************
     * Name
     ************************************************************************/
public:
    /**
     * Set either the name of the CueStack itself ($index = -1) or a Cue at
     * the given index ($index > -1).
     *
     * @param name The name to set
     * @param index -1 to set the name of the CueStack, anything else to set
     *              the name of a specific cue.
     */
    void setName(const QString& name, int index = -1);

    /**
     * Get either the name of the CueStack itself ($index = -1) or a Cue at
     * the given index ($index > -1).
     *
     * @param index -1 to get the name of the CueStack, anything else to get
     *              the name of a specific cue.
     * @return The name of the CueStack or a specific cue
     */
    QString name(int index = -1) const;

private:
    /** The name of the CueStack */
    QString m_name;

    /************************************************************************
     * Speed
     ************************************************************************/
public:
    /**
     * Set the fade in speed, either for the CueStack ($index = -1) or for a
     * specific cue at the given index ($index > -1).
     *
     * @param ms The fade in speed in milliseconds
     * @param index -1 to set the fade in speed for the CueStack or anything else
     *              to set the fade in speed for a specific cue.
     */
    void setFadeInSpeed(uint ms, int index = -1);

    /**
     * Get the fade in speed, either for the CueStack ($index = -1) or for a
     * specific cue at the given index ($index > -1).
     *
     * @param index -1 to get the fade in speed for the CueStack or anything else
     *              to get the fade in speed for a specific cue.
     * @return The fade in speed in milliseconds
     */
    uint fadeInSpeed(int index = -1) const;

    /**
     * Set the fade out speed, either for the CueStack ($index = -1) or for a
     * specific cue at the given index ($index > -1).
     *
     * @param ms The fade out speed in milliseconds
     * @param index -1 to set the fade out speed for the CueStack or anything else
     *              to set the fade out speed for a specific cue.
     */
    void setFadeOutSpeed(uint ms, int index = -1);

    /**
     * Get the fade out speed, either for the CueStack ($index = -1) or for a
     * specific cue at the given index ($index > -1).
     *
     * @param index -1 to get the fade out speed for the CueStack or anything else
     *              to get the fade out speed for a specific cue.
     * @return The fade out speed in milliseconds
     */
    uint fadeOutSpeed(int index = -1) const;

    /**
     * Set the duration, either for the CueStack ($index = -1) or for a
     * specific cue at the given index ($index > -1).
     *
     * @param ms The duration in milliseconds
     * @param index -1 to set the duration for the CueStack or anything else
     *              to set the duration for a specific cue.
     */
    void setDuration(uint ms, int index = -1);

    /**
     * Get the duration, either for the CueStack ($index = -1) or for a
     * specific cue at the given index ($index > -1).
     *
     * @param index -1 to get the duration for the CueStack or anything else
     *              to get the duration for a specific cue.
     * @return The duration in milliseconds
     */
    uint duration(int index = -1) const;

private:
    uint m_fadeInSpeed;
    uint m_fadeOutSpeed;
    uint m_duration;

    /************************************************************************
     * Cues
     ************************************************************************/
public:
    /** Append $cue at the end of the cue stack */
    void appendCue(const Cue& cue);

    /** Insert $cue at the given $index */
    void insertCue(int index, const Cue& cue);

    /** Replace the cue at the given $index with $cue */
    void replaceCue(int index, const Cue& cue);

    /** Remove the cue at the given $index */
    void removeCue(int index);

    /** Remove cues from the given $indexes */
    void removeCues(const QList <int>& indexes);

    /** Get a list of all cues */
    QList <Cue> cues() const;

signals:
    void added(int index);
    void removed(int index);
    void changed(int index);

private:
    QList <Cue> m_cues;
    QMutex m_mutex;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    static uint loadXMLID(QXmlStreamReader &root);
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc, uint id) const;

    /************************************************************************
     * Running
     ************************************************************************/
public:
    void start();
    void stop();
    bool isRunning() const;

    void setCurrentIndex(int index);
    int currentIndex() const;

    void previousCue();
    void nextCue();

    void adjustIntensity(qreal fraction);
    qreal intensity() const;

signals:
    void started();
    void stopped();
    void currentCueChanged(int index);

private:
    bool m_running;
    qreal m_intensity;
    int m_currentIndex;

    /************************************************************************
     * Flashing
     ************************************************************************/
public:
    void setFlashing(bool enable);
    bool isFlashing() const;

    void writeDMX(MasterTimer* timer, QList<Universe *> ua);

private:
    bool m_flashing;

    /************************************************************************
     * Writing
     ************************************************************************/
public:
    bool isStarted() const;

    void preRun();
    void write(QList<Universe *> ua);
    void postRun(MasterTimer *timer, QList<Universe *> ua);

private:
    int next();
    int previous();
    FadeChannel *getFader(QList<Universe *> universes, quint32 universeID, quint32 fixtureID, quint32 channel);
    void updateFaderValues(FadeChannel *fc, uchar value, uint fadeTime);
    void switchCue(int from, int to, const QList<Universe *> ua);

private:
    /** Map used to lookup a GenericFader instance for a Universe ID */
    QMap<quint32, QSharedPointer<GenericFader> > m_fadersMap;
    uint m_elapsed;
    bool m_previous;
    bool m_next;
};

/** @} */

#endif
