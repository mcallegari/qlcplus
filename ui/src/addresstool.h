/*
  Q Light Controller Plus
  addresstool.h

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

#ifndef ADDRESSTOOL_H
#define ADDRESSTOOL_H

#include <QDialog>
#include <QWidget>
#include <QMap>

namespace Ui {
class AddressTool;
}

/** @addtogroup ui UI
 * @{
 */

class DIPSwitchSlider : public QObject
{
    Q_OBJECT
public:
    DIPSwitchSlider(QObject *parent = 0);
    ~DIPSwitchSlider();

    void setPosition(QPoint pos, QSize size);
    void paint(QPainter *painter, bool value, bool vreverse);
    bool isClicked(QPoint click);

private:
    QPoint m_pos;
    QSize m_size;
};

class DIPSwitchWidget: public QWidget
{
    Q_OBJECT

public:
    DIPSwitchWidget(QWidget *parent = 0, int presetValue = 1);
    ~DIPSwitchWidget();

    void setColor(QColor col);

signals:
    void valueChanged(int value);

public slots:
    void slotReverseVertically(bool toggle);
    void slotReverseHorizontally(bool toggle);
    void slotSetValue(int value);

private:
    void updateSliders();

    qint16 m_value;
    QFont m_font;
    QColor m_backCol;
    bool m_verticalReverse;
    bool m_horizontalReverse;
    QMap<quint8, DIPSwitchSlider*> m_sliders;

protected:
    void paintEvent(QPaintEvent* e);
    void mousePressEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);
};

class AddressTool : public QDialog
{
    Q_OBJECT

public:
    explicit AddressTool(QWidget *parent = 0, int presetValue = 1);
    ~AddressTool();

    int getAddress();

private:
    Ui::AddressTool *ui;
    DIPSwitchWidget *m_dipSwitch;

protected slots:
    void slotChangeColor();

};

/** @} */

#endif // ADDRESSTOOL_H
