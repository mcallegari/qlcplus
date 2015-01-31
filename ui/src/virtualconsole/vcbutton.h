/*
  Q Light Controller
  vcbutton.h

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

#ifndef VCBUTTON_H
#define VCBUTTON_H

#include <QKeySequence>
#include <QWidget>
#include <QIcon>

#include "vcwidget.h"

class QDomDocument;
class QDomElement;
class QMouseEvent;
class QPaintEvent;
class VCButton;
class QAction;
class QPoint;
class QEvent;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCButton "Button"
#define KXMLQLCVCButtonIcon "Icon"

#define KXMLQLCVCButtonFunction "Function"
#define KXMLQLCVCButtonFunctionID "ID"

#define KXMLQLCVCButtonAction "Action"
#define KXMLQLCVCButtonActionFlash "Flash"
#define KXMLQLCVCButtonActionToggle "Toggle"
#define KXMLQLCVCButtonActionBlackout "Blackout"
#define KXMLQLCVCButtonActionStopAll "StopAll"

#define KXMLQLCVCButtonStopAllFadeTime "FadeOut"

#define KXMLQLCVCButtonKey "Key"

#define KXMLQLCVCButtonIntensity "Intensity"
#define KXMLQLCVCButtonIntensityAdjust "Adjust"

class VCButton : public VCWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VCButton)

public:
    /** Default size for newly-created buttons */
    static const QSize defaultSize;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCButton(QWidget* parent, Doc* doc);
    ~VCButton();

    /*********************************************************************
     * ID
     *********************************************************************/
public:
    /** @reimpl */
    void setID(quint32 id);

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    /** Create a copy of this widget to the given parent */
    VCWidget* createCopy(VCWidget* parent);

protected:
    /** Copy the contents for this widget from another widget */
    bool copyFrom(const VCWidget* widget);

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    /** Edit this widget's properties */
    void editProperties();

    /*********************************************************************
     * Background image
     *********************************************************************/
public:
    /* Don't allow background image setting for buttons */
    void setBackgroundImage(const QString& path);

    /*********************************************************************
     * Background color
     *********************************************************************/
public:
    /** Set the button's background color */
    void setBackgroundColor(const QColor& color);

    /** Get the button's background color */
    QColor backgroundColor() const;

    /** Reset the button's background color to whatever the platform uses */
    void resetBackgroundColor();

    /*********************************************************************
     * Foreground color
     *********************************************************************/
public:
    /** Set the button's foreground color */
    void setForegroundColor(const QColor& color);

    /** Get the button's foreground color */
    QColor foregroundColor() const;

    /** Reset the button's foreground color to whatever the platform uses */
    virtual void resetForegroundColor();

    /*********************************************************************
     * Button icon
     *********************************************************************/
public:

    /** Get the button icon's path
        @return absolute path
     */
    QString iconPath() const;

    /** Set the icon's path 
     */
    void setIconPath(const QString& iconPath);

private:

    /** Reload icon file from disk */
    void updateIcon();
    
public slots:
    void slotChooseIcon();
    void slotResetIcon();

protected:
    QString m_iconPath;
    QIcon m_icon;
    QSize m_iconSize;

    QAction* m_chooseIconAction;
    QAction* m_resetIconAction;

    /*********************************************************************
     * Function attachment
     *********************************************************************/
public:
    /**
     * Attach a function to a VCButton. This function is started when the
     * button is pressed down.
     *
     * @param function An ID of a function to attach
     */
    void setFunction(quint32 function);

    /**
     * Get the ID of the function attached to a VCButton
     *
     * @return The ID of the attached function or Function::invalidId()
     *         if there isn't one
     */
    quint32 function() const;

    /** @reimp */
    virtual void notifyFunctionStarting(quint32 fid);

protected slots:
    /** Invalidates the button's function if the function is destroyed */
    void slotFunctionRemoved(quint32 fid);

protected:
    /** The function that this button is controlling */
    quint32 m_function;

    /*********************************************************************
     * Button state
     *********************************************************************/
public:
    void setOn(bool on);
    bool isOn() const;
    void updateOnState();

