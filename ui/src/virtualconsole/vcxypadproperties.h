/*
  Q Light Controller
  vcxypadproperties.h

  Copyright (c) Stefan Krumm, Heikki Junnila

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

#ifndef VCXYPADPROPERTIES_H
#define VCXYPADPROPERTIES_H

#include <QDialog>

#include "ui_vcxypadproperties.h"
#include "vcxypadfixture.h"
#include "dmxsource.h"

class InputSelectionWidget;
class VCXYPadPreset;
class VCXYPadArea;
class MasterTimer;
class VCXYPad;
class Doc;

/** @addtogroup ui_vc_props
 * @{
 */

class VCXYPadProperties : public QDialog, public Ui_VCXYPadProperties, public DMXSource
{
    Q_OBJECT
    Q_DISABLE_COPY(VCXYPadProperties)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    VCXYPadProperties(VCXYPad *xypad, Doc *doc);
    ~VCXYPadProperties();

private:
    VCXYPad *m_xypad;
    Doc *m_doc;
    InputSelectionWidget *m_panInputWidget;
    InputSelectionWidget *m_panFineInputWidget;
    InputSelectionWidget *m_tiltInputWidget;
    InputSelectionWidget *m_tiltFineInputWidget;
    InputSelectionWidget *m_widthInputWidget;
    InputSelectionWidget *m_heightInputWidget;

    /********************************************************************
     * Fixtures page
     ********************************************************************/
private:
    void fillFixturesTree();
    void updateFixturesTree(VCXYPadFixture::DisplayMode mode = VCXYPadFixture::Degrees);
    QList <VCXYPadFixture> selectedFixtures() const;
    QTreeWidgetItem* fixtureItem(const VCXYPadFixture& fxi);

    void updateFixtureItem(QTreeWidgetItem* item, const VCXYPadFixture& fxi);
    void removeFixtureItem(GroupHead const & head);

    void stopAutodetection(quint8 sourceId);

private slots:
    void slotAddClicked();
    void slotRemoveClicked();
    void slotEditClicked();
    void slotSelectionChanged(QTreeWidgetItem* item);

    void slotPercentageRadioChecked();
    void slotDegreesRadioChecked();
    void slotDMXRadioChecked();

    /********************************************************************
     * External controls
     ********************************************************************/
private slots:
    void slotPanAutoDetectToggled(bool toggled);
    void slotPanInputValueChanged(quint32 uni, quint32 ch);
    void slotPanFineAutoDetectToggled(bool toggled);
    void slotPanFineInputValueChanged(quint32 uni, quint32 ch);
    void slotTiltAutoDetectToggled(bool toggled);
    void slotTiltInputValueChanged(quint32 uni, quint32 ch);
    void slotTiltFineAutoDetectToggled(bool toggled);
    void slotTiltFineInputValueChanged(quint32 uni, quint32 ch);

    /********************************************************************
     * Presets
     ********************************************************************/
public:
    /** @reimp */
    void writeDMX(MasterTimer *timer, QList<Universe*> universes);

private:
    void updatePresetsTree();
    void selectItemOnPresetsTree(quint8 presetId);
    void updateTreeItem(VCXYPadPreset const& preset);
    VCXYPadPreset *getSelectedPreset();
    void removePreset(quint8 id);

    //move preset up and swap id with previous preset. Return new preset id.
    quint8 moveUpPreset(quint8 id);

    //move preset down and swap id with the next preset. Return new preset id.
    quint8 moveDownPreset(quint8 id);

protected slots:
    void slotAddPositionClicked();
    void slotAddEFXClicked();
    void slotAddSceneClicked();
    void slotAddFixtureGroupClicked();
    void slotRemovePresetClicked();
    void slotMoveUpPresetClicked();
    void slotMoveDownPresetClicked();
    void slotPresetNameEdited(QString const& newName);
    void slotPresetSelectionChanged();
    void slotXYPadPositionChanged(const QPointF& pt);
    void slotInputValueChanged(quint32 universe, quint32 channel);
    void slotKeySequenceChanged(QKeySequence key);

private:
    VCXYPadArea *m_xyArea;
    InputSelectionWidget *m_presetInputWidget;

    quint8 m_lastAssignedID;
    QList<VCXYPadPreset*> m_presetList;

    /** Map used to lookup a GenericFader instance for a Universe ID */
    QMap<quint32, QSharedPointer<GenericFader> > m_fadersMap;

    /********************************************************************
     * OK/Cancel
     ********************************************************************/
public slots:
    void accept();
};

/** @} */

#endif
