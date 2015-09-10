/*
  Q Light Controller
  vcspeeddialproperties.h

  Copyright (c) Heikki Junnila

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

#ifndef VCSPEEDDIALPROPERTIES_H
#define VCSPEEDDIALPROPERTIES_H

#include <QDialog>

#include "ui_vcspeeddialproperties.h"
#include "qlcinputsource.h"

class VCSpeedDial;
class VCSpeedDialFunction;
class VCSpeedDialPreset;
class SpeedDialWidget;
class Doc;

/** @addtogroup ui_vc_props
 * @{
 */

class VCSpeedDialProperties : public QDialog, public Ui_VCSpeedDialProperties
{
    Q_OBJECT
    Q_DISABLE_COPY(VCSpeedDialProperties)

public:
    explicit VCSpeedDialProperties(VCSpeedDial* dial, Doc* doc);
    ~VCSpeedDialProperties();

public slots:
    /** @reimp */
    void accept();

private:
    VCSpeedDial* m_dial;
    Doc* m_doc;

    /************************************************************************
     * Functions page
     ************************************************************************/
private slots:
    void slotAddClicked();
    void slotRemoveClicked();

private:
    /** Generate a QList of functions currently in the tree widget */
    QList <VCSpeedDialFunction> functions() const;

    /** Create a tree item for the given function $id */
    void createFunctionItem(const VCSpeedDialFunction &speeddialfunction);

    /************************************************************************
     * Input page
     ************************************************************************/
private:
    void updateInputSources();

private slots:
    void slotAutoDetectAbsoluteInputSourceToggled(bool checked);
    void slotChooseAbsoluteInputSourceClicked();
    void slotAbsoluteInputValueChanged(quint32 universe, quint32 channel);

    void slotAutoDetectTapInputSourceToggled(bool checked);
    void slotChooseTapInputSourceClicked();
    void slotTapInputValueChanged(quint32 universe, quint32 channel);

    void slotAbsolutePrecisionCbChecked(bool checked);

    void slotAttachKey();
    void slotDetachKey();

    void slotAutoDetectInfiniteInputSourceToggled(bool checked);
    void slotChooseInfiniteInputSourceClicked();
    void slotInfiniteInputValueChanged(quint32 universe, quint32 channel);

    void slotAttachInfiniteKey();
    void slotDetachInfiniteKey();

private:
    QSharedPointer<QLCInputSource> m_absoluteInputSource;
    QSharedPointer<QLCInputSource> m_tapInputSource;
    QKeySequence m_tapKeySequence;
    QSharedPointer<QLCInputSource> m_infiniteInputSource;
    QKeySequence m_infiniteKeySequence;

    /*********************************************************************
     * Presets
     *********************************************************************/
private:
    void updateTree();
    void updateTreeItem(VCSpeedDialPreset const& preset);
    VCSpeedDialPreset* getSelectedPreset();
    void addPreset(VCSpeedDialPreset* control);
    void removePreset(quint8 id);
    void updatePresetInputSource(QSharedPointer<QLCInputSource> const& source);

protected slots:
    void slotTreeSelectionChanged();
    void slotAddPresetClicked();
    void slotRemovePresetClicked();
    void slotShowPresetNameClicked();
    void slotPresetNameEdited(QString const& newName);
    void slotSpeedDialWidgetToggle(bool state);
    void slotSpeedDialWidgetDestroyed(QObject* dial);
    void slotSpeedDialWidgetDurationChanged(int ms);

    void slotAutoDetectPresetInputToggled(bool checked);
    void slotPresetInputValueChanged(quint32 universe, quint32 channel);
    void slotChoosePresetInputClicked();

    void slotAttachPresetKey();
    void slotDetachPresetKey();

protected:
    quint8 m_lastAssignedID;
    QList<VCSpeedDialPreset*> m_presets;
    SpeedDialWidget* m_speedDialWidget;
};

/** @} */

#endif
