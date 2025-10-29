/*
  Q Light Controller Plus
  fixtureutils.cpp

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

#include <QDebug>

#include "monitorproperties.h"
#include "qlcfixturemode.h"
#include "qlccapability.h"
#include "fixtureutils.h"
#include "qlcmacros.h"
#include "fixture.h"
#include "doc.h"

#define FIXTURE_ID_BITS     16
#define FIXTURE_HEAD_BITS   8
#define FIXTURE_LINKED_BITS 8
#define MAX_FIXTURE_NUMBER  (1 << FIXTURE_ID_BITS)
#define MAX_HEADS_NUMBER    (1 << FIXTURE_HEAD_BITS)
#define MAX_LINKED_NUMBER   (1 << FIXTURE_LINKED_BITS)

#define MIN_STROBE_FREQ_HZ  0.5
#define MAX_STROBE_FREQ_HZ  10.0
#define MIN_PULSE_FREQ_HZ   0.25
#define MAX_PULSE_FREQ_HZ   5

#define MIN_POSITION_SPEED  4000 // ms
#define MAX_POSITION_SPEED  20000 // ms

#define MIN_GOBO_SPEED      500 // ms
#define MAX_GOBO_SPEED      5000 // ms

FixtureUtils::FixtureUtils()
{

}

quint32 FixtureUtils::fixtureItemID(quint32 fid, quint16 headIndex, quint16 linkedIndex)
{
    Q_ASSERT(fid < MAX_FIXTURE_NUMBER);
    Q_ASSERT(headIndex < MAX_HEADS_NUMBER);
    Q_ASSERT(linkedIndex < MAX_LINKED_NUMBER);

    return (fid << (FIXTURE_HEAD_BITS + FIXTURE_LINKED_BITS)) |
            ((quint32)headIndex << FIXTURE_LINKED_BITS) |
            (quint32)linkedIndex;
}

quint32 FixtureUtils::itemFixtureID(quint32 itemID)
{
    return (itemID >> (FIXTURE_HEAD_BITS + FIXTURE_LINKED_BITS));
}

quint16 FixtureUtils::itemHeadIndex(quint32 itemID)
{
    return ((itemID >> FIXTURE_LINKED_BITS) & ((1 << FIXTURE_HEAD_BITS) - 1));
}

quint16 FixtureUtils::itemLinkedIndex(quint32 itemID)
{
    return (itemID & ((1 << FIXTURE_LINKED_BITS) - 1));
}

QPointF FixtureUtils::item2DPosition(MonitorProperties *monProps, int pointOfView,
                                     QVector3D pos)
{
    QPointF point(0, 0);
    float gridUnits = monProps->gridUnits() == MonitorProperties::Meters ? 1000.0 : 304.8;

    switch(pointOfView)
    {
        case MonitorProperties::TopView:
            point.setX(pos.x());
            point.setY(pos.z());
        break;
        case MonitorProperties::Undefined:
        case MonitorProperties::FrontView:
            point.setX(pos.x());
            point.setY((monProps->gridSize().y() * gridUnits) - pos.y());
        break;
        case MonitorProperties::RightSideView:
            point.setX((monProps->gridSize().x() * gridUnits) - pos.z());
            point.setY((monProps->gridSize().y() * gridUnits) - pos.y());
        break;
        case MonitorProperties::LeftSideView:
            point.setX(pos.z());
            point.setY((monProps->gridSize().y() * gridUnits) - pos.y());
        break;
    }

    return point;
}


float FixtureUtils::item2DRotation(int pointOfView, QVector3D rot)
{
    switch(pointOfView)
    {
        case MonitorProperties::TopView:
            return rot.y();
        break;
        case MonitorProperties::RightSideView:
        case MonitorProperties::LeftSideView:
            return rot.x();
        break;
        default:
            return rot.z();
        break;
    }

    return 0;
}

QSizeF FixtureUtils::item2DDimension(QLCFixtureMode *fxMode, int pointOfView)
{
    QSizeF size(300, 300);

    if (fxMode == nullptr)
        return size;

    QLCPhysical phy = fxMode->physical();
    if (phy.width() == 0)
        phy.setWidth(300);
    if (phy.height() == 0)
        phy.setHeight(300);
    if (phy.depth() == 0)
        phy.setDepth(300);

    switch(pointOfView)
    {
        case MonitorProperties::TopView:
            size.setWidth(phy.width());
            size.setHeight(phy.depth());
        break;
        case MonitorProperties::Undefined:
        case MonitorProperties::FrontView:
            size.setWidth(phy.width());
            size.setHeight(phy.height());
        break;
        case MonitorProperties::RightSideView:
        case MonitorProperties::LeftSideView:
            size.setWidth(phy.depth());
            size.setHeight(phy.height());
        break;
    }

    return size;
}

void FixtureUtils::alignItem(QVector3D refPos, QVector3D &origPos, int pointOfView, int alignment)
{
    switch(pointOfView)
    {
        case MonitorProperties::TopView:
        {
            switch(alignment)
            {
                case Qt::AlignTop: origPos.setZ(refPos.z()); break;
                case Qt::AlignLeft: origPos.setX(refPos.x()); break;
            }
        }
        break;
        case MonitorProperties::Undefined:
        case MonitorProperties::FrontView:
        {
            switch(alignment)
            {
                case Qt::AlignTop: origPos.setY(refPos.y()); break;
                case Qt::AlignLeft: origPos.setX(refPos.x()); break;
            }
        }
        break;
        case MonitorProperties::RightSideView:
        case MonitorProperties::LeftSideView:
        {
            switch(alignment)
            {
                case Qt::AlignTop: origPos.setY(refPos.y()); break;
                case Qt::AlignLeft: origPos.setZ(refPos.z()); break;
            }
        }
        break;
    }
}

QVector3D FixtureUtils::item3DPosition(MonitorProperties *monProps, QPointF point, float thirdVal)
{
    QVector3D pos(point.x(), point.y(), thirdVal);

    switch(monProps->pointOfView())
    {
        case MonitorProperties::TopView:
            pos = QVector3D(point.x(), thirdVal, point.y());
        break;
        case MonitorProperties::RightSideView:
            pos = QVector3D(thirdVal, point.y(), monProps->gridSize().z() - point.x());
        break;
        case MonitorProperties::LeftSideView:
            pos = QVector3D(thirdVal, point.y(), point.x());
        break;
        default:
        break;
    }

    return pos;
}

QPointF FixtureUtils::available2DPosition(Doc *doc, int pointOfView, QRectF fxRect)
{
    MonitorProperties *monProps = doc->monitorProperties();
    if (monProps == nullptr)
        return QPointF(0, 0);

    qreal xPos = fxRect.x(), yPos = fxRect.y();
    qreal maxYOffset = 0;

    float gridUnits = monProps->gridUnits() == MonitorProperties::Meters ? 1000.0 : 304.8;
    QSize gridSize;

    switch (pointOfView)
    {
        case MonitorProperties::TopView:
            gridSize = QSize(monProps->gridSize().x(), monProps->gridSize().z());
        break;
        case MonitorProperties::RightSideView:
        case MonitorProperties::LeftSideView:
            gridSize = QSize(monProps->gridSize().z(), monProps->gridSize().y());
        break;
        default:
            gridSize = QSize(monProps->gridSize().x(), monProps->gridSize().y());
        break;
    }

    QRectF gridArea(0, 0, (float)gridSize.width() * gridUnits, (float)gridSize.height() * gridUnits);

    qreal origWidth = fxRect.width();
    qreal origHeight = fxRect.height();

    for (Fixture *fixture : doc->fixtures())
    {
        if (monProps->containsFixture(fixture->id()) == false)
            continue;

        QLCFixtureMode *fxMode = fixture->fixtureMode();

        for (quint32 subID : monProps->fixtureIDList(fixture->id()))
        {
            quint16 headIndex = monProps->fixtureHeadIndex(subID);
            quint16 linkedIndex = monProps->fixtureLinkedIndex(subID);
            QPointF fxPoint = item2DPosition(monProps, pointOfView,
                                             monProps->fixturePosition(fixture->id(), headIndex, linkedIndex));
            QSizeF fxSize = item2DDimension(fixture->type() == QLCFixtureDef::Dimmer ? nullptr : fxMode, pointOfView);
            qreal itemXPos = fxPoint.x();
            qreal itemYPos = fxPoint.y();
            qreal itemWidth = fxSize.width();
            qreal itemHeight = fxSize.height();

            // store the next Y row in case we need to lower down
            if (itemYPos + itemHeight > maxYOffset)
                maxYOffset = itemYPos + itemHeight;

            QRectF itemRect(itemXPos, itemYPos, itemWidth, itemHeight);

            //qDebug() << "item rect:" << itemRect << "fxRect:" << fxRect;

            if (fxRect.intersects(itemRect) == true)
            {
                xPos = itemXPos + itemWidth + 50; //add an extra 50mm spacing
                if (xPos + fxRect.width() > gridArea.width())
                {
                    xPos = 0;
                    yPos = maxYOffset + 50;
                    maxYOffset = 0;
                }
                fxRect.setX(xPos);
                fxRect.setY(yPos);
                // restore width and height as setX and setY mess them
                fxRect.setWidth(origWidth);
                fxRect.setHeight(origHeight);
            }
        }
    }

    return QPointF(xPos, yPos);
}

QColor FixtureUtils::blendColors(QColor a, QColor b, float mix)
{
    // linear blending - https://en.wikipedia.org/wiki/Alpha_compositing#Alpha_blending
    qreal mr = b.redF() * mix + a.redF() * (1 - mix);
    qreal mg = b.greenF() * mix + a.greenF() * (1 - mix);
    qreal mb = b.blueF() * mix + a.blueF() * (1 - mix);

/*
    // non linear blending
    qreal mr = qSqrt((1 - mix) * qPow(a.redF(), 2) + mix * qPow(b.redF(), 2));
    qreal mg = qSqrt((1 - mix) * qPow(a.greenF(), 2) + mix * qPow(b.greenF(), 2));
    qreal mb = qSqrt((1 - mix) * qPow(a.blueF(), 2) + mix * qPow(b.blueF(), 2));
*/
    return QColor(mr * 255.0, mg * 255.0, mb * 255.0);
}

