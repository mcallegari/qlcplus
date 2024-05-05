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

#define KXMLQLCVCFrame              QString("Frame")
#define KXMLQLCVCFrameAllowChildren QString("AllowChildren")  // LEGACY
#define KXMLQLCVCFrameAllowResize   QString("AllowResize")
#define KXMLQLCVCFrameShowHeader    QString("ShowHeader")
#define KXMLQLCVCFrameIsCollapsed   QString("Collapsed")
#define KXMLQLCVCFrameIsDisabled    QString("Disabled")
#define KXMLQLCVCFrameEnableSource  QString("Enable")
#define KXMLQLCVCFrameShowEnableButton QString("ShowEnableButton")
#define KXMLQLCVCFramePIN           QString("PIN")

#define KXMLQLCVCFrameMultipage     QString("Multipage")
#define KXMLQLCVCFramePagesNumber   QString("PagesNum")
#define KXMLQLCVCFrameCurrentPage   QString("CurrentPage")
#define KXMLQLCVCFrameKey           QString("Key")
#define KXMLQLCVCFrameNext          QString("Next")
#define KXMLQLCVCFramePrevious      QString("Previous")
#define KXMLQLCVCFramePagesLoop     QString("PagesLoop")
#define KXMLQLCVCFrameShortcut      QString("Shortcut")
#define KXMLQLCVCFrameShortcutPage  QString("Page")
#define KXMLQLCVCFrameShortcutName  QString("Name")

class VirtualConsole;

class VCFrame : public VCWidget
{
    Q_OBJECT

