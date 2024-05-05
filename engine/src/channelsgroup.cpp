/*
  Q Light Controller
  channelsgroup.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

#include "qlcfixturemode.h"
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

    init();
}

ChannelsGroup::ChannelsGroup(Doc* doc, const ChannelsGroup* chg)
    : QObject(doc)
    , m_doc(doc)
    , m_id(chg->id())
    , m_name(chg->name())
    , m_masterValue(0)
    , m_channels(chg->getChannels())
    , m_input(chg->inputSource())
{
    init();
}

ChannelsGroup::~ChannelsGroup()
{
}

void ChannelsGroup::init()
{
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)),
            this, SLOT(slotFixtureRemoved(quint32)));
}

void ChannelsGroup::slotFixtureRemoved(quint32 fixtureId)
{
    bool hasChanged = false;

    QMutableListIterator<SceneValue> channelsIt(m_channels);
    while (channelsIt.hasNext())
    {
        SceneValue scv(channelsIt.next());
        if (scv.fxi == fixtureId)
        {
            channelsIt.remove();
            hasChanged = true;
        }
    }

    if (hasChanged)
        emit changed(this->id());
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

void ChannelsGroup::resetChannels()
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
        const QLCFixtureMode *mode = fixture->fixtureMode();
        QString chInfo("<TR><TD>%1</TD><TD>%2</TD><TD>%3</TD></TR>");
        if (mode != NULL)
        {
            info += chInfo.arg(fixture->name()).arg(value.channel + 1)
                .arg(mode->channels().at(value.channel)->name());
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
void ChannelsGroup::setInputSource(QSharedPointer<QLCInputSource> const& source)
{
    if (!m_input.isNull() && m_input->isValid())
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));

    m_input = source;

    // Connect when the first valid input source is set
    if (!source.isNull() && source->isValid())
        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));
}

QSharedPointer<QLCInputSource> const& ChannelsGroup::inputSource() const
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

    if (inputSource() != NULL &&
        inputSource()->universe() == universe &&
        inputSource()->channel() == channel)
    {
        emit valueChanged(channel, value);
    }
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/
bool ChannelsGroup::loader(QXmlStreamReader &xmlDoc, Doc* doc)
{
    bool result = false;

    ChannelsGroup* grp = new ChannelsGroup(doc);
    Q_ASSERT(grp != NULL);

    if (grp->loadXML(xmlDoc) == true)
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

bool ChannelsGroup::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    QString str;
    foreach (SceneValue value, this->getChannels())
    {
        if (str.isEmpty() == false)
            str.append(",");
        str.append(QString("%1,%2").arg(value.fxi).arg(value.channel));
    }

    /* Channels Group entry */
    doc->writeStartElement(KXMLQLCChannelsGroup);
    doc->writeAttribute(KXMLQLCChannelsGroupID, QString::number(this->id()));
    doc->writeAttribute(KXMLQLCChannelsGroupName, this->name());
    doc->writeAttribute(KXMLQLCChannelsGroupValue, QString::number(m_masterValue));

    if (!m_input.isNull() && m_input->isValid())
    {
        doc->writeAttribute(KXMLQLCChannelsGroupInputUniverse,QString("%1").arg(m_input->universe()));
        doc->writeAttribute(KXMLQLCChannelsGroupInputChannel, QString("%1").arg(m_input->channel()));
    }
    if (str.isEmpty() == false)
        doc->writeCharacters(str);

    doc->writeEndElement();

    return true;
}

bool ChannelsGroup::loadXML(QXmlStreamReader &xmlDoc)
{
    if (xmlDoc.name() != KXMLQLCChannelsGroup)
    {
        qWarning() << Q_FUNC_INFO << "Channels group node not found";
        return false;
    }

    QXmlStreamAttributes attrs = xmlDoc.attributes();

    bool ok = false;
    quint32 id = attrs.value(KXMLQLCChannelsGroupID).toString().toUInt(&ok);
    if (ok == false)
    {
        qWarning() << "Invalid ChannelsGroup ID:" << attrs.value(KXMLQLCChannelsGroupID).toString();
        return false;
    }

    // Assign the ID to myself
    m_id = id;

    if (attrs.hasAttribute(KXMLQLCChannelsGroupName) == true)
        m_name = attrs.value(KXMLQLCChannelsGroupName).toString();
    if (attrs.hasAttribute(KXMLQLCChannelsGroupValue) == true)
        m_masterValue = uchar(attrs.value(KXMLQLCChannelsGroupValue).toString().toInt());

    QString chansValues = xmlDoc.readElementText();
    if (chansValues.isEmpty() == false)
    {
        QStringList varray = chansValues.split(",");
        for (int i = 0; i < varray.count(); i+=2)
        {
            SceneValue scv(QString(varray.at(i)).toUInt(),
                           QString(varray.at(i + 1)).toUInt(), 0);
            Fixture* fxi = m_doc->fixture(scv.fxi);
            if (fxi == NULL)
            {
                qWarning() << Q_FUNC_INFO << "Fixture not present:" << scv.fxi;
                continue;
            }
            const QLCChannel* ch = fxi->channel(scv.channel);
            if (ch == NULL)
            {
                qWarning() << Q_FUNC_INFO << "Fixture" << scv.fxi << "does not have channel" << scv.channel;
                continue;
            }
            m_channels.append(scv);
        }
    }

    if (attrs.hasAttribute(KXMLQLCChannelsGroupInputUniverse) == true &&
        attrs.hasAttribute(KXMLQLCChannelsGroupInputChannel) == true)
    {
        quint32 uni = attrs.value(KXMLQLCChannelsGroupInputUniverse).toString().toInt();
        quint32 ch = attrs.value(KXMLQLCChannelsGroupInputChannel).toString().toInt();
        setInputSource(QSharedPointer<QLCInputSource>(new QLCInputSource(uni, ch)));
    }

    return true;
}
