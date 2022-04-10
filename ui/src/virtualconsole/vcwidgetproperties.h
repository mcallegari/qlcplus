/*
  Q Light Controller
  vcwidgetproperties.h

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

#ifndef VCWIDGETPROPERTIES_H
#define VCWIDGETPROPERTIES_H

#include <Qt>

class QXmlStreamReader;
class QXmlStreamWriter;

class QWidget;

/** @addtogroup ui_vc_props
 * @{
 */

#define KXMLQLCWidgetProperties QString("WidgetProperties")
#define KXMLQLCWidgetX          QString("X")
#define KXMLQLCWidgetY          QString("Y")
#define KXMLQLCWidgetWidth      QString("Width")
#define KXMLQLCWidgetHeight     QString("Height")
#define KXMLQLCWidgetState      QString("State")
#define KXMLQLCWidgetVisible    QString("Visible")

/** Simple class to store a widget's visibility, state and dimensions */
class VCWidgetProperties
{
public:
    VCWidgetProperties();
    VCWidgetProperties(const VCWidgetProperties& properties);
    virtual ~VCWidgetProperties();

    /************************************************************************
     * Properties
     ************************************************************************/
public:
    QFlags <Qt::WindowState> state() const;
    bool visible() const;
    int x() const;
    int y() const;
    int width() const;
    int height() const;

    /** Get properties from the given widget and store them in memory. */
    virtual void store(QWidget* widget);

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** Load properties from an XML tag */
    virtual bool loadXML(QXmlStreamReader &root);

    /** Save stored properties to the given XML document */
    virtual bool saveXML(QXmlStreamWriter *doc);

protected:
    QFlags <Qt::WindowState> m_state;
    bool m_visible;
    int m_x;
    int m_y;
    int m_width;
    int m_height;
};

/** @} */

#endif
