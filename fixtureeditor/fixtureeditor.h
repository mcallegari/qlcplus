/*
  Q Light Controller - Fixture Definition Editor
  qlcfixtureeditor.h

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

#ifndef QLCFIXTUREEDITOR_H
#define QLCFIXTUREEDITOR_H

#include <QWidget>
#include "ui_fixtureeditor.h"

class QCloseEvent;
class QString;

class QLCFixtureMode;
class QLCFixtureDef;
class QLCChannel;

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

    void setFileName(QString path) {
        m_fileName = path;
    }
    QString fileName() const {
        return m_fileName;
    }

    bool modified() const {
        return m_modified;
    }
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
    void slotChannelListSelectionChanged(QTreeWidgetItem* item);
    void slotAddChannel();
    void slotRemoveChannel();
    void slotEditChannel();
    void slotCopyChannel();
    void slotPasteChannel();
    void slotExpandChannels();
    void slotChannelListContextMenuRequested();

protected:
    QLCChannel* currentChannel();
    void refreshChannelList();
    void updateChannelItem(const QLCChannel* channel,
                           QTreeWidgetItem* item);

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

protected:
    QLCFixtureMode* currentMode();
    void refreshModeList();
    void updateModeItem(const QLCFixtureMode* mode, QTreeWidgetItem* item);

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public slots:
    void slotClipboardChanged();
};

#endif
