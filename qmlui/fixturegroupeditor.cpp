/*
  Q Light Controller Plus
  fixturegroupeditor.cpp

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

#include <qmath.h>
#include <QImage>
#include <QDebug>
#include <QQmlContext>

#include "fixturegroupeditor.h"
#include "doc.h"

FixtureGroupEditor::FixtureGroupEditor(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
{
    Q_ASSERT(m_doc != NULL);

    m_view->rootContext()->setContextProperty("fixtureGroupEditor", this);
    qmlRegisterUncreatableType<FixtureGroupEditor>("org.qlcplus.classes", 1, 0,  "FixtureGroupEditor", "Can't create a FixtureGroupEditor !");

    connect(m_doc, SIGNAL(loaded()), this, SLOT(slotDocLoaded()));
}

FixtureGroupEditor::~FixtureGroupEditor()
{
    m_view->rootContext()->setContextProperty("fixtureGroupEditor", NULL);
}

QVariant FixtureGroupEditor::groupsListModel()
{
    QVariantList groupsList;

    foreach(FixtureGroup *grp, m_doc->fixtureGroups())
    {
        QVariantMap grpMap;
        grpMap.insert("mIcon", "qrc:/group.svg");
        grpMap.insert("mLabel", grp->name());
        grpMap.insert("mValue", grp->id());
        groupsList.append(grpMap);
    }

    return QVariant::fromValue(groupsList);
}

void FixtureGroupEditor::resetGroup()
{
    if (m_editGroup == NULL)
        return;

    m_editGroup->reset();
    updateGroupMap();
}

void FixtureGroupEditor::slotDocLoaded()
{
    emit groupsListModelChanged();
}

/*********************************************************************
 * Fixture Group Grid Editing
 *********************************************************************/

void FixtureGroupEditor::setEditGroup(QVariant reference)
{
    if (reference.canConvert<FixtureGroup *>() == false)
        return;

    m_editGroup = reference.value<FixtureGroup *>();

    emit groupNameChanged();
    emit groupSizeChanged();
    updateGroupMap();
}

QString FixtureGroupEditor::groupName() const
{
    return m_editGroup == NULL ? "" : m_editGroup->name();
}

void FixtureGroupEditor::setGroupName(QString name)
{
    if (m_editGroup == NULL || m_editGroup->name() == name)
        return;

    m_editGroup->setName(name);

    emit groupNameChanged();
}

QSize FixtureGroupEditor::groupSize() const
{
    if (m_editGroup == NULL)
        return QSize();

    return m_editGroup->size();
}

void FixtureGroupEditor::setGroupSize(QSize size)
{
    if (m_editGroup == NULL || size == m_editGroup->size())
        return;

    m_editGroup->setSize(size);
    emit groupSizeChanged();
    updateGroupMap();
}

QVariantList FixtureGroupEditor::groupMap()
{
    return m_groupMap;
}

QVariantList FixtureGroupEditor::groupLabels()
{
    return m_groupLabels;
}

QVariantList FixtureGroupEditor::selectionData()
{
    return m_groupSelection;
}

void FixtureGroupEditor::resetSelection()
{
    m_groupSelection.clear();
}

QVariantList FixtureGroupEditor::groupSelection(int x, int y, int mouseMods)
{
    qDebug() << "Requested selection at" << x << y << "mods:" << mouseMods;
    if (m_editGroup == NULL)
        return m_groupSelection;

    int absIndex = (y * m_editGroup->size().width()) + x;

    if (m_groupSelection.contains(absIndex))
        return m_groupSelection;

    //if (mouseMods == 0)
    //    m_groupSelection.clear();

    GroupHead head = m_editGroup->head(QLCPoint(x, y));
    if (head.isValid() == false)
    {
        m_groupSelection.clear();
        return m_groupSelection;
    }

    Fixture *fixture = m_doc->fixture(head.fxi);
    if (fixture == NULL)
        return m_groupSelection;

    m_groupSelection.append(absIndex);

    std::sort(m_groupSelection.begin(), m_groupSelection.end());

    qDebug() << "Selection size" << m_groupSelection.count() << m_groupSelection;

    return m_groupSelection;
}

