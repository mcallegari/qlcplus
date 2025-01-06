/*
  Q Light Controller Plus
  fixture.cpp

  Copyright (C) Heikki Junnila
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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QString>
#include <QtMath>
#include <QDebug>

#include "qlcfixturedefcache.h"
#include "channelmodifier.h"
#include "qlcfixturemode.h"
#include "qlcfixturehead.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "qlcfile.h"

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
    m_crossUniverse = false;

    m_fixtureDef = NULL;
    m_fixtureMode = NULL;
}

Fixture::~Fixture()
{
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

QString Fixture::typeString()
{
    if (m_fixtureDef != NULL)
        return m_fixtureDef->typeToString(m_fixtureDef->type());
    else
        return QString(KXMLFixtureDimmer);
}

QLCFixtureDef::FixtureType Fixture::type() const
{
    if (m_fixtureDef != NULL)
        return m_fixtureDef->type();
    else
        return QLCFixtureDef::Dimmer;
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

void Fixture::setCrossUniverse(bool enable)
{
    m_crossUniverse = enable;
}

bool Fixture::crossUniverse() const
{
    return m_crossUniverse;
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
    if (m_fixtureDef == NULL && m_fixtureMode == NULL)
    {
        QLCFixtureDef *fixtureDef = genericDimmerDef(channels);
        QLCFixtureMode *fixtureMode = genericDimmerMode(fixtureDef, channels);
        setFixtureDefinition(fixtureDef, fixtureMode);
    }
    else
    {
        if ((quint32)m_fixtureMode->channels().size() != channels)
        {
            QLCFixtureDef *fixtureDef = genericDimmerDef(channels);
            QLCFixtureMode *fixtureMode = genericDimmerMode(fixtureDef, channels);
            setFixtureDefinition(fixtureDef, fixtureMode);
        }
    }

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

quint32 Fixture::channel(QLCChannel::Group group,
    QLCChannel::PrimaryColour color) const
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
            else if (group != QLCChannel::Intensity || ch->colour() == color)
            {
                /* Found the channel */
                return i;
            }
        }

        /* Went thru all channels but a match was not found */
        return QLCChannel::invalid();
    }
}

QSet <quint32> Fixture::channels(QLCChannel::Group group, QLCChannel::PrimaryColour color) const
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
            else if (group != QLCChannel::Intensity || ch->colour() == color)
            {
                /* Found the channel */
                set << i;
            }
        }
    }

    return set;
}

quint32 Fixture::channelNumber(int type, int controlByte, int head) const
{
    if (m_fixtureMode == NULL || head < 0 || head >= m_fixtureMode->heads().size())
        return QLCChannel::invalid();

    return m_fixtureMode->heads().at(head).channelNumber(type, controlByte);
}

quint32 Fixture::masterIntensityChannel() const
{
    if (m_fixtureMode == NULL)
        return QLCChannel::invalid();

    return m_fixtureMode->masterIntensityChannel();
}

QVector <quint32> Fixture::rgbChannels(int head) const
{
    if (m_fixtureMode == NULL || head < 0 || head >= m_fixtureMode->heads().size())
        return QVector <quint32> ();

    return m_fixtureMode->heads().at(head).rgbChannels();
}

QVector <quint32> Fixture::cmyChannels(int head) const
{
    if (m_fixtureMode == NULL || head < 0 || head >= m_fixtureMode->heads().size())
        return QVector <quint32> ();

    return m_fixtureMode->heads().at(head).cmyChannels();
}

