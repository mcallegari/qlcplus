/*
  Q Light Controller Plus
  rgbtext.cpp

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
#include <QPainter>
#include <QImage>
#include <QDebug>

#include "rgbtext.h"

#define KXMLQLCRGBTextContent        QString("Content")
#define KXMLQLCRGBTextFont           QString("Font")
#define KXMLQLCRGBTextAnimationStyle QString("Animation")
#define KXMLQLCRGBTextOffset         QString("Offset")
#define KXMLQLCRGBTextOffsetX        QString("X")
#define KXMLQLCRGBTextOffsetY        QString("Y")

RGBText::RGBText(Doc * doc)
    : RGBAlgorithm(doc)
    , m_text(" Q LIGHT CONTROLLER + ")
    , m_animationStyle(Horizontal)
    , m_xOffset(0)
    , m_yOffset(0)
{
}

RGBText::RGBText(const RGBText& t)
    : RGBAlgorithm(t.doc())
    , m_text(t.text())
    , m_font(t.font())
    , m_animationStyle(t.animationStyle())
    , m_xOffset(t.xOffset())
    , m_yOffset(t.yOffset())
{
}

RGBText::~RGBText()
{
}

RGBAlgorithm* RGBText::clone() const
{
    RGBText* txt = new RGBText(*this);
    return static_cast<RGBAlgorithm*> (txt);
}

/****************************************************************************
 * Text & Font
 ****************************************************************************/

void RGBText::setText(const QString& str)
{
    m_text = str;
}

QString RGBText::text() const
{
    return m_text;
}

void RGBText::setFont(const QFont& font)
{
    m_font = font;
}

QFont RGBText::font() const
{
    return m_font;
}

/****************************************************************************
 * Animation
 ****************************************************************************/

void RGBText::setAnimationStyle(RGBText::AnimationStyle ani)
{
    if (ani >= StaticLetters && ani <= Vertical)
        m_animationStyle = ani;
    else
        m_animationStyle = StaticLetters;
}

RGBText::AnimationStyle RGBText::animationStyle() const
{
    return m_animationStyle;
}

QString RGBText::animationStyleToString(RGBText::AnimationStyle ani)
{
    switch (ani)
    {
    default:
    case StaticLetters:
        return QString("Letters");
    case Horizontal:
        return QString("Horizontal");
    case Vertical:
        return QString("Vertical");
    }
}

RGBText::AnimationStyle RGBText::stringToAnimationStyle(const QString& str)
{
    if (str == QString("Horizontal"))
        return Horizontal;
    else if (str == QString("Vertical"))
        return Vertical;
    else
        return StaticLetters;
}

QStringList RGBText::animationStyles()
{
    QStringList list;
    list << animationStyleToString(StaticLetters);
    list << animationStyleToString(Horizontal);
    list << animationStyleToString(Vertical);
    return list;
}

void RGBText::setXOffset(int offset)
{
    m_xOffset = offset;
}

int RGBText::xOffset() const
{
    return m_xOffset;
}

void RGBText::setYOffset(int offset)
{
    m_yOffset = offset;
}

int RGBText::yOffset() const
{
    return m_yOffset;
}

int RGBText::scrollingTextStepCount() const
{
    QFontMetrics fm(m_font);
    if (animationStyle() == Vertical)
        return m_text.length() * fm.ascent();
    else{
#if (QT_VERSION < QT_VERSION_CHECK(5, 11, 0))
        return fm.width(m_text);
#else
        return fm.horizontalAdvance(m_text);
#endif
    }
}

void RGBText::renderScrollingText(const QSize& size, uint rgb, int step, RGBMap &map) const
{
    QImage image;
    if (animationStyle() == Horizontal)
        image = QImage(scrollingTextStepCount(), size.height(), QImage::Format_RGB32);
    else
        image = QImage(size.width(), scrollingTextStepCount(), QImage::Format_RGB32);
    image.fill(QRgb(0));

    QPainter p(&image);
    p.setRenderHint(QPainter::TextAntialiasing, false);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setFont(m_font);
    p.setPen(QColor(rgb));

    if (animationStyle() == Vertical)
    {
        QFontMetrics fm(m_font);
        QRect rect(0, 0, image.width(), image.height());

        for (int i = 0; i < m_text.length(); i++)
        {
            rect.setY((i * fm.ascent()) + yOffset());
            rect.setX(xOffset());
            rect.setHeight(fm.ascent());
            p.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, m_text.mid(i, 1));
        }
    }
    else
    {
        // Draw the whole text each time
        QRect rect(xOffset(), yOffset(), image.width(), image.height());
        p.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, m_text);
    }
    p.end();

    // Treat the RGBMap as a "window" on top of the fully-drawn text and pick the
    // correct pixels according to $step.
    map.resize(size.height());
    for (int y = 0; y < size.height(); y++)
    {
        map[y].resize(size.width());
        for (int x = 0; x < size.width(); x++)
        {
            if (animationStyle() == Horizontal)
            {
                if (step + x < image.width())
                    map[y][x] = image.pixel(step + x, y);
            }
            else
            {
                if (step + y < image.height())
                    map[y][x] = image.pixel(x, step + y);
            }
        }
    }
}

