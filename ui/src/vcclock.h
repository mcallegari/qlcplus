/*
  Q Light Controller
  vcclock.h

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

#ifndef VCCLOCK_H
#define VCCLOCK_H

#include "vcwidget.h"

class QDomDocument;
class QDomElement;
class QPaintEvent;
class InputMap;
class Doc;

#define KXMLQLCVCClock "Clock"

class VCClock : public VCWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VCClock)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCClock(QWidget* parent, Doc* doc);
    ~VCClock();

    /*********************************************************************
     * Type
     *********************************************************************/
public:
    enum ClockType
    {
        Clock,
        Stopwatch,
        Countdown
    };

    void setClockType(ClockType type);
    ClockType clockType();

    QString typeToString(ClockType type);
    ClockType stringToType(QString str);

private:
    ClockType m_clocktype;

    /*********************************************************************
     * Time
     *********************************************************************/

public:
    void setCountdown(int h, int m, int s);
    long currentTime() { return m_currentTime; }
    void resetTime();
    int getHours() { return m_hh; }
    int getMinutes() { return m_mm; }
    int getSeconds() { return m_ss; }

protected slots:
    void slotUpdateTime();

private:
    int m_hh, m_mm, m_ss;
    quint32 m_targetTime;
    quint32 m_currentTime;
    bool m_isPaused;

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    VCWidget* createCopy(VCWidget* parent);

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    void editProperties();

    /*****************************************************************************
     * External input
     *****************************************************************************/
    /** @reimp */
    void updateFeedback() { }

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXML(const QDomElement* root);
    bool saveXML(QDomDocument* doc, QDomElement* vc_root);

    /*********************************************************************
     * Painting
     *********************************************************************/
protected:
    void paintEvent(QPaintEvent* e);
    void mousePressEvent(QMouseEvent* e);
};

#endif

