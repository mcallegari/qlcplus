/*
  Q Light Controller Plus
  vcframe.h

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

#ifndef VCFRAME_H
#define VCFRAME_H

#include "vcwidget.h"

#define KXMLQLCVCFrame "Frame"
#define KXMLQLCVCFrameAllowChildren "AllowChildren"
#define KXMLQLCVCFrameAllowResize   "AllowResize"
#define KXMLQLCVCFrameShowHeader    "ShowHeader"
#define KXMLQLCVCFrameIsCollapsed   "Collapsed"
#define KXMLQLCVCFrameIsDisabled    "Disabled"
#define KXMLQLCVCFrameEnableSource  "Enable"
#define KXMLQLCVCFrameShowEnableButton "ShowEnableButton"

#define KXMLQLCVCFrameMultipage   "Multipage"
#define KXMLQLCVCFramePagesNumber "PagesNum"
#define KXMLQLCVCFrameCurrentPage "CurrentPage"
#define KXMLQLCVCFrameKey         "Key"
#define KXMLQLCVCFrameNext        "Next"
#define KXMLQLCVCFramePrevious    "Previous"
#define KXMLQLCVCFramePagesLoop   "PagesLoop"

class VirtualConsole;

class VCFrame : public VCWidget
{
    Q_OBJECT

    Q_PROPERTY(bool showHeader READ showHeader WRITE setShowHeader NOTIFY showHeaderChanged)
    Q_PROPERTY(bool showEnable READ showEnable WRITE setShowEnable NOTIFY showEnableChanged)
    Q_PROPERTY(bool multipageMode READ multipageMode WRITE setMultipageMode NOTIFY multipageModeChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCFrame(Doc* doc = NULL, VirtualConsole *vc = NULL, QObject *parent = 0);
    virtual ~VCFrame();

    virtual void render(QQuickView *view, QQuickItem *parent);

protected:
    /** Reference to the Virtual Console, used to add new widgets */
    VirtualConsole *m_vc;

    /*********************************************************************
     * Children
     *********************************************************************/
public:
    bool hasChildren();

    QList<VCWidget *>children();

    Q_INVOKABLE void addWidget(QQuickItem *parent, QString wType, QPoint pos);

protected:
    /** List holdin the Frame children
     *  To be decided if we need a QMap here */
    QList<VCWidget *> m_childrenList;

    /*********************************************************************
     * Header
     *********************************************************************/
public:
    /** Return if this Frame must show a header or not */
    bool showHeader() const;

    /** Set if this Frame must show a header */
    void setShowHeader(bool showHeader);

signals:
    void showHeaderChanged(bool showHeader);

protected:
    bool m_showHeader;

    /*********************************************************************
     * Enable button
     *********************************************************************/
public:
    /** Return if this Frame must show a enable button */
    bool showEnable() const;

    /** Set if this Frame must show a enable button */
    void setShowEnable(bool showEnable);

signals:
    void showEnableChanged(bool showEnable);

protected:
    bool m_showEnable;

    /*********************************************************************
     * Multi page mode
     *********************************************************************/
public:
    bool multipageMode() const;

    void setMultipageMode(bool multipageMode);

signals:
    void multipageModeChanged(bool multipageMode);

protected:
    bool m_multipageMode;

    /*********************************************************************
     * Load & Save
     *********************************************************************/

public:
    bool loadXML(const QDomElement* root);
    //bool saveXML(QDomDocument* doc, QDomElement* vc_root, bool mainFrame = false);

protected:
    /** Can be overridden by subclasses */
    virtual QString xmlTagName() const;
};

#endif
