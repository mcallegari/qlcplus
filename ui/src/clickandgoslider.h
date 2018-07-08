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

#define SLIDER_SS_COMMON  \
    "QSlider::groove:vertical { background: transparent; position: absolute; left: 4px; right: 4px; } " \
    "QSlider::sub-page:vertical { background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #888, stop: 1 #ddd );" \
    "border: 1px solid #8E8A86; margin: 0 9px; }" \
    "QSlider::handle:vertical:disabled { " \
    "background: QLinearGradient(x1:0, y1:0, x2:0, y2:1, stop:0 #eee, stop:0.45 #999, stop:0.50 #555, stop:0.55 #999, stop:1 #aaa);" \
    "border: 1px solid #666; }"

const QString CNG_DEFAULT_STYLE =
    SLIDER_SS_COMMON

    "QSlider::handle:vertical { "
    "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ddd, stop:0.45 #888, stop:0.50 #000, stop:0.55 #888, stop:1 #999);"
    "border: 1px solid #5c5c5c;"
    "border-radius: 4px; margin: 0 -4px; height: 20px; }"

    "QSlider::handle:vertical:hover {"
    "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #eee, stop:0.45 #999, stop:0.50 #ff0000, stop:0.55 #999, stop:1 #ccc);"
    "border: 1px solid #000; }"

    "QSlider::add-page:vertical { background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #78d, stop: 1 #97CDEC );"
    "border: 1px solid #5288A7; margin: 0 9px; }";

class ClickAndGoSlider : public QSlider
{
    Q_OBJECT
public:
    ClickAndGoSlider(QWidget *parent = 0);
    void setSliderStyleSheet(const QString& styleSheet);
    void setShadowLevel(int level);

protected:
    void mousePressEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void paintEvent(QPaintEvent *e);
    void showEvent(QShowEvent *e);

signals:
    void controlClicked();

private:
    QString m_styleSheet;
    float m_shadowLevel;
};

/** @} */

#endif

