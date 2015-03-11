/*
  Q Light Controller
  efxeditor.cpp

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

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QMessageBox>
#include <QPaintEvent>
#include <QSettings>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPainter>
#include <QLabel>
#include <QDebug>
#include <QPen>

#include "qlcfixturedef.h"
#include "qlcchannel.h"

#include "fixtureselection.h"
#include "speeddialwidget.h"
#include "efxpreviewarea.h"
#include "efxeditor.h"
#include "fixture.h"
#include "apputil.h"
#include "doc.h"

#include "efxuistate.h"

#define SETTINGS_GEOMETRY "efxeditor/geometry"

#define KColumnNumber  0
#define KColumnName    1
#define KColumnReverse 2
#define KColumnStartOffset 3
#define KColumnIntensity 4

#define PROPERTY_FIXTURE "fixture"

#define KTabGeneral 0
#define KTabMovement 1

/*****************************************************************************
 * Initialization
 *****************************************************************************/

EFXEditor::EFXEditor(QWidget* parent, EFX* efx, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_efx(efx)
    , m_previewArea(NULL)
    , m_points(NULL)
    , m_speedDials(NULL)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(efx != NULL);

    setupUi(this);

    connect(m_speedDial, SIGNAL(toggled(bool)),
            this, SLOT(slotSpeedDialToggle(bool)));

    initGeneralPage();
    initMovementPage();

    // Start new (==empty) scenes from the first tab and ones with something in them
    // on the first fixture page.
    if (m_tab->count() == 0)
        slotTabChanged(KTabGeneral);
    else
        m_tab->setCurrentIndex(efxUiState()->currentTab());

    /* Tab widget */
    connect(m_tab, SIGNAL(currentChanged(int)),
            this, SLOT(slotTabChanged(int)));

    // Used for intensity changes
    m_testTimer.setSingleShot(true);
    m_testTimer.setInterval(500);
    connect(&m_testTimer, SIGNAL(timeout()), this, SLOT(slotRestartTest()));
    connect(m_doc, SIGNAL(modeChanged(Doc::Mode)), this, SLOT(slotModeChanged(Doc::Mode)));

    updateSpeedDials();

    // Set focus to the editor
    m_nameEdit->setFocus();
}

EFXEditor::~EFXEditor()
{
    if (m_testButton->isChecked() == true)
        m_efx->stopAndWait();
}

void EFXEditor::stopTest()
{
    if (m_testButton->isChecked() == true)
        m_testButton->click();
}

void EFXEditor::slotFunctionManagerActive(bool active)
{
    if (active == true)
    {
        updateSpeedDials();
    }
    else
    {
        if (m_speedDials != NULL)
            m_speedDials->deleteLater();
        m_speedDials = NULL;
    }
}

void EFXEditor::initGeneralPage()
{
    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));

    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotFixtureItemChanged(QTreeWidgetItem*,int)));

    connect(m_addFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotAddFixtureClicked()));
    connect(m_removeFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveFixtureClicked()));

    connect(m_raiseFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotRaiseFixtureClicked()));
    connect(m_lowerFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotLowerFixtureClicked()));

    connect(m_parallelRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotParallelRadioToggled(bool)));
    connect(m_serialRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotSerialRadioToggled(bool)));
    connect(m_asymmetricRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotAsymmetricRadioToggled(bool)));

    // Test slots
    connect(m_testButton, SIGNAL(clicked()),
            this, SLOT(slotTestClicked()));
    connect(m_raiseFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotRestartTest()));
    connect(m_lowerFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotRestartTest()));
    connect(m_parallelRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotRestartTest()));
    connect(m_serialRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotRestartTest()));
    connect(m_asymmetricRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotRestartTest()));

    // Doc
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(slotFixtureRemoved()));
    connect(m_doc, SIGNAL(fixtureChanged(quint32)), this, SLOT(slotFixtureChanged()));

    /* Set the EFX's name to the name field */
    m_nameEdit->setText(m_efx->name());
    m_nameEdit->setSelection(0, m_nameEdit->text().length());

    /* Put all of the EFX's fixtures to the tree view */
    updateFixtureTree();

    /* Set propagation mode */
    if (m_efx->propagationMode() == EFX::Serial)
        m_serialRadio->setChecked(true);
    else if (m_efx->propagationMode() == EFX::Asymmetric)
        m_asymmetricRadio->setChecked(true);
    else
        m_parallelRadio->setChecked(true);

    /* Disable test button if we're in operate mode */
    if (m_doc->mode() == Doc::Operate)
        m_testButton->setEnabled(false);
}

