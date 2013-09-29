/*
  Q Light Controller
  vcsoloframe.h

  Copyright (c) Anders Thomsen

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

#ifndef VCSOLOFRAME_H
#define VCSOLOFRAME_H

#include "vcwidget.h"
#include "vcframe.h"
#include "function.h"

class QDomDocument;
class QDomElement;
class QMouseEvent;
class QString;
class Doc;

#define KXMLQLCVCSoloFrame "SoloFrame"

class VCSoloFrame : public VCFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(VCSoloFrame)

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    VCSoloFrame(QWidget* parent, Doc* doc, bool canCollapse = false);
    virtual ~VCSoloFrame();

    /*************************************************************************
     * Clipboard
     *************************************************************************/
public:
    /** @reimp */
    VCWidget* createCopy(VCWidget* parent);

    /*************************************************************************
    * Solo behaviour
    *************************************************************************/
protected:
    bool thisIsNearestSoloFrameParent(QWidget* widget);

protected slots:
    virtual void slotModeChanged(Doc::Mode mode);
    void slotButtonFunctionStarting();


    /*********************************************************************
     * Web access
     *********************************************************************/
public:
    /** @reimpl */
    QString getCSS();

    /*************************************************************************
     * Load & Save
     *************************************************************************/
protected:
    virtual QString xmlTagName() const;

    /*************************************************************************
     * Event handlers
     *************************************************************************/
protected:
    virtual void paintEvent(QPaintEvent* e);

};

#endif
