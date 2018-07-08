/*
  Q Light Controller
  vcpropertieseditor.h

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

#ifndef VCPROPERTIESEDITOR_H
#define VCPROPERTIESEDITOR_H

#include <QDialog>

#include "ui_vcproperties.h"
#include "vcwidgetproperties.h"
#include "vcproperties.h"
#include "universe.h"

class VirtualConsole;
class InputOutputMap;
class VCFrame;

/** @addtogroup ui_vc_props
 * @{
 */

#define SETTINGS_BUTTON_SIZE        "virtualconsole/buttonsize"
#define SETTINGS_BUTTON_STATUSLED   "virtualconsole/buttonstatusled"
#define SETTINGS_SLIDER_SIZE        "virtualconsole/slidersize"
#define SETTINGS_SPEEDDIAL_SIZE     "virtualconsole/speeddialsize"
#define SETTINGS_SPEEDDIAL_VALUE    "virtualconsole/speeddialvalue"
#define SETTINGS_XYPAD_SIZE         "virtualconsole/xypadsize"
#define SETTINGS_CUELIST_SIZE       "virtualconsole/cuelistsize"
#define SETTINGS_FRAME_SIZE         "virtualconsole/framesize"
#define SETTINGS_SOLOFRAME_SIZE     "virtualconsole/soloframesize"
#define SETTINGS_AUDIOTRIGGERS_SIZE "virtualconsole/audiotriggerssize"
#define SETTINGS_RGBMATRIX_SIZE     "virtualconsole/rgbmatrixsize"

class VCPropertiesEditor : public QDialog, public Ui_VCPropertiesEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(VCPropertiesEditor)

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    VCPropertiesEditor(QWidget* parent, const VCProperties& properties,
                       InputOutputMap* ioMap);
    ~VCPropertiesEditor();

    VCProperties properties() const;

    QSize buttonSize();
    bool buttonStatusLED();
    QSize sliderSize();
    QSize speedDialSize();
    uint speedDialValue();
    QSize xypadSize();
    QSize cuelistSize();
    QSize frameSize();
    QSize soloFrameSize();
    QSize audioTriggersSize();
    QSize rgbMatrixSize();

private:
    VCProperties m_properties;
    InputOutputMap* m_ioMap;

    /*************************************************************************
     * Layout page
     *************************************************************************/
private slots:
    void slotSizeXChanged(int value);
    void slotSizeYChanged(int value);

    /*************************************************************************
     * Widgets page
     *************************************************************************/
protected slots:
    void slotSpeedDialConfirmed();

    /*************************************************************************
     * Grand Master page
     *************************************************************************/
private slots:
    void slotGrandMasterIntensityToggled(bool checked);
    void slotGrandMasterReduceToggled(bool checked);
    void slotGrandMasterSliderNormalToggled(bool checked);
    void slotAutoDetectGrandMasterInputToggled(bool checked);
    void slotGrandMasterInputValueChanged(quint32 universe, quint32 channel);
    void slotChooseGrandMasterInputClicked();

private:
    void updateGrandMasterInputSource();
};

/** @} */

#endif
