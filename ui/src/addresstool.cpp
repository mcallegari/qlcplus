/*
  Q Light Controller Plus
  addresstool.cpp

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

#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QSettings>

#include "addresstool.h"
#include "ui_addresstool.h"

#define SETTINGS_GEOMETRY "addresstool/geometry"

AddressTool::AddressTool(QWidget *parent, int presetValue) :
    QDialog(parent)
  , ui(new Ui::AddressTool)
  , m_dipSwitch(NULL)
{
    ui->setupUi(this);
    QPixmap px(16, 16);

    px.fill(QColor("#E7354A"));
    ui->m_redBtn->setIcon(QIcon(px));

    px.fill(QColor("#0165DF"));
    ui->m_blueBtn->setIcon(QIcon(px));

    px.fill(Qt::black);
    ui->m_blackBtn->setIcon(QIcon(px));

    ui->m_addressSpin->setValue(presetValue);

    m_dipSwitch = new DIPSwitchWidget(this, presetValue);
    ui->m_gridLayout->addWidget(m_dipSwitch, 0, 0, 1, 5);
    m_dipSwitch->setMinimumHeight(80);

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());

    connect(ui->m_addressSpin, SIGNAL(valueChanged(int)),
            m_dipSwitch, SLOT(slotSetValue(int)));
    connect(m_dipSwitch, SIGNAL(valueChanged(int)),
            ui->m_addressSpin, SLOT(setValue(int)));
    connect(ui->m_reverseVertCheck, SIGNAL(toggled(bool)),
            m_dipSwitch, SLOT(slotReverseVertically(bool)));
    connect(ui->m_reverseHorizCheck, SIGNAL(toggled(bool)),
            m_dipSwitch, SLOT(slotReverseHorizontally(bool)));

}

AddressTool::~AddressTool()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());

    delete ui;
}

int AddressTool::getAddress()
{
    return (ui->m_addressSpin->value());
}

void AddressTool::slotChangeColor()
{
    if (m_dipSwitch == NULL)
        return;

    if (sender() == ui->m_blueBtn)
        m_dipSwitch->setColor(QColor("#0165DF"));
    if (sender() == ui->m_redBtn)
        m_dipSwitch->setColor(QColor("#E7354A"));
    else if (sender() == ui->m_blackBtn)
        m_dipSwitch->setColor(Qt::black);
}

/***************************************************************************
 *
 * DIPSwitchWidget class implementation
 *
 ***************************************************************************/

DIPSwitchWidget::DIPSwitchWidget(QWidget *parent, int presetValue) :
    QWidget(parent)
{
    m_value = presetValue;
    m_backCol = QColor("#0165DF");
    m_verticalReverse = false;
    m_horizontalReverse = false;

    m_font = QApplication::font();
    m_font.setBold(true);
    m_font.setPixelSize(12);

    for (quint8 i=0; i < 10; ++i)
        m_sliders[i] = new DIPSwitchSlider(this);
}

DIPSwitchWidget::~DIPSwitchWidget()
{
}

void DIPSwitchWidget::slotReverseVertically(bool toggle)
{
    m_verticalReverse = toggle;
    update();
}

void DIPSwitchWidget::slotReverseHorizontally(bool toggle)
{
    m_horizontalReverse = toggle;
    updateSliders();
    update();
}

void DIPSwitchWidget::slotSetValue(int value)
{
    m_value = value;
    update();
}

void DIPSwitchWidget::setColor(QColor col)
{
    m_backCol = col;
    update();
}

void DIPSwitchWidget::updateSliders()
{
    int margin = 20;
    float minDiv = (width() - (margin * 2)) / 10;
    float dipW = (minDiv / 3) * 2;
    float xpos = margin + (minDiv / 3);

    for (quint8 i = 0; i < 10; i++)
    {
        quint8 slider_id = i;
        if (m_horizontalReverse) slider_id = 9 - i;

        m_sliders[slider_id]->setPosition(QPoint(xpos, 20), QSize(dipW, height() - 40));
        xpos += minDiv;
    }
}

void DIPSwitchWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    updateSliders();
}

void DIPSwitchWidget::mousePressEvent(QMouseEvent *e)
{
    QMap<quint8, DIPSwitchSlider*>::iterator it;
    for (it = m_sliders.begin(); it != m_sliders.end(); ++it)
    {
        if (it.value()->isClicked(e->pos()))
        {
            quint32 newvalue = m_value ^ (1<<it.key());

			if (newvalue == 0 && m_value != 512) newvalue = m_value;
            if (newvalue == 0) newvalue = 1;
            if (newvalue > 512) newvalue = 512;

            m_value = newvalue;
            update();
            emit valueChanged(m_value);
        }
    }
}

void DIPSwitchWidget::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    int i, j;
    int margin = 20;
    float minDiv = (width() - (margin * 2)) / 10;
    float xpos = margin + (minDiv / 3);
    int onPos = 15; // position of the "ON" string
    int numPos = height() - 5; // position of number labels

    QPainter painter(this);

    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(m_backCol));
    painter.drawRect(0, 0, width(), height());

    // draw DIP switch sliders
    for (i = 0; i < 10; i++)
        m_sliders[i]->paint(&painter, (1<<i) & m_value, m_verticalReverse);

    // draw labels and value
    painter.setFont(m_font);
    painter.setPen(Qt::white);

    xpos = margin + (minDiv / 3);
    if (m_verticalReverse == true)
    {
        onPos = height() - 5;
        numPos = 15;
    }

    painter.drawText(xpos, onPos, "ON");

    if (m_horizontalReverse == false)
    {

        for (i = 0, j = 9; i < 10; i++, j--)
        {
            painter.drawText((i == 9)?(xpos-2):(xpos+2), numPos, QString("%1").arg(i + 1));
            xpos += minDiv;
        }
    }
    else
    {
        for (i = 10, j = 0; i > 0; i--, j++)
        {
            painter.drawText((i == 10)?(xpos-2):(xpos + 2), numPos, QString("%1").arg(i));
            xpos += minDiv;
        }
    }
}

/***************************************************************************
 *
 * DIPSwitchSlider class implementation
 *
 ***************************************************************************/
DIPSwitchSlider::DIPSwitchSlider(QObject *parent) :
    QObject(parent)
{
}

DIPSwitchSlider::~DIPSwitchSlider()
{
}

void DIPSwitchSlider::paint(QPainter *painter, bool value, bool vreverse)
{
    // Draw outer Rectangle
    painter->setBrush(Qt::darkGray);
    painter->setPen(QPen(Qt::black, 2));
    painter->drawRect(QRect(m_pos, m_size));

    // Draw inner Rectangle (slider position)
    painter->setPen(Qt::white);
    painter->setBrush(Qt::white);

    QPoint slider_pos(m_pos.x() + 1, m_pos.y() + 1);
    QSize slider_size(m_size.width() - 3, m_size.width() - 3);
    if (slider_size.height() > m_size.height() / 2)
        slider_size.setHeight(m_size.height() / 2);

    if (value == vreverse) // down
        slider_pos.setY(slider_pos.y() + m_size.height() - slider_size.height() - 3);

    painter->drawRect(QRect(slider_pos, slider_size));
}

void DIPSwitchSlider::setPosition(QPoint pos, QSize size)
{
    m_pos = pos;
    m_size = size;
}


bool DIPSwitchSlider::isClicked(QPoint click)
{
    return(QRect(m_pos, m_size).contains(click));
}
