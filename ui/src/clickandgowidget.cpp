/*
  Q Light Controller Plus
  clickandgowidget.cpp

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

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <qmath.h>
#include <QDebug>
#include <QImage>

#include "clickandgowidget.h"
#include "qlccapability.h"
#include "qlcmacros.h"
#include "gradient.h"

#define CELL_W  150
#define CELL_H  45
#define TITLE_H  18

ClickAndGoWidget::ClickAndGoWidget(QWidget *parent) :
    QWidget(parent)
{
    // This makes the application crash when a clickAndGoWidget
    // is created in a QDialog.
    //    setAttribute(Qt::WA_StaticContents);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setMouseTracking(true);

    m_type = None;
    m_linearColor = false;
    m_width = 10;
    m_height = 10;
    m_cols = 0;
    m_rows = 0;
    m_cellWidth = CELL_W;
    m_hoverCellIdx = -1;
    m_cellBarXpos = 1;
    m_cellBarYpos = 1;
    m_cellBarWidth = 0;
    m_levelLowLimit = 0;
    m_levelHighLimit = 255;
}

void ClickAndGoWidget::setupGradient(QColor begin, QColor end)
{
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

void ClickAndGoWidget::setupColorPicker()
{
    int cw = 15;

    m_width = 256 + 30;
    m_height = 256;
    m_image = QImage(m_width, m_height, QImage::Format_RGB32);
    QPainter painter(&m_image);

    // Draw 16 default color squares
    painter.fillRect(0, 0, cw, 32, QColor(Qt::white));
    painter.fillRect(cw, 0, cw, 32, QColor(Qt::black));
    painter.fillRect(0, 32, cw, 64, QColor(Qt::red));
    painter.fillRect(cw, 32, cw, 64, QColor(Qt::darkRed));
    painter.fillRect(0, 64, cw, 96, QColor(Qt::green));
    painter.fillRect(cw, 64, cw, 96, QColor(Qt::darkGreen));
    painter.fillRect(0, 96, cw, 128, QColor(Qt::blue));
    painter.fillRect(cw, 96, cw, 128, QColor(Qt::darkBlue));
    painter.fillRect(0, 128, cw, 160, QColor(Qt::cyan));
    painter.fillRect(cw, 128, cw, 160, QColor(Qt::darkCyan));
    painter.fillRect(0, 160, cw, 192, QColor(Qt::magenta));
    painter.fillRect(cw, 160, cw, 192, QColor(Qt::darkMagenta));
    painter.fillRect(0, 192, cw, 224, QColor(Qt::yellow));
    painter.fillRect(cw, 192, cw, 224, QColor(Qt::darkYellow));
    painter.fillRect(0, 224, cw, 256, QColor(Qt::gray));
    painter.fillRect(cw, 224, cw, 256, QColor(Qt::darkGray));

    painter.drawImage(cw * 2, 0, Gradient::getRGBGradient());
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
        setupGradient(Qt::black, Qt::red);
    else if (type == Green)
        setupGradient(Qt::black, Qt::green);
    else if (type == Blue)
        setupGradient(Qt::black, Qt::blue);
    else if (type == Cyan)
        setupGradient(Qt::white, Qt::cyan);
    else if (type == Magenta)
        setupGradient(Qt::white, Qt::magenta);
    else if (type == Yellow)
        setupGradient(Qt::white, Qt::yellow);
    else if (type == Amber)
        setupGradient(Qt::black, 0xFFFF7E00);
    else if (type == White)
        setupGradient(Qt::black, Qt::white);
    else if (type == UV)
        setupGradient(Qt::black, 0xFF9400D3);
    else if (type == Lime)
        setupGradient(Qt::black, 0xFFADFF2F);
    else if (type == Indigo)
        setupGradient(Qt::black, 0xFF4B0082);
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

void ClickAndGoWidget::setLevelLowLimit(int min)
{
    this->m_levelLowLimit = min;
}

void ClickAndGoWidget::setLevelHighLimit(int max)
{
    this->m_levelHighLimit = max;
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
        case Amber: return "Amber"; break;
        case White: return "White"; break;
        case UV: return "UV"; break;
        case Lime: return "Lime"; break;
        case Indigo: return "Indigo"; break;
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
    else if (str == "Amber") return Amber;
    else if (str == "White") return White;
    else if (str == "UV") return UV;
    else if (str == "Lime") return Lime;
    else if (str == "Indigo") return Indigo;
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
        foreach (PresetResource res, m_resources)
        {
            if (value >= res.m_resLowLimit && value <= res.m_resHighLimit)
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

    m_title = chan->name();
    m_resources.clear();

    //qDebug() << Q_FUNC_INFO << "cap #" << chan->capabilities().size();

    foreach (QLCCapability* cap, chan->capabilities())
    {
        if (cap->presetType() == QLCCapability::Picture)
        {
            m_resources.append(PresetResource(cap->resource(0).toString(), cap->name(),
                                              cap->min(), cap->max()));
        }
        else if (cap->presetType() == QLCCapability::SingleColor)
        {
            QColor col1 = cap->resource(0).value<QColor>();
            m_resources.append(PresetResource(col1, QColor(), cap->name(), cap->min(), cap->max()));
        }
        else if (cap->presetType() == QLCCapability::DoubleColor)
        {
            QColor col1 = cap->resource(0).value<QColor>();
            QColor col2 = cap->resource(1).value<QColor>();
            m_resources.append(PresetResource(col1, col2, cap->name(), cap->min(), cap->max()));
        }
        else
        {
            m_resources.append(PresetResource(i, cap->name(), cap->min(), cap->max()));
        }
        i++;
    }
}

void ClickAndGoWidget::setupPresetPicker()
{
    if (m_resources.size() == 0)
        return;

    QScreen *scr = QGuiApplication::screens().first();
    QRect screen = scr->availableGeometry();

    m_cols = 2;
    m_rows = qCeil((qreal)m_resources.size() / 2);
    m_width = m_cellWidth * m_cols;
    m_height = CELL_H * m_rows + TITLE_H;

    // first check if the menu fits vertically
    if (m_height > screen.height())
    {
        m_rows = qFloor((qreal)screen.height() / CELL_H);
        m_cols = qCeil((qreal)m_resources.size() / m_rows);
        m_width = m_cellWidth * m_cols;
        m_height = CELL_H * m_rows + TITLE_H;
    }

    // then check if it has to be rescaled horizontally
    if (m_width > screen.width())
    {
        m_cellWidth = screen.width() / m_cols;
        m_width = m_cellWidth * m_cols;
    }

    int x = 0;
    int y = 0;
    m_image = QImage(m_width, m_height, QImage::Format_RGB32);
    QPainter painter(&m_image);
    painter.setRenderHint(QPainter::Antialiasing);
    QLinearGradient presetGrad(QPointF(0,0), QPointF(0, m_height));
    presetGrad.setColorAt(0, QApplication::palette().window().color());
    presetGrad.setColorAt(1, QColor(173, 171, 179));
    painter.fillRect(0, 0, m_width, m_height, presetGrad);

    // title
    painter.setPen(Qt::black);
    painter.drawText(x + 3, y, m_width - 3, TITLE_H, Qt::AlignVCenter | Qt::TextSingleLine, m_title);
    y += TITLE_H;

    for (int i = 0; i < m_resources.size(); i++)
    {
        PresetResource res = m_resources.at(i);
        if (res.m_resLowLimit > m_levelHighLimit || res.m_resHighLimit < m_levelLowLimit)
            continue;
        painter.setPen(Qt::black);
        painter.drawRect(x, y, m_cellWidth, CELL_H);
        painter.drawImage(x + 1, y + 4, res.m_thumbnail);
        painter.drawText(x + 43, y + 4, m_cellWidth - 42, CELL_H - 5, Qt::TextWordWrap|Qt::AlignVCenter, res.m_descr);
        if (i % m_cols == m_cols - 1)
        {
            y += CELL_H;
            x = 0;
        }
        else
            x += m_cellWidth;

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
        if (event->pos().x() <= 10)
            emit levelChanged(0);
        else if (event->pos().x() > 10 && event->pos().x() < 256)
            emit levelChanged((uchar)(event->pos().x() - 10));
        else
            emit levelChanged(255);
    }
    else if (m_type == RGB || m_type == CMY)
    {
        emit colorChanged(m_image.pixel(event->pos().x(), event->pos().y()));
    }
    else if (m_type == Preset)
    {
        if (m_hoverCellIdx >= 0 && m_hoverCellIdx < m_resources.length())
        {
            PresetResource res = m_resources.at(m_hoverCellIdx);
            qDebug() << "Mouse press. cellW: " << m_cellBarWidth << "min: " << res.m_resLowLimit << "max:" << res.m_resHighLimit;

            float f = SCALE(float(m_cellBarWidth),
                        float(0),
                        float(m_cellWidth),
                        float(0), float(res.m_resHighLimit - res.m_resLowLimit));
            emit levelAndPresetChanged((uchar)f + res.m_resLowLimit, res.m_thumbnail);
        }
    }
    QWidget::mousePressEvent(event);
}

void ClickAndGoWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_linearColor == true && event->buttons() == Qt::LeftButton)
    {
        if (event->pos().x() <= 10)
            emit levelChanged(0);
        else if (event->pos().x() > 10 && event->pos().x() < 256)
            emit levelChanged((uchar)(event->pos().x() - 10));
        else
            emit levelChanged(255);
    }
    else if ((m_type == RGB || m_type == CMY) && event->buttons() == Qt::LeftButton)
    {
        emit colorChanged(m_image.pixel(event->pos().x(), event->pos().y()));
    }
    else if (m_type == Preset)
    {
        // calculate the index of the resource where the cursor is
        int floorX = qFloor(event->pos().x() / m_cellWidth);
        int floorY = qFloor((event->pos().y() - TITLE_H) / CELL_H);
        int tmpCellIDx = (floorY * m_cols) + floorX;
        if (event->pos().y() < TITLE_H || tmpCellIDx < 0 || tmpCellIDx >= m_resources.length())
        {
            m_hoverCellIdx = -1;
            update();
            return;
        }
        m_cellBarXpos = floorX * m_cellWidth;
        m_cellBarYpos = floorY * CELL_H + TITLE_H;
        m_cellBarWidth = event->pos().x() - m_cellBarXpos;
        m_hoverCellIdx = tmpCellIDx;
        update();
        qDebug() << "Idx:" << m_hoverCellIdx << "X:" << m_cellBarXpos << "mX:" << event->pos().x();
    }
}

void ClickAndGoWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.drawImage(QPoint(0, 0), m_image);
    if (m_type == Preset && m_hoverCellIdx >= 0)
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(76, 136, 255, 255)));
        painter.drawRect(m_cellBarXpos, m_cellBarYpos + 1, m_cellBarWidth, 3);
    }
}


ClickAndGoWidget::PresetResource::PresetResource(QString path, QString text, uchar min, uchar max)
{
    m_descr = text;
    m_resLowLimit = min;
    m_resHighLimit = max;
    QImage px(path);
    m_thumbnail = QImage(40, 40, QImage::Format_RGB32);
    m_thumbnail.fill(Qt::white);
    QPainter painter(&m_thumbnail);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawImage(QRect(0,0,40,40), px);
    //qDebug() << "PATH: adding " << path << ", descr: " << text;
}

ClickAndGoWidget::PresetResource::PresetResource(QColor color1, QColor color2,
                                                 QString text, uchar min, uchar max)
{
    m_descr = text;
    m_resLowLimit = min;
    m_resHighLimit = max;
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
    m_resLowLimit = min;
    m_resHighLimit = max;
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


