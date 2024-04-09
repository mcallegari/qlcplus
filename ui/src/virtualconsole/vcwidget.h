/*
  Q Light Controller Plus
  vcwidget.h

  Copyright (c) Heikki Junnila
                Massimo Callegari

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

#ifndef VCWIDGET_H
#define VCWIDGET_H

#include <QKeySequence>
#include <QWidget>
#include "doc.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class QLCInputSource;
class QPaintEvent;
class QMouseEvent;
class QString;
class QMenu;
class QFile;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCCaption    QString("Caption")
#define KXMLQLCVCFrameStyle QString("FrameStyle")

#define KXMLQLCVCWidgetID           QString("ID")
#define KXMLQLCVCWidgetPage         QString("Page")
#define KXMLQLCVCWidgetAppearance   QString("Appearance")

#define KXMLQLCVCWidgetForegroundColor  QString("ForegroundColor")
#define KXMLQLCVCWidgetBackgroundColor  QString("BackgroundColor")
#define KXMLQLCVCWidgetColorDefault     QString("Default")

#define KXMLQLCVCWidgetFont         QString("Font")
#define KXMLQLCVCWidgetFontDefault  QString("Default")

#define KXMLQLCVCWidgetBackgroundImage      QString("BackgroundImage")
#define KXMLQLCVCWidgetBackgroundImageNone  QString("None")

#define KVCFrameStyleSunken (QFrame::Panel | QFrame::Sunken)
#define KVCFrameStyleRaised (QFrame::Panel | QFrame::Raised)
#define KVCFrameStyleNone   (QFrame::NoFrame)

#define KXMLQLCVCWidgetKey                  QString("Key")
#define KXMLQLCVCWidgetInput                QString("Input")
#define KXMLQLCVCWidgetInputUniverse        QString("Universe")
#define KXMLQLCVCWidgetInputChannel         QString("Channel")
#define KXMLQLCVCWidgetInputLowerValue      QString("LowerValue")
#define KXMLQLCVCWidgetInputUpperValue      QString("UpperValue")
#define KXMLQLCVCWidgetInputMonitorValue    QString("MonitorValue")
#define KXMLQLCVCWidgetInputLowerParams     QString("LowerParams")
#define KXMLQLCVCWidgetInputUpperParams     QString("UpperParams")
#define KXMLQLCVCWidgetInputMonitorParams   QString("MonitorParams")

#define KXMLQLCWindowState          QString("WindowState")
#define KXMLQLCWindowStateVisible   QString("Visible")
#define KXMLQLCWindowStateX         QString("X")
#define KXMLQLCWindowStateY         QString("Y")
#define KXMLQLCWindowStateWidth     QString("Width")
#define KXMLQLCWindowStateHeight    QString("Height")

class VCWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VCWidget)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCWidget(QWidget* parent, Doc* doc);
    virtual ~VCWidget();

protected:
    Doc* m_doc;

    /*********************************************************************
     * ID
     *********************************************************************/
public:
    /**
     * Set this widget's unique ID
     *
     * @param id This widget's unique ID
     */
    virtual void setID(quint32 id);

    /**
     * Get this widget's unique ID
     */
    quint32 id() const;

    /**
     * Get the value for an invalid widget ID (for comparison etc.)
     */
    static quint32 invalidId();

private:
    quint32 m_id;

    /*********************************************************************
     * Type
     *********************************************************************/
public:
    enum WidgetType
    {
        UnknownWidget,
        ButtonWidget,
        SliderWidget,
        XYPadWidget,
        FrameWidget,
        SoloFrameWidget,
        SpeedDialWidget,
        CueListWidget,
        LabelWidget,
        AudioTriggersWidget,
        AnimationWidget,
        ClockWidget
    };

public:
    /** Set the widget's type */
    void setType(int type);

    /** Get the widget's type */
    int type();

    static QString typeToString(int type);
    static QIcon typeToIcon(int type);

protected:
    int m_type;

    /*********************************************************************
     * Disable state
     *********************************************************************/
    /**
     * The disable state works in conjunction with the QLC+ operate mode.
     * Only VC Frames can set/unset the disable state of themselves and
     * their children widgets.
     * A widget in disable state cannot be clicked and won't accept external
     * input signals.
     */
