/*
  Q Light Controller
  consolechannel.h

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

/** @addtogroup ui_simpledesk
 * @{
 */

class ConsoleChannel : public QGroupBox
{
    Q_OBJECT
    Q_DISABLE_COPY(ConsoleChannel)

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    ConsoleChannel(QWidget *parent, Doc* doc, quint32 fixture, quint32 channelIndex, bool isCheckable = true);

    ~ConsoleChannel();

private:
    void init();

protected:
    void showEvent(QShowEvent* ev);

private:
    QString m_styleSheet;

    /*************************************************************************
     * Fixture & Channel
     *************************************************************************/
public:
    /** Get the fixture that this channel belongs to */
    quint32 fixture() const;

    /** Get the channel number within $m_fixture that this channel controls */
    quint32 channelIndex() const;

    const QLCChannel *channel();

private:
    Doc* m_doc;
    quint32 m_fixture;
    quint32 m_chIndex;
    const QLCChannel *m_channel;
    /** this value is set only through setChannelsGroup,
     *  to emit the proper signal when value changes */
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
    void slotInputValueChanged(quint32 channelIndex, uchar value);

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
    void valueChanged(quint32 fxi, quint32 channelIndex, uchar value);

    /*************************************************************************
     * Look & Feel
     *************************************************************************/
public:
    void setChannelStyleSheet(const QString& styleSheet);
    void showResetButton(bool show);
    bool hasResetButton();

private slots:
    void slotResetButtonClicked();

signals:
    void checked(quint32 fxi, quint32 channelIndex, bool state);
    void resetRequest(quint32 fxi, quint32 channelIndex);

private:
    QToolButton* m_presetButton;
    ClickAndGoWidget *m_cngWidget;
    QSpinBox* m_spin;
    ClickAndGoSlider* m_slider;
    QLabel* m_label;
    QToolButton* m_resetButton;

    bool m_showResetButton;

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

/** @} */

#endif
