/*
  Q Light Controller
  monitorfixture.h

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

#ifndef MONITORFIXTURE_H
#define MONITORFIXTURE_H

#include <QFrame>
#include <QList>
#include <QFont>

#include "monitor.h"

class QByteArray;
class OutputMap;
class QFrame;
class QLabel;
class Doc;

/** \addtogroup ui_mon DMX Monitor
 * @{
 */

class MonitorFixture : public QFrame
{
    Q_OBJECT

public:
    MonitorFixture(QWidget* parent, Doc* doc);
    virtual ~MonitorFixture();

    /** Less-than operator for qSort() */
    bool operator<(const MonitorFixture& mof);

    /** Update channel and value label styles */
    void updateLabelStyles();

private:
    Doc* m_doc;

    /********************************************************************
     * Fixture
     ********************************************************************/
public:
    void setFixture(quint32 fxi_id);
    quint32 fixture() const;

public slots:
    void slotChannelStyleChanged(MonitorProperties::ChannelStyle style);

protected:
    quint32 m_fixture;
    MonitorProperties::ChannelStyle m_channelStyle;
    QLabel* m_fixtureLabel;
    QList <QLabel*> m_iconsLabels;
    QList <QLabel*> m_channelLabels;

    /********************************************************************
     * Values
     ********************************************************************/
public slots:
    void slotValueStyleChanged(MonitorProperties::ValueStyle style);
    void slotValuesChanged();

protected:
    QList <QLabel*> m_valueLabels;
    MonitorProperties::ValueStyle m_valueStyle;
};

/** @} */

#endif
