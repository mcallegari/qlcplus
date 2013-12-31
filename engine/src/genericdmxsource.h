/*
  Q Light Controller
  genericdmxsource.h

  Copyright (C) Heikki Junnila

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

#ifndef GENERICDMXSOURCE_H
#define GENERICDMXSOURCE_H

#include <QMutex>
#include <QPair>
#include <QMap>

#include "scenevalue.h"
#include "dmxsource.h"

class Doc;

/**
 * This is a generic DMX source, that registers itself to doc->masterTimer() when
 * started and unregisters when deleted. Values set with set() are written to
 * UniverseArray on each writeDMX() call (called by MasterTimer); HTP values continuously
 * and LTP values only once (after which they will be removed from m_values).
 */
class GenericDMXSource : public DMXSource
{
public:
    GenericDMXSource(Doc* doc);
    ~GenericDMXSource();

    /** Set the value of a fixture channel */
    void set(quint32 fxi, quint32 ch, uchar value);

    /** Unset the value of a fixture channel */
    void unset(quint32 fxi, quint32 ch);

    /** Enable/disable output */
    void setOutputEnabled(bool enable);

    /** Check, whether output is enabled */
    bool isOutputEnabled() const;

    /** @reimp */
    void writeDMX(MasterTimer* timer, QList<Universe*> ua);

private:
    Doc* m_doc;
    QMutex m_mutex;
    QMap <QPair<quint32,quint32>,uchar> m_values;
    bool m_outputEnabled;
};

#endif
