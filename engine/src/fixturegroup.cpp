/*
  Q Light Controller Plus
  fixturegroup.cpp

  Copyright (C) Heikki Junnila
                Massimo Callegari

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

#include "fixturegroup.h"
#include "qlcpoint.h"
#include "fixture.h"
#include "doc.h"

#define KXMLQLCFixtureGroupHead QString("Head")
#define KXMLQLCFixtureGroupSize QString("Size")
#define KXMLQLCFixtureGroupName QString("Name")

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
    m_heads = grp->headsMap();
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
    if (m_name == name)
        return;

    m_name = name;
    emit nameChanged();
    emit changed(this->id());
}

QString FixtureGroup::name() const
{
    return m_name;
}

/****************************************************************************
 * Fixtures
 ****************************************************************************/

bool FixtureGroup::assignFixture(quint32 id, const QLCPoint& pt)
{
    Fixture* fxi = doc()->fixture(id);
    Q_ASSERT(fxi != NULL);
    QLCPoint tmp = pt;
    int headAddedcount = 0;

    for (int i = 0; i < fxi->heads(); i++)
    {
        if (pt.isNull())
        {
            if (assignHead(pt, GroupHead(fxi->id(), i)) == true)
                headAddedcount++;
        }
        else
        {
            if (assignHead(tmp, GroupHead(fxi->id(), i)) == true)
                headAddedcount++;

            tmp.setX(tmp.x() + 1);
            if (tmp.x() >= size().width())
            {
                tmp.setX(0);
                tmp.setY(tmp.y() + 1);
            }
        }
    }

    return headAddedcount ? true : false;
}

bool FixtureGroup::assignHead(const QLCPoint& pt, const GroupHead& head)
{
    if (m_heads.values().contains(head) == true)
        return false;

    if (size().isValid() == false)
        setSize(QSize(1, 1));

    if (pt.isNull() == false)
    {
        m_heads[pt] = head;
    }
    else
    {
        int y = 0;
        int x = 0;
        int xmax = size().width();
        int ymax = size().height();

        while (1)
        {
            for (; y < ymax; y++)
            {
                for (x = 0; x < xmax; x++)
                {
                    QLCPoint tmp(x, y);
                    if (m_heads.contains(tmp) == false)
                    {
                        m_heads[tmp] = head;
                        emit changed(this->id());
                        return true;
                    }
                }
            }

            ymax++;
        }
    }

    emit changed(this->id());
    return true;
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

void FixtureGroup::reset()
{
    m_heads.clear();
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

QMap<QLCPoint, GroupHead> FixtureGroup::headsMap() const
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

bool FixtureGroup::loader(QXmlStreamReader &xmlDoc, Doc* doc)
{
    bool result = false;

    FixtureGroup* grp = new FixtureGroup(doc);
    Q_ASSERT(grp != NULL);

    if (grp->loadXML(xmlDoc) == true)
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

bool FixtureGroup::loadXML(QXmlStreamReader &xmlDoc)
{
    if (xmlDoc.name() != KXMLQLCFixtureGroup)
    {
        qWarning() << Q_FUNC_INFO << "Fixture group node not found";
        return false;
    }

    bool ok = false;
    quint32 id = xmlDoc.attributes().value(KXMLQLCFixtureGroupID).toString().toUInt(&ok);
    if (ok == false)
    {
        qWarning() << "Invalid FixtureGroup ID:" << xmlDoc.attributes().value(KXMLQLCFixtureGroupID).toString();
        return false;
    }

    // Assign the ID to myself
    m_id = id;

    while (xmlDoc.readNextStartElement())
    {
        QXmlStreamAttributes attrs = xmlDoc.attributes();
        if (xmlDoc.name() == KXMLQLCFixtureGroupHead)
        {
            bool xok = false, yok = false, idok = false, headok = false;
            int x = attrs.value("X").toString().toInt(&xok);
            int y = attrs.value("Y").toString().toInt(&yok);
            quint32 id = attrs.value("Fixture").toString().toUInt(&idok);
            int head = xmlDoc.readElementText().toInt(&headok);

            // Don't use assignFixture() here because it assigns complete fixtures at once
            if (xok == true && yok == true && idok == true && headok == true)
                m_heads[QLCPoint(x, y)] = GroupHead(id, head);
        }
        else if (xmlDoc.name() == KXMLQLCFixtureGroupSize)
        {
            bool xok = false, yok = false;
            int x = attrs.value("X").toString().toInt(&xok);
            int y = attrs.value("Y").toString().toInt(&yok);

            if (xok == true && yok == true)
                m_size = QSize(x, y);
            xmlDoc.skipCurrentElement();
        }
        else if (xmlDoc.name() == KXMLQLCFixtureGroupName)
        {
            m_name = xmlDoc.readElementText();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown fixture group tag:" << xmlDoc.name();
            xmlDoc.skipCurrentElement();
        }
    }

    return true;
}

bool FixtureGroup::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Fixture Group entry */
    doc->writeStartElement(KXMLQLCFixtureGroup);
    doc->writeAttribute(KXMLQLCFixtureGroupID, QString::number(this->id()));

    /* Name */
    doc->writeTextElement(KXMLQLCFixtureGroupName, name());

    /* Matrix size */
    doc->writeStartElement(KXMLQLCFixtureGroupSize);
    doc->writeAttribute("X", QString::number(size().width()));
    doc->writeAttribute("Y", QString::number(size().height()));
    doc->writeEndElement();

    /* Fixture heads */
    QList<QLCPoint> pointsList = m_heads.keys();

    foreach (QLCPoint pt, pointsList)
    {
        GroupHead head = m_heads[pt];
        doc->writeStartElement(KXMLQLCFixtureGroupHead);
        doc->writeAttribute("X", QString::number(pt.x()));
        doc->writeAttribute("Y", QString::number(pt.y()));
        doc->writeAttribute("Fixture", QString::number(head.fxi));
        doc->writeCharacters(QString::number(head.head));
        doc->writeEndElement();
    }

    doc->writeEndElement();

    return true;
}
