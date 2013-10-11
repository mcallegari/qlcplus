/*
  Q Light Controller - Fixture Definition Editor
  editmode.h

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

#ifndef EDITMODE_H
#define EDITMODE_H

#include <QDialog>
#include "ui_editmode.h"

#include "qlcphysical.h"

class QLCFixtureMode;
class QLCFixtureHead;
class QLCFixtureDef;
class QLCChannel;

class EditMode : public QDialog, public Ui_EditMode
{
    Q_OBJECT

public:
    /** Use this constructor to edit an existing mode */
    EditMode(QWidget* parent, QLCFixtureMode* mode);

    /** Use this constructor to create a new mode for the fixture */
    EditMode(QWidget* parent, QLCFixtureDef* fixtureDef);

    /** Destructor */
    ~EditMode();

protected:
    void loadDefaults();
    void init();

    /*********************************************************************
     * Fixture Mode
     *********************************************************************/
public:
    /** Get the mode that was being edited. Don't save the pointer! */
    QLCFixtureMode* mode() {
        return m_mode;
    }

private:
    QLCFixtureMode* m_mode;

    /*************************************************************************
     * Channels page
     *************************************************************************/
protected slots:
    void slotAddChannelClicked();
    void slotRemoveChannelClicked();
    void slotRaiseChannelClicked();
    void slotLowerChannelClicked();

protected:
    void refreshChannelList();
    QLCChannel* currentChannel();
    void selectChannel(const QString &name);

    /************************************************************************
     * Heads page
     ************************************************************************/
private slots:
    void slotAddHeadClicked();
    void slotRemoveHeadClicked();
    void slotEditHeadClicked();
    void slotRaiseHeadClicked();
    void slotLowerHeadClicked();

private:
    void refreshHeadList();
    QLCFixtureHead currentHead();
    void selectHead(int index);

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    QLCPhysical getClipboard();
    void setClipboard(QLCPhysical physical);

private slots:
    void slotCopyToClipboard();
    void slotPasteFromClipboard();

private:
    QLCPhysical m_clipboard;

    /*********************************************************************
     * Accept
     *********************************************************************/
protected slots:
    void accept();
};

#endif
