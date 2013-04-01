/*
  Q Light Controller Plus
  addresstool.h

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

#ifndef ADDRESSTOOL_H
#define ADDRESSTOOL_H

#include <QDialog>
#include <QWidget>

namespace Ui {
class AddressTool;
}

class DIPSwitchWidget: public QWidget
{
    Q_OBJECT

public:
    DIPSwitchWidget(QWidget *parent = 0);
    ~DIPSwitchWidget();

    void setColor(QColor col);

public slots:
    void slotReverseVertically(bool toggle);
    void slotReverseHorizontally(bool toggle);
    void slotSetValue(int value);

private:
    int m_value;
    QFont m_font;
    QColor m_backCol;
    bool m_verticalReverse;
    bool m_horizontalReverse;

protected:
    void paintEvent(QPaintEvent* e);
};

class AddressTool : public QDialog
{
    Q_OBJECT
    
public:
    explicit AddressTool(QWidget *parent = 0);
    ~AddressTool();
    
private:
    Ui::AddressTool *ui;
    DIPSwitchWidget *m_dipSwitch;

protected slots:
    void slotChangeColor();

};

#endif // ADDRESSTOOL_H
