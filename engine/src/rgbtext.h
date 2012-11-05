/*
  Q Light Controller
  rgbtext.h

  Copyright (c) Heikki Junnila

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

#ifndef RGBTEXT_H
#define RGBTEXT_H

#include <QString>
#include <QFont>

#include "rgbalgorithm.h"

#define KXMLQLCRGBText               "Text"

class RGBText : public RGBAlgorithm
{
public:
    RGBText();
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
    RGBMap renderScrollingText(const QSize& size, uint rgb, int step) const;
    RGBMap renderStaticLetters(const QSize& size, uint rgb, int step) const;

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
