/*
  Q Light Controller
  fixturegroup.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomText>
#include <QDebug>

#include "fixturegroup.h"
#include "qlcpoint.h"
#include "fixture.h"
#include "doc.h"

#define KXMLQLCFixtureGroupID "ID"
#define KXMLQLCFixtureGroupHead "Head"
#define KXMLQLCFixtureGroupSize "Size"
#define KXMLQLCFixtureGroupName "Name"

/****************************************************************************
 * Initialization
 ****************************************************************************/

FixtureGroup::FixtureGroup(Doc* parent)
    : QObject(parent)
    , m_id(FixtureGroup::invalidId())
{
    Q_ASSERT(parent != NULL);

    // Listen to fixture removals
    connect(parent, SIGNAL(fixtureRemoved(quint32)),
            this, SLOT(slotFixtureRemoved(quint32)));
}

FixtureGroup::~FixtureGroup()
{
}

void FixtureGroup::copyFrom(const FixtureGroup* grp)
{
    // Don't copy ID
    m_name = grp->name();
    m_size = grp->size();
    m_heads = grp->headHash();
}

Doc* FixtureGroup::doc() const
{
    return qobject_cast<Doc*> (parent());
}

/****************************************************************************
 * ID
 ****************************************************************************/

void FixtureGroup::setId(quint32 id)
{
    m_id = id;
}

quint32 FixtureGroup::id() const
{
    return m_id;
}

quint32 FixtureGroup::invalidId()
{
    return UINT_MAX;
}

/****************************************************************************
 * Name
 ****************************************************************************/

void FixtureGroup::setName(const QString& name)
{
    m_name = name;
    emit changed(this->id());
}

QString FixtureGroup::name() const
{
    return m_name;
}

/****************************************************************************
 * Fixtures
 ****************************************************************************/

void FixtureGroup::assignFixture(quint32 id, const QLCPoint& pt)
{
    Fixture* fxi = doc()->fixture(id);
    Q_ASSERT(fxi != NULL);
    for (int i = 0; i < fxi->heads(); i++)
        assignHead(pt, GroupHead(fxi->id(), i));
}

void FixtureGroup::assignHead(const QLCPoint& pt, const GroupHead& head)
{
    if (m_heads.values().contains(head) == true)
        return;

    if (size().isValid() == false)
        setSize(QSize(1, 1));

    if (pt.isNull() == false)
    {
        m_heads[pt] = head;
    }
    else
    {
        bool assigned = false;
        int y = 0;
        int x = 0;
        int xmax = size().width();
        int ymax = size().height();

        while (assigned == false)
        {
            for (; y < ymax; y++)
            {
                for (x = 0; x < xmax; x++)
                {
                    QLCPoint tmp(x, y);
                    if (m_heads.contains(tmp) == false)
                    {
                        m_heads[tmp] = head;
                        assigned = true;
                        break;
                    }
                }

                if (assigned == true)
                    break;
            }

            ymax++;
        }
    }

    emit changed(this->id());
}

void FixtureGroup::resignFixture(quint32 id)
{
    foreach (QLCPoint pt, m_heads.keys())
    {
        if (m_heads[pt].fxi == id)
            m_heads.remove(pt);
    }

    emit changed(this->id());
}

bool FixtureGroup::resignHead(const QLCPoint& pt)
{
    if (m_heads.contains(pt) == true)
    {
        m_heads.remove(pt);
        emit changed(this->id());
        return true;
    }
    else
    {
        return false;
    }
}

void FixtureGroup::swap(const QLCPoint& a, const QLCPoint& b)
{
    GroupHead ah = m_heads.value(a);
    GroupHead bh = m_heads.value(b);

    if (ah.isValid() == true)
        m_heads[b] = ah;
    else
        m_heads.remove(b);

    if (bh.isValid() == true)
        m_heads[a] = bh;
    else
        m_heads.remove(a);

    emit changed(this->id());
}

GroupHead FixtureGroup::head(const QLCPoint& pt) const
{
    return m_heads.value(pt);
}

