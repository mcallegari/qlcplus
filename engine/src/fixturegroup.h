/*
  Q Light Controller
  fixturegroup.h

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

#ifndef FIXTUREGROUP_H
#define FIXTUREGROUP_H

#include <QObject>
#include <QList>
#include <QSize>
#include <QHash>

#include "grouphead.h"
#include "qlcpoint.h"
#include "fixture.h"

#define KXMLQLCFixtureGroup "FixtureGroup"

class QDomDocument;
class QDomElement;
class Doc;

class FixtureGroup : public QObject
{
    Q_OBJECT

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
     */
    void assignFixture(quint32 id, const QLCPoint& pt = QLCPoint());

    /**
     * Assign a fixture head to a group at the given point. If point is null,
     * then the fixture will be automatically placed to the next free slot.
     * If the fixture head is already present in the group, it is moved from its
     * current position to the new position. If another fixture head occupies the
     * new point, the two fixture heads will simply switch places.
     *
     * @param pt The point to assign to
     * @param head The fixture head to assign
     */
    void assignHead(const QLCPoint& pt, const GroupHead& head);

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
    QHash <QLCPoint,GroupHead> headHash() const;

    /** Get a list of fixtures assigned to the group */
    QList <quint32> fixtureList() const;

private slots:
    /** Listens to Doc fixture removals */
    void slotFixtureRemoved(quint32 id);

private:
    QHash <QLCPoint,GroupHead> m_heads;

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
    static bool loader(const QDomElement& root, Doc* doc);
    bool loadXML(const QDomElement& root);
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);
};

#endif
