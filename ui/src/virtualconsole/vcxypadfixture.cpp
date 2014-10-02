/*
  Q Light Controller
  vcxypadfixture.cpp

  Copyright (c) Heikki Junnila

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

#include <QStringList>
#include <QVariant>
#include <QString>
#include <QtXml>

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
}

VCXYPadFixture::VCXYPadFixture(Doc* doc, const QVariant& variant)
    : m_doc(doc)
{
    Q_ASSERT(m_doc != NULL);

    if (variant.canConvert(QVariant::StringList) == true)
    {
        QStringList list(variant.toStringList());
        if (list.size() == 8)
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
    GroupHead head;
    head.fxi = root.attribute(KXMLQLCVCXYPadFixtureID).toInt();
    head.head = root.attribute(KXMLQLCVCXYPadFixtureHead).toInt();
    setHead(head);

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
    root.setAttribute(KXMLQLCVCXYPadFixtureID, QString("%1").arg(m_head.fxi));
    root.setAttribute(KXMLQLCVCXYPadFixtureHead, QString("%1").arg(m_head.head));
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

void VCXYPadFixture::writeDMX(qreal xmul, qreal ymul, QList<Universe *> universes)
{
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

    x /= USHRT_MAX;    
    y /= USHRT_MAX;

    if (x < m_xMin || x > m_xMax || y < m_yMin || y > m_yMax) // out of range
        return;

    if (m_xReverse == true)
        x = m_xMax - x;
    if (m_yReverse == true)
        y = m_yMax - y;

    if (m_xMax == m_xMin || m_yMax == m_yMin) // avoid divide by zero below
        return;
        
    xmul = (x - m_xMin) / (m_xMax - m_xMin);
    ymul = (y - m_yMin) / (m_yMax - m_yMin);
}

