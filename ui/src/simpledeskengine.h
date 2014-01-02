/*
  Q Light Controller
  simpledeskengine.h

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

#ifndef SIMPLEDESKENGINE_H
#define SIMPLEDESKENGINE_H

#include <QObject>
#include <QMutex>
#include <QHash>
#include <QList>

#include "dmxsource.h"
#include "cue.h"

class UniverseArray;
class QDomDocument;
class QDomElement;
class MasterTimer;
class CueStack;
class Doc;

/** @addtogroup ui_simpledesk
 * @{
 */

#define KXMLQLCSimpleDeskEngine "Engine"

class SimpleDeskEngine : public QObject, public DMXSource
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    SimpleDeskEngine(Doc* doc);
    virtual ~SimpleDeskEngine();

    /** Start from scratch; clear everything */
    void clearContents();

private:
    /** Get the parent Doc object */
    Doc* doc() const;

    /************************************************************************
     * Universe Values
     ************************************************************************/
public:
    /** Set the value of a single channel */
    void setValue(uint channel, uchar value);

    /** Get the value of a single channel */
    uchar value(uint channel) const;

    bool hasChannel(uint channel);

    /** Set a complete cue to universe */
    void setCue(const Cue& cue);

    /** Get universe contents as a Cue */
    Cue cue() const;

    /** Reset the values of the given universe to zero */
    void resetUniverse(int universe);

private:
    QHash <uint,uchar> m_values;

    /************************************************************************
     * Cue Stacks
     ************************************************************************/
public:
    /** Get (and create if necessary) a cue stack with the given stack ID */
    CueStack* cueStack(uint stack);

signals:
    /** Tells that the current cue within cuestack $stack has changed to $index */
    void currentCueChanged(uint stack, int index);

    /** Tells that the cuestack $stack was started */
    void cueStackStarted(uint stack);

    /** Tells that the cuestack $stack was stopped */
    void cueStackStopped(uint stack);

private:
    CueStack* createCueStack();

private slots:
    void slotCurrentCueChanged(int index);
    void slotCueStackStarted();
    void slotCueStackStopped();

private:
    QHash <uint,CueStack*> m_cueStacks;
    mutable QMutex m_mutex;

    /************************************************************************
     * Save & Load
     ************************************************************************/
public:
    /** Load SimpleDeskEngine contents from the given XML $root tag */
    bool loadXML(const QDomElement& root);

    /** Save SimpleDeskEngine content to the given XML $doc, under $wksp_root */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root) const;

    /************************************************************************
     * DMXSource
     ************************************************************************/
public:
    /** @reimpl */
    void writeDMX(MasterTimer* timer, QList<Universe*> ua);

};

/** @} */

#endif
