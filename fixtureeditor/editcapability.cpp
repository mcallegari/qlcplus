/*
  Q Light Controller - Fixture Definition Editor
  editcapability.cpp

  Copyright (C) Heikki Junnila

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

#include <QColorDialog>
#include <QFileDialog>
#include <QLineEdit>
#include <QSettings>
#include <QSpinBox>

#include "editcapability.h"
#include "qlccapability.h"
#include "qlcconfig.h"
#include "util.h"

#define KSettingsGeometry "editcapability/geometry"

EditCapability::EditCapability(QWidget* parent, const QLCCapability* cap, QLCChannel::Group group)
    : QDialog(parent)
{
    m_capability = new QLCCapability(cap);

    setupUi(this);

    if (group != QLCChannel::Gobo)
        m_resourceGroup->hide();

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    m_minSpin->setValue(m_capability->min());
    m_maxSpin->setValue(m_capability->max());
    m_descriptionEdit->setText(m_capability->name());
    m_descriptionEdit->setValidator(CAPS_VALIDATOR(this));
    m_minSpin->setFocus();
    m_minSpin->selectAll();

    if (m_capability->resourceName().isEmpty() == false)
        m_resourceButton->setIcon(QIcon(m_capability->resourceName()));
    else if (m_capability->resourceColor().isValid())
    {
        QPixmap pix(58, 58);
        pix.fill(m_capability->resourceColor());
        m_resourceButton->setIcon(pix);
    }

    connect(m_minSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotMinSpinChanged(int)));
    connect(m_maxSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotMaxSpinChanged(int)));
    connect(m_descriptionEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotDescriptionEdited(const QString&)));
    connect(m_resourceButton, SIGNAL(pressed()),
            this, SLOT(slotResourceButtonPressed()));
    connect(m_colorButton, SIGNAL(pressed()),
            this, SLOT(slotColorButtonPressed()));

    QSettings settings;
    QVariant var = settings.value(KSettingsGeometry);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
}

EditCapability::~EditCapability()
{
    QSettings settings;
    settings.setValue(KSettingsGeometry, saveGeometry());

    if (m_capability != NULL)
        delete m_capability;
}

void EditCapability::slotMinSpinChanged(int value)
{
    m_capability->setMin(value);
}

void EditCapability::slotMaxSpinChanged(int value)
{
    m_capability->setMax(value);
}

void EditCapability::slotDescriptionEdited(const QString& text)
{
    m_capability->setName(text);
}

void EditCapability::slotResourceButtonPressed()
{
    QFileDialog dialog(this);

    dialog.setWindowTitle(tr("Open Gobo File"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setDirectory(GOBODIR);

    dialog.setFilter(tr("Gobo pictures (*.jpg *.jpeg *.png *.bmp)"));

    /* Get file name */
    if (dialog.exec() != QDialog::Accepted)
        return;

    QString filename = dialog.selectedFiles().first();
    if (filename.isEmpty() == true)
        return;

    m_resourceButton->setIcon(QIcon(filename));
    m_capability->setResourceName(filename);
}

void EditCapability::slotColorButtonPressed()
{
    QColorDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QColor color = dialog.selectedColor();
    QPixmap pix(58, 58);
    pix.fill(color);
    m_resourceButton->setIcon(pix);
    m_capability->setResourceColor(color);
}

