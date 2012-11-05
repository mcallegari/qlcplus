/*
  Q Light Controller - Fixture Definition Editor
  editchannel.h

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

#ifndef EDITCHANNEL_H
#define EDITCHANNEL_H

#include <QWidget>
#include "ui_editchannel.h"

class QTreeWidgetItem;
class QString;

class QLCChannel;
class QLCCapability;

class EditChannel : public QDialog, public Ui_EditChannel
{
    Q_OBJECT
public:
    EditChannel(QWidget* parent, QLCChannel* channel = NULL);
    ~EditChannel();

protected:
    void init();

    /*********************************************************************
     * Channel
     *********************************************************************/
public:
    /** Get the channel that was edited. */
    QLCChannel* channel() {
        return m_channel;
    }

protected:
    QLCChannel* m_channel;

    /*********************************************************************
     * Basic channel info
     *********************************************************************/
protected slots:
    void slotNameChanged(const QString& name);
    void slotGroupActivated(const QString& group);
    void slotMsbRadioToggled(bool toggled);
    void slotLsbRadioToggled(bool toggled);
    void slotColourActivated(const QString& colour);

    /*********************************************************************
     * Capabilities
     *********************************************************************/
protected slots:
    void slotCapabilityListSelectionChanged(QTreeWidgetItem* item);
    void slotAddCapabilityClicked();
    void slotRemoveCapabilityClicked();
    void slotEditCapabilityClicked();
    void slotWizardClicked();

protected:
    void refreshCapabilities();
    QLCCapability* currentCapability();
};

#endif
