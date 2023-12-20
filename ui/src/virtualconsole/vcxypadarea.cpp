/*
  Q Light Controller
  vcxypadarea.cpp

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QCursor>
#include <qmath.h>
#include <QMutex>
#include <QDebug>
#include <QPoint>

#include "qlcmacros.h"

#include "efxpreviewarea.h"
#include "vcxypadarea.h"
#include "vcframe.h"

const qreal MAX_VALUE = 256.0;
const qreal MAX_DMX_VALUE = MAX_VALUE - 1.0/256;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCXYPadArea::VCXYPadArea(QWidget* parent)
    : QFrame(parent)
    , m_changed(false)
    , m_activePixmap(":/xypad-point-blue.png")
    , m_fixturePixmap(":/xypad-point.png")
    , m_rangeDmxRect()
    , m_rangeWindowRect()
    , m_degreesRange()
    , m_previewArea(NULL)
{
    setFrameStyle(KVCFrameStyleSunken);
    setWindowTitle("XY Pad");
    setMode(Doc::Design);
    setFocusPolicy(Qt::ClickFocus);
    new QVBoxLayout(this);
}

VCXYPadArea::~VCXYPadArea()
{
}

void VCXYPadArea::setMode(Doc::Mode mode)
{
    m_mode = mode;
    if (mode == Doc::Design)
        setEnabled(false);
    else
        setEnabled(true);
    update();
}

/*****************************************************************************
 * Current XY position
 *****************************************************************************/

QPointF VCXYPadArea::position(bool resetChanged) const
{
    QMutexLocker locker(&m_mutex);
    QPointF pos(m_dmxPos);
    if (resetChanged)
        m_changed = false;
    return pos;
}

void VCXYPadArea::setPosition(const QPointF& point)
{
    {
        QMutexLocker locker(&m_mutex);

        if (m_dmxPos != point)
        {
            m_dmxPos = point;

            if (m_dmxPos.x() > MAX_DMX_VALUE)
                m_dmxPos.setX(MAX_DMX_VALUE);
            if (m_dmxPos.y() > MAX_DMX_VALUE)
                m_dmxPos.setY(MAX_DMX_VALUE);

            m_changed = true;
        }
    }

    emit positionChanged(point);
}

void VCXYPadArea::nudgePosition(qreal dx, qreal dy)
{
    {
        QMutexLocker locker(&m_mutex);

        m_dmxPos.setX(CLAMP(m_dmxPos.x() + dx, qreal(0), MAX_DMX_VALUE));
        m_dmxPos.setY(CLAMP(m_dmxPos.y() + dy, qreal(0), MAX_DMX_VALUE));

        m_changed = true;
    }

    emit positionChanged(m_dmxPos);
}

bool VCXYPadArea::hasPositionChanged()
{
    QMutexLocker locker(&m_mutex);
    return m_changed;
}

void VCXYPadArea::slotFixturePositions(const QVariantList positions)
{
    if (positions == m_fixturePositions)
        return;

    m_fixturePositions = positions;
    update();
}

void VCXYPadArea::checkDmxRange()
{
     QPointF pt(CLAMP(m_dmxPos.x(), m_rangeDmxRect.left(), m_rangeDmxRect.right()),
         CLAMP(m_dmxPos.y(), m_rangeDmxRect.top(), m_rangeDmxRect.bottom()));

     setPosition(pt);
}

void VCXYPadArea::updateWindowPos()
{
    m_windowPos.setX(SCALE(m_dmxPos.x(), qreal(0), qreal(256), qreal(0), qreal(width())));
    m_windowPos.setY(SCALE(m_dmxPos.y(), qreal(0), qreal(256), qreal(0), qreal(height())));
}

static int coarseByte(qreal value)
{
    return value;
}

static int fineByte(qreal value)
{
    return (value - floor(value)) * 256;
}

QString VCXYPadArea::positionString() const
{
    QPointF pos = position(false);
    return QString("%1.%2 : %3.%4")
        .arg(coarseByte(pos.x()), 3, 10, QChar('0'))
        .arg(fineByte(pos.x()), 3, 10, QChar('0'))
        .arg(coarseByte(pos.y()), 3, 10, QChar('0'))
        .arg(fineByte(pos.y()), 3,10, QChar('0'));
}