void EFXEditor::initMovementPage()
{
    new QHBoxLayout(m_previewFrame);
    m_previewArea = new EFXPreviewArea(m_previewFrame);
    m_previewFrame->layout()->setMargin(0);
    m_previewFrame->layout()->addWidget(m_previewArea);

    /* Get supported algorithms and fill the algorithm combo with them */
    m_algorithmCombo->addItems(EFX::algorithmList());

    connect(m_loop, SIGNAL(clicked()),
            this, SLOT(slotLoopClicked()));
    connect(m_singleShot, SIGNAL(clicked()),
            this, SLOT(slotSingleShotClicked()));
    connect(m_pingPong, SIGNAL(clicked()),
            this, SLOT(slotPingPongClicked()));

    connect(m_forward, SIGNAL(clicked()),
            this, SLOT(slotForwardClicked()));
    connect(m_backward, SIGNAL(clicked()),
            this, SLOT(slotBackwardClicked()));

    connect(m_algorithmCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotAlgorithmSelected(const QString&)));
    connect(m_widthSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotWidthSpinChanged(int)));
    connect(m_heightSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotHeightSpinChanged(int)));
    connect(m_xOffsetSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotXOffsetSpinChanged(int)));
    connect(m_yOffsetSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotYOffsetSpinChanged(int)));
    connect(m_rotationSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotRotationSpinChanged(int)));
    connect(m_startOffsetSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotStartOffsetSpinChanged(int)));
    connect(m_isRelativeCheckbox, SIGNAL(stateChanged(int)),
            this, SLOT(slotIsRelativeCheckboxChanged(int)));

    connect(m_xFrequencySpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotXFrequencySpinChanged(int)));
    connect(m_yFrequencySpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotYFrequencySpinChanged(int)));
    connect(m_xPhaseSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotXPhaseSpinChanged(int)));
    connect(m_yPhaseSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotYPhaseSpinChanged(int)));

    QString algo(EFX::algorithmToString(m_efx->algorithm()));
    /* Select the EFX's algorithm from the algorithm combo */
    for (int i = 0; i < m_algorithmCombo->count(); i++)
    {
        if (m_algorithmCombo->itemText(i) == algo)
        {
            m_algorithmCombo->setCurrentIndex(i);
            break;
        }
    }

    /* Causes the EFX function to update the preview point array */
    slotAlgorithmSelected(algo);

    /* Get the algorithm parameters */
    m_widthSpin->setValue(m_efx->width());
    m_heightSpin->setValue(m_efx->height());
    m_xOffsetSpin->setValue(m_efx->xOffset());
    m_yOffsetSpin->setValue(m_efx->yOffset());
    m_rotationSpin->setValue(m_efx->rotation());
    m_startOffsetSpin->setValue(m_efx->startOffset());
    m_isRelativeCheckbox->setChecked(m_efx->isRelative());

    m_xFrequencySpin->setValue(m_efx->xFrequency());
    m_yFrequencySpin->setValue(m_efx->yFrequency());
    m_xPhaseSpin->setValue(m_efx->xPhase());
    m_yPhaseSpin->setValue(m_efx->yPhase());

    /* Running order */
    switch (m_efx->runOrder())
    {
    default:
    case Function::Loop:
        m_loop->setChecked(true);
        break;
    case Function::SingleShot:
        m_singleShot->setChecked(true);
        break;
    case Function::PingPong:
        m_pingPong->setChecked(true);
        break;
    }

    /* Direction */
    switch (m_efx->direction())
    {
    default:
    case Function::Forward:
        m_forward->setChecked(true);
        break;
    case Function::Backward:
        m_backward->setChecked(true);
        break;
    }

    redrawPreview();
}