QColor FixtureUtils::headColor(Fixture *fixture, bool hasDimmer, int headIndex)
{
    if (fixture == nullptr)
        return QColor();

    QColor finalColor = hasDimmer ? Qt::black : Qt::white;
    bool colorFound = false;

    QVector <quint32> rgbCh = fixture->rgbChannels(headIndex);
    if (rgbCh.size() == 3)
    {
        finalColor.setRgb(fixture->channelValueAt(rgbCh.at(0)),
                          fixture->channelValueAt(rgbCh.at(1)),
                          fixture->channelValueAt(rgbCh.at(2)));
        colorFound = true;
    }

    QVector <quint32> cmyCh = fixture->cmyChannels(headIndex);
    if (cmyCh.size() == 3)
    {
        finalColor.setCmyk(fixture->channelValueAt(cmyCh.at(0)),
                           fixture->channelValueAt(cmyCh.at(1)),
                           fixture->channelValueAt(cmyCh.at(2)), 0);
        colorFound = true;
    }

    quint32 white = fixture->channelNumber(QLCChannel::White, QLCChannel::MSB, headIndex);
    quint32 amber = fixture->channelNumber(QLCChannel::Amber, QLCChannel::MSB, headIndex);
    quint32 UV = fixture->channelNumber(QLCChannel::UV, QLCChannel::MSB, headIndex);
    quint32 lime = fixture->channelNumber(QLCChannel::Lime, QLCChannel::MSB, headIndex);
    quint32 indigo = fixture->channelNumber(QLCChannel::Indigo, QLCChannel::MSB, headIndex);

    if (white != QLCChannel::invalid() && fixture->channelValueAt(white))
        finalColor = blendColors(finalColor, Qt::white, (float)fixture->channelValueAt(white) / 255.0);

    if (amber != QLCChannel::invalid() && fixture->channelValueAt(amber))
        finalColor = blendColors(finalColor, QColor(0xFFFF7E00), (float)fixture->channelValueAt(amber) / 255.0);

    if (UV != QLCChannel::invalid() && fixture->channelValueAt(UV))
        finalColor = blendColors(finalColor, QColor(0xFF9400D3), (float)fixture->channelValueAt(UV) / 255.0);

    if (lime != QLCChannel::invalid() && fixture->channelValueAt(lime))
        finalColor = blendColors(finalColor, QColor(0xFFADFF2F), (float)fixture->channelValueAt(lime) / 255.0);

    if (indigo != QLCChannel::invalid() && fixture->channelValueAt(indigo))
        finalColor = blendColors(finalColor, QColor(0xFF4B0082), (float)fixture->channelValueAt(indigo) / 255.0);

    if (white != QLCChannel::invalid() || amber != QLCChannel::invalid() || UV != QLCChannel::invalid() ||
        lime != QLCChannel::invalid() || indigo != QLCChannel::invalid())
        colorFound = true;

    //qDebug() << "fixture" << fixture->name() << "head" << headIndex << "hasdimmer" << hasDimmer;

    if (colorFound == false)
        return Qt::white;

    return finalColor;
}

