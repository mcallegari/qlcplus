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

class InputSelectionWidget;
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
private slots:
    void slotAbsolutePrecisionCbChecked(bool checked);

private:
    InputSelectionWidget *m_absoluteInputWidget;
    InputSelectionWidget *m_tapInputWidget;
    InputSelectionWidget *m_applyInputWidget;

    InputSelectionWidget *m_multInputWidget;
    InputSelectionWidget *m_divInputWidget;
    InputSelectionWidget *m_multDivResetInputWidget;

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
    void slotPresetNameEdited(QString const& newName);
    void slotSpeedDialWidgetValueChanged(int ms);

    void slotAutoDetectPresetInputToggled(bool checked);
    void slotPresetInputValueChanged(quint32 universe, quint32 channel);
    void slotChoosePresetInputClicked();

    void slotAttachPresetKey();
    void slotDetachPresetKey();

protected:
    quint8 m_lastAssignedID;
    QList<VCSpeedDialPreset*> m_presets;
};

/** @} */

#endif