void EFXEditor::slotTestClicked()
{
    if (m_testButton->isChecked() == true)
        m_efx->start(m_doc->masterTimer());
    else
        m_efx->stopAndWait();
}

void EFXEditor::slotRestartTest()
{
    if (m_testButton->isChecked() == true)
    {
        // Toggle off, toggle on. Duh.
        m_testButton->click();
        m_testButton->click();
    }
}

void EFXEditor::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        if (m_efx->stopped() == false)
            m_efx->stopAndWait();
        m_testButton->setChecked(false);
        m_testButton->setEnabled(false);
    }
    else
    {
        m_testButton->setEnabled(true);
    }
}

void EFXEditor::slotTabChanged(int tab)
{
    efxUiState()->setCurrentTab(tab);
}

bool EFXEditor::interruptRunning()
{
    if (m_testButton->isChecked() == true)
    {
        m_efx->stopAndWait();
        m_testButton->setChecked(false);
        return true;
    }
    else
    {
        return false;
    }
}

void EFXEditor::continueRunning(bool running)
{
    if (running == true)
    {
        if (m_doc->mode() == Doc::Operate)
            m_efx->start(m_doc->masterTimer());
        else
            m_testButton->click();
    }
}

EfxUiState * EFXEditor::efxUiState()
{
    return qobject_cast<EfxUiState*>(m_efx->uiState());
}

/*****************************************************************************
 * General page
 *****************************************************************************/

void EFXEditor::updateFixtureTree()
{
    m_tree->clear();
    QListIterator <EFXFixture*> it(m_efx->fixtures());
    while (it.hasNext() == true)
        addFixtureItem(it.next());
    m_tree->resizeColumnToContents(KColumnNumber);
    m_tree->resizeColumnToContents(KColumnName);
    m_tree->resizeColumnToContents(KColumnReverse);
    m_tree->resizeColumnToContents(KColumnStartOffset);
    m_tree->resizeColumnToContents(KColumnIntensity);
}

QTreeWidgetItem* EFXEditor::fixtureItem(EFXFixture* ef)
{
    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        QTreeWidgetItem* item = *it;
        EFXFixture* ef_item = reinterpret_cast<EFXFixture*>
                              (item->data(0, Qt::UserRole).toULongLong());
        if (ef_item == ef)
            return item;
        ++it;
    }

    return NULL;
}

const QList <EFXFixture*> EFXEditor::selectedFixtures() const
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    QList <EFXFixture*> list;

    /* Put all selected fixture IDs to a list and return it */
    while (it.hasNext() == true)
    {
        EFXFixture* ef = reinterpret_cast <EFXFixture*>
                         (it.next()->data(0, Qt::UserRole).toULongLong());
        list << ef;
    }

    return list;
}

void EFXEditor::updateIndices(int from, int to)
{
    QTreeWidgetItem* item;
    int i;

    for (i = from; i <= to; i++)
    {
        item = m_tree->topLevelItem(i);
        Q_ASSERT(item != NULL);

        item->setText(KColumnNumber,
                      QString("%1").arg(i + 1, 3, 10, QChar('0')));
    }
}

void EFXEditor::addFixtureItem(EFXFixture* ef)
{
    QTreeWidgetItem* item;
    Fixture* fxi;

    Q_ASSERT(ef != NULL);

    fxi = m_doc->fixture(ef->head().fxi);
    if (fxi == NULL)
        return;

    item = new QTreeWidgetItem(m_tree);

    if (fxi->heads() > 1)
    {
        item->setText(KColumnName, QString("%1 [%2]").arg(fxi->name()).arg(ef->head().head));
    }
    else
    {
        item->setText(KColumnName, fxi->name());
    }
    item->setData(0, Qt::UserRole, QVariant(reinterpret_cast<qulonglong> (ef)));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

    if (ef->direction() == Function::Backward)
        item->setCheckState(KColumnReverse, Qt::Checked);
    else
        item->setCheckState(KColumnReverse, Qt::Unchecked);

    updateIntensityColumn(item, ef);
    updateStartOffsetColumn(item, ef);

    updateIndices(m_tree->indexOfTopLevelItem(item),
                  m_tree->topLevelItemCount() - 1);

    /* Select newly-added fixtures so that they can be moved quickly */
    m_tree->setCurrentItem(item);
    redrawPreview();

    m_tree->resizeColumnToContents(KColumnNumber);
    m_tree->resizeColumnToContents(KColumnName);
    m_tree->resizeColumnToContents(KColumnReverse);
    m_tree->resizeColumnToContents(KColumnStartOffset);
    m_tree->resizeColumnToContents(KColumnIntensity);
}

