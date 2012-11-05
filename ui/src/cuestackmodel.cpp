/*
  Q Light Controller
  cuestackmodel.cpp

  Copyright (c) Heikki Junnila

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

#include <QApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QMimeData>
#include <QPalette>
#include <QBrush>
#include <QDebug>
#include <QIcon>

#include "cuestackmodel.h"
#include "cuestack.h"
#include "function.h"

#define MIMEDATA_ROOT       "MimeData"
#define MIMEDATA_DRAGINDEX  "DragIndex"

CueStackModel::CueStackModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_cueStack(NULL)
{
}

CueStackModel::~CueStackModel()
{
}

void CueStackModel::setCueStack(CueStack* cs)
{
    qDebug() << Q_FUNC_INFO << "old:" << (void*)m_cueStack << "new:" << (void*) cs;

    if (m_cueStack != NULL)
    {
        // Don't attempt to remove anything if there's nothing to remove
        int last = m_cueStack->cues().size() - 1;
        if (last >= 0)
            beginRemoveRows(QModelIndex(), 0, last);

        disconnect(m_cueStack, SIGNAL(added(int)), this, SLOT(slotAdded(int)));
        disconnect(m_cueStack, SIGNAL(removed(int)), this, SLOT(slotRemoved(int)));
        disconnect(m_cueStack, SIGNAL(changed(int)), this, SLOT(slotChanged(int)));
        disconnect(m_cueStack, SIGNAL(currentCueChanged(int)), this, SLOT(slotCurrentCueChanged(int)));
        m_cueStack = NULL;

        if (last >= 0)
            endRemoveRows();
    }

    if (cs != NULL)
    {
        // Don't attempt to insert anything if there's nothing to insert
        if (cs->cues().size() > 0)
            beginInsertRows(QModelIndex(), 0, cs->cues().size() - 1);
        m_cueStack = cs;
        connect(m_cueStack, SIGNAL(added(int)), this, SLOT(slotAdded(int)));
        connect(m_cueStack, SIGNAL(removed(int)), this, SLOT(slotRemoved(int)));
        connect(m_cueStack, SIGNAL(changed(int)), this, SLOT(slotChanged(int)));
        connect(m_cueStack, SIGNAL(currentCueChanged(int)), this, SLOT(slotCurrentCueChanged(int)));
        if (cs->cues().size() > 0)
            endInsertRows();
    }
}

CueStack* CueStackModel::cueStack() const
{
    return m_cueStack;
}

/****************************************************************************
 * CueStack slots
 ****************************************************************************/

void CueStackModel::slotAdded(int index)
{
    Q_ASSERT(m_cueStack != NULL);
    beginInsertRows(QModelIndex(), index, index);
    endInsertRows();
}

void CueStackModel::slotRemoved(int index)
{
    Q_ASSERT(m_cueStack != NULL);
    beginRemoveRows(QModelIndex(), index, index);
    endRemoveRows();
}

void CueStackModel::slotChanged(int index)
{
    Q_ASSERT(m_cueStack != NULL);
    emit dataChanged(createIndex(index, 0, 0), createIndex(index, 1, 0));
}

void CueStackModel::slotCurrentCueChanged(int index)
{
    emit dataChanged(createIndex(index, 0, 0), createIndex(index, 1, 0));
}

/****************************************************************************
 * QAbstractItemModel
 ****************************************************************************/

int CueStackModel::columnCount(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return CueStackModel::ColumnCount;
}

QVariant CueStackModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch(section)
    {
    case IndexColumn:
        return tr("Number");
    case FadeInColumn:
        return tr("Fade In");
    case FadeOutColumn:
        return tr("Fade Out");
    case DurationColumn:
        return tr("Duration");
    case NameColumn:
        return tr("Cue");
    default:
        return QVariant();
    }
}

QModelIndex CueStackModel::index(int row, int column, const QModelIndex& parent) const
{
    if (m_cueStack == NULL || parent.isValid() == true) // No parents
        return QModelIndex();
    else
        return createIndex(row, column, 0);
}

QModelIndex CueStackModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int CueStackModel::rowCount(const QModelIndex& parent) const
{
    if (m_cueStack == NULL || parent.isValid() == true) // No parents
        return 0;
    else
        return m_cueStack->cues().size();
}

