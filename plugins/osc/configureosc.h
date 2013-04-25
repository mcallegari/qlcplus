/*
  Q Light Controller Plus
  configureosc.h

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

#ifndef CONFIGUREOSC_H
#define CONFIGUREOSC_H

#include "ui_configureosc.h"

class OSCPlugin;

class ConfigureOSC : public QDialog, public Ui_ConfigureOSC
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    ConfigureOSC(OSCPlugin* plugin, QWidget* parent = 0);
    virtual ~ConfigureOSC();

    /** @reimp */
    void accept();

public slots:
    int exec();

private:
    OSCPlugin* m_plugin;

};

#endif
