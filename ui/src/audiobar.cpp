/*
  Q Light Controller Plus
  audiobar.cpp

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

#include <QtXml>

#include "audiobar.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcspeeddial.h"
#include "vccuelist.h"
#include "virtualconsole.h"

AudioBar::AudioBar(int t, uchar v)
{
    m_type = t;
    m_value = v;
    m_tapped = false;
    m_dmxChannels.clear();
    m_absDmxChannels.clear();
    m_function = NULL;
    m_widget = NULL;
    m_widgetID = VCWidget::invalidId();
    m_minThreshold = 51; // 20%
    m_maxThreshold = 204; // 80%
    m_divisor = 1;
    m_skippedBeats = 0;
}

AudioBar *AudioBar::createCopy()
{
    AudioBar *copy = new AudioBar();
    copy->m_type = m_type;
    copy->m_value = m_value;
    copy->m_name = m_name;
    copy->m_tapped = m_tapped;
    copy->m_dmxChannels = m_dmxChannels;
    copy->m_absDmxChannels = m_absDmxChannels;
    copy->m_function = m_function;
    copy->m_widget = m_widget;
    copy->m_minThreshold = m_minThreshold;
    copy->m_maxThreshold = m_maxThreshold;
    copy->m_divisor = m_divisor;
    copy->m_skippedBeats = m_skippedBeats;

    return copy;
}

void AudioBar::setName(QString nme)
{
    m_name = nme;
}

void AudioBar::setType(int type)
{
    m_type = type;
    if (m_type == None)
    {
        m_value = 0;
        m_tapped = false;
        m_dmxChannels.clear();
        m_absDmxChannels.clear();
        m_function = NULL;
        m_widget = NULL;
        m_widgetID = VCWidget::invalidId();
        m_minThreshold = 51; // 20%
        m_maxThreshold = 204; // 80%
        m_divisor = 1;
        m_skippedBeats = 0;
    }
}

void AudioBar::setMinThreshold(uchar value)
{
    m_minThreshold = value;
}

void AudioBar::setMaxThreshold(uchar value)
{
    m_maxThreshold = value;
}

void AudioBar::setDivisor(int value)
{
    m_divisor = value;
    if (m_skippedBeats >= m_divisor)
        m_skippedBeats = 0;
}

void AudioBar::attachDmxChannels(Doc *doc, QList<SceneValue> list)
{
    m_dmxChannels.clear();
    m_dmxChannels = list;
    m_absDmxChannels.clear();
    foreach(SceneValue scv, m_dmxChannels)
    {
        Fixture *fx = doc->fixture(scv.fxi);
        if (fx != NULL)
        {
            quint32 absAddr = fx->universeAddress() + scv.channel;
            m_absDmxChannels.append(absAddr);
        }
    }
}

void AudioBar::attachFunction(Function *func)
{
    if (func != NULL)
    {
        qDebug() << Q_FUNC_INFO << "Attaching function:" << func->name();
        m_function = func;
    }
}

void AudioBar::attachWidget(quint32 wID)
{
    if (wID == VCWidget::invalidId())
        return;

    qDebug() << Q_FUNC_INFO << "Attaching widget with ID" << wID;
    m_widgetID = wID;
    m_widget = NULL;
    m_tapped = false;
}

VCWidget * AudioBar::widget()
{
    if (m_widget == NULL)
        m_widget = VirtualConsole::instance()->widget(m_widgetID);

    return m_widget;
}

void AudioBar::checkFunctionThresholds(Doc *doc)
{
    if (m_function == NULL)
        return;
    if (m_value >= m_maxThreshold && m_function->isRunning() == false)
        m_function->start(doc->masterTimer());
    else if (m_value < m_minThreshold && m_function->isRunning() == true)
        m_function->stop();
}

void AudioBar::checkWidgetFunctionality()
{
    if (m_widgetID == VCWidget::invalidId())
        return;

    if (widget() == NULL) // fills m_widget if needed
        return;

    if (m_widget->type() == VCWidget::ButtonWidget)
    {
        VCButton *btn = (VCButton *)m_widget;
        if (m_value >= m_maxThreshold && btn->isOn() == false)
            btn->setOn(true);
        else if (m_value < m_minThreshold && btn->isOn() == true)
            btn->setOn(false);
    }
    else if (m_widget->type() == VCWidget::SliderWidget)
    {
        VCSlider *slider = (VCSlider *)m_widget;
        slider->setSliderValue(m_value);
    }
    else if (m_widget->type() == VCWidget::SpeedDialWidget)
    {
        VCSpeedDial *speedDial = (VCSpeedDial *)m_widget;
        if (m_value >= m_maxThreshold && !m_tapped)
        {
            if (m_skippedBeats == 0)
               speedDial->tap();
            
            m_tapped = true;
            m_skippedBeats = (m_skippedBeats + 1) % m_divisor;
        }
        else if (m_value < m_minThreshold)
        {
            m_tapped = false;
        }
    }
    else if (m_widget->type() == VCWidget::CueListWidget)
    {
        VCCueList *cueList = (VCCueList *)m_widget;
        if (m_value >= m_maxThreshold && !m_tapped)
        {
            if (m_skippedBeats == 0)
                cueList->slotNextCue();

            m_tapped = true;
            m_skippedBeats = (m_skippedBeats + 1) % m_divisor;
        }
        else if (m_value < m_minThreshold)
            m_tapped = false;
    }
}

void AudioBar::debugInfo()
{
    qDebug() << "[AudioBar] " << m_name;
    qDebug() << "   type:" << m_type << ", value:" << m_value;

}

bool AudioBar::loadXML(const QDomElement &root, Doc *doc)
{
    if (root.hasAttribute(KXMLQLCAudioBarName))
        m_name = root.attribute(KXMLQLCAudioBarName);

    if (root.hasAttribute(KXMLQLCAudioBarType))
    {
        m_type = root.attribute(KXMLQLCAudioBarType).toInt();
        m_minThreshold = root.attribute(KXMLQLCAudioBarMinThreshold).toInt();
        m_maxThreshold = root.attribute(KXMLQLCAudioBarMaxThreshold).toInt();
        m_divisor = root.attribute(KXMLQLCAudioBarDivisor).toInt();

        if (m_type == AudioBar::DMXBar)
        {
            QDomNode node = root.firstChild();
            if (node.isNull() == false)
            {
                QDomElement tag = node.toElement();
                if (tag.tagName() == KXMLQLCAudioBarDMXChannels)
                {
                    QString dmxValues = tag.text();
                    if (dmxValues.isEmpty() == false)
                    {
                        QList<SceneValue> channels;
                        QStringList varray = dmxValues.split(",");
                        for (int i = 0; i < varray.count(); i+=2)
                        {
                            channels.append(SceneValue(QString(varray.at(i)).toUInt(),
                                                         QString(varray.at(i + 1)).toUInt(), 0));
                        }
                        attachDmxChannels(doc, channels);
                    }
                }
            }
        }
    }
    return true;
}

bool AudioBar::saveXML(QDomDocument *doc, QDomElement *atf_root, QString tagName, int index)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(atf_root != NULL);

    qDebug() << Q_FUNC_INFO;

    QDomElement ab_tag = doc->createElement(tagName);
    ab_tag.setAttribute(KXMLQLCAudioBarName, m_name);
    ab_tag.setAttribute(KXMLQLCAudioBarType, m_type);
    ab_tag.setAttribute(KXMLQLCAudioBarMinThreshold, m_minThreshold);
    ab_tag.setAttribute(KXMLQLCAudioBarMaxThreshold, m_maxThreshold);
    ab_tag.setAttribute(KXMLQLCAudioBarDivisor, m_divisor);
    ab_tag.setAttribute(KXMLQLCAudioBarIndex, index);
    if (m_type == AudioBar::DMXBar && m_dmxChannels.count() > 0)
    {
        QDomElement dmx_tag = doc->createElement(KXMLQLCAudioBarDMXChannels);
        QString chans;
        foreach (SceneValue scv, m_dmxChannels)
        {
            if (chans.isEmpty() == false)
                chans.append(",");
            chans.append(QString("%1,%2").arg(scv.fxi).arg(scv.channel));
        }
        if (chans.isEmpty() == false)
        {
            QDomText text = doc->createTextNode(chans);
            dmx_tag.appendChild(text);
        }

        ab_tag.appendChild(dmx_tag);
    }
    else if (m_type == AudioBar::FunctionBar && m_function != NULL)
    {
        ab_tag.setAttribute(KXMLQLCAudioBarFunction, m_function->id());
    }
    else if (m_type == AudioBar::VCWidgetBar && m_widget != NULL)
    {
        ab_tag.setAttribute(KXMLQLCAudioBarWidget, m_widget->id());
    }
    atf_root->appendChild(ab_tag);

    return true;
}
