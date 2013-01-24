/*
  Q Light Controller
  configureartnet.h

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

#ifndef CONFIGUREARTNET_H
#define CONFIGUREARTNET_H

#include "ui_configureartnet.h"

class ArtNetPlugin;

class ConfigureArtNet : public QDialog, public Ui_ConfigureArtNet
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    ConfigureArtNet(ArtNetPlugin* plugin, QWidget* parent = 0);
    virtual ~ConfigureArtNet();

    /** @reimp */
    void accept();

public slots:
    int exec();

private:
    ArtNetPlugin* m_plugin;

};

#endif
