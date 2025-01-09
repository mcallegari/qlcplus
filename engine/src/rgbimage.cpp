/*
  Q Light Controller Plus
  rgbimage.cpp

  Copyright (c) Heikki Junnila
  Copyright (c) Jano Svitok
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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QPainter>
#include <QDebug>

#include "rgbimage.h"
#include "qlcmacros.h"
#include "doc.h"

#define KXMLQLCRGBImageFilename       QString("Filename")
#define KXMLQLCRGBImageAnimationStyle QString("Animation")
#define KXMLQLCRGBImageOffset         QString("Offset")
#define KXMLQLCRGBImageOffsetX        QString("X")
#define KXMLQLCRGBImageOffsetY        QString("Y")

RGBImage::RGBImage(Doc * doc)
    : RGBAlgorithm(doc)
    , m_filename("")
    , m_animatedSource(false)
    , m_animationStyle(Static)
    , m_xOffset(0)
    , m_yOffset(0)
{
}

RGBImage::RGBImage(const RGBImage& i)
    : RGBAlgorithm(i.doc())
    , m_filename(i.filename())
    , m_animatedSource(i.animatedSource())
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

void RGBImage::setImageData(int width, int height, const QByteArray &pixelData)
{
    QMutexLocker locker(&m_mutex);

    qDebug() << "[RGBImage] setting image data:" << width << height << pixelData.length();
    QImage newImg(width, height, QImage::Format_RGB888);
    newImg.fill(Qt::black);

    int i = 0;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (i + 3 > pixelData.length())
                break;
            QRgb pixel = qRgb((uchar)pixelData.at(i), (uchar)pixelData.at(i + 1), (uchar)pixelData.at(i + 2));
            newImg.setPixel(x, y, pixel);
            i+=3;
        }
    }
    m_image = newImg;
}

bool RGBImage::animatedSource() const
{
    return m_animatedSource;
}

void RGBImage::rewindAnimation()
{
    if (m_animatedSource)
        m_animatedPlayer.jumpToFrame(0);
}

void RGBImage::reloadImage()
{
    m_animatedSource = false;

    if (m_filename.isEmpty())
    {
        qDebug() << "[RGBImage] Empty image!";
        return;
    }

    QMutexLocker locker(&m_mutex);

    if (m_filename.endsWith(".gif"))
    {
        m_animatedPlayer.setFileName(m_filename);
        if (m_animatedPlayer.frameCount() > 1)
            m_animatedSource = true;
    }

    if (m_animatedSource == false)
    {
        if (!m_image.load(m_filename))
        {
            qDebug() << "[RGBImage] Failed to load" << m_filename;
            return;
        }
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
    QMutexLocker locker(&m_mutex);

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

void RGBImage::rgbMapSetColors(QVector<uint> &colors)
{
    Q_UNUSED(colors);
}

QVector<uint> RGBImage::rgbMapGetColors()
{
    return QVector<uint>();
}

void RGBImage::rgbMap(const QSize& size, uint rgb, int step, RGBMap &map)
{
    Q_UNUSED(rgb);

    QMutexLocker locker(&m_mutex);

    if (m_animatedSource == false && (m_image.width() == 0 || m_image.height() == 0))
        return;

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

    if (m_animatedSource)
    {
        m_animatedPlayer.jumpToNextFrame();
        m_image = m_animatedPlayer.currentImage().scaled(size);
    }

    map.resize(size.height());
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

bool RGBImage::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCRGBAlgorithm)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm node not found";
        return false;
    }

    if (root.attributes().value(KXMLQLCRGBAlgorithmType).toString() != KXMLQLCRGBImage)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm is not Image";
        return false;
    }

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCRGBImageFilename)
        {
            setFilename(doc()->denormalizeComponentPath(root.readElementText()));
        }
        else if (root.name() == KXMLQLCRGBImageAnimationStyle)
        {
            setAnimationStyle(stringToAnimationStyle(root.readElementText()));
        }
        else if (root.name() == KXMLQLCRGBImageOffset)
        {
            QString str;
            int value;
            bool ok;
            QXmlStreamAttributes attrs = root.attributes();

            str = attrs.value(KXMLQLCRGBImageOffsetX).toString();
            ok = false;
            value = str.toInt(&ok);
            if (ok == true)
                setXOffset(value);
            else
                qWarning() << Q_FUNC_INFO << "Invalid X offset:" << str;

            str = attrs.value(KXMLQLCRGBImageOffsetY).toString();
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
            qWarning() << Q_FUNC_INFO << "Unknown RGBImage tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool RGBImage::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCRGBAlgorithm);
    doc->writeAttribute(KXMLQLCRGBAlgorithmType, KXMLQLCRGBImage);

    doc->writeTextElement(KXMLQLCRGBImageFilename, this->doc()->normalizeComponentPath(m_filename));

    doc->writeTextElement(KXMLQLCRGBImageAnimationStyle, animationStyleToString(animationStyle()));

    doc->writeStartElement(KXMLQLCRGBImageOffset);
    doc->writeAttribute(KXMLQLCRGBImageOffsetX, QString::number(xOffset()));
    doc->writeAttribute(KXMLQLCRGBImageOffsetY, QString::number(yOffset()));
    doc->writeEndElement();

    /* End the <Algorithm> tag */
    doc->writeEndElement();

    return true;
}