QVariant CueStackModel::data(const QModelIndex& index, int role) const
{
    if (m_cueStack == NULL)
        return QVariant();

    QVariant var;
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole)
    {
        switch (index.column())
        {
        case IndexColumn:
            var = QVariant(index.row() + 1);
            break;
        case NameColumn:
            var = QVariant(m_cueStack->cues()[index.row()].name());
            break;
        case FadeInColumn:
        {
            uint ms = m_cueStack->cues()[index.row()].fadeInSpeed();
            if (ms > 0)
                var = QVariant(Function::speedToString(ms));
            else
                var = QVariant();
            break;
        }
        case FadeOutColumn:
        {
            uint ms = m_cueStack->cues()[index.row()].fadeOutSpeed();
            if (ms > 0)
                var = QVariant(Function::speedToString(ms));
            else
                var = QVariant();
            break;
        }
        case DurationColumn:
        {
            uint ms = m_cueStack->cues()[index.row()].duration();
            if (ms > 0)
                var = QVariant(Function::speedToString(ms));
            else
                var = QVariant();
            break;
        }
        default:
            var = QVariant();
            break;
        }
    }
    else if (role == Qt::DecorationRole)
    {
        if (m_cueStack->currentIndex() == index.row() && index.column() == 0)
            var = QVariant(QIcon(":/current.png"));
    }

    return var;
}

QStringList CueStackModel::mimeTypes () const
{
    return QStringList() << QString("text/plain");
}

Qt::DropActions CueStackModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

Qt::ItemFlags CueStackModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (index.isValid() == true)
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

bool CueStackModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
                                 int column, const QModelIndex& parent)
{
    qDebug() << Q_FUNC_INFO;

    Q_UNUSED(row);
    Q_UNUSED(column);

    if (m_cueStack == NULL || action != Qt::MoveAction)
        return false;

    if (data->hasText() == true)
    {
        QDomDocument doc;
        if (doc.setContent(data->text()) == true && doc.firstChild().isNull() == false)
        {
            QDomElement root = doc.firstChild().toElement();
            if (root.tagName() != MIMEDATA_ROOT)
            {
                qWarning() << Q_FUNC_INFO << "Invalid MIME data";
                return false;
            }

            // Dig the drag index from the XML
            int dragIndex = root.attribute(MIMEDATA_DRAGINDEX).toInt();
            int index = parent.row();
            if (dragIndex < index)
                index += 1; // Moving items from above drop index

            // Insert each dropped Cue as a new Cue since the originals are in fact
            // removed in removeRows().
            QDomNode node = root.firstChild();
            while (node.isNull() == false)
            {
                Cue cue;
                if (cue.loadXML(node.toElement()) == true)
                {
                    m_cueStack->insertCue(index, cue);
                    index++; // Shift insertion point forwards
                }

                node = node.nextSibling();
            }
        }

        return true;
    }
    else
    {
        return false;
    }
}

QMimeData* CueStackModel::mimeData(const QModelIndexList& indexes) const
{
    qDebug() << Q_FUNC_INFO << indexes;

    if (m_cueStack == NULL || indexes.size() == 0)
        return NULL;

    // MIME data is essentially a bunch of XML "Cue" entries (plus drag index)
    QDomDocument doc;
    QDomElement root = doc.createElement(MIMEDATA_ROOT);
    root.setAttribute(MIMEDATA_DRAGINDEX, indexes.first().row());
    doc.appendChild(root);

    QSet <int> rows;
    foreach (QModelIndex index, indexes)
    {
        // $indexes contains all rows' columns but we want to store one row only once.
        // So, discard $index if it appears more than once.
        if (rows.contains(index.row()) == true)
            continue;
        else if (index.row() >= 0 && index.row() < m_cueStack->cues().size())
            m_cueStack->cues().at(index.row()).saveXML(&doc, &root);
        rows << index.row();
    }

    QMimeData* data = new QMimeData;
    data->setText(doc.toString());
    return data;
}

bool CueStackModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (m_cueStack == NULL || parent.isValid() == true)
        return false;

    for (int i = 0; i < count; i++)
        m_cueStack->removeCue(row);

    return true;
}
