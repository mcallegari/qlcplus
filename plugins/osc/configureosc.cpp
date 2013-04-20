/*
  Q Light Controller Plus
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

    m_port1Spin->setValue(plugin->getPort(0).toUInt());
    m_outAddr1Edit->setText(plugin->getOutputAddress(0));

    m_port2Spin->setValue(plugin->getPort(1).toUInt());
    m_outAddr2Edit->setText(plugin->getOutputAddress(1));

    m_port3Spin->setValue(plugin->getPort(2).toUInt());
    m_outAddr3Edit->setText(plugin->getOutputAddress(2));

    m_port4Spin->setValue(plugin->getPort(3).toUInt());
    m_outAddr4Edit->setText(plugin->getOutputAddress(3));
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
    m_plugin->setPort(0, QString("%1").arg(m_port1Spin->value()));
    m_plugin->setOutputAddress(0, m_outAddr1Edit->text());

    m_plugin->setPort(1, QString("%1").arg(m_port2Spin->value()));
    m_plugin->setOutputAddress(1, m_outAddr2Edit->text());

    m_plugin->setPort(2, QString("%1").arg(m_port3Spin->value()));
    m_plugin->setOutputAddress(2, m_outAddr3Edit->text());

    m_plugin->setPort(3, QString("%1").arg(m_port4Spin->value()));
    m_plugin->setOutputAddress(3, m_outAddr4Edit->text());

    QDialog::accept();
}

int ConfigureOSC::exec()
{
    return QDialog::exec();
}

