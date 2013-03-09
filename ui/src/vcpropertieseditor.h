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
