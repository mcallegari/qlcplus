/*
  Q Light Controller Plus
  clickandgowidget.cpp

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

#include <QApplication>
#include <QPainter>
#include <QImage>
#include <QDesktopWidget>

#include "clickandgowidget.h"
#include "qlccapability.h"
#include "qlcmacros.h"
#include "vcslider.h"

#define CELL_W  150
#define CELL_H  45

ClickAndGoWidget::ClickAndGoWidget(QWidget *parent) :
    QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setMouseTracking(true);

    m_type = None;
    m_linearColor = false;
    m_width = 10;
    m_height = 10;
    m_cols = 0;
    m_rows = 0;
    m_hoverCellIdx = 0;
    m_cellBarXpos = 1;
    m_cellBarYpos = 1;
    m_cellBarWidth = 0;
}

void ClickAndGoWidget::setupGradient(QColor end)
{
    QColor begin = Qt::black;

    QLinearGradient linearGrad(QPointF(10,0), QPointF(266, 0));
    linearGrad.setColorAt(0, begin);
    linearGrad.setColorAt(1, end);

    // create image and fill it with gradient
    m_width = 276;
    m_height = 40;
    m_image = QImage(m_width, m_height, QImage::Format_RGB32);
    QPainter painter(&m_image);
    painter.fillRect(m_image.rect(), linearGrad);

    m_linearColor = true;
}

void ClickAndGoWidget::fillWithGradient(int r, int g, int b, QPainter *painter, int x)
{
    QColor top = Qt::black;
    QColor col(r, g , b);
    QColor bottom = Qt::white;

    QLinearGradient blackGrad(QPointF(0,0), QPointF(0, 127));
    blackGrad.setColorAt(0, top);
    blackGrad.setColorAt(1, col);
    QLinearGradient whiteGrad(QPointF(0,128), QPointF(0, 255));
    whiteGrad.setColorAt(0, col);
    whiteGrad.setColorAt(1, bottom);

    painter->fillRect(x, 0, x, 128, blackGrad);
    painter->fillRect(x, 128, x, 256, whiteGrad);
}

void ClickAndGoWidget::setupColorPicker()
{
    int r = 0xFF;
    int g = 0;
    int b = 0;
    int x = 30;
    int i = 0;
    int cw = 15;

    m_width = 252 + 30;
    m_height = 256;
    m_image = QImage(m_width, m_height, QImage::Format_RGB32);
    QPainter painter(&m_image);

    // Draw 16 default color squares
    painter.fillRect(0, 0, cw, 32, QColor(Qt::white));
    painter.fillRect(cw, 0, cw + cw, 32, QColor(Qt::black));
    painter.fillRect(0, 32, cw, 64, QColor(Qt::red));
    painter.fillRect(cw, 32, cw + cw, 64, QColor(Qt::darkRed));
    painter.fillRect(0, 64, cw, 96, QColor(Qt::green));
    painter.fillRect(cw, 64, cw + cw, 96, QColor(Qt::darkGreen));
    painter.fillRect(0, 96, cw, 128, QColor(Qt::blue));
    painter.fillRect(cw, 96, cw + cw, 128, QColor(Qt::darkBlue));
    painter.fillRect(0, 128, cw, 160, QColor(Qt::cyan));
    painter.fillRect(cw, 128, cw + cw, 160, QColor(Qt::darkCyan));
    painter.fillRect(0, 160, cw, 192, QColor(Qt::magenta));
    painter.fillRect(cw, 160, cw + cw, 192, QColor(Qt::darkMagenta));
    painter.fillRect(0, 192, cw, 224, QColor(Qt::yellow));
    painter.fillRect(cw, 192, cw + cw, 224, QColor(Qt::darkYellow));
    painter.fillRect(0, 224, cw, 256, QColor(Qt::gray));
    painter.fillRect(cw, 224, cw + cw, 256, QColor(Qt::darkGray));

    // R: 255  G:  0  B:   0
    for (i = x; i < x + 42; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        g+=6;
        if (g == 252) g = 255;
    }
    x+=42;
    // R: 255  G: 255  B:   0
    for (i = x; i < x + 42; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        r-=6;
        if (r < 6) r = 0;
    }
    x+=42;
    // R: 0  G: 255  B:  0
    for (i = x; i < x + 42; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        b+=6;
        if (b == 252) b = 255;
    }
    x+=42;
    // R: 0  G: 255  B:  255
    for (i = x; i < x + 42; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        g-=6;
        if (g < 6) g = 0;
    }
    x+=42;
    // R: 0  G:  0  B:  255
    for (i = x; i < x + 42; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        r+=6;
        if (r == 252) r = 255;
    }
    x+=42;
    // R: 255  G:  0  B:  255
    for (i = x; i < x + 42; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        b-=6;
        if (b < 6) b = 0;
    }
    // R: 255  G:  0  B:  0
}

void ClickAndGoWidget::setType(int type, const QLCChannel *chan)
{
    m_linearColor = false;
    //qDebug() << Q_FUNC_INFO << "Type: " << type;
    if (type == None)
    {
        m_image = QImage();
    }
    else if (type == Red)
        setupGradient(Qt::red);
    else if (type == Green)
        setupGradient(Qt::green);
    else if (type == Blue)
        setupGradient(Qt::blue);
    else if (type == Cyan)
        setupGradient(Qt::cyan);
    else if (type == Magenta)
        setupGradient(Qt::magenta);
    else if (type == Yellow)
        setupGradient(Qt::yellow);
    else if (type == White)
        setupGradient(Qt::white);
    else if (type == RGB || type == CMY)
    {
        setupColorPicker();
    }
    else if (type == Preset)
    {
        createPresetList(chan);
        setupPresetPicker();
    }

    m_type = type;
}

int ClickAndGoWidget::getType()
{
    return m_type;
}

QString ClickAndGoWidget::clickAndGoTypeToString(ClickAndGoWidget::ClickAndGo type)
{
    switch (type)
    {
        default:
        case None: return "None"; break;
        case Red: return "Red"; break;
        case Green: return "Green"; break;
        case Blue: return "Blue"; break;
        case Cyan: return "Cyan"; break;
        case Magenta: return "Magenta"; break;
        case Yellow: return "Yellow"; break;
        case White: return "White"; break;
        case RGB: return "RGB"; break;
        case CMY: return "CMY"; break;
        case Preset: return "Preset"; break;
    }
}

ClickAndGoWidget::ClickAndGo ClickAndGoWidget::stringToClickAndGoType(QString str)
{
    if (str == "Red") return Red;
    else if (str == "Green") return Green;
    else if (str == "Blue") return Blue;
    else if (str == "Cyan") return Cyan;
    else if (str == "Magenta") return Magenta;
    else if (str == "Yellow") return Yellow;
    else if (str == "White") return White;
    else if (str == "RGB") return RGB;
    else if (str == "CMY") return CMY;
    else if (str == "Preset") return Preset;

    return None;
}

QColor ClickAndGoWidget::getColorAt(uchar pos)
{
    if (m_linearColor == true)
    {
        QRgb col = m_image.pixel(10 + pos, 10);
        return QColor(col);
    }
    return QColor(0,0,0);
}

QImage ClickAndGoWidget::getImageFromValue(uchar value)
{
    /** If the widget type is a Preset, return directly
     *  the pre-loaded resource */
    if (m_type == Preset)
    {
        foreach(PresetResource res, m_resources)
        {
            if (value >= res.m_min && value <= res.m_max)
                return res.m_thumbnail;
        }
    }

    QImage img(42, 42, QImage::Format_RGB32);
    if (m_type == None)
    {
        img.fill(Qt::black);
    }
    else if (m_linearColor == true)
    {
        QRgb col = m_image.pixel(10 + value, 10);
        img.fill(QColor(col).rgb());
    }

    return img;
}