QColor FixtureUtils::applyColorFilter(QColor source, QColor filter)
{
    //qDebug() << "SOURCE" << source << "FILTER" << filter;
    return QColor(source.redF() * filter.redF() * 255.0,
                  source.greenF() * filter.greenF() * 255.0,
                  source.blueF() * filter.blueF() * 255.0);
}

void FixtureUtils::positionTimings(const QLCChannel *ch, uchar value, int &panDuration, int &tiltDuration)
{
    panDuration = -1;
    tiltDuration = -1;

    switch (ch->preset())
    {
        case QLCChannel::SpeedPanTiltFastSlow:
            panDuration = tiltDuration = SCALE(value, 0, 255, MIN_POSITION_SPEED, MAX_POSITION_SPEED);
        break;
        case QLCChannel::SpeedPanTiltSlowFast:
            panDuration = tiltDuration = SCALE(255 - value, 0, 255, MIN_POSITION_SPEED, MAX_POSITION_SPEED);
        break;
        case QLCChannel::SpeedPanFastSlow:
            panDuration = SCALE(value, 0, 255, MIN_POSITION_SPEED, MAX_POSITION_SPEED);
        break;
        case QLCChannel::SpeedPanSlowFast:
            panDuration = SCALE(255 - value, 0, 255, MIN_POSITION_SPEED, MAX_POSITION_SPEED);
        break;
        case QLCChannel::SpeedTiltFastSlow:
            tiltDuration = SCALE(value, 0, 255, MIN_POSITION_SPEED, MAX_POSITION_SPEED);
        break;
        case QLCChannel::SpeedTiltSlowFast:
            tiltDuration = SCALE(255 - value, 0, 255, MIN_POSITION_SPEED, MAX_POSITION_SPEED);
        break;
        default:
        break;
    }
}

