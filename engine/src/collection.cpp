/*
  Q Light Controller
  collection.cpp

  Copyright (c) Heikki Junnila

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
#include <QtXml>
#include <QMutexLocker>

#include "qlcfile.h"

#include "mastertimer.h"
#include "collection.h"
#include "function.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Collection::Collection(Doc* doc) : Function(doc, Function::Collection)
{
    setName(tr("New Collection"));

    // Listen to member Function removals
    connect(doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
}

Collection::~Collection()
{
    QMutexLocker locker(&m_functionListMutex);
    m_functions.clear();
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

bool Collection::addFunction(quint32 fid)
{
    if (fid != this->id() && m_functions.contains(fid) == false)
    {
        {
            QMutexLocker locker(&m_functionListMutex);
            m_functions.append(fid);
        }

        emit changed(this->id());
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
        return true;
    }
    else
    {
        return false;
    }
}

QList <quint32> Collection::functions() const
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

bool Collection::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;
    int i = 0;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Function tag */
    root = doc->createElement(KXMLQLCFunction);
    wksp_root->appendChild(root);

    /* Common attributes */
    saveXMLCommon(&root);

    /* Steps */
    QListIterator <quint32> it(m_functions);
    while (it.hasNext() == true)
    {
        /* Step tag */
        tag = doc->createElement(KXMLQLCFunctionStep);
        root.appendChild(tag);

        /* Step number */
        tag.setAttribute(KXMLQLCFunctionNumber, i++);

        /* Step Function ID */
        str.setNum(it.next());
        text = doc->createTextNode(str);
        tag.appendChild(text);
    }

    return true;
}

bool Collection::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attribute(KXMLQLCFunctionType) != typeToString(Function::Collection))
    {
        qWarning() << Q_FUNC_INFO << root.attribute(KXMLQLCFunctionType)
                   << "is not a collection";
        return false;
    }

    /* Load collection contents */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();

        if (tag.tagName() == KXMLQLCFunctionStep)
            addFunction(tag.text().toInt());
        else
            qWarning() << Q_FUNC_INFO << "Unknown collection tag:" << tag.tagName();

        node = node.nextSibling();
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
        if (doc->function(it.next()) == NULL)
            it.remove();
    }
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Collection::preRun(MasterTimer* timer)
{
    {
        QMutexLocker locker(&m_functionListMutex);
        m_runningChildren.clear();
    }
    Function::preRun(timer);
}

void Collection::write(MasterTimer* timer, QList<Universe *> universes)
{
    Q_UNUSED(universes);

    if (elapsed() == 0)
    {
        Doc* doc = qobject_cast <Doc*> (parent());
        Q_ASSERT(doc != NULL);

        QMutexLocker locker(&m_functionListMutex);
        QListIterator <quint32> it(m_functions);
        while (it.hasNext() == true)
        {
            Function* function = doc->function(it.next());
            Q_ASSERT(function != NULL);

            // Append the IDs of all functions started by this collection
            // to a set so that we can track which of them are still controlled
            // by this collection which are not.
            m_runningChildren << function->id();

            // Listen to the children's stopped signals so that this collection
            // can give up its rights to stop the function later.
            connect(function, SIGNAL(stopped(quint32)),
                    this, SLOT(slotChildStopped(quint32)));

            function->start(timer, true, 0, overrideFadeInSpeed(), overrideFadeOutSpeed(), overrideDuration());
        }
    }

    incrementElapsed();

    {
        QMutexLocker locker(&m_functionListMutex);
        if (m_runningChildren.size() > 0)
          return;
    }

    stop();
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
            function->stop();
        }

        m_runningChildren.clear();
    }

    Function::postRun(timer, universes);
}

void Collection::slotChildStopped(quint32 fid)
{
    Doc* doc = qobject_cast <Doc*> (parent());
    Q_ASSERT(doc != NULL);

    Function* function = doc->function(fid);
    disconnect(function, SIGNAL(stopped(quint32)),
               this, SLOT(slotChildStopped(quint32)));

    QMutexLocker locker(&m_functionListMutex);
    m_runningChildren.remove(fid);
}
