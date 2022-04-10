/*
  Q Light Controller
  inputoutputpatcheditor.h

  Copyright (C) Massimo Callegari

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

#ifndef INPUTOUTPUTPATCHEDITOR_H
#define INPUTOUTPUTPATCHEDITOR_H

#include <QWidget>

#include "ui_inputoutputpatcheditor.h"

class InputOutputMap;
class AudioCapture;
class OutputPatch;
class InputPatch;
class Doc;

/** @addtogroup ui_io
 * @{
 */

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
    InputOutputPatchEditor(QWidget* parent, quint32 universe, InputOutputMap* ioMap, Doc* doc);
    ~InputOutputPatchEditor();

signals:
    /** Tells that the mapping settings have changed */
    void mappingChanged();

    /** Tells that the audio input device has changed */
    void audioInputDeviceChanged();

private:
    InputOutputMap* m_ioMap;
    Doc *m_doc;

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
    void showPluginMappingError();

private slots:
    void slotMapCurrentItemChanged(QTreeWidgetItem* item);
    void slotMapItemChanged(QTreeWidgetItem* item, int col);
    void slotConfigureInputClicked();
    void slotPluginConfigurationChanged(const QString& pluginName, bool success);
    void slotHotpluggingChanged(bool checked);

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
    void initAudioTab();

private slots:
    void slotAudioDeviceItemChanged(QTreeWidgetItem* item, int col);
    void slotSampleRateIndexChanged(int index);
    void slotAudioChannelsChanged(int index);
    void slotAudioInputPreview(bool enable);
    void slotAudioUpdateLevel(double *spectrumBands, int size, double maxMagnitude, quint32 power);

private:
    AudioCapture *m_inputCapture;
};

/** @} */

#endif /* INPUTOUTPUTPATCHEDITOR_H */
