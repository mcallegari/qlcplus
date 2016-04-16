/*
  Q Light Controller Plus
  vcxypadfixture.cpp

  Copyright (c) Heikki Junnila
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
#include <QStringList>
#include <QVariant>
#include <QString>
#include <QDebug>
#include <math.h>

#include "qlcfixturemode.h"
#include "qlcchannel.h"
#include "qlcmacros.h"
#include "qlcfile.h"

#include "vcxypadfixture.h"
#include "universe.h"
#include "fixture.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCXYPadFixture::VCXYPadFixture(Doc* doc)
    : m_doc(doc)
    , m_head()
{
    Q_ASSERT(m_doc != NULL);

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

    precompute();

    m_enabled = true;
    m_displayMode = Degrees;
}

VCXYPadFixture::VCXYPadFixture(Doc* doc, const QVariant& variant)
    : m_doc(doc)
{
    Q_ASSERT(m_doc != NULL);

    if (variant.canConvert(QVariant::StringList) == true)
    {
        QStringList list(variant.toStringList());
        if (list.size() == 10)
        {
            m_head.fxi = list.takeFirst().toUInt();
            m_head.head = list.takeFirst().toInt();

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

            m_xMSB = QLCChannel::invalid();
            m_xLSB = QLCChannel::invalid();
            m_yMSB = QLCChannel::invalid();
            m_yLSB = QLCChannel::invalid();

            precompute();

            m_enabled = bool(list.takeFirst().toInt());
            m_displayMode = DisplayMode(list.takeFirst().toInt());
            Q_ASSERT(list.isEmpty() == true);
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

    m_head = fxi.m_head;

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

    precompute();

    m_enabled = fxi.m_enabled;
    m_displayMode = fxi.m_displayMode;

    return *this;
}

bool VCXYPadFixture::operator==(const VCXYPadFixture& fxi) const
{
    if (m_head == fxi.m_head)
        return true;
    else
        return false;
}

VCXYPadFixture::operator QVariant() const
{
    QStringList list;

    list << QString("%1").arg(m_head.fxi);
    list << QString("%1").arg(m_head.head);

    list << QString("%1").arg(m_xMin);
    list << QString("%1").arg(m_xMax);
    list << QString("%1").arg(m_xReverse);

    list << QString("%1").arg(m_yMin);
    list << QString("%1").arg(m_yMax);
    list << QString("%1").arg(m_yReverse);

    list << QString("%1").arg(m_enabled);
    list << QString("%1").arg(m_displayMode);

    return QVariant(list);
}

/****************************************************************************
 * Fixture
 ****************************************************************************/

void VCXYPadFixture::setHead(GroupHead const & head)
{
    m_head = head;
}

GroupHead const & VCXYPadFixture::head() const
{
    return m_head;
}

QRectF VCXYPadFixture::degreesRange() const
{
    Fixture* fxi = m_doc->fixture(m_head.fxi);
    if (fxi == NULL)
    {
        return QRectF();
    }
    else
    {
       return fxi->degreesRange(m_head.head);
    }
}

QString VCXYPadFixture::name() const
{
    if (!m_head.isValid())
        return QString();

    Fixture* fxi = m_doc->fixture(m_head.fxi);
    if (fxi == NULL)
        return QString();

    if (m_head.head >= fxi->heads())
        return QString();

    if (fxi->heads() == 1)
        return fxi->name();

    return QString("%1 [%2]").arg(fxi->name()).arg(m_head.head);
}

/****************************************************************************
 * X-Axis
 ****************************************************************************/

void VCXYPadFixture::setX(qreal min, qreal max, bool reverse)
{
    m_xMin = CLAMP(min, 0.0, 1.0);
    m_xMax = CLAMP(max, 0.0, 1.0);
    m_xReverse = reverse;
    precompute();
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
    qreal scale = 100.0;
    QString units = "%";

    if (m_displayMode == DMX)
    {
        scale = 255.0;
        units = "";
    }
    else if (m_displayMode == Degrees)
    {
        scale = degreesRange().width();
        units = "°";
    }

    if (m_xReverse == false)
        return QString("%1%3 - %2%3").arg(qRound(m_xMin * scale)).arg(qRound(m_xMax * scale)).arg(units);
    else
        return QString("%1: %2%4 - %3%4").arg(QObject::tr("Reversed"))
                                      .arg(qRound(m_xMax * scale)).arg(qRound(m_xMin * scale)).arg(units);

}

