/*
  Q Light Controller Plus
  gradient.h

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

#ifndef GRADIENT_H
#define GRADIENT_H

#include <QObject>

class QImage;
class QColor;
class QPainter;

class Gradient
{
public:
    /** Get a gradient of default size (252x256) */
    static QImage getRGBGradient();

    /** Get a gradient of input size */
    static QImage getRGBGradient(const int width, const int height);

private:
    static QImage m_rgb;

    static void initialize();
    static void fillWithGradient(int r, int g, int b, QPainter *painter, int x);
};

#endif // GRADIENT_H
