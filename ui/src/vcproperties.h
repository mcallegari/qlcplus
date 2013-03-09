/*
  Q Light Controller
  vcproperties.h

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

#ifndef VCPROPERTIES_H
#define VCPROPERTIES_H

#include "universearray.h"

class VirtualConsole;
class QDomDocument;
class QDomElement;
class MasterTimer;
class OutputMap;
class InputMap;
class VCFrame;
class QWidget;
class Doc;

#define KXMLQLCVirtualConsole "VirtualConsole"

#define KXMLQLCVCProperties "Properties"
#define KXMLQLCVCPropertiesSize "Size"
#define KXMLQLCVCPropertiesSizeWidth "Width"
#define KXMLQLCVCPropertiesSizeHeight "Height"

#define KXMLQLCVCPropertiesKeyboard "Keyboard"
#define KXMLQLCVCPropertiesKeyboardTapModifier "TapModifier"

#define KXMLQLCVCPropertiesGrandMaster "GrandMaster"
#define KXMLQLCVCPropertiesGrandMasterVisible "Visible"
#define KXMLQLCVCPropertiesGrandMasterChannelMode "ChannelMode"
#define KXMLQLCVCPropertiesGrandMasterValueMode "ValueMode"
#define KXMLQLCVCPropertiesGrandMasterSliderMode "SliderMode"

#define KXMLQLCVCPropertiesInput "Input"
#define KXMLQLCVCPropertiesInputUniverse "Universe"
#define KXMLQLCVCPropertiesInputChannel "Channel"

/*****************************************************************************
 * Properties
 *****************************************************************************/

class VCProperties
{
public:
    VCProperties();
    VCProperties(const VCProperties& properties);
    ~VCProperties();

    /*********************************************************************
     * Size
     *********************************************************************/
public:
    /** Set Virtual Console bottom frame size */
    void setSize(const QSize& size);

    /** Get Virtual Console bottom frame size */
    QSize size() const;

private:
    QSize m_size;

    /*********************************************************************
     * Keyboard
     *********************************************************************/
public:
    /** Set the tap modifier key */
    void setTapModifier(Qt::KeyboardModifier mod);

    /** Get the tap modifier key */
    Qt::KeyboardModifier tapModifier() const;

private:
    Qt::KeyboardModifier m_tapModifier;

    /*************************************************************************
     * Grand Master
     *************************************************************************/
public:
    void setGrandMasterChannelMode(UniverseArray::GMChannelMode mode);
    UniverseArray::GMChannelMode grandMasterChannelMode() const;

    void setGrandMasterValueMode(UniverseArray::GMValueMode mode);
    UniverseArray::GMValueMode grandMasterValueMode() const;

    void setGrandMasterSliderMode(UniverseArray::GMSliderMode mode);
    UniverseArray::GMSliderMode grandMasterSlideMode() const;

    void setGrandMasterInputSource(quint32 universe, quint32 channel);
    quint32 grandMasterInputUniverse() const;
    quint32 grandMasterInputChannel() const;

private:
    UniverseArray::GMChannelMode m_gmChannelMode;
    UniverseArray::GMValueMode m_gmValueMode;
    UniverseArray::GMSliderMode m_gmSliderMode;
    quint32 m_gmInputUniverse;
    quint32 m_gmInputChannel;

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    /** Load VirtualConsole properties from the given XML tag */
    bool loadXML(const QDomElement& vc_root);

    /** Save VirtualConsole properties to the given XML document */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root) const;

private:
    /** Load the properties of a default slider */
    static bool loadXMLInput(const QDomElement& tag, quint32* universe, quint32* channel);
};

#endif
