/*
  Q Light Controller Plus
  channelmodifiereditor.cpp

  Copyright (c) Massimo Callegari

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

#include <QMessageBox>
#include <QUrl>
#include <QSettings>

#include "channelmodifiergraphicsview.h"
#include "channelmodifiereditor.h"
#include "qlcmodifierscache.h"
#include "channelmodifier.h"
#include "qlcfile.h"
#include "doc.h"

#define SETTINGS_GEOMETRY "channelmodifiereditor/geometry"

ChannelModifierEditor::ChannelModifierEditor(Doc *doc, QString modifier, QWidget *parent)
    : QDialog(parent)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);

    setupUi(this);

    m_view = new ChannelModifierGraphicsView(this);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setAcceptDrops(true);
    m_view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_view->setBackgroundBrush(QBrush(QColor(11, 11, 11, 255), Qt::SolidPattern));

    m_mainGrid->addWidget(m_view, 2, 0);

    m_origDMXSpin->setEnabled(false);
    m_modifiedDMXSpin->setEnabled(false);
    m_deleteHandlerButton->setEnabled(false);

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());

    connect(m_view, SIGNAL(itemClicked(uchar,uchar)),
            this, SLOT(slotHandlerClicked(uchar,uchar)));
    connect(m_view, SIGNAL(itemDMXMapChanged(uchar,uchar)),
            this, SLOT(slotItemDMXChanged(uchar,uchar)));
    connect(m_view, SIGNAL(viewClicked(QMouseEvent*)),
            this, SLOT(slotViewClicked()));

    connect(m_templatesTree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));

    connect(m_origDMXSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotOriginalDMXValueChanged(int)));
    connect(m_modifiedDMXSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotModifiedDMXValueChanged(int)));

    connect(m_addHandlerButton, SIGNAL(clicked()),
            this, SLOT(slotAddHandlerClicked()));
    connect(m_deleteHandlerButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveHandlerClicked()));
    connect(m_saveButton, SIGNAL(clicked()),
            this, SLOT(slotSaveClicked()));

    connect(m_unsetButton, SIGNAL(clicked()),
            this, SLOT(slotUnsetClicked()));

    updateModifiersList(modifier);
}

ChannelModifierEditor::~ChannelModifierEditor()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

ChannelModifier *ChannelModifierEditor::selectedModifier()
{
    return m_currentTemplate;
}

static bool alphabeticSort(QString const & left, QString const & right)
{
  return QString::compare(left, right) < 0;
}

void ChannelModifierEditor::updateModifiersList(QString modifier)
{
    QList<QString> names = m_doc->modifiersCache()->templateNames();
    std::stable_sort(names.begin(), names.end(), alphabeticSort);

    m_templatesTree->clear();
    foreach (QString name, names)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_templatesTree);
        item->setText(0, name);
        if (name == modifier)
            item->setSelected(true);
    }
    if (m_templatesTree->topLevelItemCount() > 0 &&
        m_templatesTree->selectedItems().count() == 0)
            m_templatesTree->setCurrentItem(m_templatesTree->topLevelItem(0));
}

void ChannelModifierEditor::slotViewClicked()
{
    m_origDMXSpin->setEnabled(false);
    m_modifiedDMXSpin->setEnabled(false);
    m_deleteHandlerButton->setEnabled(false);
}

void ChannelModifierEditor::slotHandlerClicked(uchar pos, uchar value)
{
    if (pos != 0 && pos != 255)
    {
        m_origDMXSpin->setEnabled(true);
        m_deleteHandlerButton->setEnabled(true);
    }
    else
        m_deleteHandlerButton->setEnabled(false);
    m_modifiedDMXSpin->setEnabled(true);
    m_origDMXSpin->blockSignals(true);
    m_modifiedDMXSpin->blockSignals(true);
    m_origDMXSpin->setValue(pos);
    m_modifiedDMXSpin->setValue(value);
    m_origDMXSpin->blockSignals(false);
    m_modifiedDMXSpin->blockSignals(false);
}

void ChannelModifierEditor::slotItemDMXChanged(uchar pos, uchar value)
{
    m_origDMXSpin->blockSignals(true);
    m_modifiedDMXSpin->blockSignals(true);
    m_origDMXSpin->setValue(pos);
    m_modifiedDMXSpin->setValue(value);
    m_origDMXSpin->blockSignals(false);
    m_modifiedDMXSpin->blockSignals(false);
}

void ChannelModifierEditor::slotItemSelectionChanged()
{
    if (m_templatesTree->selectedItems().count() > 0)
    {
        QTreeWidgetItem *item = m_templatesTree->selectedItems().first();
        m_currentTemplate = m_doc->modifiersCache()->modifier(item->text(0));
        m_view->setModifierMap(m_currentTemplate->modifierMap());
        m_templateNameEdit->setText(m_currentTemplate->name());
    }
}

void ChannelModifierEditor::slotOriginalDMXValueChanged(int value)
{
    m_view->setHandlerDMXValue(uchar(value), uchar(m_modifiedDMXSpin->value()));
}

void ChannelModifierEditor::slotModifiedDMXValueChanged(int value)
{
    m_view->setHandlerDMXValue(uchar(m_origDMXSpin->value()), uchar(value));
}

void ChannelModifierEditor::slotAddHandlerClicked()
{
    m_view->addNewHandler();
}

void ChannelModifierEditor::slotRemoveHandlerClicked()
{
    m_view->removeHander();
}

void ChannelModifierEditor::slotSaveClicked()
{
    ChannelModifier *modifier = m_doc->modifiersCache()->modifier(m_templateNameEdit->text());
    if (modifier != NULL && modifier->type() == ChannelModifier::SystemTemplate)
    {
        // cannot overwrite a system template !
        QMessageBox::critical(this, tr("Error"),
                              tr("You are trying to overwrite a system template! Please choose another name "
                                 "and the template will be saved in your channel modifier's user folder."),
                              QMessageBox::Close);
        return;
    }

    QList< QPair<uchar, uchar> > map = m_view->modifiersMap();
    QString filename = QString("%1/%2%3")
            .arg(QLCModifiersCache::userTemplateDirectory().absolutePath())
            .arg(m_templateNameEdit->text().simplified())
            .arg(KExtModifierTemplate);
    ChannelModifier *newModifier = new ChannelModifier();
    newModifier->setName(m_templateNameEdit->text());
    newModifier->setModifierMap(map);
    newModifier->saveXML(filename);

    if (modifier == NULL)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_templatesTree);
        item->setText(0, m_templateNameEdit->text());
        m_doc->modifiersCache()->addModifier(newModifier);
    }
    else
        modifier->setModifierMap(map);
}

void ChannelModifierEditor::slotUnsetClicked()
{
    m_currentTemplate = NULL;
    QDialog::accept();
}
