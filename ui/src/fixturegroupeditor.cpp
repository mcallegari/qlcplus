/*
  Q Light Controller
  fixturegroupeditor.cpp

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

#include <QTableWidgetItem>
#include <QTableWidget>
#include <QSettings>
#include <QLineEdit>
#include <QSpinBox>
#include <QDebug>

#include "fixturegroupeditor.h"
#include "fixtureselection.h"
#include "fixturegroup.h"
#include "fixture.h"
#include "doc.h"

#define SETTINGS_GEOMETRY "fixturegroupeditor/geometry"

#define PROP_FIXTURE Qt::UserRole
#define PROP_HEAD Qt::UserRole + 1

FixtureGroupEditor::FixtureGroupEditor(FixtureGroup* grp, Doc* doc, QWidget* parent)
    : QWidget(parent)
    , m_grp(grp)
    , m_doc(doc)
{
    Q_ASSERT(grp != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    m_nameEdit->setText(m_grp->name());
    m_xSpin->setValue(m_grp->size().width());
    m_ySpin->setValue(m_grp->size().height());

    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_xSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotXSpinValueChanged(int)));
    connect(m_ySpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotYSpinValueChanged(int)));

    connect(m_rightButton, SIGNAL(clicked()),
            this, SLOT(slotRightClicked()));
    connect(m_leftButton, SIGNAL(clicked()),
            this, SLOT(slotLeftClicked()));
    connect(m_downButton, SIGNAL(clicked()),
            this, SLOT(slotDownClicked()));
    connect(m_upButton, SIGNAL(clicked()),
            this, SLOT(slotUpClicked())),
    connect(m_removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveFixtureClicked()));

    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setIconSize(QSize(20, 20));
    updateTable();
}

FixtureGroupEditor::~FixtureGroupEditor()
{
}

void FixtureGroupEditor::updateTable()
{
    qDebug() << Q_FUNC_INFO;
    // Store these since they might get reset
    int savedRow = m_row;
    int savedCol = m_column;

    disconnect(m_table, SIGNAL(cellChanged(int,int)),
               this, SLOT(slotCellChanged(int,int)));
    disconnect(m_table, SIGNAL(cellPressed(int,int)),
               this, SLOT(slotCellActivated(int,int)));
    disconnect(m_table->horizontalHeader(), SIGNAL(sectionResized(int,int,int)),
            this, SLOT(slotResized()));

    m_table->clear();

    m_table->setRowCount(m_grp->size().height());
    m_table->setColumnCount(m_grp->size().width());

    QMapIterator <QLCPoint,GroupHead> it(m_grp->headsMap());
    while (it.hasNext() == true)
    {
        it.next();

        QLCPoint pt(it.key());

        GroupHead head(it.value());
        Fixture* fxi = m_doc->fixture(head.fxi);
        if (fxi == NULL)
            continue;

        QIcon icon = fxi->getIconFromType();
        QString str = QString("%1 H:%2\nA:%3 U:%4").arg(fxi->name())
                                               .arg(head.head + 1)
                                               .arg(fxi->address() + 1)
                                               .arg(fxi->universe() + 1);

        QTableWidgetItem* item = new QTableWidgetItem(icon, str);
        item->setData(PROP_FIXTURE, head.fxi);
        item->setData(PROP_HEAD, head.head);
        item->setToolTip(str);

        m_table->setItem(pt.y(), pt.x(), item);
    }

    connect(m_table, SIGNAL(cellPressed(int,int)),
            this, SLOT(slotCellActivated(int,int)));
    connect(m_table, SIGNAL(cellChanged(int,int)),
            this, SLOT(slotCellChanged(int,int)));
    connect(m_table->horizontalHeader(), SIGNAL(sectionResized(int,int,int)),
            this, SLOT(slotResized()));

    if (savedRow < m_table->rowCount() && savedCol < m_table->columnCount())
    {
        m_row = savedRow;
        m_column = savedCol;
    }
    else
    {
        m_row = 0;
        m_column = 0;
    }

    m_table->setCurrentCell(m_row, m_column);
    slotResized();
}

void FixtureGroupEditor::slotNameEdited(const QString& text)
{
    m_grp->setName(text);
}

void FixtureGroupEditor::slotXSpinValueChanged(int value)
{
    m_grp->setSize(QSize(value, m_grp->size().height()));
    updateTable();
}

void FixtureGroupEditor::slotYSpinValueChanged(int value)
{
    m_grp->setSize(QSize(m_grp->size().width(), value));
    updateTable();
}

void FixtureGroupEditor::slotRightClicked()
{
    addFixtureHeads(Qt::RightArrow);
}

void FixtureGroupEditor::slotLeftClicked()
{
    addFixtureHeads(Qt::LeftArrow);
}

void FixtureGroupEditor::slotDownClicked()
{
    addFixtureHeads(Qt::DownArrow);
}

void FixtureGroupEditor::slotUpClicked()
{
    addFixtureHeads(Qt::UpArrow);
}


void FixtureGroupEditor::slotRemoveFixtureClicked()
{
    QTableWidgetItem* item = m_table->currentItem();
    if (item == NULL)
        return;

    if (m_grp->resignHead(QLCPoint(m_column, m_row)) == true)
        delete item;
}

void FixtureGroupEditor::slotCellActivated(int row, int column)
{
    m_row = row;
    m_column = column;

    if (m_table->currentItem() == NULL)
        m_removeButton->setEnabled(false);
    else
        m_removeButton->setEnabled(true);
}

void FixtureGroupEditor::slotCellChanged(int row, int column)
{
    if (row < 0 || column < 0)
    {
        updateTable();
        return;
    }

    QMap <QLCPoint,GroupHead> hash = m_grp->headsMap();
    QLCPoint from(m_column, m_row);
    QLCPoint to(column, row);
    GroupHead fromHead;
    GroupHead toHead;

    if (hash.contains(from) == true)
        fromHead = hash[from];
    if (hash.contains(to) == true)
        toHead = hash[to];

    m_grp->swap(from, to);

    updateTable();
    m_table->setCurrentCell(row, column);
    slotCellActivated(row, column);
}

void FixtureGroupEditor::slotResized()
{
    disconnect(m_table, SIGNAL(cellChanged(int,int)),
               this, SLOT(slotCellChanged(int,int)));

    float cellWidth = (float)(m_table->columnWidth(0) - m_table->iconSize().width());
    QFont font = m_table->font();
    QFontMetrics fm(font);
    float pSizeF = font.pointSizeF();

    for (int y = 0; y < m_table->rowCount(); y++)
    {
        for (int x = 0; x < m_table->columnCount(); x++)
        {
            QTableWidgetItem* item = m_table->item(y, x);
            if (item != NULL)
            {
                QFont scaledFont = font;
#if (QT_VERSION < QT_VERSION_CHECK(5, 11, 0))
                float baseWidth  = (float)fm.width(item->text());
#else
                float baseWidth  = (float)fm.horizontalAdvance(item->text());
#endif
                float factor = cellWidth / baseWidth;
                if (factor != 1)
                    scaledFont.setPointSizeF((pSizeF * factor) + 2);
                else
                    scaledFont.setPointSize(font.pointSize() - 2);

                item->setFont(scaledFont);
            }
        }
    }

    connect(m_table, SIGNAL(cellChanged(int,int)),
            this, SLOT(slotCellChanged(int,int)));
}

void FixtureGroupEditor::addFixtureHeads(Qt::ArrowType direction)
{
    FixtureSelection fs(this, m_doc);
    fs.setMultiSelection(true);
    fs.setSelectionMode(FixtureSelection::Heads);
    fs.setDisabledHeads(m_grp->headList());
    if (fs.exec() == QDialog::Accepted)
    {
        int row = m_row;
        int col = m_column;
        foreach (GroupHead gh, fs.selectedHeads())
        {
            m_grp->assignHead(QLCPoint(col, row), gh);
            if (direction == Qt::RightArrow)
                col++;
            else if (direction == Qt::DownArrow)
                row++;
            else if (direction == Qt::LeftArrow)
                col--;
            else if (direction == Qt::UpArrow)
                row--;
        }

        updateTable();
        m_table->setCurrentCell(row, col);
    }
}
