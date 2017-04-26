/*
  Q Light Controller Plus
  vcwidget.h

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

#ifndef VCWIDGET_H
#define VCWIDGET_H

#include <QQuickView>
#include <QQuickItem>
#include <QObject>
#include <QColor>
#include <QRect>
#include <QFont>

#include "qlcfile.h"
#include "doc.h"


#define KXMLQLCVCCaption "Caption"
#define KXMLQLCVCFrameStyle "FrameStyle"    // LEGACY

#define KXMLQLCVCWidgetID "ID"
#define KXMLQLCVCWidgetPage "Page"
#define KXMLQLCVCWidgetAppearance "Appearance"

#define KXMLQLCVCWidgetForegroundColor "ForegroundColor"
#define KXMLQLCVCWidgetBackgroundColor "BackgroundColor"
#define KXMLQLCVCWidgetColorDefault "Default"

#define KXMLQLCVCWidgetFont "Font"
#define KXMLQLCVCWidgetFontDefault "Default"

#define KXMLQLCVCWidgetBackgroundImage "BackgroundImage"
#define KXMLQLCVCWidgetBackgroundImageNone "None"

#define KXMLQLCWindowState "WindowState"
#define KXMLQLCWindowStateVisible "Visible"
#define KXMLQLCWindowStateX "X"
#define KXMLQLCWindowStateY "Y"
#define KXMLQLCWindowStateWidth "Width"
#define KXMLQLCWindowStateHeight "Height"

#define KXMLQLCVCWidgetKey "Key"
#define KXMLQLCVCWidgetInput "Input"
#define KXMLQLCVCWidgetInputUniverse "Universe"
#define KXMLQLCVCWidgetInputChannel "Channel"
#define KXMLQLCVCWidgetInputLowerValue "LowerValue"
#define KXMLQLCVCWidgetInputUpperValue "UpperValue"

typedef struct
{
    quint8 id;
    QString name;
    bool allowKeyboard;
} ExternalControlInfo;

class VCWidget : public QObject
{
    Q_OBJECT

    Q_PROPERTY(quint32 id READ id CONSTANT)
    Q_PROPERTY(QString propertiesResource READ propertiesResource CONSTANT)
    Q_PROPERTY(bool isEditing READ isEditing WRITE setIsEditing NOTIFY isEditingChanged)
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry NOTIFY geometryChanged)
    Q_PROPERTY(bool allowResize READ allowResize WRITE setAllowResize NOTIFY allowResizeChanged)
    Q_PROPERTY(bool isDisabled READ isDisabled WRITE setDisabled NOTIFY disabledStateChanged)
    Q_PROPERTY(bool isVisible READ isVisible WRITE setVisible NOTIFY isVisibleChanged)
    Q_PROPERTY(QString caption READ caption WRITE setCaption NOTIFY captionChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QString backgroundImage READ backgroundImage WRITE setBackgroundImage NOTIFY backgroundImageChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY foregroundColorChanged)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(int page READ page WRITE setPage NOTIFY pageChanged)

    Q_PROPERTY(int externalControlsCount READ externalControlsCount CONSTANT)
    Q_PROPERTY(QVariant externalControlsList READ externalControlsList CONSTANT)
    Q_PROPERTY(QVariant inputSourcesList READ inputSourcesList NOTIFY inputSourcesListChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/

public:
    VCWidget(Doc* doc = NULL, QObject* parent = 0);
    virtual ~VCWidget();

    void setDocModified();

    virtual void render(QQuickView *view, QQuickItem *parent);

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

    /** Convert a widget's type to a string */
    static QString typeToString(int type);

    /** Convert a widget's type to a SVG icon */
    static QString typeToIcon(int type);

    /** Convert a widget's type string into type */
    static WidgetType stringToType(QString str);

protected:
    int m_type;

    /*********************************************************************
     * Geometry
     *********************************************************************/
public:
    /** Get this widget's geometry */
    QRect geometry() const;

    /** Set this widget's geometry. x/y position is relative to
     *  the widget's parent */
    void setGeometry(QRect rect);

signals:
    void geometryChanged(QRect rect);

protected:
    QRect m_geometry;

    /*********************************************************************
     * Allow resize
     *********************************************************************/
public:
    bool allowResize() const;

    void setAllowResize(bool allowResize);

signals:
    void allowResizeChanged(bool allowResize);

protected:
    bool m_allowResize;

    /*********************************************************************
     * Disable state
     *********************************************************************/
    /**
     * A widget in disable state cannot be clicked and won't accept external
     * input signals.
     * Only VC Frames can set/unset the disable state of themselves and
     * their children widgets.
     */
