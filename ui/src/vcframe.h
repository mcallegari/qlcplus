/*
  Q Light Controller
  vcframe.h

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

#ifndef VCFRAME_H
#define VCFRAME_H

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>

#include "vcwidget.h"

class QDomDocument;
class QDomElement;
class QMouseEvent;
class QString;

#define KXMLQLCVCFrame "Frame"
#define KXMLQLCVCFrameAllowChildren "AllowChildren"
#define KXMLQLCVCFrameAllowResize "AllowResize"
#define KXMLQLCVCFrameIsCollapsed "Collapsed"

class VCFrame : public VCWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VCFrame)

public:
    /** Default size for newly-created frames */
    static const QSize defaultSize;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCFrame(QWidget* parent, Doc* doc, bool canCollapse = false);
    virtual ~VCFrame();

    void init(bool bottomFrame = false);

    /* Check if this is the virtual console's draw area */
    bool isBottomFrame();


    /*********************************************************************
     * GUI
     *********************************************************************/
public:
    /** @reimp */
    void setCaption(const QString& text);

    bool isCollapsed();

protected slots:
    void slotCollapseButtonToggled(bool toggle);

protected:
    QHBoxLayout *m_hbox;
    QToolButton *m_button;
    QLabel *m_label;
    bool m_collapsed;
    int m_width, m_height;

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    /** Create a copy of this widget into the given parent */
    VCWidget* createCopy(VCWidget* parent);

protected:
    /** Copy the contents for this widget from another widget */
    bool copyFrom(VCWidget* widget);

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    /** @reimp */
    void editProperties();

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXML(const QDomElement* root);
    bool saveXML(QDomDocument* doc, QDomElement* vc_root);

    /**
     * @reimp
     *
     * Propagates the postLoad() call to all children.
     */
    void postLoad();

protected:
    /** Can be overridden by subclasses */
    virtual QString xmlTagName() const;

    /*********************************************************************
     * Custom menu
     *********************************************************************/
public:
    /** Get a custom menu specific to this widget. Ownership is transferred
        to the caller, which must delete the returned menu pointer. */
    virtual QMenu* customMenu(QMenu* parentMenu);

    /*********************************************************************
     * Event handlers
     *********************************************************************/
protected:
    void handleWidgetSelection(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
};

#endif
