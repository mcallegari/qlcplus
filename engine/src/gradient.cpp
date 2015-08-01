/*
  Q Light Controller Plus
  gradient.cpp

  Copyright (c) Giorgio Rebecchi

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

#include "gradient.h"

#include <QImage>
#include <QColor>
#include <QPainter>

QImage Gradient::m_rgb = QImage();

QImage Gradient::getRGBGradient()
{
    initialize();

    return m_rgb;
}

QColor Gradient::getRGBColor(const quint32 x, const quint32 y)
{
    initialize();

    return QColor(m_rgb.pixel (qMin(x, (quint32)255), qMin(y,(quint32)255)));
}

void Gradient::fillWithGradient(int r, int g, int b, QPainter *painter, int x)
{
    QColor top = Qt::black;
    QColor col(r, g , b);
    QColor bottom = Qt::white;

    QLinearGradient blackGrad(QPointF(0,0), QPointF(0, 127));
    blackGrad.setColorAt(0, top);
    blackGrad.setColorAt(1, col);
    QLinearGradient whiteGrad(QPointF(0,128), QPointF(0, 255));
    whiteGrad.setColorAt(0, col);
    whiteGrad.setColorAt(1, bottom);

    painter->fillRect(x, 0, x, 128, blackGrad);
    painter->fillRect(x, 128, x, 256, whiteGrad);
}

void Gradient::initialize ()
{
    if( m_rgb.isNull () == false )
        return;

    //m_rgb = QImage(252, 256, QImage::Format_RGB32);
    m_rgb = QImage(256, 256, QImage::Format_RGB32);

    int r = 0xFF;
    int g = 0;
    int b = 0;
    int x = 0;
    int i = 0;

    QPainter painter(&m_rgb);

    // R: 255  G:  0  B:   0
    for (i = x; i < x + 42; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        g+=6;
        if (g == 252) g = 255;
    }
    x+=42;

    // R: 255  G: 255  B:   0
    for (i = x; i < x + 42; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        r-=6;
        if (r < 6) r = 0;
    }
    x+=42;

    // R: 0  G: 255  B:  0
    for (i = x; i < x + 43; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        b+=6;
        if (b == 252) b = 255;
    }
    x+=43;

    // R: 0  G: 255  B:  255
    r=0; g=255;b=255;
    for (i = x; i < x + 43; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        g-=6;
        if (g < 6) g = 0;
    }
    x+=43;

    // R: 0  G:  0  B:  255
    r=0; g=0;b=255;
    for (i = x; i < x + 43; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        r+=6;
        if (r == 252) r = 255;
    }
    x+=43;

    // R: 255  G:  0  B:  255
    r=255; g=0;b=255;
    for (i = x; i < x + 43; i++)
    {
        fillWithGradient(r, g, b, &painter, i);
        b-=6;
        if (b < 6) b = 0;
    }
    x+=43;
    // R: 255  G:  0  B:  0
}


