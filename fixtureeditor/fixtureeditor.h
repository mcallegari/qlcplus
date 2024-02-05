/*
  Q Light Controller - Fixture Definition Editor
  qlcfixtureeditor.h

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

#ifndef QLCFIXTUREEDITOR_H
#define QLCFIXTUREEDITOR_H

#include <QWidget>
#include "ui_fixtureeditor.h"

#include "qlcphysical.h"

class QCloseEvent;
class QString;

class QLCFixtureMode;
class QLCFixtureDef;
class EditPhysical;
class QLCChannel;

/** @addtogroup fixtureeditor Fixture Editor
 * @{
 */

class QLCFixtureEditor : public QWidget, public Ui_FixtureEditor
{
    Q_OBJECT

public:
    QLCFixtureEditor(QWidget* parent, QLCFixtureDef* fixtureDef,
                     const QString& fileName = QString());
    virtual ~QLCFixtureEditor();

protected:
    void init();
    void closeEvent(QCloseEvent* e);

    /*********************************************************************
     * Saving
     *********************************************************************/
public:
    bool save();
    bool saveAs();

    void setFileName(QString path);
    QString fileName() const;

    bool modified() const;
    void setModified(bool modified = true);

protected:
    QLCFixtureDef* m_fixtureDef;
    QString m_fileName;
    bool m_modified;

    /*********************************************************************
     * General
     *********************************************************************/
protected slots:
    void slotManufacturerTextEdited(const QString &text);
    void slotModelTextEdited(const QString &text);
    void slotAuthorTextEdited(const QString &text);
    void slotTypeActivated(const QString &text);

protected:
    bool checkManufacturerModel();
    void setCaption();
    void ensureNewExtension();
    bool newExtensionReminder();

    /*********************************************************************
     * Channels
     *********************************************************************/
protected slots:
    void slotChannelListSelectionChanged(QTreeWidgetItem *item);
    void slotAddChannel();
    void slotRemoveChannel();
    void slotEditChannel();
    void slotCopyChannel();
    void slotPasteChannel();
    void slotExpandChannels();
    void slotChannelListContextMenuRequested();
    void slotChannelItemExpanded();

protected:
    QLCChannel* currentChannel();
    void refreshChannelList();
    void updateChannelItem(const QLCChannel *channel, QTreeWidgetItem *item);

    /*********************************************************************
     * Modes
     *********************************************************************/
protected slots:
    void slotModeListSelectionChanged(QTreeWidgetItem* item);
    void slotAddMode();
    void slotRemoveMode();
    void slotEditMode();
    void slotCloneMode();
    void slotExpandModes();
    void slotModeListContextMenuRequested();
    void slotModeItemExpanded();

protected:
    QLCFixtureMode* currentMode();
    void refreshModeList();
    void updateModeItem(const QLCFixtureMode *mode, QTreeWidgetItem *item);

    /*********************************************************************
     * Aliases
     *********************************************************************/
protected slots:
    void slotAddAliasClicked();
    void slotRemoveAliasClicked();
    void refreshAliasModes();
    void updateAliasModeName(QString oldName, QString newName);
    void refreshAliasModeChannels();

protected:
    void refreshAliasList();
    void refreshAliasAllChannels();
    void refreshAliasTree();
    void checkAliasAddButton();

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public slots:
    void slotClipboardChanged();
    void slotCopyPhysicalClipboard(QLCPhysical clipboard);
    void slotPastePhysicalInfo();

private:
    EditPhysical *m_phyEdit;
    QLCPhysical m_physicalClipboard;
};

/** @} */

#endif
