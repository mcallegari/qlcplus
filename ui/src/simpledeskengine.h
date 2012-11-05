/*
  Q Light Controller
  simpledeskengine.h

  Copyright (c) Heikki Junnila

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

#ifndef SIMPLEDESKENGINE_H
#define SIMPLEDESKENGINE_H

#include <QObject>
#include <QMutex>
#include <QHash>
#include <QList>

#include "dmxsource.h"
#include "cue.h"

#define KXMLQLCSimpleDeskEngine "Engine"

class UniverseArray;
class QDomDocument;
class QDomElement;
class MasterTimer;
class CueStack;
class Doc;

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

    /** Set a complete cue to universe */
    void setCue(const Cue& cue);

    /** Get universe contents as a Cue */
    Cue cue() const;

    /** Reset all universe values to zero */
    void resetUniverse();

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
    QMutex m_mutex;

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
    void writeDMX(MasterTimer* timer, UniverseArray* ua);

};

#endif