public:
    /**
     * Virtual method that sets the disable state flag. Afterward,
     * it calls enableWidgetUI which (if defined) turns the VCWidget
     * graphics element into a QWidget disable state.
     */
    virtual void setDisableState(bool disable);

    virtual void enableWidgetUI(bool enable);

    bool isDisabled();

signals:
    void disableStateChanged(bool disable);

protected:
    bool m_disableState;

    /*********************************************************************
     * Page
     *********************************************************************/
public:
    void setPage(int pNum);
    int page();

protected:
    int m_page;

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    /** Create a copy of this widget into the given parent and return it */
    virtual VCWidget* createCopy(VCWidget* parent) = 0;

protected:
    /** Copy the contents for this widget from the given widget */
    virtual bool copyFrom(const VCWidget* widget);

    /*********************************************************************
     * Background image
     *********************************************************************/
public:
    /** Set the widget's background image */
    virtual void setBackgroundImage(const QString& path);

    /** Get the widget's background image */
    virtual QString backgroundImage() const;

protected:
    QString m_backgroundImage;

    /*********************************************************************
     * Background color
     *********************************************************************/
public:
    /** Set the widget's background color and invalidate background image */
    virtual void setBackgroundColor(const QColor& color);

    /** Get the widget's background color. The color is invalid if the
        widget has a background image. */
    virtual QColor backgroundColor() const;

    /** Check, whether the widget has a custom background color */
    virtual bool hasCustomBackgroundColor() const;

    /** Reset the widget's background color to whatever the platform uses */
    virtual void resetBackgroundColor();

protected:
    bool m_hasCustomBackgroundColor;

    /*********************************************************************
     * Foreground color
     *********************************************************************/
public:
    /** Set the widget's foreground color */
    virtual void setForegroundColor(const QColor& color);

    /** Get the widget's foreground color */
    virtual QColor foregroundColor() const;

    /** Check, whether the widget has a custom foreground color */
    virtual bool hasCustomForegroundColor() const;

    /** Reset the widget's foreground color to whatever the platform uses */
    virtual void resetForegroundColor();

protected:
    bool m_hasCustomForegroundColor;

    /*********************************************************************
     * Font
     *********************************************************************/
public:
    /** Set the font used for the widget's caption */
    virtual void setFont(const QFont& font);

    /** Get the font used for the widget's caption */
    virtual QFont font() const;

    /** Check, whether the widget has a custom font */
    virtual bool hasCustomFont() const;

    /** Reset the font used for the widget's caption to whatever the
        platform uses */
    virtual void resetFont();

protected:
    bool m_hasCustomFont;

    /*********************************************************************
     * Caption
     *********************************************************************/
public:
    /** Set this widget's caption text */
    virtual void setCaption(const QString& text);

    /** Get this widget's caption text */
    virtual QString caption() const;

    /*********************************************************************
     * Frame style
     *********************************************************************/
public:
    /** Set the widget's frame style (Using QFrame::Shape) */
    void setFrameStyle(int style);

    /** Get the widget's frame style */
    int frameStyle() const;

    /** Reset frame style to QFrame::None */
    void resetFrameStyle();

    /** Convert the given frame style to a string */
    static QString frameStyleToString(int style);

    /** Convert the given string to frame style */
    static int stringToFrameStyle(const QString& style);

protected:
    int m_frameStyle;

    /*********************************************************************
     * Allow adding children
     *********************************************************************/
public:
    /** Set, whether the widget can contain children. */
    void setAllowChildren(bool allow);

    /**
     * Check, if the widget can contain children. This property is not saved
     * in XML by VCWidget because the default behaviour for all widgets is to
     * not allow children. Widgets that make an exception to this can save the
     * property if needed. Otherwise, if all widgets had this property in
     * the workspace file, user could (hack it and) allow children under any
     * widget, which is bad mmkay.
     */
    bool allowChildren() const;

private:
    bool m_allowChildren;

    /*********************************************************************
     * Allow resizing
     *********************************************************************/
public:
    /**
     * Set, whether the widget can be resized. This property is not saved
     * in XML by VCWidget because the default behaviour for all widgets is to
     * always allow resizing. Widgets that make an exception to this can save
     * the property if needed.
     */
    void setAllowResize(bool allow);

    /** Check if the widget can be resized */
    bool allowResize() const;

private:
    bool m_allowResize;

    /*********************************************************************
     * Widget Function
     *********************************************************************/
