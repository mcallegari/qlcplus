/*
  Q Light Controller
  vcxypadfixture.cpp

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

#include <QStringList>
#include <QVariant>
#include <QString>
#include <QtXml>

#include "qlcfixturemode.h"
#include "qlcchannel.h"
#include "qlcmacros.h"
#include "qlcfile.h"

#include "vcxypadfixture.h"
#include "universearray.h"
#include "fixture.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCXYPadFixture::VCXYPadFixture(Doc* doc)
    : m_doc(doc)
{
    Q_ASSERT(m_doc != NULL);

    m_fixture = Fixture::invalidId();

    m_xMin = 0;
    m_xMax = 1;
    m_xReverse = false;

    m_yMin = 0;
    m_yMax = 1;
    m_yReverse = false;

    m_xLSB = QLCChannel::invalid();
    m_xMSB = QLCChannel::invalid();
    m_yLSB = QLCChannel::invalid();
    m_yMSB = QLCChannel::invalid();
}

VCXYPadFixture::VCXYPadFixture(Doc* doc, const QVariant& variant)
    : m_doc(doc)
{
    Q_ASSERT(m_doc != NULL);

    if (variant.canConvert(QVariant::StringList) == true)
    {
        QStringList list(variant.toStringList());
        if (list.size() == 7)
        {
            m_fixture = list.takeFirst().toUInt();

            m_xMin = qreal(list.takeFirst().toDouble());
            m_xMin = CLAMP(m_xMin, qreal(0), qreal(1));
            m_xMax = qreal(list.takeFirst().toDouble());
            m_xMax = CLAMP(m_xMax, qreal(0), qreal(1));
            m_xReverse = bool(list.takeFirst().toInt());

            m_yMin = qreal(list.takeFirst().toDouble());
            m_yMin = CLAMP(m_yMin, qreal(0), qreal(1));
            m_yMax = qreal(list.takeFirst().toDouble());
            m_yMax = CLAMP(m_yMax, qreal(0), qreal(1));
            m_yReverse = bool(list.takeFirst().toInt());
            Q_ASSERT(list.isEmpty() == true);

            m_xMSB = QLCChannel::invalid();
            m_xLSB = QLCChannel::invalid();
            m_yMSB = QLCChannel::invalid();
            m_yLSB = QLCChannel::invalid();
        }
        else
        {
            /* Construct an empty fixture */
            *this = VCXYPadFixture(doc);
        }
    }
    else
    {
        /* Construct an empty fixture */
        *this = VCXYPadFixture(doc);
    }
}

VCXYPadFixture::~VCXYPadFixture()
{
}

VCXYPadFixture& VCXYPadFixture::operator=(const VCXYPadFixture& fxi)
{
    m_doc = fxi.m_doc;
    Q_ASSERT(m_doc != NULL);

    m_fixture = fxi.m_fixture;

    m_xMin = fxi.m_xMin;
    m_xMax = fxi.m_xMax;
    m_xReverse = fxi.m_xReverse;

    m_yMin = fxi.m_yMin;
    m_yMax = fxi.m_yMax;
    m_yReverse = fxi.m_yReverse;

    m_xMSB = fxi.m_xMSB;
    m_xLSB = fxi.m_xLSB;

    m_yMSB = fxi.m_yMSB;
    m_yLSB = fxi.m_yLSB;

    return *this;
}

bool VCXYPadFixture::operator==(const VCXYPadFixture& fxi) const
{
    if (m_fixture == fxi.m_fixture)
        return true;
    else
        return false;
}

VCXYPadFixture::operator QVariant() const
{
    QStringList list;

    list << QString("%1").arg(m_fixture);

    list << QString("%1").arg(m_xMin);
    list << QString("%1").arg(m_xMax);
    list << QString("%1").arg(m_xReverse);

    list << QString("%1").arg(m_yMin);
    list << QString("%1").arg(m_yMax);
    list << QString("%1").arg(m_yReverse);

    return QVariant(list);
}

/****************************************************************************
 * Fixture
 ****************************************************************************/

void VCXYPadFixture::setFixture(quint32 fxi_id)
{
    m_fixture = fxi_id;
}

quint32 VCXYPadFixture::fixture() const
{
    return m_fixture;
}

