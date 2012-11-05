/*
  Q Light Controller
  speeddial.h

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef SPEEDDIAL_H
#define SPEEDDIAL_H

#include <QGroupBox>
#include <QSpinBox>

class QPushButton;
class QToolButton;
class QFocusEvent;
class QCheckBox;
class QTimer;
class QDial;
class QTime;

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

signals:
    void valueChanged(int ms);
    void tapped();

    /*************************************************************************
     * Private
     *************************************************************************/
private:
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

    QTime* m_tapTime;
};

#endif