bool FixtureUtils::goboTiming(const QLCCapability *cap, uchar value, int &speed)
{
    speed = MIN_GOBO_SPEED;
    bool clockwise = true;

    if (cap->preset() == QLCCapability::Custom)
    {
        speed = -1;
        return true;
    }

    if (cap->preset() != QLCCapability::RotationStop)
        value = SCALE(value, cap->min(), cap->max(), 1, 255);

    switch (cap->preset())
    {
        case QLCCapability::RotationClockwise:
            speed = MIN_GOBO_SPEED + ((MAX_GOBO_SPEED - MIN_GOBO_SPEED) / 2);
        break;
        case QLCCapability::RotationClockwiseFastToSlow:
            speed = SCALE(value, 0, 255, MIN_GOBO_SPEED, MAX_GOBO_SPEED);
        break;
        case QLCCapability::RotationClockwiseSlowToFast:
            speed = SCALE(255 - value, 0, 255, MIN_GOBO_SPEED, MAX_GOBO_SPEED);
        break;
        case QLCCapability::RotationStop:
            speed = 0;
        break;
        case QLCCapability::RotationCounterClockwise:
            speed = MIN_GOBO_SPEED + ((MAX_GOBO_SPEED - MIN_GOBO_SPEED) / 2);
            clockwise = false;
        break;
        case QLCCapability::RotationCounterClockwiseFastToSlow:
            speed = SCALE(value, 0, 255, MIN_GOBO_SPEED, MAX_GOBO_SPEED);
            clockwise = false;
        break;
        case QLCCapability::RotationCounterClockwiseSlowToFast:
            speed = SCALE(255 - value, 0, 255, MIN_GOBO_SPEED, MAX_GOBO_SPEED);
            clockwise = false;
        break;
        default:
            // not a handled/valid capability. Invalidate speed
            speed = -1;
        break;
    }

    return clockwise;
}