QString VCXYPadFixture::name() const
{
    if (m_fixture == Fixture::invalidId())
        return QString();

    Fixture* fxi = m_doc->fixture(m_fixture);
    if (fxi != NULL)
        return fxi->name();
    else
        return QString();
}

/****************************************************************************
 * X-Axis
 ****************************************************************************/

void VCXYPadFixture::setX(qreal min, qreal max, bool reverse)
{
    m_xMin = CLAMP(min, 0.0, 1.0);
    m_xMax = CLAMP(max, 0.0, 1.0);
    m_xReverse = reverse;
}

qreal VCXYPadFixture::xMin() const
{
    return m_xMin;
}

qreal VCXYPadFixture::xMax() const
{
    return m_xMax;
}

bool VCXYPadFixture::xReverse() const
{
    return m_xReverse;
}

QString VCXYPadFixture::xBrief() const
{
    if (m_xReverse == false)
        return QString("%1 - %2%").arg(m_xMin * 100).arg(m_xMax * 100);
    else
        return QString("%1: %2 - %3%").arg(QObject::tr("Reversed"))
                                      .arg(m_xMax * 100).arg(m_xMin * 100);
}

/****************************************************************************
 * Y-Axis
 ****************************************************************************/

void VCXYPadFixture::setY(qreal min, qreal max, bool reverse)
{
    m_yMin = CLAMP(min, 0.0, 1.0);
    m_yMax = CLAMP(max, 0.0, 1.0);
    m_yReverse = reverse;
}

qreal VCXYPadFixture::yMin() const
{
    return m_yMin;
}

qreal VCXYPadFixture::yMax() const
{
    return m_yMax;
}

bool VCXYPadFixture::yReverse() const
{
    return m_yReverse;
}

