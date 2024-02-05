/*
  Q Light Controller Plus
  fixturegroup.h

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

#ifndef FIXTUREGROUP_H
#define FIXTUREGROUP_H

#include <QObject>
#include <QList>
#include <QSize>
#include <QMap>

#include "grouphead.h"
#include "qlcpoint.h"
#include "fixture.h"

class QXmlStreamReader;
class Doc;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCFixtureGroup     QString("FixtureGroup")
#define KXMLQLCFixtureGroupID   QString("ID")

class FixtureGroup : public QObject
{
    Q_OBJECT

    Q_PROPERTY(quint32 id READ id CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    FixtureGroup(Doc* parent);
    ~FixtureGroup();

    /** Copy properties from $grp to this */
    void copyFrom(const FixtureGroup* grp);

signals:
    /** Emitted whenever a fixture group's properties are changed */
    void changed(quint32 id);

private:
    Doc* doc() const;

    /************************************************************************
     * ID
     ************************************************************************/
public:
    /** Set a fixture group's id (only Doc is allowed to do this!) */
    void setId(quint32 id);

    /** Get a fixture group's unique id */
    quint32 id() const;

    /** Get an invalid fixture group id */
    static quint32 invalidId();

private:
    quint32 m_id;

    /************************************************************************
     * Name
     ************************************************************************/
public:
    /** Set the name of a fixture group */
    void setName(const QString& name);

    /** Get the name of a fixture group */
    QString name() const;

signals:
    void nameChanged();

private:
    QString m_name;

    /************************************************************************
     * Fixtures
     ************************************************************************/
public:
    /**
     * Assign the given fixture $id to a FixtureGroup by placing all of its
     * heads in consecutive order, starting from point $pt.
     *
     * @param id The ID of the fixture to add
     * @param point The point to start from
     *
     * @return true if at least one fixture head was added, otherwise false
     */
    bool assignFixture(quint32 id, const QLCPoint& pt = QLCPoint());

    /**
     * Assign a fixture head to a group at the given point. If point is null,
     * then the fixture will be automatically placed to the next free slot.
     * If the fixture head is already present in the group, it is moved from its
     * current position to the new position. If another fixture head occupies the
     * new point, the two fixture heads will simply switch places.
     *
     * @param pt The point to assign to
     * @param head The fixture head to assign
     *
     * @return true if head was added, otherwise false
     */
    bool assignHead(const QLCPoint& pt, const GroupHead& head);

    /**
     * Resign a fixture, along with all of its heads from a group.
     *
     * @param id Fixture ID to remove
     */
    void resignFixture(quint32 id);

    /**
     * Remove the head assignment at the given point. If the point doesn't
     * exist in the group, nothing is done and this returns false.
     *
     * @param pt The point to clear
     * @return true if successful (cleared), otherwise false (no such point)
     */
    bool resignHead(const QLCPoint& pt);

    /**
     * Switch places with fixture heads at two points a and b.
     *
     * @param a First point
     * @param b Second point
     */
    void swap(const QLCPoint& a, const QLCPoint& b);

    /** Reset the whole group but preserve its size */
    void reset();

    /**
     * Get a fixture head by its position in the group. If nothing has been assigned
     * at the given point, returns an invalid GroupHead.
     *
     * @param pt Get the fixture head at this point
     * @return The fixture head at the given point or an invalid head
     */
    GroupHead head(const QLCPoint& pt) const;

    /** Get a list of fixtures assigned to a group */
    QList <GroupHead> headList() const;

    /** Get the fixture head hash */
    QMap <QLCPoint,GroupHead> headsMap() const;

    /** Get a list of fixture IDs assigned to the group */
    QList <quint32> fixtureList() const;

private slots:
    /** Listens to Doc fixture removals */
    void slotFixtureRemoved(quint32 id);

private:
    QMap <QLCPoint,GroupHead> m_heads;

    /************************************************************************
     * Size
     ************************************************************************/
public:
    /** Set the group's matrix size */
    void setSize(const QSize& sz);

    /** Get the group's matrix size */
    QSize size() const;

private:
    QSize m_size;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    static bool loader(QXmlStreamReader &xmlDoc, Doc* doc);
    bool loadXML(QXmlStreamReader &xmlDoc);
    bool saveXML(QXmlStreamWriter *doc);
};

/** @} */

#endif
