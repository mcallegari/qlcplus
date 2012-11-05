/*
  Q Light Controller
  playbackslider.h

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

#ifndef PLAYBACKSLIDER_H
#define PLAYBACKSLIDER_H

#include <QWidget>

class QToolButton;
class QLineEdit;
class QSlider;
class QLabel;

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
    QSlider* m_slider;
    QLabel* m_label;
    QToolButton* m_flash;
    int m_previousValue;
};

#endif
