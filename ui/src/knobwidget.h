/*
  Q Light Controller Plus
  knobwidget.h

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

#ifndef KNOBWIDGET_H
#define KNOBWIDGET_H

#include <QDial>

/** @addtogroup ui UI
 * @{
 */

class KnobWidget : public QDial
{
    Q_OBJECT
public:
    /** Constructor */
    KnobWidget(QWidget *parent = 0);

    /** Destructor */
    ~KnobWidget();

    void setEnabled(bool);

    void setColor(QColor color);

protected:
    void prepareCursor();
    void prepareBody();

    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent* e);

private:
    QPixmap rotatePix(QPixmap *p_pix, float p_deg);
private:
    QPixmap* m_background;
    QPixmap* m_cursor;
    QColor m_gradStartColor;
    QColor m_gradEndColor;
};

/** @} */

#endif
