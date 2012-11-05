/*
  Q Light Controller
  grandmasterslider.h

  Copyright (c) Heikki Junnila

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

#ifndef GRANDMASTERSLIDER_H
#define GRANDMASTERSLIDER_H

#include <QFrame>

#include "../../engine/src/universearray.h"

class OutputMap;
class InputMap;
class QSlider;
class QLabel;

class GrandMasterSlider : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(GrandMasterSlider)

public:
    GrandMasterSlider(QWidget* parent, OutputMap* outputMap, InputMap* inputMap);
    virtual ~GrandMasterSlider();

private:
    void updateTooltip();
    void updateDisplayValue();

protected slots:
    void slotValueChanged(int value);
    void slotGrandMasterValueChanged(uchar value);
    void slotGrandMasterValueModeChanged(UniverseArray::GMValueMode mode);

protected:
    QLabel* m_valueLabel;
    QSlider* m_slider;
    QLabel* m_nameLabel;
    OutputMap* m_outputMap;
    InputMap* m_inputMap;

    /*************************************************************************
     * External input
     *************************************************************************/
protected slots:
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);
};

#endif
