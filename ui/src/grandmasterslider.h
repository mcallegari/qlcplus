/*
  Q Light Controller
  grandmasterslider.h

  Copyright (c) Heikki Junnila

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

#ifndef GRANDMASTERSLIDER_H
#define GRANDMASTERSLIDER_H

#include <QFrame>

#include "grandmaster.h"

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

    bool invertedAppearance() const;
    void setInvertedAppearance(bool invert);

private:
    void updateTooltip();
    void updateDisplayValue();

protected slots:
    void slotValueChanged(int value);
    void slotGrandMasterValueChanged(uchar value);
    void slotGrandMasterValueModeChanged(GrandMaster::GMValueMode mode);

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
