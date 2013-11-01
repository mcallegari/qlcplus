/*
  Q Light Controller Plus
  rgbimage.cpp

  Copyright (c) Heikki Junnila
  Copyright (c) Jano Svitok

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

#include <QDomDocument>
#include <QDomElement>
#include <QPainter>
#include <QDebug>

#include "rgbimage.h"
#include "qlcmacros.h"

#define KXMLQLCRGBImageFilename      "Filename"
#define KXMLQLCRGBImageAnimationStyle "Animation"
#define KXMLQLCRGBImageOffset         "Offset"
#define KXMLQLCRGBImageOffsetX        "X"
#define KXMLQLCRGBImageOffsetY        "Y"

RGBImage::RGBImage()
    : RGBAlgorithm()
    , m_filename("")
    , m_animationStyle(Static)
    , m_xOffset(0)
    , m_yOffset(0)
{
}

RGBImage::RGBImage(const RGBImage& i)
    : RGBAlgorithm()
    , m_filename(i.filename())
    , m_animationStyle(i.animationStyle())
    , m_xOffset(i.xOffset())
    , m_yOffset(i.yOffset())
{
    reloadImage();
}

RGBImage::~RGBImage()
{
}

RGBAlgorithm* RGBImage::clone() const
{
    RGBImage* image = new RGBImage(*this);
    return static_cast<RGBAlgorithm*> (image);
}

/****************************************************************************
 * Image file
 ****************************************************************************/

void RGBImage::setFilename(const QString& filename)
{
    m_filename = filename;
    reloadImage();
}

QString RGBImage::filename() const
{
    return m_filename;
}

void RGBImage::reloadImage()
{
    if (m_filename.isEmpty())
    {
        qDebug() << "Empty image!";
        return;
    }

    if (!m_image.load(m_filename))
    {
        qDebug() << "Load failed!";
        return;
    }
}

/****************************************************************************
 * Animation
 ****************************************************************************/

void RGBImage::setAnimationStyle(RGBImage::AnimationStyle ani)
{
    if (ani >= Static && ani <= Animation)
        m_animationStyle = ani;
    else
        m_animationStyle = Static;
}

RGBImage::AnimationStyle RGBImage::animationStyle() const
{
    return m_animationStyle;
}

QString RGBImage::animationStyleToString(RGBImage::AnimationStyle ani)
{
    switch (ani)
    {
    default:
    case Static:
        return QString("Static");
    case Horizontal:
        return QString("Horizontal");
    case Vertical:
        return QString("Vertical");
    case Animation:
        return QString("Animation");
    }
}

RGBImage::AnimationStyle RGBImage::stringToAnimationStyle(const QString& str)
{
    if (str == QString("Horizontal"))
        return Horizontal;
    else if (str == QString("Vertical"))
        return Vertical;
    else if (str == QString("Animation"))
        return Animation;
    else
        return Static;
}

QStringList RGBImage::animationStyles()
{
    QStringList list;
    list << animationStyleToString(Static);
    list << animationStyleToString(Horizontal);
    list << animationStyleToString(Vertical);
    list << animationStyleToString(Animation);
    return list;
}

void RGBImage::setXOffset(int offset)
{
    m_xOffset = offset;
}

int RGBImage::xOffset() const
{
    return m_xOffset;
}

void RGBImage::setYOffset(int offset)
{
    m_yOffset = offset;
}

int RGBImage::yOffset() const
{
    return m_yOffset;
}

/****************************************************************************
 * RGBAlgorithm
 ****************************************************************************/

int RGBImage::rgbMapStepCount(const QSize& size)
{
    switch (animationStyle())
    {
    default:
    case Static:
        return 1;
    case Horizontal:
        return m_image.width();
    case Vertical:
        return m_image.height();
    case Animation:
        qDebug() << m_image.width() << " " << size.width() << " " << (m_image.width() / size.width());
        return MAX(1, m_image.width() / size.width());
    }
}

RGBMap RGBImage::rgbMap(const QSize& size, uint rgb, int step)
{
    Q_UNUSED(rgb);

    int xOffs = xOffset();
    int yOffs = yOffset();

    switch(animationStyle()) 
    {
    default:
    case Static:
        break;
    case Horizontal:
        xOffs += step;
        break;
    case Vertical:
        yOffs += step;
        break;
    case Animation:
        xOffs += step * size.width();
        break;
    }

    RGBMap map(size.height());
    for (int y = 0; y < size.height(); y++)
    {
        map[y].resize(size.width());
        for (int x = 0; x < size.width(); x++)
        {
            int x1 = (x + xOffs) % m_image.width();
            int y1 = (y + yOffs) % m_image.height();

            map[y][x] = m_image.pixel(x1,y1);
            if (qAlpha(map[y][x]) == 0)
                map[y][x] = 0;
        }
    }

    return map;
}

QString RGBImage::name() const
{
    return QString("Image");
}

QString RGBImage::author() const
{
    return QString("Jano Svitok");
}

int RGBImage::apiVersion() const
{
    return 1;
}

RGBAlgorithm::Type RGBImage::type() const
{
    return RGBAlgorithm::Image;
}

bool RGBImage::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCRGBAlgorithm)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm node not found";
        return false;
    }

    if (root.attribute(KXMLQLCRGBAlgorithmType) != KXMLQLCRGBImage)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm is not Image";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCRGBImageFilename)
        {
            setFilename(tag.text());
        }
        else if (tag.tagName() == KXMLQLCRGBImageAnimationStyle)
        {
            setAnimationStyle(stringToAnimationStyle(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCRGBImageOffset)
        {
            QString str;
            int value;
            bool ok;

            str = tag.attribute(KXMLQLCRGBImageOffsetX);
            ok = false;
            value = str.toInt(&ok);
            if (ok == true)
                setXOffset(value);
            else
                qWarning() << Q_FUNC_INFO << "Invalid X offset:" << str;

            str = tag.attribute(KXMLQLCRGBImageOffsetY);
            ok = false;
            value = str.toInt(&ok);
            if (ok == true)
                setYOffset(value);
            else
                qWarning() << Q_FUNC_INFO << "Invalid Y offset:" << str;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown RGBImage tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool RGBImage::saveXML(QDomDocument* doc, QDomElement* mtx_root) const
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(mtx_root != NULL);

    QDomElement root = doc->createElement(KXMLQLCRGBAlgorithm);
    root.setAttribute(KXMLQLCRGBAlgorithmType, KXMLQLCRGBImage);
    mtx_root->appendChild(root);

    QDomElement filename = doc->createElement(KXMLQLCRGBImageFilename);
    QDomText filenameText = doc->createTextNode(m_filename);
    filename.appendChild(filenameText);
    root.appendChild(filename);

    QDomElement ani = doc->createElement(KXMLQLCRGBImageAnimationStyle);
    QDomText aniText = doc->createTextNode(animationStyleToString(animationStyle()));
    ani.appendChild(aniText);
    root.appendChild(ani);

    QDomElement offset = doc->createElement(KXMLQLCRGBImageOffset);
    offset.setAttribute(KXMLQLCRGBImageOffsetX, xOffset());
    offset.setAttribute(KXMLQLCRGBImageOffsetY, yOffset());
    root.appendChild(offset);

    return true;
}
