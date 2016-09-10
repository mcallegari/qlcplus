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

class VCPage : public VCFrame
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCPage(Doc* doc = NULL, VirtualConsole *vc = NULL, QObject *parent = 0);
    ~VCPage();

    /*********************************************************************
     * External input
     *********************************************************************/
public:
    /** Map a single input source for a specific VC widget. */
    void mapInputSource(QSharedPointer<QLCInputSource> source, VCWidget *widget);

    /** Map all the children widgets input sources into $m_inputSourcesMap.
     *  This method is called only by VirtualConsole postLoad event */
    void mapChildrenInputSources();

    /** Method invoked by the Virtual Console when an input signal is received.
     *  This is in charge of delivering the event to the children widgets
     *  expecting it.
     */
    void inputValueChanged(quint32 universe, quint32 channel, uchar value);

private:
    /** This variable represents the map of all the input sources for every child
     *  of this VC Page.
     *  It works as a lookup table for signals coming from the InputOutputMap class.
     */
    QMultiHash <quint32, QPair<QSharedPointer<QLCInputSource>, VCWidget *> > m_inputSourcesMap;

};


#endif