QString VCXYPadArea::angleString() const
{
    QPointF pos = position(false);
    QRectF range = degreesRange();

    if (range.isValid())
    {
        return QString("%1%2 : %3%4")
            .arg(range.x() + pos.x() * range.width() / 256)
            .arg(QChar(0xb0))
            .arg(range.y() + pos.y() * range.height() / 256)
            .arg(QChar(0xb0));
    }
    else
    {
        return QString("%1 % : %2 %")
            .arg(pos.x() * 100 / MAX_DMX_VALUE, 7, 'f', 3, '0')
            .arg(pos.y() * 100 / MAX_DMX_VALUE, 7, 'f', 3, '0');
    }

}

/*************************************************************************
 * Range window
 *************************************************************************/

QRectF VCXYPadArea::rangeWindow()
{
    return m_rangeDmxRect;
}

void VCXYPadArea::setRangeWindow(QRectF rect)
{
    m_rangeDmxRect = rect;
    updateRangeWindow();
}

void VCXYPadArea::updateRangeWindow()
{
    int x = m_rangeDmxRect.x() * width() / 256;
    int y = m_rangeDmxRect.y() * height() / 256;
    int w = m_rangeDmxRect.width() * width() / 256;
    int h = m_rangeDmxRect.height() * height() / 256;
    m_rangeWindowRect = QRect(x, y, w, h);
}

/*************************************************************************
 * Degrees range
 *************************************************************************/

QRectF VCXYPadArea::degreesRange() const
{
    return m_degreesRange;
}

void VCXYPadArea::setDegreesRange(QRectF range)
{
    m_degreesRange = range;
    update();
}

/*************************************************************************
 * EFX Preview
 *************************************************************************/

void VCXYPadArea::enableEFXPreview(bool enable)
{
    if (enable)
    {
        if (m_previewArea == NULL)
        {
            m_previewArea = new EFXPreviewArea(this);
            m_previewArea->setBackgroundAlpha(0);
            layout()->setContentsMargins(0, 0, 0, 0);
            layout()->addWidget(m_previewArea);
        }
    }
    else
    {
        if (m_previewArea)
        {
            m_previewArea->deleteLater();
            m_previewArea = NULL;
        }
    }
}

void VCXYPadArea::setEFXPolygons(const QPolygonF &pattern, const QVector<QPolygonF> fixtures)
{
    if (m_previewArea == NULL)
        enableEFXPreview(true);

    m_previewArea->setPolygon(pattern);
    m_previewArea->setFixturePolygons(fixtures);
}

void VCXYPadArea::setEFXInterval(uint duration)
{
    m_previewArea->draw(duration / m_previewArea->polygonsCount());
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCXYPadArea::paintEvent(QPaintEvent* e)
{
    if (m_rangeWindowRect.isValid() && (m_mode == Doc::Operate))
        checkDmxRange();

    /* Let the parent class draw its stuff first */
    QFrame::paintEvent(e);

    QPainter p(this);
    QPen pen;

    if (m_previewArea == NULL)
    {
        QString title = QString("%1%2%3\n%4\n")
            .arg(windowTitle())
            .arg(windowTitle().isEmpty() ? "" : "\n")
            .arg(positionString())
            .arg(angleString());

        /* Draw name (offset just a bit to avoid frame) */
        p.drawText(1, 1, width() - 2, height() - 2,
                   Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, title);

        QFont font = p.font();
        font.setPointSize(font.pointSize() - 2);
        p.setFont(font);
        p.drawText(1, 1, width() - 2, height() - 2,
                   Qt::AlignRight | Qt::AlignBottom | Qt::TextWordWrap, tr("Shift: fine, Ctrl:10x"));
    }
    /* Draw crosshairs to indicate the center position */
    pen.setStyle(Qt::DotLine);
    pen.setColor(palette().color(QPalette::WindowText));
    pen.setWidth(0);
    p.setPen(pen);
    p.drawLine(width() / 2, 0, width() / 2, height());
    p.drawLine(0, height() / 2, width(), height() / 2);

    /* Draw the range window if not full size */
    if (m_rangeWindowRect.isValid())
    {
        pen.setStyle(Qt::SolidLine);
        pen.setColor(QColor(0, 120, 0, 170));
        p.setPen(pen);
        p.fillRect(m_rangeWindowRect, QBrush(QColor(155, 200, 165, 130)));
        p.drawRect(m_rangeWindowRect);
    }

    updateWindowPos();

    if (m_previewArea == NULL)
    {
        foreach (QVariant pos, m_fixturePositions)
        {
            QPointF pt = pos.toPointF();
            pt.setX(SCALE(pt.x(), qreal(0), qreal(256), qreal(0), qreal(width())));
            pt.setY(SCALE(pt.y(), qreal(0), qreal(256), qreal(0), qreal(height())));

            p.drawPixmap(pt.x() - (m_fixturePixmap.width() / 2),
                     pt.y() - (m_fixturePixmap.height() / 2),
                     m_fixturePixmap);
        }

        /* Draw the current point pixmap */
        p.drawPixmap(m_windowPos.x() - (m_activePixmap.width() / 2),
                     m_windowPos.y() - (m_activePixmap.height() / 2),
                     m_activePixmap);
    }
}

void VCXYPadArea::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    updateRangeWindow();
}

