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

#include "previewcontext.h"

VCPage::VCPage(QQuickView *view, Doc *doc, VirtualConsole *vc, int pageIndex, QObject *parent)
    : VCFrame(doc, vc, parent)
    , m_PIN(0)
    , m_validatedPIN(false)
{
    setAllowResize(false);
    setShowHeader(false);
    setGeometry(QRect(0, 0, 1920, 1080));
    setFont(QFont("Roboto Condensed", 16));
    setCaption(tr("Page %1").arg(pageIndex + 1));

    m_pageContext = new PreviewContext(view, doc, QString("PAGE-%1").arg(pageIndex));
    m_pageContext->setContextResource("qrc:/VCPageArea.qml");
    m_pageContext->setContextTitle(tr("Virtual Console Page %1").arg(pageIndex));
    m_pageContext->setContextPage(pageIndex);
}

VCPage::~VCPage()
{
    m_pageContext->deleteLater();
}

PreviewContext *VCPage::previewContext() const
{
    return m_pageContext;
}

/*********************************************************************
 * PIN
 *********************************************************************/

int VCPage::PIN() const
{
    return m_PIN;
}

void VCPage::setPIN(int newPIN)
{
    if (newPIN == m_PIN)
        return;

    m_PIN = newPIN;
    setDocModified();
    emit PINChanged(newPIN);
}

void VCPage::validatePIN()
{
    m_validatedPIN = true;
}

bool VCPage::requirePIN() const
{
    if (m_PIN == 0 || m_validatedPIN == true)
        return false;

    return true;
}

/*********************************************************************
 * External input
 *********************************************************************/

void VCPage::mapInputSource(QSharedPointer<QLCInputSource> source, VCWidget *widget, bool checkChildren)
{
    if (source->isValid() == false || widget == NULL)
        return;

    /** Check if the widget belongs to this page */
    if (checkChildren && children(true).contains(widget) == false)
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

void VCPage::unMapInputSource(quint32 id, quint32 universe, quint32 channel,
                              VCWidget *widget, bool checkChildren)
{
    if (widget == NULL)
        return;

    /** Check if the widget belongs to this page */
    if (checkChildren && children(true).contains(widget) == false)
        return;

    quint32 key = (universe << 16) | (channel & 0x0000FFFF);
    ushort page = channel >> 16;

    //qDebug() << "Multihash keys before deletion:" << m_inputSourcesMap.count(key);

    for(QPair<QSharedPointer<QLCInputSource>, VCWidget *> match : m_inputSourcesMap.values(key)) // C++11
    {
        if (match.first->id() == id && match.first->page() == page)
        {
            m_inputSourcesMap.remove(key, match);

            //qDebug() << "Multihash keys after deletion:" << m_inputSourcesMap.count(key);
            return;
        }
    }
}

void VCPage::mapChildrenInputSources()
{
    /** Scan all the children widgets and map the detected input sources */
    for (VCWidget *widget : children(true)) // C++11
    {
        for (QSharedPointer<QLCInputSource> source : widget->inputSources()) // C++11
            mapInputSource(source, widget);
    }
}

void VCPage::resetInputSourcesMap()
{
    m_inputSourcesMap.clear();
    m_keySequencesMap.clear();
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
        if (match.second->isDisabled() == false &&
            match.second->isEditing() == false &&
            match.first->page() == match.second->page())
        {
            match.second->slotInputValueChanged(match.first->id(), value);
        }
    }
}

void VCPage::mapKeySequence(QKeySequence sequence, quint32 id, VCWidget *widget, bool checkChildren)
{
    if (sequence.isEmpty() || widget == NULL)
        return;

    /** Check if the widget belongs to this page */
    if (checkChildren && children(true).contains(widget) == false)
        return;

    QPair <quint32, VCWidget *> refs;
    refs.first = id;
    refs.second = widget;

    m_keySequencesMap.insert(sequence, refs);
}

void VCPage::unMapKeySequence(QKeySequence sequence, quint32 id, VCWidget *widget, bool checkChildren)
{
    if (sequence.isEmpty() || widget == NULL)
        return;

    /** Check if the widget belongs to this page */
    if (checkChildren && children(true).contains(widget) == false)
        return;

    for(QPair<quint32, VCWidget *> match : m_keySequencesMap.values(sequence)) // C++11
    {
        if (match.first == id && match.second == widget)
        {
            m_keySequencesMap.remove(sequence, match);

            //qDebug() << "Multihash keys after deletion:" << m_keySequencesMap.count(key);
            return;
        }
    }
}

void VCPage::updateKeySequenceIDInMap(QKeySequence sequence, quint32 id, VCWidget *widget, bool checkChildren)
{
    if (sequence.isEmpty() || widget == NULL)
        return;

    /** Check if the widget belongs to this page */
    if (checkChildren && children(true).contains(widget) == false)
        return;

    quint32 oldId = UINT_MAX;

    /** Perform a lookup of the existing map to find the old control ID */
    for(QPair<quint32, VCWidget *> match : m_keySequencesMap.values(sequence)) // C++11
    {
        if (match.second == widget)
        {
            oldId = match.first;
            break;
        }
    }

    if (oldId == UINT_MAX)
    {
        qDebug() << "No match found for sequence" << sequence.toString() << "and widget" << widget->caption();
        return;
    }

    /** Now unmap and map the sequence again */
    unMapKeySequence(sequence, oldId, widget);
    mapKeySequence(sequence, id, widget);
}

void VCPage::handleKeyEvent(QKeyEvent *e, bool pressed)
{
    QKeySequence seq(e->key() | e->modifiers());

    for(QPair<quint32, VCWidget *> match : m_keySequencesMap.values(seq)) // C++11...yay !
    {
        if (match.second->isDisabled() == false &&
            match.second->isEditing() == false)
        {
            match.second->slotInputValueChanged(match.first, pressed ? 255 : 0);
        }
    }
}
