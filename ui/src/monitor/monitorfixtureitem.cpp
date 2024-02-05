/*
  Q Light Controller Plus
  monitorfixtureitem.cpp

  Copyright (C) Massimo Callegari

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

#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QPainter>
#include <qmath.h>
#include <QCursor>
#include <QDebug>
#include <QTimer>

#include "monitorfixtureitem.h"
#include "qlcfixturehead.h"
#include "qlcfixturemode.h"
#include "qlccapability.h"
#include "fixture.h"
#include "doc.h"

#define MOVEMENT_THICKNESS    3
#define STROBE_PERIOD 500 // 0.5s

MonitorFixtureItem::MonitorFixtureItem(Doc *doc, quint32 fid)
    : m_doc(doc)
    , m_fid(fid)
    , m_gelColor(QColor())
    , m_labelVisibility(false)
{
    Q_ASSERT(doc != NULL);

    setCursor(Qt::OpenHandCursor);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    Fixture *fxi = m_doc->fixture(fid);
    Q_ASSERT(fxi != NULL);

    m_name = fxi->name();

    setToolTip(m_name);

    m_font = qApp->font();
    m_font.setPixelSize(8);

    for (int i = 0; i < fxi->heads(); i++)
    {
        FixtureHead *fxiItem = new FixtureHead;
        fxiItem->m_item = new QGraphicsEllipseItem(this);
        fxiItem->m_item->setPen(QPen(Qt::white, 1));
        fxiItem->m_item->setBrush(QBrush(Qt::black));

        QLCFixtureHead head = fxi->head(i);
        foreach (quint32 rgbComp, head.rgbChannels())
        {
            fxiItem->m_rgb.append(rgbComp);
            //qDebug() << "Add RGB comp at address:" << rgbComp;
        }
        foreach (quint32 cmyComp, head.cmyChannels())
        {
            fxiItem->m_cmy.append(cmyComp);
            //qDebug() << "Add CMY comp at address:" << cmyComp;
        }

        fxiItem->m_dimmer = head.channelNumber(QLCChannel::Intensity, QLCChannel::MSB);
        if (fxiItem->m_dimmer != QLCChannel::invalid())
        {
            qDebug() << "Set dimmer to:" << fxiItem->m_dimmer;
        }

        fxiItem->m_masterDimmer = fxi->masterIntensityChannel();
        if (fxiItem->m_masterDimmer != QLCChannel::invalid())
        {
            qDebug() << "Set master dimmer to:" << fxiItem->m_masterDimmer;
        }

        if ((fxiItem->m_dimmer != QLCChannel::invalid()) || (fxiItem->m_masterDimmer != QLCChannel::invalid()))
        {
            fxiItem->m_back = new QGraphicsEllipseItem(this);
            fxiItem->m_back->setPen(QPen(Qt::white, 1));
            fxiItem->m_back->setBrush(QBrush(Qt::black));
        }
        else
        {
            fxiItem->m_back = NULL;
        }

        fxiItem->m_panChannel = head.channelNumber(QLCChannel::Pan, QLCChannel::MSB);
        if (fxiItem->m_panChannel != QLCChannel::invalid())
        {
            // retrieve the PAN max degrees from the fixture mode
            fxiItem->m_panMaxDegrees = 360; // fallback. Very unprecise
            QLCFixtureMode *mode = fxi->fixtureMode();
            if (mode != NULL)
            {
                if (mode->physical().focusPanMax() != 0)
                    fxiItem->m_panMaxDegrees = mode->physical().focusPanMax();
            }
            fxiItem->m_panDegrees = 0;
            qDebug() << "Pan channel on" << fxiItem->m_panChannel << "max degrees:" << fxiItem->m_panMaxDegrees;
        }

        fxiItem->m_tiltChannel = head.channelNumber(QLCChannel::Tilt, QLCChannel::MSB);
        if (fxiItem->m_tiltChannel != QLCChannel::invalid())
        {
            // retrieve the TILT max degrees from the fixture mode
            fxiItem->m_tiltMaxDegrees = 270; // fallback. Very unprecise
            QLCFixtureMode *mode = fxi->fixtureMode();
            if (mode != NULL)
            {
                if (mode->physical().focusTiltMax() != 0)
                    fxiItem->m_tiltMaxDegrees = mode->physical().focusTiltMax();
            }
            fxiItem->m_tiltDegrees = 0;
            qDebug() << "Tilt channel on" << fxiItem->m_tiltChannel << "max degrees:" << fxiItem->m_tiltMaxDegrees;
        }

        QLCFixtureMode *mode = fxi->fixtureMode();
        if (mode != NULL)
        {
            foreach (quint32 wheel, head.colorWheels())
            {
               QList<QColor> values;
               QLCChannel *ch = mode->channel(wheel);
               if (ch == NULL)
                   continue;

               bool containsColor = false;
               for (quint32 v = 0; v < 256; ++v)
               {
                   QLCCapability *cap = ch->searchCapability(v);
                   if (cap != NULL && cap->resource(0).isValid())
                   {
                       values << cap->resource(0).value<QColor>();
                       containsColor = true;
                   }
                   else
                   {
                       values << QColor();
                   }
               }

               if (containsColor)
               {
                   fxiItem->m_colorValues[wheel] = values;
                   fxiItem->m_colorWheels << wheel;
               }
            }

            foreach (quint32 shutter, head.shutterChannels())
            {
               QList<FixtureHead::ShutterState> values;
               QLCChannel *ch = mode->channel(shutter);
               if (ch == NULL)
                   continue;

               bool containsShutter = false;

               switch (ch->preset())
               {
                   case QLCChannel::ShutterStrobeFastSlow:
                   case QLCChannel::ShutterStrobeSlowFast:
                   {
                       // handle case when the channel has only one capability 0-255 strobe:
                       // make 0 Open to avoid blinking
                       values << FixtureHead::Open;
                       for (int v = 1; v < 256; v++)
                           values << FixtureHead::Strobe;
                       containsShutter = true;
                   }
                   break;
                   case QLCChannel::Custom:
                   {
                       foreach (QLCCapability *cap, ch->capabilities())
                       {
                           for (int v = cap->min(); v <= cap->max(); v++)
                           {
                               switch (cap->preset())
                               {
                                   case QLCCapability::Custom:
                                       values << FixtureHead::Open;
                                   break;
                                   case QLCCapability::ShutterOpen:
                                       values << FixtureHead::Open;
                                       containsShutter = true;
                                   break;
                                   case QLCCapability::ShutterClose:
                                       values << FixtureHead::Closed;
                                       containsShutter = true;
                                   break;
                                   default:
                                       values << FixtureHead::Strobe;
                                       containsShutter = true;
                                   break;
                               }
                           }
                       }
                   }
                   break;
                   default:
                   break;
               }

               if (containsShutter)
               {
                   fxiItem->m_shutterValues[shutter] = values;
                   fxiItem->m_shutterChannels << shutter;
               }
            }
        }

        if (!fxiItem->m_shutterChannels.isEmpty())
        {
            fxiItem->m_strobeTimer = new QTimer(this);
            connect(fxiItem->m_strobeTimer, SIGNAL(timeout()), this, SLOT(slotStrobeTimer()));
        }
        else
        {
            fxiItem->m_strobeTimer = 0;
        }

        m_heads.append(fxiItem);
    }
    slotUpdateValues();
    connect(fxi, SIGNAL(valuesChanged()), this, SLOT(slotUpdateValues()));
}

MonitorFixtureItem::~MonitorFixtureItem()
{
    if (m_fid != Fixture::invalidId())
    {
        Fixture* fxi = m_doc->fixture(m_fid);
        if (fxi != NULL)
            disconnect(fxi, SIGNAL(valuesChanged()), this, SLOT(slotUpdateValues()));
    }

    foreach (FixtureHead *head, m_heads)
    {
        if (head->m_strobeTimer != 0)
        {
            disconnect(head->m_strobeTimer, SIGNAL(timeout()), this, SLOT(slotStrobeTimer()));
            delete head->m_strobeTimer;
        }
        delete head;
    }
    m_heads.clear();
}

void MonitorFixtureItem::setSize(QSize size)
{
    prepareGeometryChange();
    m_width = size.width();
    m_height = size.height();

    if (m_width < 5 || m_height < 5)
        return;

    // if this fixture has a pan or tilt channel,
    // the head area has to be reduced to
    // leave space to movements representation
    int headsWidth = m_width;
    int headsHeight = m_height;

    // calculate the diameter of every single head
    double headArea = (headsWidth * headsHeight) / m_heads.count();
    double headSide = sqrt(headArea);
    int columns = (headsWidth / headSide) + 0.5;
    int rows = (headsHeight / headSide) + 0.5;

    // dirty workaround to correctly display right columns on one row
    if (rows == 1)
        columns = m_heads.count();
    if (columns == 1)
        rows = m_heads.count();

    //qDebug() << "Fixture columns:" << columns;

    if (columns > m_heads.count())
        columns = m_heads.count();

    if (rows < 1)
        rows = 1;
    if (columns < 1)
        columns = 1;

    double cellWidth = headsWidth / columns;
    double cellHeight = headsHeight / rows;
    double headDiam = (cellWidth < cellHeight) ? cellWidth : cellHeight;

    int ypos = (cellHeight - headDiam) / 2;
    for (int i = 0; i < rows; i++)
    {
        int xpos = (cellWidth - headDiam) / 2;
        for (int j = 0; j < columns; j++)
        {
            int index = i * columns + j;
            if (index < m_heads.size())
            {
                FixtureHead * h = m_heads.at(index);
                QGraphicsEllipseItem *head = h->m_item;
                head->setRect(xpos, ypos, headDiam, headDiam);

                if (h->m_panChannel != QLCChannel::invalid())
                {
                    head->setRect(head->rect().adjusted(MOVEMENT_THICKNESS + 1, MOVEMENT_THICKNESS + 1,
                                                        -MOVEMENT_THICKNESS - 1, -MOVEMENT_THICKNESS - 1));
                }
                if (h->m_tiltChannel != QLCChannel::invalid())
                {
                    head->setRect(head->rect().adjusted(MOVEMENT_THICKNESS + 1, MOVEMENT_THICKNESS + 1,
                                                        -MOVEMENT_THICKNESS - 1, -MOVEMENT_THICKNESS - 1));
                }

                head->setZValue(2);
                QGraphicsEllipseItem *back = m_heads.at(index)->m_back;
                if (back != NULL)
                {
                    back->setRect(head->rect());
                    back->setZValue(1);
                }
            }
            xpos += cellWidth;
        }
        ypos += cellHeight;
    }

    QFontMetrics fm(m_font);
    m_labelRect = fm.boundingRect(QRect(-10, m_height + 2, m_width + 20, 30),
                                  Qt::AlignHCenter | Qt::TextWrapAnywhere, m_name);

    setTransformOriginPoint(m_width / 2, m_height / 2);
    update();
}

QColor MonitorFixtureItem::computeColor(const FixtureHead *head, const QByteArray & values)
{
    foreach (quint32 c, head->m_colorWheels)
    {
        const uchar val = static_cast<uchar>(values.at(c));
        QColor col = head->m_colorValues[c].at(val);
        if (col.isValid() && col != Qt::black)
            return col;
    }

    if (head->m_rgb.count() > 0)
    {
        uchar r = 0, g = 0, b = 0;
        r = values.at(head->m_rgb.at(0));
        g = values.at(head->m_rgb.at(1));
        b = values.at(head->m_rgb.at(2));
        return QColor(r, g, b);
    }

    if (head->m_cmy.count() > 0)
    {
        uchar c = 0, m = 0, y = 0;
        c = values.at(head->m_cmy.at(0));
        m = values.at(head->m_cmy.at(1));
        y = values.at(head->m_cmy.at(2));
        return QColor::fromCmyk(c, m, y, 0);
    }

    if (m_gelColor.isValid())
    {
        return m_gelColor;
    }

    return QColor(255,255,255);
}
uchar MonitorFixtureItem::computeAlpha(const FixtureHead *head, const QByteArray & values)
{
    // postpone division as late as possible to improve accuracy
    unsigned mul = 255U;
    unsigned div = 1U;

    if (head->m_masterDimmer != UINT_MAX /*QLCChannel::invalid()*/)
    {
        mul *= static_cast<uchar>(values.at(head->m_masterDimmer));
        div *= 255U;
    }

    if (head->m_dimmer != UINT_MAX /*QLCChannel::invalid()*/)
    {
        mul *= static_cast<uchar>(values.at(head->m_dimmer));
        div *= 255U;
    }

    //qDebug() << mul << "/" << div << "=" << (mul /div);
    return mul / div;
}