QList<SceneValue> Fixture::positionToValues(int type, float degrees, bool isRelative)
{
    QList<SceneValue> posList;
    // cache a list of channels processed, to avoid duplicates
    QList<quint32> chDone;

    if (m_fixtureMode == NULL)
        return posList;

    QLCPhysical phy = fixtureMode()->physical();
    qreal headDegrees = degrees, maxDegrees;
    float msbValue = 0, lsbValue = 0;

    if (type == QLCChannel::Pan)
    {
        maxDegrees = phy.focusPanMax();
        if (maxDegrees == 0) maxDegrees = 360;

        for (int i = 0; i < heads(); i++)
        {
            quint32 panMSB = channelNumber(QLCChannel::Pan, QLCChannel::MSB, i);
            if (panMSB == QLCChannel::invalid() || chDone.contains(panMSB))
                continue;
            quint32 panLSB = channelNumber(QLCChannel::Pan, QLCChannel::LSB, i);

            if (isRelative)
            {
                // degrees is a relative value upon the current value.
                // Recalculate absolute degrees here
                float chDegrees = (qreal(phy.focusPanMax()) / 256.0) * channelValueAt(panMSB);
                headDegrees = qBound(0.0, chDegrees + headDegrees, maxDegrees);

                if (panLSB != QLCChannel::invalid())
                {
                    chDegrees = (qreal(phy.focusPanMax()) / 65536.0) * channelValueAt(panLSB);
                    headDegrees = qBound(0.0, chDegrees + headDegrees, maxDegrees);
                }
            }

            quint16 degToDmx = (headDegrees * 65535.0) / qreal(phy.focusPanMax());
            posList.append(SceneValue(id(), panMSB, static_cast<uchar>(degToDmx >> 8)));

            if (panLSB != QLCChannel::invalid())
                posList.append(SceneValue(id(), panLSB, static_cast<uchar>(degToDmx & 0x00FF)));

            qDebug() << "[positionToValues] Pan MSB:" << msbValue << "LSB:" << lsbValue;

            chDone.append(panMSB);
        }
    }
    else if (type == QLCChannel::Tilt)
    {
        maxDegrees = phy.focusTiltMax();
        if (maxDegrees == 0) maxDegrees = 270;

        for (int i = 0; i < heads(); i++)
        {
            quint32 tiltMSB = channelNumber(QLCChannel::Tilt, QLCChannel::MSB, i);
            if (tiltMSB == QLCChannel::invalid() || chDone.contains(tiltMSB))
                continue;
            quint32 tiltLSB = channelNumber(QLCChannel::Tilt, QLCChannel::LSB, i);

            if (isRelative)
            {
                // degrees is a relative value upon the current value.
                // Recalculate absolute degrees here
                float chDegrees = (qreal(phy.focusTiltMax()) / 256.0) * channelValueAt(tiltMSB);
                headDegrees = qBound(0.0, chDegrees + headDegrees, maxDegrees);

                if (tiltLSB != QLCChannel::invalid())
                {
                    chDegrees = (qreal(phy.focusPanMax()) / 65536.0) * channelValueAt(tiltLSB);
                    headDegrees = qBound(0.0, chDegrees + headDegrees, maxDegrees);
                }
            }

            quint16 degToDmx = (headDegrees * 65535.0) / qreal(phy.focusTiltMax());
            posList.append(SceneValue(id(), tiltMSB, static_cast<uchar>(degToDmx >> 8)));

            if (tiltLSB != QLCChannel::invalid())
                posList.append(SceneValue(id(), tiltLSB, static_cast<uchar>(degToDmx & 0x00FF)));

            qDebug() << "[positionToValues] Tilt MSB:" << msbValue << "LSB:" << lsbValue;

            chDone.append(tiltMSB);
        }

    }

    return posList;
}

QList<SceneValue> Fixture::zoomToValues(float degrees, bool isRelative)
{
    QList<SceneValue> chList;

    if (m_fixtureMode == NULL)
        return chList;

    QLCPhysical phy = fixtureMode()->physical();
    if (!isRelative)
        degrees = qBound(float(phy.lensDegreesMin()), degrees, float(phy.lensDegreesMax()));

    float deltaDegrees = phy.lensDegreesMax() - phy.lensDegreesMin();
    // delta : 0xFFFF = deg : x
    quint16 degToDmx = ((qAbs(degrees) - (isRelative ? 0 : float(phy.lensDegreesMin()))) * 65535.0) / deltaDegrees;
    qDebug() << "Degrees" << degrees << "DMX" << QString::number(degToDmx, 16);

    for (quint32 i = 0; i < quint32(m_fixtureMode->channels().size()); i++)
    {
        const QLCChannel *ch = m_fixtureMode->channel(i);

        if (ch->group() != QLCChannel::Beam)
            continue;

        if (ch->preset() != QLCChannel::BeamZoomBigSmall &&
            ch->preset() != QLCChannel::BeamZoomSmallBig &&
            ch->preset() != QLCChannel::BeamZoomFine)
            continue;

        if (isRelative)
        {
            // degrees is a relative value upon the current value.
            // Recalculate absolute degrees here
            qreal divider = ch->controlByte() == QLCChannel::MSB ? 256.0 : 65536.0;
            uchar currDmxValue = ch->preset() == QLCChannel::BeamZoomBigSmall ? UCHAR_MAX - channelValueAt(i) : channelValueAt(i);
            float chDegrees = float((phy.lensDegreesMax() - phy.lensDegreesMin()) / divider) * float(currDmxValue);

            qDebug() << "Relative channel degrees:" << chDegrees << "MSB?" << ch->controlByte();

            quint16 currDmxVal = (chDegrees * 65535.0) / deltaDegrees;
            if (degrees > 0)
                degToDmx = qBound(0, currDmxVal + degToDmx, 65535);
            else
                degToDmx = qBound(0, currDmxVal - degToDmx, 65535);
        }

        if (ch->controlByte() == QLCChannel::MSB)
        {
            if (ch->preset() == QLCChannel::BeamZoomBigSmall)
                chList.append(SceneValue(id(), i, static_cast<uchar>(UCHAR_MAX - (degToDmx >> 8))));
            else
                chList.append(SceneValue(id(), i, static_cast<uchar>(degToDmx >> 8)));
        }
        else if (ch->controlByte() == QLCChannel::LSB)
        {
            chList.append(SceneValue(id(), i, static_cast<uchar>(degToDmx & 0x00FF)));
        }
    }

    return chList;
}

void Fixture::setExcludeFadeChannels(QList<int> indices)
{
    if (indices.count() > (int)channels())
        return;

    m_excludeFadeIndices = indices;
}

QList<int> Fixture::excludeFadeChannels()
{
    return m_excludeFadeIndices;
}

