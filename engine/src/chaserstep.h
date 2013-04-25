/*
  Q Light Controller
  chaserstep.h

  Copyright (C) 2004 Heikki Junnila

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

#ifndef CHASERSTEP_H
#define CHASERSTEP_H

#include <QVariant>
#include "scenevalue.h"
#include "function.h"

class QDomDocument;
class QDomElement;

/**
 * A ChaserStep encapsulates a function ID with fade in, fade out and duration
 * speeds (in milliseconds). Thus, each step can optionally use step-specific
 * speeds if the user so wishes.
 */
class ChaserStep
{
    /************************************************************************
     * Initialization
     ***********************************************************************/
public:
    /** Construct a new ChaserStep with the given attributes */
    ChaserStep(quint32 aFid = Function::invalidId(),
               uint aFadeIn = 0, uint aHold = 0, uint aFadeOut = 0);

    /** Copy constructor */
    ChaserStep(const ChaserStep& cs);

    /** Comparison operator (only function IDs are compared) */
    bool operator==(const ChaserStep& cs) const;

    /** Return the actual function pointer for $fid from $doc */
    Function* resolveFunction(const Doc* doc) const;

    /************************************************************************
     * QVariant operations
     ***********************************************************************/
public:
#if 1
    /** Construct a new ChaserStep from the given QVariant */
    static ChaserStep fromVariant(const QVariant& var);

    /** Construct a QVariant from a ChaserStep */
    QVariant toVariant() const;
#endif
    /************************************************************************
     * Load & Save
     ***********************************************************************/
public:
    /** Load ChaserStep contents from $root and return step index in $stepNumber */
    bool loadXML(const QDomElement& root, int& stepNumber);

    /** Save ChaserStep contents to $doc, under $root with $stepNumber */
    bool saveXML(QDomDocument* doc, QDomElement* root, int stepNumber) const;

public:
    quint32 fid;                 //! The function ID
    uint fadeIn;                 //! Fade in speed
    uint hold;                   //! Hold time
    uint fadeOut;                //! Fade out speed
    uint duration;               //! Duration
    QList <SceneValue> values;   //! specific DMX values for this step (chaser in sequence mode)
    QString note;
};

#endif