int FixtureUtils::shutterTimings(const QLCChannel *ch, uchar value, int &highTime, int &lowTime)
{
    int capPreset = QLCCapability::ShutterOpen;
    float freq = 1.0;

    switch (ch->preset())
    {
        case QLCChannel::ShutterStrobeSlowFast:
            if (value)
                capPreset = QLCCapability::StrobeSlowToFast;
        break;
        case QLCChannel::ShutterStrobeFastSlow:
            if (value)
            {
                capPreset = QLCCapability::StrobeFastToSlow;
                value = 255 - value;
            }
        break;
        default:
        {
            QLCCapability *cap = ch->searchCapability(value);
            if (cap == nullptr)
                break;

            capPreset = cap->preset();
            switch (capPreset)
            {
                case QLCCapability::ShutterOpen:
                case QLCCapability::ShutterClose:
                break;
                case QLCCapability::StrobeSlowToFast:
                case QLCCapability::PulseSlowToFast:
                case QLCCapability::RampUpSlowToFast:
                case QLCCapability::RampDownSlowToFast:
                    value = SCALE(value, cap->min(), cap->max(), 1, 255);
                break;
                case QLCCapability::StrobeFastToSlow:
                case QLCCapability::PulseFastToSlow:
                case QLCCapability::RampUpFastToSlow:
                case QLCCapability::RampDownFastToSlow:
                    value = 255 - SCALE(value, cap->min(), cap->max(), 1, 255);
                break;
                case QLCCapability::StrobeFrequency:
                case QLCCapability::PulseFrequency:
                case QLCCapability::RampUpFrequency:
                case QLCCapability::RampDownFrequency:
                    freq = cap->resource(0).toFloat();
                break;
                case QLCCapability::StrobeFreqRange:
                case QLCCapability::PulseFreqRange:
                case QLCCapability::RampUpFreqRange:
                case QLCCapability::RampDownFreqRange:
                    freq = SCALE(value, cap->min(), cap->max(),
                                 cap->resource(0).toFloat(), cap->resource(1).toFloat());
                break;
                default:
                    // invalidate any other preset, to avoid messing up the preview
                    capPreset = QLCCapability::Custom;
                break;
            }
        }
        break;
    }

    switch (capPreset)
    {
        case QLCCapability::StrobeSlowToFast:
        case QLCCapability::StrobeFastToSlow:
            freq = qMax(((float)value * MAX_STROBE_FREQ_HZ) / 255.0, MIN_STROBE_FREQ_HZ);
            highTime = qBound(50.0, 500.0 / freq, 200.0);
            lowTime = qMax((1000.0 / freq) - highTime, 0.0);
        break;
        case QLCCapability::RampUpSlowToFast:
        case QLCCapability::RampUpFastToSlow:
        case QLCCapability::RampDownSlowToFast:
        case QLCCapability::RampDownFastToSlow:
        case QLCCapability::PulseSlowToFast:
        case QLCCapability::PulseFastToSlow:
            freq = qMax(((float)value * MAX_PULSE_FREQ_HZ) / 255.0, MIN_PULSE_FREQ_HZ);
            highTime = qMax(50.0, 1000.0 / freq);
            lowTime = 0;
        break;
        default:
            highTime = qBound(50.0, 500.0 / freq, 200.0);
            lowTime = qMax((1000.0 / freq) - highTime, 0.0);
        break;
    }

    //qDebug() << "Frequency:" << freq << "Hz, high:" << highTime << ", low:" << lowTime;

    return capPreset;
}