void Fixture::setChannelCanFade(int idx, bool canFade)
{
    if (canFade == false && m_excludeFadeIndices.contains(idx) == false)
    {
        m_excludeFadeIndices.append(idx);
        std::sort(m_excludeFadeIndices.begin(), m_excludeFadeIndices.end());
    }
    else if (canFade == true && m_excludeFadeIndices.contains(idx) == true)
    {
        m_excludeFadeIndices.removeOne(idx);
    }
}

bool Fixture::channelCanFade(int index)
{
    if (m_excludeFadeIndices.contains(index))
        return false;

    return true;
}

void Fixture::setForcedHTPChannels(QList<int> indices)
{
    if (indices.count() > (int)channels())
        return;
    m_forcedHTPIndices = indices;
    // cross check: if a channel is forced HTP it must be removed from
    // the forced LTP list (if present)
    for (int i = 0; i < m_forcedHTPIndices.count(); i++)
        m_forcedLTPIndices.removeAll(m_forcedHTPIndices.at(i));
}

QList<int> Fixture::forcedHTPChannels()
{
    return m_forcedHTPIndices;
}

void Fixture::setForcedLTPChannels(QList<int> indices)
{
    if (indices.count() > (int)channels())
        return;
    m_forcedLTPIndices = indices;
    // cross check: if a channel is forced LTP it must be removed from
    // the forced HTP list (if present)
    for (int i = 0; i < m_forcedLTPIndices.count(); i++)
        m_forcedHTPIndices.removeAll(m_forcedLTPIndices.at(i));
}

QList<int> Fixture::forcedLTPChannels()
{
    return m_forcedLTPIndices;
}

void Fixture::setChannelModifier(quint32 idx, ChannelModifier *mod)
{
    if (idx >= channels())
        return;

    if (mod == NULL)
    {
        m_channelModifiers.remove(idx);
        return;
    }

    qDebug() << Q_FUNC_INFO << idx << mod->name();
    m_channelModifiers[idx] = mod;
}

ChannelModifier *Fixture::channelModifier(quint32 idx)
{
    if (m_channelModifiers.contains(idx))
        return m_channelModifiers[idx];

    return NULL;
}

/*********************************************************************
 * Channel info
 *********************************************************************/

bool Fixture::setChannelValues(const QByteArray &values)
{
    const int addr = address();
    if (addr >= values.size())
        return false;

    const int chNum = qMin(values.size() - addr, (int)channels());
    bool changed = false;

    // Most of the times there are no changes,
    // so the lock is inside the cycle
    for (int i = 0; i < chNum; i++)
    {
        if (m_values.at(i) != values.at(i + addr))
        {
            changed = true;
            QMutexLocker locker(&m_channelsInfoMutex);
            m_values[i] = values.at(i + addr);
            checkAlias(i, m_values[i]);
        }
    }

    if (changed == true)
        emit valuesChanged();

    return changed;
}

QByteArray Fixture::channelValues()
{
    QMutexLocker locker(&m_channelsInfoMutex);
    return m_values;
}

uchar Fixture::channelValueAt(int idx)
{
    QMutexLocker locker(&m_channelsInfoMutex);
    if (idx >= 0 && idx < m_values.length())
        return (uchar)m_values.at(idx);
    return 0;
}

void Fixture::checkAlias(int chIndex, uchar value)
{
    if (chIndex < 0 || chIndex >= m_aliasInfo.count() ||
        m_aliasInfo[chIndex].m_hasAlias == false)
        return;

    // If the channel @chIndex has aliases, check
    // if replacements are to be done
    QLCCapability *cap = m_fixtureMode->channel(chIndex)->searchCapability(value);
    if (cap == NULL || cap == m_aliasInfo[chIndex].m_currCap)
        return;

    // first, revert any channel replaced to the original channel set
    foreach (AliasInfo alias, m_aliasInfo[chIndex].m_currCap->aliasList())
    {
        QLCFixtureMode *mode = m_fixtureDef->mode(alias.targetMode);
        if (mode != m_fixtureMode)
            continue;

        QLCChannel *currChannel = m_fixtureMode->channel(alias.targetChannel);
        QLCChannel *origChannel = m_fixtureDef->channel(alias.sourceChannel);

        m_fixtureMode->replaceChannel(currChannel, origChannel);
    }

    // now, apply the current alias changes
    foreach (AliasInfo alias, cap->aliasList())
    {
        QLCFixtureMode *mode = m_fixtureDef->mode(alias.targetMode);
        if (mode != m_fixtureMode)
            continue;

        QLCChannel *currChannel = m_fixtureMode->channel(alias.sourceChannel);
        QLCChannel *newChannel = m_fixtureDef->channel(alias.targetChannel);

        m_fixtureMode->replaceChannel(currChannel, newChannel);
    }

    emit aliasChanged();

    m_aliasInfo[chIndex].m_currCap = cap;

}

/*****************************************************************************
 * Fixture definition
 *****************************************************************************/