    Q_PROPERTY(bool showHeader READ showHeader WRITE setShowHeader NOTIFY showHeaderChanged)
    Q_PROPERTY(bool showEnable READ showEnable WRITE setShowEnable NOTIFY showEnableChanged)
    Q_PROPERTY(bool isCollapsed READ isCollapsed WRITE setCollapsed NOTIFY collapsedChanged)
    Q_PROPERTY(bool multiPageMode READ multiPageMode WRITE setMultiPageMode NOTIFY multiPageModeChanged)
    Q_PROPERTY(bool pagesLoop READ pagesLoop WRITE setPagesLoop NOTIFY pagesLoopChanged)
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int totalPagesNumber READ totalPagesNumber WRITE setTotalPagesNumber NOTIFY totalPagesNumberChanged)
    Q_PROPERTY(int PIN READ PIN WRITE setPIN NOTIFY PINChanged)
    Q_PROPERTY(QStringList pageLabels READ pageLabels NOTIFY pageLabelsChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCFrame(Doc* doc = nullptr, VirtualConsole *vc = nullptr, QObject *parent = nullptr);
    virtual ~VCFrame();

    /** @reimp */
    virtual QString defaultCaption();

    /** @reimp */
    void setupLookAndFeel(qreal pixelDensity, int page);

    /** @reimp */
    virtual void render(QQuickView *view, QQuickItem *parent);

    /** @reimp */
    QString propertiesResource() const;

    /** @reimp */
    VCWidget *createCopy(VCWidget *parent);

protected:
    /** @reimp */
    bool copyFrom(const VCWidget* widget);

protected:
    /** Reference to the Virtual Console, used to add new widgets */
    VirtualConsole *m_vc;

    /*********************************************************************
     * Children
     *********************************************************************/
public:
    /** Returns if this frame has chidren widgets */
    bool hasChildren();

    /** Returns a list of the children widgets with the specified
     *  $recursive method */
    QList<VCWidget *>children(bool recursive = false);

    /** Add a new widget of type $wType at position $pos to this frame.
     *  $parent is used only to render the new widget */
    Q_INVOKABLE void addWidget(QQuickItem *parent, QString wType, QPoint pos);

    /** Add an existing widget at position $pos to this frame.
     *  $parent is used only to render the new widget */
    void addWidget(QQuickItem *parent, VCWidget *widget, QPoint pos);

    /** Add a matrix of widgets with the specified parameters:
     *
     *  @param parent the parent item to render the matrix
     *  @param matrixType is the type of matrix (buttons, sliders)
     *  @param pos the matrix position within this frame
     *  @param matrixSize the matrix size (columns x rows)
     *  @param widgetSize the individual widget size in pixel
     *  @param soloFrame the type of Frame to create as container for the widget matrix
     */
    Q_INVOKABLE void addWidgetMatrix(QQuickItem *parent, QString matrixType, QPoint pos,
                                     QSize matrixSize, QSize widgetSize, bool soloFrame = false);

    /** Add a list of widgets previously copied to the VC clipboard
     *
     *  @param parent the parent item to render the matrix
     *  @param idsList a list of VC widget IDs
     *  @param pos the matrix position within this frame
     */
    Q_INVOKABLE void addWidgetsFromClipboard(QQuickItem *parent, QVariantList idsList, QPoint pos);

    /** Add all the Functions IDs in $idsList at position $pos to this frame.
     *  $keyModifiers determines the type of widget to create:
     *      Shift: VC Slider in Adjust mode controlling intensity
     *      Ctrl: VC Cue List (only when dropping Chasers)
     *      None: VC Button
     *  $parent is used only to render the new widget */
    Q_INVOKABLE void addFunctions(QQuickItem *parent, QVariantList idsList, QPoint pos, int keyModifiers);

    /** Delete all the frame children */
    void deleteChildren();

    /** Add a child widget to the frame page map */
    virtual void addWidgetToPageMap(VCWidget *widget);

    /** Remove the child $widget from the frame page map */
    virtual void removeWidgetFromPageMap(VCWidget *widget);

protected:
    void setupWidget(VCWidget *widget, int page);

    void checkSubmasterConnection(VCWidget *widget);

    /*********************************************************************
     * Disable state
     *********************************************************************/
public:
    /** @reimp */
    void setDisabled(bool disable);

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

    QStringList pageLabels();
    Q_INVOKABLE void setShortcutName(int pageIndex, QString name);

    Q_INVOKABLE void gotoPreviousPage();
    Q_INVOKABLE void gotoNextPage();
    Q_INVOKABLE void gotoPage(int pageIndex);
    Q_INVOKABLE void cloneFirstPage();

signals:
    void multiPageModeChanged(bool multiPageMode);
    void pagesLoopChanged(bool loop);
    void currentPageChanged(int page);
    void totalPagesNumberChanged(int num);
    void pageLabelsChanged();

protected:
    /** Flag to enable/disable multiple pages on this frame */
    bool m_multiPageMode;
    /** The currently selected page of this frame */
    ushort m_currentPage;
    /** The total number of pages of this frame */
    ushort m_totalPagesNumber;
    /** Flag to cycle through pages when reaching the end */
    bool m_pagesLoop;
    /** Map of the page index/label */
    QMap<int, QString> m_pageLabels;

    /** This holds a map of pages/widgets to be
     *  shown/hidden when page is changed */
    QMap <VCWidget *, int> m_pagesMap;

    /*********************************************************************
     * PIN
     *********************************************************************/
public:
    /** Get/Set a protection PIN for this Frame. Note that only top level frames
     *  will expose this functionality */
    int PIN() const;
    void setPIN(int newPIN);

    /** Validate the Frame PIN for the entire session */
    void validatePIN();

    /** Returns true if this Frame has a PIN set and has not been validated for the session.
     *  Otherwise false is returned, and the Frame can be displayed by everyone */
    Q_INVOKABLE bool requirePIN() const;

signals:
    void PINChanged(int PIN);

protected:
    int m_PIN;
    bool m_validatedPIN;

    /*********************************************************************
     * Widget Function
     *********************************************************************/
protected slots:
    virtual void slotFunctionStarting(VCWidget *widget, quint32 fid, qreal fIntensity = 1.0);

    /*********************************************************************
     * Submasters
     *********************************************************************/
protected slots:
    void slotSubmasterValueChanged(qreal value);

    /*********************************************************************
     * External input
     *********************************************************************/
public:
    /** @reimp */
    void updateFeedback();

public slots:
    /** @reimp */
    void slotInputValueChanged(quint8 id, uchar value);

    /*********************************************************************
     * Load & Save
     *********************************************************************/

public:
    bool loadWidgetXML(QXmlStreamReader &root, bool render = false);
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc);

protected:
    /** Can be overridden by subclasses */
    virtual QString xmlTagName() const;
};

#endif