void ClickAndGoWidget::createPresetList(const QLCChannel *chan)
{
    int i = 1;
    if (chan == NULL)
        return;

    m_resources.clear();

    //qDebug() << Q_FUNC_INFO << "cap #" << chan->capabilities().size();

    foreach(QLCCapability cap, chan->capabilities())
    {
        if (cap.resourceName().isEmpty() == false)
            m_resources.append(PresetResource(cap.resourceName(), cap.name(),
                                              cap.min(), cap.max()));
        else if (cap.resourceColor1().isValid())
            m_resources.append(PresetResource(cap.resourceColor1(), cap.resourceColor2(),
                                              cap.name(), cap.min(), cap.max()));
        else
            m_resources.append(PresetResource(i, cap.name(), cap.min(), cap.max()));
        i++;
    }
}

void ClickAndGoWidget::setupPresetPicker()
{
    if (m_resources.size() == 0)
        return;

    QRect screen = QApplication::desktop()->availableGeometry(this);

    m_cols = 2;
    m_rows = qCeil((qreal)m_resources.size() / 2);
    m_width = CELL_W * m_cols;
    m_height = CELL_H * m_rows;

    if (m_height > screen.height())
    {
        m_rows = qFloor((qreal)screen.height() / CELL_H);
        m_cols = qCeil((qreal)m_resources.size() / m_rows);
        m_width = CELL_W * m_cols;
        m_height = CELL_H * m_rows;
    }
 
    int x = 0;
    int y = 0;
    m_image = QImage(m_width, m_height, QImage::Format_RGB32);
    QPainter painter(&m_image);
    painter.setRenderHint(QPainter::Antialiasing);
    QLinearGradient presetGrad(QPointF(0,0), QPointF(0, m_height));
    presetGrad.setColorAt(0, QApplication::palette().background().color());
    presetGrad.setColorAt(1, QColor(173, 171, 179));
    painter.fillRect(0, 0, m_width, m_height, presetGrad);
    for (int i = 0; i < m_resources.size(); i++)
    {
        PresetResource res = m_resources.at(i);
        painter.setPen(Qt::black);
        painter.drawRect(x, y, CELL_W, CELL_H);
        painter.drawImage(x + 1, y + 4, res.m_thumbnail);
        painter.drawText(x + 43, y + 4, CELL_W - 42, CELL_H - 5, Qt::TextWordWrap|Qt::AlignVCenter, res.m_descr);
        if (i % m_cols == m_cols - 1)
        {
            y += CELL_H;
            x = 0;
        }
        else
            x += CELL_W;
         
    }  
}

