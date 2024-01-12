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

#include <QImage>
#include <QColor>
#include <QPainter>
#include <QLinearGradient>

#include "gradient.h"

QImage Gradient::m_rgb = QImage();

QImage Gradient::getRGBGradient()
{
    initialize();

    return m_rgb;
}

QImage Gradient::getRGBGradient(const int width, const int height)
{
    initialize();

    return m_rgb.scaled (width, height);
}

void Gradient::fillWithGradient(int r, int g, int b, QPainter *painter, int x)
{
    QColor top = Qt::black;
    QColor col(r, g , b);
    QColor bottom = Qt::white;

    QLinearGradient blackGrad(QPointF(0, 0), QPointF(0, 127));
    blackGrad.setColorAt(0, top);
    blackGrad.setColorAt(1, col);
    QLinearGradient whiteGrad(QPointF(0, 128), QPointF(0, 255));
    whiteGrad.setColorAt(0, col);
    whiteGrad.setColorAt(1, bottom);

    painter->fillRect(x, 0, x + 1, 128, blackGrad);
    painter->fillRect(x, 128, x + 1, 256, whiteGrad);
}

void Gradient::initialize()
{
    if (m_rgb.isNull() == false)
        return;

    m_rgb = QImage(256, 256, QImage::Format_RGB32);
    QPainter painter(&m_rgb);

    int x = 0;

    QList<int> baseColors;
    baseColors << 0xFF0000 << 0xFFFF00 << 0x00FF00 << 0x00FFFF << 0x0000FF << 0xFF00FF << 0xFF0000;

    for (int c = 0; c < 6; c++)
    {
        float r = (baseColors[c] >> 16) & 0x00FF;
        float g = (baseColors[c] >> 8) & 0x00FF;
        float b = baseColors[c] & 0x00FF;
        int nr = (baseColors[c + 1] >> 16) & 0x00FF;
        int ng = (baseColors[c + 1] >> 8) & 0x00FF;
        int nb = baseColors[c + 1] & 0x00FF;
        float rD = (nr - r) / 42;
        float gD = (ng - g) / 42;
        float bD = (nb - b) / 42;

        for (int i = x; i < x + 42; i++)
        {
            fillWithGradient(r, g, b, &painter, i);
            r+=rD;
            g+=gD;
            b+=bD;
        }
        x+=42;
    }
}


