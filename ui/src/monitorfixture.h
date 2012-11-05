/*
  Q Light Controller
  monitorfixture.h

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
    void slotChannelStyleChanged(Monitor::ChannelStyle style);

protected:
    quint32 m_fixture;
    Monitor::ChannelStyle m_channelStyle;
    QLabel* m_fixtureLabel;
    QList <QLabel*> m_channelLabels;

    /********************************************************************
     * Values
     ********************************************************************/
public:
    void updateValues(const QByteArray& universes);

public slots:
    void slotValueStyleChanged(Monitor::ValueStyle style);

protected:
    QList <QLabel*> m_valueLabels;
    Monitor::ValueStyle m_valueStyle;
};

#endif
