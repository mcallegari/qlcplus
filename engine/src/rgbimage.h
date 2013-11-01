/*
  Q Light Controller Plus
  rgbimage.h

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

#ifndef RGBIMAGE_H
#define RGBIMAGE_H

#include <QString>
#include <QImage>

#include "rgbalgorithm.h"

#define KXMLQLCRGBImage               "Image"


class RGBImage : public RGBAlgorithm
{
public:
    RGBImage();
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

#endif
