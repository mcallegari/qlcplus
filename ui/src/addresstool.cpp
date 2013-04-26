/*
  Q Light Controller Plus
  addresstool.cpp

  Copyright (c) Massimo Callegari

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

#include "addresstool.h"
#include "ui_addresstool.h"

#include <QPainter>
#include <QPixmap>

AddressTool::AddressTool(QWidget *parent) :
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

    m_dipSwitch = new DIPSwitchWidget(this);
    ui->m_gridLayout->addWidget(m_dipSwitch, 0, 0, 1, 5);
    m_dipSwitch->setMinimumHeight(80);

    connect(ui->m_addressSpin, SIGNAL(valueChanged(int)),
            m_dipSwitch, SLOT(slotSetValue(int)));
    connect(ui->m_reverseVertCheck, SIGNAL(toggled(bool)),
            m_dipSwitch, SLOT(slotReverseVertically(bool)));
    connect(ui->m_reverseHorizCheck, SIGNAL(toggled(bool)),
            m_dipSwitch, SLOT(slotReverseHorizontally(bool)));

}

AddressTool::~AddressTool()
{
    delete ui;
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

DIPSwitchWidget::DIPSwitchWidget(QWidget *parent) :
    QWidget(parent)
{
    m_value = 1;
    m_backCol = QColor("#0165DF");
    m_verticalReverse = false;
    m_horizontalReverse = false;

    m_font = QApplication::font();
    m_font.setBold(true);
    m_font.setPixelSize(12);
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

void DIPSwitchWidget::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    int i, j;
    int margin = 20;
    float minDiv = (width() - (margin * 2)) / 10;
    float dipW = (minDiv / 3) * 2;
    float xpos = margin + (minDiv / 3);
    int onPos = 15; // position of the "ON" string
    int numPos = height() - 5; // position of number labels
    int zeroPos = height() - 22 - dipW;
    int onePos = 21;
    QString binVal = QString("%1").arg(m_value, 10, 2, QChar('0'));

    QPainter painter(this);

    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(m_backCol));
    painter.drawRect(0, 0, width(), height());

    // draw DIP switch empty bars
    painter.setBrush(Qt::darkGray);
    for (i = 0; i < 10; i++)
    {
        painter.drawRect(xpos, 20, dipW, height() - 40);
        xpos += minDiv;
    }

    // draw labels and value
    painter.setFont(m_font);
    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    xpos = margin + (minDiv / 3);
    if (m_verticalReverse == true)
    {
        onPos = height() - 5;
        numPos = 15;
        zeroPos = 21;
        onePos = height() - 22 - dipW;
    }

    painter.drawText(xpos, onPos, "ON");

    if (m_horizontalReverse == false)
    {

        for (i = 0, j = 9; i < 10; i++, j--)
        {
            painter.drawText((i == 9)?(xpos-2):(xpos+2), numPos, QString("%1").arg(i + 1));
            if (binVal.at(j) == '0')
                painter.drawRect(xpos + 1, zeroPos, dipW - 3, dipW);
            else
                painter.drawRect(xpos + 1, onePos, dipW - 3, dipW);
            xpos += minDiv;
        }
    }
    else
    {
        for (i = 10, j = 0; i > 0; i--, j++)
        {
            painter.drawText((i == 10)?(xpos-2):(xpos + 2), numPos, QString("%1").arg(i));
            if (binVal.at(j) == '0')
                painter.drawRect(xpos + 1, zeroPos, dipW - 3, dipW);
            else
                painter.drawRect(xpos + 1, onePos, dipW - 3, dipW);
            xpos += minDiv;
        }
    }
}


