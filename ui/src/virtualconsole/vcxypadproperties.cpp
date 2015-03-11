/*
  Q Light Controller
  vcxypadproperties.h

  Copyright (C) Stefan Krumm, Heikki Junnila

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
#include <QHeaderView>
#include <QSettings>

#include "qlcinputchannel.h"
#include "qlcchannel.h"

#include "vcxypadfixtureeditor.h"
#include "selectinputchannel.h"
#include "vcxypadproperties.h"
#include "fixtureselection.h"
#include "vcxypadfixture.h"
#include "inputpatch.h"
#include "vcxypad.h"
#include "apputil.h"
#include "doc.h"

#define SETTINGS_GEOMETRY "vcxypad/geometry"

#define KColumnFixture   0
#define KColumnXAxis     1
#define KColumnYAxis     2

/****************************************************************************
 * Initialization
 ****************************************************************************/

VCXYPadProperties::VCXYPadProperties(VCXYPad* xypad, Doc* doc)
    : QDialog(xypad)
    , m_xypad(xypad)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(xypad != NULL);

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    m_nameEdit->setText(m_xypad->caption());

    if (m_xypad->invertedAppearance() == true)
        m_YInvertedRadio->setChecked(true);

    slotSelectionChanged(NULL);
    fillTree();

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);

    m_panInputSource = xypad->inputSource(VCXYPad::panInputSourceId);
    m_tiltInputSource = xypad->inputSource(VCXYPad::tiltInputSourceId);
    updatePanInputSource();
    updateTiltInputSource();
}

VCXYPadProperties::~VCXYPadProperties()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

/****************************************************************************
 * Fixtures page
 ****************************************************************************/

void VCXYPadProperties::fillTree()
{
    m_tree->clear();

    QListIterator <VCXYPadFixture> it(m_xypad->fixtures());
    while (it.hasNext() == true)
        updateFixtureItem(new QTreeWidgetItem(m_tree), it.next());
    m_tree->setCurrentItem(m_tree->topLevelItem(0));
    m_tree->resizeColumnToContents(KColumnFixture);
    m_tree->resizeColumnToContents(KColumnXAxis);
    m_tree->resizeColumnToContents(KColumnYAxis);
}

void VCXYPadProperties::updateFixtureItem(QTreeWidgetItem* item,
                                          const VCXYPadFixture& fxi)
{
    Q_ASSERT(item != NULL);

    item->setText(KColumnFixture, fxi.name());
    item->setText(KColumnXAxis, fxi.xBrief());
    item->setText(KColumnYAxis, fxi.yBrief());
    item->setData(KColumnFixture, Qt::UserRole, QVariant(fxi));
}

QList <VCXYPadFixture> VCXYPadProperties::selectedFixtures() const
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    QList <VCXYPadFixture> list;

    /* Put all selected fixtures to a list and return it */
    while (it.hasNext() == true)
        list << VCXYPadFixture(m_doc, it.next()->data(KColumnFixture, Qt::UserRole));

    return list;
}

QTreeWidgetItem* VCXYPadProperties::fixtureItem(const VCXYPadFixture& fxi)
{
    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        QVariant var((*it)->data(KColumnFixture, Qt::UserRole));
        VCXYPadFixture another(m_doc, var);
        if (fxi.head() == another.head())
            return *it;
        else
            ++it;
    }

    return NULL;
}

void VCXYPadProperties::removeFixtureItem(GroupHead const & head)
{
    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        QVariant var((*it)->data(KColumnFixture, Qt::UserRole));
        VCXYPadFixture fxi(m_doc, var);
        if (fxi.head() == head)
        {
            delete (*it);
            break;
        }

        ++it;
    }
}

void VCXYPadProperties::slotAddClicked()
{
    /* Put all fixtures already present into a list of fixtures that
       will be disabled in the fixture selection dialog */
    QList <GroupHead> disabled;
    QTreeWidgetItemIterator twit(m_tree);
    while (*twit != NULL)
    {
        QVariant var((*twit)->data(KColumnFixture, Qt::UserRole));
        VCXYPadFixture fxi(m_doc, var);
        disabled << fxi.head();
        ++twit;
    }

    /* Disable all fixtures that don't have pan OR tilt channels */
    foreach(Fixture* fixture, m_doc->fixtures())
    {
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

    /* Get a list of new fixtures to add to the pad */
    QTreeWidgetItem* item = NULL;
    FixtureSelection fs(this, m_doc);
    fs.setMultiSelection(true);
    fs.setSelectionMode(FixtureSelection::Heads);
    fs.setDisabledHeads(disabled);
    if (fs.exec() == QDialog::Accepted)
    {
        QListIterator <GroupHead> it(fs.selectedHeads());
        while (it.hasNext() == true)
        {
            VCXYPadFixture fxi(m_doc);
            fxi.setHead(it.next());
            item = new QTreeWidgetItem(m_tree);
            updateFixtureItem(item, fxi);
        }
    }

    if (item != NULL)
        m_tree->setCurrentItem(item);

    m_tree->resizeColumnToContents(KColumnFixture);
    m_tree->resizeColumnToContents(KColumnXAxis);
    m_tree->resizeColumnToContents(KColumnYAxis);
}

void VCXYPadProperties::slotRemoveClicked()
{
    int r = QMessageBox::question(
                this, tr("Remove fixtures"),
                tr("Do you want to remove the selected fixtures?"),
                QMessageBox::Yes, QMessageBox::No);

    if (r == QMessageBox::Yes)
    {
        QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
        while (it.hasNext() == true)
            delete it.next();
    }
}

void VCXYPadProperties::slotEditClicked()
{
    /* Get a list of selected fixtures */
    QList <VCXYPadFixture> list(selectedFixtures());

    /* Start editor */
    VCXYPadFixtureEditor editor(this, list);
    if (editor.exec() == QDialog::Accepted)
    {
        QListIterator <VCXYPadFixture> it(editor.fixtures());
        while (it.hasNext() == true)
        {
            VCXYPadFixture fxi(it.next());
            QTreeWidgetItem* item = fixtureItem(fxi);

            updateFixtureItem(item, fxi);
        }
    }
}

void VCXYPadProperties::slotSelectionChanged(QTreeWidgetItem* item)
{
    if (item == NULL)
    {
        m_removeButton->setEnabled(false);
        m_editButton->setEnabled(false);
    }
    else
    {
        m_removeButton->setEnabled(true);
        m_editButton->setEnabled(true);
    }
}

/****************************************************************************
 * Input page
 ****************************************************************************/

void VCXYPadProperties::slotPanAutoDetectToggled(bool toggled)
{
    if (toggled == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotPanInputValueChanged(quint32,quint32)));
        m_tiltAutoDetectButton->setChecked(false);
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotPanInputValueChanged(quint32,quint32)));
    }
}

