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

namespace Ui {
class AddressTool;
}

/** @addtogroup ui UI
 * @{
 */

class DIPSwitchWidget: public QWidget
{
    Q_OBJECT

public:
    DIPSwitchWidget(QWidget *parent = 0);
    ~DIPSwitchWidget();

    void setColor(QColor col);

public slots:
    void slotReverseVertically(bool toggle);
    void slotReverseHorizontally(bool toggle);
    void slotSetValue(int value);

private:
    int m_value;
    QFont m_font;
    QColor m_backCol;
    bool m_verticalReverse;
    bool m_horizontalReverse;

protected:
    void paintEvent(QPaintEvent* e);
};

class AddressTool : public QDialog
{
    Q_OBJECT
    
public:
    explicit AddressTool(QWidget *parent = 0);
    ~AddressTool();
    
private:
    Ui::AddressTool *ui;
    DIPSwitchWidget *m_dipSwitch;

protected slots:
    void slotChangeColor();

};

/** @} */

#endif // ADDRESSTOOL_H
