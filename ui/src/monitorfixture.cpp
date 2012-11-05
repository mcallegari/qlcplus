/*
  Q Light Controller
  monitorfixture.cpp

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

#include <QGridLayout>
#include <QByteArray>
#include <QString>
#include <QFrame>
#include <QLabel>
#include <QDebug>
#include <QFont>
#include <cmath>

#include "monitorfixture.h"
#include "outputpatch.h"
#include "outputmap.h"
#include "qlcmacros.h"
#include "fixture.h"
#include "doc.h"

MonitorFixture::MonitorFixture(QWidget* parent, Doc* doc)
    : QFrame(parent)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);

    m_fixtureLabel = NULL;
    m_fixture = Fixture::invalidId();
    m_channelStyle = Monitor::DMXChannels;
    m_valueStyle = Monitor::DMXValues;

    new QGridLayout(this);
    layout()->setMargin(3);

    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Window);
}

MonitorFixture::~MonitorFixture()
{
    if (m_fixtureLabel != NULL)
        delete m_fixtureLabel;

    while (m_channelLabels.isEmpty() == false)
        delete m_channelLabels.takeFirst();
    while (m_valueLabels.isEmpty() == false)
        delete m_valueLabels.takeFirst();
}

bool MonitorFixture::operator<(const MonitorFixture& mof)
{
    Fixture* fxi;
    Fixture* mof_fxi;

    fxi = m_doc->fixture(m_fixture);
    if (fxi == NULL)
        return false;

    mof_fxi = m_doc->fixture(mof.fixture());
    if (mof_fxi == NULL)
        return false;

    if ((*fxi) < (*mof_fxi))
        return true;
    else
        return false;
}

void MonitorFixture::updateLabelStyles()
{
    slotChannelStyleChanged(m_channelStyle);
    slotValueStyleChanged(m_valueStyle);
}

/****************************************************************************
 * Fixture
 ****************************************************************************/

void MonitorFixture::setFixture(quint32 fxi_id)
{
    Fixture* fxi;

    /* Get rid of old stuff first, if such exists */
    if (m_fixtureLabel != NULL)
        delete m_fixtureLabel;
    while (m_channelLabels.isEmpty() == false)
        delete m_channelLabels.takeFirst();
    while (m_valueLabels.isEmpty() == false)
        delete m_valueLabels.takeFirst();

    m_fixture = fxi_id;
    fxi = m_doc->fixture(m_fixture);
    if (fxi != NULL)
    {
        /* The grid layout uses columns and rows. The first row is for
           the fixture name, second row for channel numbers and the
           third row for channel values. Each channel is in its own
           column. */
        QGridLayout* lay = qobject_cast<QGridLayout*> (layout());

        /* Create a new fixture label and set the fixture name there */
        m_fixtureLabel = new QLabel(this);
        m_fixtureLabel->setText(QString("<B>%1</B>").arg(fxi->name()));

        /* Set the fixture name to span all channels horizontally */
        lay->addWidget(m_fixtureLabel, 0, 0, 1, fxi->channels(),
                       Qt::AlignLeft);

        /* Create channel numbers and value labels */
        for (quint32 i = 0; i < fxi->channels(); i++)
        {
            QLabel* label;
            QString str;

            /* Create a label for channel number */
            label = new QLabel(this);
            lay->addWidget(label, 1, i, Qt::AlignHCenter);
            m_channelLabels.append(label);

            /* Create a label for value */
            label = new QLabel(this);
            lay->addWidget(label, 2, i, Qt::AlignHCenter);
            m_valueLabels.append(label);
        }
    }
}

quint32 MonitorFixture::fixture() const
{
    return m_fixture;
}

void MonitorFixture::slotChannelStyleChanged(Monitor::ChannelStyle style)
{
    QString str;
    int i = 0;

    m_channelStyle = style;

    /* Check that this MonitorFixture represents a fixture */
    if (m_fixture == Fixture::invalidId())
        return;

    Fixture* fxi = m_doc->fixture(m_fixture);
    Q_ASSERT(fxi != NULL);

    /* Start channel numbering from this fixture's address */
    if (style == Monitor::DMXChannels)
        i = fxi->address() + 1;
    else
        i = 1;

    QListIterator <QLabel*> it(m_channelLabels);
    while (it.hasNext() == true)
        it.next()->setText(str.sprintf("<B>%.3d</B>", i++));
}

/****************************************************************************
 * Values
 ****************************************************************************/

void MonitorFixture::updateValues(const QByteArray& ua)
{
    QLabel* label;
    uchar value;
    Fixture* fxi;
    QString str;
    int i = 0;

    /* Check that this MonitorFixture represents a fixture */
    if (m_fixture == Fixture::invalidId())
        return;

    /* Check that this MonitorFixture's fixture really exists */
    fxi = m_doc->fixture(m_fixture);
    if (fxi == NULL)
        return;

    QListIterator <QLabel*> it(m_valueLabels);
    while (it.hasNext() == true)
    {
        label = it.next();
        Q_ASSERT(label != NULL);

        value = uchar(ua.at(fxi->universeAddress() + i));
        i++;

        /* Set the label's text to reflect the changed value */
        if (m_valueStyle == Monitor::DMXValues)
        {
            label->setText(str.sprintf("%.3d", value));
        }
        else
        {
            label->setText(str.sprintf("%.3d", int(ceil(SCALE(qreal(value),
                                                              qreal(0), qreal(UCHAR_MAX),
                                                              qreal(0), qreal(100))))));
        }
    }
}

void MonitorFixture::slotValueStyleChanged(Monitor::ValueStyle style)
{
    m_valueStyle = style;

    QListIterator <QLabel*> it(m_valueLabels);
    while (it.hasNext() == true)
    {
        QLabel* label;
        QString str;
        int value;

        label = it.next();
        Q_ASSERT(label != NULL);

        value = label->text().toInt();

        if (style == Monitor::DMXValues)
        {
            value = int(ceil(SCALE(qreal(value),
                                   qreal(0), qreal(100),
                                   qreal(0), qreal(UCHAR_MAX))));
        }
        else
        {
            value = int(ceil(SCALE(qreal(value),
                                   qreal(0), qreal(UCHAR_MAX),
                                   qreal(0), qreal(100))));
        }

        label->setText(str.sprintf("%.3d", value));
    }
}