FixtureHead::ShutterState MonitorFixtureItem::computeShutter(const FixtureHead *head, const QByteArray & values)
{
    FixtureHead::ShutterState result = FixtureHead::Open;

    foreach (quint32 c, head->m_shutterChannels)
    {
        const uchar val = static_cast<uchar>(values.at(c));
        FixtureHead::ShutterState state = head->m_shutterValues[c].at(val);
        if (state == FixtureHead::Closed)
        {
            return state;
        }
        else if (state == FixtureHead::Strobe)
        {
            result = state;
        }
    }

    return result;
}

void MonitorFixtureItem::slotUpdateValues()
{
    /* Check that this MonitorFixture represents a fixture */
    if (m_fid == Fixture::invalidId())
        return;

    /* Check that this MonitorFixture's fixture really exists */
    Fixture* fxi = m_doc->fixture(m_fid);
    if (fxi == NULL)
        return;

    QByteArray fxValues = fxi->channelValues();

    bool needUpdate = false;

    foreach (FixtureHead *head, m_heads)
    {
        head->m_color = computeColor(head, fxValues);
        head->m_dimmerValue = computeAlpha(head, fxValues);
        head->m_shutterState = computeShutter(head, fxValues);

        QColor col = head->m_color;
        col.setAlpha(head->m_dimmerValue);

        if (head->m_dimmerValue > 0)
        {
            if (head->m_shutterState == FixtureHead::Closed)
            {
                col.setAlpha(0);
            }

            if (head->m_shutterState == FixtureHead::Strobe)
            {
                if (head->m_strobeTimer != 0 && !head->m_strobeTimer->isActive())
                {
                    head->m_strobePhase = 0;
                    head->m_strobeTimer->start(STROBE_PERIOD);
                }
                else if (head->m_strobePhase != 0)
                {
                    col.setAlpha(0);
                }
            }
            else if (head->m_strobeTimer != 0)
            {
                head->m_strobeTimer->stop();
            }
        }
        else if (head->m_strobeTimer != 0)
        {
            head->m_strobeTimer->stop();
        }

        head->m_item->setBrush(QBrush(col));

        if (head->m_panChannel != UINT_MAX /*QLCChannel::invalid()*/)
        {
            computePanPosition(head, fxValues.at(head->m_panChannel));
            needUpdate = true;
        }

        if (head->m_tiltChannel != UINT_MAX /*QLCChannel::invalid()*/)
        {
            computeTiltPosition(head, fxValues.at(head->m_tiltChannel));
            needUpdate = true;
        }
    }
    if (needUpdate)
        update();
}