QString VCXYPadFixture::yBrief() const
{
    if (m_yReverse == false)
        return QString("%1 - %2%").arg(m_yMin * 100).arg(m_yMax * 100);
    else
        return QString("%1: %2 - %3%").arg(QObject::tr("Reversed"))
                                      .arg(m_yMax * 100).arg(m_yMin * 100);
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool VCXYPadFixture::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCVCXYPadFixture)
    {
        qWarning() << Q_FUNC_INFO << "XYPad Fixture node not found";
        return false;
    }

    /* Fixture ID */
    setFixture(root.attribute(KXMLQLCVCXYPadFixtureID).toInt());

    /* Children */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCVCXYPadFixtureAxis)
        {
            QString axis = tag.attribute(KXMLQLCVCXYPadFixtureAxisID);
            QString min = tag.attribute(KXMLQLCVCXYPadFixtureAxisLowLimit);
            QString max = tag.attribute(KXMLQLCVCXYPadFixtureAxisHighLimit);
            QString rev = tag.attribute(KXMLQLCVCXYPadFixtureAxisReverse);

            if (axis == KXMLQLCVCXYPadFixtureAxisX)
            {
                if (rev == KXMLQLCTrue)
                    setX(min.toDouble(), max.toDouble(), true);
                else
                    setX(min.toDouble(), max.toDouble(), false);
            }
            else if (axis == KXMLQLCVCXYPadFixtureAxisY)
            {
                if (rev == KXMLQLCTrue)
                    setY(min.toDouble(), max.toDouble(), true);
                else
                    setY(min.toDouble(), max.toDouble(), false);
            }
            else
            {
                qWarning() << Q_FUNC_INFO << "Unknown XYPad axis" << axis;
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown XY Pad tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool VCXYPadFixture::saveXML(QDomDocument* doc, QDomElement* pad_root) const
{
    QDomElement root;
    QDomElement tag;
    QDomText text;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(pad_root != NULL);

    /* VCXYPad Fixture */
    root = doc->createElement(KXMLQLCVCXYPadFixture);
    root.setAttribute(KXMLQLCVCXYPadFixtureID, QString("%1").arg(m_fixture));
    root.appendChild(text);
    pad_root->appendChild(root);

    /* X-Axis */
    tag = doc->createElement(KXMLQLCVCXYPadFixtureAxis);
    root.appendChild(tag);
    tag.setAttribute(KXMLQLCVCXYPadFixtureAxisID, KXMLQLCVCXYPadFixtureAxisX);
    tag.setAttribute(KXMLQLCVCXYPadFixtureAxisLowLimit, QString("%1").arg(m_xMin));
    tag.setAttribute(KXMLQLCVCXYPadFixtureAxisHighLimit, QString("%1").arg(m_xMax));
    if (m_xReverse == true)
        tag.setAttribute(KXMLQLCVCXYPadFixtureAxisReverse, KXMLQLCTrue);
    else
        tag.setAttribute(KXMLQLCVCXYPadFixtureAxisReverse, KXMLQLCFalse);

    /* Y-Axis */
    tag = doc->createElement(KXMLQLCVCXYPadFixtureAxis);
    root.appendChild(tag);
    tag.setAttribute(KXMLQLCVCXYPadFixtureAxisID, KXMLQLCVCXYPadFixtureAxisY);
    tag.setAttribute(KXMLQLCVCXYPadFixtureAxisLowLimit, QString("%1").arg(m_yMin));
    tag.setAttribute(KXMLQLCVCXYPadFixtureAxisHighLimit, QString("%1").arg(m_yMax));
    if (m_yReverse == true)
        tag.setAttribute(KXMLQLCVCXYPadFixtureAxisReverse, KXMLQLCTrue);
    else
        tag.setAttribute(KXMLQLCVCXYPadFixtureAxisReverse, KXMLQLCFalse);

    return true;
}

/****************************************************************************
 * Running
 ****************************************************************************/

void VCXYPadFixture::arm()
{
    Fixture* fxi = m_doc->fixture(m_fixture);
    if (fxi == NULL)
    {
        m_xLSB = QLCChannel::invalid();
        m_xMSB = QLCChannel::invalid();
        m_yLSB = QLCChannel::invalid();
        m_yMSB = QLCChannel::invalid();
    }
    else
    {
        /* If this fixture has no mode, it's a generic dimmer that
           can't do pan&tilt anyway. */
        const QLCFixtureMode* mode = fxi->fixtureMode();
        if (mode == NULL)
        {
            m_xLSB = QLCChannel::invalid();
            m_xMSB = QLCChannel::invalid();
            m_yLSB = QLCChannel::invalid();
            m_yMSB = QLCChannel::invalid();

            return;
        }

        /* Find exact channel numbers for MSB/LSB pan and tilt */
        for (quint32 i = 0; i < quint32(mode->channels().size()); i++)
        {
            const QLCChannel* ch = mode->channel(i);
            Q_ASSERT(ch != NULL);

            if (ch->group() == QLCChannel::Pan)
            {
                if (ch->controlByte() == QLCChannel::MSB)
                    m_xMSB = fxi->universeAddress() + i;
                else if (ch->controlByte() == QLCChannel::LSB)
                    m_xLSB = fxi->universeAddress() + i;
            }
            else if (ch->group() == QLCChannel::Tilt)
            {
                if (ch->controlByte() == QLCChannel::MSB)
                    m_yMSB = fxi->universeAddress() + i;
                else if (ch->controlByte() == QLCChannel::LSB)
                    m_yLSB = fxi->universeAddress() + i;
            }
        }
    }
}

void VCXYPadFixture::disarm()
{
    m_xLSB = QLCChannel::invalid();
    m_xMSB = QLCChannel::invalid();
    m_yLSB = QLCChannel::invalid();
    m_yMSB = QLCChannel::invalid();
}

void VCXYPadFixture::writeDMX(qreal xmul, qreal ymul, UniverseArray* universes)
{
    Q_ASSERT(universes != NULL);

    if (m_xMSB == QLCChannel::invalid() || m_yMSB == QLCChannel::invalid())
        return;

    xmul = ((m_xMax - m_xMin) * xmul) + m_xMin;
    ymul = ((m_yMax - m_yMin) * ymul) + m_yMin;

    if (m_xReverse == true)
        xmul = m_xMax - xmul;
    if (m_yReverse == true)
        ymul = m_yMax - ymul;

    ushort x = floor((qreal(USHRT_MAX) * xmul) + 0.5);
    ushort y = floor((qreal(USHRT_MAX) * ymul) + 0.5);

    universes->write(m_xMSB, char(x >> 8), QLCChannel::Pan);
    universes->write(m_yMSB, char(y >> 8), QLCChannel::Tilt);

    if (m_xLSB != QLCChannel::invalid() && m_yLSB != QLCChannel::invalid())
    {
        universes->write(m_xLSB, char(x & 0xFF), QLCChannel::Pan);
        universes->write(m_yLSB, char(y & 0xFF), QLCChannel::Tilt);
    }
}