QList <GroupHead> FixtureGroup::headList() const
{
    return m_heads.values();
}

QHash <QLCPoint,GroupHead> FixtureGroup::headHash() const
{
    return m_heads;
}

QList <quint32> FixtureGroup::fixtureList() const
{
    QList <quint32> list;
    foreach (GroupHead head, headList())
    {
        if (list.contains(head.fxi) == false)
            list << head.fxi;
    }
    return list;
}

void FixtureGroup::slotFixtureRemoved(quint32 id)
{
    // Remove the fixture from group records since it's no longer there
    resignFixture(id);
}

/****************************************************************************
 * Size
 ****************************************************************************/

void FixtureGroup::setSize(const QSize& sz)
{
    m_size = sz;
    emit changed(this->id());
}

QSize FixtureGroup::size() const
{
    return m_size;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool FixtureGroup::loader(const QDomElement& root, Doc* doc)
{
    bool result = false;

    FixtureGroup* grp = new FixtureGroup(doc);
    Q_ASSERT(grp != NULL);

    if (grp->loadXML(root) == true)
    {
        doc->addFixtureGroup(grp, grp->id());
        result = true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "FixtureGroup" << grp->name() << "cannot be loaded.";
        delete grp;
        result = false;
    }

    return result;
}

bool FixtureGroup::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFixtureGroup)
    {
        qWarning() << Q_FUNC_INFO << "Fixture group node not found";
        return false;
    }

    bool ok = false;
    quint32 id = root.attribute(KXMLQLCFixtureGroupID).toUInt(&ok);
    if (ok == false)
    {
        qWarning() << "Invalid FixtureGroup ID:" << root.attribute(KXMLQLCFixtureGroupID);
        return false;
    }

    // Assign the ID to myself
    m_id = id;

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCFixtureGroupHead)
        {
            bool xok = false, yok = false, idok = false, headok = false;
            int x = tag.attribute("X").toInt(&xok);
            int y = tag.attribute("Y").toInt(&yok);
            quint32 id = tag.attribute("Fixture").toUInt(&idok);
            int head = tag.text().toInt(&headok);

            // Don't use assignFixture() here because it assigns complete fixtures at once
            if (xok == true && yok == true && idok == true && headok == true)
                m_heads[QLCPoint(x, y)] = GroupHead(id, head);
        }
        else if (tag.tagName() == KXMLQLCFixtureGroupSize)
        {
            bool xok = false, yok = false;
            int x = tag.attribute("X").toInt(&xok);
            int y = tag.attribute("Y").toInt(&yok);

            if (xok == true && yok == true)
                m_size = QSize(x, y);
        }
        else if (tag.tagName() == KXMLQLCFixtureGroupName)
        {
            m_name = tag.text();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown fixture group tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool FixtureGroup::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);

    /* Fixture Group entry */
    root = doc->createElement(KXMLQLCFixtureGroup);
    root.setAttribute(KXMLQLCFixtureGroupID, this->id());
    wksp_root->appendChild(root);

    /* Name */
    tag = doc->createElement(KXMLQLCFixtureGroupName);
    text = doc->createTextNode(name());
    tag.appendChild(text);
    root.appendChild(tag);

    /* Matrix size */
    tag = doc->createElement(KXMLQLCFixtureGroupSize);
    tag.setAttribute("X", size().width());
    tag.setAttribute("Y", size().height());
    root.appendChild(tag);

    /* Fixture heads */
    QHashIterator <QLCPoint,GroupHead> it(m_heads);
    while (it.hasNext() == true)
    {
        it.next();
        tag = doc->createElement(KXMLQLCFixtureGroupHead);
        tag.setAttribute("X", it.key().x());
        tag.setAttribute("Y", it.key().y());
        tag.setAttribute("Fixture", it.value().fxi);
        text = doc->createTextNode(QString::number(it.value().head));
        tag.appendChild(text);
        root.appendChild(tag);
    }

    return true;
}