void VCXYPadArea::mousePressEvent(QMouseEvent* e)
{
    if (m_mode == Doc::Operate)
    {
        QPointF pt(CLAMP(e->pos().x(), 0, width()), CLAMP(e->pos().y(), 0, height()));
        pt.setX(SCALE(pt.x(), qreal(0), qreal(width()), qreal(0), qreal(256)));
        pt.setY(SCALE(pt.y(), qreal(0), qreal(height()), qreal(0), qreal(256)));

        setPosition(pt);
        setMouseTracking(true);
        setCursor(Qt::CrossCursor);
        update();
    }

    QFrame::mousePressEvent(e);
}

void VCXYPadArea::mouseReleaseEvent(QMouseEvent* e)
{
    if (m_mode == Doc::Operate)
    {
        QPointF pt(CLAMP(e->pos().x(), 0, width()), CLAMP(e->pos().y(), 0, height()));
        pt.setX(SCALE(pt.x(), qreal(0), qreal(width()), qreal(0), qreal(256)));
        pt.setY(SCALE(pt.y(), qreal(0), qreal(height()), qreal(0), qreal(256)));

        setPosition(pt);
        setMouseTracking(false);
        unsetCursor();
    }

    QFrame::mouseReleaseEvent(e);
}

void VCXYPadArea::mouseMoveEvent(QMouseEvent* e)
{
    if (m_mode == Doc::Operate)
    {
        QPointF pt(CLAMP(e->pos().x(), 0, width()), CLAMP(e->pos().y(), 0, height()));
        pt.setX(SCALE(pt.x(), qreal(0), qreal(width()), qreal(0), qreal(256)));
        pt.setY(SCALE(pt.y(), qreal(0), qreal(height()), qreal(0), qreal(256)));

        setPosition(pt);
        update();
    }

    QFrame::mouseMoveEvent(e);
}

void VCXYPadArea::keyPressEvent(QKeyEvent *e)
{
    if (m_mode == Doc::Operate)
    {
        qreal step = 1;
        if (e->modifiers().testFlag(Qt::ControlModifier))
            step *= 10;
        if (e->modifiers().testFlag(Qt::ShiftModifier))
            step /= 256;

        if (e->key() == Qt::Key_Left)
        {
            nudgePosition(-step , 0);
            update();
        }
        else if (e->key() == Qt::Key_Right)
        {
            nudgePosition(step, 0);
            update();
        }
        else if (e->key() == Qt::Key_Up)
        {
            nudgePosition(0, -step);
            update();
        }
        else if (e->key() == Qt::Key_Down)
        {
            nudgePosition(0, step);
            update();
        }
        else
        {
            QFrame::keyPressEvent(e);
        }
    }

    else QFrame::keyPressEvent(e);
}


void VCXYPadArea::keyReleaseEvent (QKeyEvent * e)
{
    QFrame::keyReleaseEvent(e);
}

