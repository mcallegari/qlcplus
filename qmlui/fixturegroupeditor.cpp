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

#include "fixturegroupeditor.h"
#include "doc.h"

FixtureGroupEditor::FixtureGroupEditor(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
{
    Q_ASSERT(m_doc != NULL);

    connect(m_doc, SIGNAL(loaded()), this, SLOT(slotDocLoaded()));
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

QVariantList FixtureGroupEditor::groupSelection(int x, int y, int mouseMods)
{
    qDebug() << "Requested selection at" << x << y << "mods:" << mouseMods;
    if (m_editGroup == NULL)
        return m_groupSelection;

    int absIndex = (y * m_editGroup->size().width()) + x;

    if (m_groupSelection.contains(absIndex))
        return m_groupSelection;

    if (mouseMods == 0)
        m_groupSelection.clear();

    GroupHead head = m_editGroup->head(QLCPoint(x, y));
    if (head.isValid() == false)
        return m_groupSelection;

    Fixture *fixture = m_doc->fixture(head.fxi);
    if (fixture == NULL)
        return m_groupSelection;

    m_groupSelection.append(absIndex);

    qDebug() << "Selection size" << m_groupSelection.count();

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
}

void FixtureGroupEditor::updateGroupMap()
{
    /** Data format:
    * Fixture ID | headIndex | isOdd | fixture type (a lookup for icons)
    */

   m_groupMap.clear();

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
                m_groupMap.append(head.fxi);
                m_groupMap.append((gridWidth * y) + x);
                m_groupMap.append(0);
                m_groupMap.append(fx->type());
            }
       }
   }
   emit groupMapChanged();
}
