/*
  Q Light Controller
  fixtureconsole.cpp

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

#include <QHBoxLayout>
#include <QDebug>
#include <QIcon>
#include <QList>

#include "fixtureconsole.h"
#include "consolechannel.h"
#include "fixture.h"
#include "apputil.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FixtureConsole::FixtureConsole(QWidget *parent, Doc *doc, GroupType type, bool showCheck)
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

    int topMargin = m_showCheckBoxes ? 16 : 1;

    QString ssFcCommon = "QGroupBox::title {top:-15px; left: 12px; subcontrol-origin: border; background-color: transparent; } "
                         "QGroupBox::indicator { width: 18px; height: 18px; } "
                         "QGroupBox::indicator:checked { image: url(:/checkbox_full.png) } "
                         "QGroupBox::indicator:unchecked { image: url(:/checkbox_empty.png) }";

    QString ssFcEven = "QGroupBox { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #C3D1C9, stop: 1 #AFBBB4); "
                       "border: 1px solid gray; border-radius: 4px; margin-top: %1px; margin-right: 1px; } ";

    QString ssFcOdd = "QGroupBox { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D6D5E0, stop: 1 #A7A6AF); "
                      "border: 1px solid gray; border-radius: 4px; margin-top: %1px; margin-right: 1px; } ";

    QString ssFcNone = "QGroupBox { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D6D2D0, stop: 1 #AFACAB); "
                       "border: 1px solid gray; border-radius: 4px; margin-top: %1px; margin-right: 1px; } ";

    QString userStyle = AppUtil::getStyleSheet("FIXTURE_CONSOLE_COMMON");
    if (!userStyle.isEmpty())
        ssFcCommon = userStyle;

    if (m_groupType == GroupEven)
    {
        userStyle = AppUtil::getStyleSheet("FIXTURE_CONSOLE_EVEN");
        if (!userStyle.isEmpty())
            ssFcEven = userStyle;
        m_styleSheet = QString(ssFcEven + (m_showCheckBoxes ? ssFcCommon : "")).arg(topMargin);
    }
    else if (m_groupType == GroupOdd)
    {
        userStyle = AppUtil::getStyleSheet("FIXTURE_CONSOLE_ODD");
        if (!userStyle.isEmpty())
            ssFcOdd = userStyle;
        m_styleSheet = QString(ssFcOdd + (m_showCheckBoxes ? ssFcCommon : "")).arg(topMargin);
    }
    else
    {
        userStyle = AppUtil::getStyleSheet("FIXTURE_CONSOLE_NORMAL");
        if (!userStyle.isEmpty())
            ssFcNone = userStyle;
        m_styleSheet = QString(ssFcNone + (m_showCheckBoxes ? ssFcCommon : "")).arg(topMargin);
    }
}

FixtureConsole::~FixtureConsole()
{
}

void FixtureConsole::enableResetButton(bool enable)
{
    QListIterator <ConsoleChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        ConsoleChannel* cc = it.next();
        Q_ASSERT(cc != NULL);
        cc->showResetButton(enable);
        connect(cc, SIGNAL(resetRequest(quint32,quint32)),
                this, SIGNAL(resetRequest(quint32,quint32)));
    }
}

void FixtureConsole::showEvent(QShowEvent *)
{
    QListIterator <ConsoleChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        ConsoleChannel* cc = it.next();
        Q_ASSERT(cc != NULL);
        cc->setVisible(true);
    }
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
    Fixture *fxi = m_doc->fixture(id);
    Q_ASSERT(fxi != NULL);
    if (m_groupType != GroupNone)
        setTitle(fxi->name());

    /* Create channel units */
    for (uint i = 0; i < fxi->channels(); i++)
    {
        const QLCChannel *ch = fxi->channel(i);
        Q_ASSERT(ch != NULL);
        if (ch->group() == QLCChannel::NoGroup)
            continue;

        ConsoleChannel* cc = new ConsoleChannel(this, m_doc, id, i, m_showCheckBoxes);
        cc->setVisible(false);
        cc->setChannelStyleSheet(m_styleSheet);

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

    connect(fxi, SIGNAL(aliasChanged()), this, SLOT(slotAliasChanged()));
}

quint32 FixtureConsole::fixture() const
{
    return m_fixture;
}