void MonitorFixtureItem::slotStrobeTimer()
{
    QTimer* timer = qobject_cast<QTimer*>(sender());
    foreach (FixtureHead *head, m_heads)
    {
        if (head->m_strobeTimer != timer)
            continue;

        if (head->m_dimmerValue == 0 || head->m_shutterState != FixtureHead::Strobe)
            return;

        head->m_strobePhase = (head->m_strobePhase + 1) % 2;

        QColor col = head->m_color;
        col.setAlpha(head->m_dimmerValue);
        if (head->m_strobePhase != 0)
            col.setAlpha(0);
        head->m_item->setBrush(QBrush(col));
        update();
        return;
    }
}

void MonitorFixtureItem::showLabel(bool visible)
{
    prepareGeometryChange();
    m_labelVisibility = visible;
    update();
}

QRectF MonitorFixtureItem::boundingRect() const
{
    if (m_labelVisibility)
        return QRectF(-10, 0, m_width + 20, m_height + m_labelRect.height() + 2);
    else
        return QRectF(0, 0, m_width, m_height);
}

void MonitorFixtureItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QColor defColor = Qt::darkGray;

    if (this->isSelected() == true)
        defColor = Qt::yellow;

    painter->setPen(QPen(defColor, 1));

    // draw item background
    painter->setBrush(QBrush(QColor(33, 33, 33)));
    painter->drawRect(0, 0, m_width, m_height);
    foreach (FixtureHead *head, m_heads)
    {
        QRectF rect = head->m_item->rect();

        if (head->m_tiltChannel != UINT_MAX /*QLCChannel::invalid()*/)
        {
            rect.adjust(-MOVEMENT_THICKNESS, -MOVEMENT_THICKNESS, MOVEMENT_THICKNESS, MOVEMENT_THICKNESS);

            painter->setPen(QPen(defColor, MOVEMENT_THICKNESS));
            painter->drawArc(rect, 270 * 16 - head->m_tiltMaxDegrees * 16 / 2 - 8, 16);
            painter->drawArc(rect, 270 * 16 + head->m_tiltMaxDegrees * 16 / 2 - 8, 16);
            painter->setPen(QPen(QColor("turquoise"), MOVEMENT_THICKNESS));
            painter->drawArc(rect, 270 * 16, - head->m_tiltDegrees * 16);
        }

        if (head->m_panChannel != UINT_MAX /*QLCChannel::invalid()*/)
        {
            rect.adjust(-MOVEMENT_THICKNESS, -MOVEMENT_THICKNESS, MOVEMENT_THICKNESS, MOVEMENT_THICKNESS);

            painter->setPen(QPen(defColor, MOVEMENT_THICKNESS));
            painter->drawArc(rect, 270 * 16 - head->m_panMaxDegrees * 16 / 2 - 8, 16);
            painter->drawArc(rect, 270 * 16 + head->m_panMaxDegrees * 16 / 2 - 8, 16);
            painter->setPen(QPen(QColor("purple"), MOVEMENT_THICKNESS));
            painter->drawArc(rect, 270 * 16, - head->m_panDegrees * 16);
        }
    }

    if (m_labelVisibility)
    {
        painter->setFont(m_font);
        painter->setPen(QPen(Qt::NoPen));
        painter->setBrush(QBrush(QColor(33, 33, 33)));
        painter->drawRoundedRect(m_labelRect, 2, 2);
        painter->setPen(QPen(Qt::white, 1));
        painter->drawText(m_labelRect, Qt::AlignHCenter | Qt::TextWrapAnywhere, m_name);
    }
}

void MonitorFixtureItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    this->setSelected(true);
}

void MonitorFixtureItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    qDebug() << Q_FUNC_INFO << "mouse RELEASE event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    setCursor(Qt::OpenHandCursor);
    emit itemDropped(this);
}

void MonitorFixtureItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
}

void MonitorFixtureItem::computeTiltPosition(FixtureHead *h, uchar value)
{
    // find the TILT degrees based on value
    h->m_tiltDegrees = ((double)value * h->m_tiltMaxDegrees) / (256.0 - 1/256) - (h->m_tiltMaxDegrees / 2);
    //qDebug() << "TILT degrees:" << h->m_tiltDegrees;
}

void MonitorFixtureItem::computePanPosition(FixtureHead *h, uchar value)
{
    // find the PAN degrees based on value
    h->m_panDegrees = ((double)value * h->m_panMaxDegrees) / (256.0 - 1/256) - (h->m_panMaxDegrees / 2);
    //qDebug() << "PAN degrees:" << h->m_panDegrees;
}
