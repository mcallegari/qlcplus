/*
  Q Light Controller
  show.cpp

  Copyright (c) Massimo Callegari

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

#include "function.h"
#include "show.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Show::Show(Doc* doc) : Function(doc, Function::Show)
  , m_latestTrackId(0)
{
    setName(tr("New Show"));

    // Listen to member Function removals
    connect(doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
}

Show::~Show()
{

}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Show::createCopy(Doc* doc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Show(doc);
    if (copy->copyFrom(this) == false || doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool Show::copyFrom(const Function* function)
{
    const Show* coll = qobject_cast<const Show*> (function);
    if (coll == NULL)
        return false;

    return Function::copyFrom(function);
}

/*****************************************************************************
 * Tracks
 *****************************************************************************/

bool Show::addTrack(Track *track, quint32 id)
{
    Q_ASSERT(track != NULL);

    // No ID given, this method can assign one
    if (id == Track::invalidId())
        id = createTrackId();

     track->setId(id);
     m_tracks[id] = track;

     return true;
}

bool Show::removeTrack(quint32 id)
{
    if (m_tracks.contains(id) == true)
    {
        Track* trk = m_tracks.take(id);
        Q_ASSERT(trk != NULL);

        //emit trackRemoved(id);
        delete trk;

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "No channels group with id" << id;
        return false;
    }
}

Track* Show::track(quint32 id) const
{
    if (m_tracks.contains(id) == true)
        return m_tracks[id];
    else
        return NULL;
}

Track* Show::getTrackFromSceneID(quint32 id)
{
    foreach(Track *track, m_tracks)
    {
        if (track->getSceneID() == id)
            return track;
    }
    return NULL;
}

QList <Track*> Show::tracks() const
{
    return m_tracks.values();
}

void Show::slotFunctionRemoved(quint32 fid)
{
    removeTrack(fid);
}

quint32 Show::createTrackId()
{
    while (m_tracks.contains(m_latestTrackId) == true ||
           m_latestTrackId == Track::invalidId())
    {
        m_latestTrackId++;
    }

    return m_latestTrackId;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Show::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
    /*QDomText text;
    QString str;
    int i = 0;*/

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Function tag */
    root = doc->createElement(KXMLQLCFunction);
    wksp_root->appendChild(root);

    root.setAttribute(KXMLQLCFunctionID, id());
    root.setAttribute(KXMLQLCFunctionType, Function::typeToString(type()));
    root.setAttribute(KXMLQLCFunctionName, name());

    foreach(Track *track, m_tracks)
        track->saveXML(doc, &root);

    return true;
}

bool Show::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attribute(KXMLQLCFunctionType) != typeToString(Function::Show))
    {
        qWarning() << Q_FUNC_INFO << root.attribute(KXMLQLCFunctionType)
                   << "is not a show";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCTrack)
        {
            Track *trk = new Track();
            if (trk->loadXML(tag) == true)
                addTrack(trk, trk->id());
        }
        node = node.nextSibling();
    }

    return true;
}

void Show::postLoad()
{

}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Show::preRun(MasterTimer* timer)
{
    m_runningChildren.clear();
    Function::preRun(timer);
}

void Show::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(universes);
    Q_UNUSED(timer);
}

void Show::postRun(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(universes);
    Q_UNUSED(timer);
}

void Show::slotChildStopped(quint32 fid)
{
    Q_UNUSED(fid);
}
