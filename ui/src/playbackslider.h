/*
  Q Light Controller
  playbackslider.h

  Copyright (C) Heikki Junnila

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

#ifndef PLAYBACKSLIDER_H
#define PLAYBACKSLIDER_H

#include <QWidget>

class ClickAndGoSlider;
class QToolButton;
class QLineEdit;
class QLabel;

/** @addtogroup ui UI
 * @{
 */

class PlaybackSlider : public QWidget
{
    Q_OBJECT

public:
    PlaybackSlider(QWidget* parent);
    ~PlaybackSlider();

    void setValue(uchar value);
    uchar value() const;

    void setLabel(const QString& text);
    QString label() const;

    void setSelected(bool sel);

signals:
    void selected();
    void flashing(bool on);
    void valueChanged(uchar value);
    void started();
    void stopped();

private slots:
    void slotSliderChanged(int value);
    void slotFlashPressed();
    void slotFlashReleased();

private:
    QToolButton* m_select;
    QLabel* m_value;
    ClickAndGoSlider* m_slider;
    QLabel* m_label;
    QToolButton* m_flash;
    int m_previousValue;
};

/** @} */

#endif
