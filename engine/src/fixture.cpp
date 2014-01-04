/*
  Q Light Controller
  fixture.cpp

  Copyright (C) Heikki Junnila

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
#include <QtXml>

#include "qlcfixturedefcache.h"
#include "qlcfixturemode.h"
#include "qlcfixturehead.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcchannel.h"

#include "fixture.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Fixture::Fixture(QObject* parent) : QObject(parent)
{
    m_id = Fixture::invalidId();

    m_address = 0;
    m_channels = 0;

    m_fixtureDef = NULL;
    m_fixtureMode = NULL;

    m_genericChannel = NULL;
    createGenericChannel();
}

Fixture::~Fixture()
{
    if (m_genericChannel != NULL)
        delete m_genericChannel;
    m_genericChannel = NULL;
}

bool Fixture::operator<(const Fixture& fxi)
{
    if (m_address < fxi.m_address)
        return true;
    else
        return false;
}

/*****************************************************************************
 * Fixture ID
 *****************************************************************************/

void Fixture::setID(quint32 id)
{
    m_id = id;
    emit changed(m_id);
}

quint32 Fixture::id() const
{
    return m_id;
}

quint32 Fixture::invalidId()
{
    return UINT_MAX;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

void Fixture::setName(const QString& name)
{
    m_name = name;
    emit changed(m_id);
}

QString Fixture::name() const
{
    return m_name;
}

/*****************************************************************************
 * Fixture type
 *****************************************************************************/

QString Fixture::type()
{
    if (m_fixtureDef != NULL)
        return m_fixtureDef->type();
    else
        return QString(KXMLFixtureDimmer);
}

bool Fixture::isDimmer() const
{
    if (m_fixtureDef != NULL && m_fixtureMode != NULL)
        return false;
    else
        return true;
}

/*****************************************************************************
 * Universe
 *****************************************************************************/

void Fixture::setUniverse(quint32 universe)
{
    /* The universe part is stored in the highest 7 bits */
    m_address = (m_address & 0x01FF) | (universe << 9);

    emit changed(m_id);
}

quint32 Fixture::universe() const
{
    /* The universe part is stored in the highest 7 bits */
    return (m_address >> 9);
}

/*****************************************************************************
 * Address
 *****************************************************************************/

void Fixture::setAddress(quint32 address)
{
    /* Don't allow more than 512 channels per universe */
    if (address > 511)
        return;

    /* The address part is stored in the lowest 9 bits */
    m_address = (m_address & 0xFE00) | (address & 0x01FF);

    emit changed(m_id);
}

quint32 Fixture::address() const
{
    /* The address part is stored in the lowest 9 bits */
    return (m_address & 0x01FF);
}

quint32 Fixture::universeAddress() const
{
    return m_address;
}

/*****************************************************************************
 * Channels
 *****************************************************************************/

void Fixture::setChannels(quint32 channels)
{
    m_channels = channels;
    emit changed(m_id);
}

quint32 Fixture::channels() const
{
    if (m_fixtureDef != NULL && m_fixtureMode != NULL)
        return m_fixtureMode->channels().size();
    else
        return m_channels;
}

const QLCChannel* Fixture::channel(quint32 channel) const
{
    if (m_fixtureDef != NULL && m_fixtureMode != NULL)
        return m_fixtureMode->channel(channel);
    else if (channel < channels())
        return m_genericChannel;
    else
        return NULL;
}

quint32 Fixture::channelAddress(quint32 channel) const
{
    if (channel < channels())
        return universeAddress() + channel;
    else
        return QLCChannel::invalid();
}

quint32 Fixture::channel(const QString& name, Qt::CaseSensitivity cs,
                         QLCChannel::Group group) const
{
    if (m_fixtureDef == NULL && m_fixtureMode == NULL)
    {
        /* There's just one generic channel object with "Intensity" as
           its name that is the same for all channel numbers. So
           there's really no point in returning 0 or any otherwise
           valid channel number here. Which one of them the user would
           want to get? */
        return QLCChannel::invalid();
    }
    else
    {
        /* Search for the channel name (and group) from our list */
        for (quint32 i = 0; i < quint32(m_fixtureMode->channels().size()); i++)
        {
            const QLCChannel* ch = m_fixtureMode->channel(i);
            Q_ASSERT(ch != NULL);

            if (group != QLCChannel::NoGroup && ch->group() != group)
            {
                /* Given group name doesn't match */
                continue;
            }
            else if (ch->name().contains(name, cs) == true)
            {
                /* Found the channel */
                return i;
            }
        }

        /* Went thru all channels but a match was not found */
        return QLCChannel::invalid();
    }
}

QSet <quint32> Fixture::channels(const QString& name, Qt::CaseSensitivity cs,
                                 QLCChannel::Group group) const
{
    QSet <quint32> set;
    if (m_fixtureDef != NULL && m_fixtureMode != NULL)
    {
        /* Search for the channel name (and group) from our list */
        for (quint32 i = 0; i < quint32(m_fixtureMode->channels().size()); i++)
        {
            const QLCChannel* ch = m_fixtureMode->channel(i);
            Q_ASSERT(ch != NULL);

            if (group != QLCChannel::NoGroup && ch->group() != group)
            {
                /* Given group name doesn't match */
                continue;
            }
            else if (ch->name().contains(name, cs) == true)
            {
                /* Found the channel */
                set << i;
            }
        }
    }

    return set;
}

quint32 Fixture::panMsbChannel(int head) const
{
    if (m_fixtureMode != NULL)
    {
        if (head < m_fixtureMode->heads().size())
            return m_fixtureMode->heads().at(head).panMsbChannel();
        else
            return QLCChannel::invalid();
    }
    else
    {
        return QLCChannel::invalid();
    }
}

quint32 Fixture::tiltMsbChannel(int head) const
{
    if (m_fixtureMode != NULL)
    {
        if (head < m_fixtureMode->heads().size())
            return m_fixtureMode->heads().at(head).tiltMsbChannel();
        else
            return QLCChannel::invalid();
    }
    else
    {
        return QLCChannel::invalid();
    }
}

quint32 Fixture::panLsbChannel(int head) const
{
    if (m_fixtureMode != NULL)
    {
        if (head < m_fixtureMode->heads().size())
            return m_fixtureMode->heads().at(head).panLsbChannel();
        else
            return QLCChannel::invalid();
    }
    else
    {
        return QLCChannel::invalid();
    }
}

quint32 Fixture::tiltLsbChannel(int head) const
{
    if (m_fixtureMode != NULL)
    {
        if (head < m_fixtureMode->heads().size())
            return m_fixtureMode->heads().at(head).tiltLsbChannel();
        else
            return QLCChannel::invalid();
    }
    else
    {
        return QLCChannel::invalid();
    }
}

quint32 Fixture::masterIntensityChannel(int head) const
{
    if (m_fixtureMode != NULL)
    {
        quint32 dimmerCh = QLCChannel::invalid();
        if (head < m_fixtureMode->heads().size())
            dimmerCh = m_fixtureMode->heads().at(head).masterIntensityChannel();

        if (dimmerCh == QLCChannel::invalid())
        {
            QList <quint32> dList = channels("dimmer", Qt::CaseInsensitive, QLCChannel::Intensity).toList();

            if (dList.count() > 0)
                dimmerCh = dList.at(0);
        }
        return dimmerCh;
    }
    else
    {
        return QLCChannel::invalid();
    }
}

QList <quint32> Fixture::rgbChannels(int head) const
{
    if (m_fixtureMode != NULL)
    {
        if (head < m_fixtureMode->heads().size())
            return m_fixtureMode->heads().at(head).rgbChannels();
        else
            return QList <quint32> ();
    }
    else
    {
        return QList <quint32> ();
    }
}

QList <quint32> Fixture::cmyChannels(int head) const
{
    if (m_fixtureMode != NULL)
    {
        if (head < m_fixtureMode->heads().size())
            return m_fixtureMode->heads().at(head).cmyChannels();
        else
            return QList <quint32> ();
    }
    else
    {
        return QList <quint32> ();
    }
}

void Fixture::setExcludeFadeChannels(QList<int> indexes)
{
    if (indexes.count() > (int)channels())
        return;
    m_excludeFadeIndexes = indexes;
}

QList<int> Fixture::excludeFadeChannels()
{
    return m_excludeFadeIndexes;
}

void Fixture::setChannelCanFade(int idx, bool canFade)
{
    if (canFade == false && m_excludeFadeIndexes.contains(idx) == false)
    {
        m_excludeFadeIndexes.append(idx);
        qSort(m_excludeFadeIndexes.begin(), m_excludeFadeIndexes.end());
    }
    else if (canFade == true && m_excludeFadeIndexes.contains(idx) == true)
    {
        m_excludeFadeIndexes.removeOne(idx);
    }
}

bool Fixture::channelCanFade(int index)
{
    if (m_excludeFadeIndexes.contains(index))
        return false;

    return true;
}

void Fixture::createGenericChannel()
{
    if (m_genericChannel == NULL)
    {
        m_genericChannel = new QLCChannel();
        Q_ASSERT(m_genericChannel != NULL);
        m_genericChannel->setGroup(QLCChannel::Intensity);
        m_genericChannel->setName(tr("Intensity"));
        m_genericChannel->addCapability(
                            new QLCCapability(0, UCHAR_MAX, tr("Intensity")));
    }
}

/*****************************************************************************
 * Fixture definition
 *****************************************************************************/

void Fixture::setFixtureDefinition(QLCFixtureDef* fixtureDef,
                                   QLCFixtureMode* fixtureMode)
{
    if (fixtureDef != NULL && fixtureMode != NULL)
    {
        m_fixtureDef = fixtureDef;
        m_fixtureMode = fixtureMode;

        // If there are no head entries in the mode, create one that contains
        // all channels. This const_cast is a bit heretic, but it's easier this
        // way, than to change everything def & mode related non-const, which would
        // be worse than one constness violation here.
        QLCFixtureMode* mode = const_cast<QLCFixtureMode*> (fixtureMode);
        if (mode->heads().size() == 0)
        {
            QLCFixtureHead head;
            for (int i = 0; i < mode->channels().size(); i++)
                head.addChannel(i);
            mode->insertHead(-1, head);
        }

        // Cache all head channels
        mode->cacheHeads();

        if (m_genericChannel != NULL)
            delete m_genericChannel;
        m_genericChannel = NULL;
    }
    else
    {
        m_fixtureDef = NULL;
        m_fixtureMode = NULL;
        createGenericChannel();
    }

    emit changed(m_id);
}

QLCFixtureDef* Fixture::fixtureDef() const
{
    return m_fixtureDef;
}

QLCFixtureMode* Fixture::fixtureMode() const
{
    return m_fixtureMode;
}

int Fixture::heads() const
{
    if (isDimmer() == true)
        return channels();
    else
        return m_fixtureMode->heads().size();
}

QLCFixtureHead Fixture::head(int index) const
{
    if (isDimmer() == true)
    {
        if (index < int(channels()))
            return QLCDimmerHead(index);
        else
            return QLCFixtureHead();
    }
    else
    {
        if (index < m_fixtureMode->heads().size())
            return m_fixtureMode->heads().at(index);
        else
            return QLCFixtureHead();
    }
}

QIcon Fixture::getIconFromType(QString type) const
{
    if (type == "Color Changer")
        return QIcon(":/fixture.png");
    else if (type == "Dimmer")
        return QIcon(":/dimmer.png");
    else if (type == "Effect")
        return QIcon(":/effect.png");
    else if (type == "Fan")
         return QIcon(":/fan.png");
    else if (type == "Flower")
        return QIcon(":/flower.png");
    else if (type == "Hazer")
        return QIcon(":/hazer.png");
    else if (type == "Laser")
        return QIcon(":/laser.png");
    else if (type == "Moving Head")
        return QIcon(":/movinghead.png");
    else if (type == "Scanner")
        return QIcon(":/scanner.png");
    else if (type == "Smoke")
        return QIcon(":/smoke.png");
    else if (type == "Strobe")
        return QIcon(":/strobe.png");

    return QIcon(":/other.png");
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Fixture::loader(const QDomElement& root, Doc* doc)
{
    bool result = false;

    Fixture* fxi = new Fixture(doc);
    Q_ASSERT(fxi != NULL);

    if (fxi->loadXML(root, doc, doc->fixtureDefCache()) == true)
    {
        if (doc->addFixture(fxi, fxi->id()) == true)
        {
            /* Success */
            result = true;
        }
        else
        {
            /* Doc is full */
            qWarning() << Q_FUNC_INFO << "Fixture" << fxi->name()
                       << "cannot be created.";
            delete fxi;
        }
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Fixture" << fxi->name() << "cannot be loaded.";
        delete fxi;
    }

    return result;
}

bool Fixture::loadXML(const QDomElement& root, Doc *doc,
                      const QLCFixtureDefCache* fixtureDefCache)
{
    QLCFixtureDef* fixtureDef = NULL;
    QLCFixtureMode* fixtureMode = NULL;
    QString manufacturer;
    QString model;
    QString modeName;
    QString name;
    quint32 id = Fixture::invalidId();
    quint32 universe = 0;
    quint32 address = 0;
    quint32 channels = 0;
    QList<int> excludeList;

    if (root.tagName() != KXMLFixture)
    {
        qWarning() << Q_FUNC_INFO << "Fixture node not found";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();

        if (tag.tagName() == KXMLQLCFixtureDefManufacturer)
        {
            manufacturer = tag.text();
        }
        else if (tag.tagName() == KXMLQLCFixtureDefModel)
        {
            model = tag.text();
        }
        else if (tag.tagName() == KXMLQLCFixtureMode)
        {
            modeName = tag.text();
        }
        else if (tag.tagName() == KXMLFixtureID)
        {
            id = tag.text().toUInt();
        }
        else if (tag.tagName() == KXMLFixtureName)
        {
            name = tag.text();
        }
        else if (tag.tagName() == KXMLFixtureUniverse)
        {
            universe = tag.text().toInt();
        }
        else if (tag.tagName() == KXMLFixtureAddress)
        {
            address = tag.text().toInt();
        }
        else if (tag.tagName() == KXMLFixtureChannels)
        {
            channels = tag.text().toInt();
        }
        else if (tag.tagName() == KXMLFixtureExcludeFade)
        {
            QString list = tag.text();
            QStringList values = list.split(",");

            for (int i = 0; i < values.count(); i++)
                excludeList.append(values.at(i).toInt());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown fixture tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    /* Find the given fixture definition, unless its a generic dimmer */
    if (model != KXMLFixtureGeneric)
    {
        fixtureDef = fixtureDefCache->fixtureDef(manufacturer, model);
        if (fixtureDef == NULL)
        {
            doc->appendToErrorLog(QString("No fixture definition found for <%1> <%2>")
                                  .arg(manufacturer)
                                  .arg(model));
        }
        else
        {
            /* Find the given fixture mode */
            fixtureMode = fixtureDef->mode(modeName);
            if (fixtureMode == NULL)
            {
                doc->appendToErrorLog(QString("Fixture mode <%1> not found for <%2> <%3>")
                                      .arg(modeName).arg(manufacturer).arg(model));

                /* Set this also NULL so that a generic dimmer will be
                   created instead as a backup. */
                fixtureDef = NULL;
            }
        }
    }

    /* Number of channels */
    if (channels <= 0)
    {
        doc->appendToErrorLog(QString("%1 channels of fixture <%2> are our of bounds")
                              .arg(QString::number(channels))
                              .arg(name));
        channels = 1;
    }

    /* Make sure that address is something sensible */
    if (address > 511 || address + (channels - 1) > 511)
    {
        doc->appendToErrorLog(QString("Fixture address range %1-%2 is out of DMX bounds")
                              .arg(QString::number(address))
                              .arg(QString::number(address + channels - 1)));
        address = 0;
    }

    /* Check that the invalid ID is not used */
    if (id == Fixture::invalidId())
    {
        qWarning() << Q_FUNC_INFO << "Fixture ID" << id << "is not allowed.";
        return false;
    }

    if (fixtureDef != NULL && fixtureMode != NULL)
    {
        /* Assign fixtureDef & mode only if BOTH are not NULL */
        setFixtureDefinition(fixtureDef, fixtureMode);
    }
    else
    {
        /* Otherwise set just the channel count */
        setChannels(channels);
    }

    setAddress(address);
    setUniverse(universe);
    setName(name);
    setExcludeFadeChannels(excludeList);
    setID(id);

    return true;
}

bool Fixture::saveXML(QDomDocument* doc, QDomElement* wksp_root) const
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);

    /* Fixture Instance entry */
    root = doc->createElement(KXMLFixture);
    wksp_root->appendChild(root);

    /* Manufacturer */
    tag = doc->createElement(KXMLQLCFixtureDefManufacturer);
    root.appendChild(tag);

    if (m_fixtureDef != NULL)
        text = doc->createTextNode(m_fixtureDef->manufacturer());
    else
        text = doc->createTextNode(KXMLFixtureGeneric);

    tag.appendChild(text);

    /* Model */
    tag = doc->createElement(KXMLQLCFixtureDefModel);
    root.appendChild(tag);

    if (m_fixtureDef != NULL)
        text = doc->createTextNode(m_fixtureDef->model());
    else
        text = doc->createTextNode(KXMLFixtureGeneric);

    tag.appendChild(text);

    /* Fixture mode */
    tag = doc->createElement(KXMLQLCFixtureMode);
    root.appendChild(tag);

    if (m_fixtureMode != NULL)
        text = doc->createTextNode(m_fixtureMode->name());
    else
        text = doc->createTextNode(KXMLFixtureGeneric);

    tag.appendChild(text);

    /* ID */
    tag = doc->createElement(KXMLFixtureID);
    root.appendChild(tag);
    str.setNum(id());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Name */
    tag = doc->createElement(KXMLFixtureName);
    root.appendChild(tag);
    text = doc->createTextNode(m_name);
    tag.appendChild(text);

    /* Universe */
    tag = doc->createElement(KXMLFixtureUniverse);
    root.appendChild(tag);
    str.setNum(universe());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Address */
    tag = doc->createElement(KXMLFixtureAddress);
    root.appendChild(tag);
    str.setNum(address());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Channel count */
    tag = doc->createElement(KXMLFixtureChannels);
    root.appendChild(tag);
    str.setNum(channels());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    if (m_excludeFadeIndexes.count() > 0)
    {
        tag = doc->createElement(KXMLFixtureExcludeFade);
        root.appendChild(tag);
        QString list;
        for (int i = 0; i < m_excludeFadeIndexes.count(); i++)
        {
            if (list.isEmpty() == false)
                list.append(QString(","));
            list.append(QString("%1").arg(m_excludeFadeIndexes.at(i)));
        }
        text = doc->createTextNode(list);
        tag.appendChild(text);
    }

    return true;
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString Fixture::status() const
{
    QString info;
    QString t;

    QString title("<TR><TD CLASS='hilite' COLSPAN='3'>%1</TD></TR>");
    QString subTitle("<TR><TD CLASS='subhi' COLSPAN='3'>%1</TD></TR>");
    QString genInfo("<TR><TD CLASS='emphasis'>%1</TD><TD COLSPAN='2'>%2</TD></TR>");

    /********************************************************************
     * General info
     ********************************************************************/

    info += "<TABLE COLS='3' WIDTH='100%'>";

    // Fixture title
    info += title.arg(name());

    // Manufacturer
    if (isDimmer() == false)
    {
        info += genInfo.arg(tr("Manufacturer")).arg(m_fixtureDef->manufacturer());
        info += genInfo.arg(tr("Model")).arg(m_fixtureDef->model());
        info += genInfo.arg(tr("Mode")).arg(m_fixtureMode->name());
        info += genInfo.arg(tr("Type")).arg(m_fixtureDef->type());
    }
    else
    {
        info += genInfo.arg(tr("Type")).arg(tr("Generic Dimmer"));
    }

    // Universe
    info += genInfo.arg(tr("Universe")).arg(universe() + 1);

    // Address
    QString range = QString("%1 - %2").arg(address() + 1).arg(address() + channels());
    info += genInfo.arg(tr("Address Range")).arg(range);

    // Channels
    info += genInfo.arg(tr("Channels")).arg(channels());

    // Binary address
    QString binaryStr = QString("%1").arg(address() + 1, 10, 2, QChar('0'));
    QString dipTable("<TABLE COLS='33' cellspacing='0'><TR><TD COLSPAN='33'><IMG SRC=\"" ":/ds_top.png\"></TD></TR>");
    dipTable += "<TR><TD><IMG SRC=\"" ":/ds_border.png\"></TD><TD><IMG SRC=\"" ":/ds_border.png\"></TD>";
    for (int i = 9; i >= 0; i--)
    {
        if (binaryStr.at(i) == '0')
            dipTable += "<TD COLSPAN='3'><IMG SRC=\"" ":/ds_off.png\"></TD>";
        else
            dipTable += "<TD COLSPAN='3'><IMG SRC=\"" ":/ds_on.png\"></TD>";
    }
    dipTable += "<TD><IMG SRC=\"" ":/ds_border.png\"></TD></TR>";
    dipTable += "<TR><TD COLSPAN='33'><IMG SRC=\"" ":/ds_bottom.png\"></TD></TR>";
    dipTable += "</TABLE>";

    info += genInfo.arg(tr("Binary Address (DIP)"))
            .arg(QString("%1").arg(dipTable));

    /********************************************************************
     * Channels
     ********************************************************************/

    // Title row
    info += QString("<TR><TD CLASS='subhi'>%1</TD>").arg(tr("Channel"));
    info += QString("<TD CLASS='subhi'>%1</TD>").arg(tr("DMX"));
    info += QString("<TD CLASS='subhi'>%1</TD></TR>").arg(tr("Name"));

    // Fill table with the fixture's channels
    for (quint32 ch = 0; ch < channels();	ch++)
    {
        QString chInfo("<TR><TD>%1</TD><TD>%2</TD><TD>%3</TD></TR>");
        info += chInfo.arg(ch + 1).arg(address() + ch + 1)
                .arg(channel(ch)->name());
    }

    /********************************************************************
     * Extended device information for non-dimmers
     ********************************************************************/

    if (isDimmer() == false)
    {
        QLCPhysical physical = m_fixtureMode->physical();
        info += title.arg(tr("Physical"));

        float mmInch = 0.0393700787;
        float kgLbs = 2.20462262;
        QString mm("%1mm (%2\")");
        QString kg("%1kg (%2 lbs)");
        QString W("%1W");
        info += genInfo.arg(tr("Width")).arg(mm.arg(physical.width()))
                                        .arg(physical.width() * mmInch, 0, 'g', 4);
        info += genInfo.arg(tr("Height")).arg(mm.arg(physical.height()))
                                         .arg(physical.height() * mmInch, 0, 'g', 4);
        info += genInfo.arg(tr("Depth")).arg(mm.arg(physical.depth()))
                                        .arg(physical.depth() * mmInch, 0, 'g', 4);
        info += genInfo.arg(tr("Weight")).arg(kg.arg(physical.weight()))
                                         .arg(physical.weight() * kgLbs, 0, 'g', 4);
        info += genInfo.arg(tr("Power consumption")).arg(W.arg(physical.powerConsumption()));
        info += genInfo.arg(tr("DMX Connector")).arg(physical.dmxConnector());

        // Bulb
        QString K("%1K");
        QString lm("%1lm");
        info += subTitle.arg(tr("Bulb"));
        info += genInfo.arg(tr("Type")).arg(physical.bulbType());
        info += genInfo.arg(tr("Luminous Flux")).arg(lm.arg(physical.bulbLumens()));
        info += genInfo.arg(tr("Colour Temperature")).arg(K.arg(physical.bulbColourTemperature()));

        // Lens
        QString angle1("%1&deg;");
        QString angle2("%1&deg; &ndash; %2&deg;");

        info += subTitle.arg(tr("Lens"));
        info += genInfo.arg(tr("Name")).arg(physical.lensName());

        if (physical.lensDegreesMin() == physical.lensDegreesMax())
        {
            info += genInfo.arg(tr("Beam Angle"))
                .arg(angle1.arg(physical.lensDegreesMin()));
        }
        else
        {
            info += genInfo.arg(tr("Beam Angle"))
                .arg(angle2.arg(physical.lensDegreesMin())
                .arg(physical.lensDegreesMax()));
        }


        // Focus
        QString range("%1&deg;");
        info += subTitle.arg(tr("Focus"));
        info += genInfo.arg(tr("Type")).arg(physical.focusType());
        info += genInfo.arg(tr("Pan Range")).arg(range.arg(physical.focusPanMax()));
        info += genInfo.arg(tr("Tilt Range")).arg(range.arg(physical.focusTiltMax()));
    }

    // HTML document & table closure
    info += "</TABLE>";

    if (isDimmer() == false)
    {
        info += "<HR>";
        info += "<DIV CLASS='author' ALIGN='right'>";
        info += tr("Fixture definition author: ") + fixtureDef()->author();
        info += "</DIV>";
    }

    return info;
}
