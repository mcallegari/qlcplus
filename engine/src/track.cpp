/*
  Q Light Controller
  track.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QDebug>

#include "track.h"
#include "scene.h"
#include "doc.h"

#define KXMLQLCTrackID        "ID"
#define KXMLQLCTrackName      "Name"
#define KXMLQLCTrackSceneID   "SceneID"
#define KXMLQLCTrackIsMute    "isMute"

#define KXMLQLCTrackSequences "Sequences"

Track::Track(quint32 sceneID)
    : m_id(Track::invalidId())
    , m_sceneID(sceneID)
    , m_isMute(false)

{
    setName(tr("New Track"));
}

Track::~Track()
{

}

/****************************************************************************
 * ID
 ****************************************************************************/

void Track::setId(quint32 id)
{
    m_id = id;
}

quint32 Track::id() const
{
    return m_id;
}

quint32 Track::invalidId()
{
    return UINT_MAX;
}

/****************************************************************************
 * Name
 ****************************************************************************/

void Track::setName(const QString& name)
{
    m_name = name;
    emit changed(this->id());
}

QString Track::name() const
{
    return m_name;
}

/*********************************************************************
 * Scene
 *********************************************************************/
quint32 Track::getSceneID()
{
    return m_sceneID;
}

/*********************************************************************
 * Mute state
 *********************************************************************/
void Track::setMute(bool state)
{
    m_isMute = state;
}

bool Track::isMute()
{
    return m_isMute;
}

/*********************************************************************
 * Sequences
 *********************************************************************/

bool Track::addSequenceID(quint32 id)
{
    if (m_sequences.count() > 0 && m_sequences.contains(id))
        return false;
    m_sequences.append(id);

    return true;
}

bool Track::removeSequenceID(quint32 id)
{
    if (m_sequences.count() > 0 && m_sequences.contains(id) == false)
        return false;
    int idx = m_sequences.indexOf(id);
    if (idx < 0)
        return false;
    m_sequences.takeAt(idx);

    return true;
}

QList <quint32> Track::sequencesID()
{
    return m_sequences;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/
bool Track::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement tag;
    QDomElement ids;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);

    /* Track entry */
    tag = doc->createElement(KXMLQLCTrack);
    tag.setAttribute(KXMLQLCTrackID, this->id());
    tag.setAttribute(KXMLQLCTrackName, this->name());
    if (m_sceneID != Scene::invalidId())
        tag.setAttribute(KXMLQLCTrackSceneID, m_sceneID);
    tag.setAttribute(KXMLQLCTrackIsMute, m_isMute);

    /* Save the list of Chasers IDs if present */
    if (m_sequences.count() > 0)
    {
        ids = doc->createElement(KXMLQLCTrackSequences);
        foreach(quint32 id, m_sequences)
        {
            if (str.isEmpty() == false)
                str.append(QString(","));
            str.append(QString("%1").arg(id));
        }
        text = doc->createTextNode(str);
        ids.appendChild(text);
        tag.appendChild(ids);
    }

    wksp_root->appendChild(tag);

    return true;
}

bool Track::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCTrack)
    {
        qWarning() << Q_FUNC_INFO << "Track node not found";
        return false;
    }

    bool ok = false;
    quint32 id = root.attribute(KXMLQLCTrackID).toUInt(&ok);
    if (ok == false)
    {
        qWarning() << "Invalid Track ID:" << root.attribute(KXMLQLCTrackID);
        return false;
    }
    // Assign the ID to myself
    m_id = id;

    if (root.hasAttribute(KXMLQLCTrackName) == true)
        m_name = root.attribute(KXMLQLCTrackName);

    ok = false;
    id = root.attribute(KXMLQLCTrackSceneID).toUInt(&ok);
    if (ok == false)
    {
        qWarning() << "Invalid Scene ID:" << root.attribute(KXMLQLCTrackSceneID);
        return false;
    }
    m_sceneID = id;

    ok = false;
    bool mute = root.attribute(KXMLQLCTrackIsMute).toInt(&ok);
    if (ok == false)
    {
        qWarning() << "Invalid Mute flag:" << root.attribute(KXMLQLCTrackIsMute);
        return false;
    }
    m_isMute = mute;

    /* look for chaser IDs */
    if (root.hasChildNodes())
    {
        QDomNode node = root.firstChild();
        if (node.isNull() == false)
        {
            QDomElement tag = node.toElement();
            if (tag.tagName() == KXMLQLCTrackSequences)
            {
                QString strvals = tag.text();
                if (strvals.isEmpty() == false)
                {
                    QStringList varray = strvals.split(",");
                    for (int i = 0; i < varray.count(); i++)
                        m_sequences.append(QString(varray.at(i)).toUInt());
                }
            }
        }
    }

    return true;
}
