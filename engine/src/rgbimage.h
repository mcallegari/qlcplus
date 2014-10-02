/*
  Q Light Controller Plus
  rgbimage.h

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

#ifndef RGBIMAGE_H
#define RGBIMAGE_H

#include <QString>
#include <QImage>

#include "rgbalgorithm.h"

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCRGBImage "Image"

class RGBImage : public RGBAlgorithm
{
public:
    RGBImage(const Doc * doc);
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

private:

    void reloadImage();

private:
    QString m_filename;
    QImage m_image;

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
    RGBMap rgbMap(const QSize& size, uint rgb, int step);

    /** @reimp */
    QString name() const;

    /** @reimp */
    QString author() const;

    /** @reimp */
    int apiVersion() const;

    /** @reimp */
    RGBAlgorithm::Type type() const;

    /** @reimp */
    bool loadXML(const QDomElement& root);

    /** @reimp */
    bool saveXML(QDomDocument* doc, QDomElement* mtx_root) const;
};

/** @} */

#endif
