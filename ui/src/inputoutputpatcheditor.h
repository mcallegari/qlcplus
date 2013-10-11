/*
  Q Light Controller
  inputoutputpatcheditor.h

  Copyright (C) Massimo Callegari

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

#ifndef INPUTOUTPUTPATCHEDITOR_H
#define INPUTOUTPUTPATCHEDITOR_H

#include <QWidget>

#include "ui_inputoutputpatcheditor.h"

class QStringList;
class OutputPatch;
class InputPatch;
class InputMap;
class OutputMap;

class InputOutputPatchEditor : public QWidget, public Ui_InputOutputPatchEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(InputOutputPatchEditor)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /**
     * Create a new output patch editor for the given universe.
     *
     * @param parent The owning parent widget
     * @param universe The universe whose settings to edit
     * @param outputMap The output map object that handles DMX output
     */
    InputOutputPatchEditor(QWidget* parent, quint32 universe, InputMap* inputMap, OutputMap* outputMap);
    ~InputOutputPatchEditor();

signals:
    /** Tells that the mapping settings have changed */
    void mappingChanged();

private:
    InputMap* m_inputMap;
    OutputMap* m_outputMap;

    quint32 m_universe; //! The input universe that is being edited

    QString m_currentInputPluginName;
    quint32 m_currentInput;
    QString m_currentOutputPluginName;
    quint32 m_currentOutput;
    QString m_currentProfileName;
    QString m_currentFeedbackPluginName;
    quint32 m_currentFeedback;

    /************************************************************************
     * Mapping page
     ************************************************************************/
private:
    InputPatch* patch() const;
    QTreeWidgetItem* currentlyMappedItem() const;
    void setupMappingPage();
    QTreeWidgetItem *itemLookup(QString pluginName, QString devName);
    void fillMappingTree();
    QTreeWidgetItem* pluginItem(const QString& pluginName);

private slots:
    void slotMapCurrentItemChanged(QTreeWidgetItem* item);
    void slotMapItemChanged(QTreeWidgetItem* item, int col);
    void slotConfigureInputClicked();
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

    /************************************************************************
     * Audio page
     ************************************************************************/
private:
    void fillAudioTree();

private slots:
    void slotAudioDeviceItemChanged(QTreeWidgetItem* item, int col);
};

#endif /* INPUTOUTPUTPATCHEDITOR_H */