QVariantList FixtureGroupEditor::dragSelection(QVariant reference, int x, int y, int mouseMods)
{
    if (m_editGroup == NULL)
        return m_groupSelection;

    if (mouseMods == 0)
        m_groupSelection.clear();

    int absIndex = (y * m_editGroup->size().width()) + x;

    if (reference.canConvert<Fixture *>())
    {
        Fixture *fixture = reference.value<Fixture *>();

        for (int headIdx = 0; headIdx < fixture->heads(); headIdx++)
            m_groupSelection.append(absIndex + headIdx);
    }

    return m_groupSelection;
}

void FixtureGroupEditor::addFixture(QVariant reference, int x, int y)
{
    if (m_editGroup == NULL)
        return;

    qDebug() << Q_FUNC_INFO << reference << x << y;

    if (reference.canConvert<Fixture *>())
    {
        Fixture *fixture = reference.value<Fixture *>();
        m_editGroup->assignFixture(fixture->id(), QLCPoint(x, y));
        updateGroupMap();
    }
}

bool FixtureGroupEditor::checkSelection(int x, int y, int offset)
{
    Q_UNUSED(x)
    Q_UNUSED(y)

    if (m_editGroup == NULL)
        return false;

    // search for heads already occupying the target positions
    for (int i = 0; i < m_groupSelection.count(); i++)
    {
        int targetPos = m_groupSelection.at(i).toInt() + offset;
        if (m_groupSelection.contains(targetPos))
            continue;

        int yPos = qFloor(targetPos / m_editGroup->size().width());
        int xPos = targetPos - (yPos * m_editGroup->size().width());

        GroupHead head = m_editGroup->head(QLCPoint(xPos, yPos));
        if (head.isValid())
            return false;
    }

    return true;
}

void FixtureGroupEditor::moveSelection(int x, int y, int offset)
{
    Q_UNUSED(x)
    Q_UNUSED(y)

    if (m_editGroup == NULL)
        return;

    if (checkSelection(x, y, offset) == false)
        return;

    for (int i = 0; i < m_groupSelection.count(); i++)
    {
        int origPos = m_groupSelection.at(i).toInt();
        int origYPos = qFloor(origPos / m_editGroup->size().width());
        int origXPos = origPos - (origYPos * m_editGroup->size().width());
        int targetPos = origPos + offset;
        int targetYPos = qFloor(targetPos / m_editGroup->size().width());
        int targetXPos = targetPos - (targetYPos * m_editGroup->size().width());

        qDebug() << "Moving head from" << origXPos << origYPos << "to" << targetXPos << targetYPos;

        m_editGroup->swap(QLCPoint(origXPos, origYPos), QLCPoint(targetXPos, targetYPos));
    }
    updateGroupMap();

    for (int i = 0; i < m_groupSelection.count(); i++)
        m_groupSelection.replace(i, m_groupSelection.at(i).toInt() + offset);
}

