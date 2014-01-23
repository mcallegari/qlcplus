/*
  Q Light Controller Plus
  addrgbpanel.cpp

  Copyright (c) Massimo Callegari

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

#include "addrgbpanel.h"
#include "ui_addrgbpanel.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "doc.h"

AddRGBPanel::AddRGBPanel(QWidget *parent, const Doc *doc)
    : QDialog(parent)
    , m_doc(doc)
{
    setupUi(this);

    /* Fill universe combo with available universes */
    m_uniCombo->addItems(m_doc->inputOutputMap()->universeNames());

    connect(m_columnSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotSizeChanged(int)));
    connect(m_rowSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotSizeChanged(int)));
}

AddRGBPanel::~AddRGBPanel()
{
}

QString AddRGBPanel::name()
{
    return m_nameEdit->text();
}

int AddRGBPanel::universeIndex()
{
    return m_uniCombo->currentIndex();
}

int AddRGBPanel::address()
{
    return m_addressSpin->value() - 1;
}

int AddRGBPanel::columns()
{
    return m_columnSpin->value();
}

int AddRGBPanel::rows()
{
    return m_rowSpin->value();
}

AddRGBPanel::Type AddRGBPanel::type()
{
    if (m_snakeRadio->isChecked())
        return Snake;
    else if (m_zigzagRadio->isChecked())
        return ZigZag;

    return Unknown;
}

QLCFixtureDef *AddRGBPanel::rowDefinition()
{
    QLCFixtureDef *def = new QLCFixtureDef();
    def->setManufacturer(KXMLFixtureGeneric);
    def->setModel(KXMLFixtureRGBPanel);
    def->setType("LED Bar");
    def->setAuthor("QLC+");
    for (int i = 0; i < m_columnSpin->value(); i++)
    {
        QLCChannel* red = new QLCChannel();
        red->setName(QString("Red %1").arg(i + 1));
        red->setGroup(QLCChannel::Intensity);
        red->setColour(QLCChannel::Red);

        QLCChannel* green = new QLCChannel();
        green->setName(QString("Green %1").arg(i + 1));
        green->setGroup(QLCChannel::Intensity);
        green->setColour(QLCChannel::Green);

        QLCChannel* blue = new QLCChannel();
        blue->setName(QString("Blue %1").arg(i + 1));
        blue->setGroup(QLCChannel::Intensity);
        blue->setColour(QLCChannel::Blue);

        def->addChannel(red);
        def->addChannel(green);
        def->addChannel(blue);
    }

    return def;
}

QLCFixtureMode *AddRGBPanel::rowMode(QLCFixtureDef *def)
{
    Q_ASSERT(def != NULL);
    QLCFixtureMode *mode = new QLCFixtureMode(def);
    mode->setName("Default");
    QList<QLCChannel *>channels = def->channels();
    for (int i = 0; i < channels.count(); i++)
    {
        QLCChannel *ch = channels.at(i);
        mode->insertChannel(ch, i);
        if (i%3 == 0)
        {
            QLCFixtureHead head;
            head.addChannel(i);
            head.addChannel(i+1);
            head.addChannel(i+2);
            mode->insertHead(-1, head);
        }
    }

    return mode;
}

void AddRGBPanel::slotSizeChanged(int)
{
    m_totalLabel->setText(QString::number(m_columnSpin->value() * m_rowSpin->value()));
}
