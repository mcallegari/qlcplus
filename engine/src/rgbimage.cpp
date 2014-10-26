/*
  Q Light Controller Plus
  rgbimage.cpp

  Copyright (c) Heikki Junnila
  Copyright (c) Jano Svitok

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
#include <QDebug>

#include "rgbimage.h"
#include "qlcmacros.h"
#include "doc.h"

#define KXMLQLCRGBImageFilename      "Filename"
#define KXMLQLCRGBImageAnimationStyle "Animation"
#define KXMLQLCRGBImageOffset         "Offset"
#define KXMLQLCRGBImageOffsetX        "X"
#define KXMLQLCRGBImageOffsetY        "Y"

RGBImage::RGBImage(const Doc * doc)
    : RGBAlgorithm(doc)
    , m_filename("")
    , m_animationStyle(Static)
    , m_xOffset(0)
    , m_yOffset(0)
{
}

RGBImage::RGBImage(const RGBImage& i)
    : RGBAlgorithm( i.doc())
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
        qDebug() << "[RGBImage] Failed to load" << m_filename;
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

    if (m_image.width() == 0 || m_image.height() == 0)
        return RGBMap();

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

int RGBImage::acceptColors() const
{
    return 0;
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
            setFilename(doc()->denormalizeComponentPath(tag.text()));
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
    QDomText filenameText =
       doc->createTextNode(this->doc()->normalizeComponentPath(m_filename));
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
