/*
  Q Light Controller
  virtualconsole.h

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

#ifndef VIRTUALCONSOLE_H
#define VIRTUALCONSOLE_H

#include <QKeySequence>
#include <QWidget>
#include <QFrame>
#include <QList>

#include "vcproperties.h"
#include "doc.h"

class VirtualConsole;
class QDomDocument;
class QActionGroup;
class QVBoxLayout;
class QScrollArea;
class QDomElement;
class VCDockArea;
class QKeyEvent;
class QToolBar;
class VCWidget;
class VCFrame;
class QAction;
class KeyBind;
class QMenu;

/** @addtogroup ui_vc
 * @{
 */

class VirtualConsole : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VirtualConsole)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VirtualConsole(QWidget* parent, Doc* doc);
    ~VirtualConsole();

    /** Get the singleton instance */
    static VirtualConsole* instance();

    Doc *getDoc();

protected:
    /** Create a new channels group ID */
    quint32 newWidgetId();

protected:
    static VirtualConsole* s_instance;
    Doc* m_doc;

private:
    /** Latest assigned fixture group ID */
    quint32 m_latestWidgetId;

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    /** Get Virtual Console properties (read-only) */
    VCProperties properties() const;

private:
    VCProperties m_properties;

    /*********************************************************************
     * Selected widgets
     *********************************************************************/
public:
    enum EditAction { EditNone, EditCut, EditCopy };

    /** Set the edit action for selected widgets */
    void setEditAction(EditAction action);

    /** Get the edit action for selected widgets */
    EditAction editAction() const;

    /** Get a list of currently selected widgets */
    const QList <VCWidget*> selectedWidgets() const;

    /** Either select or unselect a widget */
    void setWidgetSelected(VCWidget* widget, bool selected);

    /** Check, whether the given widget is selected */
    bool isWidgetSelected(VCWidget* widget) const;

    /** Clear the list of selected widgets */
    void clearWidgetSelection();

    /** Reselect widgets (== clear selection and select) */
    void reselectWidgets();

protected:
    /** The widgets that are currently selected */
    QList <VCWidget*> m_selectedWidgets;

    /** The widgets that are currently either copied or cut */
    QList <VCWidget*> m_clipboard;

    /** Indicates, whether the selected widgets should be copied or cut */
    EditAction m_editAction;

    /*********************************************************************
     * Actions, menu- and toolbar
     *********************************************************************/
public:
    QMenu* customMenu() const;
    QMenu* toolsMenu() const;
    QMenu* editMenu() const;
    QMenu* addMenu() const;

protected:
    /** Initialize actions */
    void initActions();

    /** Initialize menus and toolbar */
    void initMenuBar();

    /** Change the custom menu to the last selected widget's menu */
    void updateCustomMenu();

    /** Enable or disable actions based on current selection */
    void updateActions();

protected:
    QToolBar* m_toolbar;

    QActionGroup* m_addActionGroup;
    QActionGroup* m_editActionGroup;
    QActionGroup* m_bgActionGroup;
    QActionGroup* m_fgActionGroup;
    QActionGroup* m_fontActionGroup;
    QActionGroup* m_frameActionGroup;
    QActionGroup* m_stackingActionGroup;

    QAction* m_addButtonAction;
    QAction* m_addButtonMatrixAction;
    QAction* m_addSliderAction;
    QAction* m_addSliderMatrixAction;
    QAction* m_addKnobAction;
    QAction* m_addSpeedDialAction;
    QAction* m_addXYPadAction;
    QAction* m_addCueListAction;
    QAction* m_addFrameAction;
    QAction* m_addSoloFrameAction;
    QAction* m_addLabelAction;
    QAction* m_addAudioTriggersAction;
    QAction* m_addClockAction;
    QAction* m_addAnimationAction;

    QAction* m_toolsSettingsAction;

    QAction* m_editCutAction;
    QAction* m_editCopyAction;
    QAction* m_editPasteAction;
    QAction* m_editDeleteAction;
    QAction* m_editPropertiesAction;
    QAction* m_editRenameAction;

    QAction* m_bgColorAction;
    QAction* m_bgImageAction;
    QAction* m_bgDefaultAction;

    QAction* m_fgColorAction;
    QAction* m_fgDefaultAction;

    QAction* m_fontAction;
    QAction* m_resetFontAction;

    QAction* m_frameSunkenAction;
    QAction* m_frameRaisedAction;
    QAction* m_frameNoneAction;

    QAction* m_stackingRaiseAction;
    QAction* m_stackingLowerAction;

protected:
    QMenu* m_customMenu;
    QMenu* m_editMenu;
    QMenu* m_addMenu;

    /*********************************************************************
     * Add menu callbacks
     *********************************************************************/