void VCXYPadFixture::precompute()
{
    if (m_xReverse)
    {
        m_xOffset = m_xMax * qreal(USHRT_MAX);
        m_xRange = (m_xMin - m_xMax) * qreal(USHRT_MAX);
    }
    else
    {
        m_xOffset = m_xMin * qreal(USHRT_MAX);
        m_xRange = (m_xMax - m_xMin) * qreal(USHRT_MAX);
    }

    if (m_yReverse)
    {
        m_yOffset = m_yMax *qreal(USHRT_MAX);
        m_yRange = (m_yMin - m_yMax) * qreal(USHRT_MAX);
    }
    else
    {
        m_yOffset = m_yMin * qreal(USHRT_MAX);
        m_yRange = (m_yMax - m_yMin) * qreal(USHRT_MAX);
    }
}

/****************************************************************************
 * Y-Axis
 ****************************************************************************/

void VCXYPadFixture::setY(qreal min, qreal max, bool reverse)
{
    m_yMin = CLAMP(min, 0.0, 1.0);
    m_yMax = CLAMP(max, 0.0, 1.0);
    m_yReverse = reverse;
    precompute();
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
    qreal scale = 100.0;
    QString units = "%";

    if (m_displayMode == DMX)
    {
        scale = 255.0;
        units = "";
    }
    else if (m_displayMode == Degrees)
    {
        scale = degreesRange().height();
        units = "°";
    }

    if (m_yReverse == false)
        return QString("%1%3 - %2%3").arg(qRound(m_yMin * scale)).arg(qRound(m_yMax * scale)).arg(units);
    else
        return QString("%1: %2%4 - %3%4").arg(QObject::tr("Reversed"))
                .arg(qRound(m_yMax * scale)).arg(qRound(m_yMin * scale)).arg(units);
}

/********************************************************************
 * Display mode
 ********************************************************************/

void VCXYPadFixture::setDisplayMode(VCXYPadFixture::DisplayMode mode)
{
    m_displayMode = mode;
}

VCXYPadFixture::DisplayMode VCXYPadFixture::displayMode() const
{
    return m_displayMode;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool VCXYPadFixture::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCXYPadFixture)
    {
        qWarning() << Q_FUNC_INFO << "XYPad Fixture node not found";
        return false;
    }

    /* Fixture ID */
    GroupHead head;
    head.fxi = root.attributes().value(KXMLQLCVCXYPadFixtureID).toString().toInt();
    head.head = root.attributes().value(KXMLQLCVCXYPadFixtureHead).toString().toInt();
    setHead(head);

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVCXYPadFixtureAxis)
        {
            QXmlStreamAttributes attrs = root.attributes();
            QString axis = attrs.value(KXMLQLCVCXYPadFixtureAxisID).toString();
            QString min = attrs.value(KXMLQLCVCXYPadFixtureAxisLowLimit).toString();
            QString max = attrs.value(KXMLQLCVCXYPadFixtureAxisHighLimit).toString();
            QString rev = attrs.value(KXMLQLCVCXYPadFixtureAxisReverse).toString();

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
            root.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown XY Pad tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCXYPadFixture::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    /* VCXYPad Fixture */
    doc->writeStartElement(KXMLQLCVCXYPadFixture);
    doc->writeAttribute(KXMLQLCVCXYPadFixtureID, QString("%1").arg(m_head.fxi));
    doc->writeAttribute(KXMLQLCVCXYPadFixtureHead, QString("%1").arg(m_head.head));

    /* X-Axis */
    doc->writeStartElement(KXMLQLCVCXYPadFixtureAxis);
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisID, KXMLQLCVCXYPadFixtureAxisX);
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisLowLimit, QString("%1").arg(m_xMin));
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisHighLimit, QString("%1").arg(m_xMax));
    if (m_xReverse == true)
        doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisReverse, KXMLQLCTrue);
    else
        doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisReverse, KXMLQLCFalse);
    doc->writeEndElement();

    /* Y-Axis */
    doc->writeStartElement(KXMLQLCVCXYPadFixtureAxis);
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisID, KXMLQLCVCXYPadFixtureAxisY);
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisLowLimit, QString("%1").arg(m_yMin));
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisHighLimit, QString("%1").arg(m_yMax));
    if (m_yReverse == true)
        doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisReverse, KXMLQLCTrue);
    else
        doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisReverse, KXMLQLCFalse);
    doc->writeEndElement();

    /* End the <Fixture> tag */
    doc->writeEndElement();

    return true;
}