public:
    /** This is a virtual method for VCWidgets attached to a Function.
     *  At the moment only Buttons, Sliders (in playback mode) and Cue Lists
     *  can benefit from this.
     *  Basically when placed in a Solo frame, with this method it is
     *  possible to stop the currently running Function */
    virtual void notifyFunctionStarting(quint32 fid, qreal intensity)
    { Q_UNUSED(fid); Q_UNUSED(intensity); }

    virtual void adjustFunctionIntensity(Function *f, qreal value);

    void resetIntensityOverrideAttribute();

signals:
    /** Signal emitted when a VCWidget controlling a Function has been
      * requested to start the Function.
      * At the moment this is used by a restriceted number of widgets (see above)
      */
    void functionStarting(quint32 fid, qreal intensity = 1.0);

protected:
    int m_intensityOverrideId;

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    /** Invoke the property dialog if the widget has one */
    virtual void editProperties();

    /*********************************************************************
     * Intensity
     *********************************************************************/
public:
    /** Set the widget intensity value. This is mostly used by submasters */
    virtual void adjustIntensity(qreal val);

    virtual qreal intensity() const;

private:
    qreal m_intensity;

    /*********************************************************************
     * External input
     *********************************************************************/
public:

    /**
     * Helper method to check if the widget is in a state to accept external inputs
     */
    bool acceptsInput();

    /**
     * Check the input source with the given id against
     * the given universe and channel
     *
     * @param universe the target universe to compare to
     * @param channel the target channel to compare to
     * @param value the value received in case a relative source needs to be updated
     * @param sender the QObject that sent the event. This is used to check
     *               if the event is synthetic or coming from an external controller
     * @param id the source ID to check
     * @return true in case source and target matches and the event
     *         can pass through, otherwise false
     */
    bool checkInputSource(quint32 universe, quint32 channel,
                          uchar value, QObject *sender, quint32 id = 0);

    /**
     * Set external input $source to listen to. If a widget supports more
     * than one input source, specify an $id for each input source. Setting
     * multiple sources under the same id overwrites the previous ones.
     *
     * @param source The input source to set
     * @param id The id of the source (default: 0)
     */
    void setInputSource(QSharedPointer<QLCInputSource> const& source, quint8 id = 0);

    /**
     * Get an assigned external input source. Without parameters the
     * method returns the first input source (if any).
     *
     * @param id The id of the source to get
     */
    QSharedPointer<QLCInputSource> inputSource(quint8 id = 0) const;

    /**
     * When cloning a widget on a multipage frame, this function
     * will remap the original input source to respond to a new
     * page source
     */
    void remapInputSources(int pgNum);

    /**
     * Send feedback to an external controller.
     *
     * @param value value from 0 to 255 to be sent
     * @param id ID of the input source where to send feedback
     */
    void sendFeedback(int value, quint8 id = 0);

    /**
     * Send feedback to an external controller.
     *
     * @param value value from 0 to 255 to be sent
     * @param src the QLCInputSource reference to send the feedback to
     */
    void sendFeedback(int value, QSharedPointer<QLCInputSource> src, QVariant extraParams = QVariant());

    /**
     * Send the feedback data again, e.g. after page flip
     */
    virtual void updateFeedback() = 0;

protected slots:
    /**
     * Slot that receives external input data. Overwrite in subclasses to
     * get input data to your widget.
     *
     * @param universe Input universe
     * @param channel Input channel
     * @param value New value for universe & value
     */
    virtual void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

    /**
     * Slot called when an input profile has been changed and
     * at least one input source has been set
     *
     * @param universe the profile universe
     * @param profileName the profile name
     */
    virtual void slotInputProfileChanged(quint32 universe, const QString& profileName);

protected:
    QHash <quint8, QSharedPointer<QLCInputSource> > m_inputs;

    /*********************************************************************
     * Key sequence handler
     *********************************************************************/
public:
    /** Strip restricted keys from the given QKeySequence */
    static QKeySequence stripKeySequence(const QKeySequence& seq);

protected slots:
    /** Handle key presses. Default implementation passes to children. */
    virtual void slotKeyPressed(const QKeySequence& keySequence);

    /** Handle key releases. Default implementation passes to children. */
    virtual void slotKeyReleased(const QKeySequence& keySequence);