private:
    /**
     * Attempt to find the closest parent for new widget that accepts children
     *
     * @return Closest parent VCWidget* that allows children
     */
    VCWidget* closestParent() const;

    /**
     * If a newly created widget belongs to a multipage frame,
     * then assign the current frame page to the widget and
     * inform the frame of the new addition.
     * This shall be called every time a widget is added in a frame.
     */
    void connectWidgetToParent(VCWidget *widget, VCWidget *parent);

    /**
     * If a widget is moved away from a frame (because of
     * a deletion or a cut/paste), this shall be called.
     */
    void disconnectWidgetFromParent(VCWidget *widget, VCWidget *parent);

public slots:
    void slotAddButton();
    void slotAddButtonMatrix();
    void slotAddSlider();
    void slotAddSliderMatrix();
    void slotAddKnob();
    void slotAddSpeedDial();
    void slotAddXYPad();
    void slotAddCueList();
    void slotAddFrame();
    void slotAddSoloFrame();
    void slotAddLabel();
    void slotAddAudioTriggers();
    void slotAddClock();
    void slotAddAnimation();

    /*********************************************************************
     * Tools menu callbacks
     *********************************************************************/
public slots:
    void slotToolsSettings();

    /*********************************************************************
     * Edit menu callbacks
     *********************************************************************/
public slots:
    void slotEditCut();
    void slotEditCopy();
    void slotEditPaste();
    void slotEditDelete();
    void slotEditRename();
    void slotEditProperties();

    /*********************************************************************
     * Background menu callbacks
     *********************************************************************/
public slots:
    void slotBackgroundColor();
    void slotBackgroundImage();
    void slotBackgroundNone();

    /*********************************************************************
     * Foreground menu callbacks
     *********************************************************************/
public slots:
    void slotForegroundColor();
    void slotForegroundNone();

    /*********************************************************************
     * Font menu callbacks
     *********************************************************************/
public slots:
    void slotFont();
    void slotResetFont();

    /*********************************************************************
     * Frame menu callbacks
     *********************************************************************/
public slots:
    void slotFrameSunken();
    void slotFrameRaised();
    void slotFrameNone();

    /*********************************************************************
     * Stacking menu callbacks
     *********************************************************************/
public slots:
    void slotStackingRaise();
    void slotStackingLower();

    /*********************************************************************
     * Dock Area
     *********************************************************************/
public:
    /** Get a pointer to the dock area that holds the default sliders */
    VCDockArea* dockArea() const;

protected:
    /** Initialize default sliders */
    void initDockArea();

protected:
    /** Dock area that holds the default sliders */
    VCDockArea* m_dockArea;

    /*********************************************************************
     * Contents
     *********************************************************************/
public:
    /** Get the Virtual Console's current contents */
    VCFrame* contents() const;

    /** Reset the Virtual Console contents to an initial state */
    void resetContents();

    void addWidgetInMap(VCWidget* widget);
    void setupWidget(VCWidget *widget, VCWidget *parent);

    VCWidget *widget(quint32 id);

protected:
    /** Place the contents area to the VC view */
    void initContents();

    QList<VCWidget *> getChildren(VCWidget *obj);

protected:
    QVBoxLayout* m_contentsLayout;
    QScrollArea* m_scrollArea;
    VCFrame* m_contents;
    QHash <quint32, VCWidget *> m_widgetsMap;

    /*********************************************************************
     * Key press handler
     *********************************************************************/
public:
    /** Check if the tap modifier is currently pressed */
    bool isTapModifierDown() const;

protected:
    /** Handler for keyboard key presse events */
    void keyPressEvent(QKeyEvent* event);

    /** Handler for keyboard key release events */
    void keyReleaseEvent(QKeyEvent* event);

signals:
    /** Signal telling that the keySequence was pressed down */
    void keyPressed(const QKeySequence& keySequence);

    /** Signal telling that the keySequence was released */
    void keyReleased(const QKeySequence& keySequence);

private:
    bool m_tapModifierDown;

    /*********************************************************************
     * Main application mode
     *********************************************************************/
public:
    /**
     * Check if the startup function is associated to a Virtual Console
     * widget. In particular, Chasers associated to Cue Lists will run
     * with a CueListRunner instead of a ChaserRunner.
     *
     * @param fid The ID of the startup function
     * @return true in case Virtual Console started the function
     *         false in case Doc has to start it
     */
    bool checkStartupFunction(quint32 fid);

private:
    bool m_liveEdit;
public:
    /** Toggle Virtual Console live editting */
    void toggleLiveEdit();
    bool liveEdit() const;

private:
    /** Enable or disable the Virtual Console edit tools */
    void enableEdit();
    void disableEdit();

public slots:
    /** Slot that catches main application mode changes */
    void slotModeChanged(Doc::Mode mode);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /** Load properties and contents from an XML tree */
    bool loadXML(const QDomElement& root);

    /** Save properties and contents to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

    /** Do post-load cleanup & checks */
    void postLoad();

signals:
    void loaded();
};

/** @} */

#endif
