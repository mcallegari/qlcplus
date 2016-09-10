/*
  Q Light Controller Plus
  vcpage.cpp

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

#include "vcpage.h"

VCPage::VCPage(Doc *doc, VirtualConsole *vc, QObject *parent)
    : VCFrame(doc, vc, parent)
{

}

VCPage::~VCPage()
{

}

void VCPage::mapInputSource(QSharedPointer<QLCInputSource> source, VCWidget *widget)
{
    if (source->isValid() == false || widget == NULL)
        return;

    qDebug() << "Mapping input source. Universe:" << source->universe() << ", channel:" << source->channel() << ", widget:" << widget->id();

    /** Note that the channel is mapped without the page information.
     *  This is because input signals come without it, so the page match
     *  has to be performed later */
    quint32 key = (source->universe() << 16) | (source->channel() & 0x0000FFFF);
    QPair <QSharedPointer<QLCInputSource>, VCWidget *> refs;
    refs.first = source;
    refs.second = widget;

    m_inputSourcesMap.insert(key, refs);
}

void VCPage::mapChildrenInputSources()
{
    /** Scan all the children widgets and map the detected input sources */
    foreach(VCWidget *widget, children(true))
    {
        foreach (QSharedPointer<QLCInputSource> source, widget->inputSources())
            mapInputSource(source, widget);
    }
}

void VCPage::inputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    quint32 key = (universe << 16) | channel;

    /** Here is where the magic happens.
     *  For each input source that matches the given universe/channel,
     *  check also if the page matches and finally inform the VC widget
     *  about the event, including the source ID
     */
    for(QPair<QSharedPointer<QLCInputSource>, VCWidget *> match : m_inputSourcesMap.values(key)) // C++11...yay !
    {
        if (match.second->isDisabled() == false && match.first->page() == match.second->page())
            match.second->slotInputValueChanged(match.first->id(), value);
    }
}
