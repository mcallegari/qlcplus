/*
  Q Light Controller
  enttecdmxusbconfig.h

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef ENTTECDMXUSBCONFIG_H
#define ENTTECDMXUSBCONFIG_H

#include <QDialog>

class DMXUSBWidget;
class QTreeWidgetItem;
class DMXUSB;
class QPushButton;
class QTreeWidget;
class QComboBox;

class DMXUSBConfig : public QDialog
{
    Q_OBJECT

public:
    DMXUSBConfig(DMXUSB *plugin, QWidget* parent = 0);
    ~DMXUSBConfig();

private slots:
    void slotTypeComboActivated(int index);
    void slotRefresh();

private:
    QComboBox* createTypeCombo(DMXUSBWidget* widget);

private:
    DMXUSB* m_plugin;

    QTreeWidget* m_tree;
    QPushButton* m_refreshButton;
    QPushButton* m_closeButton;

    bool m_ignoreItemChanged;
};

#endif
