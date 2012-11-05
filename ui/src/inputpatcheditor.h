/*
  Q Light Controller
  inputpatcheditor.h

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

#ifndef INPUTPATCHEDITOR_H
#define INPUTPATCHEDITOR_H

#include <QWidget>

#include "ui_inputpatcheditor.h"
#include "qlcinputprofile.h"
#include "inputpatch.h"

class QStringList;
class InputMap;

class InputPatchEditor : public QWidget, public Ui_InputPatchEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(InputPatchEditor)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /**
     * Create a new input patch editor for the given universe.
     *
     * @param widget Parent widget
     * @param universe The universe whose settings are being edited
     * @param inputMap The input map object that manages input plugin data
     */
    InputPatchEditor(QWidget* parent, quint32 universe, InputMap* inputMap);
    ~InputPatchEditor();

signals:
    /** Tells that the mapping settings have changed */
    void mappingChanged();

private:
    InputMap* m_inputMap;

    quint32 m_universe; //! The input universe that is being edited

    QString m_currentPluginName;
    quint32 m_currentInput;
    QString m_currentProfileName;
    bool m_currentFeedbackEnabled;

    /************************************************************************
     * Mapping page
     ************************************************************************/
private:
    InputPatch* patch() const;
    QTreeWidgetItem* currentlyMappedItem() const;
    void setupMappingPage();
    void fillMappingTree();
    void fillPluginItem(const QString& pluginName, QTreeWidgetItem* item);
    QTreeWidgetItem* pluginItem(const QString& pluginName);

private slots:
    void slotMapCurrentItemChanged(QTreeWidgetItem* item);
    void slotMapItemChanged(QTreeWidgetItem* item);
    void slotConfigureInputClicked();
    void slotFeedbackToggled(bool enable);
    void slotPluginConfigurationChanged(const QString& pluginName);

    /************************************************************************
     * Profile page
     ************************************************************************/
private:
    void setupProfilePage();
    void fillProfileTree();
    void updateProfileItem(const QString& name, QTreeWidgetItem* item);
    QString fullProfilePath(const QString& manufacturer, const QString& model) const;

private slots:
    void slotProfileItemChanged(QTreeWidgetItem* item);
    void slotAddProfileClicked();
    void slotRemoveProfileClicked();
    void slotEditProfileClicked();
};

#endif /* INPUTPATCHEDITOR_H */
