/*
  Q Light Controller - Fixture Definition Editor
  editcapability.h

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

#ifndef EDITCAPABILITY_H
#define EDITCAPABILITY_H

#include <QWidget>
#include "ui_editcapability.h"

class QWidget;
class QLCCapability;

class EditCapability : public QDialog, public Ui_EditCapability
{
    Q_OBJECT

public:
    EditCapability(QWidget* parent, const QLCCapability* capability = NULL);
    ~EditCapability();

    /*********************************************************************
     * Capability
     *********************************************************************/
public:
    QLCCapability* capability() const {
        return m_capability;
    }

protected:
    QLCCapability* m_capability;

    /*********************************************************************
     * UI slots
     *********************************************************************/
public slots:
    void slotMinSpinChanged(int value);
    void slotMaxSpinChanged(int value);
    void slotDescriptionEdited(const QString& text);
};

#endif
