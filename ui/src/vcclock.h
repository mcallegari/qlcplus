/*
  Q Light Controller
  vcclock.h

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

#ifndef VCCLOCK_H
#define VCCLOCK_H

#include "vcwidget.h"

class QDomDocument;
class QDomElement;
class QPaintEvent;
class InputMap;
class Doc;

#define KXMLQLCVCClock "Clock"

class VCClock : public VCWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VCClock)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCClock(QWidget* parent, Doc* doc);
    ~VCClock();

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    VCWidget* createCopy(VCWidget* parent);

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    void editProperties();

    /*****************************************************************************
     * External input
     *****************************************************************************/
    /** @reimp */
    void updateFeedback() { }

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXML(const QDomElement* root);
    bool saveXML(QDomDocument* doc, QDomElement* vc_root);

    /*********************************************************************
     * Painting
     *********************************************************************/
protected:
    void paintEvent(QPaintEvent* e);
};

#endif

