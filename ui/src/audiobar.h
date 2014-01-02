/*
  Q Light Controller Plus
  audiobar.h

  Copyright (c) Massimo Callegari

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

#ifndef AUDIOBAR_H
#define AUDIOBAR_H

#include "scenevalue.h"
#include "vcwidget.h"
#include "function.h"
#include "fixture.h"

class QDomDocument;
class QDomElement;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCAudioBarIndex "Index"
#define KXMLQLCAudioBarName "Name"
#define KXMLQLCAudioBarType "Type"
#define KXMLQLCAudioBarDMXChannels "DMXChannels"
#define KXMLQLCAudioBarFunction "FunctionID"
#define KXMLQLCAudioBarWidget "WidgetID"
#define KXMLQLCAudioBarMinThreshold "MinThreshold"
#define KXMLQLCAudioBarMaxThreshold "MaxThreshold"
#define KXMLQLCAudioBarDivisor "Divisor"

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
    void setType(int type);
    void setMinThreshold(uchar value);
    void setMaxThreshold(uchar value);
    void setDivisor(int value);

    void attachDmxChannels(Doc *doc, QList<SceneValue>list);
    void attachFunction(Function *func);
    void attachWidget(quint32 wID);

    /** Get widget, sets m_widget to proper value if necessary */
    VCWidget * widget();

    void checkFunctionThresholds(Doc *doc);
    void checkWidgetFunctionality();

    void debugInfo();

    /** Load properties and contents from an XML tree */
    bool loadXML(const QDomElement& root, Doc *doc);

    /** Save properties and contents to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* atf_root, QString tagName, int index);

public:
    QString m_name;
    int m_type;
    uchar m_value;
    bool m_tapped;

    /** List of individual DMX channels when m_type == DMXBar */
    QList<SceneValue> m_dmxChannels;

    /** List of absolute DMX channel addresses when m_type == DMXBar.
      * This is precalculated to speed up writeDMX */
    QList<int> m_absDmxChannels;

    /** Reference to an attached Function when m_type == FunctionBar */
    Function *m_function;

    /** ID of the attchaed VCWidget when m_type == VCWidgetBar */
    quint32 m_widgetID;

    uchar m_minThreshold, m_maxThreshold;
    int m_divisor;

    int m_skippedBeats;

private:

    /** Reference to an attached VCWidget when m_type == VCWidgetBar */
    VCWidget *m_widget;

};

/** @} */

#endif // AUDIOBAR_H