public:
    /** Get the widget's disable state */
    bool isDisabled();

    /** Set the widget's disable state flag */
    virtual void setDisabled(bool disable);

signals:
    void disabledStateChanged(bool state);

protected:
    bool m_isDisabled;

    /*********************************************************************
     * Visibility state
     *********************************************************************/
public:
    void setVisible(bool isVisible);

    bool isVisible() const;
signals:
    void isVisibleChanged(bool isVisible);

protected:
    bool m_isVisible;

    /*********************************************************************
     * Caption
     *********************************************************************/
public:
    /** Virtual method to retrieve the widget default name which is "Widget ID" */
    virtual QString defaultCaption();

    /** Get this widget's caption text */
    QString caption() const;

    /** Set this widget's caption text */
    void setCaption(QString caption);

signals:
    void captionChanged(QString caption);

protected:
    QString m_caption;

    /*********************************************************************
     * Background color
     *********************************************************************/
public:

    /** Get the widget's background color. The color is invalid if the
        widget has a background image. */
    QColor backgroundColor() const;

    /** Set the widget's background color and invalidate background image */
    void setBackgroundColor(QColor backgroundColor);

    /** Check, whether the widget has a custom background color */
    bool hasCustomBackgroundColor() const;

    /** Reset the widget's background color to default */
    void resetBackgroundColor();

signals:
    void backgroundColorChanged(QColor backgroundColor);

protected:
    QColor m_backgroundColor;
    bool m_hasCustomBackgroundColor;

    /*********************************************************************
     * Background image
     *********************************************************************/
public:
    /** Set the widget's background image */
    void setBackgroundImage(QString path);

    /** Get the widget's background image */
    QString backgroundImage() const;

signals:
    void backgroundImageChanged(const QString& path);

protected:
    QString m_backgroundImage;

    /*********************************************************************
     * Foreground color
     *********************************************************************/
public:
    /** Get the widget's foreground color */
    QColor foregroundColor() const;

    /** Set the widget's foreground color */
    void setForegroundColor(QColor foregroundColor);

    /** Check, whether the widget has a custom foreground color */
    bool hasCustomForegroundColor() const;

    /** Reset the widget's foreground color to whatever the platform uses */
    void resetForegroundColor();

signals:
    void foregroundColorChanged(QColor foregroundColor);

protected:
    QColor m_foregroundColor;
    bool m_hasCustomForegroundColor;

    /*********************************************************************
     * Font
     *********************************************************************/
public:
    /** Set the initial size of the font for the widget's caption.
     *  This doesn't affect the custom font flag */
    void setDefaultFontSize(qreal size);

    /** Set the font used for the widget's caption */
    void setFont(const QFont& font);

    /** Get the font used for the widget's caption */
    QFont font() const;

    /** Check, whether the widget has a custom font */
    bool hasCustomFont() const;

    /** Reset the font used for the widget's caption to the default */
    void resetFont();

signals:
    void fontChanged();

protected:
    QFont m_font;
    bool m_hasCustomFont;

    /*********************************************************************
     * Page
     *********************************************************************/
public:
    void setPage(int pNum);
    int page();

signals:
    void pageChanged(int page);

protected:
    int m_page;

    /*********************************************************************
     * Widget Function
     *********************************************************************/
public:
    /** Return true if the widget's parent is a Solo Frame */
    bool hasSoloParent();

    /** This is a virtual method for VCWidgets attached to a Function.
     *  At the moment only Buttons, Sliders (in playback mode), Cue Lists
     *  and Audio Triggers can benefit from this.
     *  Basically when placed in a Solo frame, with this method it is
     *  possible to stop the currently running Function */
    virtual void notifyFunctionStarting(VCWidget *widget, quint32 fid, qreal fIntensity);

signals:
    /** Signal emitted when a VCWidget controlling a Function has been
      * requested to start the Function.
      * At the moment this is used by a restriceted number of widgets (see above)
      */
    void functionStarting(VCWidget *widget, quint32 fid, qreal intensity = 1.0);

    /*********************************************************************
     * Intensity
     *********************************************************************/
public:
    /** Set the widget intensity value. This is mostly used by submasters */
    virtual void adjustIntensity(qreal val);

    virtual qreal intensity();

private:
    qreal m_intensity;

    /*********************************************************************
     * Properties editing
     *********************************************************************/
public:
    /** Return the widget editing flag */
    bool isEditing() const;

    /** Set the widget editing flag. This is used to indicate that
     *  the properties page is open and it can be useful to defer
     *  some actions */
    void setIsEditing(bool edit);

    virtual QString propertiesResource() const;

signals:
    void isEditingChanged();

private:
    bool m_isEditing;

    /*********************************************************************
     * External inputs
     *********************************************************************/
