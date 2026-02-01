/*
  Q Light Controller
  monitorfixture.cpp

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

#include <QGridLayout>
#include <QByteArray>
#include <QString>
#include <QFrame>
#include <QLabel>
#include <QDebug>
#include <QFont>
#include <cmath>

#include "monitorfixture.h"
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
    m_channelStyle = MonitorProperties::DMXChannels;
    m_valueStyle = MonitorProperties::DMXValues;

    new QGridLayout(this);
    layout()->setContentsMargins(3, 3, 3, 3);

    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Window);
}

MonitorFixture::~MonitorFixture()
{
    if (m_fixture != Fixture::invalidId())
    {
        Fixture* fxi = m_doc->fixture(m_fixture);
        if (fxi != NULL)
            disconnect(fxi, SIGNAL(valuesChanged()), this, SLOT(slotValuesChanged()));
    }

    if (m_fixtureLabel != NULL)
        delete m_fixtureLabel;

    while (m_iconsLabels.isEmpty() == false)
        delete m_iconsLabels.takeFirst();
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
    while (m_iconsLabels.isEmpty() == false)
        delete m_iconsLabels.takeFirst();
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
        lay->setVerticalSpacing(1);

        /* Create a new fixture label and set the fixture name there */
        m_fixtureLabel = new QLabel(this);
        m_fixtureLabel->setText(QString("<B>%1</B>").arg(fxi->name()));

        /* Set the fixture name to span all channels horizontally */
        lay->addWidget(m_fixtureLabel, 0, 0, 1, fxi->channels(),
                       Qt::AlignLeft);

        QByteArray fxValues = fxi->channelValues();

        /* Create channel numbers and value labels */
        for (quint32 i = 0; i < fxi->channels(); i++)
        {
            const QLCChannel * channel = fxi->channel(i);
            /* Create the icon over the channel number */
            QLabel *icon = new QLabel(this);
            icon->setFixedSize(22, 22);

            /* Create a label for channel number */
            QLabel *label = new QLabel(this);

            if (channel != NULL)
            {
                icon->setToolTip(channel->name());
                label->setToolTip(channel->name());
                QString resStr = channel->getIconNameFromGroup(channel->group());

                if (resStr.startsWith(":"))
                    icon->setStyleSheet("QLabel { border-image: url(" + resStr + ") 0 0 0 0 stretch stretch; }");
                else
                    icon->setStyleSheet("QLabel { background: " + resStr + "; }");
            }
            lay->addWidget(icon, 1, i, Qt::AlignHCenter);
            lay->addWidget(label, 2, i, Qt::AlignHCenter);
            m_iconsLabels.append(icon);
            m_channelLabels.append(label);

            /* Create a label for value */
            QString str;
            label = new QLabel(this);
            lay->addWidget(label, 3, i, Qt::AlignHCenter);
            label->setText(str.asprintf("%.3d", uchar(fxValues.at(i))));
            m_valueLabels.append(label);
        }
        connect(fxi, SIGNAL(valuesChanged()), this, SLOT(slotValuesChanged()));
    }
}

quint32 MonitorFixture::fixture() const
{
    return m_fixture;
}

void MonitorFixture::slotChannelStyleChanged(MonitorProperties::ChannelStyle style)
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
    if (style == MonitorProperties::DMXChannels)
        i = fxi->address() + 1;
    else
        i = 1;

    QListIterator <QLabel*> it(m_channelLabels);
    while (it.hasNext() == true)
        it.next()->setText(str.asprintf("<B>%.3d</B>", i++));
}

/****************************************************************************
 * Values
 ****************************************************************************/

void MonitorFixture::slotValueStyleChanged(MonitorProperties::ValueStyle style)
{
    if (m_valueStyle == style)
        return;

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

        if (style == MonitorProperties::DMXValues)
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

        label->setText(str.asprintf("%.3d", value));
    }
}

void MonitorFixture::slotValuesChanged()
{
    /* Check that this MonitorFixture represents a fixture */
    if (m_fixture == Fixture::invalidId())
        return;

    /* Check that this MonitorFixture's fixture really exists */
    Fixture* fxi = m_doc->fixture(m_fixture);
    if (fxi == NULL)
        return;

    QByteArray fxValues = fxi->channelValues();
    int i = 0;

    QListIterator <QLabel*> it(m_valueLabels);
    while (it.hasNext() == true)
    {
        QLabel* label = it.next();
        Q_ASSERT(label != NULL);
        QString str;

        /* Set the label's text to reflect the changed value */
        if (m_valueStyle == MonitorProperties::DMXValues)
        {
            label->setText(str.asprintf("%.3d", uchar(fxValues.at(i))));
        }
        else
        {
            label->setText(str.asprintf("%.3d", int(ceil(SCALE(qreal(uchar(fxValues.at(i))),
                                                               qreal(0), qreal(UCHAR_MAX),
                                                               qreal(0), qreal(100))))));
        }
        i++;
    }
}