void EFXEditor::updateIntensityColumn(QTreeWidgetItem* item, EFXFixture* ef)
{
    Q_ASSERT(item != NULL);
    Q_ASSERT(ef != NULL);

    if (m_tree->itemWidget(item, KColumnIntensity) == NULL)
    {
        QSpinBox* spin = new QSpinBox(m_tree);
        spin->setAutoFillBackground(true);
        spin->setRange(0, 255);
        spin->setValue(ef->fadeIntensity());
        m_tree->setItemWidget(item, KColumnIntensity, spin);
        spin->setProperty(PROPERTY_FIXTURE, (qulonglong) ef);
        connect(spin, SIGNAL(valueChanged(int)),
                this, SLOT(slotFixtureIntensityChanged(int)));
    }
}

void EFXEditor::updateStartOffsetColumn(QTreeWidgetItem* item, EFXFixture* ef)
{
    Q_ASSERT(item != NULL);
    Q_ASSERT(ef != NULL);

    if (m_tree->itemWidget(item, KColumnStartOffset) == NULL)
    {
        QSpinBox* spin = new QSpinBox(m_tree);
        spin->setAutoFillBackground(true);
        spin->setRange(0, 359);
        spin->setValue(ef->startOffset());
        spin->setSuffix(QChar(0x00b0)); // degree
        m_tree->setItemWidget(item, KColumnStartOffset, spin);
        spin->setProperty(PROPERTY_FIXTURE, (qulonglong) ef);
        connect(spin, SIGNAL(valueChanged(int)),
                this, SLOT(slotFixtureStartOffsetChanged(int)));
    }
}

void EFXEditor::removeFixtureItem(EFXFixture* ef)
{
    QTreeWidgetItem* item;
    int from;

    Q_ASSERT(ef != NULL);

    item = fixtureItem(ef);
    Q_ASSERT(item != NULL);

    from = m_tree->indexOfTopLevelItem(item);
    delete m_tree->itemWidget(item, KColumnIntensity);
    delete item;

    updateIndices(from, m_tree->topLevelItemCount() - 1);
    redrawPreview();

    m_tree->resizeColumnToContents(KColumnNumber);
    m_tree->resizeColumnToContents(KColumnName);
    m_tree->resizeColumnToContents(KColumnReverse);
    m_tree->resizeColumnToContents(KColumnStartOffset);
    m_tree->resizeColumnToContents(KColumnIntensity);
}

void EFXEditor::slotDialDestroyed(QObject *)
{
    m_speedDial->setChecked(false);
}

void EFXEditor::createSpeedDials()
{
    if (m_speedDials == NULL)
    {
        m_speedDials = new SpeedDialWidget(this);
        m_speedDials->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_speedDials, SIGNAL(fadeInChanged(int)), this, SLOT(slotFadeInChanged(int)));
        connect(m_speedDials, SIGNAL(fadeOutChanged(int)), this, SLOT(slotFadeOutChanged(int)));
        connect(m_speedDials, SIGNAL(holdChanged(int)), this, SLOT(slotHoldChanged(int)));
        connect(m_speedDials, SIGNAL(holdTapped()), this, SLOT(slotDurationTapped()));
        connect(m_speedDials, SIGNAL(destroyed(QObject*)), this, SLOT(slotDialDestroyed(QObject*)));
    }

    m_speedDials->show();
}

