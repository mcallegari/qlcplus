/*
  Q Light Controller
  consolechannel.h

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

#ifndef CONSOLECHANNEL_H
#define CONSOLECHANNEL_H

#include <QGroupBox>
#include <QMutex>
#include <QIcon>

#include "clickandgoslider.h"
#include "clickandgowidget.h"
#include "qlcchannel.h"
#include "dmxsource.h"

class QContextMenuEvent;
class QIntValidator;
class QToolButton;
class QSpinBox;
class QLabel;
class QMenu;

class QLCChannel;
class Doc;

class ConsoleChannel : public QGroupBox
{
    Q_OBJECT
    Q_DISABLE_COPY(ConsoleChannel)

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    ConsoleChannel(QWidget *parent, Doc* doc, quint32 fixture, quint32 channel, bool isCheckable = true);

    ~ConsoleChannel();

private:
    void init();

    /*************************************************************************
     * Fixture & Channel
     *************************************************************************/
public:
    /** Get the fixture that this channel belongs to */
    quint32 fixture() const;

    /** Get the channel number within $m_fixture that this channel controls */
    quint32 channel() const;

private:
    Doc* m_doc;
    quint32 m_fixture;
    quint32 m_channel;
    /** this value is set only through setChannelsGroup, to emit the proper signal when value changes */
    quint32 m_group;

    /*************************************************************************
     * Group
     *************************************************************************/
public:
    /** Set the channel's label */
    void setLabel(QString label);

    /** Set the group ID this channel controls */
    void setChannelsGroup(quint32 grpid);

private slots:
    void slotInputValueChanged(quint32 channel, uchar value);

signals:
    void groupValueChanged(quint32 group, uchar value);

    /*************************************************************************
     * Value
     *************************************************************************/
public:
    /** Set the channel's current value */
    void setValue(uchar value, bool apply = true);

    /** Get the channel's current value */
    uchar value() const;

private slots:
    void slotSpinChanged(int value);
    void slotSliderChanged(int value);
    void slotChecked(bool state);

signals:
    void valueChanged(quint32 fxi, quint32 channel, uchar value);
    void checked(quint32 fxi, quint32 channel, bool state);

private:
    QToolButton* m_presetButton;
    ClickAndGoWidget *m_cngWidget;
    QSpinBox* m_spin;
    ClickAndGoSlider* m_slider;
    QLabel* m_label;

    /*************************************************************************
     * Menu
     *************************************************************************/
private slots:
    void slotContextMenuTriggered(QAction* action);
    void slotClickAndGoLevelChanged(uchar level);
    void slotClickAndGoLevelAndPresetChanged(uchar level, QImage img);

private:
    void contextMenuEvent(QContextMenuEvent*);
    void initMenu();
    void initCapabilityMenu(const QLCChannel* ch);
    void setIntensityButton(const QLCChannel* ch);
    static QIcon colorIcon(const QString& name);

private:
    QMenu* m_menu;

    /*************************************************************************
     * Selection
     *************************************************************************/
public:
    bool isSelected();

private slots:
    void slotControlClicked();

private:
    bool m_selected;
    QString m_originalStyle;
};

#endif
