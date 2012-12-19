/*
  Q Light Controller
  configureosc.cpp

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

#include <QString>
#include <QDebug>

#include "configureosc.h"
#include "oscplugin.h"

#define KColumnNumber  0
#define KColumnName    1

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureOSC::ConfigureOSC(OSCPlugin* plugin, QWidget* parent)
        : QDialog(parent)
{
    Q_ASSERT(plugin != NULL);
    m_plugin = plugin;

    /* Setup UI controls */
    setupUi(this);

    m_portSpin->setValue(plugin->getPort().toUInt());
}

ConfigureOSC::~ConfigureOSC()
{
}

/*****************************************************************************
 * Dialog actions
 *****************************************************************************/

void ConfigureOSC::accept()
{
    qDebug() << Q_FUNC_INFO;
    m_plugin->setPort(QString("%1").arg(m_portSpin->value()));
    QDialog::accept();
}

int ConfigureOSC::exec()
{
    return QDialog::exec();
}

