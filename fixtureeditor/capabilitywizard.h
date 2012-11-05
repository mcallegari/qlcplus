/*
  Q Light Controller - Fixture Definition Editor
  capabilitywizard.h

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

#ifndef CAPABILITYWIZARD_H
#define CAPABILITYWIZARD_H

#include <QDialog>

#include "ui_capabilitywizard.h"

class QLCCapability;
class QLCChannel;

class CapabilityWizard : public QDialog, public Ui_CapabilityWizard
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    CapabilityWizard(QWidget* parent, const QLCChannel* channel);
    ~CapabilityWizard();

    /********************************************************************
     * Capabilities
     ********************************************************************/
public:
    const QList <QLCCapability*> capabilities() const {
        return m_caps;
    }

protected slots:
    void slotCreateCapabilities();

protected:
    QList <QLCCapability*> m_caps;
    const QLCChannel* m_channel;

};

#endif
