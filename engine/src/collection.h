/*
  Q Light Controller Plus
  collection.h

  Copyright (c) Heikki Junnila
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

#ifndef COLLECTION_H
#define COLLECTION_H

#include <QMutex>
#include <QList>
#include <QSet>

#include "function.h"

class QXmlStreamReader;

/** @addtogroup engine_functions Functions
 * @{
 */

typedef struct
{
    quint32 m_id;
    int m_intensityId;
} ChildFunction;

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

    /** @reimp */
    QIcon getIcon() const;

    /** @reimp */
    quint32 totalDuration();

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimp */
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
    bool addFunction(quint32 fid, int insertIndex = -1);

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

signals:
    void functionsChanged();

public slots:
    /** Catches Doc::functionRemoved() so that destroyed members can be
        removed immediately. */
    void slotFunctionRemoved(quint32 function);

protected:
    /** The list of Function IDs added to this Collection */
    QList <quint32> m_functions;
    /** A list of intesity attribute override IDs populated when this Collection is
     *  started and cleaned when it's stopped */
    QList <int> m_intensityOverrideIds;

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    mutable QMutex m_functionListMutex;
#else
    mutable QRecursiveMutex m_functionListMutex;
#endif

    /*********************************************************************
     * Save & Load
     *********************************************************************/
public:
    /** Save function's contents to an XML document */
    bool saveXML(QXmlStreamWriter *doc);

    /** Load function's contents from an XML document */
    bool loadXML(QXmlStreamReader &root);

    /** @reimp */
    void postLoad();

public:
    /** @reimp */
    bool contains(quint32 functionId);

    /** @reimp */
    QList<quint32> components();

    /*********************************************************************
     * Running
     *********************************************************************/
private:
    FunctionParent functionParent() const;

public:
    /** @reimpl */
    void preRun(MasterTimer* timer);

    /** @reimpl */
    void setPause(bool enable);

    /** @reimpl */
    void write(MasterTimer* timer, QList<Universe *> universes);

    /** @reimpl */
    void postRun(MasterTimer* timer, QList<Universe *> universes);

protected slots:
    /** Called whenever one of this function's child functions stops */
    void slotChildStopped(quint32 fid);

    /** Called whenever one of this function's child functions stops */
    void slotChildStarted(quint32 fid);

protected:
    /** Number of currently running children */
    QSet <quint32> m_runningChildren;
    unsigned int m_tick;

    /*************************************************************************
     * Intensity
     *************************************************************************/
public:
    /** @reimp */
    int adjustAttribute(qreal fraction, int attributeId);

    /*************************************************************************
     * Blend mode
     *************************************************************************/
public:
    /** @reimp */
    void setBlendMode(Universe::BlendMode mode);
};

/** @} */

#endif