void Fixture::setFixtureDefinition(QLCFixtureDef* fixtureDef,
                                   QLCFixtureMode* fixtureMode)
{
    if (fixtureDef != NULL && fixtureMode != NULL)
    {
        int i, chNum;

        if (m_fixtureDef != NULL && m_fixtureDef != fixtureDef &&
            m_fixtureDef->manufacturer() == KXMLFixtureGeneric &&
            m_fixtureDef->model() == KXMLFixtureGeneric)
        {
            delete m_fixtureDef;
        }

        m_fixtureDef = fixtureDef;
        m_fixtureMode = fixtureMode;
        chNum = fixtureMode->channels().size();

        // If there are no head entries in the mode, create one that contains
        // all channels. This const_cast is a bit heretic, but it's easier this
        // way, than to change everything def & mode related non-const, which would
        // be worse than one constness violation here.
        if (fixtureMode->heads().size() == 0)
        {
            QLCFixtureHead head;
            for (i = 0; i < chNum; i++)
                head.addChannel(i);
            fixtureMode->insertHead(-1, head);
        }

        m_aliasInfo.resize(chNum);

        for (i = 0; i < chNum; i++)
        {
            QLCChannel *channel = fixtureMode->channel(i);
            const QList <QLCCapability*> capsList = channel->capabilities();

            // initialize values with the channel default
            m_values.append(channel->defaultValue());

            // look for aliases
            m_aliasInfo[i].m_hasAlias = false;
            m_aliasInfo[i].m_currCap = capsList.count() ? capsList.at(0) : NULL;

            foreach (QLCCapability *cap, capsList)
            {
                if (cap->preset() == QLCCapability::Alias)
                    m_aliasInfo[i].m_hasAlias = true;
            }
        }

        // Cache all head channels
        fixtureMode->cacheHeads();
    }
    else
    {
        m_fixtureDef = NULL;
        m_fixtureMode = NULL;
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
    return m_fixtureMode->heads().size();
}

QLCFixtureHead Fixture::head(int index) const
{
    if (index < m_fixtureMode->heads().size())
        return m_fixtureMode->heads().at(index);
    else
        return QLCFixtureHead();
}

QString Fixture::iconResource(bool svg) const
{
    QString prefix = svg ? "qrc" : "";
    QString ext = svg ? "svg" : "png";

    switch(type())
    {
        case QLCFixtureDef::ColorChanger: return QString("%1:/fixture.%2").arg(prefix).arg(ext);
        case QLCFixtureDef::Dimmer: return QString("%1:/dimmer.%2").arg(prefix).arg(ext);
        case QLCFixtureDef::Effect: return QString("%1:/effect.%2").arg(prefix).arg(ext);
        case QLCFixtureDef::Fan: return QString("%1:/fan.%2").arg(prefix).arg(ext);
        case QLCFixtureDef::Flower: return QString("%1:/flower.%2").arg(prefix).arg(ext);
        case QLCFixtureDef::Hazer: return QString("%1:/hazer.%2").arg(prefix).arg(ext);
        case QLCFixtureDef::Laser: return QString("%1:/laser.%2").arg(prefix).arg(ext);
        case QLCFixtureDef::MovingHead: return QString("%1:/movinghead.%2").arg(prefix).arg(ext);
        case QLCFixtureDef::Scanner: return QString("%1:/scanner.%2").arg(prefix).arg(ext);
        case QLCFixtureDef::Smoke: return QString("%1:/smoke.%2").arg(prefix).arg(ext);
        case QLCFixtureDef::Strobe: return QString("%1:/strobe.%2").arg(prefix).arg(ext);
        case QLCFixtureDef::LEDBarBeams: return QString("%1:/ledbar_beams.%2").arg(prefix).arg(ext);
        case QLCFixtureDef::LEDBarPixels: return QString("%1:/ledbar_pixels.%2").arg(prefix).arg(ext);
        default: break;
    }

    return QString("%1:/other.%2").arg(prefix).arg(ext);
}

QIcon Fixture::getIconFromType() const
{
    return QIcon(iconResource());
}

QRectF Fixture::degreesRange(int head) const
{
    // TODO: handle fixtures with only pan or tilt

    if (m_fixtureMode != NULL && head < m_fixtureMode->heads().size())
    {
        QLCPhysical physical(m_fixtureMode->physical());
        qreal pan = physical.focusPanMax();
        qreal tilt = physical.focusTiltMax();

        if (pan != 0 && tilt != 0)
        {
            return QRectF(-pan/2, -tilt/2, pan, tilt);
        }
    }

    return QRectF();
}

/*********************************************************************
 * Generic Dimmer
 *********************************************************************/

QLCFixtureDef *Fixture::genericDimmerDef(int channels)
{
    QLCFixtureDef *def = new QLCFixtureDef();
    def->setManufacturer(KXMLFixtureGeneric);
    def->setModel(KXMLFixtureGeneric);
    def->setType(QLCFixtureDef::Dimmer);
    def->setAuthor("QLC+");

    for (int i = 0; i < channels; i++)
    {
        QLCChannel *intensity = new QLCChannel();
        intensity->setGroup(QLCChannel::Intensity);
        intensity->setName(tr("Dimmer #%1").arg(i + 1));
        intensity->addCapability(new QLCCapability(0, UCHAR_MAX, tr("Intensity")));
        def->addChannel(intensity);
    }

    return def;
}

QLCFixtureMode *Fixture::genericDimmerMode(QLCFixtureDef *def, int channels)
{
    Q_ASSERT(def != NULL);
    QLCFixtureMode *mode = new QLCFixtureMode(def);

    mode->setName(QString("%1 Channel").arg(channels));
    QList<QLCChannel *>chList = def->channels();
    for (int i = 0; i < chList.count(); i++)
    {
        QLCChannel *ch = chList.at(i);
        mode->insertChannel(ch, i);
        QLCFixtureHead head;
        head.addChannel(i);
        mode->insertHead(-1, head);
    }

    QLCPhysical physical;
    physical.setWidth(300 * channels);
    physical.setHeight(300);
    physical.setDepth(300);

    mode->setPhysical(physical);
    def->addMode(mode);

    return mode;
}

/*********************************************************************
 * Generic RGB panel
 *********************************************************************/

QString Fixture::componentsToString(Components comp, bool is16bit)
{
    QString compStr;

    switch (comp)
    {
        case BGR:
            compStr = "BGR";
        break;
        case BRG:
            compStr = "BRG";
        break;
        case GBR:
            compStr = "GBR";
        break;
        case GRB:
            compStr = "GRB";
        break;
        case RBG:
            compStr = "RBG";
        break;
        case RGBW:
            compStr = "RGBW";
        break;
        default:
            compStr = "RGB";
        break;
    }

    if (is16bit)
        compStr += " 16bit";

    return compStr;
}

Fixture::Components Fixture::stringToComponents(QString str, bool &is16bit)
{
    QStringList strToken = str.split(' ');
    is16bit = false;

    if (strToken.count() == 2)
    {
        if (strToken.at(1) == "16bit")
            is16bit = true;
    }

    if (strToken.at(0) == "BGR") return BGR;
    else if (strToken.at(0) == "BRG") return BRG;
    else if (strToken.at(0) == "GBR") return GBR;
    else if (strToken.at(0) == "GRB") return GRB;
    else if (strToken.at(0) == "RBG") return RBG;
    else if (strToken.at(0) == "RGBW") return RGBW;
    else return RGB;
}

QLCFixtureDef *Fixture::genericRGBPanelDef(int columns, Components components, bool is16bit)
{
    QLCFixtureDef *def = new QLCFixtureDef();
    def->setManufacturer(KXMLFixtureGeneric);
    def->setModel(KXMLFixtureRGBPanel);
    def->setType(QLCFixtureDef::LEDBarPixels);
    def->setAuthor("QLC+");

    for (int i = 0; i < columns; i++)
    {
        QLCChannel *red = new QLCChannel();
        red->setName(QString("Red %1").arg(i + 1));
        red->setGroup(QLCChannel::Intensity);
        red->setColour(QLCChannel::Red);

        QLCChannel *green = new QLCChannel();
        green->setName(QString("Green %1").arg(i + 1));
        green->setGroup(QLCChannel::Intensity);
        green->setColour(QLCChannel::Green);

        QLCChannel *blue = new QLCChannel();
        blue->setName(QString("Blue %1").arg(i + 1));
        blue->setGroup(QLCChannel::Intensity);
        blue->setColour(QLCChannel::Blue);

        QLCChannel *redFine = NULL;
        QLCChannel *greenFine = NULL;
        QLCChannel *blueFine = NULL;

        if (is16bit)
        {
            redFine = new QLCChannel();
            redFine->setName(QString("Red Fine %1").arg(i + 1));
            redFine->setGroup(QLCChannel::Intensity);
            redFine->setColour(QLCChannel::Red);
            redFine->setControlByte(QLCChannel::LSB);

            greenFine = new QLCChannel();
            greenFine->setName(QString("Green Fine %1").arg(i + 1));
            greenFine->setGroup(QLCChannel::Intensity);
            greenFine->setColour(QLCChannel::Green);
            greenFine->setControlByte(QLCChannel::LSB);

            blueFine = new QLCChannel();
            blueFine->setName(QString("Blue Fine %1").arg(i + 1));
            blueFine->setGroup(QLCChannel::Intensity);
            blueFine->setColour(QLCChannel::Blue);
            blueFine->setControlByte(QLCChannel::LSB);
        }

        if (components == BGR)
        {
            def->addChannel(blue);
            if (is16bit) def->addChannel(blueFine);
            def->addChannel(green);
            if (is16bit) def->addChannel(greenFine);
            def->addChannel(red);
            if (is16bit) def->addChannel(redFine);
        }
        else if (components == BRG)
        {
            def->addChannel(blue);
            if (is16bit) def->addChannel(blueFine);
            def->addChannel(red);
            if (is16bit) def->addChannel(redFine);
            def->addChannel(green);
            if (is16bit) def->addChannel(greenFine);
        }
        else if (components == GBR)
        {
            def->addChannel(green);
            if (is16bit) def->addChannel(greenFine);
            def->addChannel(blue);
            if (is16bit) def->addChannel(blueFine);
            def->addChannel(red);
            if (is16bit) def->addChannel(redFine);
        }
        else if (components == GRB)
        {
            def->addChannel(green);
            if (is16bit) def->addChannel(greenFine);
            def->addChannel(red);
            if (is16bit) def->addChannel(redFine);
            def->addChannel(blue);
            if (is16bit) def->addChannel(blueFine);
        }
        else if (components == RBG)
        {
            def->addChannel(red);
            if (is16bit) def->addChannel(redFine);
            def->addChannel(blue);
            if (is16bit) def->addChannel(blueFine);
            def->addChannel(green);
            if (is16bit) def->addChannel(greenFine);
        }
        else if (components == RGBW)
        {
            QLCChannel *white = new QLCChannel();
            white->setName(QString("White %1").arg(i + 1));
            white->setGroup(QLCChannel::Intensity);
            white->setColour(QLCChannel::White);

            def->addChannel(red);
            if (is16bit) def->addChannel(redFine);
            def->addChannel(green);
            if (is16bit) def->addChannel(greenFine);
            def->addChannel(blue);
            if (is16bit) def->addChannel(blueFine);
            def->addChannel(white);

            if (is16bit)
            {
                QLCChannel *whiteFine = new QLCChannel();
                whiteFine->setName(QString("White Fine %1").arg(i + 1));
                whiteFine->setGroup(QLCChannel::Intensity);
                whiteFine->setColour(QLCChannel::White);
                whiteFine->setControlByte(QLCChannel::LSB);
                def->addChannel(whiteFine);
            }
        }
        else
        {
            def->addChannel(red);
            if (is16bit) def->addChannel(redFine);
            def->addChannel(green);
            if (is16bit) def->addChannel(greenFine);
            def->addChannel(blue);
            if (is16bit) def->addChannel(blueFine);
        }
    }

    return def;
}

QLCFixtureMode *Fixture::genericRGBPanelMode(QLCFixtureDef *def, Components components, bool is16bit,
                                             quint32 width, quint32 height)
{
    Q_ASSERT(def != NULL);

    QLCFixtureMode *mode = new QLCFixtureMode(def);
    QString modeName = componentsToString(components, is16bit);
    mode->setName(modeName);

    int compNum = components == RGBW ? 4 : 3;
    if (is16bit)
        compNum *= 2;

    QList<QLCChannel *>channels = def->channels();
    int i = 0;

    // add channels and heads
    for (int h = 0; h < channels.count() / compNum; h++)
    {
        QLCFixtureHead head;

        for (int c = 0; c < compNum; c++, i++)
        {
            QLCChannel *ch = channels.at(i);
            mode->insertChannel(ch, i);
            head.addChannel(i);
        }

        mode->insertHead(-1, head);
    }

    QLCPhysical physical;
    physical.setWidth(width);
    physical.setHeight(height);
    physical.setDepth(height);
    physical.setLayoutSize(QSize(mode->heads().count(), 1));

    mode->setPhysical(physical);
    def->addMode(mode);

    return mode;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Fixture::loader(QXmlStreamReader &root, Doc* doc)
{
    bool result = false;

    Fixture* fxi = new Fixture(doc);
    Q_ASSERT(fxi != NULL);

    if (fxi->loadXML(root, doc, doc->fixtureDefCache()) == true)
    {
        if (doc->addFixture(fxi, fxi->id(), fxi->crossUniverse()) == true)
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

bool Fixture::loadXML(QXmlStreamReader &xmlDoc, Doc *doc,
                      QLCFixtureDefCache *fixtureDefCache)
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
    quint32 width = 0, height = 0;
    QList<int> excludeList;
    QList<int> forcedHTP;
    QList<int> forcedLTP;
    QList<quint32>modifierIndices;
    QList<ChannelModifier *>modifierPointers;

    if (xmlDoc.name() != KXMLFixture)
    {
        qWarning() << Q_FUNC_INFO << "Fixture node not found";
        return false;
    }

    while (xmlDoc.readNextStartElement())
    {
        if (xmlDoc.name() == KXMLQLCFixtureDefManufacturer)
        {
            manufacturer = xmlDoc.readElementText();
        }
        else if (xmlDoc.name() == KXMLQLCFixtureDefModel)
        {
            model = xmlDoc.readElementText();
        }
        else if (xmlDoc.name() == KXMLQLCFixtureMode)
        {
            modeName = xmlDoc.readElementText();
        }
        else if (xmlDoc.name() == KXMLQLCPhysicalDimensionsWidth)
        {
            width = xmlDoc.readElementText().toUInt();
        }
        else if (xmlDoc.name() == KXMLQLCPhysicalDimensionsHeight)
        {
            height = xmlDoc.readElementText().toUInt();
        }
        else if (xmlDoc.name() == KXMLFixtureID)
        {
            id = xmlDoc.readElementText().toUInt();
        }
        else if (xmlDoc.name() == KXMLFixtureName)
        {
            name = xmlDoc.readElementText();
        }
        else if (xmlDoc.name() == KXMLFixtureUniverse)
        {
            universe = xmlDoc.readElementText().toInt();
        }
        else if (xmlDoc.name() == KXMLFixtureCrossUniverse)
        {
            xmlDoc.readElementText();
            setCrossUniverse(true);
        }
        else if (xmlDoc.name() == KXMLFixtureAddress)
        {
            address = xmlDoc.readElementText().toInt();
        }
        else if (xmlDoc.name() == KXMLFixtureChannels)
        {
            channels = xmlDoc.readElementText().toInt();
        }
        else if (xmlDoc.name() == KXMLFixtureExcludeFade)
        {
            QString list = xmlDoc.readElementText();
            QStringList values = list.split(",");

            for (int i = 0; i < values.count(); i++)
                excludeList.append(values.at(i).toInt());
        }
        else if (xmlDoc.name() == KXMLFixtureForcedHTP)
        {
            QString list = xmlDoc.readElementText();
            QStringList values = list.split(",");

            for (int i = 0; i < values.count(); i++)
                forcedHTP.append(values.at(i).toInt());
        }
        else if (xmlDoc.name() == KXMLFixtureForcedLTP)
        {
            QString list = xmlDoc.readElementText();
            QStringList values = list.split(",");

            for (int i = 0; i < values.count(); i++)
                forcedLTP.append(values.at(i).toInt());
        }
        else if (xmlDoc.name() == KXMLFixtureChannelModifier)
        {
            QXmlStreamAttributes attrs = xmlDoc.attributes();
            if (attrs.hasAttribute(KXMLFixtureChannelIndex) &&
                attrs.hasAttribute(KXMLFixtureModifierName))
            {
                quint32 chIdx = attrs.value(KXMLFixtureChannelIndex).toString().toUInt();
                QString modName = attrs.value(KXMLFixtureModifierName).toString();
                ChannelModifier *chMod = doc->modifiersCache()->modifier(modName);
                if (chMod != NULL)
                {
                    modifierIndices.append(chIdx);
                    modifierPointers.append(chMod);
                }
                xmlDoc.skipCurrentElement();
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown fixture tag:" << xmlDoc.name();
            xmlDoc.skipCurrentElement();
        }
    }

    /* Find the given fixture definition, unless its a generic dimmer */
    if (model != KXMLFixtureGeneric && model != KXMLFixtureRGBPanel)
    {
        fixtureDef = fixtureDefCache->fixtureDef(manufacturer, model);
        if (fixtureDef == NULL)
        {
            // fallback to project local path
            QString man(manufacturer);
            QString mod(model);
            QString path = QString("%1%2%3-%4%5")
                    .arg(doc->getWorkspacePath()).arg(QDir::separator())
                    .arg(man.replace(" ", "-")).arg(mod.replace(" ", "-")).arg(KExtFixture);

            qDebug() << "Fixture not found. Fallback to:" << path;

            if (fixtureDefCache->loadQXF(path, true) == false)
                qDebug() << "Failed to load definition" << path;

            fixtureDef = fixtureDefCache->fixtureDef(manufacturer, model);
            if (fixtureDef == NULL)
            {
                doc->appendToErrorLog(QString("No fixture definition found for <b>%1</b> <b>%2</b>")
                                      .arg(manufacturer).arg(model));
            }
        }

        if (fixtureDef != NULL)
        {
            /* Find the given fixture mode */
            fixtureMode = fixtureDef->mode(modeName);
            if (fixtureMode == NULL)
            {
                doc->appendToErrorLog(QString("Fixture mode <b>%1</b> not found for <b>%2</b> <b>%3</b>")
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
        doc->appendToErrorLog(QString("%1 channels of fixture <b>%2</b> are out of bounds")
                              .arg(QString::number(channels))
                              .arg(name));
        channels = 1;
    }

    /* Make sure that address is something sensible */
    if (!crossUniverse() && (address > 511 || address + (channels - 1) > 511))
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

    if (model == KXMLFixtureGeneric)
    {
        fixtureDef = genericDimmerDef(channels);
        fixtureMode = genericDimmerMode(fixtureDef, channels);
    }
    else if (model == KXMLFixtureRGBPanel)
    {
        bool is16bit = false;
        Components components = stringToComponents(modeName, is16bit);
        int compNum = components == RGBW ? 4 : 3;
        if (is16bit)
            compNum *= 2;

        fixtureDef = genericRGBPanelDef(channels / compNum, components, is16bit);
        fixtureMode = genericRGBPanelMode(fixtureDef, components, is16bit, width, height);
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
    setForcedHTPChannels(forcedHTP);
    setForcedLTPChannels(forcedLTP);
    for (int i = 0; i < modifierIndices.count(); i++)
        setChannelModifier(modifierIndices.at(i), modifierPointers.at(i));
    setID(id);

    return true;
}

bool Fixture::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    /* Fixture Instance entry */
    doc->writeStartElement(KXMLFixture);

    /* Manufacturer */
    if (m_fixtureDef != NULL)
        doc->writeTextElement(KXMLQLCFixtureDefManufacturer, m_fixtureDef->manufacturer());
    else
        doc->writeTextElement(KXMLQLCFixtureDefManufacturer, KXMLFixtureGeneric);

    /* Model */
    if (m_fixtureDef != NULL)
        doc->writeTextElement(KXMLQLCFixtureDefModel, m_fixtureDef->model());
    else
        doc->writeTextElement(KXMLQLCFixtureDefModel, KXMLFixtureGeneric);

    /* Fixture mode */
    if (m_fixtureMode != NULL)
        doc->writeTextElement(KXMLQLCFixtureMode, m_fixtureMode->name());
    else
        doc->writeTextElement(KXMLQLCFixtureMode, KXMLFixtureGeneric);

    /* RGB Panel physical dimensions */
    if (m_fixtureDef != NULL && m_fixtureDef->model() == KXMLFixtureRGBPanel && m_fixtureMode != NULL)
    {
        doc->writeTextElement(KXMLQLCPhysicalDimensionsWidth,
                              QString::number(m_fixtureMode->physical().width()));

        doc->writeTextElement(KXMLQLCPhysicalDimensionsHeight,
                              QString::number(m_fixtureMode->physical().height()));
    }

    /* ID */
    doc->writeTextElement(KXMLFixtureID, QString::number(id()));
    /* Name */
    doc->writeTextElement(KXMLFixtureName, m_name);
    /* Universe */
    doc->writeTextElement(KXMLFixtureUniverse, QString::number(universe()));
    if (crossUniverse())
        doc->writeTextElement(KXMLFixtureCrossUniverse, KXMLQLCTrue);
    /* Address */
    doc->writeTextElement(KXMLFixtureAddress, QString::number(address()));
    /* Channel count */
    doc->writeTextElement(KXMLFixtureChannels, QString::number(channels()));

    if (m_excludeFadeIndices.count() > 0)
    {
        QString list;
        for (int i = 0; i < m_excludeFadeIndices.count(); i++)
        {
            if (list.isEmpty() == false)
                list.append(QString(","));
            list.append(QString("%1").arg(m_excludeFadeIndices.at(i)));
        }
        doc->writeTextElement(KXMLFixtureExcludeFade, list);
    }

    if (m_forcedHTPIndices.count() > 0)
    {
        QString list;
        for (int i = 0; i < m_forcedHTPIndices.count(); i++)
        {
            if (list.isEmpty() == false)
                list.append(QString(","));
            list.append(QString("%1").arg(m_forcedHTPIndices.at(i)));
        }
        doc->writeTextElement(KXMLFixtureForcedHTP, list);
    }

    if (m_forcedLTPIndices.count() > 0)
    {
        QString list;
        for (int i = 0; i < m_forcedLTPIndices.count(); i++)
        {
            if (list.isEmpty() == false)
                list.append(QString(","));
            list.append(QString("%1").arg(m_forcedLTPIndices.at(i)));
        }
        doc->writeTextElement(KXMLFixtureForcedLTP, list);
    }

    if (m_channelModifiers.isEmpty() == false)
    {
        QHashIterator<quint32, ChannelModifier *> it(m_channelModifiers);
        while (it.hasNext())
        {
            it.next();
            quint32 ch = it.key();
            ChannelModifier *mod = it.value();
            if (mod != NULL)
            {
                doc->writeStartElement(KXMLFixtureChannelModifier);
                doc->writeAttribute(KXMLFixtureChannelIndex, QString::number(ch));
                doc->writeAttribute(KXMLFixtureModifierName, mod->name());
                doc->writeEndElement();
            }
        }
    }

    /* End the <Fixture> tag */
    doc->writeEndElement();

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

    if (m_fixtureDef != NULL && m_fixtureMode != NULL)
    {
        // Manufacturer
        info += genInfo.arg(tr("Manufacturer")).arg(m_fixtureDef->manufacturer());
        info += genInfo.arg(tr("Model")).arg(m_fixtureDef->model());
        info += genInfo.arg(tr("Mode")).arg(m_fixtureMode->name());
        info += genInfo.arg(tr("Type")).arg(m_fixtureDef->typeToString(m_fixtureDef->type()));
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
     * Extended device information
     ********************************************************************/

    if (m_fixtureMode != NULL)
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
        QString frange("%1&deg;");
        info += subTitle.arg(tr("Head(s)"));
        info += genInfo.arg(tr("Type")).arg(physical.focusType());
        info += genInfo.arg(tr("Pan Range")).arg(frange.arg(physical.focusPanMax()));
        info += genInfo.arg(tr("Tilt Range")).arg(frange.arg(physical.focusTiltMax()));
        if (physical.layoutSize() != QSize(1, 1))
        {
            info += genInfo.arg(tr("Layout"))
                           .arg(QString("%1 x %2").arg(physical.layoutSize().width()).arg(physical.layoutSize().height()));
        }
    }

    // HTML document & table closure
    info += "</TABLE>";

    if (m_fixtureDef != NULL)
    {
        info += "<HR>";
        info += "<DIV CLASS='author' ALIGN='right'>";
        info += tr("Fixture definition author: ") + m_fixtureDef->author();
        info += "</DIV>";
    }

    return info;
}
