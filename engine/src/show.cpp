/*
  Q Light Controller
  show.cpp

  Copyright (c) Massimo Callegari

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

#include "qlcfile.h"
#include "qlcmacros.h"

#include "showrunner.h"
#include "function.h"
#include "chaser.h"
#include "show.h"
#include "doc.h"

#define KXMLQLCShowTimeDivision "TimeDivision"
#define KXMLQLCShowTimeType "Type"
#define KXMLQLCShowTimeBPM "BPM"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Show::Show(Doc* doc) : Function(doc, Function::Show)
  , m_timeDivType(QString("Time"))
  , m_timeDivBPM(120)
  , m_latestTrackId(0)
  , m_runner(NULL)
{
    setName(tr("New Show"));

    // Clear attributes here. I want attributes to be mapped
    // exactly like the Show tracks
    unregisterAttribute(tr("Intensity"));
}

Show::~Show()
{
    m_tracks.clear();
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Show::createCopy(Doc* doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Show(doc);
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

bool Show::copyFrom(const Function* function)
{
    const Show* show = qobject_cast<const Show*> (function);
    if (show == NULL)
        return false;

    m_timeDivType = show->m_timeDivType;
    m_timeDivBPM = show->m_timeDivBPM;
    m_latestTrackId = show->m_latestTrackId;

    // create a copy of each track
    foreach(Track *track, show->tracks())
    {
        quint32 sceneID = track->getSceneID();
        Track* newTrack = new Track(sceneID);
        newTrack->setName(track->name());
        addTrack(newTrack);

        // create a copy of each sequence/audio in a track
        foreach(ShowFunction *sfunc, track->showFunctions())
        {
            Function* function = doc()->function(sfunc->functionID());
            if (function == NULL)
                continue;

            /* Attempt to create a copy of the function to Doc */
            Function* copy = function->createCopy(doc());
            if (copy != NULL)
            {
                copy->setName(tr("Copy of %1").arg(function->name()));
                newTrack->createShowFunction(copy->id());
            }
        }
    }

    return Function::copyFrom(function);
}

/*********************************************************************
 * Time division
 *********************************************************************/

void Show::setTimeDivision(QString type, int BPM)
{
    qDebug() << "[setTimeDivision] type:" << type << ", BPM:" << BPM;
    m_timeDivType = type;
    m_timeDivBPM = BPM;
}

QString Show::getTimeDivisionType()
{
    return m_timeDivType;
}

int Show::getTimeDivisionBPM()
{
    return m_timeDivBPM;
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

     registerAttribute(track->name());

     return true;
}

bool Show::removeTrack(quint32 id)
{
    if (m_tracks.contains(id) == true)
    {
        Track* trk = m_tracks.take(id);
        Q_ASSERT(trk != NULL);

        unregisterAttribute(trk->name());

        //emit trackRemoved(id);
        delete trk;

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "No track found with id" << id;
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

void Show::moveTrack(Track *track, int direction)
{
    if (track == NULL)
        return;

    qint32 trkID = track->id();
    if (trkID == 0 && direction == -1)
        return;
    qint32 maxID = -1;
    Track *swapTrack = NULL;
    qint32 swapID = -1;
    if (direction > 0) swapID = INT_MAX;

    foreach(quint32 id, m_tracks.keys())
    {
        qint32 signedID = (qint32)id;
        if (signedID > maxID) maxID = signedID;
        if (direction == -1 && signedID > swapID && signedID < trkID)
            swapID = signedID;
        else if (direction == 1 && signedID < swapID && signedID > trkID)
            swapID = signedID;
    }

    qDebug() << Q_FUNC_INFO << "Direction:" << direction << ", trackID:" << trkID << ", swapID:" << swapID;
    if (swapID == trkID || (direction > 0 && trkID == maxID))
        return;

    swapTrack = m_tracks[swapID];
    m_tracks[swapID] = track;
    m_tracks[trkID] = swapTrack;
    track->setId(swapID);
    swapTrack->setId(trkID);
}

QList <Track*> Show::tracks() const
{
    return m_tracks.values();
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

    /* Common attributes */
    saveXMLCommon(&root);

    QDomElement td = doc->createElement(KXMLQLCShowTimeDivision);
    td.setAttribute(KXMLQLCShowTimeType, m_timeDivType);
    td.setAttribute(KXMLQLCShowTimeBPM, m_timeDivBPM);
    root.appendChild(td);

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
        if (tag.tagName() == KXMLQLCShowTimeDivision)
        {
            QString type = tag.attribute(KXMLQLCShowTimeType);
            int bpm = (tag.attribute(KXMLQLCShowTimeBPM)).toInt();
            setTimeDivision(type, bpm);
        }
        else if (tag.tagName() == KXMLQLCTrack)
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

    m_runner = new ShowRunner(doc(), this->id(), elapsed());
    int i = 0;
    foreach(Track *track, m_tracks.values())
        m_runner->adjustIntensity(getAttributeValue(i++), track);

    connect(m_runner, SIGNAL(timeChanged(quint32)), this, SIGNAL(timeChanged(quint32)));
    connect(m_runner, SIGNAL(showFinished()), this, SIGNAL(showFinished()));
    m_runner->start();
}

void Show::write(MasterTimer* timer, QList<Universe *> universes)
{
    Q_UNUSED(universes);
    Q_UNUSED(timer);
    m_runner->write();
}

void Show::postRun(MasterTimer* timer, QList<Universe *> universes)
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

/*****************************************************************************
 * Attributes
 *****************************************************************************/

void Show::adjustAttribute(qreal fraction, int attributeIndex)
{
    Function::adjustAttribute(fraction, attributeIndex);

    if (m_runner != NULL)
    {
        QList<Track*> trkList = m_tracks.values();
        if (trkList.isEmpty() == false &&
            attributeIndex >= 0 && attributeIndex < trkList.count())
        {
            Track *track = trkList.at(attributeIndex);
            if (track != NULL)
                m_runner->adjustIntensity(fraction, track);
        }
    }
}