protected:
    bool m_on;
    bool m_ledStyle;

    /*********************************************************************
     * Key sequence handler
     *********************************************************************/
public:
    void setKeySequence(const QKeySequence& keySequence);
    QKeySequence keySequence() const;

protected slots:
    void slotKeyPressed(const QKeySequence& keySequence);
    void slotKeyReleased(const QKeySequence& keySequence);

protected:
    QKeySequence m_keySequence;

    /*********************************************************************
     * External input
     *********************************************************************/
public:
    void updateFeedback();

protected slots:
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

    /*********************************************************************
     * Button action
     *********************************************************************/
public:
    /**
     * Toggle: Start/stop the assigned function.
     * Flash: Keep the function running as long as the button is kept down.
     * Blackout: Toggle blackout on/off.
     * StopAll: Stop all functions (panic button).
     */
    enum Action { Toggle, Flash, Blackout, StopAll };

    /** Set this button's action */
    void setAction(Action action);

    /** Get this button's action */
    Action action() const;

    static QString actionToString(Action action);
    static Action stringToAction(const QString& str);

    void setStopAllFadeOutTime(int ms);
    int stopAllFadeTime();

protected:
    Action m_action;
    int m_blackoutFadeOutTime;

    /*********************************************************************
     * Startup intensity adjustment
     *********************************************************************/
public:
    /**
     * Make the button adjust the attached function's intensity when the
     * button is used to start the function.
     *
     * @param adjust true to make the button adjust intensity, false to disable
     *               intensity adjustment
     */
    void enableStartupIntensity(bool enable);

    /** Check, whether the button adjusts intensity */
    bool isStartupIntensityEnabled() const;

    /**
     * Set the amount of intensity adjustment.
     *
     * @param fraction Intensity adjustment amount (0.0 - 1.0)
     */
    void setStartupIntensity(qreal fraction);

    /** Get the amount of intensity adjustment. */
    qreal startupIntensity() const;

protected:
    bool m_startupIntensityEnabled;
    qreal m_startupIntensity;

protected slots:
    void slotAttributeChanged(int value);

    /*********************************************************************
     * Button press / release handlers
     *********************************************************************/
public:
    /** Handler for button presses (mouse/key)button down, not click */
    void pressFunction();

    /** Handler for button releases (mouse/key)button up, not click */
    void releaseFunction();

    /** Blink the button for $ms milliseconds */
    void blink(int ms);

protected slots:
    /** Handler for function running signal */
    void slotFunctionRunning(quint32 fid);

    /** Handler for function stop signal */
    void slotFunctionStopped(quint32 fid);

    /** Basically the same as slotFunctionStopped() but for flash signal */
    void slotFunctionFlashing(quint32 fid, bool state);

    /** Slot for brief widget blink when controlled function stops */
    void slotBlink();

    /** Listent to OutputMap::blackoutChanged() signals in Blackout mode */
    void slotBlackoutChanged(bool state);

protected:
    /** Check if the button's parent is a VCSoloFrame */
    bool isChildOfSoloFrame() const;

signals:
    /** Signal emitted when the button has actually changed the graphic state */
    void pressedState(bool on);

    /*********************************************************************
    * Custom menu
    *********************************************************************/
public:
    /** Get a custom menu specific to this widget. Must be deleted. */
    QMenu* customMenu(QMenu* parentMenu);

    /*********************************************************************
     * Intensity
     *********************************************************************/
public:
    /** @reimp */
    void adjustIntensity(qreal val);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /**
     * Load a VCButton's properties from an XML document node
     *
     * @param doc An XML document to load from
     * @param btn_root A VCButton XML root node containing button properties
     * @return true if successful; otherwise false
     */
    bool loadXML(const QDomElement* btn_root);

    /**
     * Save a VCButton's properties to an XML document node
     *
     * @param doc The master XML document to save to
     * @param frame_root The button's VCFrame XML parent node to save to
     */
    bool saveXML(QDomDocument* doc, QDomElement* frame_root);

    /*********************************************************************
     * Event Handlers
     *********************************************************************/
protected:
    void paintEvent(QPaintEvent* e);

    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
};

/** @} */

#endif