void EFXEditor::updateSpeedDials()
{
   if (m_speedDial->isChecked() == false)
        return;

    createSpeedDials();

    m_speedDials->setWindowTitle(m_efx->name());
    m_speedDials->setFadeInSpeed(m_efx->fadeInSpeed());
    m_speedDials->setFadeOutSpeed(m_efx->fadeOutSpeed());
    if ((int)m_efx->duration() < 0)
        m_speedDials->setDuration(m_efx->duration());
    else
        m_speedDials->setDuration(m_efx->duration() - m_efx->fadeInSpeed() - m_efx->fadeOutSpeed());
}

void EFXEditor::slotNameEdited(const QString &text)
{
    m_efx->setName(text);
    if (m_speedDials)
        m_speedDials->setWindowTitle(text);
}

void EFXEditor::slotSpeedDialToggle(bool state)
{
    efxUiState()->setShowSpeedDial(state);

    if (state == true)
        updateSpeedDials();
    else
    {
        if (m_speedDials != NULL)
            m_speedDials->deleteLater();
        m_speedDials = NULL;
    }
}

void EFXEditor::slotFixtureItemChanged(QTreeWidgetItem* item, int column)
{
    if (column == KColumnReverse)
    {
        EFXFixture* ef = reinterpret_cast <EFXFixture*>
                         (item->data(0, Qt::UserRole).toULongLong());
        Q_ASSERT(ef != NULL);

        if (item->checkState(column) == Qt::Checked)
            ef->setDirection(Function::Backward);
        else
            ef->setDirection(Function::Forward);

	redrawPreview();
    }
}

void EFXEditor::slotFixtureIntensityChanged(int intensity)
{
    QSpinBox* spin = qobject_cast<QSpinBox*>(QObject::sender());
    Q_ASSERT(spin != NULL);
    EFXFixture* ef = (EFXFixture*) spin->property(PROPERTY_FIXTURE).toULongLong();
    Q_ASSERT(ef != NULL);
    ef->setFadeIntensity(uchar(intensity));

    // Restart the test after the latest intensity change, delayed
    m_testTimer.start();
}

void EFXEditor::slotFixtureStartOffsetChanged(int startOffset)
{
    QSpinBox* spin = qobject_cast<QSpinBox*>(QObject::sender());
    Q_ASSERT(spin != NULL);
    EFXFixture* ef = (EFXFixture*) spin->property(PROPERTY_FIXTURE).toULongLong();
    Q_ASSERT(ef != NULL);
    ef->setStartOffset(startOffset);

    // Restart the test after the latest intensity change, delayed
    m_testTimer.start();
}

void EFXEditor::slotAddFixtureClicked()
{
    /* Put all fixtures already present into a list of fixtures that
       will be disabled in the fixture selection dialog */
    QList <GroupHead> disabled;
    QTreeWidgetItemIterator twit(m_tree);
    while (*twit != NULL)
    {
        EFXFixture* ef = reinterpret_cast <EFXFixture*>
                         ((*twit)->data(0, Qt::UserRole).toULongLong());
        Q_ASSERT(ef != NULL);

        disabled.append(ef->head());
        twit++;
    }

    /* Disable all fixtures that don't have pan OR tilt channels */
    QListIterator <Fixture*> fxit(m_doc->fixtures());
    while (fxit.hasNext() == true)
    {
        Fixture* fixture(fxit.next());
        Q_ASSERT(fixture != NULL);

        // If a channel with pan group exists, don't disable this fixture
        if (fixture->channel(QLCChannel::Pan) != QLCChannel::invalid())
        {
            continue;
        }

        // If a channel with tilt group exists, don't disable this fixture
        if (fixture->channel(QLCChannel::Tilt) != QLCChannel::invalid())
        {
            continue;
        }

        // Disable all fixtures without pan or tilt channels
        disabled << fixture->id();
    }

    /* Get a list of new fixtures to add to the scene */
    FixtureSelection fs(this, m_doc);
    fs.setMultiSelection(true);
    fs.setSelectionMode(FixtureSelection::Heads);
    fs.setDisabledHeads(disabled);
    if (fs.exec() == QDialog::Accepted)
    {
        // Stop running while adding fixtures
        bool running = interruptRunning();

        QListIterator <GroupHead> it(fs.selectedHeads());
        while (it.hasNext() == true)
        {
            EFXFixture* ef = new EFXFixture(m_efx);
            ef->setHead(it.next());

            if (m_efx->addFixture(ef) == true)
                addFixtureItem(ef);
            else
                delete ef;
        }

        redrawPreview();

        // Continue running if appropriate
        continueRunning(running);
    }
}