signals:
    /** Tell listeners that a key was pressed */
    void keyPressed(const QKeySequence& keySequence);

    /** Tell listeners that a key was released */
    void keyReleased(const QKeySequence& keySequence);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    virtual bool loadXML(QXmlStreamReader &root) = 0;
    virtual bool saveXML(QXmlStreamWriter *doc) = 0;

    /**
     * Called for every VCWidget-based object after everything has been loaded.
     * Do any post-load cleanup, function mappings etc. if needed. Default
     * implementation does nothing.
     */
    virtual void postLoad();

    static QSharedPointer<QLCInputSource> getXMLInput(QXmlStreamReader &root);

    /** Save input source from a $src input source to $root */
    static bool saveXMLInput(QXmlStreamWriter *doc, const QLCInputSource *src);
    /** Save input source from a $src input source to $root */
    static bool saveXMLInput(QXmlStreamWriter *doc, QSharedPointer<QLCInputSource> const& src);

protected:
    bool loadXMLCommon(QXmlStreamReader &root);
    bool loadXMLAppearance(QXmlStreamReader &appearance_root);
    bool loadXMLInput(QXmlStreamReader &root, const quint8& id = 0);

    /** Parse an input XML section and:
     *  - set an input source with the given $sourceID
     *  - return a string of a KeySequence if present
     */
    QString loadXMLSources(QXmlStreamReader &root, quint8 sourceID);

    /** Load input source from $root to $uni and $ch */
    bool loadXMLInput(QXmlStreamReader &root, quint32* uni, quint32* ch) const;

    static QString extraParamToString(QVariant param);
    bool saveXMLCommon(QXmlStreamWriter *doc);
    bool saveXMLAppearance(QXmlStreamWriter *doc);
    /** Save the defualt input source to $root */
    bool saveXMLInput(QXmlStreamWriter *doc);

    /**
     * Write this widget's geometry and visibility to an XML document.
     *
     * @param doc A QXmlStreamReader to save the tag to
     *
     * @return true if succesful, otherwise false
     */
    bool saveXMLWindowState(QXmlStreamWriter *doc);

    /**
     * Read this widget's geometry and visibility from an XML tag.
     *
     * @param tag A QXmlStreamReader under which the window state is saved
     * @param x Loaded x position
     * @param y Loaded y position
     * @param w Loaded w position
     * @param h Loaded h position
     * @param visible Loaded visible status
     *
     * @return true if succesful, otherwise false
     */
    bool loadXMLWindowState(QXmlStreamReader &tag, int* x, int* y,
                            int* w, int* h, bool* visible);

    /*********************************************************************
     * QLC+ Mode change
     *********************************************************************/
public:
    /**
     * Virtual method that sets the liveEdit flag.
     * If widget is not disabled, this calls enableWidgetUI.
     */
    virtual void setLiveEdit(bool liveEdit);
    void cancelLiveEdit();

protected slots:
    /** Listens to Doc mode changes */
    virtual void slotModeChanged(Doc::Mode mode);

protected:
    /** Shortcut for inheritors to check current mode */
    /** Does not reflect application mode, but virtualconsole mode */
    Doc::Mode mode() const;

protected:
    bool m_liveEdit;

    /*********************************************************************
     * Widget menu
     *********************************************************************/
protected:
    /** Invoke a context menu */
    virtual void invokeMenu(const QPoint& point);

    /*********************************************************************
     * Custom menu
     *********************************************************************/
public:
    /** Get a custom menu specific to this widget. Ownership is transferred
        to the caller, which must delete the returned menu pointer. */
    virtual QMenu* customMenu(QMenu* parentMenu);

    /*********************************************************************
     * Widget move & resize
     *********************************************************************/
public:
    /** Resize this widget to the given size. */
    virtual void resize(const QSize& size);

    /** Move this widget to the given point */
    virtual void move(const QPoint& point);

    /** Get the point where the mouse was clicked last in this widget */
    QPoint lastClickPoint() const;

protected:
    QPoint m_mousePressPoint;
    bool m_resizeMode;

    /*********************************************************************
     * Event handlers
     *********************************************************************/
protected:
    virtual void paintEvent(QPaintEvent* e);

    virtual void mousePressEvent(QMouseEvent* e);
    virtual void handleWidgetSelection(QMouseEvent* e);

    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mouseDoubleClickEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
};

/** @} */

#endif