void FixtureConsole::slotAliasChanged()
{
    quint32 i = 0;
    Fixture *fxi = m_doc->fixture(m_fixture);

    QListIterator <ConsoleChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        ConsoleChannel* cc = it.next();
        if (cc->channel() != fxi->channel(i))
        {
            disconnect(cc, SIGNAL(valueChanged(quint32,quint32,uchar)),
                       this, SIGNAL(valueChanged(quint32,quint32,uchar)));
            disconnect(cc, SIGNAL(checked(quint32,quint32,bool)),
                       this, SIGNAL(checked(quint32,quint32,bool)));

            ConsoleChannel* newCC = new ConsoleChannel(this, m_doc, fxi->id(), i, m_showCheckBoxes);
            newCC->setVisible(false);
            newCC->setChannelStyleSheet(m_styleSheet);
            if (cc->hasResetButton())
                newCC->showResetButton(true);
            newCC->setValue(cc->value());
            newCC->setVisible(true);

            connect(newCC, SIGNAL(valueChanged(quint32,quint32,uchar)),
                    this, SIGNAL(valueChanged(quint32,quint32,uchar)));
            connect(newCC, SIGNAL(checked(quint32,quint32,bool)),
                    this, SIGNAL(checked(quint32,quint32,bool)));

            QLayoutItem *item = m_layout->replaceWidget(cc, newCC);
            delete item;
            delete cc;
            m_channels.replace(i, newCC);
        }
        i++;
    }
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
        if (channel == UINT_MAX || channel == cc->channelIndex())
            cc->setChecked(state);
    }
}

void FixtureConsole::setSceneValue(const SceneValue& scv)
{
    Q_ASSERT(scv.fxi == m_fixture);

    QListIterator <ConsoleChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        ConsoleChannel* cc = it.next();
        Q_ASSERT(cc != NULL);
        if (cc->channelIndex() == scv.channel)
        {
            cc->setChecked(true);
            cc->setValue(scv.value);
        }
    }
}

QList <SceneValue> FixtureConsole::values() const
{
    QList <SceneValue> list; // list of all checked channels
    QList <SceneValue> selectedList; // list of selected channels only
    QListIterator <ConsoleChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        ConsoleChannel* cc = it.next();
        Q_ASSERT(cc != NULL);
        if (cc->isChecked() == true)
        {
            list.append(SceneValue(m_fixture, cc->channelIndex(), cc->value()));
            if (cc->isSelected())
                selectedList.append(SceneValue(m_fixture, cc->channelIndex(), cc->value()));
        }
    }

    if (selectedList.count() > 0)
        return selectedList;
    else
        return list;
}

bool FixtureConsole::hasSelections()
{
    foreach (ConsoleChannel *cc, m_channels)
    {
        Q_ASSERT(cc != NULL);
        if (cc->isChecked() && cc->isSelected())
            return true;
    }

    return false;
}

void FixtureConsole::setValues(const QList <SceneValue>& list, bool fromSelection)
{
    QList<ConsoleChannel *> toUncheckList = m_channels;

    QListIterator <SceneValue> it(list);
    while (it.hasNext() == true)
    {
        SceneValue val(it.next());
        if (val.channel < quint32(children().size()))
        {
            ConsoleChannel* cc = channel(val.channel);
            if (cc != NULL)
            {
                if (cc->isChecked() == false)
                    cc->setChecked(true);
                cc->setValue(val.value);
                toUncheckList.removeOne(cc);
            }
        }
    }

    if (fromSelection == false)
    {
        foreach (ConsoleChannel *cc, toUncheckList)
            cc->setChecked(false);
    }
}

void FixtureConsole::setValue(quint32 ch, uchar value, bool apply)
{
    ConsoleChannel* cc = channel(ch);
    if (cc != NULL)
        cc->setValue(value, apply);
}

uchar FixtureConsole::value(quint32 ch) const
{
    ConsoleChannel* cc = channel(ch);
    if (cc != NULL)
        return cc->value();
    else
        return 0;
}

void FixtureConsole::setChannelStylesheet(quint32 ch, QString ss)
{
    ConsoleChannel* cc = channel(ch);
    if (cc != NULL)
        cc->setChannelStyleSheet(ss);
}

void FixtureConsole::resetChannelsStylesheet()
{
    QListIterator <ConsoleChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        ConsoleChannel* cc = it.next();
        Q_ASSERT(cc != NULL);
        cc->setChannelStyleSheet(m_styleSheet);
    }
}

ConsoleChannel* FixtureConsole::channel(quint32 ch) const
{
    QListIterator <ConsoleChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        ConsoleChannel* cc = it.next();
        Q_ASSERT(cc != NULL);
        if (cc->channelIndex() == ch)
            return cc;
    }

    return NULL;
}