void EFXEditor::slotRemoveFixtureClicked()
{
    int r = QMessageBox::question(
                this, tr("Remove fixtures"),
                tr("Do you want to remove the selected fixture(s)?"),
                QMessageBox::Yes, QMessageBox::No);

    if (r == QMessageBox::Yes)
    {
        // Stop running while removing fixtures
        bool running = interruptRunning();

        QListIterator <EFXFixture*> it(selectedFixtures());
        while (it.hasNext() == true)
        {
            EFXFixture* ef = it.next();
            Q_ASSERT(ef != NULL);

            removeFixtureItem(ef);
            if (m_efx->removeFixture(ef) == true)
                delete ef;
        }

	redrawPreview();

        // Continue if appropriate
        continueRunning(running);
    }
}

void EFXEditor::slotRaiseFixtureClicked()
{
    // Stop running while moving fixtures
    bool running = interruptRunning();

    QTreeWidgetItem* item = m_tree->currentItem();
    if (item != NULL)
    {
        int index = m_tree->indexOfTopLevelItem(item);
        if (index == 0)
            return;

        EFXFixture* ef = reinterpret_cast <EFXFixture*>
                         (item->data(0, Qt::UserRole).toULongLong());
        Q_ASSERT(ef != NULL);

        if (m_efx->raiseFixture(ef) == true)
        {
            item = m_tree->takeTopLevelItem(index);

            m_tree->insertTopLevelItem(index - 1, item);
            m_tree->setCurrentItem(item);
            updateIntensityColumn(item, ef);
            updateStartOffsetColumn(item, ef);

            updateIndices(index - 1, index);
	    redrawPreview();
        }
    }

    // Continue running if appropriate
    continueRunning(running);
}

void EFXEditor::slotLowerFixtureClicked()
{
    // Stop running while moving fixtures
    bool running = interruptRunning();

    QTreeWidgetItem* item = m_tree->currentItem();
    if (item != NULL)
    {
        int index = m_tree->indexOfTopLevelItem(item);
        if (index == (m_tree->topLevelItemCount() - 1))
            return;

        EFXFixture* ef = reinterpret_cast <EFXFixture*>
                         (item->data(0, Qt::UserRole).toULongLong());
        Q_ASSERT(ef != NULL);

        if (m_efx->lowerFixture(ef) == true)
        {
            item = m_tree->takeTopLevelItem(index);
            m_tree->insertTopLevelItem(index + 1, item);
            m_tree->setCurrentItem(item);
            updateIntensityColumn(item, ef);
            updateStartOffsetColumn(item, ef);

            updateIndices(index, index + 1);
	    redrawPreview();
        }
    }

    // Continue running if appropriate
    continueRunning(running);
}

void EFXEditor::slotParallelRadioToggled(bool state)
{
    Q_ASSERT(m_efx != NULL);
    if (state == true)
        m_efx->setPropagationMode(EFX::Parallel);
}

void EFXEditor::slotSerialRadioToggled(bool state)
{
    Q_ASSERT(m_efx != NULL);
    if (state == true)
        m_efx->setPropagationMode(EFX::Serial);
}

void EFXEditor::slotAsymmetricRadioToggled(bool state)
{
    Q_ASSERT(m_efx != NULL);
    if (state == true)
        m_efx->setPropagationMode(EFX::Asymmetric);
}

void EFXEditor::slotFadeInChanged(int ms)
{
    m_efx->setFadeInSpeed(ms);
    slotRestartTest();
}

void EFXEditor::slotFadeOutChanged(int ms)
{
    m_efx->setFadeOutSpeed(ms);
}

