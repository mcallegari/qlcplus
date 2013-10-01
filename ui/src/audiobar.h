/*
  Q Light Controller Plus
  audiobar.h

  Copyright (c) Massimo Callegari

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

#ifndef AUDIOBAR_H
#define AUDIOBAR_H

#include "scenevalue.h"
#include "vcwidget.h"
#include "function.h"
#include "fixture.h"

#define KXMLQLCAudioBarIndex "Index"
#define KXMLQLCAudioBarName "Name"
#define KXMLQLCAudioBarType "Type"
#define KXMLQLCAudioBarDMXChannels "DMXChannels"
#define KXMLQLCAudioBarFunction "FunctionID"
#define KXMLQLCAudioBarWidget "WidgetID"

class QDomDocument;
class QDomElement;

class AudioBar
{
public:
    /** Normal constructor */
    AudioBar(int t = 0, uchar v = 0);

    /** Destructor */
    ~AudioBar() { }

    enum BarType
    {
        None = 0,
        DMXBar,
        FunctionBar,
        VCWidgetBar
    };

    AudioBar *createCopy();
    void setName(QString nme);
    void setMinThreshold(uchar value);
    void setMaxThreshold(uchar value);

    void attachDmxChannels(Doc *doc, QList<SceneValue>list);
    void attachFunction(Function *func);
    void attachWidget(VCWidget *widget);

    void checkFunctionThresholds(Doc *doc);
    void checkWidgetFunctionality();

    void debugInfo();

    /** Load properties and contents from an XML tree */
    bool loadXML(const QDomElement& root);

    /** Save properties and contents to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* atf_root, QString tagName, int index);

public:
    QString m_name;
    int m_type;
    uchar m_value;

    /** List of individual DMX channels when m_type == DMXBar */
    QList<SceneValue> m_dmxChannels;
    /** List of absolute DMX channel addresses when m_type == DMXBar.
      * This is precalculated to speed up writeDMX */
    QList<int> m_absDmxChannels;
    /** Reference to an attached Function when m_type == FunctionBar */
    Function *m_function;
    /** Reference to an attached VCWidget when m_type == VCWidgetBar */
    VCWidget *m_widget;

    uchar m_minThreshold, m_maxThreshold;
};

#endif // AUDIOBAR_H
