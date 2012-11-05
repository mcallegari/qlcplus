/*
  Q Light Controller
  dmxslider.h

  Copyright (C) Heikki Junnila

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