void EFXEditor::slotHoldChanged(int ms)
{
    uint duration = 0;
    if (ms < 0)
        duration = ms;
    else
        duration = m_efx->fadeInSpeed() + ms + m_efx->fadeOutSpeed();
    m_efx->setDuration(duration);
    redrawPreview();
}

void EFXEditor::slotFixtureRemoved()
{
    // EFX already catches fixture removals so just update the list
    updateFixtureTree();
    redrawPreview();
}

void EFXEditor::slotFixtureChanged()
{
    // Update the tree in case fixture's name changes
    updateFixtureTree();
}

/*****************************************************************************
 * Movement page
 *****************************************************************************/

void EFXEditor::slotAlgorithmSelected(const QString &text)
{
    Q_ASSERT(m_efx != NULL);

    EFX::Algorithm algo = EFX::stringToAlgorithm(text);
    m_efx->setAlgorithm(algo);

    if (m_efx->isFrequencyEnabled())
    {
        m_xFrequencyLabel->setEnabled(true);
        m_yFrequencyLabel->setEnabled(true);

        m_xFrequencySpin->setEnabled(true);
        m_yFrequencySpin->setEnabled(true);
    }
    else
    {
        m_xFrequencyLabel->setEnabled(false);
        m_yFrequencyLabel->setEnabled(false);

        m_xFrequencySpin->setEnabled(false);
        m_yFrequencySpin->setEnabled(false);
    }

    if (m_efx->isPhaseEnabled())
    {
        m_xPhaseLabel->setEnabled(true);
        m_yPhaseLabel->setEnabled(true);

        m_xPhaseSpin->setEnabled(true);
        m_yPhaseSpin->setEnabled(true);
    }
    else
    {
        m_xPhaseLabel->setEnabled(false);
        m_yPhaseLabel->setEnabled(false);

        m_xPhaseSpin->setEnabled(false);
        m_yPhaseSpin->setEnabled(false);
    }

    redrawPreview();
}

void EFXEditor::slotWidthSpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setWidth(value);
    redrawPreview();
}

void EFXEditor::slotHeightSpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setHeight(value);
    redrawPreview();
}

void EFXEditor::slotRotationSpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setRotation(value);
    redrawPreview();
}

void EFXEditor::slotStartOffsetSpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setStartOffset(value);
    redrawPreview();
}

void EFXEditor::slotIsRelativeCheckboxChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setIsRelative(value == Qt::Checked);
}

void EFXEditor::slotXOffsetSpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setXOffset(value);
    redrawPreview();
}

void EFXEditor::slotYOffsetSpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setYOffset(value);
    redrawPreview();
}

void EFXEditor::slotXFrequencySpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setXFrequency(value);
    redrawPreview();
}

void EFXEditor::slotYFrequencySpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setYFrequency(value);
    redrawPreview();
}

void EFXEditor::slotXPhaseSpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setXPhase(value);
    redrawPreview();
}

void EFXEditor::slotYPhaseSpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setYPhase(value);
    redrawPreview();
}

/*****************************************************************************
 * Run order
 *****************************************************************************/

void EFXEditor::slotLoopClicked()
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setRunOrder(Function::Loop);
}

void EFXEditor::slotSingleShotClicked()
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setRunOrder(Function::SingleShot);
}

void EFXEditor::slotPingPongClicked()
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setRunOrder(Function::PingPong);
}

/*****************************************************************************
 * Direction
 *****************************************************************************/

void EFXEditor::slotForwardClicked()
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setDirection(Function::Forward);
    redrawPreview();
}

void EFXEditor::slotBackwardClicked()
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setDirection(Function::Backward);
    redrawPreview();
}

void EFXEditor::redrawPreview()
{
    if (m_previewArea == NULL)
        return;

    QVector <QPoint> points;
    m_efx->preview(points);

    QVector <QVector <QPoint> > fixturePoints;
    m_efx->previewFixtures(fixturePoints);
 
    m_previewArea->setPoints(points);
    m_previewArea->setFixturePoints(fixturePoints);

    m_previewArea->draw(m_efx->duration() / points.size());
}
