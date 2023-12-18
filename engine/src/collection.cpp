/*
  Q Light Controller Plus
  collection.cpp

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

#include <QString>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QMutexLocker>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "mastertimer.h"
#include "collection.h"
#include "function.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Collection::Collection(Doc* doc)
    : Function(doc, Function::CollectionType)
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    , m_functionListMutex(QMutex::Recursive)
#endif
{
    setName(tr("New Collection"));

    // Listen to member Function removals
    connect(doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
}

Collection::~Collection()
{
}

QIcon Collection::getIcon() const
{
    return QIcon(":/collection.png");
}

quint32 Collection::totalDuration()
{
    quint32 totalDuration = 0;

    foreach (QVariant fid, functions())
    {
        Function* function = doc()->function(fid.toUInt());
        totalDuration += function->totalDuration();
    }

    return totalDuration;
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Collection::createCopy(Doc* doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Collection(doc);
    if (copy->copyFrom(this) == false)
    {
        delete copy;
        copy = NULL;
    }
    if (addToDoc == true && doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool Collection::copyFrom(const Function* function)
{
    const Collection* coll = qobject_cast<const Collection*> (function);
    if (coll == NULL)
        return false;

    m_functions.clear();
    m_functions = coll->m_functions;

    return Function::copyFrom(function);
}

/*****************************************************************************
 * Contents
 *****************************************************************************/

bool Collection::addFunction(quint32 fid, int insertIndex)
{
    if (fid != this->id() && m_functions.contains(fid) == false)
    {
        {
            QMutexLocker locker(&m_functionListMutex);
            if (insertIndex == -1)
                m_functions.append(fid);
            else
                m_functions.insert(insertIndex, fid);
        }

        emit changed(this->id());
        emit functionsChanged();
        return true;
    }
    else
    {
        return false;
    }
}

bool Collection::removeFunction(quint32 fid)
{
    int num = 0;
    {
        QMutexLocker locker(&m_functionListMutex);
        num = m_functions.removeAll(fid);
    }

    if (num > 0)
    {
        emit changed(this->id());
        emit functionsChanged();
        return true;
    }
    else
    {
        return false;
    }
}

QList<quint32> Collection::functions() const
{
    QMutexLocker locker(&m_functionListMutex);
    return m_functions;
}

void Collection::slotFunctionRemoved(quint32 fid)
{
    removeFunction(fid);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Collection::saveXML(QXmlStreamWriter *doc)
{
    int i = 0;

    Q_ASSERT(doc != NULL);

    /* Function tag */
    doc->writeStartElement(KXMLQLCFunction);

    /* Common attributes */
    saveXMLCommon(doc);

    /* Steps */
    QListIterator <quint32> it(m_functions);
    while (it.hasNext() == true)
    {
        /* Step tag */
        doc->writeStartElement(KXMLQLCFunctionStep);

        /* Step number */
        doc->writeAttribute(KXMLQLCFunctionNumber, QString::number(i++));

        /* Step Function ID */
        doc->writeCharacters(QString::number(it.next()));
        doc->writeEndElement();
    }

    /* End the <Function> tag */
    doc->writeEndElement();

    return true;
}

bool Collection::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attributes().value(KXMLQLCFunctionType).toString() != typeToString(Function::CollectionType))
    {
        qWarning() << Q_FUNC_INFO << root.attributes().value(KXMLQLCFunctionType).toString()
                   << "is not a collection";
        return false;
    }

    /* Load collection contents */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCFunctionStep)
            addFunction(root.readElementText().toUInt());
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown collection tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

void Collection::postLoad()
{
    Doc* doc = qobject_cast <Doc*> (parent());
    Q_ASSERT(doc != NULL);

    /* Check that all member functions exist (nonexistent functions can
       be present only when a corrupted file has been loaded) */
    QMutableListIterator<quint32> it(m_functions);
    while (it.hasNext() == true)
    {
        /* Remove any nonexistent member functions */
        Function* function = doc->function(it.next());

        if (function == NULL || function->contains(id())) // forbid self-containment
            it.remove();
    }
}

bool Collection::contains(quint32 functionId)
{
    Doc* doc = qobject_cast <Doc*> (parent());
    Q_ASSERT(doc != NULL);

    foreach (quint32 fid, m_functions)
    {
        Function* function = doc->function(fid);
        // contains() can be called during init, function may be NULL
        if (function == NULL)
            continue;

        if (function->id() == functionId)
            return true;
        if (function->contains(functionId))
            return true;
    }

    return false;
}

