/*
  Q Light Controller
  rgbtext.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QPainter>
#include <QImage>
#include <QDebug>

#include "rgbtext.h"

#define KXMLQLCRGBTextContent        "Content"
#define KXMLQLCRGBTextFont           "Font"
#define KXMLQLCRGBTextAnimationStyle "Animation"
#define KXMLQLCRGBTextOffset         "Offset"
#define KXMLQLCRGBTextOffsetX        "X"
#define KXMLQLCRGBTextOffsetY        "Y"

RGBText::RGBText(const Doc * doc)
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
    else
        return fm.width(m_text);
}

RGBMap RGBText::renderScrollingText(const QSize& size, uint rgb, int step) const
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
    RGBMap map(size.height());
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

    return map;
}

RGBMap RGBText::renderStaticLetters(const QSize& size, uint rgb, int step) const
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

    RGBMap map(size.height());
    for (int y = 0; y < size.height(); y++)
    {
        map[y].resize(size.width());
        for (int x = 0; x < size.width(); x++)
            map[y][x] = image.pixel(x, y);
    }

    return map;
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

RGBMap RGBText::rgbMap(const QSize& size, uint rgb, int step)
{
    if (animationStyle() == StaticLetters)
        return renderStaticLetters(size, rgb, step);
    else
        return renderScrollingText(size, rgb, step);
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

bool RGBText::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCRGBAlgorithm)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm node not found";
        return false;
    }

    if (root.attribute(KXMLQLCRGBAlgorithmType) != KXMLQLCRGBText)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm is not Text";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCRGBTextContent)
        {
            setText(tag.text());
        }
        else if (tag.tagName() == KXMLQLCRGBTextFont)
        {
            QFont font;
            if (font.fromString(tag.text()) == true)
                setFont(font);
            else
                qWarning() << Q_FUNC_INFO << "Invalid font:" << tag.text();
        }
        else if (tag.tagName() == KXMLQLCRGBTextAnimationStyle)
        {
            setAnimationStyle(stringToAnimationStyle(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCRGBTextOffset)
        {
            QString str;
            int value;
            bool ok;

            str = tag.attribute(KXMLQLCRGBTextOffsetX);
            ok = false;
            value = str.toInt(&ok);
            if (ok == true)
                setXOffset(value);
            else
                qWarning() << Q_FUNC_INFO << "Invalid X offset:" << str;

            str = tag.attribute(KXMLQLCRGBTextOffsetY);
            ok = false;
            value = str.toInt(&ok);
            if (ok == true)
                setYOffset(value);
            else
                qWarning() << Q_FUNC_INFO << "Invalid Y offset:" << str;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown RGBText tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool RGBText::saveXML(QDomDocument* doc, QDomElement* mtx_root) const
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(mtx_root != NULL);

    QDomElement root = doc->createElement(KXMLQLCRGBAlgorithm);
    root.setAttribute(KXMLQLCRGBAlgorithmType, KXMLQLCRGBText);
    mtx_root->appendChild(root);

    QDomElement content = doc->createElement(KXMLQLCRGBTextContent);
    QDomText contentText = doc->createTextNode(m_text);
    content.appendChild(contentText);
    root.appendChild(content);

    QDomElement font = doc->createElement(KXMLQLCRGBTextFont);
    QDomText fontText = doc->createTextNode(m_font.toString());
    font.appendChild(fontText);
    root.appendChild(font);

    QDomElement ani = doc->createElement(KXMLQLCRGBTextAnimationStyle);
    QDomText aniText = doc->createTextNode(animationStyleToString(animationStyle()));
    ani.appendChild(aniText);
    root.appendChild(ani);

    QDomElement offset = doc->createElement(KXMLQLCRGBTextOffset);
    offset.setAttribute(KXMLQLCRGBTextOffsetX, xOffset());
    offset.setAttribute(KXMLQLCRGBTextOffsetY, yOffset());
    root.appendChild(offset);

    return true;
}
