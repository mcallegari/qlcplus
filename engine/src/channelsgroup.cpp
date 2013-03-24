/*
  Q Light Controller
  channelsgroup.cpp

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

#include "qlcfixturedef.h"
#include "channelsgroup.h"
#include "scenevalue.h"
#include "fixture.h"
#include "doc.h"

#define KXMLQLCChannelsGroupID    "ID"
#define KXMLQLCChannelsGroupName  "Name"
#define KXMLQLCChannelsGroupValue "Value"

#define KXMLQLCChannelsGroupInputUniverse "InputUniverse"
#define KXMLQLCChannelsGroupInputChannel  "InputChannel"


ChannelsGroup::ChannelsGroup(Doc* doc)
    : QObject(doc)
    , m_id(ChannelsGroup::invalidId())
    , m_masterValue(0)

{
    setName(tr("New Group"));
    m_doc = doc;
}

ChannelsGroup::ChannelsGroup(Doc* doc, const ChannelsGroup* chg)
    : QObject(doc)
    , m_doc(doc)
    , m_id(chg->id())
    , m_name(chg->name())
    , m_masterValue(0)
    , m_channels(chg->getChannels())
{

}

ChannelsGroup::~ChannelsGroup()
{
    m_channels.clear();
}

/****************************************************************************
 * ID
 ****************************************************************************/

void ChannelsGroup::setId(quint32 id)
{
    m_id = id;
}

quint32 ChannelsGroup::id() const
{
    return m_id;
}

quint32 ChannelsGroup::invalidId()
{
    return UINT_MAX;
}

/****************************************************************************
 * Name
 ****************************************************************************/

void ChannelsGroup::setName(const QString& name)
{
    m_name = name;
    emit changed(this->id());
}

QString ChannelsGroup::name() const
{
    return m_name;
}

/************************************************************************
 * Channels
 ************************************************************************/

void ChannelsGroup::resetList()
{
    m_channels.clear();
}

bool ChannelsGroup::addChannel(quint32 fxid, quint32 channel)
{
    if (fxid == invalidId())
        return false;

    m_channels.append(SceneValue(fxid, channel, 0));

    return true;
}


QList <SceneValue> ChannelsGroup::getChannels() const
{
    return m_channels;
}

/*********************************************************************
 * Status
 *********************************************************************/
QString ChannelsGroup::status(Doc *doc) const
{
    QString info;

    QString title("<TR><TD CLASS='hilite' COLSPAN='3'><CENTER>%1</CENTER></TD></TR>");
    info += "<TABLE COLS='3' WIDTH='100%'>";

    // Fixture title
    info += title.arg(name());

    /********************************************************************
     * Channels
     ********************************************************************/

    // Title row
    info += QString("<TR><TD CLASS='subhi'>%1</TD>").arg(tr("Fixture"));
    info += QString("<TD CLASS='subhi'>%1</TD>").arg(tr("Channel"));
    info += QString("<TD CLASS='subhi'>%1</TD></TR>").arg(tr("Description"));

    foreach (SceneValue value, m_channels)
    {
        Fixture *fixture = doc->fixture(value.fxi);
        if (fixture == NULL)
            return QString();
        const QLCFixtureDef *def = fixture->fixtureDef();
        QString chInfo("<TR><TD>%1</TD><TD>%2</TD><TD>%3</TD></TR>");
        if (def != NULL)
        {
            info += chInfo.arg(fixture->name()).arg(value.channel + 1)
                .arg(def->channels().at(value.channel)->name());
        }
        else
        {
            info += chInfo.arg(fixture->name()).arg(value.channel + 1)
                .arg(QString(tr("Channel %1")).arg(value.channel));
        }
    }

    // HTML document & table closure
    info += "</TABLE>";

    return info;
}

/*********************************************************************
 * External input
 *********************************************************************/
void ChannelsGroup::setInputSource(const QLCInputSource& source)
{
    m_input = source;
    // Connect when the first valid input source is set
    if (source.isValid() == true)
        connect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));
}

QLCInputSource ChannelsGroup::inputSource() const
{
    return m_input;
}

void ChannelsGroup::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    Q_UNUSED(value);

    /* Don't let input data thru in operate mode */
    if (m_doc->mode() == Doc::Operate)
        return;

    //qDebug() << Q_FUNC_INFO << "universe: " << universe << ", channel: " << channel << ", value: " << value;

    if (inputSource() == QLCInputSource(universe, channel))
    {
        emit valueChanged(channel, value);
    }
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/
bool ChannelsGroup::loader(const QDomElement& root, Doc* doc)
{
    bool result = false;

    ChannelsGroup* grp = new ChannelsGroup(doc);
    Q_ASSERT(grp != NULL);

    if (grp->loadXML(root) == true)
    {
        doc->addChannelsGroup(grp, grp->id());
        result = true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "ChannelsGroup" << grp->name() << "cannot be loaded.";
        delete grp;
        result = false;
    }

    return result;
}

bool ChannelsGroup::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);

    foreach(SceneValue value, this->getChannels())
    {
        if (str.isEmpty() == false)
            str.append(",");
        str.append(QString("%1,%2").arg(value.fxi).arg(value.channel));
    }

    /* Channels Group entry */
    tag = doc->createElement(KXMLQLCChannelsGroup);
    tag.setAttribute(KXMLQLCChannelsGroupID, this->id());
    tag.setAttribute(KXMLQLCChannelsGroupName, this->name());
    tag.setAttribute(KXMLQLCChannelsGroupValue, this->m_masterValue);
    if (m_input.isValid() == true)
    {
        tag.setAttribute(KXMLQLCChannelsGroupInputUniverse,QString("%1").arg(m_input.universe()));
        tag.setAttribute(KXMLQLCChannelsGroupInputChannel, QString("%1").arg(m_input.channel()));
    }
    if (str.isEmpty() == false)
    {
        text = doc->createTextNode(str);
        tag.appendChild(text);
    }

    wksp_root->appendChild(tag);

    return true;
}

bool ChannelsGroup::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCChannelsGroup)
    {
        qWarning() << Q_FUNC_INFO << "Channels group node not found";
        return false;
    }

    bool ok = false;
    quint32 id = root.attribute(KXMLQLCChannelsGroupID).toUInt(&ok);
    if (ok == false)
    {
        qWarning() << "Invalid ChannelsGroup ID:" << root.attribute(KXMLQLCChannelsGroupID);
        return false;
    }

    // Assign the ID to myself
    m_id = id;

    if (root.hasAttribute(KXMLQLCChannelsGroupName) == true)
        m_name = root.attribute(KXMLQLCChannelsGroupName);
    if (root.hasAttribute(KXMLQLCChannelsGroupValue) == true)
        m_masterValue = uchar(root.attribute(KXMLQLCChannelsGroupValue).toInt());

    QString chansValues = root.text();
    if (chansValues.isEmpty() == false)
    {
        QStringList varray = chansValues.split(",");
        for (int i = 0; i < varray.count(); i+=2)
        {
            m_channels.append(SceneValue(QString(varray.at(i)).toUInt(),
                                         QString(varray.at(i + 1)).toUInt(), 0));
        }
    }

    if (root.hasAttribute(KXMLQLCChannelsGroupInputUniverse) == true &&
        root.hasAttribute(KXMLQLCChannelsGroupInputChannel) == true)
    {
        quint32 uni = root.attribute(KXMLQLCChannelsGroupInputUniverse).toInt();
        quint32 ch = root.attribute(KXMLQLCChannelsGroupInputChannel).toInt();
        setInputSource(QLCInputSource(uni, ch));
    }

    return true;
}
