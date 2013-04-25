/*
  Q Light Controller
  collection.h

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

#ifndef COLLECTION_H
#define COLLECTION_H

#include <QMutex>
#include <QList>
#include <QSet>

#include "function.h"

class QDomDocument;

class Collection : public Function
{
    Q_OBJECT
    Q_DISABLE_COPY(Collection)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    Collection(Doc* doc);
    virtual ~Collection();

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimpl */
    Function* createCopy(Doc* doc, bool addToDoc = true);

    /** Copy the contents for this function from another function */
    bool copyFrom(const Function* function);

    /*********************************************************************
     * Contents
     *********************************************************************/
public:
    /**
     * Add a function to this collection. If the function is already a
     * member of the collection, this call fails.
     *
     * @param fid The function to add
     * @return true if successful, otherwise false
     */
    bool addFunction(quint32 fid);

    /**
     * Remove a function from this collection. If the function is not a
     * member of the collection, this call fails.
     *
     * @param fid The function to remove
     * @return true if successful, otherwise false
     */
    bool removeFunction(quint32 fid);

    /**
     * Get this function's list of member functions
     */
    QList <quint32> functions() const;

public slots:
    /** Catches Doc::functionRemoved() so that destroyed members can be
        removed immediately. */
    void slotFunctionRemoved(quint32 function);

protected:
    QList <quint32> m_functions;
    QMutex m_functionListMutex;

    /*********************************************************************
     * Save & Load
     *********************************************************************/
public:
    /** Save function's contents to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

    /** Load function's contents from an XML document */
    bool loadXML(const QDomElement& root);

    /** @reimp */
    void postLoad();

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    /** @reimpl */
    void preRun(MasterTimer* timer);

    /** @reimpl */
    void write(MasterTimer* timer, UniverseArray* universes);

    /** @reimpl */
    void postRun(MasterTimer* timer, UniverseArray* universes);

protected slots:
    /** Called whenever one of this function's child functions stops */
    void slotChildStopped(quint32 fid);

protected:
    /** Number of currently running children */
    QSet <quint32> m_runningChildren;
};

#endif