public:
    enum InputSourceTypes { Controller, Keyboard };
    Q_ENUM(InputSourceTypes)

    /************************
     * Controls
     ************************/
    /** Register some external control information known by this widget
     *
     *  @param id a unique id identifying the external control
     *  @param name a translatable string to be displayed by the UI
     *  @param allowKeyboard a boolean flag indicating if the external control
     *         can be a keyboard sequence
     */
    void registerExternalControl(quint8 id, QString name, bool allowKeyboard);

    /** Returns the number of external controls registered by this widget */
    int externalControlsCount() const;

    /** Returns a list of the registered external controls suitable for the UI */
    QVariant externalControlsList() const;

    /************************
     * Input sources
     ************************/
    /**
     * Add an external input $source to the sources known by thie widget.
     *
     * @param source The input source to add
     */
    void addInputSource(QSharedPointer<QLCInputSource> const& source);

    /** Update an existing input source with the provided $universe and $channel */
    bool updateInputSource(const QSharedPointer<QLCInputSource> &source, quint32 universe, quint32 channel);

    /** Update the control ID of an existing input source bound to $universe and $channel */
    bool updateInputSourceControlID(quint32 universe, quint32 channel, quint32 id);

    /** Delete an existing input source from this widget */
    void deleteInputSurce(quint32 id, quint32 universe, quint32 channel);

    /** Return a list of references to the input sources currently
     *  added to this widget */
    QList <QSharedPointer<QLCInputSource> > inputSources() const;

    /** Return a list of input sources to be used by the UI */
    QVariant inputSourcesList() const;

    /** Return a input source reference that matches the specified $id, $universe and $channel */
    QSharedPointer<QLCInputSource> inputSource(quint32 id, quint32 universe, quint32 channel) const;

    /************************
     * Key sequences
     ************************/
    /** Add a new key sequence to this widget, bound to the specified control $id */
    void addKeySequence(const QKeySequence& keySequence, const quint32& id = 0);

    /** Delete an existing key sequence from this widget */
    void deleteKeySequence(const QKeySequence& keySequence);

    /** Update an existing key sequence with the specified $id */
    void updateKeySequence(QKeySequence oldSequence, QKeySequence newSequence, const quint32 id = 0);

    /** Update the control ID of an existing key sequence */
    void updateKeySequenceControlID(QKeySequence sequence, quint32 id);

public slots:
    /** Virtual slot called when an input value changed */
    virtual void slotInputValueChanged(quint8 id, uchar value);

signals:
    void inputSourcesListChanged();

protected:
    /** A list of the external controls known by this widget */
    QList <ExternalControlInfo> m_externalControlList;

    /** The list of input sources that can control this widget */
    QList <QSharedPointer<QLCInputSource> > m_inputSources;

    /** The map of key sequences that can control this widget,
     *  arranged by sequence / control ID */
    QMap <QKeySequence, quint32> m_keySequenceMap;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    virtual bool loadXML(QXmlStreamReader &root);
    virtual bool saveXML(QXmlStreamWriter *doc);

protected:
    bool loadXMLCommon(QXmlStreamReader &root);

    /**
     * Read this widget's appearance XML tag, to load properties
     * such as background and foreground color, font, etc..
     */
    bool loadXMLAppearance(QXmlStreamReader &root);

    /**
     * Read this widget's geometry and visibility from an XML tag.
     *
     * @param tag A QXmlStreamReader from which to read the window state
     * @param x Loaded x position
     * @param y Loaded y position
     * @param w Loaded w position
     * @param h Loaded h position
     * @param visible Loaded visible status
     *
     * @return true if succesful, otherwise false
     */
    bool loadXMLWindowState(QXmlStreamReader &root, int* x, int* y,
                            int* w, int* h, bool* visible);

    /** Load an input source from $root with the given $id */
    bool loadXMLInputSource(QXmlStreamReader &root, const quint8& id = 0);

    /** Load an input key sequence from $root with the given $id */
    bool loadXMLInputKey(QXmlStreamReader &root, const quint8& id = 0);

    /** Parse the $root XML section and:
     *  - set an input source with the given $id
     *  - if present, set an input key sequence with the given $id
     */
    bool loadXMLSources(QXmlStreamReader &root, const quint8& id);

    /** Save the widget common properties */
    bool saveXMLCommon(QXmlStreamWriter *doc);

    /** Save the widget appearance, if customized */
    bool saveXMLAppearance(QXmlStreamWriter *doc);

    /**
     * Write this widget's geometry and visibility to an XML document.
     *
     * @param doc A QXmlStreamReader to save the tag to
     *
     * @return true if succesful, otherwise false
     */
    bool saveXMLWindowState(QXmlStreamWriter *doc);
};

#endif
