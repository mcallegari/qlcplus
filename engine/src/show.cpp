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

#include "showrunner.h"
#include "function.h"
#include "chaser.h"
#include "show.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Show::Show(Doc* doc) : Function(doc, Function::Show)
  , m_latestTrackId(0)
  , m_runner(NULL)
{
    setName(tr("New Show"));

    // Listen to member Function removals
    connect(doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
}

Show::~Show()
{
    m_tracks.clear();
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
    const Show* show = qobject_cast<const Show*> (function);
    if (show == NULL)
        return false;

    // create a copy of each track
    foreach(Track *track, show->tracks())
    {
        quint32 sceneID = track->getSceneID();
        Track* newTrack = new Track(sceneID);
        newTrack->setName(track->name());
        addTrack(newTrack);

        // create a copy of each sequence/audio in a track
        foreach(quint32 fid, track->functionsID())
        {
            Function* function = doc()->function(fid);
            if (function == NULL)
                continue;

            /* Attempt to create a copy of the function to Doc */
            Function* copy = function->createCopy(doc());
            if (copy != NULL)
            {
                copy->setName(tr("Copy of %1").arg(function->name()));
                newTrack->addFunctionID(copy->id());
            }
        }
    }

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

int Show::getTracksCount()
{
    return m_tracks.size();
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
    Function::preRun(timer);
    m_runningChildren.clear();
    if (m_runner != NULL)
    {
        m_runner->stop();
        delete m_runner;
    }

    m_runner = new ShowRunner(doc(), this->id());
    connect(m_runner, SIGNAL(timeChanged(quint32)), this, SIGNAL(timeChanged(quint32)));
    connect(m_runner, SIGNAL(showFinished()), this, SIGNAL(showFinished()));
    m_runner->start();
}

void Show::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(universes);
    Q_UNUSED(timer);
    m_runner->write();
}

void Show::postRun(MasterTimer* timer, UniverseArray* universes)
{
    if (m_runner != NULL)
    {
        m_runner->stop();
        delete m_runner;
        m_runner = NULL;
    }
    Function::postRun(timer, universes);
}

void Show::slotChildStopped(quint32 fid)
{
    Q_UNUSED(fid);
}
