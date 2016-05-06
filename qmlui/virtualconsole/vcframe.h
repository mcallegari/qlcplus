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
#define KXMLQLCVCFrameAllowChildren "AllowChildren"  // LEGACY
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
    Q_PROPERTY(bool isCollapsed READ isCollapsed WRITE setCollapsed NOTIFY collapsedChanged)
    Q_PROPERTY(bool multiPageMode READ multiPageMode WRITE setMultiPageMode NOTIFY multiPageModeChanged)
    Q_PROPERTY(int currentPage READ currentPage NOTIFY currentPageChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/

public:
    VCFrame(Doc* doc = NULL, VirtualConsole *vc = NULL, QObject *parent = 0);
    virtual ~VCFrame();

    virtual void render(QQuickView *view, QQuickItem *parent);

    /** Method used to indicate if this Frame has a SoloFrame parent
     *  at any lower level. This is used to determine if
     *  children widget should be connected to handle the Solo Frame
     *  feature */
    void setHasSoloParent(bool hasSoloParent);

    /** Returns if this Frame has a Solo Frame parent at any lower level */
    bool hasSoloParent() const;

protected:
    /** Reference to the Virtual Console, used to add new widgets */
    VirtualConsole *m_vc;

    /** Flag that holds if this frame has a Solo parent widget */
    bool m_hasSoloParent;

    /*********************************************************************
     * Children
     *********************************************************************/
public:
    /** Returns if this frame has chidren widgets */
    bool hasChildren();

    QList<VCWidget *>children(bool recursive = false);

    Q_INVOKABLE void addWidget(QQuickItem *parent, QString wType, QPoint pos);
    Q_INVOKABLE void addFunction(QQuickItem *parent, quint32 funcID, QPoint pos, bool modifierPressed);

    void deleteChildren();

protected:
    void setupWidget(VCWidget *widget);
    void deleteWidget(VCWidget *widget);

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
     * Collapsed state
     *********************************************************************/
public:
    bool isCollapsed() const;

    void setCollapsed(bool isCollapsed);

signals:
    void collapsedChanged(bool isCollapsed);

protected:
    bool m_isCollapsed;

    /*********************************************************************
     * Multi page mode
     *********************************************************************/
public:
    bool multiPageMode() const;
    void setMultiPageMode(bool multiPageMode);

    void setTotalPagesNumber(int num);
    int totalPagesNumber() const;

    int currentPage() const;
    void setCurrentPage(int pageNum);

    void setPagesLoop(bool pagesLoop);
    bool pagesLoop() const;

    Q_INVOKABLE void gotoPreviousPage();
    Q_INVOKABLE void gotoNextPage();

signals:
    void multiPageModeChanged(bool multiPageMode);
    void currentPageChanged(int page);

protected:
    /** Flag to enable/disable multiple pages on this frame */
    bool m_multiPageMode;
    /** The currently selected page of this frame */
    ushort m_currentPage;
    /** The total number of pages of this frame */
    ushort m_totalPagesNumber;
    /** Flag to cycle through pages when reaching the end */
    bool m_pagesLoop;

    /** Here's where the magic takes place. This holds a map
     *  of pages/widgets to be shown/hidden when page is changed */
    QMap <VCWidget *, int> m_pagesMap;

    /*********************************************************************
     * Widget Function
     *********************************************************************/
protected slots:
    virtual void slotFunctionStarting(VCWidget *widget, quint32 fid, qreal fIntensity = 1.0);

    /*********************************************************************
     * Load & Save
     *********************************************************************/

public:
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc);

protected:
    /** Can be overridden by subclasses */
    virtual QString xmlTagName() const;
};

#endif