QSize ClickAndGoWidget::sizeHint() const
{
    return QSize(m_width, m_height);
}

void ClickAndGoWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_linearColor == true)
    {
        if (event->x() <= 10)
            emit levelChanged(0);
        else if (event->x() > 10 && event->x() < 256)
            emit levelChanged((uchar)(event->x() - 10));
        else
            emit levelChanged(255);
    }
    else if (m_type == RGB || m_type == CMY)
    {
        emit colorChanged(m_image.pixel(event->x(), event->y()));
    }
    else if (m_type == Preset)
    {
        if (m_hoverCellIdx >= 0 && m_hoverCellIdx < m_resources.length())
        {
            PresetResource res = m_resources.at(m_hoverCellIdx);
            qDebug() << "Mouse press. cellW: " << m_cellBarWidth << "min: " << res.m_min << "max:" << res.m_max;

            float f = SCALE(float(m_cellBarWidth),
                        float(0),
                        float(CELL_W),
                        float(0), float(res.m_max - res.m_min));
            emit levelAndPresetChanged((uchar)f + res.m_min, res.m_thumbnail);
        }
    }
    QWidget::mousePressEvent(event);
}

void ClickAndGoWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_type != Preset)
        return;
    // calculate the index of the resource where the cursor is
    int floorX = qFloor(event->x() / CELL_W);
    int floorY = qFloor(event->y() / CELL_H);
    int tmpCellIDx = (floorY * m_cols) + floorX;
    if (tmpCellIDx < 0 && tmpCellIDx >= m_resources.length())
        return;
    m_cellBarXpos = floorX * CELL_W;
    m_cellBarYpos = floorY * CELL_H;
    m_cellBarWidth = event->x() - m_cellBarXpos;
    m_hoverCellIdx = tmpCellIDx;
    update();
    //qDebug() << "Idx:" << m_hoverCellIdx << "X:" << m_cellBarXpos << "mX:" << event->x();
}

void ClickAndGoWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.drawImage(QPoint(0, 0), m_image);
    if (m_type == Preset)
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(76, 136, 255, 255)));
        painter.drawRect(m_cellBarXpos, m_cellBarYpos + 1, m_cellBarWidth, 3);
    }
}


ClickAndGoWidget::PresetResource::PresetResource(QString path, QString text, uchar min, uchar max)
{
    m_descr = text;
    m_min = min;
    m_max = max;
    QImage px(path);
    m_thumbnail = QImage(40, 40, QImage::Format_RGB32);
    QPainter painter(&m_thumbnail);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawImage(QRect(0,0,40,40), px);
    //qDebug() << "PATH: adding " << path << ", descr: " << text;
}

ClickAndGoWidget::PresetResource::PresetResource(QColor color1, QColor color2,
                                                 QString text, uchar min, uchar max)
{
    m_descr = text;
    m_min = min;
    m_max = max;
    m_thumbnail = QImage(40, 40, QImage::Format_RGB32);
    if (color2.isValid() == false)
        m_thumbnail.fill(color1.rgb());
    else
    {
        QPainter painter(&m_thumbnail);
        painter.fillRect(0, 0, 20, 40, color1);
        painter.fillRect(20, 0, 40, 40, color2);
    }
    //qDebug() << "COLOR: adding " << color1.name() << ", descr: " << text;
}

ClickAndGoWidget::PresetResource::PresetResource(int index, QString text, uchar min, uchar max)
{
    m_descr = text;
    m_min = min;
    m_max = max;
    m_thumbnail = QImage(40, 40, QImage::Format_RGB32);
    m_thumbnail.fill(Qt::white);
    QFont tfont = QApplication::font();
    tfont.setBold(true);
    tfont.setPixelSize(20);
    QPainter painter(&m_thumbnail);
    painter.setFont(tfont);
    painter.drawText(0, 0, 40, 40, Qt::AlignHCenter|Qt::AlignVCenter, QString("%1").arg(index));
    //qDebug() << "GENERIC: adding " << index << ", descr: " << text;
}


