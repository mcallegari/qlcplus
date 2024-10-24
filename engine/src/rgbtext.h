/*
  Q Light Controller Plus
  rgbtext.h

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

#ifndef RGBTEXT_H
#define RGBTEXT_H

#include <QString>
#include <QFont>

#include "rgbalgorithm.h"

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCRGBText "Text"

class RGBText : public RGBAlgorithm
{
public:
    RGBText(Doc * doc);
    RGBText(const RGBText& t);
    ~RGBText();

    /** @reimp */
    RGBAlgorithm* clone() const;

    /************************************************************************
     * Text & Font
     ************************************************************************/
public:
    /** Set the text to be rendered */
    void setText(const QString& str);

    /** Get the text to be rendered */
    QString text() const;

    /** Set the font with which to render the text */
    void setFont(const QFont& font);

    /** Get the font with which to render the text */
    QFont font() const;

private:
    QString m_text;
    QFont m_font;

    /************************************************************************
     * Animation
     ************************************************************************/
public:
    enum AnimationStyle { StaticLetters, Horizontal, Vertical };

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
    int scrollingTextStepCount() const;
    void renderScrollingText(const QSize& size, uint rgb, int step, RGBMap &map) const;
    void renderStaticLetters(const QSize& size, uint rgb, int step, RGBMap &map) const;

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