void FixtureGroupEditor::transformSelection(int transformation)
{
    if (m_editGroup == NULL)
        return;

    int minX = m_editGroup->size().width();
    int minY = m_editGroup->size().height();
    int maxX = 0;
    int maxY = 0;
    QList<QPoint> pointsList;
    QList<GroupHead> headsList;

    /** If the selection list is empty, it means the operation
     *  has to be performed on the whole group, so create
     *  a selection with everything in it */
    if (m_groupSelection.isEmpty())
    {
        for (int y = 0; y < m_editGroup->size().height(); y++)
        {
            for (int x = 0; x < m_editGroup->size().width(); x++)
            {
                int absIndex = (y * m_editGroup->size().width()) + x;
                GroupHead head = m_editGroup->head(QLCPoint(x, y));
                if (head.isValid())
                    m_groupSelection.append(absIndex);
            }
        }
    }

    /** From the current selection:
     *  - create a list of the original head points
     *  - create a list of the original GroupHeads
     *  - determine the rectangular size of the selection
     *  - remove the original heads
     */
    for (QVariant headOffset : m_groupSelection)
    {
        int yPos = qFloor(headOffset.toInt() / m_editGroup->size().width());
        int xPos = headOffset.toInt() - (yPos * m_editGroup->size().width());

        if (yPos < minY) minY = yPos;
        if (yPos > maxY) maxY = yPos;
        if (xPos < minX) minX = xPos;
        if (xPos > maxX) maxX = xPos;
        pointsList.append(QPoint(xPos, yPos));
        headsList.append(m_editGroup->head(QLCPoint(xPos, yPos)));

        // WARNING: point of no return !
        m_editGroup->resignHead(QLCPoint(xPos, yPos));
    }

    /** Here's the trick. Instead of dragging in a lot of code to perform
     *  transformations, let's leverage the QImage/QTransform ready-made code.
     *  Here a QImage is filled with "pixels" (at the scaled position)
     *  whose color is actually the head position in the points list
     *  created above */
    QImage matrix(maxX - minX + 1, maxY - minY + 1, QImage::Format_RGB32);
    matrix.fill(Qt::black);
    qDebug() << "Original matrix size is" << matrix.size();

    for (int i = 0; i < pointsList.count(); i++)
    {
        QPoint point = pointsList.at(i);
        matrix.setPixel(point.x() - minX, point.y() - minY, QRgb(i + 1));
        //qDebug() << "set pixel" << (point.x() - minX) << (point.y() - minY) << m_groupSelection.at(i).toUInt();
    }

    /** Perform the requested transformation ! */
    QTransform transform;
    QImage trImage;

    switch(TransformType(transformation))
    {
        case Rotate90:
            transform = transform.rotate(90);
            trImage = matrix.transformed(transform);
        break;
        case Rotate180:
            transform = transform.rotate(180);
            trImage = matrix.transformed(transform);
        break;
        case Rotate270:
            transform = transform.rotate(270);
            trImage = matrix.transformed(transform);
        break;
        case HorizontalFlip:
            trImage = matrix.mirrored(true, false);
        break;
        case VerticalFlip:
            trImage = matrix.mirrored(false, true);
        break;
    }

    /** Now assign to the group the original heads but on the new
     *  positions. Also, restore the original selection with
     *  the transformed head positions */
    m_groupSelection.clear();

    for (int y = 0; y < trImage.height(); y++)
    {
        for (int x = 0; x < trImage.width(); x++)
        {
            unsigned int pixel = 0x00FFFFFF & (unsigned int)trImage.pixel(x, y);
            //qDebug() << x << y << "pixel:" << QString::number(pixel);

            if (pixel == 0)
                continue;

            m_editGroup->assignHead(QLCPoint(x + minX, y + minY), headsList.at(pixel - 1));
            int absIndex = ((y + minY) * m_editGroup->size().width()) + (x + minX);
            m_groupSelection.append(absIndex);
        }
    }

    /** Finally, inform the UI that the map has changed */
    updateGroupMap();
}

void FixtureGroupEditor::updateGroupMap()
{
    /** Data format:
    * Fixture ID | headIndex | isOdd | fixture type (a lookup for icons)
    */

   m_groupMap.clear();
   m_groupLabels.clear();

   if (m_editGroup == NULL)
       return;

   int gridWidth = m_editGroup->size().width();

   for (int y = 0; y < m_editGroup->size().height(); y++)
   {
       for (int x = 0; x < gridWidth; x++)
       {
            GroupHead head = m_editGroup->head(QLCPoint(x, y));
            if (head.isValid())
            {
                Fixture *fx = m_doc->fixture(head.fxi);
                m_groupMap.append(head.fxi); // item ID
                m_groupMap.append((gridWidth * y) + x); // absolute index
                m_groupMap.append(0); // isOdd
                m_groupMap.append(fx->type()); // item type

                QString str = QString("%1\nH:%2 A:%3 U:%4").arg(fx->name())
                                                       .arg(head.head + 1)
                                                       .arg(fx->address() + 1)
                                                       .arg(fx->universe() + 1);
                m_groupLabels.append(head.fxi); // item ID
                m_groupLabels.append((gridWidth * y) + x); // absolute index
                m_groupLabels.append(1); // width
                m_groupLabels.append(str); // label
            }
       }
   }
   emit groupMapChanged();
   emit groupLabelsChanged();
}
