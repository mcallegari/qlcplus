/*
  Q Light Controller
  speeddial.h

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

#ifndef SPEEDDIAL_H
#define SPEEDDIAL_H

#include <QGroupBox>
#include <QSpinBox>

class QElapsedTimer;
class QPushButton;
class QToolButton;
class QFocusEvent;
class QCheckBox;
class QLabel;
class QTimer;
class QDial;

/** @addtogroup ui UI
 * @{
 */

/****************************************************************************
 * FocusSpinBox
 ****************************************************************************/

/**
 * This is a normal QSpinBox that is able to tell, thru a signal, when it
 * gains the input focus (i.e. when it is clicked or tab-focused).
 */
class FocusSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    FocusSpinBox(QWidget* parent = 0);

signals:
    void focusGained();

protected:
    void focusInEvent(QFocusEvent* event);
};

/****************************************************************************
 * SpeedDial
 ****************************************************************************/

class SpeedDial : public QGroupBox
{
    Q_OBJECT
    Q_DISABLE_COPY(SpeedDial)

public:
    enum Visibility
    {
        None         = 0,
        PlusMinus    = 1 << 0,
        Dial         = 1 << 1,
        Tap          = 1 << 2,
        Hours        = 1 << 3,
        Minutes      = 1 << 4,
        Seconds      = 1 << 5,
        Milliseconds = 1 << 6,
        Infinite     = 1 << 7,
    };

    SpeedDial(QWidget* parent);
    ~SpeedDial();

    /**
     * Set the dial's current time value in milliseconds.
     *
     * @param ms Dial's value in milliseconds
     * @param emitValue If false (default), calling this function will not
     *                  cause the emission of valueChanged() signal with
     *                  the new value. If true, the new value is emitted.
     */
    void setValue(int ms, bool emitValue = false);
    int value() const;

    /** Produce a tap programmatically */
    void tap();

    void toggleInfinite();

    void stopTimers(bool stopTime = true, bool stopTapTimer = true);

    bool isTapTick();

signals:
    void valueChanged(int ms);
    void tapped();
    void tapTimeout();

    /*************************************************************************
     * Private
     *************************************************************************/
private:
    void updateTapTimer();
    void setSpinValues(int ms);
    int spinValues() const;

    /** Calculate the value to add/subtract when a dial has been moved */
    int dialDiff(int value, int previous, int step);

private slots:
    void slotPlusMinus();
    void slotPlusMinusTimeout();
    void slotDialChanged(int value);
    void slotHoursChanged();
    void slotMinutesChanged();
    void slotSecondsChanged();
    void slotMSChanged();
    void slotInfiniteChecked(bool state);
    void slotSpinFocusGained();
    void slotTapClicked();
    void slotTapTimeout();

private:
    QTimer* m_timer;
    QDial* m_dial;
    QToolButton* m_plus;
    QToolButton* m_minus;
    FocusSpinBox* m_hrs;
    FocusSpinBox* m_min;
    FocusSpinBox* m_sec;
    FocusSpinBox* m_ms;
    QCheckBox* m_infiniteCheck;
    QPushButton* m_tap;
    FocusSpinBox* m_focus;

    int m_previousDialValue;
    bool m_preventSignals;
    int m_value;
    QList<int> m_tapHistory;

    bool m_tapTick;
    QElapsedTimer* m_tapTime;
    QTimer* m_tapTickTimer;
    QTimer* m_tapTickElapseTimer;

    /*************************************************************************
     * Elements visibility
     *************************************************************************/
public:
    /** Return the widget's elements default visibility bitmask */
    static quint16 defaultVisibilityMask();

    /** Return the widget's elements visibility bitmask */
    quint16 visibilityMask();

    /** Set the visibility of the widget's elements
      * according to the provided bitmask */
    void setVisibilityMask(quint16 mask);

private:
    quint16 m_visibilityMask;
};

/** @} */

#endif
