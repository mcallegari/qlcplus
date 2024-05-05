/*
  Q Light Controller Plus
  monitorbackgroundselection.cpp

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

#include <QTreeWidgetItem>
#include <QFileDialog>
#include <QSettings>

#include "monitorbackgroundselection.h"
#include "monitorproperties.h"
#include "functionselection.h"
#include "function.h"
#include "doc.h"

#define KColumnName     0
#define KColumnImage    1

#define SETTINGS_GEOMETRY "monitorbackgroundselection/geometry"

MonitorBackgroundSelection::MonitorBackgroundSelection(QWidget *parent, Doc *doc)
    : QDialog(parent)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);
    setupUi(this);

    m_props = doc->monitorProperties();
    Q_ASSERT(m_props != NULL);

    m_commonBackgroundImage = m_props->commonBackgroundImage();
    m_customBackgroundImages = m_props->customBackgroundList();

    m_lastUsedPath = QString();

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());

    connect(m_noBgRadio, SIGNAL(clicked(bool)),
            this, SLOT(slotNoBackgroundChecked(bool)));
    connect(m_commonBgRadio, SIGNAL(clicked(bool)),
            this, SLOT(slotCommonBackgroundChecked(bool)));
    connect(m_customBgRadio, SIGNAL(clicked(bool)),
            this, SLOT(slotCustomBackgroundChecked(bool)));

    if (m_commonBackgroundImage.isEmpty() == false)
    {
        m_commonBgRadio->setChecked(true);
        slotCommonBackgroundChecked(true);
    }
    else if (m_customBackgroundImages.isEmpty() == false)
    {
        m_customBgRadio->setChecked(true);
        slotCustomBackgroundChecked(true);
    }
    else
    {
        m_noBgRadio->setChecked(true);
        slotNoBackgroundChecked(true);
    }

    updateCustomTree();

    connect(m_commonButton, SIGNAL(clicked()),
            this, SLOT(slotSelectCommonBackground()));
    connect(m_customAddButton, SIGNAL(clicked()),
            this, SLOT(slotAddCustomBackground()));
    connect(m_customRemoveButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveCustomBackground()));
}

MonitorBackgroundSelection::~MonitorBackgroundSelection()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void MonitorBackgroundSelection::accept()
{
    m_props->setCommonBackgroundImage(QString());
    m_props->resetCustomBackgroundList();

    if (m_commonBgRadio->isChecked())
    {
        m_props->setCommonBackgroundImage(m_commonBackgroundImage);
    }
    else if (m_customBgRadio->isChecked())
    {
        m_props->setCustomBackgroundList(m_customBackgroundImages);
    }
    QDialog::accept();
}

void MonitorBackgroundSelection::updateCustomTree()
{
    m_customTree->clear();
    QMapIterator <quint32, QString> it(m_customBackgroundImages);
    while (it.hasNext() == true)
    {
        it.next();

        quint32 fid = it.key();
        Function *f = m_doc->function(fid);
        if (f != NULL)
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(m_customTree);
            item->setIcon(KColumnName, f->getIcon());
            item->setText(KColumnName, f->name());
            item->setData(KColumnName, Qt::UserRole, fid);
            item->setText(KColumnImage, it.value());
        }
    }
}

void MonitorBackgroundSelection::slotNoBackgroundChecked(bool checked)
{
    if (checked == true)
    {
        m_commonButton->setEnabled(false);
        m_customTree->setEnabled(false);
        m_customAddButton->setEnabled(false);
        m_customRemoveButton->setEnabled(false);
    }
}

void MonitorBackgroundSelection::slotCommonBackgroundChecked(bool checked)
{
    if (checked == true)
    {
        m_commonButton->setEnabled(true);
        m_customTree->setEnabled(false);
        m_customAddButton->setEnabled(false);
        m_customRemoveButton->setEnabled(false);
    }
}

void MonitorBackgroundSelection::slotCustomBackgroundChecked(bool checked)
{
    if (checked == true)
    {
        m_commonButton->setEnabled(false);
        m_customTree->setEnabled(true);
        m_customAddButton->setEnabled(true);
        m_customRemoveButton->setEnabled(true);
    }
}

void MonitorBackgroundSelection::slotSelectCommonBackground()
{
    QString filename = m_props->commonBackgroundImage();

    filename = QFileDialog::getOpenFileName(this,
                            tr("Select background image"),
                            m_lastUsedPath,
                            QString("%1 (*.png *.bmp *.jpg *.jpeg *.gif)").arg(tr("Images")));

    if (filename.isEmpty() == false)
    {
        m_commonLabel->setText(filename);
        m_commonBackgroundImage = filename;
        m_lastUsedPath = QFileInfo(filename).canonicalPath();
    }
}

void MonitorBackgroundSelection::slotAddCustomBackground()
{
    FunctionSelection fs(this, m_doc);
    fs.setDisabledFunctions(m_customBackgroundImages.keys());
    fs.setMultiSelection(false);

    if (fs.exec() == QDialog::Accepted)
    {
        quint32 fid = fs.selection().first();
        QString filename = QFileDialog::getOpenFileName(this,
                                tr("Select background image"),
                                m_lastUsedPath,
                                QString("%1 (*.png *.bmp *.jpg *.jpeg *.gif)").arg(tr("Images")));

        if (filename.isEmpty() == false)
        {
            m_customBackgroundImages[fid] = filename;
            updateCustomTree();
            m_lastUsedPath = QFileInfo(filename).canonicalPath();
        }
    }
}

void MonitorBackgroundSelection::slotRemoveCustomBackground()
{
    if (m_customTree->selectedItems().isEmpty())
        return;

    QTreeWidgetItem *selItem = m_customTree->selectedItems().first();
    quint32 fid = selItem->data(KColumnName, Qt::UserRole).toUInt();
    m_customBackgroundImages.remove(fid);
    updateCustomTree();
}
