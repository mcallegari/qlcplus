/*
  Q Light Controller Plus
  clickandgoslider.h

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


#ifndef CLICKNGOSLIDER_H
#define CLICKNGOSLIDER_H

#include <QMouseEvent>
#include <QSlider>

/** @addtogroup ui UI
 * @{
 */

class ClickAndGoSlider : public QSlider
{
    Q_OBJECT
public:
    ClickAndGoSlider ( QWidget * parent = 0 );

protected:
    void mousePressEvent ( QMouseEvent * event );

signals:
    void controlClicked();
};

/** @} */

#endif