void RGBText::renderStaticLetters(const QSize& size, uint rgb, int step, RGBMap &map) const
{
    QImage image(size, QImage::Format_RGB32);
    image.fill(QRgb(0));

    QPainter p(&image);
    p.setRenderHint(QPainter::TextAntialiasing, false);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setFont(m_font);
    p.setPen(QColor(rgb));

    // Draw one letter at a time
    QRect rect(xOffset(), yOffset(), size.width(), size.height());
    p.drawText(rect, Qt::AlignCenter, m_text.mid(step, 1));
    p.end();

    map.resize(size.height());
    for (int y = 0; y < size.height(); y++)
    {
        map[y].resize(size.width());
        for (int x = 0; x < size.width(); x++)
            map[y][x] = image.pixel(x, y);
    }
}

/****************************************************************************
 * RGBAlgorithm
 ****************************************************************************/

int RGBText::rgbMapStepCount(const QSize& size)
{
    Q_UNUSED(size);
    if (animationStyle() == StaticLetters)
        return m_text.length();
    else
        return scrollingTextStepCount();
}

void RGBText::rgbMapSetColors(QVector<uint> &colors)
{
    Q_UNUSED(colors);
}

QVector<uint> RGBText::rgbMapGetColors()
{
    return QVector<uint>();
}

void RGBText::rgbMap(const QSize& size, uint rgb, int step, RGBMap &map)
{
    if (animationStyle() == StaticLetters)
        renderStaticLetters(size, rgb, step, map);
    else
        renderScrollingText(size, rgb, step, map);
}

QString RGBText::name() const
{
    return QString("Text");
}

QString RGBText::author() const
{
    return QString("Heikki Junnila");
}

int RGBText::apiVersion() const
{
    return 1;
}

RGBAlgorithm::Type RGBText::type() const
{
    return RGBAlgorithm::Text;
}

int RGBText::acceptColors() const
{
    return 2; // start and end colors accepted
}

bool RGBText::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCRGBAlgorithm)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm node not found";
        return false;
    }

    if (root.attributes().value(KXMLQLCRGBAlgorithmType).toString() != KXMLQLCRGBText)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm is not Text";
        return false;
    }

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCRGBTextContent)
        {
            setText(root.readElementText());
        }
        else if (root.name() == KXMLQLCRGBTextFont)
        {
            QFont font;
            QString fontName = root.readElementText();
            if (font.fromString(fontName) == true)
                setFont(font);
            else
                qWarning() << Q_FUNC_INFO << "Invalid font:" << fontName;
        }
        else if (root.name() == KXMLQLCRGBTextAnimationStyle)
        {
            setAnimationStyle(stringToAnimationStyle(root.readElementText()));
        }
        else if (root.name() == KXMLQLCRGBTextOffset)
        {
            QString str;
            int value;
            bool ok;
            QXmlStreamAttributes attrs = root.attributes();

            str = attrs.value(KXMLQLCRGBTextOffsetX).toString();
            ok = false;
            value = str.toInt(&ok);
            if (ok == true)
                setXOffset(value);
            else
                qWarning() << Q_FUNC_INFO << "Invalid X offset:" << str;

            str = attrs.value(KXMLQLCRGBTextOffsetY).toString();
            ok = false;
            value = str.toInt(&ok);
            if (ok == true)
                setYOffset(value);
            else
                qWarning() << Q_FUNC_INFO << "Invalid Y offset:" << str;
            root.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown RGBText tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool RGBText::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCRGBAlgorithm);
    doc->writeAttribute(KXMLQLCRGBAlgorithmType, KXMLQLCRGBText);

    doc->writeTextElement(KXMLQLCRGBTextContent, m_text);

    doc->writeTextElement(KXMLQLCRGBTextFont, m_font.toString());

    doc->writeTextElement(KXMLQLCRGBTextAnimationStyle, animationStyleToString(animationStyle()));

    doc->writeStartElement(KXMLQLCRGBTextOffset);
    doc->writeAttribute(KXMLQLCRGBTextOffsetX, QString::number(xOffset()));
    doc->writeAttribute(KXMLQLCRGBTextOffsetY, QString::number(yOffset()));
    doc->writeEndElement();

    /* End the <Algorithm> tag */
    doc->writeEndElement();

    return true;
}
