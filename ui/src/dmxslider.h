/*
  Q Light Controller
  dmxslider.h

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

#ifndef DMXSLIDER_H
#define DMXSLIDER_H

#include <QWidget>

class QLineEdit;
class QSlider;
class QLabel;
class QSize;

class DMXSlider : public QWidget
{
    Q_OBJECT

public:
    DMXSlider(QWidget* parent);
    ~DMXSlider();

    void setValue(uchar value);
    uchar value() const;

    void setLabel(const QString& text);
    QString label() const;

    void setVerticalLabel(const QString& text);
    QString verticalLabel() const;

signals:
    void valueChanged(uchar value);

private slots:
    void slotValueEdited(const QString& text);
    void slotSliderChanged(int value);

protected:
    void paintEvent(QPaintEvent* e);

private:
    QLineEdit* m_edit;
    QSlider* m_slider;
    QLabel* m_label;
    QString m_verticalText;
};

#endif
