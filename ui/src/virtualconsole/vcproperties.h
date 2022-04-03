/*
  Q Light Controller Plus
  vcproperties.h

  Copyright (c) Heikki Junnila
                Massimo Callegari

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

#ifndef VCPROPERTIES_H
#define VCPROPERTIES_H

#include "grandmaster.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class VirtualConsole;
class MasterTimer;
class VCFrame;
class QWidget;
class Doc;

/** @addtogroup ui_vc_props
 * @{
 */

#define KXMLQLCVirtualConsole           QString("VirtualConsole")

#define KXMLQLCVCProperties             QString("Properties")
#define KXMLQLCVCPropertiesSize         QString("Size")
#define KXMLQLCVCPropertiesSizeWidth    QString("Width")
#define KXMLQLCVCPropertiesSizeHeight   QString("Height")

#define KXMLQLCVCPropertiesGrandMaster              QString("GrandMaster")
#define KXMLQLCVCPropertiesGrandMasterVisible       QString("Visible")
#define KXMLQLCVCPropertiesGrandMasterChannelMode   QString("ChannelMode")
#define KXMLQLCVCPropertiesGrandMasterValueMode     QString("ValueMode")
#define KXMLQLCVCPropertiesGrandMasterSliderMode    QString("SliderMode")

#define KXMLQLCVCPropertiesInput         QString("Input")
#define KXMLQLCVCPropertiesInputUniverse QString("Universe")
#define KXMLQLCVCPropertiesInputChannel  QString("Channel")

/*****************************************************************************
 * Properties
 *****************************************************************************/

class VCProperties
{
public:
    VCProperties();
    VCProperties(const VCProperties& properties);
    ~VCProperties();

    VCProperties& operator=(const VCProperties& props);

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

    /*************************************************************************
     * Grand Master
     *************************************************************************/
public:
    void setGrandMasterChannelMode(GrandMaster::ChannelMode mode);
    GrandMaster::ChannelMode grandMasterChannelMode() const;

    void setGrandMasterValueMode(GrandMaster::ValueMode mode);
    GrandMaster::ValueMode grandMasterValueMode() const;

    void setGrandMasterSliderMode(GrandMaster::SliderMode mode);
    GrandMaster::SliderMode grandMasterSlideMode() const;

    void setGrandMasterInputSource(quint32 universe, quint32 channel);
    quint32 grandMasterInputUniverse() const;
    quint32 grandMasterInputChannel() const;

private:
    GrandMaster::ChannelMode m_gmChannelMode;
    GrandMaster::ValueMode m_gmValueMode;
    GrandMaster::SliderMode m_gmSliderMode;
    quint32 m_gmInputUniverse;
    quint32 m_gmInputChannel;

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    /** Load VirtualConsole properties from the given XML tag */
    bool loadXML(QXmlStreamReader &vc_root);

    /** Save VirtualConsole properties to the given XML document */
    bool saveXML(QXmlStreamWriter *doc) const;

private:
    /** Load the properties of a default slider */
    static bool loadXMLInput(QXmlStreamReader &root, quint32* universe, quint32* channel);
};

/** @} */

#endif