QList<quint32> Collection::components()
{
    return m_functions;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

FunctionParent Collection::functionParent() const
{
    return FunctionParent(FunctionParent::Function, id());
}

void Collection::preRun(MasterTimer *timer)
{
    Doc *doc = this->doc();
    Q_ASSERT(doc != NULL);
    {
        QMutexLocker locker(&m_functionListMutex);
        m_runningChildren.clear();
        foreach (quint32 fid, m_functions)
        {
            Function *function = doc->function(fid);
            Q_ASSERT(function != NULL);

            m_intensityOverrideIds << function->requestAttributeOverride(Function::Intensity, getAttributeValue(Function::Intensity));

            // Append the IDs of all functions started by this collection
            // to a set so that we can track which of them are still controlled
            // by this collection which are not.
            m_runningChildren << function->id();

            // Listen to the children's stopped signals so that this Collection
            // can give up its rights to stop the function later.
            connect(function, SIGNAL(stopped(quint32)),
                    this, SLOT(slotChildStopped(quint32)));

            // Listen to the children's stopped signals so that this collection
            // can give up its rights to stop the function later.
            connect(function, SIGNAL(running(quint32)),
                    this, SLOT(slotChildStarted(quint32)));

            //function->adjustAttribute(getAttributeValue(Function::Intensity), Function::Intensity);
            function->start(timer, functionParent(), 0, overrideFadeInSpeed(), overrideFadeOutSpeed(), overrideDuration());
        }
        m_tick = 1;
    }
    Function::preRun(timer);
}

void Collection::setPause(bool enable)
{
    Doc *doc = this->doc();
    Q_ASSERT(doc != NULL);
    foreach (quint32 fid, m_runningChildren)
    {
        Function *function = doc->function(fid);
        Q_ASSERT(function != NULL);
        function->setPause(enable);
    }
    Function::setPause(enable);
}

void Collection::write(MasterTimer *timer, QList<Universe *> universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);

    if (isPaused())
        return;

    // During first tick, children may be stopped & started.
    if (m_tick == 1)
        m_tick = 2;
    else if (m_tick == 2)
    {
        m_tick = 0;
        Doc *doc = this->doc();
        Q_ASSERT(doc != NULL);

        QMutexLocker locker(&m_functionListMutex);
        foreach (quint32 fid, m_runningChildren)
        {
            Function *function = doc->function(fid);
            Q_ASSERT(function != NULL);

            // First tick may correspond to this collection starting the function
            // Now that first tick is over, stop listening to running signal
            disconnect(function, SIGNAL(running(quint32)),
                    this, SLOT(slotChildStarted(quint32)));
        }
    }

    incrementElapsed();

    {
        QMutexLocker locker(&m_functionListMutex);
        if (m_runningChildren.size() > 0)
            return;
    }

    stop(functionParent());
}

void Collection::postRun(MasterTimer* timer, QList<Universe *> universes)
{
    Doc* doc = qobject_cast <Doc*> (parent());
    Q_ASSERT(doc != NULL);

    {
        QMutexLocker locker(&m_functionListMutex);
        /** Stop the member functions only if they have been started by this
            collection. */
        QSetIterator <quint32> it(m_runningChildren);
        while (it.hasNext() == true)
        {
            Function* function = doc->function(it.next());
            Q_ASSERT(function != NULL);
            function->stop(functionParent());
        }

        m_runningChildren.clear();

        for (int i = 0; i < m_functions.count(); i++)
        {
            Function* function = doc->function(m_functions.at(i));
            Q_ASSERT(function != NULL);

            disconnect(function, SIGNAL(stopped(quint32)),
                    this, SLOT(slotChildStopped(quint32)));
            if (m_tick == 2)
            {
                disconnect(function, SIGNAL(running(quint32)),
                        this, SLOT(slotChildStarted(quint32)));
            }
        }

        m_intensityOverrideIds.clear();
    }

    Function::postRun(timer, universes);
}

void Collection::slotChildStopped(quint32 fid)
{
    QMutexLocker locker(&m_functionListMutex);
    m_runningChildren.remove(fid);
}

void Collection::slotChildStarted(quint32 fid)
{
    QMutexLocker locker(&m_functionListMutex);
    m_runningChildren << fid;
}

int Collection::adjustAttribute(qreal fraction, int attributeId)
{
    int attrIndex = Function::adjustAttribute(fraction, attributeId);

    if (isRunning() && attrIndex == Intensity)
    {
        Doc* document = doc();
        Q_ASSERT(document != NULL);

        QMutexLocker locker(&m_functionListMutex);

        for (int i = 0; i < m_functions.count(); i++)
        {
            Function* function = document->function(m_functions.at(i));
            Q_ASSERT(function != NULL);
            function->adjustAttribute(getAttributeValue(Function::Intensity), m_intensityOverrideIds.at(i));
        }
    }

    return attrIndex;
}

void Collection::setBlendMode(Universe::BlendMode mode)
{
    if (mode == blendMode())
        return;

    qDebug() << "Collection" << name() << "blend mode set to" << Universe::blendModeToString(mode);

    if (isRunning())
    {
        for (int i = 0; i < m_functions.count(); i++)
        {
            Function* function = doc()->function(m_functions.at(i));
            Q_ASSERT(function != NULL);
            function->setBlendMode(mode);
        }
    }

    Function::setBlendMode(mode);
}
