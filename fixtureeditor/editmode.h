/*
  Q Light Controller - Fixture Definition Editor
  editmode.h

  Copyright (C) Heikki Junnila

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef EDITMODE_H
#define EDITMODE_H

#include <QDialog>
#include "ui_editmode.h"

#include "qlcphysical.h"

class QLCFixtureMode;
class QLCFixtureHead;
class QLCFixtureDef;
class EditPhysical;
class QLCChannel;

/** @addtogroup fixtureeditor Fixture Editor
 * @{
 */

class EditMode : public QDialog, public Ui_EditMode
{
    Q_OBJECT

public:
    /** Use this constructor to edit an existing mode */
    EditMode(QWidget *parent, QLCFixtureMode *mode);

    /** Use this constructor to create a new mode for the fixture */
    EditMode(QWidget *parent, QLCFixtureDef *fixtureDef);

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
    QLCFixtureMode *mode();

private:
    QLCFixtureMode *m_mode;

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
    QLCChannel *currentChannel();
    void selectChannel(const QString &name);

private slots:
    void setActsOnChannel(int index);

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
     * Physical and clipboard
     *********************************************************************/
public:
    void pasteFromClipboard(QLCPhysical clipboard);

signals:
    void copyToClipboard(QLCPhysical physical);
    void requestPasteFromClipboard();

private slots:
    void slotPhysicalModeChanged();

private:
    EditPhysical *m_phyEdit;

    /*********************************************************************
     * Accept
     *********************************************************************/
protected slots:
    void accept();
};

/** @} */

#endif
