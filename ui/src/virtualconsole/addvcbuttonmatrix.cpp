/*
  Q Light Controller
  addvcbuttonmatrix.cpp

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

#include <QSettings>
#include <QDebug>
#include <QAction>

#include "addvcbuttonmatrix.h"
#include "functionselection.h"
#include "vcbutton.h"
#include "function.h"
#include "doc.h"

#define KColumnFunction 0
#define KColumnType     1

#define HORIZONTAL_COUNT "addvcbuttonmatrix/horizontalcount"
#define VERTICAL_COUNT "addvcbuttonmatrix/verticalcount"
#define BUTTON_SIZE "addvcbuttonmatrix/buttonsize"
#define FRAME_STYLE "addvcbuttonmatrix/framestyle"
#define SETTINGS_GEOMETRY "addvcbuttonmatrix/geometry"

AddVCButtonMatrix::AddVCButtonMatrix(QWidget* parent, Doc* doc)
    : QDialog(parent)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);

    QSettings settings;
    QVariant var;

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    var = settings.value(HORIZONTAL_COUNT);
    if (var.isValid() == true)
        m_horizontalSpin->setValue(var.toInt());
    else
        m_horizontalSpin->setValue(5);
    m_horizontalCount = m_horizontalSpin->value();

    var = settings.value(VERTICAL_COUNT);
    if (var.isValid() == true)
        m_verticalSpin->setValue(var.toInt());
    else
        m_verticalSpin->setValue(5);
    m_verticalCount = m_verticalSpin->value();

    var = settings.value(BUTTON_SIZE);
    if (var.isValid() == true)
        m_sizeSpin->setValue(var.toInt());
    else
        m_sizeSpin->setValue(VCButton::defaultSize.width());
    m_buttonSize = m_sizeSpin->value();

    var = settings.value(FRAME_STYLE);
    if (var.isValid() == true)
        setFrameStyle(AddVCButtonMatrix::FrameStyle(var.toInt()));
    else
        setFrameStyle(AddVCButtonMatrix::NormalFrame);

    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());

    setAllocationText();
}

AddVCButtonMatrix::~AddVCButtonMatrix()
{
    QSettings settings;
    settings.setValue(HORIZONTAL_COUNT, horizontalCount());
    settings.setValue(VERTICAL_COUNT, verticalCount());
    settings.setValue(BUTTON_SIZE, buttonSize());
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

QList <quint32> AddVCButtonMatrix::functions() const
{
    return m_functions;
}

quint32 AddVCButtonMatrix::horizontalCount() const
{
    return m_horizontalCount;
}

quint32 AddVCButtonMatrix::verticalCount() const
{
    return m_verticalCount;
}

quint32 AddVCButtonMatrix::buttonSize() const
{
    return m_buttonSize;
}

AddVCButtonMatrix::FrameStyle AddVCButtonMatrix::frameStyle() const
{
    return m_frameStyle;
}

void AddVCButtonMatrix::slotAddClicked()
{
    FunctionSelection fs(this, m_doc);
    fs.setDisabledFunctions(functions());
    if (fs.exec() == true)
    {
        QListIterator <quint32> it(fs.selection());
        while (it.hasNext() == true)
            addFunction(it.next());
    }

    setAllocationText();
}

void AddVCButtonMatrix::slotRemoveClicked()
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        m_functions.removeAll(item->data(KColumnFunction, Qt::UserRole).toUInt());
        delete item;
    }

    setAllocationText();
}

void AddVCButtonMatrix::slotHorizontalChanged()
{
    m_horizontalCount = m_horizontalSpin->value();
    setAllocationText();
}

void AddVCButtonMatrix::slotVerticalChanged()
{
    m_verticalCount = m_verticalSpin->value();
    setAllocationText();
}

void AddVCButtonMatrix::slotButtonSizeChanged()
{
    m_buttonSize = m_sizeSpin->value();
}

void AddVCButtonMatrix::slotNormalFrameToggled(bool toggled)
{
    if (toggled == true)
        setFrameStyle(AddVCButtonMatrix::NormalFrame);
    else
        setFrameStyle(AddVCButtonMatrix::SoloFrame);
}

void AddVCButtonMatrix::accept()
{
    QDialog::accept();
}

void AddVCButtonMatrix::addFunction(quint32 fid)
{
    Function* function = m_doc->function(fid);
    if (function == NULL)
        return;

    QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
    item->setText(KColumnFunction, function->name());
    item->setText(KColumnType, function->typeString());
    item->setData(KColumnFunction, Qt::UserRole, fid);

    m_functions << fid;
}

void AddVCButtonMatrix::setAllocationText()
{
    QString text("%1 / %2");
    m_allocationEdit->setText(text.arg(m_tree->topLevelItemCount())
                              .arg(horizontalCount() * verticalCount()));
}

void AddVCButtonMatrix::setFrameStyle(AddVCButtonMatrix::FrameStyle style)
{
    switch (style)
    {
    default:
    case NormalFrame:
        m_frameNormalRadio->setChecked(true);
        m_frameStyle = NormalFrame;
        break;
    case SoloFrame:
        m_frameSoloRadio->setChecked(true);
        m_frameStyle = SoloFrame;
        break;
    }
}
