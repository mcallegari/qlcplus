/*
  Q Light Controller
  dmxaddresstool.h

  Copyright (C) Heikki Junnila

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

#ifndef DMXADDRESSTOOL_H
#define DMXADDRESSTOOL_H

#include <QDialog>
#include "ui_dmxaddresstool.cpp"

class QString;

class DMXAddressTool : public QDialog, public Ui_DMXAddressTool
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    DMXAddressTool(QWidget* parent);
    ~DMXAddressTool();

private:
    Q_DISABLE_COPY(DMXAddressTool)

    /********************************************************************
     * Selection
     ********************************************************************/
public:
    int address() {
        return m_address;
    }
    void setAddress(int address);

protected slots:
    void slotSliderValueChanged(int value);
    void slotDecimalChanged(const QString &text);

protected:
    int m_address;
    bool m_updateValue;
};

#endif









































