/*
  Q Light Controller
  inputchanneleditor.cpp

  Copyright (C) Heikki Junnila

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

#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QIcon>

#include "qlcchannel.h"
#include "qlcinputprofile.h"
#include "qlcinputchannel.h"
#include "inputchanneleditor.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

InputChannelEditor::InputChannelEditor(QWidget* parent,
                                       const QLCInputProfile* profile,
                                       const QLCInputChannel* channel)
        : QDialog(parent)
{
    m_channel = 0;
    m_type = QLCInputChannel::NoType;

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    /* Connect to these already now so that the handlers get called
       during initialization. */
    connect(m_numberSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotNumberChanged(int)));
    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_typeCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotTypeActivated(const QString &)));

    /* Fill type combo with type icons and names */
    QStringListIterator it(QLCInputChannel::types());
    while (it.hasNext() == true)
    {
        QString str(it.next());
        m_typeCombo->addItem(QLCInputChannel::stringToIcon(str), str);
    }

    if (channel != NULL && profile != NULL)
    {
        QString type;
        quint32 num;

        /* Channel number */
        num = profile->channelNumber(channel);
        if (num != QLCChannel::invalid())
            m_numberSpin->setValue(num + 1);
        else
            m_numberSpin->setValue(1);

        /* Channel name */
        m_nameEdit->setText(channel->name());

        /* Channel type */
        type = QLCInputChannel::typeToString(channel->type());
        m_typeCombo->setCurrentIndex(m_typeCombo->findText(type));
    }
    else
    {
        /* Multiple channels are being edited. Disable the channel
           number spin. */
        m_numberSpin->setEnabled(false);
    }
}

InputChannelEditor::~InputChannelEditor()
{
}

/****************************************************************************
 * Properties
 ****************************************************************************/

quint32 InputChannelEditor::channel() const
{
    return m_channel;
}

QString InputChannelEditor::name() const
{
    return m_name;
}

QLCInputChannel::Type InputChannelEditor::type() const
{
    return m_type;
}

void InputChannelEditor::slotNumberChanged(int number)
{
    m_channel = number - 1;
}

void InputChannelEditor::slotNameEdited(const QString& text)
{
    m_name = text;
}

void InputChannelEditor::slotTypeActivated(const QString& text)
{
    m_type = QLCInputChannel::stringToType(text);
}
