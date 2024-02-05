/*
  Q Light Controller Plus
  vcpage.h

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

#ifndef VCPAGE_H
#define VCPAGE_H

#include "vcframe.h"

class PreviewContext;

class VCPage : public VCFrame
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCPage(QQuickView *view = nullptr, Doc* doc = nullptr, VirtualConsole *vc = nullptr, int pageIndex = 0, QObject *parent = nullptr);
    ~VCPage();

    /** Return the Preview Context associated to this VC page */
    PreviewContext *previewContext() const;

    /** Get/Set the widgets scale factor currently applied to this VC page */
    qreal pageScale() const;
    void setPageScale(qreal factor);

private:
    /** Reference to a PreviewContext, registered to the Context Manager */
    PreviewContext *m_pageContext;

    qreal m_pageScale;

    /*********************************************************************
     * External input
     *********************************************************************/
public:
    /** Map a single input source for a specific VC widget. */
    void mapInputSource(QSharedPointer<QLCInputSource> source, VCWidget *widget, bool checkChildren = false);

    /** Unmap a single input source for a specific VC widget. */
    void unMapInputSource(quint32 id, quint32 universe, quint32 channel, VCWidget *widget, bool checkChildren = false);

    /** Map all the children widgets input sources into $m_inputSourcesMap.
     *  This method is called only by VirtualConsole postLoad event */
    void mapChildrenInputSources();

    /** Reset the input source map */
    void resetInputSourcesMap();

    /** Method invoked by the Virtual Console when an input signal is received.
     *  This is in charge of delivering the event to the children widgets expecting it. */
    void inputValueChanged(quint32 universe, quint32 channel, uchar value);

    /** Map a single key sequence for a specific VC widget. */
    void mapKeySequence(QKeySequence sequence, quint32 id, VCWidget *widget, bool checkChildren = false);

    /** Unmap a single key sequence for a specific VC widget. */
    void unMapKeySequence(QKeySequence sequence, quint32 id, VCWidget *widget, bool checkChildren = false);

    /** Update the key sequences map for a matching $sequence and $widget with the specified $id */
    void updateKeySequenceIDInMap(QKeySequence sequence, quint32 id, VCWidget *widget, bool checkChildren = false);

    /** Rebuild the entire key sequence map for all the child widgets
     *  of thiss page. This is called on project XML loading */
    void buildKeySequenceMap();

    /** Method invoked by the Virtual Console when an key press/release signal is received.
     *  This is in charge of delivering the event to the children widgets expecting it. */
    void handleKeyEvent(QKeyEvent *e, bool pressed);

private:
    /** This variable represents the map of all the external controllers
     *  input sources for every child of this VC Page.
     *  It works as a lookup table for signals coming from the InputOutputMap class.
     *
     *  The hash key is the source universe shifted 16bits left, masked with the
     *  source channel without the widget page information. Example:
     *      (source->universe() << 16) | (source->channel() & 0x0000FFFF);
     *
     *  The hash value is a pair of the actual input source and VC widget references
     */
    QMultiHash <quint32, QPair<QSharedPointer<QLCInputSource>, VCWidget *> > m_inputSourcesMap;

    /** This variable represents the map of all the key bindings for every
     *  child widget of this VC Page.
     *
     *  The hash key is a QKeySequence, which can be used by multiple widgets
     *  The hash value is a pair of the widget reference and control ID
     */
    QMultiHash <QKeySequence, QPair<quint32, VCWidget *> > m_keySequencesMap;
};


#endif
