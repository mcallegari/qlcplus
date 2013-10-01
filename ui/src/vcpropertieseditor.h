/*
  Q Light Controller
  vcpropertieseditor.h

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

#ifndef VCPROPERTIESEDITOR_H
#define VCPROPERTIESEDITOR_H

#include <QDialog>

#include "ui_vcproperties.h"
#include "vcwidgetproperties.h"
#include "universearray.h"
#include "vcproperties.h"

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

class VirtualConsole;
class QDomDocument;
class QDomElement;
class InputMap;
class VCFrame;

class VCPropertiesEditor : public QDialog, public Ui_VCPropertiesEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(VCPropertiesEditor)

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    VCPropertiesEditor(QWidget* parent, const VCProperties& properties,
                       InputMap* inputMap);
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

private:
    VCProperties m_properties;
    InputMap* m_inputMap;

    /*************************************************************************
     * Layout page
     *************************************************************************/
private:
    void fillTapModifierCombo();

private slots:
    void slotSizeXChanged(int value);
    void slotSizeYChanged(int value);
    void slotTapModifierActivated(int index);

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

    /*************************************************************************
     * Input Source helper
     *************************************************************************/
private:
    bool inputSourceNames(quint32 universe, quint32 channel,
                          QString& uniName, QString& chName) const;
};

#endif
