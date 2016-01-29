/*
  Q Light Controller
  rgbitem.h

  Copyright (c) Heikki Junnila

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

#ifndef RGBITEM_H
#define RGBITEM_H

#include <QAbstractGraphicsShapeItem>
#include <QColor>

/** @addtogroup ui_functions
 * @{
 */

class RGBItem
{
public:
    RGBItem(QAbstractGraphicsShapeItem* graphicsItem);

    void setColor(QRgb rgb);
    QRgb color() const;

    void draw(uint elapsedMs, uint targetMs);

    QAbstractGraphicsShapeItem* graphicsItem() const;

private:
    QColor m_color;
    QColor m_oldColor;
    uint m_elapsed;
    QScopedPointer<QAbstractGraphicsShapeItem> m_graphicsItem;
};

/** @} */

#endif
