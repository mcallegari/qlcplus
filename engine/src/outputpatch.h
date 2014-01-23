/*
  Q Light Controller
  outputpatch.h

  Copyright (c) Heikki Junnila

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

#ifndef OUTPUTPATCH_H
#define OUTPUTPATCH_H

#include <QObject>

class QLCIOPlugin;

/** @addtogroup engine Engine
 * @{
 */

#define KOutputNone QObject::tr("None")

#define KXMLQLCOutputPatch "Patch"
#define KXMLQLCOutputPatchUniverse "Universe"
#define KXMLQLCOutputPatchPlugin "Plugin"
#define KXMLQLCOutputPatchOutput "Output"

class OutputPatch : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OutputPatch)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    OutputPatch(QObject* parent);
    virtual ~OutputPatch();

    /********************************************************************
     * Plugin & output
     ********************************************************************/
public:
    void set(QLCIOPlugin* plugin, quint32 output);
    void reconnect();

    QLCIOPlugin* plugin() const;
    QString pluginName() const;

    quint32 output() const;
    QString outputName() const;

    bool isPatched() const;

private:
    QLCIOPlugin* m_plugin;
    quint32 m_output;

    /********************************************************************
     * Value dump
     ********************************************************************/
public:
    /** Write the contents of a 512 channel value buffer to the plugin.
      * Called periodically by OutputMap. No need to call manually. */
    void dump(quint32 universe, const QByteArray &data);
};

/** @} */

#endif
