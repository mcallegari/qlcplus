/*
  Q Light Controller
  efxeditor.h

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

#ifndef EFXEDITOR_H
#define EFXEDITOR_H

#include <QPolygon>
#include <QWidget>
#include <QFrame>
#include <QTimer>

#include "ui_efxeditor.h"
#include "efx.h"
#include "doc.h"

class SpeedDialWidget;
class EFXPreviewArea;
class Doc;

class EfxUiState;

/** @addtogroup ui_functions
 * @{
 */

class EFXEditor : public QWidget, public Ui_EFXEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(EFXEditor)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    EFXEditor(QWidget* parent, EFX* efx, Doc* doc);
    ~EFXEditor();

    void stopTest();

public slots:
    void slotFunctionManagerActive(bool active);

private:
    Doc* m_doc;
    EFX* m_efx; // The EFX being edited

private:
    void initGeneralPage();
    void initMovementPage();
    void initInitializationPage();

    /** If the EFX is currently running as a test or during operate mode, interrupt it
        and return true. If it wasn't running, return false. */
    bool interruptRunning();

    /** Re-start the EFX if $running == true. If in Design mode, it is restarted as a test,
        otherwise it is restarted as a normal EFX. */
    void continueRunning(bool running);

    FunctionParent functionParent() const;

    EfxUiState * efxUiState();

private slots:
    void slotTestClicked();
    void slotRestartTest();
    void slotModeChanged(Doc::Mode mode);
    void slotTabChanged(int tab);
    void slotSetColorBackground(bool checked);

private:
    EFXPreviewArea* m_previewArea;
    QPolygon* m_points;
    QTimer m_testTimer;

    /*********************************************************************
     * General page
     *********************************************************************/
private:
    void updateFixtureTree();
    QTreeWidgetItem* fixtureItem(EFXFixture* ef);
    const QList <EFXFixture*> selectedFixtures() const;
    void updateIndices(int from, int to);
    void addFixtureItem(EFXFixture* ef);
    void updateModeColumn(QTreeWidgetItem* item, EFXFixture* ef);
    void updateIntensityColumn(QTreeWidgetItem* item, EFXFixture* ef);
    void updateStartOffsetColumn(QTreeWidgetItem* item, EFXFixture* ef);
    void removeFixtureItem(EFXFixture* ef);
    void createSpeedDials();
    void updateSpeedDials();

private slots:
    void slotNameEdited(const QString &text);
    void slotSpeedDialToggle(bool state);
    void slotFixtureItemChanged(QTreeWidgetItem* item, int column);
    void slotFixtureModeChanged(int index);
    void slotFixtureStartOffsetChanged(int intensity);
    void slotAddFixtureClicked();
    void slotRemoveFixtureClicked();
    void slotRaiseFixtureClicked();
    void slotLowerFixtureClicked();

    void slotParallelRadioToggled(bool state);
    void slotSerialRadioToggled(bool state);
    void slotAsymmetricRadioToggled(bool state);

    void slotFadeInChanged(int ms);
    void slotFadeOutChanged(int ms);
    void slotHoldChanged(int ms);
    void slotDialDestroyed(QObject* dial);

    void slotFixtureRemoved();
    void slotFixtureChanged();

private:
    SpeedDialWidget *m_speedDials;

    /*********************************************************************
     * Movement page
     *********************************************************************/
private slots:
    void slotAlgorithmSelected(int algoIndex);
    void slotWidthSpinChanged(int value);
    void slotHeightSpinChanged(int value);
    void slotXOffsetSpinChanged(int value);
    void slotYOffsetSpinChanged(int value);
    void slotRotationSpinChanged(int value);
    void slotStartOffsetSpinChanged(int value);
    void slotIsRelativeCheckboxChanged(int value);

    void slotXFrequencySpinChanged(int value);
    void slotYFrequencySpinChanged(int value);
    void slotXPhaseSpinChanged(int value);
    void slotYPhaseSpinChanged(int value);

    void slotLoopClicked();
    void slotSingleShotClicked();
    void slotPingPongClicked();

    void slotForwardClicked();
    void slotBackwardClicked();

private:
    void redrawPreview();
};

/** @} */

#endif