/****************************************************************************
 * Running
 ****************************************************************************/

void VCXYPadFixture::arm()
{
    Fixture* fxi = m_doc->fixture(m_head.fxi);
    if (fxi == NULL)
    {
        m_xMSB = QLCChannel::invalid();
        m_xLSB = QLCChannel::invalid();
        m_yMSB = QLCChannel::invalid();
        m_yLSB = QLCChannel::invalid();
    }
    else
    {
        m_xMSB = fxi->panMsbChannel(m_head.head);
        if (m_xMSB != QLCChannel::invalid() )
            m_xMSB += fxi->universeAddress();

        m_xLSB = fxi->panLsbChannel(m_head.head);
        if (m_xLSB != QLCChannel::invalid() )
            m_xLSB += fxi->universeAddress();

        m_yMSB = fxi->tiltMsbChannel(m_head.head);
        if (m_yMSB != QLCChannel::invalid() )
            m_yMSB += fxi->universeAddress();

        m_yLSB = fxi->tiltLsbChannel(m_head.head);
        if (m_yLSB != QLCChannel::invalid() )
            m_yLSB += fxi->universeAddress();
    }
}

void VCXYPadFixture::disarm()
{
    m_xLSB = QLCChannel::invalid();
    m_xMSB = QLCChannel::invalid();
    m_yLSB = QLCChannel::invalid();
    m_yMSB = QLCChannel::invalid();
}

void VCXYPadFixture::setEnabled(bool enable)
{
    m_enabled = enable;
}

bool VCXYPadFixture::isEnabled() const
{
    return m_enabled;
}

void VCXYPadFixture::writeDMX(qreal xmul, qreal ymul, QList<Universe *> universes)
{
    if (m_xMSB == QLCChannel::invalid() || m_yMSB == QLCChannel::invalid())
        return;

    ushort x = floor(m_xRange * xmul + m_xOffset + 0.5);
    ushort y = floor(m_yRange * ymul + m_yOffset + 0.5);

    quint32 address = m_xMSB & 0x01FF;
    int uni = m_xMSB >> 9;
    if (uni < universes.count())
        universes[uni]->write(address, char(x >> 8));

    address = m_yMSB & 0x01FF;
    uni = m_yMSB >> 9;
    if (uni < universes.count())
        universes[uni]->write(address, char(y >> 8));

    if (m_xLSB != QLCChannel::invalid() && m_yLSB != QLCChannel::invalid())
    {
        address = m_xLSB & 0x01FF;
        uni = m_xLSB >> 9;
        if (uni < universes.count())
            universes[uni]->write(address, char(x & 0xFF));

        address = m_yLSB & 0x01FF;
        uni = m_yLSB >> 9;
        if (uni < universes.count())
            universes[uni]->write(address, char(y & 0xFF));
    }
}

void VCXYPadFixture::readDMX(QList<Universe*> universes, qreal & xmul, qreal & ymul)
{
    xmul = -1;
    ymul = -1;

    if (m_xMSB == QLCChannel::invalid() || m_yMSB == QLCChannel::invalid())
        return;

    qreal x = 0;
    qreal y = 0;

    quint32 address = m_xMSB & 0x01FF;
    int uni = m_xMSB >> 9;
    if (uni < universes.count())
        x = universes[uni]->preGMValue(address) * 256;

    address = m_yMSB & 0x01FF;
    uni = m_yMSB >> 9;
    if (uni < universes.count())
        y = universes[uni]->preGMValue(address) * 256;

    if (m_xLSB != QLCChannel::invalid() && m_yLSB != QLCChannel::invalid())
    {
        address = m_xLSB & 0x01FF;
        uni = m_xLSB >> 9;
        if (uni < universes.count())
            x += universes[uni]->preGMValue(address);

        address = m_yLSB & 0x01FF;
        uni = m_yLSB >> 9;
        if (uni < universes.count())
            y += universes[uni]->preGMValue(address);
    }

    if (m_xRange == 0 || m_yRange == 0)
    {
        Q_ASSERT(m_xRange != 0);
        Q_ASSERT(m_yRange != 0);
        return; // potential divide by zero!
    }

    x = (x - m_xOffset) / m_xRange;
    y = (y - m_yOffset) / m_yRange;

    x = CLAMP(x, qreal(0), qreal(1));
    y = CLAMP(y, qreal(0), qreal(1));

    xmul = x;
    ymul = y;
}
