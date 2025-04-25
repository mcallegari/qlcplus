/*
  Q Light Controller Plus
  rgbimage.h

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

#ifndef RGBIMAGE_H
#define RGBIMAGE_H

#include <QMutexLocker>
#include <QString>
#include <QMovie>
#include <QImage>

#include "rgbalgorithm.h"

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCRGBImage "Image"

class RGBImage : public RGBAlgorithm
{
public:
    RGBImage(Doc * doc);
    RGBImage(const RGBImage& t);
    ~RGBImage();

    /** @reimp */
    RGBAlgorithm* clone() const;

    /************************************************************************
     * Image file
     ************************************************************************/
public:
    /** Set filename of the image */
    void setFilename(const QString& fileName);

    /** Get filename of the image */
    QString filename() const;

    /** Set the image data from an array of RGB888 values */
    void setImageData(int width, int height, const QByteArray& pixelData);

    bool animatedSource() const;
    void rewindAnimation();

private:
    void reloadImage();

protected slots:
    void frameChanged(int num);

private:
    QString m_filename;
    bool m_animatedSource;
    QMovie m_animatedPlayer;
    QImage m_image;
    QMutex m_mutex;

    /************************************************************************
     * Animation
     ************************************************************************/
public:
    enum AnimationStyle { Static, Horizontal, Vertical, Animation };

    void setAnimationStyle(AnimationStyle ani);
    AnimationStyle animationStyle() const;

    static QString animationStyleToString(AnimationStyle ani);
    static AnimationStyle stringToAnimationStyle(const QString& str);
    static QStringList animationStyles();

    void setXOffset(int offset);
    int xOffset() const;

    void setYOffset(int offset);
    int yOffset() const;

private:
    AnimationStyle m_animationStyle;
    int m_xOffset;
    int m_yOffset;

    /************************************************************************
     * RGBAlgorithm
     ************************************************************************/
public:
    /** @reimp */
    int rgbMapStepCount(const QSize& size);

    /** @reimp */
    void rgbMapSetColors(QVector<uint> &colors);

    /** @reimp */
    QVector<uint> rgbMapGetColors();

    /** @reimp */
    void rgbMap(const QSize& size, uint rgb, int step, RGBMap &map);

    /** @reimp */
    QString name() const;

    /** @reimp */
    QString author() const;

    /** @reimp */
    int apiVersion() const;

    /** @reimp */
    RGBAlgorithm::Type type() const;

    /** @reimp */
    int acceptColors() const;

    /** @reimp */
    bool loadXML(QXmlStreamReader &root);

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc) const;
};

/** @} */

#endif
