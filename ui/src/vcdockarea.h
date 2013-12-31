/*
  Q Light Controller
  vcdockarea.h

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

#ifndef VCDOCKAREA_H
#define VCDOCKAREA_H

#include "grandmaster.h"

#include <qframe.h>

class GrandMasterSlider;
class InputOutputMap;
class QShowEvent;
class QHideEvent;

class VCDockArea : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(VCDockArea)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCDockArea(QWidget* parent, InputOutputMap* ioMap);
    ~VCDockArea();

    void setGrandMasterInvertedAppearance(GrandMaster::GMSliderMode mode);

signals:
    void visibilityChanged(bool isVisible);

private:
    GrandMasterSlider* m_gm;

    /*********************************************************************
     * Event Handlers & Signals
     *********************************************************************/
protected:
    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent* event);
};

#endif
