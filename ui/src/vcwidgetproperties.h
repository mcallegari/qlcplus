/*
  Q Light Controller
  vcwidgetproperties.h

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

#ifndef VCWIDGETPROPERTIES_H
#define VCWIDGETPROPERTIES_H

#include <QtXml>

class QWidget;

#define KXMLQLCWidgetProperties "WidgetProperties"
#define KXMLQLCWidgetX "X"
#define KXMLQLCWidgetY "Y"
#define KXMLQLCWidgetWidth "Width"
#define KXMLQLCWidgetHeight "Height"
#define KXMLQLCWidgetState "State"
#define KXMLQLCWidgetVisible "Visible"

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
    virtual bool loadXML(const QDomElement& root);

    /** Save stored properties to the given XML document */
    virtual bool saveXML(QDomDocument* doc, QDomElement* root);

protected:
    QFlags <Qt::WindowState> m_state;
    bool m_visible;
    int m_x;
    int m_y;
    int m_width;
    int m_height;
};

#endif
