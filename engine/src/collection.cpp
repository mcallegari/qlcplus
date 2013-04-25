/*
  Q Light Controller
  collection.cpp

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

#include <QString>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QtXml>

#include "qlcfile.h"

#include "universearray.h"
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
    m_functionListMutex.lock();
    m_functions.clear();
    m_functionListMutex.unlock();
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
        m_functionListMutex.lock();
        m_functions.append(fid);
        m_functionListMutex.unlock();

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
    m_functionListMutex.lock();
    int num = m_functions.removeAll(fid);
    m_functionListMutex.unlock();

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

    root.setAttribute(KXMLQLCFunctionID, id());
    root.setAttribute(KXMLQLCFunctionType, Function::typeToString(type()));
    root.setAttribute(KXMLQLCFunctionName, name());

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
    m_runningChildren.clear();
    Function::preRun(timer);
}

void Collection::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(universes);

    if (elapsed() == 0)
    {
        Doc* doc = qobject_cast <Doc*> (parent());
        Q_ASSERT(doc != NULL);

        m_functionListMutex.lock();
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

            function->start(timer, true, overrideFadeInSpeed(), overrideFadeOutSpeed(), overrideDuration());
        }
        m_functionListMutex.unlock();
    }

    incrementElapsed();

    if (m_runningChildren.size() == 0)
        stop();
}

void Collection::postRun(MasterTimer* timer, UniverseArray* universes)
{
    Doc* doc = qobject_cast <Doc*> (parent());
    Q_ASSERT(doc != NULL);

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
    Function::postRun(timer, universes);
}

void Collection::slotChildStopped(quint32 fid)
{
    Doc* doc = qobject_cast <Doc*> (parent());
    Q_ASSERT(doc != NULL);

    Function* function = doc->function(fid);
    disconnect(function, SIGNAL(stopped(quint32)),
               this, SLOT(slotChildStopped(quint32)));

    m_runningChildren.remove(fid);
}
