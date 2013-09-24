/*
  Q Light Controller Plus
  audiobar.cpp

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

#include <QtXml>

#include "audiobar.h"
#include "vcbutton.h"
#include "vcslider.h"

AudioBar::AudioBar(int t, uchar v)
{
    m_type = t;
    m_value = v;
    m_dmxChannels.clear();
    m_absDmxChannels.clear();
    m_function = NULL;
    m_widget = NULL;
    m_minThreshold = 51; // 20%
    m_maxThreshold = 204; // 80%
}

AudioBar *AudioBar::createCopy()
{
    AudioBar *copy = new AudioBar();
    copy->m_type = m_type;
    copy->m_value = m_value;
    copy->m_name = m_name;
    copy->m_dmxChannels = m_dmxChannels;
    copy->m_absDmxChannels = m_absDmxChannels;
    copy->m_function = m_function;
    copy->m_widget = m_widget;
    copy->m_minThreshold = m_minThreshold;
    copy->m_maxThreshold = m_maxThreshold;

    return copy;
}

void AudioBar::setName(QString nme)
{
    m_name = nme;
}

void AudioBar::setMinThreshold(uchar value)
{
    m_minThreshold = value;
}

void AudioBar::setMaxThreshold(uchar value)
{
    m_maxThreshold = value;
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

void AudioBar::attachWidget(VCWidget *widget)
{
    if (widget != NULL)
    {
        qDebug() << Q_FUNC_INFO << "Attaching widget:" << widget->caption();
        m_widget = widget;
    }
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
    if (m_widget == NULL)
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
}

void AudioBar::debugInfo()
{
    qDebug() << "[AudioBar] " << m_name;
    qDebug() << "   type:" << m_type << ", value:" << m_value;

}

bool AudioBar::loadXML(const QDomElement &root)
{
    if (root.hasAttribute(KXMLQLCAudioBarName))
        m_name = root.attribute(KXMLQLCAudioBarName);

    if (root.hasAttribute(KXMLQLCAudioBarType))
    {
        m_type = root.attribute(KXMLQLCAudioBarType).toInt();

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
                        m_dmxChannels.clear();
                        QStringList varray = dmxValues.split(",");
                        for (int i = 0; i < varray.count(); i+=2)
                        {
                            m_dmxChannels.append(SceneValue(QString(varray.at(i)).toUInt(),
                                                         QString(varray.at(i + 1)).toUInt(), 0));
                        }
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
