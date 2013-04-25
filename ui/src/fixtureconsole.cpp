/*
  Q Light Controller
  fixtureconsole.cpp

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

#include <QHBoxLayout>
#include <QDebug>
#include <QIcon>
#include <QList>
#include <QtXml>

#include "qlcfile.h"

#include "fixtureconsole.h"
#include "consolechannel.h"
#include "inputmap.h"
#include "fixture.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FixtureConsole::FixtureConsole(QWidget* parent, Doc* doc, GroupType type, bool showCheck)
    : QGroupBox(parent)
    , m_doc(doc)
    , m_groupType(type)
    , m_showCheckBoxes(showCheck)
    , m_fixture(Fixture::invalidId())
{
    Q_ASSERT(doc != NULL);

    m_layout = new QHBoxLayout(this);
    layout()->setSpacing(0);
    layout()->setContentsMargins(0, 1, 0, 1);
}

FixtureConsole::~FixtureConsole()
{
}

/*****************************************************************************
 * Fixture
 *****************************************************************************/

void FixtureConsole::setFixture(quint32 id)
{
    /* Get rid of any previous channels */
    while (m_channels.isEmpty() == false)
        delete m_channels.takeFirst();

    /* Get the new fixture */
    Fixture* fxi = m_doc->fixture(id);
    Q_ASSERT(fxi != NULL);
    if (m_groupType != GroupNone)
        setTitle(fxi->name());

    int topMargin = m_showCheckBoxes?16:1;

    QString common = "QGroupBox::title {top:-15px; left: 12px; subcontrol-origin: border; background-color: transparent; } "
                     "QGroupBox::indicator { width: 18px; height: 18px; } "
                     "QGroupBox::indicator:checked { image: url(:/checkbox_full.png) } "
                     "QGroupBox::indicator:unchecked { image: url(:/checkbox_empty.png) }";

    QString ssEven = QString("QGroupBox { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #C3D1C9, stop: 1 #AFBBB4); "
                             "border: 1px solid gray; border-radius: 4px; margin-top: %1px; margin-right: 1px; } " +
                             (m_showCheckBoxes?common:"")).arg(topMargin);

    QString ssOdd = QString("QGroupBox { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D6D5E0, stop: 1 #A7A6AF); "
                            "border: 1px solid gray; border-radius: 4px; margin-top: %1px; margin-right: 1px; } " +
                            (m_showCheckBoxes?common:"")).arg(topMargin);

    QString ssNone = QString("QGroupBox { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D6D2D0, stop: 1 #AFACAB); "
                             "border: 1px solid gray; border-radius: 4px; margin-top: %1px; margin-right: 1px; } " +
                             (m_showCheckBoxes?common:"")).arg(topMargin);

    /* Create channel units */
    for (uint i = 0; i < fxi->channels(); i++)
    {
        const QLCChannel* ch = fxi->channel(i);
        Q_ASSERT(ch != NULL);
        if (ch->group() == QLCChannel::NoGroup)
            continue;

        ConsoleChannel* cc = new ConsoleChannel(this, m_doc, id, i, m_showCheckBoxes);
        if (m_groupType == GroupNone)
            cc->setStyleSheet(ssNone);
        else if (m_groupType == GroupOdd)
            cc->setStyleSheet(ssOdd);
        else if (m_groupType == GroupEven)
            cc->setStyleSheet(ssEven);

        m_layout->addWidget(cc);
        m_channels.append(cc);
        connect(cc, SIGNAL(valueChanged(quint32,quint32,uchar)),
                this, SIGNAL(valueChanged(quint32,quint32,uchar)));
        connect(cc, SIGNAL(checked(quint32,quint32,bool)),
                this, SIGNAL(checked(quint32,quint32,bool)));
    }

    /* Make a spacer item eat excess space to justify channels left */
    m_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

    m_fixture = id;
}

quint32 FixtureConsole::fixture() const
{
    return m_fixture;
}

/*****************************************************************************
 * Channels
 *****************************************************************************/

void FixtureConsole::setChecked(bool state, quint32 channel)
{
    QListIterator <ConsoleChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        ConsoleChannel* cc = it.next();
        Q_ASSERT(cc != NULL);
        if (channel == UINT_MAX || channel == cc->channel())
            cc->setChecked(state);
    }
}

void FixtureConsole::setOutputDMX(bool state)
{
    Q_UNUSED(state);
    // TODO
}

void FixtureConsole::setSceneValue(const SceneValue& scv)
{
    Q_ASSERT(scv.fxi == m_fixture);

    QListIterator <ConsoleChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        ConsoleChannel* cc = it.next();
        Q_ASSERT(cc != NULL);
        if (cc->channel() == scv.channel)
        {
            cc->setChecked(true);
            cc->setValue(scv.value);
        }
    }
}

QList <SceneValue> FixtureConsole::values() const
{
    QList <SceneValue> list;
    QListIterator <ConsoleChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        ConsoleChannel* cc = it.next();
        Q_ASSERT(cc != NULL);
        if (cc->isChecked() == true)
            list.append(SceneValue(m_fixture, cc->channel(), cc->value()));
    }

    return list;
}

void FixtureConsole::setValues(const QList <SceneValue>& list)
{
    setChecked(false);

    QListIterator <SceneValue> it(list);
    while (it.hasNext() == true)
    {
        SceneValue val(it.next());
        if (val.channel < quint32(children().size()))
        {
            ConsoleChannel* cc = channel(val.channel);
            if (cc != NULL)
            {
                cc->setChecked(true);
                cc->setValue(val.value);
            }
        }
    }
}

void FixtureConsole::setValue(quint32 ch, uchar value)
{
    ConsoleChannel* cc = channel(ch);
    if (cc != NULL)
        cc->setValue(value);
}

uchar FixtureConsole::value(quint32 ch) const
{
    ConsoleChannel* cc = channel(ch);
    if (cc != NULL)
        return cc->value();
    else
        return 0;
}

ConsoleChannel* FixtureConsole::channel(quint32 ch) const
{
    QListIterator <ConsoleChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        ConsoleChannel* cc = it.next();
        Q_ASSERT(cc != NULL);
        if (cc->channel() == ch)
            return cc;
    }

    return NULL;
}