void VCXYPadProperties::slotPanChooseClicked()
{
    m_panAutoDetectButton->setChecked(false);
    m_tiltAutoDetectButton->setChecked(false);

    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        if (m_panInputSource != NULL)
            delete m_panInputSource;
        m_panInputSource = new QLCInputSource(sic.universe(), sic.channel());
        updatePanInputSource();
    }
}

void VCXYPadProperties::slotPanInputValueChanged(quint32 uni, quint32 ch)
{
    QLCInputSource *tmpSource = new QLCInputSource(uni, (m_xypad->page() << 16) | ch);
    // if both Pan and Tilt come from the same external control, here's
    // where I will discover it
    if (m_panInputSource != NULL &&
        m_panInputSource->channel() != UINT_MAX &&
        tmpSource->channel() != m_panInputSource->channel())
    {
        if (m_tiltInputSource != NULL)
            delete m_tiltInputSource;
        m_tiltInputSource = new QLCInputSource(uni, (m_xypad->page() << 16) | ch);
        updateTiltInputSource();
        return;
    }

    if (m_panInputSource != NULL)
        delete m_panInputSource;
    m_panInputSource = new QLCInputSource(uni, (m_xypad->page() << 16) | ch);
    updatePanInputSource();
    delete tmpSource;
}

void VCXYPadProperties::slotTiltAutoDetectToggled(bool toggled)
{
    if (toggled == true)
    {
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotTiltInputValueChanged(quint32,quint32)));
        m_panAutoDetectButton->setChecked(false);
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotTiltInputValueChanged(quint32,quint32)));
    }
}

void VCXYPadProperties::slotTiltChooseClicked()
{
    m_panAutoDetectButton->setChecked(false);
    m_tiltAutoDetectButton->setChecked(false);

    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        if (m_tiltInputSource != NULL)
            delete m_tiltInputSource;
        m_tiltInputSource = new QLCInputSource(sic.universe(), sic.channel());
        updateTiltInputSource();
    }
}

void VCXYPadProperties::slotTiltInputValueChanged(quint32 uni, quint32 ch)
{
    QLCInputSource *tmpSource = new QLCInputSource(uni, (m_xypad->page() << 16) | ch);
    // if both Pan and Tilt come from the same external control, here's
    // where I will discover it
    if (m_tiltInputSource != NULL &&
        m_tiltInputSource->channel() != UINT_MAX &&
        tmpSource->channel() != m_tiltInputSource->channel())
    {
        if (m_panInputSource != NULL)
            delete m_panInputSource;
        m_panInputSource = new QLCInputSource(uni, (m_xypad->page() << 16) | ch);
        updatePanInputSource();
        return;
    }

    if (m_tiltInputSource != NULL)
        delete m_tiltInputSource;
    m_tiltInputSource = new QLCInputSource(uni, (m_xypad->page() << 16) | ch);
    updateTiltInputSource();
    delete tmpSource;
}

void VCXYPadProperties::updatePanInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputOutputMap()->inputSourceNames(m_panInputSource, uniName, chName) == false)
    {
        uniName = KInputNone;
        chName = KInputNone;
    }

    m_panUniverseEdit->setText(uniName);
    m_panChannelEdit->setText(chName);
}

void VCXYPadProperties::updateTiltInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputOutputMap()->inputSourceNames(m_tiltInputSource, uniName, chName) == false)
    {
        uniName = KInputNone;
        chName = KInputNone;
    }

    m_tiltUniverseEdit->setText(uniName);
    m_tiltChannelEdit->setText(chName);
}

/****************************************************************************
 * OK/Cancel
 ****************************************************************************/

void VCXYPadProperties::accept()
{
    m_xypad->clearFixtures();
    m_xypad->setCaption(m_nameEdit->text());
    m_xypad->setInputSource(m_panInputSource, VCXYPad::panInputSourceId);
    m_xypad->setInputSource(m_tiltInputSource, VCXYPad::tiltInputSourceId);
    if (m_YNormalRadio->isChecked())
        m_xypad->setInvertedAppearance(false);
    else
        m_xypad->setInvertedAppearance(true);

    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        QVariant var((*it)->data(KColumnFixture, Qt::UserRole));
        m_xypad->appendFixture(VCXYPadFixture(m_doc, var));
        ++it;
    }

    QDialog::accept();
}
